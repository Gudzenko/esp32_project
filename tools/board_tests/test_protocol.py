import json

import pytest

from conftest import error_code


@pytest.mark.parametrize("command", ["ping", "describe", "get"])
def test_command_ok(board, command):
    reply = board.request(cmd=command)
    assert reply["status"] == "ok"
    assert reply["id"] == 1
    assert "time" in reply


def test_describe(board):
    data = board.request(cmd="describe")["data"]
    assert data["board"] == "esp32"
    assert data["protocol"] == 1
    assert set(data["devices"]) == {"led", "boot"}


@pytest.mark.parametrize(
    "raw, code",
    [
        ("not json", "bad_json"),
        ('{"id": 4, "cmd": "ping"', "bad_json"),
        ("[1, 2]", "bad_request"),
        ('"text"', "bad_request"),
        ('{"cmd": "ping"}', "missing_id"),
        ('{"id": -1, "cmd": "ping"}', "bad_request"),
        ('{"id": 1.5, "cmd": "ping"}', "bad_request"),
        ('{"id": "7", "cmd": "ping"}', "bad_request"),
        ('{"id": 5}', "bad_request"),
        ('{"id": 6, "cmd": 42}', "bad_request"),
        ('{"id": 7, "cmd": "foo"}', "unknown_cmd"),
        ('{"id": 8, "cmd": "get", "device": "motor"}', "unknown_device"),
        ('{"id": 9, "cmd": "get", "device": 3}', "bad_request"),
        ('{"id": 10, "cmd": "set", "device": "motor", "data": {"speed": 10}}', "unknown_device"),
        ('{"id": 11, "cmd": "set"}', "bad_request"),
    ],
)
def test_error(board, raw, code):
    board.send_raw(raw)
    reply = board.receive()
    assert reply["status"] == "error"
    assert error_code(reply) == code
    assert "time" in reply
    request = json.loads(raw) if raw.startswith("{") and raw.endswith("}") else {}
    valid_id = isinstance(request.get("id"), int) and request["id"] >= 0
    assert reply.get("id") == (request["id"] if valid_id else None)


def test_binary_message_is_handled(board):
    board.send_raw(b'{"id": 20, "cmd": "ping"}')
    assert board.receive()["status"] == "ok"
    board.send_raw(bytes([0xFF, 0x00, 0x13]))
    assert error_code(board.receive()) == "bad_json"
