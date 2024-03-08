#include <zephyr/zbus/zbus.h>
#include "indicator_messages.pb.h"

ZBUS_CHAN_DECLARE(chan_indicator_command);

ZBUS_CHAN_DECLARE(chan_indicator_response);

ZBUS_CHAN_DECLARE(chan_indicator_event);

// struct indicator_pulse {
// 	int duration;
// };
//
// enum indicator_command_type {
// 	INDICATOR_CMD_OFF,
// 	INDICATOR_CMD_ON,
// 	INDICATOR_CMD_TOGGLE,
// 	INDICATOR_CMD_PULSE,
// 	INDICATOR_CMD_GET_PULSE_DURATION,
// 	INDICATOR_CMD_SET_PULSE_DURATION,
// };

// /**
//  * @brief The message indicator command struct
//  */
// struct msg_indicator_command {
// 	/** The command type field. */
// 	enum indicator_command_type type;
// 	/** The content type field which will depend on the type field. */
// 	union {
// 		/** Content of INDICATOR_CMD_SET_PULSE_DURATION type command */
// 		struct indicator_pulse pulse;
// 	};
// };

/* Compound literals constructors for the messages
 * For reference: https://gcc.gnu.org/onlinedocs/gcc/Compound-Literals.html */

#define MSG_INDICATOR_CMD(_val, ...) ((struct msg_indicator_command[]){{_val, ##__VA_ARGS__}})

// struct indicator_state {
// 	bool is_on;
// };
//
// enum indicator_response_type {
// 	INDICATOR_RSP_STATE,
// 	INDICATOR_RSP_PULSE_DURATION,
// };
//
// /**
//  * @brief The message indicator response struct
//  */
// struct msg_indicator_response {
// 	/** The response type field. */
// 	enum indicator_response_type type;
// 	/** The content type field which will depend on the type field. */
// 	union {
// 		/** Content of INDICATOR_RSP_STATE type response */
// 		struct indicator_state state;
// 		/** Content of INDICATOR_RSP_PULSE_DURATION type response */
// 		struct indicator_pulse pulse;
// 	};
// };

#define MSG_INDICATOR_RSP(_val, ...) ((struct msg_indicator_response[]){{_val, ##__VA_ARGS__}})

// enum indicator_event_type {
// 	INDICATOR_EVENT_READY,
// 	INDICATOR_EVENT_INITIADED,
// 	INDICATOR_EVENT_FAILED,
// };
//
// struct indicator_init_error {
// 	int code;
// };
//
// struct msg_indicator_event {
// 	enum indicator_event_type type;
// 	union {
// 		/** Content of INDICATOR_EVENT_FAILED type event */
// 		struct indicator_init_error error;
// 	};
// };

#define MSG_INDICATOR_EVT(_val, ...) ((struct msg_indicator_event[]){{_val, ##__VA_ARGS__}})
