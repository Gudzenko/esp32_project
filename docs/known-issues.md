# Known issues

## Garbage and repeated lines when the serial port is opened

**Setup:** ESP32 DevKit with CP2102, Linux kernel 7.0, driver `cp210x`.

**Symptoms** (not every time, often after upload or after a pause):

- Hundreds or thousands of copies of the same line, all with the same timestamp:
  `Blink #1`, `Blink #1`, ...
- Invisible characters right after upload: bytes `\x00\x80\x00` repeated many times.
- The first real messages (boot log, `started`) can be lost.

**What we found:**

- The board sends correct data. A few `\x00\x80` bytes on reset are normal electrical noise.
- The host receives far more data than 115200 baud can carry (e.g. 154 KB in 4 s, max ~46 KB),
  so the last received data is replayed on the PC side (driver or USB-UART adapter).
- Not caused by PlatformIO monitor filters (same with plain pyserial), not by USB autosuspend,
  not by the USB port (same on another port).
- Possible causes: a CP2102 clone chip on the board, or the `cp210x` driver in kernel 7.0.
  To check: try the second board.

**Workaround:**

- Upload and monitor as separate commands:
  `pio run -d projects/NN-name -t upload`, then `pio device monitor -d projects/NN-name`.
- If there is garbage at the start, press **EN** on the board to restart it and get clean output.
- The `esp32-build-flash` skill reads the port with a script that drops the garbage first.
- Firmware upload is not affected (esptool verifies the hash of written data).

## `esp32.local` does not resolve

**Symptom:** the board works by IP, but `esp32.local` fails in the browser (`DNS_PROBE_FINISHED_NXDOMAIN`)
and in `ping`. Sometimes it works for a short time after the board starts.

**Findings:** the board's mDNS responder works. A query that asks for a direct (unicast) reply is
answered every time. The normal multicast replies do not reach the PC: the home router (HALNy)
drops them, on 2.4 GHz and 5 GHz. No access to the router settings.

**Workaround:** static IP on the board (`secrets.h`, an address far from the DHCP range,
e.g. `.200`) and a line in `/etc/hosts` on the PC:

```bash
echo "192.168.34.200 esp32 esp32.local" | sudo tee -a /etc/hosts
```
