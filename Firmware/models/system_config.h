#ifndef SYSTEM_CONFIG_H
#define SYSTEM_CONFIG_H

#include "button_model.h"
#include "pedal_model.h"

struct SystemConfig {
    PedalModel pedals[2]; // Configuration for Pedal 1 and Pedal 2
    ButtonModel buttons[8]; // Configuration for up to 8 buttons
};

#endif // SYSTEM_CONFIG_H