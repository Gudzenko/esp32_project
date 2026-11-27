#pragma once

#include <Arduino.h>

#include <functional>
#include <vector>

namespace bot {

class Button {
public:
    using Listener = std::function<void(bool pressed)>;

    explicit Button(uint8_t pin);

    void begin();
    void update();

    bool isPressed() const;
    bool wasPressed();
    void addListener(Listener listener);

private:
    // TODO: debounce. Not needed for the on-board BOOT button; add with the first external button.
    uint8_t pin_;
    bool pressed_ = false;
    bool pressedEvent_ = false;
    std::vector<Listener> listeners_;
};

}  // namespace bot
