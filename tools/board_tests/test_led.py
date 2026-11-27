import time

import pytest

from conftest import error_code

MODES = ["off", "linear", "quadratic", "cubic", "blink"]


@pytest.fixture
def led(board):
    yield board
    board.request(cmd="set", device="led", data={"mode": "off"})


def test_describe(led):
    props = led.request(cmd="describe")["data"]["devices"]["led"]["props"]
    assert props["mode"]["values"] == MODES
    assert props["duty"] == {"min": 0, "max": 4095, "readonly": True}


@pytest.mark.parametrize("mode", MODES)
def test_set_mode(led, mode):
    reply = led.request(cmd="set", device="led", data={"mode": mode})
    assert reply["status"] == "ok"
    assert reply["device"] == "led"
    assert reply["data"]["mode"] == mode
    if mode == "off":
        assert reply["data"]["duty"] == 0


def test_duty_changes_while_breathing(led):
    led.request(cmd="set", device="led", data={"mode": "linear"})
    duties = set()
    for _ in range(5):
        duties.add(led.request(cmd="get", device="led")["data"]["duty"])
        time.sleep(0.2)
    assert len(duties) > 2


@pytest.mark.parametrize(
    "data, code, field",
    [
        ({"mode": "rainbow"}, "bad_value", "mode"),
        ({"mode": 3}, "bad_value", "mode"),
        ({"color": "red"}, "unknown_prop", "color"),
        ({"duty": 100}, "read_only", "duty"),
    ],
)
def test_set_error(led, data, code, field):
    reply = led.request(cmd="set", device="led", data=data)
    assert error_code(reply) == code
    assert reply["error"]["field"] == field


def test_all_or_nothing(led):
    led.request(cmd="set", device="led", data={"mode": "linear"})
    reply = led.request(cmd="set", device="led", data={"mode": "blink", "duty": 100})
    assert error_code(reply) == "read_only"
    assert led.request(cmd="get", device="led")["data"]["mode"] == "linear"


@pytest.mark.parametrize(
    "data, message", [(None, "data is required"), ("blink", "data must be an object")]
)
def test_set_bad_data(led, data, message):
    request = {"cmd": "set", "device": "led"}
    if data is not None:
        request["data"] = data
    reply = led.request(**request)
    assert error_code(reply) == "bad_request"
    assert reply["error"]["message"] == message
