# 03-wifi-web

The board connects to the home Wi-Fi and is controlled over a WebSocket API.
A simple web page served by the board is a test client of the same API.

Status: **done.**

## Goal

- Wi-Fi client with automatic reconnect, static IP, mDNS name
- Web server and WebSocket on the board
- A JSON protocol that can grow with new devices (motors, sensors): [docs/protocol.md](../../docs/protocol.md)
- Firmware split into layers with clear responsibilities
- Events: the board tells the client about changes on the board itself (button)
- Tests on the PC (no board) and against the running board (no upload)

## Wiring

Nothing to connect.

| Part | ESP32 pin |
|---|---|
| Blue LED (on board) | GPIO2 |
| BOOT button (on board) | GPIO0 |

## Build and run

1. Copy `src/secrets.example.h` to `src/secrets.h` and fill in the Wi-Fi name, password and a
   static IP of your network (`secrets.h` is git-ignored).
2. Build, upload, watch the log:

   ```bash
   pio run -d projects/03-wifi-web -t upload
   pio device monitor -d projects/03-wifi-web
   ```

3. Open `http://esp32.local/` (or the IP from `secrets.h`).
   If `esp32.local` does not resolve, see [docs/known-issues.md](../../docs/known-issues.md).

## Result

- The board connects in ~2.5 s, prints its IP and serves the page.
- The page builds its controls from `describe`: buttons for the LED modes, values of `duty` and
  of the BOOT button. It shows every message in a log with the round-trip time.
- LED modes switch from the page (`set`) and with the BOOT button (event to the page).
- One client at a time; a second one gets `busy`. A lost client is dropped in ~4 s,
  the page notices a lost board in ≤ 5 s and reconnects by itself.
- Firmware: ~1.07 MB (54 % of the app slot), static RAM ~52 KB, free heap with a client ~210 KB.

Log example:

```
2495 [wifi] INFO connected to <ssid>, IP 192.168.34.200, signal -32 dBm
2502 [http] INFO server started on port 80
2503 [ws] INFO server started on port 81
5221 [ws] INFO client connected from 192.168.34.2, free heap 212684
7303 [led] INFO pin 2 mode blink
8047 [protocol] WARN unknown_cmd: unknown command: foo, id 7
```

## Architecture

Layers. A lower layer knows nothing about the upper ones. Each layer checks what it is responsible for.

| Layer | Classes | Responsibility | Checks |
|---|---|---|---|
| Network | `WifiLink` | Wi-Fi, reconnect every 5 s, static IP, mDNS | – |
| Connection | `HttpServer`, `WsServer` | Page on port 80, WebSocket on port 81, one client, lost client detection | One client (`busy`), message size |
| Protocol | `Protocol` | JSON message → action → response or error; event text | JSON, `id`, command, field types |
| Devices | `Devices`, `Device`, `LedDevice`, `ButtonDevice` | Devices by name; JSON ↔ hardware calls | Device and property exist, read-only, value |
| Hardware | `Led`, `bot::Button` | PWM and modes, button input. No JSON | – |
| Behavior | `Controller` | Rules "when X happens, do Y" (BOOT pressed → next LED mode) | – |

- `Connectivity` holds the network and connection layers and starts the servers after Wi-Fi connects.
- `main.cpp` only creates the objects and connects them.
- Everything runs in `loop()`, one thread, no `delay()`. Each part has a short non-blocking `update()`.
  The only exception: the Wi-Fi disconnect reason comes from the Wi-Fi task (`std::atomic`).

### Request

Page → `WsServer` → `Protocol` (checks the request) → `Devices` (finds the device) →
`LedDevice` (checks the values) → `Led`. The response goes back the same way, with the same `id`.

### Event (BOOT pressed)

1. `Button` notifies its listeners: `ButtonDevice` and `Controller`.
2. `ButtonDevice` → `Devices` → `main.cpp` → `Protocol` (event text) → `WsServer`: event `boot`.
3. `Controller` sets the next LED mode → `Led` → `LedDevice` → `Devices` → … : event `led`.

Nobody below knows who listens. `Devices` does not forward a change made by the client's own `set`:
while `set` runs, it marks the device and drops its notifications. The client gets the response.

### Adding a device

1. A hardware class (for example `Motor`), without JSON.
2. A `Device` class for it (`MotorDevice`): `name`, `type`, `describe`, `get`, `set`;
   it calls `notifyChanged()` when the hardware reports a change.
3. Create both in `main.cpp`, add the device to `Devices`, start it in `beginHardware()`.

`Protocol`, `Devices` and the page do not change.

## Files

