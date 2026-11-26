# 01-blink

Blink the on-board LED and print status to Serial without blocking.

## Goal

- Program structure: `setup()` and `loop()`
- GPIO output: `pinMode`, `digitalWrite`
- Serial output: `Serial.begin`, `Serial.printf`
- Non-blocking timing with `millis()` instead of `delay()`

## Wiring

Nothing to connect. The blue on-board LED is on GPIO2.

## Build and run

```bash
pio run -d projects/01-blink -t upload
pio device monitor -d projects/01-blink
```

## Result

- The blue LED blinks once per second.
- Serial output every 2 seconds:

```
01-blink started
uptime 2000 ms | blinks 2 | loop() runs ... times/s
```

`loop()` runs hundreds of thousands of times per second: the LED and the report are two
independent tasks, and the CPU is free between them. With `delay()` it would run once per second.

## Lessons

- `delay()` blocks the whole program. Use `if (now - last >= interval)` with `millis()`.
- Write the check as `now - last`, not `now >= last + interval`: it stays correct when
  `millis()` overflows (after ~49.7 days).
