# Idea: Raspberry Pi with ROS 2 + ESP32

Status: **idea, not decided.** Come back to it when we know more.

## Goal

- Raspberry Pi 4 with Linux and ROS 2: the robot logic (map, navigation, camera, path planning).
- ESP32: reads sensors, sends the data, receives commands, drives the motors.

## Split of work

Fast and critical loops run on the ESP32, slow logic on the Raspberry Pi.
Linux does not guarantee reaction time: it can pause for tens of milliseconds.

| Where | What | Typical rate |
|---|---|---|
| ESP32 | IMU (1000 Hz checked), motor feedback, PWM, encoders, emergency stop | 200–1000 Hz |
| Raspberry Pi + ROS 2 | Map, navigation, camera, planning; commands like "speed X, turn Y" | 10–50 Hz |

- Safety: the ESP32 stops the motors by itself when no command came for ~200–300 ms.
- Quadcopter: flight stabilization only on the ESP32 (or a flight controller) at 500–1000 Hz.
  The Raspberry Pi gives only high-level commands.

## Protocol options

1. **micro-ROS** — the ESP32 becomes a normal ROS 2 node (USB/UART or Wi-Fi UDP).
   Heavy for the ESP32, harder build. Support for Arduino-ESP32 3.x on pioarduino is **not confirmed**:
   check it in practice ([micro_ros_platformio](https://github.com/micro-ROS/micro_ros_platformio)).
2. **Own compact binary protocol over USB/UART** + a small ROS 2 node in Python on the Raspberry Pi
   that translates it to ROS messages. Simpler, fully under our control.

Current preference: option 2 to start, check micro-ROS later.
The JSON over WebSocket protocol stays for the web page and debugging, not for real-time control.

## Link

| Link | Latency | Stability | Rate |
|---|---|---|---|
| USB/UART (921600 baud, ~90 KB/s) | 1–2 ms | Stable | 500–1000 Hz small messages |
| Wi-Fi | 2–20 ms, drops to 100+ ms | Jumps | 50–200 Hz |

Raspberry Pi on the robot → USB cable to the ESP32. Wi-Fi for watching from the PC.

## Hardware notes

- Raspberry Pi 4: own good 5 V 3 A supply. Motors: separate supply, common ground.
- Usual software: Ubuntu 24.04 + ROS 2 Jazzy (long-term support).

## Open questions

- Does micro-ROS work with our platform version?
- Message format of our own protocol (binary, checksum, message types).
- Real latency and rate on our hardware: measure.
