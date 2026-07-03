#ifndef PEDAL_SERVICE_H
#define PEDAL_SERVICE_H

#include <stdint.h>

#include "../../../models/pedal_model.h"

using PedalMidiSendFn = void (*)(uint8_t channel, uint8_t cc, uint8_t value);

void pedal_service_init(PedalModel &pedal, uint8_t ads_channel, uint8_t midi_channel, uint8_t midi_cc);
bool pedal_service_process(PedalModel &pedal, int raw_value, bool is_connected, PedalMidiSendFn send_fn = nullptr);
uint8_t pedal_service_get_midi_value(const PedalModel &pedal);

#endif // PEDAL_SERVICE_H