from conftest import error_code


def test_boot_button_device(board):
    devices = board.request(cmd="describe")["data"]["devices"]
    assert devices["boot"] == {"type": "button", "props": {"pressed": {"readonly": True}}}
    assert board.request(cmd="get", device="boot")["data"] == {"pressed": False}


def test_boot_button_is_read_only(board):
    reply = board.request(cmd="set", device="boot", data={"pressed": True})
    assert error_code(reply) == "read_only"


def test_own_set_gives_response_without_event(board):
    reply = board.request(cmd="set", device="led", data={"mode": "blink"})
    assert reply["status"] == "ok"
    assert board.receive_nothing(timeout=0.8) is None
    board.request(cmd="set", device="led", data={"mode": "off"})
