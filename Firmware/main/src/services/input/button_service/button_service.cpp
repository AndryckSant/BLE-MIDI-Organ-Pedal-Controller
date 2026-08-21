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

namespace
{
//=============================================================================
// Private Constants
//=============================================================================

/**
 * @brief GPIO input mode used by the button service.
 */
constexpr uint8_t kButtonInputMode = INPUT;

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
    buttonCount = count;

    for(uint8_t index = 0U; index < buttonCount; ++index){
        ButtonModel &button = buttons[index];

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
        pinMode(button.gpio_pin, kButtonInputMode);

        /*
         * Initialize both states with the current physical state.
         *
         * This prevents a button that is already pressed during
         * initialization from generating a false press event.
         */
        bool state = readButtonState(button);

        button.pressed = state;
        button.last_pressed = state;
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

        /*
         * Disabled buttons remain in the released state.
         */
        if(!button.enabled){
            button.last_pressed = button.pressed;
            button.pressed = false;

            continue;
        }

        /*
         * Preserve the previous logical state before reading
         * the new GPIO state.
         */
        button.last_pressed = button.pressed;
        button.pressed = readButtonState(button);
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