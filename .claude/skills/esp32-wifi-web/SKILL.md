---
name: esp32-wifi-web
description: Wi-Fi, web page and WebSocket API on an ESP32 with Arduino-ESP32 3.x - proven setup, library pitfalls (WebSockets heartbeat, message size), the JSON protocol, browser-side reconnect, network tests with pytest. Use when a project connects the board to Wi-Fi, serves a page, adds a WebSocket/HTTP API, changes the protocol, or when the board is not reachable over the network.
---

# ESP32 Wi-Fi, web page and WebSocket API

Reference implementation: `projects/03-wifi-web` (README: architecture, decisions, tests).
Protocol: `docs/protocol.md`. Change both together.

## Wi-Fi

- ESP32 sees **2.4 GHz only**. A 5 GHz-only network is invisible. Check: `nmcli -f SSID,CHAN,FREQ dev wifi`.
- `WiFi.setSleep(false)`: without it modem sleep adds 100–300 ms latency and loses multicast (mDNS).
- `WiFi.setHostname()` **before** `WiFi.mode()`/`WiFi.begin()`.
- Own reconnect instead of the built-in one: `setAutoReconnect(false)`, in `update()` retry every
  N s if `WiFi.status() != WL_CONNECTED` (`disconnect()` + `begin()`). Predictable and visible in the log.
- The disconnect reason exists only in the event callback, which runs in **another task**:
  store it in a `std::atomic<uint8_t>` and read it in `loop()`.
  `WiFi.disconnectReasonName(reason)` gives the text (`4WAY_HANDSHAKE_TIMEOUT` = wrong password,
  `NO_AP_FOUND` = no network). Ignore `WIFI_REASON_ASSOC_LEAVE` (our own `disconnect()`).
- **Static IP** (`WiFi.config(ip, gateway, subnet, dns)` before `begin`) when there is no router
  access for a DHCP reservation: take an address far from the DHCP range (`.200`), check it is free
  with `ping`. `disconnect()`/`begin()` keep the static config.
- **mDNS** (`MDNS.begin(name)` after the first connect) may not work: some routers drop multicast
  replies. Test: a unicast query is answered, a normal one is not → router. Workaround: static IP
  and a line in `/etc/hosts` (`docs/known-issues.md`).
- Credentials and the static IP go to the git-ignored `secrets.h` (`secrets.example.h` is committed).
  The password can be taken from the PC: `nmcli -s -g 802-11-wireless-security.psk connection show <name>`
  — write it to the file, **never print it**.

## Web page

- Built-in `WebServer` (synchronous, in `loop()`): one thread, no locks. Avoid `AsyncWebServer`
  unless needed: its handlers run in another task.
- Keep the page as a normal file and embed it at build time:

  ```ini
  board_build.embed_txtfiles = src/index.html
  ```

  ```cpp
  extern const char kIndexHtml[] asm("_binary_src_index_html_start");
  ```

  (null-terminated, no file system needed).
- Send it with `server.sendHeader("Cache-Control", "no-cache")`, otherwise the browser shows the
  old page after a new firmware.
- `<link rel="icon" href="data:,">` stops the `/favicon.ico` request (no 404 in the log).
- Only registered methods are served: `HEAD /` gets 404 when only `HTTP_GET` is registered.

## WebSocket (links2004/WebSockets 2.7.3)

- Runs on **its own port** (81), not on the page port. The page connects to `ws://<host>:81/`.
- Events come from `webSocket.loop()` inside our `loop()` (same thread).
- **Heartbeat:** `enableHeartbeat(pingMs, pongTimeoutMs, missedCount)`.
  **`pongTimeoutMs` must be smaller than `pingMs`**, otherwise a new ping resets the timer and a lost
  client is **never** dropped. Guard it: `static_assert(kPongTimeoutMs < kPingIntervalMs, ...)`.
  2500 / 2000 / 2 drops a silent client in ~4 s.
- **Message size limit is fixed in the library** (`WEBSOCKETS_MAX_DATA_SIZE`, 15 KB on ESP32,
  `#define` without `#ifndef`). It is checked before reading: a bigger message closes the
  connection with code 1009, memory is safe.
- One client: on `WStype_CONNECTED` while another is active, send a `busy` error and `disconnect()`.
- Answer `WStype_BIN` too (some clients send binary); log `WStype_ERROR`.
- **No same-origin protection:** any web site in a browser on the same network can open the socket.
  Before motors: check the `Origin` header and a token.

## Browser side

- A browser **does not notice** a board reset or power loss: the TCP connection just stays silent.
  The page sends an application `ping` every 2.5 s and reconnects after 5 s without any message
  (also covers a hanging connect). After reconnect: `describe` + `get` again.
- Board restart detection: `time` in replies goes down.
- Close the old socket without its `onclose` handler before reconnecting, so the reconnect runs once.

## Protocol

JSON objects, rules in `docs/protocol.md`: `id` + `cmd` → `id` + `time` + `status` (+ `device`, `data`
or `error {code, field, message}`); events `{"event": "changed", "time", "device", "data"}` without `id`.
Each layer checks what it owns: connection (one client, size), protocol (shape: `bad_json`,
`bad_request`, `missing_id`, `unknown_cmd`), device (`unknown_device`, `unknown_prop`, `read_only`,
`bad_value`). `set` is all-or-nothing. Architecture of devices and events: skill `esp32-arduino-patterns`.

## Tests over the network

`tools/board_tests` (pytest + `websockets`, no upload):

```bash
python3 -m pytest tools/board_tests --board 192.168.34.200
```

- **Close the web page first** — the board accepts one client, tests would get `busy`.
- Each test opens its own connection (fixture `board`) and restores what it changed.
- websockets 10 (Ubuntu) has no sync client: `conftest.py` runs the async one in its own event loop;
  create the connection **inside** that loop (`await websockets.connect(...)` in a coroutine).
- A raw TCP socket that never answers pings tests the "silent client" drop.
- To read the board log during a network test, see skill `esp32-build-flash` (never upload and
  read the port at the same time).

## Not reachable — checklist

1. Log: `connected ... IP ...`? If not: SSID 2.4 GHz, password (`4WAY_HANDSHAKE_TIMEOUT`).
2. `ping <IP>`. Works but the name does not → mDNS (see above).
3. `curl -s -o /dev/null -w "%{http_code}" http://<IP>/` → 200?
4. WebSocket `busy` → another tab/test/phone holds the only slot; it frees in ~4 s after it is gone.
