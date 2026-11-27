#pragma once

#include <string>

#include "HttpServer.h"
#include "WifiLink.h"
#include "WsServer.h"

struct ConnectivitySettings {
    const char* ssid;
    const char* password;
    const char* hostName;
    StaticIp staticIp;
    uint32_t wifiRetryIntervalMs;
    uint16_t httpPort;
    uint16_t wsPort;
};

class Connectivity {
public:
    explicit Connectivity(const ConnectivitySettings& settings);
    void begin();
    void update(uint32_t now);
    void onMessage(WsServer::MessageHandler handler);
    void onBusy(WsServer::BusyResponder responder);
    void send(const std::string& text);

private:
    void startServers();

    WifiLink wifi_;
    HttpServer http_;
    WsServer ws_;
    bool serversStarted_ = false;
};