| File | What it does |
|---|---|
| `main.cpp` | Creates the objects, connects them, `setup()` and `loop()` |
| `config.h` | All constants: names, pins, PWM, timeouts, ports |
| `secrets.h` / `secrets.example.h` | Wi-Fi and static IP (the real file is git-ignored) |
| `Connectivity` | `WifiLink` + `HttpServer` + `WsServer` |
| `WifiLink` | Wi-Fi connection, reconnect, mDNS |
| `HttpServer` | Serves `index.html` (embedded at build time) |
| `WsServer` | WebSocket, one client, heartbeat, `busy` |
| `Protocol` | Requests, responses, errors, events |
| `ProtocolError.h` | Error codes and helpers (`unknownProp`, `readOnly`, `badValue`, …) |
| `Devices`, `Device.h` | Device list and the interface of a device |
| `LedDevice`, `ButtonDevice` | LED and BOOT button as protocol devices |
| `Led` | PWM, modes, mode change listeners |
| `Controller` | Behavior rules |
| `Log`, `Uptime.h`, `Names.h` | Log lines, board time, name comparison |
| `index.html` | Test page |

## Decisions

| Topic | Decision |
|---|---|
| Address | Static IP from `secrets.h` (`.200`, far from the DHCP range; no router access for a DHCP reservation). `esp32.local` via mDNS is unreliable in this network: use `/etc/hosts` (see `docs/known-issues.md`) |
| Wi-Fi | 2.4 GHz only (ESP32 limit). `WiFi.setSleep(false)`: fast replies (no 100–300 ms power-save delay) |
| Clients | One at a time; the second gets `busy` |
| Page | `index.html` is embedded into the firmware at build time (no file system), sent with `Cache-Control: no-cache`, so a new firmware always gives the new page |
| Libraries | Built-in `WebServer`, `WebSockets` (links2004, 2.7.3), `ArduinoJson` 7.4.3, `lib/Button`; versions pinned |
| Message size | 15 KB, set inside the WebSockets library (cannot be changed from outside). The library checks the size before reading and closes the connection (code 1009): the memory is safe |
| Heartbeat | The board pings every 2.5 s and drops a client after 2 missed pongs (~4 s). The pong timeout must be shorter than the ping interval (`static_assert`), otherwise the library never drops a client |
| Page heartbeat | The page sends `ping` every 2.5 s and reconnects after 5 s of silence: a browser does not notice a board reset by itself |
| Logs | `<board ms> [layer] LEVEL text`, only state changes and errors. The PC adds date and time when reading |
| Portable code | `Protocol` and `Devices` do not use Arduino: they build on the PC for unit tests. Use `PRIu32` for `uint32_t` in `printf` (`%lu` is right only on the ESP32) |

## Tests

**Unit tests on the PC** (no board, not part of the firmware). They build `Protocol` and `Devices`
with a fake device and check commands, every error, all-or-nothing `set` and events:

```bash
pio test -d projects/03-wifi-web -e native
```

**Board tests** talk to the running board over the network (the firmware is not uploaded again):
protocol, error codes, LED, events, one client, oversized message, silent client.

```bash
python3 -m pytest tools/board_tests              # board at esp32.local
python3 -m pytest tools/board_tests --board 192.168.34.200
```

- Needs `pytest` and `websockets` for Python (`sudo apt install python3-pytest python3-websockets`).
- **Close the web page first:** the board accepts one client, the page would make the tests get `busy`.

**Manual checks** (cannot be automated):

- Press BOOT with the page open: three events (`boot` pressed, `led` mode, `boot` released),
  the LED and the page show the next mode.
- Press EN: the page shows `no connection`, reconnects and logs `board restarted`.
- Turn the router off and on: the board reconnects with the same IP (not checked yet).

## Security

- No password: anyone in the home network can control the board.
- A WebSocket has no same-origin protection: **any web site open in a browser on this network can
  connect to `ws://<board>:81/`**. Fine for an LED; must be fixed before motors
  (for example, check the `Origin` header and a token).

## Known limits and TODO

- The WebSocket port is written twice: `config::kWsPort` and `kWsPort` in `index.html` (`TODO` in both).
- Free heap in the log is for debugging only (`TODO` in `WsServer.cpp`).
- Device names are not checked for duplicates (the first one wins).
- If a future `Controller` rule changes another device during a `set`, its event is sent before the response.
- `HEAD /` gets 404 (only `GET` is registered).

## Later

- Fallback access point when the home Wi-Fi is not available
- Firmware update over Wi-Fi (OTA)
- Several sensor samples in one message
- Access protection (see Security)
