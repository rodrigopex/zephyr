#include <zephyr/zbus/zbus.h>
#include "trigger_messages.pb.h"

ZBUS_CHAN_DECLARE(chan_trigger_event);

// enum trigger_event_type {
// 	TRIGGER_EVENT_READY,
// 	TRIGGER_EVENT_INITIATED,
// 	TRIGGER_EVENT_FAILED,
// 	TRIGGER_EVENT_ACTIVATED,
// };
//
// struct trigger_init_error {
// 	int code;
// };
//
// struct msg_trigger_event {
// 	enum trigger_event_type type;
// 	union {
// 		struct trigger_init_error error;
// 	};
// };

/* Compound literals constructors for the messages */
#define MSG_TRIGGER_EVT(_val, ...) ((struct msg_trigger_event[]){{_val, ##__VA_ARGS__}})
