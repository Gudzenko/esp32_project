#include "ButtonDevice.h"

#include "Names.h"

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
    if (values.size() == 0) {
        get(data);
        return std::nullopt;
    }
    const char* prop = values.begin()->key().c_str();
    return sameName(prop, kPressed) ? readOnly(kPressed) : unknownProp(prop);
}
