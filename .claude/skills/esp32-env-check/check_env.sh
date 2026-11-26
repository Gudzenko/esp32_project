#!/usr/bin/env bash
# Read-only check of the ESP32 development environment on Linux.
# Usage: check_env.sh [--chip]   (--chip also reads chip info from the board)

ok()   { echo "[OK]   $*"; }
warn() { echo "[WARN] $*"; }
fail() { echo "[FAIL] $*"; }

PIO="$HOME/.platformio/penv/bin/pio"
PY="$HOME/.platformio/penv/bin/python"

# 1. USB-UART adapter (CP210x, CH340, FTDI)
adapters=$(lsusb 2>/dev/null | grep -iE "10c4:ea60|1a86:7523|1a86:55d4|0403:6001")
if [ -n "$adapters" ]; then
    ok "USB-UART adapter: $(echo "$adapters" | sed 's/.*ID //' | paste -sd ';')"
else
    fail "No USB-UART adapter found (cable? board connected?)"
fi

# 2. Serial ports
ports=$(ls /dev/ttyUSB* /dev/ttyACM* 2>/dev/null)
if [ -n "$ports" ]; then
    ok "Serial ports: $(echo $ports)"
else
    fail "No /dev/ttyUSB* or /dev/ttyACM* port"
fi

# 3. brltty can steal USB-UART adapters
if dpkg -l brltty 2>/dev/null | grep -q '^ii'; then
    warn "brltty is installed (may grab the port; remove it if the port disappears)"
fi

# 4. dialout group
if id -nG | grep -qw dialout; then
    ok "User is in dialout (current session)"
elif id -nG "$USER" | grep -qw dialout; then
    fail "User is in dialout, but not in this session: log out and log in"
else
    fail "User is not in dialout group"
fi

# 5. udev rules
if [ -f /etc/udev/rules.d/99-platformio-udev.rules ]; then
    ok "PlatformIO udev rules installed"
else
    warn "PlatformIO udev rules not installed"
fi

# 6. PlatformIO
if [ -x "$PIO" ]; then
    ok "PlatformIO Core: $("$PIO" --version 2>/dev/null)"
else
    fail "PlatformIO Core not installed"
fi
if command -v pio >/dev/null 2>&1; then
    ok "pio in PATH"
else
    warn "pio not in PATH"
fi

# 7. ESP32 platform
if [ -x "$PIO" ]; then
    platform=$("$PIO" pkg list -g 2>/dev/null | grep -oE "── espressif32 @ [0-9.]+" | sed "s/── //")
    if [ -n "$platform" ]; then
        ok "Platform: $platform"
    else
        warn "espressif32 platform not installed globally (it may still be installed by a project build)"
    fi
fi

# 8. Optional: read chip info
if [ "$1" = "--chip" ]; then
    for port in $ports; do
        if fuser "$port" >/dev/null 2>&1; then
            fail "$port is busy (close Serial Monitor)"
            continue
        fi
        echo "--- esptool flash-id on $port ---"
        if out=$("$PY" -m esptool --port "$port" flash-id 2>&1); then
            echo "$out" | grep -E "Chip type|Features|MAC|flash size"
            ok "Board on $port responds"
        else
            echo "$out" | tail -5
            fail "Board on $port does not respond"
        fi
    done
fi
