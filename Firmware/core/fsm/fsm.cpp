#include "fsm.h"

namespace {

void handle_event(FsmContext &context, FsmEvent event) {
	switch (context.state) {
	case FsmState::Boot:
		if (event == FsmEvent::BootOk) {
			context.state = FsmState::BleDisconnected;
		} else if (event == FsmEvent::BootFail || event == FsmEvent::RuntimeFault) {
			context.state = FsmState::Error;
		}
		break;

	case FsmState::BleDisconnected:
		if (event == FsmEvent::BleConnected) {
			context.state = FsmState::BleConnected;
		} else if (event == FsmEvent::RuntimeFault) {
			context.state = FsmState::Error;
		}
		break;

	case FsmState::BleConnected:
		if (event == FsmEvent::StartStream) {
			context.state = FsmState::ReadAndSend;
		} else if (event == FsmEvent::BleDisconnected) {
			context.state = FsmState::BleDisconnected;
		} else if (event == FsmEvent::EnterConfig) {
			context.state = FsmState::Config;
		} else if (event == FsmEvent::RuntimeFault) {
			context.state = FsmState::Error;
		}
		break;

	case FsmState::ReadAndSend:
		if (event == FsmEvent::StopStream) {
			context.state = FsmState::BleConnected;
		} else if (event == FsmEvent::BleDisconnected) {
			context.state = FsmState::BleDisconnected;
		} else if (event == FsmEvent::EnterConfig) {
			context.state = FsmState::Config;
		} else if (event == FsmEvent::RuntimeFault) {
			context.state = FsmState::Error;
		}
		break;

	case FsmState::Config:
		if (event == FsmEvent::ExitConfig) {
			context.state = FsmState::BleDisconnected;
		} else if (event == FsmEvent::BleDisconnected) {
			context.state = FsmState::BleDisconnected;
		} else if (event == FsmEvent::RuntimeFault) {
			context.state = FsmState::Error;
		}
		break;

	case FsmState::Error:
		break;
	}
}

} // namespace

void fsm_init(FsmContext &context) {
	context.state = FsmState::Boot;
	event_queue_init(context.queue);
	fsm_post(context, FsmEvent::BootOk);
}

void fsm_post(FsmContext &context, FsmEvent event) {
	event_queue_push(context.queue, event);
}

void fsm_run(FsmContext &context) {
	FsmEvent event = FsmEvent::None;
	while (event_queue_pop(context.queue, event)) {
		handle_event(context, event);
	}
}

FsmState fsm_get_state(const FsmContext &context) {
	return context.state;
}

const char *fsm_state_name(FsmState state) {
	switch (state) {
	case FsmState::Boot:
		return "Boot";
	case FsmState::BleDisconnected:
		return "BleDisconnected";
	case FsmState::BleConnected:
		return "BleConnected";
	case FsmState::ReadAndSend:
		return "ReadAndSend";
	case FsmState::Config:
		return "Config";
	case FsmState::Error:
		return "Error";
	}

	return "Unknown";
}
