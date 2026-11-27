#include <Arduino.h>
#include <Button.h>

#include "ButtonDevice.h"
#include "Controller.h"
#include "Devices.h"
#include "Led.h"
#include "LedDevice.h"
#include "Log.h"
#include "Connectivity.h"
#include "Protocol.h"
#include "Uptime.h"
#include "config.h"

#if __has_include("secrets.h")
#include "secrets.h"
#else
#error "Copy src/secrets.example.h to src/secrets.h and fill in the Wi-Fi name and password"
#endif

Connectivity connectivity({secrets::kWifiSsid, secrets::kWifiPassword, config::kBoardName,
                           {secrets::kStaticIp, secrets::kGateway, secrets::kSubnet},
                           config::kWifiRetryIntervalMs, config::kHttpPort, config::kWsPort});
bot::Button bootButton(config::kBootButtonPin);
Led led({config::kLedPin, config::kPwmFrequencyHz, config::kPwmResolutionBits,
         config::kLedBreathPeriodMs, config::kLedBlinkHalfPeriodMs});
LedDevice ledDevice(config::kLedName, led);
ButtonDevice bootButtonDevice(config::kBootButtonName, bootButton);
Devices devices({&ledDevice, &bootButtonDevice});
Controller controller(bootButton, led);
Protocol protocol(config::kBoardName, config::kFirmwareVersion, config::kProtocolVersion,
                  devices);

void handleMessage(const char* text, size_t length) {
    connectivity.send(protocol.handle(text, length, uptimeMs()));
}

std::string busyResponse() {
    return protocol.busyError(uptimeMs());
}

void sendChangedEvent(const Device& device) {
    connectivity.send(protocol.changedEvent(device, uptimeMs()));
}

void beginHardware() {
    if (!led.begin()) {
        logError("main", "LED PWM setup failed");
    }
    bootButton.begin();
}

void connectMessages() {
    devices.onChanged(sendChangedEvent);
    connectivity.onMessage(handleMessage);
    connectivity.onBusy(busyResponse);
}

void setup() {
    Serial.begin(config::kSerialBaud);
    logInfo("main", "%s started, firmware %s", config::kBoardName, config::kFirmwareVersion);
    beginHardware();
    connectMessages();
    connectivity.begin();
}

void loop() {
    const uint32_t now = millis();
    connectivity.update(now);
    bootButton.update();
    led.update(now);
}
