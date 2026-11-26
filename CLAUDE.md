# ESP32 learning project

A learning project: from first experiments with an ESP32 to a mobile robot (maybe a quadcopter later).
The user knows Python well and C++ only at a basic level. Talk to the user in Russian.

## How we work (most important)

- This is a learning project. Work in **small steps, one step at a time**.
- **Discuss first, implement second.** Do not start coding until the user agrees on the step.
- Explain what you do and why. After each step, let the user check and confirm before moving on.
- Tell the user about open questions, things we may have missed, and hardware nuances
  (voltage, pins, power, safety).
- If a skill or tool could help with the task, say so.
- Keep it minimal: add files and folders only when needed. Refactor later instead of creating unused stuff.
- After each big step, review what can be turned into a new skill or used to improve an existing one,
  so it can be reused in other projects. Suggest it to the user.

## Hardware

- Board: ESP32 DevKit (ESP-WROOM-32, CP2102 USB-UART), 4 MB flash, no PSRAM. Two boards.
- Serial port: usually `/dev/ttyUSB0`. It can change (second board, replug) – check with `pio device list`.
- Logic level is **3.3 V**. Never put 5 V on a GPIO.
- Do not use GPIO6–11 (flash). GPIO34–39 are input only, no pull-ups.
- Boot pins GPIO0, 2, 12, 15: be careful (GPIO12 high at boot breaks boot).
- ADC2 does not work while Wi-Fi is on; use ADC1 (GPIO32–39).
- Motors always get a separate power supply; connect grounds together.

## Stack

- C++ with PlatformIO + Arduino-ESP32 framework, edited in VS Code.
- Platform: pioarduino `55.03.313` = Arduino-ESP32 **3.3.13** + ESP-IDF 5.5 (version is pinned).
  Many examples on the internet are for Arduino-ESP32 2.x – check the API (e.g. LEDC/PWM changed in 3.x).
- Python for tools on the PC (scripts, plotting, data).
- Environment setup: `docs/setup.md`. Problems with the board connection: skill `esp32-env-check`.
- Build, upload, read serial output: skill `esp32-build-flash`. Known problems: `docs/known-issues.md`.
- Shared PlatformIO settings: `platformio-common.ini` (each project `extends = common_esp32`).

## Repository layout

Planned layout. Create folders only when the first real content appears.

- `.claude/skills/` – Claude Code skills for this project
- `docs/` – documentation (`docs/hardware/inventory.md` – parts we actually connected)
- `lib/` – shared libraries used by several projects (linked with `symlink://` in `platformio.ini`)
- `projects/NN-name/` – separate PlatformIO projects, one per learning step
- `templates/` – templates for new projects and libraries
- `tools/` – Python scripts for the PC

## Conventions

- Code, comments and docs: English. Docs are short and simple.
- **No comments in code.** Code must be clear by itself: good names, named constants, small functions.
  Only critical comments (non-obvious hardware reasons, pitfalls) and `TODO`s.
- Code style: `.clang-format` (Google, 4 spaces, 100 columns). Constants `kName`, `constexpr` instead of `#define`.
- Build and upload from the terminal (the user does not reopen VS Code as a workspace):
  `pio run -d projects/NN-name` and `pio run -d projects/NN-name -t upload`.
- Each project has a README: goal, wiring table (ESP32 pin → module pin, power), result.
- **The repository is public (MIT license).** Never commit secrets: Wi-Fi passwords, API keys, tokens,
  private keys. They go to `secrets.h` (git-ignored); commit only `secrets.example.h`.
  Before every commit, check the staged diff for secrets and warn the user if something looks private.
- Commit after each working step (only when the user asks).
