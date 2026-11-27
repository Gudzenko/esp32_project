#pragma once

#include <ArduinoJson.h>

#include <optional>
#include <vector>

#include "Device.h"
#include "ProtocolError.h"

class Devices {
public:
    explicit Devices(std::vector<Device*> devices);
    void describe(JsonObject devices) const;
    void getAll(JsonObject data) const;
    std::optional<ProtocolError> get(const char* name, JsonObject data) const;
    std::optional<ProtocolError> set(const char* name, JsonObjectConst values, JsonObject data);

private:
    Device* find(const char* name) const;

    std::vector<Device*> devices_;
};
