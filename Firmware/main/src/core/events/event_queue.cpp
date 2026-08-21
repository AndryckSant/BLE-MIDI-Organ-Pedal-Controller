#include "event_queue.h"

void event_queue_init(EventQueue &queue) {
	queue.head = 0;
	queue.tail = 0;
	queue.count = 0;
}

bool event_queue_push(EventQueue &queue, FsmEvent event) {
	if (queue.count >= 8) {
		return false;
	}

	queue.items[queue.tail] = event;
	queue.tail = static_cast<uint8_t>((queue.tail + 1) % 8);
	queue.count++;
	return true;
}

bool event_queue_pop(EventQueue &queue, FsmEvent &event) {
	if (queue.count == 0) {
		return false;
	}

	event = queue.items[queue.head];
	queue.head = static_cast<uint8_t>((queue.head + 1) % 8);
	queue.count--;
	return true;
}

bool event_queue_is_empty(const EventQueue &queue) {
	return queue.count == 0;
}
