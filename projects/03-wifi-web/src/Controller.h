#pragma once

#include <Button.h>

#include "Led.h"

class Controller {
public:
    Controller(bot::Button& bootButton, Led& led);

private:
    void onBootButton(bool pressed);

    Led& led_;
};
