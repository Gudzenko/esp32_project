#pragma once

#include <ArduinoJson.h>

#include <cstdint>
#include <optional>
#include <string>

#include "Devices.h"
#include "ProtocolError.h"

class Protocol {
public:
    Protocol(const char* boardName, const char* firmwareVersion, int protocolVersion,
             Devices& devices);
    std::string handle(const char* text, size_t length, uint64_t timeMs);
    std::string busyError(uint64_t timeMs) const;
    std::string changedEvent(const Device& device, uint64_t timeMs) const;

private:
    using Result = std::optional<ProtocolError>;

    Result execute(JsonVariantConst request, uint64_t timeMs, JsonDocument& response);
    Result validate(JsonVariantConst request) const;
    Result dispatch(const char* cmd, JsonVariantConst request, JsonDocument& response);
    void describe(JsonObject data) const;
    Result get(JsonVariantConst device, JsonDocument& response) const;
    Result set(JsonVariantConst device, JsonVariantConst values, JsonDocument& response);
    std::string errorResponse(JsonVariantConst request, uint64_t timeMs,
                              const ProtocolError& error) const;

    const char* boardName_;
    const char* firmwareVersion_;
    int protocolVersion_;
    Devices& devices_;
};
