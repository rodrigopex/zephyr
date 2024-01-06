#include <zephyr/kernel.h>
#include <zephyr/zbus/zbus.h>

#include "ifaces/trigger.h"

struct msg_timer_trigger_settings {
	int timeout;
};

void timer_trigger_handler(struct k_timer *dummy)
{
	zbus_chan_pub(&chan_trigger_event, MSG_TRIGGER_EVT(.type = TRIGGER_EVENT_ACTIVATED),
		      K_NO_WAIT);
}

K_TIMER_DEFINE(timer_trigger, timer_trigger_handler, NULL);

void timer_trigger_timout_lis_callback(const struct zbus_channel *chan)
{
	const struct msg_timer_trigger_settings *timer_settings = zbus_chan_const_msg(chan);

	k_timer_start(&timer_trigger, K_MSEC(timer_settings->timeout),
		      K_MSEC(timer_settings->timeout));
}

ZBUS_LISTENER_DEFINE(lis_timer_trigger, timer_trigger_timout_lis_callback);

ZBUS_CHAN_DEFINE(chan_timer_trigger_settings, struct msg_timer_trigger_settings, NULL, NULL,
		 ZBUS_OBSERVERS(lis_timer_trigger), ZBUS_MSG_INIT(.timeout = 1000));

int timer_trigger_init(void)
{
	timer_trigger_timout_lis_callback(&chan_timer_trigger_settings);

	zbus_chan_pub(&chan_trigger_event, MSG_TRIGGER_EVT(.type = TRIGGER_EVENT_READY), K_NO_WAIT);

	return 0;
}
SYS_INIT(timer_trigger_init, APPLICATION, 99);

#if defined(CONFIG_SHELL)
#include <stdlib.h>
#include <zephyr/shell/shell.h>

#define ZBUS_CHAN_LOCK(_chan, _timeout, _err)                                                      \
	for (int i = 0, e = zbus_chan_claim(_chan, _timeout); e == 0 && i == 0;                    \
	     zbus_chan_finish(_chan), *(_err) = e, ++i)

static int trigger_timeout_get_cmd_handler(const struct shell *sh, size_t argc, char **argv,
					   void *data)
{
	int err;
	struct msg_timer_trigger_settings timer_settings;

	err = zbus_chan_read(&chan_timer_trigger_settings, &timer_settings, K_MSEC(500));
	if (err) {
		shell_error(sh, "Could not retrieve the trigger timeout");

		return err;
	}

	shell_print(sh, "Timer trigger timeout: %d", timer_settings.timeout);

	return 0;
}

static int trigger_timeout_set_cmd_handler(const struct shell *sh, size_t argc, char **argv,
					   void *data)
{
	int err;

	ZBUS_CHAN_LOCK(&chan_timer_trigger_settings, K_MSEC(250), &err)
	{
		struct msg_timer_trigger_settings *timer_settings =
			zbus_chan_msg(&chan_timer_trigger_settings);

		timer_settings->timeout = atoi(argv[1]);
	}

	if (err) {
		shell_error(sh, "Could not set the timeout value");

		return err;
	} else {
		zbus_chan_notify(&chan_timer_trigger_settings, K_MSEC(250));
	}

	return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(sub_trigger_timeout,
			       SHELL_CMD(get_timeout, NULL, "Get trigger timeout.",
					 trigger_timeout_get_cmd_handler),
			       SHELL_CMD_ARG(set_timeout, NULL, "Set trigger timeout",
					     trigger_timeout_set_cmd_handler, 2, 0),
			       SHELL_SUBCMD_SET_END);

SHELL_CMD_REGISTER(trigger_timer, &sub_trigger_timeout, "Trigger timer settings", NULL);
#endif /* CONFIG_SHELL */
