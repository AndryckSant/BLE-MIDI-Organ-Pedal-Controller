#ifndef STATE_ACTIONS_H
#define STATE_ACTIONS_H

#include <stdint.h>

struct StateActions {
    void (*set_led)(bool on);
    void (*show_status)(bool connected, uint8_t cc_value);
    void (*log_message)(const char *message);
    void (*send_control_change)(uint8_t channel, uint8_t cc, uint8_t value);
    void (*display_show_config)(const char *message);
};

#endif // STATE_ACTIONS_H