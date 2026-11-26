#pragma once

#include <Arduino.h>

namespace bot {

class Button {
public:
    explicit Button(uint8_t pin);

    void begin();
    void update();

    bool isPressed() const;
    bool wasPressed();

private:
    // TODO: debounce. Not needed for the on-board BOOT button; add with the first external button.
    uint8_t pin_;
    bool pressed_ = false;
    bool pressedEvent_ = false;
};

}  // namespace bot
