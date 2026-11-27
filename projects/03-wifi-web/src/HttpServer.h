#pragma once

#include <Arduino.h>
#include <WebServer.h>

class HttpServer {
public:
    explicit HttpServer(uint16_t port);
    void begin();
    void update();
    bool isStarted() const;

private:
    void sendPage();
    void sendNotFound();

    WebServer server_;
    uint16_t port_;
    bool started_ = false;
};
