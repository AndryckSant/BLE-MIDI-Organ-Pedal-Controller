/******************************************************************************
 *
 * @file    button_service.cpp
 * @author  Andryck Santiago
 * @brief   Button input service implementation.
 *
 * @details
 * This module implements the processing logic for configurable physical
 * buttons.
 *
 * The service configures the button GPIOs, reads their states, applies
 * the configured input polarity, and updates the corresponding button
 * models.
 *
 * The service also allows the application to detect button press events.
 *
 * MIDI generation is handled separately by the MIDI service.
 *
 ******************************************************************************/

#include "button_service.h"

#include <Arduino.h>

#include "../../../models/system_config.h"
#include "../../../board_pins.h"

namespace
{
//=============================================================================
// Private Constants
//=============================================================================

/**
 * @brief GPIO input mode used by the button service.
 */
constexpr uint8_t kButtonInputMode = INPUT;
constexpr unsigned long kButtonDebounceMs = 40UL;

//=============================================================================
// Private Types
//=============================================================================

//=============================================================================
// Private Variables
//=============================================================================

/**
 * @brief Pointer to the array containing the button models.
 */
ButtonModel *buttons = nullptr;

/**
 * @brief Number of configured buttons.
 */
uint8_t buttonCount = 0U;

/**
 * @brief Indicates whether the button service has been initialized.
 */
bool initialized = false;

bool rawStates[kMaxButtons] = {};
unsigned long lastRawChangeTimes[kMaxButtons] = {};

//=============================================================================
// Private Functions
//=============================================================================

bool readButtonState(const ButtonModel &button)
{
    bool state = digitalRead(button.gpio_pin) == HIGH;

    if(button.inverted){
        state = !state;
    }

    return state;
}

} // namespace

//=============================================================================
// Public Functions
//=============================================================================

void button_init(ButtonModel *buttonsArray,
                 uint8_t count)
{
    /*
     * Reset the service state before initialization.
     */
    buttons = nullptr;
    buttonCount = 0U;
    initialized = false;

    if(buttonsArray == nullptr){
        return;
    }

    if(count == 0U){
        return;
    }

    buttons = buttonsArray;
    buttonCount = count > kMaxButtons ? kMaxButtons : count;

    for(uint8_t index = 0U; index < buttonCount; ++index){
        ButtonModel &button = buttons[index];
        button.latched = false;

        rawStates[index] = false;
        lastRawChangeTimes[index] = millis();

        /*
         * Disabled buttons do not configure or access their GPIO.
         */
        if(!button.enabled){
            button.pressed = false;
            button.last_pressed = false;

            continue;
        }

        /*
         * Configure the GPIO as a standard digital input. 
         * 
         * The button circuit provides the required pull-down resistor
         * externally, so no internal pull resistor is enabled.
         * 
         * The inverted configuration determines whether the
         * electrical LOW or HIGH state represents a logical press.
         */
        if (!BoardPins::isMidiButton(button.gpio_pin))
        {
            button.pressed = false;
            button.last_pressed = false;
            continue;
        }

        pinMode(button.gpio_pin, kButtonInputMode);

        rawStates[index] = readButtonState(button);
        button.pressed = false;
        button.last_pressed = false;
    }

    initialized = true;
}

void button_process()
{
    if(!initialized){
        return;
    }

    if(buttons == nullptr){
        return;
    }

    for(uint8_t index = 0U; index < buttonCount; ++index){
        ButtonModel &button = buttons[index];
        button.last_pressed = button.pressed;

        /*
         * Disabled buttons remain in the released state.
         */
        if(!button.enabled){
            button.pressed = false;
            rawStates[index] = false;
            lastRawChangeTimes[index] = millis();

            continue;
        }

        if (!BoardPins::isMidiButton(button.gpio_pin))
        {
            button.pressed = false;
            continue;
        }

        const bool rawState = readButtonState(button);
        const unsigned long currentTime = millis();

        if (rawState != rawStates[index])
        {
            rawStates[index] = rawState;
            lastRawChangeTimes[index] = currentTime;
        }

        if (rawStates[index] != button.pressed &&
            currentTime - lastRawChangeTimes[index] >= kButtonDebounceMs)
        {
            button.pressed = rawStates[index];
        }
    }
}

bool button_was_pressed(uint8_t index)
{
    if(!initialized){
        return false;
    }

    if(buttons == nullptr){
        return false;
    }

    if(index >= buttonCount){
        return false;
    }

    const ButtonModel &button = buttons[index];

    /*
     * A press event occurs only when the logical button state
     * changes from released to pressed.
     */
    return button.pressed && !button.last_pressed;
}

bool button_was_released(uint8_t index)
{
    if (!initialized || buttons == nullptr || index >= buttonCount)
    {
        return false;
    }

    const ButtonModel &button = buttons[index];
    return !button.pressed && button.last_pressed;
}