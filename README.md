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

## Shared libraries

| Library | Description |
|---|---|
| [Button](lib/Button) | Push button to GND with press events |
