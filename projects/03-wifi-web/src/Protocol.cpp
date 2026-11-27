#include "Protocol.h"

#include <cinttypes>

#include "Log.h"
#include "Names.h"

namespace {

constexpr const char* kLayer = "protocol";

void logRequestError(const char* code, const std::string& message, JsonVariantConst request) {
    const JsonVariantConst id = request["id"];
    if (id.is<uint32_t>()) {
        logWarn(kLayer, "%s: %s, id %" PRIu32, code, message.c_str(), id.as<uint32_t>());
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

Protocol::Protocol(const char* boardName, const char* firmwareVersion, int protocolVersion,
                   Devices& devices)
    : boardName_(boardName),
      firmwareVersion_(firmwareVersion),
      protocolVersion_(protocolVersion),
      devices_(devices) {}

std::string Protocol::handle(const char* text, size_t length, uint64_t timeMs) {
    JsonDocument request;
    const DeserializationError parseError = deserializeJson(request, text, length);
    if (parseError) {
        const ProtocolError error{ErrorCode::kBadJson, "",
                                  std::string("invalid JSON: ") + parseError.c_str()};
        logRequestError(errorCodeName(error.code), error.message, JsonVariantConst());
        return errorResponse(JsonVariantConst(), timeMs, error);
    }

    JsonDocument response;
    const Result error = execute(request.as<JsonVariantConst>(), timeMs, response);
    if (error) {
        logRequestError(errorCodeName(error->code), error->message, request.as<JsonVariantConst>());
        return errorResponse(request.as<JsonVariantConst>(), timeMs, *error);
    }
    return serialize(response);
}

std::string Protocol::busyError(uint64_t timeMs) const {
    return errorResponse(JsonVariantConst(), timeMs,
                         {ErrorCode::kBusy, "", "another client is connected"});
}

std::string Protocol::changedEvent(const Device& device, uint64_t timeMs) const {
    JsonDocument event;
    event["event"] = "changed";
    event["time"] = timeMs;
    event["device"] = device.name();
    device.get(event["data"].to<JsonObject>());
    return serialize(event);
}

Protocol::Result Protocol::execute(JsonVariantConst request, uint64_t timeMs,
                                   JsonDocument& response) {
    const Result invalid = validate(request);
    if (invalid) {
        return invalid;
    }
    response["id"] = request["id"];
    response["time"] = timeMs;
    response["status"] = "ok";
    return dispatch(request["cmd"].as<const char*>(), request, response);
}

Protocol::Result Protocol::validate(JsonVariantConst request) const {
    if (!request.is<JsonObjectConst>()) {
        return ProtocolError{ErrorCode::kBadRequest, "", "message must be a JSON object"};
    }
    const JsonVariantConst id = request["id"];
    if (id.isNull()) {
        return ProtocolError{ErrorCode::kMissingId, "id", "id is required"};
    }
    if (!id.is<uint32_t>()) {
        return ProtocolError{ErrorCode::kBadRequest, "id", "id must be a non-negative integer"};
    }
    const JsonVariantConst cmd = request["cmd"];
    if (cmd.isNull()) {
        return ProtocolError{ErrorCode::kBadRequest, "cmd", "cmd is required"};
    }
    if (!cmd.is<const char*>()) {
        return ProtocolError{ErrorCode::kBadRequest, "cmd", "cmd must be a string"};
    }
    const JsonVariantConst device = request["device"];
    if (!device.isNull() && !device.is<const char*>()) {
        return ProtocolError{ErrorCode::kBadRequest, "device", "device must be a string"};
    }
    return std::nullopt;
}

Protocol::Result Protocol::dispatch(const char* cmd, JsonVariantConst request,
                                    JsonDocument& response) {
    if (sameName(cmd, "ping")) {
        return std::nullopt;
    }
    if (sameName(cmd, "describe")) {
        describe(response["data"].to<JsonObject>());
        return std::nullopt;
    }
    if (sameName(cmd, "get")) {
        return get(request["device"], response);
    }
    if (sameName(cmd, "set")) {
        return set(request["device"], request["data"], response);
    }
    return ProtocolError{ErrorCode::kUnknownCmd, "cmd", std::string("unknown command: ") + cmd};
}

void Protocol::describe(JsonObject data) const {
    data["board"] = boardName_;
    data["firmware"] = firmwareVersion_;
    data["protocol"] = protocolVersion_;
    devices_.describe(data["devices"].to<JsonObject>());
}

Protocol::Result Protocol::get(JsonVariantConst device, JsonDocument& response) const {
    if (device.isNull()) {
        devices_.getAll(response["data"].to<JsonObject>());
        return std::nullopt;
    }
    response["device"] = device;
    return devices_.get(device.as<const char*>(), response["data"].to<JsonObject>());
}

Protocol::Result Protocol::set(JsonVariantConst device, JsonVariantConst values,
                               JsonDocument& response) {
    if (device.isNull()) {
        return ProtocolError{ErrorCode::kBadRequest, "device", "device is required"};
    }
    if (values.isNull()) {
        return ProtocolError{ErrorCode::kBadRequest, "data", "data is required"};
    }
    if (!values.is<JsonObjectConst>()) {
        return ProtocolError{ErrorCode::kBadRequest, "data", "data must be an object"};
    }
    response["device"] = device;
    return devices_.set(device.as<const char*>(), values.as<JsonObjectConst>(),
                        response["data"].to<JsonObject>());
}

std::string Protocol::errorResponse(JsonVariantConst request, uint64_t timeMs,
                                    const ProtocolError& error) const {
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
    details["code"] = errorCodeName(error.code);
    if (!error.field.empty()) {
        details["field"] = error.field;
    }
    details["message"] = error.message;
    return serialize(response);
}
