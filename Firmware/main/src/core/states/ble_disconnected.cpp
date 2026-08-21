#include "ble_disconnected.h"

void ble_disconnected_enter(const StateActions &actions) {
    if (actions.set_led != nullptr) {
        actions.set_led(false);
    }
    if (actions.log_message != nullptr) {
        actions.log_message("BLE Client disconnected.");
    }
    if (actions.show_status != nullptr) {
        actions.show_status(false, 0);
    }
}