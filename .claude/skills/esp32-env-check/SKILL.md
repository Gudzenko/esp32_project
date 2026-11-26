---
name: esp32-env-check
description: Check the ESP32 development environment and board connection on Linux (USB-UART adapter, serial port, dialout group, udev rules, PlatformIO, ESP32 platform, esptool chip read). Use when setting up a new machine, or when the board is not detected, upload fails, or "Permission denied" / "Failed to connect" errors appear.
---

# ESP32 environment check

Run the check script (read-only, it changes nothing):

```bash
bash .claude/skills/esp32-env-check/check_env.sh          # environment only
bash .claude/skills/esp32-env-check/check_env.sh --chip   # + read chip info via esptool
```

`--chip` talks to the board (resets it). Use it only when the user agrees or asked to check the board.

## Read the result

Each line is `[OK]`, `[WARN]` or `[FAIL]`. For every non-OK line, explain the cause and the fix
to the user, and **do not run sudo commands yourself** — give the user the command.

| Problem | Fix |
|---|---|
| No USB-UART adapter in `lsusb` | Data USB cable (not charge-only), other USB port, check board LED |
| Adapter found but no `/dev/ttyUSB*` | `brltty` grabbed it: `sudo apt remove brltty`, replug |
| Not in `dialout` | `sudo usermod -aG dialout $USER`, then log out / log in |
| In `dialout` in system, not in session | Log out / log in (VS Code restart is not enough) |
| No udev rules | See `docs/setup.md`, step 3 |
| `pio` not found | `ln -sf ~/.platformio/penv/bin/pio ~/.local/bin/pio` |
| No espressif32 platform | See `docs/setup.md`, step 5 |
| esptool `Failed to connect` | Close Serial Monitor (port busy); hold BOOT, press EN, release BOOT; try lower speed |
| Port busy | Another program uses the port: `fuser /dev/ttyUSB0` |

Full setup steps: `docs/setup.md`.
