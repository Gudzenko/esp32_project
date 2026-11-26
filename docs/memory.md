# Memory

ESP32-WROOM-32: **4 MB flash** (program and data, kept after power off) and **320 KB RAM**
(working memory, no PSRAM).

## Flash partitions

The flash is split into partitions, like a disk. We use the `min_spiffs` layout
(`board_build.partitions = min_spiffs.csv` in `platformio-common.ini`):

| Partition | Size | Purpose |
|---|---|---|
| bootloader, partition table | ~36 KB | Start the chip, describe the layout |
| `nvs` | 20 KB | Settings (`Preferences` library) |
| `otadata` | 8 KB | Which app slot to boot |
| `app0` | 1.9 MB | Our program (the size limit of the firmware) |
| `app1` | 1.9 MB | Second slot for OTA (firmware update over Wi-Fi) |
| `spiffs` | 128 KB | File system (files: web pages, logs) |
| `coredump` | 64 KB | Crash dump |

Why not the default layout: it has 1.25 MB app slots and 1.4 MB for files.
We do not store files (web pages are in the firmware, settings in `nvs`), and a firmware with
Wi-Fi + web server + WebSocket + JSON already takes ~1.0 MB.

Other layouts in the framework (`~/.platformio/packages/framework-arduinoespressif32/tools/partitions/`):

| Layout | App | OTA | Files |
|---|---|---|---|
| `default.csv` | 1.25 MB | yes | 1.4 MB |
| `min_spiffs.csv` (used) | 1.9 MB | yes | 128 KB |
| `huge_app.csv` | 3 MB | no | 0.9 MB |

A custom `.csv` is possible if we need more files later.
**Changing the layout erases the file system content.**

Check the layout on the board:

```bash
~/.platformio/penv/bin/python -m esptool --port /dev/ttyUSB0 read-flash 0x8000 0xC00 pt.bin
```

## Typical sizes

Measured (build output, `Flash:` line):

| Firmware | Flash | RAM (static) |
|---|---|---|
| Empty sketch | 257 KB | 22 KB |
| 02-button-pwm | 297 KB | 22 KB |
| Wi-Fi + WebServer + WebSockets + ArduinoJson + mDNS | ~1.0 MB | 51 KB |

Wi-Fi uses another ~50–70 KB of RAM at runtime.

Approximate library sizes (only used code is linked):

| Library | Size |
|---|---|
| ESP32Servo, AccelStepper | ~5 KB |
| Sensor drivers (BME280, VL53L0X, MPU6050) | 10–20 KB |
| NeoPixel / FastLED | 10–40 KB |
| OLED SSD1306 + GFX | 30–50 KB + fonts |
| TFT_eSPI | 50–150 KB |
| Bluetooth: NimBLE / Bluedroid | ~250 / ~500 KB |
