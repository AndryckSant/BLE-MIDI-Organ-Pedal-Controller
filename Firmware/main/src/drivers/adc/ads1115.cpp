/******************************************************************************
 * @file    ads1115.cpp
 * @author  Andryck Santiago
 * @brief   ADS1115 ADC Driver.
 *
 * @details
 * This module encapsulates the Adafruit ADS1115 library and provides
 * functions for initializing the ADC and acquiring analog measurements.
 *
 * This module is responsible only for hardware-level ADC communication.
 * Signal filtering, calibration, normalization, and application logic
 * are handled by higher layers of the application.
 ******************************************************************************/

#include "ads1115.h"

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_ADS1X15.h>

namespace {
    //=========================================================================
    // Private Constants
    //=========================================================================

    /// I2C SDA pin connected to the ADS1115.
	constexpr uint8_t kI2cSda = 21;

	/// I2C SCL pin connected to the ADS1115.
	constexpr uint8_t kI2cScl = 22;

	/// ADS1115 default I2C address.
    constexpr uint8_t kAds1115Address = 0x48;

	//=========================================================================
    // Private Variables
    //=========================================================================

    Adafruit_ADS1115 ads;

    bool isInitialized = false;

} // namespace

//=============================================================================
// Public Functions
//=============================================================================


ADS1115_Status_t ads1115_init(void){
	if (isInitialized) {
		return ADS1115_OK;
	}

	Wire.begin(kI2cSda, kI2cScl);
	ads.setGain(GAIN_ONE);

	if(!ads.begin(kAds1115Address, &Wire)){
		return ADS1115_ERROR;
	}
	isInitialized = true;
	return ADS1115_OK;
}

int16_t ads1115_read_channel(uint8_t channel){
	if (!isInitialized) {
		return 0.0f;
	}
	if(channel > 3){
		return 0;
	}

	return ads.readADC_SingleEnded(channel);
}

float ads1115_read_voltage(uint8_t channel)
{
    if(!isInitialized)
    {
        return 0.0f;
    }

    if(channel > 3)
    {
        return 0.0f;
    }

    return ads.computeVolts(ads.readADC_SingleEnded(channel));
}

bool ads1115_is_ready()
{
    return isInitialized;
}