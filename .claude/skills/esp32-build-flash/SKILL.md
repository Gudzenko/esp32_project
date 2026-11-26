---
name: esp32-build-flash
description: Build a PlatformIO ESP32 project, upload it to the board, and read the board's serial output to check the result. Use after changing firmware code, when the user asks to build/flash/upload/run a project, or to see what the board prints.
---

# Build, flash and check an ESP32 project

Projects live in `projects/NN-name/`. Run all commands from the repository root.

## 1. Build

```bash
pio run -d projects/NN-name
```

Fix all errors **and warnings** (`-Wall -Wextra` is on). Format check:
`clang-format --style=file --dry-run <file>` (binary: `~/.vscode/extensions/ms-vscode.cpptools-*/LLVM/bin/clang-format`).

## 2. Upload

First make sure the port is free: `fuser -v /dev/ttyUSB0`.
If the user's `pio device monitor` holds it, **do not kill it** — ask the user to close it (Ctrl+C).

```bash
pio run -d projects/NN-name -t upload
```

Success: `Hash of data verified.` for every image and `[SUCCESS]`.

## 3. Read the serial output

`pio device monitor` needs an interactive terminal and cannot be used here. Use the script:

```bash
~/.platformio/penv/bin/python -I .claude/skills/esp32-build-flash/serial_read.py --seconds 5
```

It drops stale/replayed data (see `docs/known-issues.md`), resets the board via RTS and prints
the output from the start. Options: `--port`, `--baud` (default 115200), `--no-reset`,
`--no-boot-log` (hide ROM bootloader lines; keep them when looking for the reset reason or a crash).
Lines starting with `---` are script info (stderr).

Compare the output with what the code should print. Report the result to the user.

## 4. Commands for the user

The user works in the terminal. Give upload and monitor as **separate** commands:

```bash
pio run -d projects/NN-name -t upload
pio device monitor -d projects/NN-name
```

If there is garbage at the start of the monitor, press **EN** on the board.

## Common problems

| Message | Cause and fix |
|---|---|
| `Could not open /dev/ttyUSB0, the port is busy` | Monitor is open. Ask the user to close it |
| `Failed to connect to ESP32` | Hold BOOT, press EN, release BOOT, retry. Check cable. Run skill `esp32-env-check` |
| `Permission denied` on the port | `dialout` group: run skill `esp32-env-check` |
| Garbled text in the monitor | `Serial.begin()` speed differs from `monitor_speed` |
| `rst:0x...` with `WDT` or `Guru Meditation Error` | Crash: read the backtrace, the monitor's `esp32_exception_decoder` decodes it |
| `rst:` repeats in a loop (boot loop) | Crash in `setup()`, or a boot pin (GPIO0/2/12/15) is pulled by wiring |
| `boot:0x...(DOWNLOAD_BOOT...)` | Board waits for firmware: BOOT held or GPIO0 is LOW |
