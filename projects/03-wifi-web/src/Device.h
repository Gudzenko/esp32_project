#pragma once

#include <ArduinoJson.h>

#include <optional>

#include "ProtocolError.h"

class Device {
public:
    virtual ~Device() = default;
    virtual const char* name() const = 0;
    virtual const char* type() const = 0;
    virtual void describe(JsonObject props) const = 0;
    virtual void get(JsonObject data) const = 0;
    virtual std::optional<ProtocolError> set(JsonObjectConst values, JsonObject data) = 0;
};
