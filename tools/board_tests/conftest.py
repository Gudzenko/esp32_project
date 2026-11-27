import asyncio
import json

import pytest
import websockets

WS_PORT = 81


def pytest_addoption(parser):
    parser.addoption("--board", default="esp32.local", help="board host name or IP")


class BoardClient:
    def __init__(self, url):
        self._loop = asyncio.new_event_loop()
        self._socket = self._run(self._connect(url))
        self._next_id = 0

    @staticmethod
    async def _connect(url):
        return await websockets.connect(url)

    def _run(self, coroutine):
        return self._loop.run_until_complete(coroutine)

    def send_raw(self, data):
        self._run(self._socket.send(data))

    def receive(self, timeout=2.0):
        return json.loads(self._run(asyncio.wait_for(self._socket.recv(), timeout)))

    def receive_nothing(self, timeout):
        try:
            message = self._run(asyncio.wait_for(self._socket.recv(), timeout))
        except asyncio.TimeoutError:
            return None
        return message

    def request(self, **message):
        self._next_id += 1
        self.send_raw(json.dumps({"id": self._next_id, **message}))
        return self.receive()

    def close(self):
        self._run(self._socket.close())
        self._loop.close()


@pytest.fixture
def board_url(request):
    return f"ws://{request.config.getoption('--board')}:{WS_PORT}/"


@pytest.fixture
def board(board_url):
    client = BoardClient(board_url)
    yield client
    client.close()


def error_code(reply):
    return reply.get("error", {}).get("code")
