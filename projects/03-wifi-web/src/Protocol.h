#pragma once

#include <ArduinoJson.h>

#include <cstdint>
#include <optional>
#include <string>

class Protocol {
public:
    Protocol(const char* boardName, const char* firmwareVersion, int protocolVersion);
    std::string handle(const char* text, size_t length, uint64_t timeMs) const;
    std::string busyError(uint64_t timeMs) const;

private:
    enum class ErrorCode : uint8_t {
        kBadJson,
        kBadRequest,
        kMissingId,
        kUnknownCmd,
        kUnknownDevice,
        kBusy,
    };

    struct Error {
        ErrorCode code;
        const char* field;
        std::string message;
    };

    static const char* codeName(ErrorCode code);

    std::optional<Error> execute(JsonVariantConst request, uint64_t timeMs,
                                 JsonDocument& response) const;
    std::optional<Error> validate(JsonVariantConst request) const;
    std::optional<Error> dispatch(const char* cmd, JsonVariantConst device,
                                  JsonDocument& response) const;
    void describe(JsonObject data) const;
    std::optional<Error> get(JsonVariantConst device, JsonDocument& response) const;
    std::optional<Error> set(JsonVariantConst device) const;
    std::string errorResponse(JsonVariantConst request, uint64_t timeMs, const Error& error) const;

    const char* boardName_;
    const char* firmwareVersion_;
    int protocolVersion_;
};
