#include "LedDevice.h"

#include <cstring>
#include <string>

namespace {

constexpr const char* kMode = "mode";
constexpr const char* kDuty = "duty";

bool isName(const char* name, const char* expected) {
    return std::strcmp(name, expected) == 0;
}

std::string allowedModes() {
    std::string text = "allowed:";
    for (uint8_t i = 0; i < static_cast<uint8_t>(Led::Mode::kCount); ++i) {
        text += i == 0 ? " " : ", ";
        text += Led::modeName(static_cast<Led::Mode>(i));
    }
    return text;
}

}  // namespace

LedDevice::LedDevice(const char* name, Led& led) : name_(name), led_(led) {
    led_.addListener([this] { notifyChanged(); });
}

const char* LedDevice::name() const {
    return name_;
}

const char* LedDevice::type() const {
    return "led";
}

void LedDevice::describe(JsonObject props) const {
    JsonArray modes = props[kMode]["values"].to<JsonArray>();
    for (uint8_t i = 0; i < static_cast<uint8_t>(Led::Mode::kCount); ++i) {
        modes.add(Led::modeName(static_cast<Led::Mode>(i)));
    }
    JsonObject duty = props[kDuty].to<JsonObject>();
    duty["min"] = 0;
    duty["max"] = Led::kMaxDuty;
    duty["readonly"] = true;
}

void LedDevice::get(JsonObject data) const {
    data[kMode] = Led::modeName(led_.mode());
    data[kDuty] = led_.duty();
}

std::optional<ProtocolError> LedDevice::set(JsonObjectConst values, JsonObject data) {
    std::optional<Led::Mode> newMode;
    for (JsonPairConst value : values) {
        const char* prop = value.key().c_str();
        if (isName(prop, kDuty)) {
            return ProtocolError{ErrorCode::kReadOnly, kDuty, "duty is read-only"};
        }
        if (!isName(prop, kMode)) {
            return ProtocolError{ErrorCode::kUnknownProp, prop,
                                 std::string("unknown property: ") + prop};
        }
        const char* name = value.value().as<const char*>();
        newMode = name != nullptr ? Led::parseMode(name) : std::nullopt;
        if (!newMode) {
            return ProtocolError{ErrorCode::kBadValue, kMode, allowedModes()};
        }
    }
    if (newMode) {
        led_.setMode(*newMode);
    }
    get(data);
    return std::nullopt;
}
