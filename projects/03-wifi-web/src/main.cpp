#include <Arduino.h>

#include "Log.h"
#include "WifiLink.h"
#include "config.h"

#if __has_include("secrets.h")
#include "secrets.h"
#else
#error "Copy src/secrets.example.h to src/secrets.h and fill in the Wi-Fi name and password"
#endif

WifiLink wifi(secrets::kWifiSsid, secrets::kWifiPassword, config::kBoardName,
              config::kWifiRetryIntervalMs);

bool ledOn = false;

void setLed(bool on) {
    if (on == ledOn) {
        return;
    }
    ledOn = on;
    digitalWrite(config::kLedPin, on ? HIGH : LOW);
}

void updateStatusLed(uint32_t now) {
    if (wifi.isConnected()) {
        setLed(true);
        return;
    }
    setLed((now / config::kLedBlinkIntervalMs) % 2 == 0);
}

void setup() {
    Serial.begin(config::kSerialBaud);
    pinMode(config::kLedPin, OUTPUT);
    logInfo("main", "%s started, firmware %s", config::kBoardName, config::kFirmwareVersion);
    wifi.begin();
}

void loop() {
    const uint32_t now = millis();
    wifi.update(now);
    updateStatusLed(now);
}
