#include <Arduino.h>

#include "Devices.h"
#include "HttpServer.h"
#include "Led.h"
#include "LedDevice.h"
#include "Log.h"
#include "Protocol.h"
#include "Uptime.h"
#include "WifiLink.h"
#include "WsServer.h"
#include "config.h"

#if __has_include("secrets.h")
#include "secrets.h"
#else
#error "Copy src/secrets.example.h to src/secrets.h and fill in the Wi-Fi name and password"
#endif

WifiLink wifi(secrets::kWifiSsid, secrets::kWifiPassword, config::kBoardName,
              {secrets::kStaticIp, secrets::kGateway, secrets::kSubnet},
              config::kWifiRetryIntervalMs);
HttpServer http(config::kHttpPort);
WsServer ws(config::kWsPort);
Led led(config::kLedName, config::kLedPin);
LedDevice ledDevice(led);
Devices devices({&ledDevice});
Protocol protocol(config::kBoardName, config::kFirmwareVersion, config::kProtocolVersion,
                  devices);

bool serversStarted = false;

void startServers() {
    http.begin();
    ws.begin();
    serversStarted = true;
}

void updateServers() {
    if (!wifi.isConnected()) {
        return;
    }
    if (!serversStarted) {
        startServers();
    }
    http.update();
    ws.update();
}

void handleMessage(const char* text, size_t length) {
    const std::string response = protocol.handle(text, length, uptimeMs());
    ws.send(response.c_str(), response.size());
}

std::string busyResponse() {
    return protocol.busyError(uptimeMs());
}

void setup() {
    Serial.begin(config::kSerialBaud);
    logInfo("main", "%s started, firmware %s", config::kBoardName, config::kFirmwareVersion);
    if (!led.begin()) {
        logError("main", "LED PWM setup failed");
    }
    ws.onMessage(handleMessage);
    ws.onBusy(busyResponse);
    wifi.begin();
}

void loop() {
    const uint32_t now = millis();
    wifi.update(now);
    updateServers();
    led.update(now);
}
