#include <zephyr/zbus/zbus.h>
#include "indicator.pb.h"

ZBUS_CHAN_DECLARE(chan_indicator_cmd);

ZBUS_CHAN_DECLARE(chan_indicator_rsp);

ZBUS_CHAN_DECLARE(chan_indicator_evt);

/* Compound literals constructors for the messages
 * For reference: https://gcc.gnu.org/onlinedocs/gcc/Compound-Literals.html */

#define MSG_INDICATOR_CMD(_val, ...) ((struct msg_indicator_cmd[]){{_val, ##__VA_ARGS__}})

#define MSG_INDICATOR_RSP(_val, ...) ((struct msg_indicator_rsp[]){{_val, ##__VA_ARGS__}})

#define MSG_INDICATOR_EVT(_val, ...) ((struct msg_indicator_evt[]){{_val, ##__VA_ARGS__}})
