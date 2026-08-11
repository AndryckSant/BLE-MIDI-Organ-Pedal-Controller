#ifndef BLE_DRIVER_H
#define BLE_DRIVER_H

/******************************************************************************
 * @file    ble_driver.h
 * @author  Andryck Santiago
 * @brief   BLE MIDI Driver public interface.
 *
 * @details
 * Provides the public API for initializing the BLE MIDI driver,
 * querying the BLE connection state, and sending MIDI Control
 * Change messages.
 ******************************************************************************/

#include <stdint.h>

/**
 * @brief Initializes the BLE MIDI driver.
 */
void ble_init();

/**
 * @brief Returns the BLE connection status.
 *
 * @return true if a BLE client is connected.
 * @return false otherwise.
 */
bool ble_is_connected();

/**
 * @brief Sends a MIDI Control Change message.
 *
 * If no BLE client is connected, the message is ignored.
 *
 * @param channel MIDI channel (1-16).
 * @param cc MIDI Control Change number.
 * @param value MIDI Control Change value (0-127).
 */
void ble_send_control_change(uint8_t channel,
                             uint8_t cc,
                             uint8_t value);

#endif // BLE_DRIVER_H