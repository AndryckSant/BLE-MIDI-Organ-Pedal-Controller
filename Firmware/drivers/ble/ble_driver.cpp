/******************************************************************************
 * @file    ble_driver.cpp
 * @author  Andryck Santiago
 * @brief   BLE MIDI Driver.
 *
 * @details
 * This module encapsulates the BLEMidi library and provides a simple
 * interface for initializing the BLE MIDI service, monitoring the BLE
 * connection state, and sending MIDI Control Change messages.
 *
 * Responsibilities:
 *  - Initialize the BLE MIDI service.
 *  - Monitor the BLE connection status.
 *  - Send BLE MIDI Control Change messages.
 *
 * This module does not implement application logic and has no dependency
 * on the application's Finite State Machine (FSM) or Services layer.
 ******************************************************************************/

#include "ble_driver.h"

#include <BLEMidi.h>
#include <Arduino.h>

namespace {
    //=========================================================================
    // Private Constants
    //=========================================================================
    
    /// BLE device advertised name.
    constexpr char kBleName[] = "BLE MIDI Controller";

    //=========================================================================
    // Private Types
    //=========================================================================

    struct BleState
    {
        bool isConnected = false;
    };

    //=========================================================================
    // Private Variables
    //=========================================================================

    BleState bleState;

    //=========================================================================
    // Private Function Prototypes
    //=========================================================================

    void onConnect();
    void onDisconnect();
}

//=============================================================================
// Public Functions
//=============================================================================

void ble_init() {
    BLEMidiServer.begin(kBleName);

#ifdef BLE_DEBUG
    BLEMidiServer.enableDebugging();
#endif

    BLEMidiServer.setOnConnectCallback(onConnect);
    BLEMidiServer.setOnDisconnectCallback(onDisconnect);
}

bool ble_is_connected() {
    return bleState.isConnected;
}

void ble_send_control_change(uint8_t channel, uint8_t cc, uint8_t value) {
    if(!bleState.isConnected){
        return;
    }

    BLEMidiServer.controlChange(channel, cc, value);
}

//=============================================================================
// Private Functions
//=============================================================================

/**
 * @brief BLE connection callback.
 *
 * Invoked automatically when a BLE client establishes a connection.
 */
void onConnect() {
    bleState.isConnected = true;
}

/**
 * @brief BLE disconnection callback.
 *
 * Invoked automatically when the BLE client disconnects.
 */
void onDisconnect() {
    bleState.isConnected = false;
}
