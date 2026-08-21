/******************************************************************************
 * @file    pedal_service.h
 * @author  Andryck Santiago
 * @brief   Pedal input service public interface.
 *
 * @details
 * Provides functions for initializing and processing the configurable
 * expression pedals.
 *
 * This service handles ADC acquisition, filtering, calibration,
 * normalization, and MIDI value generation.
 ******************************************************************************/

#ifndef PEDAL_SERVICE_H
#define PEDAL_SERVICE_H

#include <stdbool.h>
#include <stdint.h>

#include "../../../models/pedal_model.h"

/**
 * @brief Initializes the pedal service.
 *
 * @param pedals Array containing the pedal configurations.
 * @param count Number of pedals.
 */
void pedal_init(PedalModel *pedals,
                uint8_t count);

/**
 * @brief Processes all configured pedals.
 *
 * Reads ADC values, applies filtering and calibration,
 * and updates the corresponding pedal models.
 */
void pedal_process();

/**
 * @brief Returns whether a pedal generated a new MIDI value.
 *
 * @param index Pedal index.
 *
 * @return true if the MIDI value changed enough to be transmitted.
 * @return false otherwise.
 */
bool pedal_has_new_value(uint8_t index);

#endif // PEDAL_SERVICE_H