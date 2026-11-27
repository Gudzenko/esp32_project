# Protocol

How clients (web page, Python tools, mobile app) talk to the board.

- Transport: WebSocket, one JSON object per message.
- Only **one client** at a time. A second client gets the `busy` error and is disconnected.
- The board validates every request and always answers.

## Message

| Field | In | Meaning |
|---|---|---|
| `id` | request, response | Request number set by the client (1, 2, 3, ...). The response has the same `id`. |
| `cmd` | request | Command: `describe`, `get`, `set`, `ping` |
| `status` | response | `ok` or `error` |
| `event` | event | What happened on the board: `changed` |
| `time` | response, event | Milliseconds since the board started (64-bit) |
| `device` | all | Device id, e.g. `led`, `motor_left` |
| `data` | all | Properties and values: `{"mode": "blink"}` |
| `error` | error response | `{"code", "field", "message"}` |

A message from the board is:
- an **event** if it has `event` (no `id`);
- a **response** if it has `id`;
- a response to an unreadable request if it has neither (`bad_json`).

## Commands

### set

```json
→ {"id": 7, "cmd": "set", "device": "led", "data": {"mode": "blink"}}
← {"id": 7, "time": 152340, "status": "ok", "device": "led", "data": {"mode": "blink"}}
```

Several properties can be set at once. **All or nothing:** if one value is wrong, nothing changes.

### get

```json
→ {"id": 8, "cmd": "get", "device": "led"}
← {"id": 8, "time": 152400, "status": "ok", "device": "led", "data": {"mode": "blink"}}

→ {"id": 9, "cmd": "get"}
← {"id": 9, "time": 152410, "status": "ok", "data": {"led": {"mode": "blink"}}}
```

Without `device`, the board returns the state of all devices.

### describe

What the board has. Clients call it after connecting.

```json
→ {"id": 1, "cmd": "describe"}
← {"id": 1, "time": 1200, "status": "ok", "data": {
     "board": "esp32-01", "firmware": "0.1.0", "protocol": 1,
     "devices": {
       "led": {"type": "led", "props": {"mode": {"values": ["off", "linear", "quadratic", "cubic", "blink"]}}},
       "motor_left": {"type": "motor", "props": {"speed": {"min": -100, "max": 100}, "current": {"readonly": true}}}}}}
```

Property description: `values` (list of allowed values), `min`/`max` (number range), `readonly`.

### ping

Checks that the connection is alive.

```json
→ {"id": 12, "cmd": "ping"}
← {"id": 12, "time": 15230, "status": "ok"}
```

The client sends it every 2.5 s. If nothing comes from the board for 5 s, the connection is lost:
the client closes it and connects again. Without it, the client does not notice a board reset or
power loss: the connection just stays silent.

## Events

The board sends an event when a value changes on the board itself (button, sensor, failsafe).
The client that changed a value with `set` gets the response, not an event.

```json
← {"event": "changed", "time": 152871, "device": "led", "data": {"mode": "cubic"}}
```

## Errors

```json
← {"id": 8, "time": 152500, "status": "error", "device": "led",
   "error": {"code": "bad_value", "field": "mode", "message": "allowed: off, linear, quadratic, cubic, blink"}}
```

| Code | When |
|---|---|
| `bad_json` | The message is not valid JSON |
| `bad_request` | Valid JSON, but not a request: not an object, `id` not a non-negative integer, `cmd` or `device` not a string, a required field is missing |
| `missing_id` | The request has no `id` |
| `unknown_cmd` | No such command |
| `unknown_device` | No such device |
| `unknown_prop` | The device has no such property |
| `read_only` | The property cannot be changed |
| `bad_value` | Wrong type or out of range |
| `busy` | Another client is connected |

## Rules

- `time` is when the value was obtained (for a sensor: when it was measured), not when it was sent.
- `time` smaller than before means the board has restarted: call `describe` and `get` again.
- New devices, properties and commands do not change `protocol`. Changing the meaning of
  existing fields does.

## Later

- Several sensor samples in one message (when we add sensors).
