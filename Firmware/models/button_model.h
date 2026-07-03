#ifndef BUTTON_MODEL_H
#define BUTTON_MODEL_H

#include <stdint.h>

struct ButtonModel {
    bool enabled;            // Whether the button is enabled
    uint8_t gpio_pin;         // GPIO pin the button is connected to
    bool inverted;       // Whether the button logic is inverted (active low)
    uint8_t midi_channel;     // MIDI channel for the button
    uint8_t midi_cc;          // MIDI Control Change number for the button
    bool pressed;          // Current state of the button (pressed or not)
    bool last_pressed;         // Last state of the button (pressed or not)
};

#endif // BUTTON_MODEL_H