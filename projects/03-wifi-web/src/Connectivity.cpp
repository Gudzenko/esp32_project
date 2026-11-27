#include "Connectivity.h"

Connectivity::Connectivity(const ConnectivitySettings& settings)
    : wifi_(settings.ssid, settings.password, settings.hostName, settings.staticIp,
            settings.wifiRetryIntervalMs),
      http_(settings.httpPort),
      ws_(settings.wsPort) {}

void Connectivity::begin() {
    wifi_.begin();
}

void Connectivity::update(uint32_t now) {
    wifi_.update(now);
    if (!wifi_.isConnected()) {
        return;
    }
    if (!serversStarted_) {
        startServers();
    }
    http_.update();
    ws_.update();
}

void Connectivity::onMessage(WsServer::MessageHandler handler) {
    ws_.onMessage(std::move(handler));
}

void Connectivity::onBusy(WsServer::BusyResponder responder) {
    ws_.onBusy(std::move(responder));
}

void Connectivity::send(const std::string& text) {
    ws_.send(text.c_str(), text.size());
}

void Connectivity::startServers() {
    http_.begin();
    ws_.begin();
    serversStarted_ = true;
}
