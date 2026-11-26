#include <Arduino.h>
#include <Button.h>

enum class Mode : uint8_t { kOff, kLinear, kQuadratic, kCubic, kBlink, kCount };

constexpr uint8_t kLedPin = 2;
constexpr uint8_t kBootButtonPin = 0;
constexpr uint32_t kSerialBaud = 115200;
constexpr uint32_t kPwmFrequencyHz = 5000;
constexpr uint8_t kPwmResolutionBits = 12;
constexpr uint32_t kMaxDuty = (1u << kPwmResolutionBits) - 1;
constexpr uint32_t kBreathPeriodMs = 2000;
constexpr uint32_t kBlinkHalfPeriodMs = 500;

bot::Button button(kBootButtonPin);
Mode mode = Mode::kOff;
uint32_t lastDuty = UINT32_MAX;

const char* modeName(Mode m) {
    switch (m) {
        case Mode::kOff:
            return "off";
        case Mode::kLinear:
            return "breathing, linear";
        case Mode::kQuadratic:
            return "breathing, quadratic";
        case Mode::kCubic:
            return "breathing, cubic";
        case Mode::kBlink:
            return "blink";
        case Mode::kCount:
            break;
    }
    return "unknown";
}

Mode nextMode(Mode m) {
    const uint8_t next = (static_cast<uint8_t>(m) + 1) % static_cast<uint8_t>(Mode::kCount);
    return static_cast<Mode>(next);
}

float breathLevel(uint32_t now) {
    const float phase = static_cast<float>(now % kBreathPeriodMs) / kBreathPeriodMs;
    return phase < 0.5f ? phase * 2.0f : 2.0f - phase * 2.0f;
}

uint32_t toDuty(float level) {
    return static_cast<uint32_t>(level * kMaxDuty + 0.5f);
}

uint32_t dutyFor(Mode m, uint32_t now) {
    const float level = breathLevel(now);
    switch (m) {
        case Mode::kOff:
            return 0;
        case Mode::kLinear:
            return toDuty(level);
        case Mode::kQuadratic:
            return toDuty(level * level);
        case Mode::kCubic:
            return toDuty(level * level * level);
        case Mode::kBlink:
            return (now / kBlinkHalfPeriodMs) % 2 == 0 ? kMaxDuty : 0;
        case Mode::kCount:
            break;
    }
    return 0;
}

void setup() {
    Serial.begin(kSerialBaud);
    button.begin();
    if (!ledcAttach(kLedPin, kPwmFrequencyHz, kPwmResolutionBits)) {
        Serial.println("ERROR: PWM setup failed");
    }
    Serial.println("02-button-pwm started");
    Serial.printf("mode: %s\n", modeName(mode));
}

void loop() {
    button.update();
    if (button.wasPressed()) {
        mode = nextMode(mode);
        Serial.printf("mode: %s\n", modeName(mode));
    }

    const uint32_t duty = dutyFor(mode, millis());
    if (duty != lastDuty) {
        ledcWrite(kLedPin, duty);
        lastDuty = duty;
    }
}
