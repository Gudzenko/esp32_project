import argparse
import glob
import sys
import time

import serial

BITS_PER_BYTE = 10
JUNK_BYTES = b"\x00\x80"
BOOT_LOG_PREFIXES = ("ets ", "rst:", "configsip:", "clk_drv:", "mode:DIO", "mode:QIO", "load:0x",
                     "ho 0 tail", "entry 0x")


def parse_args():
    parser = argparse.ArgumentParser(description="Reset an ESP32 board and read its serial output.")
    parser.add_argument("--port", help="serial port (default: first /dev/ttyUSB* or /dev/ttyACM*)")
    parser.add_argument("--baud", type=int, default=115200)
    parser.add_argument("--seconds", type=float, default=5.0, help="how long to read after reset")
    parser.add_argument("--no-reset", action="store_true",
                        help="no extra RTS reset (on Linux opening the port still resets the board)")
    parser.add_argument("--no-boot-log", action="store_true",
                        help="hide the ROM bootloader lines (ets, rst:, load:, ...)")
    return parser.parse_args()


def hide_boot_log(text):
    lines = text.split("\n")
    kept = [line for line in lines if not line.startswith(BOOT_LOG_PREFIXES)]
    return "\n".join(kept).lstrip("\n")


def find_port():
    ports = sorted(glob.glob("/dev/ttyUSB*") + glob.glob("/dev/ttyACM*"))
    if not ports:
        sys.exit("No serial port found (/dev/ttyUSB*, /dev/ttyACM*)")
    return ports[0]


def open_port(port, baud):
    ser = serial.Serial()
    ser.port = port
    ser.baudrate = baud
    ser.timeout = 0.05
    ser.dtr = False
    ser.rts = False
    ser.open()
    return ser


def drain(ser, quiet_s=0.3, max_s=3.0):
    # Old data can be replayed many times by the USB-UART driver after open (docs/known-issues.md).
    dropped = 0
    start = last_data = time.monotonic()
    while time.monotonic() - start < max_s:
        data = ser.read(4096)
        now = time.monotonic()
        if data:
            dropped += len(data)
            last_data = now
        elif now - last_data >= quiet_s:
            break
    ser.reset_input_buffer()
    return dropped


def reset_board(ser):
    ser.rts = True
    time.sleep(0.1)
    ser.rts = False


def read_for(ser, seconds):
    received = bytearray()
    end = time.monotonic() + seconds
    while time.monotonic() < end:
        received += ser.read(4096)
    return bytes(received)


def main():
    args = parse_args()
    port = args.port or find_port()
    ser = open_port(port, args.baud)
    try:
        dropped = drain(ser)
        if not args.no_reset:
            reset_board(ser)
        data = read_for(ser, args.seconds)
    finally:
        ser.close()

    max_bytes = args.baud / BITS_PER_BYTE * args.seconds
    junk = sum(data.count(bytes([b])) for b in JUNK_BYTES)
    clean = bytes(b for b in data if b not in JUNK_BYTES)

    print(f"--- {port} @ {args.baud}, read {args.seconds:g} s, {len(data)} bytes", file=sys.stderr)
    if dropped:
        print(f"--- dropped {dropped} stale bytes before reading", file=sys.stderr)
    if junk:
        print(f"--- removed {junk} junk bytes (0x00/0x80)", file=sys.stderr)
    if len(data) > max_bytes * 1.1:
        print(f"--- WARNING: more data than {args.baud} baud can carry: replayed data", file=sys.stderr)
    text = clean.decode("utf-8", "replace").replace("\r\n", "\n")
    if args.no_boot_log:
        text = hide_boot_log(text)
    sys.stdout.write(text)


if __name__ == "__main__":
    main()
