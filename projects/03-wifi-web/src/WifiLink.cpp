#include "WifiLink.h"

#include <ESPmDNS.h>
#include <WiFi.h>

#include "Log.h"

namespace {

constexpr const char* kLayer = "wifi";
constexpr uint8_t kNoReason = 0;

}  // namespace

WifiLink::WifiLink(const char* ssid, const char* password, const char* hostName,
                   uint32_t retryIntervalMs)
    : ssid_(ssid), password_(password), hostName_(hostName), retryIntervalMs_(retryIntervalMs) {}

void WifiLink::begin() {
    WiFi.onEvent(
        [this](arduino_event_id_t, arduino_event_info_t info) {
            const uint8_t reason = info.wifi_sta_disconnected.reason;
            if (reason != WIFI_REASON_ASSOC_LEAVE) {
                lastDisconnectReason_ = reason;
            }
        },
        ARDUINO_EVENT_WIFI_STA_DISCONNECTED);

    WiFi.setHostname(hostName_);
    WiFi.mode(WIFI_STA);
    WiFi.setSleep(false);
    WiFi.setAutoReconnect(false);
    logInfo(kLayer, "connecting to %s", ssid_);
    connect(millis());
}

void WifiLink::update(uint32_t now) {
    const bool connectedNow = WiFi.status() == WL_CONNECTED;
    if (connectedNow != connected_) {
        connected_ = connectedNow;
        if (connected_) {
            onConnected();
        } else {
            onDisconnected();
        }
        lastAttemptMs_ = now;
    }

    if (!connected_ && now - lastAttemptMs_ >= retryIntervalMs_) {
        logWarn(kLayer, "not connected (%s), retrying", lastFailureReason());
        connect(now);
    }
}

bool WifiLink::isConnected() const {
    return connected_;
}

void WifiLink::connect(uint32_t now) {
    lastAttemptMs_ = now;
    lastDisconnectReason_ = kNoReason;
    WiFi.disconnect();
    WiFi.begin(ssid_, password_);
}

void WifiLink::onConnected() {
    logInfo(kLayer, "connected to %s, IP %s, signal %d dBm", ssid_,
            WiFi.localIP().toString().c_str(), WiFi.RSSI());
    if (mdnsStarted_) {
        return;
    }
    mdnsStarted_ = MDNS.begin(hostName_);
    if (mdnsStarted_) {
        logInfo(kLayer, "mDNS name %s.local", hostName_);
    } else {
        logError(kLayer, "mDNS start failed");
    }
}

void WifiLink::onDisconnected() {
    logWarn(kLayer, "disconnected (%s)", lastFailureReason());
}

const char* WifiLink::lastFailureReason() const {
    const uint8_t reason = lastDisconnectReason_;
    if (reason == kNoReason) {
        return "no answer yet";
    }
    return WiFi.disconnectReasonName(static_cast<wifi_err_reason_t>(reason));
}
