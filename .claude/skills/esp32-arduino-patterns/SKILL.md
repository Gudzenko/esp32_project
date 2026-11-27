---
name: esp32-arduino-patterns
description: How to write ESP32 Arduino (PlatformIO) firmware - non-blocking patterns, class structure, firmware architecture (layers, devices, events, Controller), Arduino-ESP32 3.x API pitfalls, unit tests on the PC, hardware safety checklist, project and shared library layout. Use when writing, reviewing or planning firmware code, wiring a new part, or creating a new project or shared library.
---

# ESP32 Arduino patterns

Practices proven in this repository. Add new ones when a project teaches something reusable.

## Code patterns

- **Never block.** No `delay()` in `loop()`. Use `if (now - last >= interval)` with `millis()`.
  It stays correct after the `millis()` overflow (~49.7 days); `now >= last + interval` does not.
- **Classes:** constructor stores configuration only; `begin()` touches hardware (call it in
  `setup()`, global objects are created before the hardware is ready); `update()` is called on every
  `loop()` and never blocks.
- **Events:** `update()` sets a flag, `wasPressed()` returns it and clears it, so each event is
  handled exactly once. Detect the edge (`now && !before`), not the level.
- **Derive state from time** when possible (`breathLevel(now)`, `(now / halfPeriod) % 2`)
  instead of storing and updating it.
- **Modes:** `enum class Mode : uint8_t { ..., kCount }`; next mode via `% kCount`;
  `switch` over all values without `default`, so `-Wall` warns when a new value is not handled.
- **Write to hardware only when the value changes** (`ledcWrite`, `digitalWrite`):
  `loop()` runs ~700 000 times per second.
- **Types:** `uint32_t` for time; `float` (hardware FPU), not `double` (software); literals `2.0f`.
- **Strings:** no `String` concatenation in `loop()` (heap fragmentation); use `Serial.printf`.
  `%lu` for `uint32_t` works only on ESP32. In code that also builds on the PC (unit tests) use
  `"%" PRIu32` from `<cinttypes>`.
- **Style:** `constexpr` named constants (`kLedPin`), no magic numbers, no comments except critical
  hardware reasons and `TODO`s. Format with `.clang-format`.

## Firmware architecture

Proven in `projects/03-wifi-web` (see its README). Start simple, refactor towards this when needed.

- **Hardware classes** (`Led`, `Button`, later `Motor`): only the hardware. No JSON, no network.
  Settings come in the constructor as a struct (`LedSettings`), not from `config.h`:
  then the class can move to `lib/` unchanged.
- **Device adapters** (`LedDevice : Device`): show a hardware class to the protocol
  (`name`, `type`, `describe`, `get`, `set`). The name is given at creation: several devices
  of one type are possible. A list `Devices` finds them by name and knows no concrete types.
- **Behavior** (`Controller`): rules "when X happens, do Y". Knows the hardware classes,
  not the network. Devices do not know each other.
- **Events go up by listeners:** hardware → its device → `Devices` → `main.cpp` → protocol → network.
  Nobody below knows who listens. A change caused by the client's own request is not sent back
  as an event (`Devices` marks the device while `set` runs).
- **`main.cpp` only creates objects and connects them** (`beginHardware()`, `connectMessages()`).
  Group a subsystem into one class when `main.cpp` starts to hold its logic (`Connectivity`).
- **Each layer checks what it owns:** connection (one client, size), protocol (message shape),
  device (property, value). One error type with helpers (`unknownProp`, `readOnly`, `badValue`)
  keeps the messages the same everywhere.
- **Constants:** all in the project `config.h`; secrets in a git-ignored `secrets.h`.

## Arduino-ESP32 3.x pitfalls

Most tutorials on the internet are for 2.x. The 2.x code does not compile with 3.x.

| 2.x | 3.x |
|---|---|
| `ledcSetup(ch, freq, bits); ledcAttachPin(pin, ch); ledcWrite(ch, duty)` | `ledcAttach(pin, freq, bits); ledcWrite(pin, duty)` (by pin; `ledcAttachChannel` for a fixed channel) |
| `timerBegin(0, 80, true)` | `timerBegin(frequencyHz)` |
| `timerAttachInterrupt(t, fn, true)` | `timerAttachInterrupt(t, fn)` |
| `timerAlarmWrite(t, value, reload); timerAlarmEnable(t)` | `timerAlarm(t, value, autoreload, reloadCount)` |
| `hallRead()` | removed |

