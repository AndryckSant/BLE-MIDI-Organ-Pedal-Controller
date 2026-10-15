/**
 * @file ble_connected.h
 * @brief BLE-connected state entry interface.
 */

#ifndef BLE_CONNECTED_STATE_H
#define BLE_CONNECTED_STATE_H

#include <stdint.h>

#include "state_actions.h"

void ble_connected_enter(const StateActions &actions, uint8_t midiChannel, uint8_t midiCc,
                         uint8_t midiValue);

#endif // BLE_CONNECTED_STATE_H