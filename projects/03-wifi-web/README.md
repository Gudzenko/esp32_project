# 03-wifi-web

The board connects to the home Wi-Fi and is controlled over a WebSocket API.
A simple web page served by the board is a test client of the same API.

Status: **design, not implemented yet.**

## Goal

- Wi-Fi client with automatic reconnect, mDNS name
- Web server and WebSocket on the board
- A JSON protocol that can grow with new devices (motors, sensors): [docs/protocol.md](../../docs/protocol.md)
- Firmware split into layers with clear responsibilities

## Architecture

Four layers. A lower layer knows nothing about the upper ones.

| Layer | Responsibility | Checks |
|---|---|---|
| Network | Connect to Wi-Fi, reconnect every 5 s, host name, mDNS, print IP | – |
| Connection | Serve the page, WebSocket with one client, detect a lost client (5 s) | Only one client (`busy`), message size |
| Protocol | JSON message → action → response or error | JSON, `id`, command, device, property |
| Logic | What the board does (now: the LED) | Value meaning (allowed mode, range) |

- Everything runs in `loop()`, one thread, no `delay()`. Each layer has a short non-blocking `update()`.
- Two directions: request → response, and events from the board (button) → client, if connected.
- Board states: starting → connecting to Wi-Fi → online, no client → client connected.
  The logic works in all states. When Wi-Fi or the client is lost, the board steps back and waits.
- Future motors: stop when the client is lost.

## Decisions

| Topic | Decision |
|---|---|
| Address | Static IP from `secrets.h` (`.200`, far from the DHCP range; no router access for a DHCP reservation). `esp32.local` via mDNS is unreliable in this network: use `/etc/hosts` (see `docs/known-issues.md`) |
| Wi-Fi | 2.4 GHz only (ESP32 limit). `WiFi.setSleep(false)`: fast replies (no 100–300 ms power-save delay) |
| Clients | One at a time; the second gets `busy`. No password: anyone in the home network can connect (protection later) |
| Page | Simple: connection state, LED buttons, message log. `index.html` is embedded into the firmware at build time (no file system). Reconnects every 5 s, then asks `describe` and `get` again |
| Libraries | Built-in `WebServer`, `WebSockets` (links2004), `ArduinoJson` 7, versions pinned |
| Constants | All in `src/config.h` (board name, firmware version, pins, timeouts, limits) |
| Secrets | Wi-Fi name and password in `src/secrets.h` (git-ignored). `src/secrets.example.h` is committed |
| Message size | Generous limit (~8 KB): protects the memory, never reached by normal messages |
| Logs | `<board ms> [layer] LEVEL text`, only state changes and errors. The PC adds date and time when reading |
| Free memory | Printed on client connect/disconnect, for debugging only (remove later) |

## Plan

Each step is built, uploaded and checked on the board before the next one.
Details are decided during the work; this plan and the README are updated as we go.

1. **Wi-Fi.** `config.h`, `secrets.h`, a small log function, the network layer.
   The LED shows the Wi-Fi state: blinks while connecting, on when connected (until step 5).
   Check: IP and `esp32.local` in the log; `ping esp32.local` works;
   a wrong password gives an error and retries every 5 s; the LED keeps blinking meanwhile
   (waiting for Wi-Fi blocks nothing).
2. **Web page.** Web server, `index.html` embedded into the firmware.
   Check: the page opens at `http://esp32.local/` and by IP.
3. **WebSocket echo.** One client, lost client detection. The page shows the connection state
   and a log; the board sends every text back.
   Check: echo works; a second tab gets `busy`; after closing the tab the slot is free within 5 s;
   after a board reset the page reconnects by itself.
4. **Protocol.** JSON parsing, `id`, commands, error responses, `time`.
   Check: `describe`/`get` answer; wrong messages give the right error codes
   (sent from the browser console, DevTools).
5. **LED over the API.** LED modes from project 02, `get`/`set`. The page shows the mode and buttons.
   Check: buttons switch the mode; wrong values give `bad_value`.
6. **Events.** The BOOT button switches the mode, the page gets `changed`.
   Check: pressing BOOT updates the page.

After the last step: result in this README, project added to the root README,
review what goes to `lib/`, skills or docs.

## Later

- Fallback access point when the home Wi-Fi is not available
- Firmware update over Wi-Fi (OTA)
- Several sensor samples in one message
- Tests on the PC (`pio test -e native`), not part of the firmware
- Python tool for the API
- Access protection