When unsure, check the header in `~/.platformio/packages/framework-arduinoespressif32/cores/esp32/`.

**Do not name your files like framework libraries** (`Network.h`, `WiFi.h`, `ETH.h`, `WebServer.h`, …):
the library finder takes your file instead, and the build fails in a framework file
(`ETH.h: fatal error: Network.h: No such file`). List: `~/.platformio/packages/framework-arduinoespressif32/libraries/`.

PWM: LEDC hardware, 5 kHz, 12 bit is a good default for LEDs (8 bit shows steps at low brightness).
The eye sees brightness non-linearly: use `level²` or `level³` for smooth fades.

## Memory

- App size limit = app partition size, not 4 MB. Use `min_spiffs.csv` (2 × 1.9 MB with OTA)
  when Wi-Fi/web is used; `huge_app.csv` (3 MB, no OTA) only if really needed.
- Settings go to NVS (`Preferences`), not to the file system.
- Heavy parts: Wi-Fi + web ~1 MB, Bluetooth 250–500 KB. Sensor/motor libraries are small.
- Details and measured numbers: `docs/memory.md`.

## Tests

- **Unit tests on the PC** (no board, not in the firmware): keep logic free of Arduino
  (protocol, parsing, rules), then build it natively with Unity:

  ```ini
  [platformio]
  default_envs = esp32          ; plain `pio run` builds only the firmware

  [env:native]
  platform = native
  test_framework = unity
  test_build_src = yes
  build_src_filter = -<*> +<Protocol.cpp> +<Devices.cpp>
  lib_deps = bblanchon/ArduinoJson@7.4.3
  ```

  Tests in `test/test_<name>/`; replace Arduino-only parts with fakes there (`fake_log.cpp`,
  a fake device). Run: `pio test -d projects/NN-name -e native`. The PC compiler also catches
  non-portable code (wrong `printf` formats).
- **Board tests** over the network: `tools/board_tests` (pytest), see skill `esp32-wifi-web`.
- What cannot be automated (a button press, a router restart) goes to a manual check list
  in the project README.

## Hardware checklist (before wiring)

- Logic is **3.3 V**. A 5 V output (e.g. HC-SR04 Echo) needs a divider or a level shifter.
- **GPIO6–11:** never (flash). **GPIO34–39:** input only, no internal pull-ups.
- **Boot pins GPIO0, 2, 12, 15:** wiring must not change their level at reset
  (GPIO12 high = boot fails; GPIO0 low = download mode).
- **ADC2** does not work while Wi-Fi is on: use ADC1 (GPIO32–39).
- **Current:** keep a GPIO at ≤ 20 mA. LED needs a resistor (220–330 Ω). Relays, motors,
  LED strips need a driver.
- **Motors:** separate power supply, common GND with the ESP32.
- **Buttons:** to GND with `INPUT_PULLUP` (pressed = `LOW`). External buttons bounce: debounce
  in software (TODO in `lib/Button`).

## New project

`projects/NN-name/` with:

```ini
; platformio.ini
[platformio]
extra_configs = ../../platformio-common.ini

[env:esp32]
extends = common_esp32
lib_deps =
    symlink://../../lib/<Library>
```

`src/main.cpp` and `README.md` with sections: Goal, Wiring (table: part → ESP32 pin, power),
Build and run, Result, Notes. Add the project to the table in the root `README.md`.
Run `tools/compiledb.sh` so VS Code IntelliSense knows the new project.

## New shared library

Create a library only when code is (or will clearly be) used by more than one project.

```
lib/Name/
├── library.json          # name, version, description, frameworks: arduino, platforms: espressif32
└── src/
    ├── Name.h            # #pragma once, namespace bot, public API first
    └── Name.cpp
```

Fields with a trailing underscore (`pin_`). Connect with `lib_deps = symlink://../../lib/Name`.
Add it to the libraries table in the root `README.md`.
