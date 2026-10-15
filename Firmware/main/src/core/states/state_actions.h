/**
 * @file state_actions.h
 * @brief Callback interface supplied to FSM states.
 */

#ifndef STATE_ACTIONS_H
#define STATE_ACTIONS_H

#include <stdint.h>

struct StateActions {
    void (*setLed)(bool on);
    void (*showStatus)(bool connected, uint8_t ccValue);
    void (*logMessage)(const char *message);
    void (*sendControlChange)(uint8_t channel, uint8_t cc, uint8_t value);
    void (*displayShowConfig)(const char *message);
    void (*displayShowError)(const char *message);
};

#endif // STATE_ACTIONS_H