#ifndef BLE_DRIVER_H
#define BLE_DRIVER_H

#include <stdint.h>

void ble_init();
void ble_is_connected();
void ble_send_midi(uint8_t channel, uint8_t cc, uint8_t value);

#endif // BLE_DRIVER_H