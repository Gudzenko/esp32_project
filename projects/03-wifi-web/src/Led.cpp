#include "Led.h"

#include "Log.h"
#include "Names.h"

namespace {

constexpr const char* kLayer = "led";

}  // namespace

Led::Led(const LedSettings& settings)
    : settings_(settings), maxDuty_((1u << settings.pwmResolutionBits) - 1) {}

bool Led::begin() {
    if (!ledcAttach(settings_.pin, settings_.pwmFrequencyHz, settings_.pwmResolutionBits)) {
        return false;
    }
    ledcWrite(settings_.pin, duty_);
    return true;
}

void Led::update(uint32_t now) {
    const uint32_t duty = dutyFor(now);
    if (duty == duty_) {
        return;
    }
    duty_ = duty;
    ledcWrite(settings_.pin, duty_);
}

void Led::setMode(Mode mode) {
    if (mode == mode_) {
        return;
    }
    mode_ = mode;
    update(millis());
    logInfo(kLayer, "pin %u mode %s", settings_.pin, modeName(mode_));
    for (const Listener& listener : listeners_) {
        listener();
    }
}

void Led::addListener(Listener listener) {
    listeners_.push_back(std::move(listener));
}

Led::Mode Led::mode() const {
    return mode_;
}

uint32_t Led::duty() const {
    return duty_;
}

uint32_t Led::maxDuty() const {
    return maxDuty_;
}

const char* Led::modeName(Mode mode) {
    switch (mode) {
        case Mode::kOff:
            return "off";
        case Mode::kLinear:
            return "linear";
        case Mode::kQuadratic:
            return "quadratic";
        case Mode::kCubic:
            return "cubic";
        case Mode::kBlink:
            return "blink";
        case Mode::kCount:
            break;
    }
    return "unknown";
}

std::optional<Led::Mode> Led::parseMode(const char* name) {
    for (uint8_t i = 0; i < static_cast<uint8_t>(Mode::kCount); ++i) {
        const Mode mode = static_cast<Mode>(i);
        if (sameName(name, modeName(mode))) {
            return mode;
        }
    }
    return std::nullopt;
}

Led::Mode Led::nextMode(Mode mode) {
    const uint8_t next = (static_cast<uint8_t>(mode) + 1) % static_cast<uint8_t>(Mode::kCount);
    return static_cast<Mode>(next);
}

uint32_t Led::dutyFor(uint32_t now) const {
    const float level = breathLevel(now);
    switch (mode_) {
        case Mode::kOff:
            return 0;
        case Mode::kLinear:
            return toDuty(level);
        case Mode::kQuadratic:
            return toDuty(level * level);
        case Mode::kCubic:
            return toDuty(level * level * level);
        case Mode::kBlink:
            return (now / settings_.blinkHalfPeriodMs) % 2 == 0 ? maxDuty_ : 0;
        case Mode::kCount:
            break;
    }
    return 0;
}

float Led::breathLevel(uint32_t now) const {
    const float phase =
        static_cast<float>(now % settings_.breathPeriodMs) / settings_.breathPeriodMs;
    return phase < 0.5f ? phase * 2.0f : 2.0f - phase * 2.0f;
}

uint32_t Led::toDuty(float level) const {
    return static_cast<uint32_t>(level * maxDuty_ + 0.5f);
}
