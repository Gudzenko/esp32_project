# Development environment setup (Linux)

Tested on Ubuntu with an ESP32 DevKit (ESP-WROOM-32, CP2102).

## 1. USB driver

Nothing to install: the CP2102 driver (`cp210x`) is built into the Linux kernel.
Connect the board with a **data** USB cable and check:

```bash
lsusb | grep CP210      # Silicon Labs CP210x UART Bridge (10c4:ea60)
ls -l /dev/ttyUSB*      # /dev/ttyUSB0
```

If the port appears and then disappears, remove `brltty`: `sudo apt remove brltty`.

## 2. Serial port access

```bash
sudo usermod -aG dialout $USER
```

Log out and log in again (or reboot). Check: `id -nG` must show `dialout`.

## 3. VS Code + PlatformIO

```bash
code --install-extension platformio.platformio-ide
```

Open VS Code and wait until PlatformIO Core is installed (notifications in the bottom right),
then reload the window. Make `pio` available in any terminal:

```bash
ln -sf ~/.platformio/penv/bin/pio ~/.local/bin/pio
pio --version
```

## 4. PlatformIO udev rules

Install the rules file that comes with the installed PlatformIO Core, so the versions match
(otherwise `pio` prints "udev rules are outdated"):

```bash
sudo cp ~/.platformio/penv/lib/python3*/site-packages/platformio/assets/system/99-platformio-udev.rules \
  /etc/udev/rules.d/
sudo udevadm control --reload-rules
sudo udevadm trigger
```

Repeat this step after a PlatformIO Core update if the warning appears again.

## 5. ESP32 platform (pioarduino)

The official PlatformIO `espressif32` platform supports only Arduino-ESP32 2.x.
We use the community fork **pioarduino** (Arduino-ESP32 3.x), with a fixed release version:

```bash
pio pkg install -g -p https://github.com/pioarduino/platform-espressif32/releases/download/55.03.313/platform-espressif32.zip
```

- Release `55.03.313` = Arduino-ESP32 3.3.13 + ESP-IDF 5.5.
- The compiler, framework and esptool are downloaded on the first build (about 1 GB, a few minutes).
- Releases: https://github.com/pioarduino/platform-espressif32/releases

## 6. Check

```bash
pio device list                                                   # /dev/ttyUSB0, CP2102
~/.platformio/penv/bin/python -m esptool --port /dev/ttyUSB0 flash-id
```

Expected: `Chip type: ESP32-D0WD-V3`, `Detected flash size: 4MB`.
This only reads info from the chip and does not change anything on the board.
