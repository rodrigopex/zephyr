#include <zephyr/zbus/zbus.h>
#include "trigger.pb.h"

ZBUS_CHAN_DECLARE(chan_trigger_evt);

/* Compound literals constructors for the messages */
#define MSG_TRIGGER_EVT(_val, ...) ((struct msg_trigger_evt[]){{_val, ##__VA_ARGS__}})
