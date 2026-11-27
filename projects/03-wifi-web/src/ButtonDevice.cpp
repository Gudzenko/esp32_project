#include "ButtonDevice.h"

#include <cstring>
#include <string>

namespace {

constexpr const char* kPressed = "pressed";

}  // namespace

ButtonDevice::ButtonDevice(const char* name, bot::Button& button) : name_(name), button_(button) {
    button_.addListener([this](bool) { notifyChanged(); });
}

const char* ButtonDevice::name() const {
    return name_;
}

const char* ButtonDevice::type() const {
    return "button";
}

void ButtonDevice::describe(JsonObject props) const {
    props[kPressed]["readonly"] = true;
}

void ButtonDevice::get(JsonObject data) const {
    data[kPressed] = button_.isPressed();
}

std::optional<ProtocolError> ButtonDevice::set(JsonObjectConst values, JsonObject data) {
    for (JsonPairConst value : values) {
        const char* prop = value.key().c_str();
        if (std::strcmp(prop, kPressed) == 0) {
            return ProtocolError{ErrorCode::kReadOnly, kPressed, "pressed is read-only"};
        }
        return ProtocolError{ErrorCode::kUnknownProp, prop,
                             std::string("unknown property: ") + prop};
    }
    get(data);
    return std::nullopt;
}
