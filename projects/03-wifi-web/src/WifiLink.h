#pragma once

#include <Arduino.h>

#include <atomic>

class WifiLink {
public:
    WifiLink(const char* ssid, const char* password, const char* hostName,
             uint32_t retryIntervalMs);
    void begin();
    void update(uint32_t now);
    bool isConnected() const;

private:
    void connect(uint32_t now);
    void onConnected();
    void onDisconnected();
    const char* lastFailureReason() const;

    const char* ssid_;
    const char* password_;
    const char* hostName_;
    uint32_t retryIntervalMs_;
    bool connected_ = false;
    bool mdnsStarted_ = false;
    uint32_t lastAttemptMs_ = 0;
    // Written by the Wi-Fi event task, read in loop().
    std::atomic<uint8_t> lastDisconnectReason_{0};
};
