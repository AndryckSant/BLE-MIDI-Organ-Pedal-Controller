#ifndef BLE_CONNECTED_STATE_H
#define BLE_CONNECTED_STATE_H

#include <stdint.h>

#include "state_actions.h"

void ble_connected_enter(const StateActions &actions, uint8_t midi_channel, uint8_t midi_cc, uint8_t midi_value);

#endif // BLE_CONNECTED_STATE_H