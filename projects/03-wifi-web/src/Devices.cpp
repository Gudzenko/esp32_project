#include "Devices.h"

#include <cstring>
#include <string>

namespace {

ProtocolError unknownDevice(const char* device) {
    return {ErrorCode::kUnknownDevice, "device", std::string("unknown device: ") + device};
}

}  // namespace

Devices::Devices(std::vector<Device*> devices) : devices_(std::move(devices)) {}

void Devices::describe(JsonObject devices) const {
    for (const Device* device : devices_) {
        JsonObject description = devices[device->name()].to<JsonObject>();
        description["type"] = device->type();
        device->describe(description["props"].to<JsonObject>());
    }
}

void Devices::getAll(JsonObject data) const {
    for (const Device* device : devices_) {
        device->get(data[device->name()].to<JsonObject>());
    }
}

std::optional<ProtocolError> Devices::get(const char* name, JsonObject data) const {
    if (const Device* device = find(name)) {
        device->get(data);
        return std::nullopt;
    }
    return unknownDevice(name);
}

std::optional<ProtocolError> Devices::set(const char* name, JsonObjectConst values,
                                          JsonObject data) {
    if (Device* device = find(name)) {
        return device->set(values, data);
    }
    return unknownDevice(name);
}

Device* Devices::find(const char* name) const {
    for (Device* device : devices_) {
        if (std::strcmp(name, device->name()) == 0) {
            return device;
        }
    }
    return nullptr;
}
