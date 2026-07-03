#ifndef CALIBRATION_MODEL_H
#define CALIBRATION_MODEL_H

struct CalibrationData {
    float minimum; // Minimum observed value
    float maximum; // Maximum observed value
    bool initialized; // Flag to indicate if calibration has been initialized
};

#endif // CALIBRATION_MODEL_H