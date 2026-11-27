# ESP32 learning project

Learning the ESP32 step by step: LEDs, buttons, sensors, wireless, motors – and later a mobile robot.

- Board: ESP32 DevKit (ESP-WROOM-32, CP2102)
- Language: C++ (Arduino framework)
- Tools: VS Code + PlatformIO
- License: [MIT](LICENSE)

## Setup

See [docs/setup.md](docs/setup.md).

## Projects

| Project | What it teaches |
|---|---|
| [01-blink](projects/01-blink) | `setup`/`loop`, GPIO output, Serial, `millis()` instead of `delay()` |
| [02-button-pwm](projects/02-button-pwm) | Button input, first shared library, PWM, brightness curves, modes |
| [03-wifi-web](projects/03-wifi-web) | Wi-Fi, web page and WebSocket on the board, JSON protocol, layered firmware, devices, events, tests on the PC and on the board |

## Shared libraries

| Library | Description |
|---|---|
| [Button](lib/Button) | Push button to GND with press events and change listeners |

## Tools

| Tool | Description |
|---|---|
| [compiledb.sh](tools/compiledb.sh) | Builds `compile_commands.json` for VS Code IntelliSense ([docs/setup.md](docs/setup.md), step 7) |
| [board_tests](tools/board_tests) | Tests of the [protocol](docs/protocol.md) against a running board (`python3 -m pytest tools/board_tests`) |

## Docs

- [docs/setup.md](docs/setup.md) – environment setup
- [docs/protocol.md](docs/protocol.md) – JSON protocol between the board and clients
- [docs/memory.md](docs/memory.md) – flash partitions and memory use
- [docs/known-issues.md](docs/known-issues.md) – known problems and workarounds
- [docs/ideas/robot-ros.md](docs/ideas/robot-ros.md) – idea: Raspberry Pi with ROS 2 + ESP32 (not decided)
