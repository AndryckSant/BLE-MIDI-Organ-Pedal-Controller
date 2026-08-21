/******************************************************************************
 *
 * @file    midi_service.cpp
 * @author  Andryck Santiago
 * @brief   MIDI service implementation.
 *
 * @details
 * This module implements high-level MIDI message generation and transmission.
 *
 * The MIDI service provides an abstraction between the application and
 * the underlying BLE MIDI driver.
 *
 ******************************************************************************/

#include "midi_service.h"

#include "../../drivers/ble/ble_driver.h"

namespace
{
    //=============================================================================
    // Private Constants
    //=============================================================================

    constexpr uint8_t kMidiChannelMinimum = 1U;
    constexpr uint8_t kMidiChannelMaximum = 16U;

    constexpr uint8_t kMidiCcMaximum = 127U;

    //=============================================================================
    // Private Types
    //=============================================================================

    //=============================================================================
    // Private Variables
    //=============================================================================

    bool initialized = false;

    //=============================================================================
    // Private Functions
    //=============================================================================

    void sendControlChange(uint8_t channel,
                        uint8_t cc,
                        uint8_t value)
    {
        ble_send_control_change(
            channel,
            cc,
            value);
    }

} // namespace

//=============================================================================
// Public Functions
//=============================================================================

void midi_init()
{
    ble_init();

    initialized = true;
}

void midi_send_control_change(uint8_t channel,
                              uint8_t cc,
                              uint8_t value)
{
    if(!initialized){
        return;
    }

    if(channel < kMidiChannelMinimum ||
       channel > kMidiChannelMaximum){
        return;
    }

    if(cc > kMidiCcMaximum){
        return;
    }

    if(value > kMidiCcMaximum){
        return;
    }

    sendControlChange(channel, cc, value);
}

void midi_send(const MidiMessage *message)
{
    if(!initialized){
        return;
    }

    if(message == nullptr){
        return;
    }

    switch(message->type){
        case MidiType::ControlChange:
            midi_send_control_change(
                message->channel,
                message->number,
                message->value);
            break;

        default:
            break;
    }
}