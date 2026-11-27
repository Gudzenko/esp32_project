#include "Protocol.h"

#include <cstring>

#include "Log.h"

namespace {

constexpr const char* kLayer = "protocol";

bool isCommand(const char* cmd, const char* name) {
    return std::strcmp(cmd, name) == 0;
}

void logRequestError(const char* code, const std::string& message, JsonVariantConst request) {
    const JsonVariantConst id = request["id"];
    if (id.is<uint32_t>()) {
        logWarn(kLayer, "%s: %s, id %lu", code, message.c_str(), id.as<uint32_t>());
    } else {
        logWarn(kLayer, "%s: %s", code, message.c_str());
    }
}

std::string serialize(const JsonDocument& document) {
    std::string text;
    serializeJson(document, text);
    return text;
}

}  // namespace

Protocol::Protocol(const char* boardName, const char* firmwareVersion, int protocolVersion)
    : boardName_(boardName), firmwareVersion_(firmwareVersion), protocolVersion_(protocolVersion) {}

std::string Protocol::handle(const char* text, size_t length, uint64_t timeMs) const {
    JsonDocument request;
    const DeserializationError parseError = deserializeJson(request, text, length);
    if (parseError) {
        const Error error{ErrorCode::kBadJson, nullptr,
                          std::string("invalid JSON: ") + parseError.c_str()};
        logRequestError(codeName(error.code), error.message, JsonVariantConst());
        return errorResponse(JsonVariantConst(), timeMs, error);
    }

    JsonDocument response;
    const std::optional<Error> error = execute(request.as<JsonVariantConst>(), timeMs, response);
    if (error) {
        logRequestError(codeName(error->code), error->message, request.as<JsonVariantConst>());
        return errorResponse(request.as<JsonVariantConst>(), timeMs, *error);
    }
    return serialize(response);
}

std::string Protocol::busyError(uint64_t timeMs) const {
    return errorResponse(JsonVariantConst(), timeMs,
                         {ErrorCode::kBusy, nullptr, "another client is connected"});
}

const char* Protocol::codeName(ErrorCode code) {
    switch (code) {
        case ErrorCode::kBadJson:
            return "bad_json";
        case ErrorCode::kBadRequest:
            return "bad_request";
        case ErrorCode::kMissingId:
            return "missing_id";
        case ErrorCode::kUnknownCmd:
            return "unknown_cmd";
        case ErrorCode::kUnknownDevice:
            return "unknown_device";
        case ErrorCode::kBusy:
            return "busy";
    }
    return "unknown";
}

std::optional<Protocol::Error> Protocol::execute(JsonVariantConst request, uint64_t timeMs,
                                                 JsonDocument& response) const {
    const std::optional<Error> invalid = validate(request);
    if (invalid) {
        return invalid;
    }
    response["id"] = request["id"];
    response["time"] = timeMs;
    response["status"] = "ok";
    return dispatch(request["cmd"].as<const char*>(), request["device"], response);
}

std::optional<Protocol::Error> Protocol::validate(JsonVariantConst request) const {
    if (!request.is<JsonObjectConst>()) {
        return Error{ErrorCode::kBadRequest, nullptr, "message must be a JSON object"};
    }
    const JsonVariantConst id = request["id"];
    if (id.isNull()) {
        return Error{ErrorCode::kMissingId, "id", "id is required"};
    }
    if (!id.is<uint32_t>()) {
        return Error{ErrorCode::kBadRequest, "id", "id must be a non-negative integer"};
    }
    const JsonVariantConst cmd = request["cmd"];
    if (cmd.isNull()) {
        return Error{ErrorCode::kBadRequest, "cmd", "cmd is required"};
    }
    if (!cmd.is<const char*>()) {
        return Error{ErrorCode::kBadRequest, "cmd", "cmd must be a string"};
    }
    const JsonVariantConst device = request["device"];
    if (!device.isNull() && !device.is<const char*>()) {
        return Error{ErrorCode::kBadRequest, "device", "device must be a string"};
    }
    return std::nullopt;
}

std::optional<Protocol::Error> Protocol::dispatch(const char* cmd, JsonVariantConst device,
                                                  JsonDocument& response) const {
    if (isCommand(cmd, "ping")) {
        return std::nullopt;
    }
    if (isCommand(cmd, "describe")) {
        describe(response["data"].to<JsonObject>());
        return std::nullopt;
    }
    if (isCommand(cmd, "get")) {
        return get(device, response);
    }
    if (isCommand(cmd, "set")) {
        return set(device);
    }
    return Error{ErrorCode::kUnknownCmd, "cmd", std::string("unknown command: ") + cmd};
}

void Protocol::describe(JsonObject data) const {
    data["board"] = boardName_;
    data["firmware"] = firmwareVersion_;
    data["protocol"] = protocolVersion_;
    data["devices"].to<JsonObject>();
}

std::optional<Protocol::Error> Protocol::get(JsonVariantConst device,
                                             JsonDocument& response) const {
    if (!device.isNull()) {
        return Error{ErrorCode::kUnknownDevice, "device",
                     std::string("unknown device: ") + device.as<const char*>()};
    }
    response["data"].to<JsonObject>();
    return std::nullopt;
}

std::optional<Protocol::Error> Protocol::set(JsonVariantConst device) const {
    if (device.isNull()) {
        return Error{ErrorCode::kBadRequest, "device", "device is required"};
    }
    return Error{ErrorCode::kUnknownDevice, "device",
                 std::string("unknown device: ") + device.as<const char*>()};
}

std::string Protocol::errorResponse(JsonVariantConst request, uint64_t timeMs,
                                    const Error& error) const {
    JsonDocument response;
    if (request["id"].is<uint32_t>()) {
        response["id"] = request["id"];
    }
    response["time"] = timeMs;
    response["status"] = "error";
    if (request["device"].is<const char*>()) {
        response["device"] = request["device"];
    }
    JsonObject details = response["error"].to<JsonObject>();
    details["code"] = codeName(error.code);
    if (error.field != nullptr) {
        details["field"] = error.field;
    }
    details["message"] = error.message;
    return serialize(response);
}
