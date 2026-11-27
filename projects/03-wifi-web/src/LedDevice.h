#pragma once

#include "Device.h"
#include "Led.h"

class LedDevice : public Device {
public:
    LedDevice(const char* name, Led& led);
    const char* name() const override;
    const char* type() const override;
    void describe(JsonObject props) const override;
    void get(JsonObject data) const override;
    std::optional<ProtocolError> set(JsonObjectConst values, JsonObject data) override;

private:
    const char* name_;
    Led& led_;
};
