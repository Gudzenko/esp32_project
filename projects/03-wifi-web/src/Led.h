#pragma once

#include <Arduino.h>

#include <functional>
#include <optional>
#include <vector>

struct LedSettings {
    uint8_t pin;
    uint32_t pwmFrequencyHz;
    uint8_t pwmResolutionBits;
    uint32_t breathPeriodMs;
    uint32_t blinkHalfPeriodMs;
};

class Led {
public:
    enum class Mode : uint8_t { kOff, kLinear, kQuadratic, kCubic, kBlink, kCount };

    using Listener = std::function<void()>;

    explicit Led(const LedSettings& settings);
    bool begin();
    void update(uint32_t now);
    void setMode(Mode mode);
    Mode mode() const;
    uint32_t duty() const;
    uint32_t maxDuty() const;
    void addListener(Listener listener);

    static const char* modeName(Mode mode);
    static std::optional<Mode> parseMode(const char* name);
    static Mode nextMode(Mode mode);

private:
    uint32_t dutyFor(uint32_t now) const;
    float breathLevel(uint32_t now) const;
    uint32_t toDuty(float level) const;

    LedSettings settings_;
    uint32_t maxDuty_;
    Mode mode_ = Mode::kOff;
    uint32_t duty_ = 0;
    std::vector<Listener> listeners_;
};
