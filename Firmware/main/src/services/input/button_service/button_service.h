/******************************************************************************
 * @file    button_service.h
 * @author  Andryck Santiago
 * @brief   Button input service public interface.
 *
 * @details
 * Provides functions for initializing and processing the configurable
 * physical buttons.
 *
 * This service handles GPIO input processing and button state detection.
 * MIDI generation is handled by the MIDI service.
 ******************************************************************************/

#ifndef BUTTON_SERVICE_H
#define BUTTON_SERVICE_H

#include <stdbool.h>
#include <stdint.h>

#include "../../../models/button_model.h"

/**
 * @brief Initializes the button service.
 *
 * @param buttons Array containing the button configurations.
 * @param count Number of buttons.
 */
void button_init(ButtonModel *buttons,
                 uint8_t count);

/**
 * @brief Processes all configured buttons.
 *
 * Reads the GPIO states and updates the corresponding button models.
 */
void button_process();

/**
 * @brief Returns whether a button changed to the pressed state.
 *
 * @param index Button index.
 *
 * @return true if the button was pressed since the last processing cycle.
 * @return false otherwise.
 */
bool button_was_pressed(uint8_t index);

#endif // BUTTON_SERVICE_H