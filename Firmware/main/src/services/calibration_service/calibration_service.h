/******************************************************************************
 * @file    calibration_service.h
 * @author  Andryck Santiago
 * @brief   Pedal calibration service public interface.
 *
 * @details
 * Provides functions for initializing, updating, resetting, and applying
 * calibration data to analog input values.
 *
 * This service contains calibration logic and does not access hardware
 * directly.
 ******************************************************************************/

#ifndef CALIBRATION_SERVICE_H
#define CALIBRATION_SERVICE_H

#include <stdint.h>

#include "../../models/calibration_data.h"

/**
 * @brief Initializes calibration data.
 *
 * @param calibration Pointer to calibration data.
 */
void calibration_init(CalibrationData *calibration);

/**
 * @brief Updates calibration data with a new ADC value.
 *
 * @param calibration Pointer to calibration data.
 * @param value Current filtered ADC value.
 */
void calibration_update(CalibrationData* calibration, int16_t value);

/**
 * @brief Resets calibration data.
 *
 * @param calibration Pointer to calibration data.
 */
void calibration_reset(CalibrationData *calibration);

/**
 * @brief Normalizes an ADC value using calibration data.
 *
 * @param calibration Pointer to calibration data.
 * @param value ADC value to normalize.
 *
 * @return Normalized value in the range 0.0 to 1.0.
 */
float calibration_normalize(const CalibrationData *calibration, int16_t value);

#endif // CALIBRATION_SERVICE_H