# ESP32 learning project

Learning the ESP32 step by step: LEDs, buttons, sensors, wireless, motors – and later a mobile robot.

- Board: ESP32 DevKit (ESP-WROOM-32, CP2102)
- Language: C++ (Arduino framework)
- Tools: VS Code + PlatformIO

## Setup (Linux)

1. Add your user to the `dialout` group, then log out and log in:
   `sudo usermod -aG dialout $USER`
2. Install PlatformIO udev rules:
   https://docs.platformio.org/en/latest/core/installation/udev-rules.html
3. Install the **PlatformIO IDE** extension in VS Code.
