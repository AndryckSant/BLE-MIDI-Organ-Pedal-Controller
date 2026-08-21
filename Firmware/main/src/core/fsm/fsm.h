#ifndef FSM_H
#define FSM_H

#include <stdint.h>

#include "../events/event_queue.h"

enum class FsmState : uint8_t {
	Boot = 0,
	BleDisconnected,
	BleConnected,
	ReadAndSend,
	Config,
	Error,
};

struct FsmContext {
	FsmState state;
	EventQueue queue;
};

void fsm_init(FsmContext &context);
void fsm_post(FsmContext &context, FsmEvent event);
void fsm_run(FsmContext &context);
FsmState fsm_get_state(const FsmContext &context);
const char *fsm_state_name(FsmState state);

#endif // FSM_H
