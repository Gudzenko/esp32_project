#include "WsServer.h"

#include "Log.h"
#include "config.h"

namespace {

constexpr const char* kLayer = "ws";

}  // namespace

WsServer::WsServer(uint16_t port) : server_(port), port_(port) {}

void WsServer::begin() {
    server_.onEvent([this](uint8_t client, WStype_t type, uint8_t* payload, size_t length) {
        handleEvent(client, type, payload, length);
    });
    server_.enableHeartbeat(config::kWsPingIntervalMs, config::kWsPongTimeoutMs,
                            config::kWsMissedPongsToDisconnect);
    server_.begin();
    logInfo(kLayer, "server started on port %u", port_);
}

void WsServer::update() {
    server_.loop();
}

void WsServer::onMessage(MessageHandler handler) {
    messageHandler_ = std::move(handler);
}

void WsServer::onBusy(BusyResponder responder) {
    busyResponder_ = std::move(responder);
}

void WsServer::send(const char* text, size_t length) {
    if (activeClient_ == kNoClient) {
        return;
    }
    server_.sendTXT(activeClient_, text, length);
}

void WsServer::handleEvent(uint8_t client, WStype_t type, uint8_t* payload, size_t length) {
    switch (type) {
        case WStype_CONNECTED:
            onConnected(client);
            break;
        case WStype_DISCONNECTED:
            onDisconnected(client);
            break;
        case WStype_ERROR:
            logWarn(kLayer, "client %u error", client);
            break;
        case WStype_TEXT:
        case WStype_BIN:
            if (client == activeClient_ && messageHandler_) {
                messageHandler_(reinterpret_cast<const char*>(payload), length);
            }
            break;
        default:
            break;
    }
}

void WsServer::onConnected(uint8_t client) {
    if (activeClient_ != kNoClient) {
        rejectBusy(client);
        return;
    }
    activeClient_ = client;
    // TODO: free heap is printed for debugging only; remove when memory use is known.
    logInfo(kLayer, "client connected from %s, free heap %lu",
            server_.remoteIP(client).toString().c_str(), ESP.getFreeHeap());
}

void WsServer::onDisconnected(uint8_t client) {
    if (client != activeClient_) {
        return;
    }
    activeClient_ = kNoClient;
    logInfo(kLayer, "client disconnected, free heap %lu", ESP.getFreeHeap());
}

void WsServer::rejectBusy(uint8_t client) {
    logWarn(kLayer, "rejected %s: busy", server_.remoteIP(client).toString().c_str());
    if (busyResponder_) {
        const std::string response = busyResponder_();
        server_.sendTXT(client, response.c_str(), response.size());
    }
    server_.disconnect(client);
}
