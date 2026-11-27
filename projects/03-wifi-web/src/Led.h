#pragma once

#include <Arduino.h>

#include <functional>
#include <optional>
#include <vector>

#include "config.h"

class Led {
public:
    enum class Mode : uint8_t { kOff, kLinear, kQuadratic, kCubic, kBlink, kCount };
    static constexpr uint32_t kMaxDuty = (1u << config::kPwmResolutionBits) - 1;

    using Listener = std::function<void()>;

    explicit Led(uint8_t pin);
    bool begin();
    void update(uint32_t now);
    void setMode(Mode mode);
    Mode mode() const;
    uint32_t duty() const;
    void addListener(Listener listener);

    static const char* modeName(Mode mode);
    static std::optional<Mode> parseMode(const char* name);
    static Mode nextMode(Mode mode);

private:
    uint32_t dutyFor(uint32_t now) const;

    uint8_t pin_;
    Mode mode_ = Mode::kOff;
    uint32_t duty_ = 0;
    std::vector<Listener> listeners_;
};
