/**
 * @file ble_disconnected.cpp
 * @brief BLE-disconnected state entry actions.
 */

#include "ble_disconnected.h"

void ble_disconnected_enter(const StateActions &actions) {
    if (actions.setLed != nullptr) {
        actions.setLed(false);
    }
    if (actions.logMessage != nullptr) {
        actions.logMessage("BLE Client disconnected.");
    }
    if (actions.showStatus != nullptr) {
        actions.showStatus(false, 0);
    }
}