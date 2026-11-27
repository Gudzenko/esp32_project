#include "HttpServer.h"

#include "Log.h"

extern const char kIndexHtml[] asm("_binary_src_index_html_start");

namespace {

constexpr const char* kLayer = "http";
constexpr int kStatusOk = 200;
constexpr int kStatusNotFound = 404;

}  // namespace

HttpServer::HttpServer(uint16_t port) : server_(port), port_(port) {}

void HttpServer::begin() {
    server_.on("/", HTTP_GET, [this] { sendPage(); });
    server_.onNotFound([this] { sendNotFound(); });
    server_.begin();
    started_ = true;
    logInfo(kLayer, "server started on port %u", port_);
}

void HttpServer::update() {
    server_.handleClient();
}

bool HttpServer::isStarted() const {
    return started_;
}

void HttpServer::sendPage() {
    server_.send(kStatusOk, "text/html", kIndexHtml);
}

void HttpServer::sendNotFound() {
    logWarn(kLayer, "not found: %s %s", http_method_str(server_.method()), server_.uri().c_str());
    server_.send(kStatusNotFound, "text/plain", "Not found");
}
