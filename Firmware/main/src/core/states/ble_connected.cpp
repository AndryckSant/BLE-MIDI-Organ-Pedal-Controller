/**
 * @file ble_connected.cpp
 * @brief BLE-connected state entry actions.
 */

#include "ble_connected.h"

void ble_connected_enter(const StateActions &actions, uint8_t midiChannel, uint8_t midiCc,
                         uint8_t midiValue) {
    if (actions.setLed != nullptr) {
        actions.setLed(true);
    }
    if (actions.logMessage != nullptr) {
        actions.logMessage("BLE Client connected!");
    }
    if (actions.sendControlChange != nullptr) {
        actions.sendControlChange(midiChannel, midiCc, midiValue);
    }
    if (actions.showStatus != nullptr) {
        actions.showStatus(true, midiValue);
    }
}