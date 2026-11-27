#pragma once

#include <Arduino.h>

#include <optional>

#include "config.h"

class Led {
public:
    enum class Mode : uint8_t { kOff, kLinear, kQuadratic, kCubic, kBlink, kCount };
    static constexpr uint32_t kMaxDuty = (1u << config::kPwmResolutionBits) - 1;

    Led(const char* name, uint8_t pin);
    const char* name() const;
    bool begin();
    void update(uint32_t now);
    void setMode(Mode mode);
    Mode mode() const;
    uint32_t duty() const;

    static const char* modeName(Mode mode);
    static std::optional<Mode> parseMode(const char* name);

private:
    uint32_t dutyFor(uint32_t now) const;

    const char* name_;
    uint8_t pin_;
    Mode mode_ = Mode::kOff;
    uint32_t duty_ = 0;
};
