#pragma once

#include <Button.h>

#include "Device.h"

class ButtonDevice : public Device {
public:
    ButtonDevice(const char* name, bot::Button& button);
    const char* name() const override;
    const char* type() const override;
    void describe(JsonObject props) const override;
    void get(JsonObject data) const override;
    std::optional<ProtocolError> set(JsonObjectConst values, JsonObject data) override;

private:
    const char* name_;
    bot::Button& button_;
};
