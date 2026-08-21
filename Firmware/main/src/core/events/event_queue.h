#ifndef EVENT_QUEUE_H
#define EVENT_QUEUE_H

#include <stdbool.h>
#include <stdint.h>

enum class FsmEvent : uint8_t {
	None = 0,
	BootOk,
	BootFail,
	BleConnected,
	BleDisconnected,
	StartStream,
	StopStream,
	EnterConfig,
	ExitConfig,
	RuntimeFault,
};

struct EventQueue {
	FsmEvent items[8];
	uint8_t head;
	uint8_t tail;
	uint8_t count;
};

void event_queue_init(EventQueue &queue);
bool event_queue_push(EventQueue &queue, FsmEvent event);
bool event_queue_pop(EventQueue &queue, FsmEvent &event);
bool event_queue_is_empty(const EventQueue &queue);

#endif // EVENT_QUEUE_H
