/******************************************************************************
 * @file    midi_service.h
 * @author  Andryck Santiago
 * @brief   MIDI service public interface.
 *
 * @details
 * Provides high-level MIDI message generation and transmission.
 *
 * This service abstracts the underlying MIDI transport and prevents
 * application services from depending directly on the BLE MIDI driver.
 ******************************************************************************/

#ifndef MIDI_SERVICE_H
#define MIDI_SERVICE_H

#include <stdint.h>

#include "../../models/midi_model.h"

/**
 * @brief Initializes the MIDI service.
 */
void midi_init();

/**
 * @brief Sends a MIDI Control Change message.
 *
 * @param channel MIDI channel (1-16).
 * @param cc MIDI Control Change number.
 * @param value MIDI value (0-127).
 */
void midi_send_control_change(uint8_t channel,
                              uint8_t cc,
                              uint8_t value);

/**
 * @brief Sends a MIDI message.
 *
 * @param message MIDI message to send.
 */
void midi_send(const MidiMessage *message);

#endif // MIDI_SERVICE_H