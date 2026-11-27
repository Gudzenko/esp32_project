import socket
import time

import pytest
import websockets

from conftest import BoardClient, WS_PORT, error_code

OVERSIZED_MESSAGE_BYTES = 16 * 1024
MESSAGE_TOO_BIG = 1009
MAX_SILENT_CLIENT_SECONDS = 7


def test_second_client_is_busy(board, board_url):
    second = BoardClient(board_url)
    try:
        assert error_code(second.receive()) == "busy"
    finally:
        second.close()
    assert board.request(cmd="ping")["status"] == "ok"


def test_oversized_message_closes_connection(board):
    board.send_raw("x" * OVERSIZED_MESSAGE_BYTES)
    with pytest.raises(websockets.ConnectionClosed) as closed:
        board.receive(timeout=3)
    assert closed.value.code == MESSAGE_TOO_BIG


def test_silent_client_is_dropped(request):
    host = request.config.getoption("--board")
    with socket.create_connection((host, WS_PORT), timeout=MAX_SILENT_CLIENT_SECONDS + 3) as sock:
        sock.sendall(
            b"GET / HTTP/1.1\r\nHost: esp32\r\nUpgrade: websocket\r\nConnection: Upgrade\r\n"
            b"Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\nSec-WebSocket-Version: 13\r\n\r\n"
        )
        sock.recv(1024)
        started = time.monotonic()
        while sock.recv(1024):
            pass
        assert time.monotonic() - started < MAX_SILENT_CLIENT_SECONDS
