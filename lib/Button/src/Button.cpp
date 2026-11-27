#include "Button.h"

namespace bot {

Button::Button(uint8_t pin) : pin_(pin) {}

void Button::begin() {
    pinMode(pin_, INPUT_PULLUP);
    pressed_ = digitalRead(pin_) == LOW;
}

void Button::update() {
    const bool pressedNow = digitalRead(pin_) == LOW;
    if (pressedNow == pressed_) {
        return;
    }
    pressed_ = pressedNow;
    if (pressed_) {
        pressedEvent_ = true;
    }
    for (const Listener& listener : listeners_) {
        listener(pressed_);
    }
}

bool Button::isPressed() const {
    return pressed_;
}

bool Button::wasPressed() {
    const bool event = pressedEvent_;
    pressedEvent_ = false;
    return event;
}

void Button::addListener(Listener listener) {
    listeners_.push_back(std::move(listener));
}

}  // namespace bot
