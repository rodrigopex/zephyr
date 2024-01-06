#include "indicator.h"

ZBUS_CHAN_DEFINE(chan_indicator_command,       /* Name */
		 struct msg_indicator_command, /* Message type */
		 NULL,                         /* Validator */
		 NULL,                         /* User data */
		 ZBUS_OBSERVERS_EMPTY,         /* Observers list */
		 ZBUS_MSG_INIT(.type = INDICATOR_CMD_OFF) /* Message initialization */);

ZBUS_CHAN_DEFINE(chan_indicator_response,       /* Name */
		 struct msg_indicator_response, /* Message type */
		 NULL,                          /* Validator */
		 NULL,                          /* User data */
		 ZBUS_OBSERVERS_EMPTY,          /* Observers list */
		 ZBUS_MSG_INIT(.type = INDICATOR_RSP_STATE,
			       .state = {.is_on = false}) /* Message initialization */);

ZBUS_CHAN_DEFINE(chan_indicator_event,       /* Name */
		 struct msg_indicator_event, /* Message type */
		 NULL,                       /* Validator */
		 NULL,                       /* User data */
		 ZBUS_OBSERVERS_EMPTY,       /* Observers list */
		 ZBUS_MSG_INIT(.type = INDICATOR_EVENT_INITIADED) /* Message initialization */);

#if defined(CONFIG_SHELL)
#include <stdlib.h>
#include <zephyr/shell/shell.h>

static struct msg_indicator_response rsp;
static const struct zbus_channel *chan;

ZBUS_MSG_SUBSCRIBER_DEFINE(msub_shell_indicator);

ZBUS_CHAN_ADD_OBS(chan_indicator_response, msub_shell_indicator, 3);

static int pulse_duration_get_cmd_handler(const struct shell *sh, size_t argc, char **argv,
					  void *data)
{
	int err;

	err = zbus_chan_pub(&chan_indicator_command,
			    MSG_INDICATOR_CMD(.type = INDICATOR_CMD_GET_PULSE_DURATION),
			    K_MSEC(500));
	if (err) {
		return err;
	}

	err = zbus_sub_wait_msg(&msub_shell_indicator, &chan, &rsp, K_MSEC(500));
	if (err) {
		return err;
	}

	shell_print(sh, "Pulse duration: %d", rsp.pulse.duration);

	return 0;
}

static int pulse_duration_set_cmd_handler(const struct shell *sh, size_t argc, char **argv,
					  void *data)
{
	int err;
	int pulse_duration = atoi(argv[1]);

	err = zbus_chan_pub(&chan_indicator_command,
			    MSG_INDICATOR_CMD(.type = INDICATOR_CMD_SET_PULSE_DURATION,
					      .pulse = {.duration = pulse_duration}),
			    K_MSEC(500));
	if (err) {
		return err;
	}

	err = zbus_sub_wait_msg(&msub_shell_indicator, &chan, &rsp, K_MSEC(500));
	if (err) {
		return err;
	}

	return pulse_duration != rsp.pulse.duration;
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
			     MSG_INDICATOR_CMD(.type = (enum indicator_command_type)data),
			     K_MSEC(500));
}

SHELL_SUBCMD_DICT_SET_CREATE(sub_indicator_action, indicator_cmd_handler,
			     (on, INDICATOR_CMD_ON, "on"), (off, INDICATOR_CMD_OFF, "off"),
			     (toggle, INDICATOR_CMD_TOGGLE, "toggle"),
			     (pulse, INDICATOR_CMD_PULSE, "pulse"));

SHELL_CMD_REGISTER(indicator, &sub_indicator_action, "Indicator action", NULL);

static int indicator_event_cmd_handler(const struct shell *sh, size_t argc, char **argv, void *data)
{
	return zbus_chan_pub(&chan_indicator_event,
			     MSG_INDICATOR_EVT(.type = (enum indicator_event_type)data),
			     K_MSEC(500));
}

SHELL_SUBCMD_DICT_SET_CREATE(sub_indicator_event, indicator_event_cmd_handler,
			     (initiated, INDICATOR_EVENT_INITIADED, "initiated"),
			     (ready, INDICATOR_EVENT_READY, "ready"),
			     (failed, INDICATOR_EVENT_FAILED, "failed"));

SHELL_CMD_REGISTER(indicator_event, &sub_indicator_event, "Indicator event", NULL);

#endif /* CONFIG_SHELL */
