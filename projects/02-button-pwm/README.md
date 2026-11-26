# 02-button-pwm

The BOOT button switches LED modes. The LED brightness is controlled with PWM.

## Goal

- Button input: `INPUT_PULLUP`, pressed = `LOW`, press event (edge) instead of level
- First shared library: `lib/Button` (class `bot::Button`)
- PWM with the LEDC hardware: `ledcAttach`, `ledcWrite` (Arduino-ESP32 3.x API)
- Brightness curves: the eye sees brightness non-linearly (gamma)
- Modes with `enum class` and `switch`

## Wiring

Nothing to connect.

| Part | ESP32 pin |
|---|---|
| BOOT button (on board) | GPIO0 |
| Blue LED (on board) | GPIO2 |

GPIO0 is a boot pin: holding BOOT during reset starts the download mode. After start it is a normal input.

## Build and run

```bash
pio run -d projects/02-button-pwm -t upload
pio device monitor -d projects/02-button-pwm
```

## Result

Each BOOT press switches to the next mode:

| Mode | LED |
|---|---|
| `off` (start) | Off |
| `breathing, linear` | Breathing, duty = level. Looks bright most of the time |
| `breathing, quadratic` | Breathing, duty = level². Smoother |
| `breathing, cubic` | Breathing, duty = level³. Long dim phase, looks most natural |
| `blink` | On/off once per second |

Serial prints `mode: <name>` on every change.

## Notes

- The BOOT button showed no contact bounce (probably an RC filter on the board).
  Debounce is not implemented yet: add it to `lib/Button` with the first external button.
- PWM: 5 kHz, 12 bit (duty 0..4095). 8 bit gives visible steps at low brightness with the cubic curve.
- Old tutorials use `ledcSetup()` + `ledcAttachPin()` (Arduino-ESP32 2.x). They do not compile with 3.x.
