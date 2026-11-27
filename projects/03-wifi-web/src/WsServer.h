#pragma once

#include <Arduino.h>
#include <WebSocketsServer.h>

#include <functional>

class WsServer {
public:
    using MessageHandler = std::function<void(const char* text, size_t length)>;

    explicit WsServer(uint16_t port);
    void begin();
    void update();
    void onMessage(MessageHandler handler);
    void send(const char* text, size_t length);

private:
    static constexpr uint8_t kNoClient = UINT8_MAX;

    void handleEvent(uint8_t client, WStype_t type, uint8_t* payload, size_t length);
    void onConnected(uint8_t client);
    void onDisconnected(uint8_t client);
    void rejectBusy(uint8_t client);

    WebSocketsServer server_;
    uint16_t port_;
    uint8_t activeClient_ = kNoClient;
    MessageHandler messageHandler_;
};
