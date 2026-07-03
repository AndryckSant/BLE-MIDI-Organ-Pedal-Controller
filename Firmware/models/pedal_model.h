#ifndef PEDAL_MODEL_H
#define PEDAL_MODEL_H

#ifndef UINT8_MAX
typedef unsigned char uint8_t;
#endif

struct PedalModel {
    bool enabled;            // Whether the pedal is enabled
    uint8_t adc_channel;      // ADC channel the pedal is connected to
    uint8_t midi_channel;     // MIDI channel for the pedal
    uint8_t midi_cc;          // MIDI Control Change number for the pedal
    int raw_value;          // Raw ADC value from the pedal
    int filtered_value;     // Filtered value after smoothing
    float calibration_min;    // Minimum observed value for calibration
    float calibration_max;    // Maximum observed value for calibration
    bool calibration_initialized; // Flag to indicate if calibration has been initialized
    uint8_t midi_value;        // MIDI value (0-127) to send
    uint8_t last_midi_value;   // Last MIDI value sent to avoid redundant messages
};

#endif // PEDAL_MODEL_H