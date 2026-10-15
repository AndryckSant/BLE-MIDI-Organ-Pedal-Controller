/******************************************************************************
 * @file    pedal_model.h
 * @author  Andryck Santiago
 * @brief   Pedal Model.
 *
 * @details
 * Defines the data structure used to represent a configurable pedal,
 * including hardware configuration, MIDI configuration, calibration
 * data, and runtime state.
 *
 * This model contains only pedal data and does not implement input
 * processing, calibration, filtering, or MIDI communication logic.
 ******************************************************************************/

#ifndef PEDAL_MODEL_H
#define PEDAL_MODEL_H

#include <stdint.h>

#include "calibration_data.h"

/**
 * @brief Represents the configuration and runtime state of a pedal.
 */
struct PedalModel {
    // Configuration
    bool enabled;        // Whether the pedal is enabled
    uint8_t adcChannel;  // ADC channel the pedal is connected to
    uint8_t midiChannel; // MIDI channel for the pedal
    uint8_t midiCc;      // MIDI Control Change number for the pedal

    // Runtime state
    int rawValue;                // Raw ADC value from the pedal
    int filteredValue;           // Filtered value after smoothing
    CalibrationData calibration; // Calibration data for the pedal
    uint8_t midiValue;           // MIDI value (0-127) to send
    uint8_t lastMidiValue;       // Last MIDI value sent to avoid redundant messages
};

#endif // PEDAL_MODEL_H