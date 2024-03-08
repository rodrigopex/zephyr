#include "indicator.h"

ZBUS_CHAN_DEFINE(chan_indicator_command,       /* Name */
		 struct msg_indicator_command, /* Message type */
		 NULL,                         /* Validator */
		 NULL,                         /* User data */
		 ZBUS_OBSERVERS_EMPTY,         /* Observers list */
		 ZBUS_MSG_INIT(0) /* Message initialization */);

ZBUS_CHAN_DEFINE(chan_indicator_response,       /* Name */
		 struct msg_indicator_response, /* Message type */
		 NULL,                          /* Validator */
		 NULL,                          /* User data */
		 ZBUS_OBSERVERS_EMPTY,          /* Observers list */
		 ZBUS_MSG_INIT(0) /* Message initialization */);

ZBUS_CHAN_DEFINE(chan_indicator_event,       /* Name */
		 struct msg_indicator_event, /* Message type */
		 NULL,                       /* Validator */
		 NULL,                       /* User data */
		 ZBUS_OBSERVERS_EMPTY,       /* Observers list */
		 ZBUS_MSG_INIT(0) /* Message initialization */);

#if defined(CONFIG_SHELL)
#include <stdlib.h>
#include <zephyr/shell/shell.h>

static struct msg_indicator_response rsp = MSG_INDICATOR_RESPONSE_INIT_DEFAULT;
static const struct zbus_channel *chan;

ZBUS_MSG_SUBSCRIBER_DEFINE(msub_shell_indicator);

ZBUS_CHAN_ADD_OBS(chan_indicator_response, msub_shell_indicator, 3);

static int pulse_duration_get_cmd_handler(const struct shell *sh, size_t argc, char **argv,
					  void *data)
{
	int err;

	err = zbus_chan_pub(&chan_indicator_command,
			    MSG_INDICATOR_CMD(.which_indicator_cmd =
						      MSG_INDICATOR_COMMAND_GET_PULSE_CONFIG_TAG),
			    K_MSEC(500));
	if (err) {
		return err;
	}

	err = zbus_sub_wait_msg(&msub_shell_indicator, &chan, &rsp, K_MSEC(500));
	if (err) {
		return err;
	}

	shell_print(sh, "Pulse duration: %d", rsp.pulse_config.duration);

	return 0;
}

static int pulse_duration_set_cmd_handler(const struct shell *sh, size_t argc, char **argv,
					  void *data)
{
	int err;
	int pulse_duration = atoi(argv[1]);

	err = zbus_chan_pub(
		&chan_indicator_command,
		MSG_INDICATOR_CMD(.which_indicator_cmd = MSG_INDICATOR_COMMAND_SET_PULSE_CONFIG_TAG,
				  .set_pulse_config = {.duration = pulse_duration}),
		K_MSEC(500));
	if (err) {
		return err;
	}

	err = zbus_sub_wait_msg(&msub_shell_indicator, &chan, &rsp, K_MSEC(500));
	if (err) {
		return err;
	}

	return pulse_duration != rsp.pulse_config.duration;
}

SHELL_STATIC_SUBCMD_SET_CREATE(sub_indicator_pulse_duration,
			       SHELL_CMD(get_duration, NULL, "Get pulse duration.",
					 pulse_duration_get_cmd_handler),
			       SHELL_CMD_ARG(set_duration, NULL, "Set pulse duration.",
					     pulse_duration_set_cmd_handler, 2, 0),
			       SHELL_SUBCMD_SET_END);

SHELL_CMD_REGISTER(indicator_pulse, &sub_indicator_pulse_duration, "Indicator settings", NULL);

static int indicator_cmd_handler(const struct shell *sh, size_t argc, char **argv, void *data)
{
	return zbus_chan_pub(&chan_indicator_command,
			     MSG_INDICATOR_CMD(.which_indicator_cmd = (int)data), K_MSEC(500));
}

SHELL_SUBCMD_DICT_SET_CREATE(sub_indicator_action, indicator_cmd_handler,
			     (on, MSG_INDICATOR_COMMAND_ON_TAG, "on"),
			     (off, MSG_INDICATOR_COMMAND_OFF_TAG, "off"),
			     (toggle, MSG_INDICATOR_COMMAND_TOGGLE_TAG, "toggle"),
			     (pulse, MSG_INDICATOR_COMMAND_PULSE_TAG, "pulse"));

SHELL_CMD_REGISTER(indicator, &sub_indicator_action, "Indicator action", NULL);

static int indicator_event_cmd_handler(const struct shell *sh, size_t argc, char **argv, void *data)
{
	return zbus_chan_pub(&chan_indicator_event,
			     MSG_INDICATOR_EVT(.which_indicator_evt = (int)data), K_MSEC(500));
}

SHELL_SUBCMD_DICT_SET_CREATE(sub_indicator_event, indicator_event_cmd_handler,
			     (initiated, MSG_INDICATOR_EVENT_INITIATED_TAG, "initiated"),
			     (ready, MSG_INDICATOR_EVENT_READY_TAG, "ready"),
			     (failed, MSG_INDICATOR_EVENT_FAILED_TAG, "failed"));

SHELL_CMD_REGISTER(indicator_event, &sub_indicator_event, "Indicator event", NULL);

#endif /* CONFIG_SHELL */
