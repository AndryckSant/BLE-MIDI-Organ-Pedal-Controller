/******************************************************************************
 *
 * @file    pedal_service.cpp
 * @author  Andryck Santiago
 * @brief   Pedal input service implementation.
 *
 * @details
 * This module implements the processing logic for configurable expression
 * pedals.
 *
 * The service acquires ADC values from the ADS1115, applies smoothing,
 * updates calibration data, normalizes the input, and converts the result
 * to a MIDI Control Change value.
 *
 * MIDI communication is handled separately by the MIDI service.
 *
 ******************************************************************************/

#include "pedal_service.h"

#include <Arduino.h>

#include "../../../drivers/adc/ads1115.h"
#include "../../calibration_service/calibration_service.h"

namespace
{
    //=============================================================================
    // Private Constants
    //=============================================================================

    /**
     * @brief Smoothing divisor used by the pedal filter.
     */
    constexpr uint8_t kSmoothingDivisor = 12U;

    /**
     * @brief Minimum MIDI value difference required to generate a new value.
     */
    constexpr uint8_t kMidiDeadBand = 2U;

    /**
     * @brief Maximum MIDI value.
     */
    constexpr uint8_t kMidiMaximum = 127U;

    /**
     * @brief Number of supported pedals.
     */
    constexpr uint8_t kPedalCount = 2U;

    //=============================================================================
    // Private Types
    //=============================================================================

    //=============================================================================
    // Private Variables
    //=============================================================================

    /**
     * @brief Pointer to the array containing the pedal models.
     */
    PedalModel *pedals = nullptr;

    /**
     * @brief Number of configured pedals.
     */
    uint8_t pedalCount = 0U;

    /**
     * @brief Indicates whether the pedal service has been initialized.
     */
    bool initialized = false;



    //=============================================================================
    // Private Functions
    //=============================================================================

    int applySmoothing(int previous,
                    int current)
    {
        return previous +
            ((current - previous) /
                kSmoothingDivisor);
    }

    uint8_t calculateMidiValue(float normalized)
    {
        if(normalized <= 0.0f){
            return 0U;
        }

        if(normalized >= 1.0f){
            return kMidiMaximum;
        }

        const float midiValue =
            normalized *
            static_cast<float>(kMidiMaximum);

        return static_cast<uint8_t>(midiValue + 0.5f);
    }

} // namespace

//=============================================================================
// Public Functions
//=============================================================================

void pedal_init(PedalModel *pedalsArray,
                uint8_t count)
{
    pedals = nullptr;
    pedalCount = 0U;
    initialized = false;

    if(pedalsArray == nullptr){
        return;
    }

    if(count == 0U){
        return;
    }

    pedals = pedalsArray;

    pedalCount = count;

    if(pedalCount > kPedalCount){
        pedalCount = kPedalCount;
    }

    for(uint8_t index = 0U; index < pedalCount; ++index){
        PedalModel &pedal = pedals[index];

        pedal.raw_value = 0;
        pedal.filtered_value = 0;
        pedal.midi_value = 0U;
        pedal.last_midi_value = 255U;

        if(!pedal.enabled){
            continue;
        }

        if(!pedal.calibration.initialized){
            calibration_init(&pedal.calibration);
        }
    }

    initialized = true;
}

void pedal_process()
{
    if(!initialized){
        return;
    }

    if(pedals == nullptr){
        return;
    }

    for(uint8_t index = 0U; index < pedalCount; ++index){
        PedalModel &pedal = pedals[index];

        if(!pedal.enabled){
            continue;
        }

        /*
         * Acquire the current ADC value.
         */
        const int16_t rawValue =
            ads1115_read_channel(pedal.adc_channel);

        pedal.raw_value = rawValue;

        /*
         * Apply simple exponential-style smoothing.
         */
        pedal.filtered_value =
            applySmoothing(
                pedal.filtered_value,
                pedal.raw_value);

        /*
         * Update the automatic calibration range.
         */
        calibration_update(
            &pedal.calibration,
            static_cast<int16_t>(pedal.filtered_value));

        /*
         * Normalize the filtered value using the current
         * calibration data.
         */
        const float normalized =
            calibration_normalize(
                &pedal.calibration,
                static_cast<int16_t>(pedal.filtered_value));

        /*
         * Convert the normalized value to MIDI range.
         */
        pedal.midi_value =
            calculateMidiValue(normalized);
    }
}

bool pedal_has_new_value(uint8_t index)
{
    if(!initialized){
        return false;
    }

    if(pedals == nullptr){
        return false;
    }

    if(index >= pedalCount){
        return false;
    }

    const PedalModel &pedal = pedals[index];

    if(!pedal.enabled){
        return false;
    }

    const int difference =
        abs(
            static_cast<int>(pedal.midi_value) -
            static_cast<int>(pedal.last_midi_value));

    if(difference < kMidiDeadBand){
        return false;
    }

    return true;
}