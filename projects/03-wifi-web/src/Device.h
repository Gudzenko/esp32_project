#pragma once

#include <ArduinoJson.h>

#include <functional>
#include <optional>

#include "ProtocolError.h"

class Device {
public:
    using ChangeHandler = std::function<void(const Device& device)>;

    virtual ~Device() = default;
    virtual const char* name() const = 0;
    virtual const char* type() const = 0;
    virtual void describe(JsonObject props) const = 0;
    virtual void get(JsonObject data) const = 0;
    virtual std::optional<ProtocolError> set(JsonObjectConst values, JsonObject data) = 0;

    void onChanged(ChangeHandler handler) {
        changeHandler_ = std::move(handler);
    }

protected:
    void notifyChanged() const {
        if (changeHandler_) {
            changeHandler_(*this);
        }
    }

private:
    ChangeHandler changeHandler_;
};
