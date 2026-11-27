#include "Controller.h"

Controller::Controller(bot::Button& bootButton, Led& led) : led_(led) {
    bootButton.addListener([this](bool pressed) { onBootButton(pressed); });
}

void Controller::onBootButton(bool pressed) {
    if (!pressed) {
        return;
    }
    led_.setMode(Led::nextMode(led_.mode()));
}
