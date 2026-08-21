/******************************************************************************
 * @file    ads1115.h
 * @author  Andryck Santiago
 * @brief   ADS1115 ADC Driver public interface.
 *
 * @details
 * Provides functions for initializing the ADS1115 ADC
 * converter and acquiring analog measurements.
 *
 * This driver abstracts the ADS1115 communication layer,
 * allowing the application to perform ADC readings without
 * directly accessing the hardware interface.
 *
 * @note
 * This driver currently supports only the ADS1115 device.
 ******************************************************************************/

#ifndef ADS1115_H
#define ADS1115_H

#include <stdint.h>

/**
 * @brief ADS1115 initialization status.
 */
typedef enum
{
    ADS1115_OK = 0,
    ADS1115_ERROR

} ADS1115_Status_t;


/**
 * @brief Initializes the ADS1115 ADC.
 *
 * Configures the communication interface and prepares
 * the ADC for operation.
 *
 * @return
 * ADS1115_OK    Initialization successful.
 * ADS1115_ERROR Initialization failed.
 */
ADS1115_Status_t ads1115_init(void);



/**
 * @brief Reads a single-ended ADC channel.
 *
 * @param channel
 * ADC input channel (0 to 3) for single-ended measurements.
 *
 * @return
 * Signed 16-bit raw ADC conversion value.
 */
int16_t ads1115_read_channel(uint8_t channel);



/**
 * @brief Reads voltage from an ADC channel.
 *
 * @param channel
 * ADC input channel (0 to 3).
 *
 * @return
 * Measured voltage in volts.
 */
float ads1115_read_voltage(uint8_t channel);

/**
 * @brief Returns the ADS1115 initialization status.
 *
 * @return true if the ADS1115 is initialized and ready.
 * @return false otherwise.
 */
bool ads1115_is_ready();

#endif // ADS1115_H