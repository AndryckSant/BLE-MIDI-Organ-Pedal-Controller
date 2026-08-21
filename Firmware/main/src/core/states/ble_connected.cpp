#include "ble_connected.h"

void ble_connected_enter(const StateActions &actions, uint8_t midi_channel, uint8_t midi_cc, uint8_t midi_value) {
    if (actions.set_led != nullptr) {
        actions.set_led(true);
    }
    if (actions.log_message != nullptr) {
        actions.log_message("BLE Client connected!");
    }
    if (actions.send_control_change != nullptr) {
        actions.send_control_change(midi_channel, midi_cc, midi_value);
    }
    if (actions.show_status != nullptr) {
        actions.show_status(true, midi_value);
    }
}