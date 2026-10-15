/******************************************************************************
 * @file    button_model.h
 * @author  Andryck Santiago
 * @brief   Button Model.
 *
 * @details
 * Defines the data structure used to represent a configurable button
 * and its current runtime state.
 *
 * The model contains button hardware configuration, MIDI configuration,
 * and runtime state information.
 ******************************************************************************/

#ifndef BUTTON_MODEL_H
#define BUTTON_MODEL_H

#include <stdint.h>

/**
 * @brief Represents the configuration and runtime state of a button.
 */
struct ButtonModel {
    bool enabled;        // Whether the button is enabled
    uint8_t gpioPin;     // GPIO pin the button is connected to
    bool inverted;       // Whether the button logic is inverted (active low)
    uint8_t midiChannel; // MIDI channel for the button
    uint8_t midiCc;      // MIDI Control Change number for the button
    bool latch;          // Toggle the MIDI value on each press instead of following press state

    // Runtime state
    bool latched;     // Current output state when latch mode is enabled
    bool pressed;     // Current state of the button (pressed or not)
    bool lastPressed; // Last state of the button (pressed or not)
};

#endif // BUTTON_MODEL_H