/******************************************************************************
 * @file    calibration_data.h
 * @author  Andryck Santiago
 * @brief   Calibration Data Model.
 *
 * @details
 * Defines the data structure used to store calibration parameters
 * and calibration state for an analog input.
 *
 * This model contains only calibration data and does not implement
 * calibration logic.
 ******************************************************************************/

#ifndef CALIBRATION_MODEL_H
#define CALIBRATION_MODEL_H

/**
 * @brief Stores calibration parameters and state.
 */
struct CalibrationData {
    float minimum; // Minimum observed value
    float maximum; // Maximum observed value
    bool initialized; // Flag to indicate if calibration has been initialized
};

#endif // CALIBRATION_MODEL_H