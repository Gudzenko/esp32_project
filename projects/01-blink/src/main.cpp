#include <Arduino.h>

constexpr uint8_t kLedPin = 2;
constexpr uint32_t kSerialBaud = 115200;
constexpr uint32_t kBlinkIntervalMs = 500;
constexpr uint32_t kReportIntervalMs = 2000;

bool ledOn = false;
uint32_t blinkCount = 0;
uint32_t loopCount = 0;
uint32_t lastToggleMs = 0;
uint32_t lastReportMs = 0;

void updateLed(uint32_t now) {
    if (now - lastToggleMs < kBlinkIntervalMs) {
        return;
    }
    lastToggleMs = now;
    ledOn = !ledOn;
    digitalWrite(kLedPin, ledOn ? HIGH : LOW);
    if (ledOn) {
        ++blinkCount;
    }
}

void updateReport(uint32_t now) {
    if (now - lastReportMs < kReportIntervalMs) {
        return;
    }
    const uint32_t loopsPerSecond = loopCount * 1000 / (now - lastReportMs);
    Serial.printf("uptime %lu ms | blinks %lu | loop() runs %lu times/s\n", now, blinkCount,
                  loopsPerSecond);
    lastReportMs = now;
    loopCount = 0;
}

void setup() {
    Serial.begin(kSerialBaud);
    pinMode(kLedPin, OUTPUT);
    Serial.println("01-blink started");
}

void loop() {
    const uint32_t now = millis();
    ++loopCount;
    updateLed(now);
    updateReport(now);
}
