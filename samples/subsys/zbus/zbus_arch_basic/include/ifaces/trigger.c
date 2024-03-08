#include "trigger.h"

ZBUS_CHAN_DEFINE(chan_trigger_event,       /* Name */
		 struct msg_trigger_event, /* Message type */
		 NULL,                     /* Validator */
		 NULL,                     /* User data */
		 ZBUS_OBSERVERS_EMPTY,     /* Observers list */
		 ZBUS_MSG_INIT(0) /* Message initialization */);

#if defined(CONFIG_SHELL)
#include <zephyr/shell/shell.h>

static int trigger_cmd_handler(const struct shell *sh, size_t argc, char **argv, void *data)
{
	return zbus_chan_pub(&chan_trigger_event, MSG_TRIGGER_EVT(.which_trigger_evt = (int)data),
			     K_MSEC(500));
}

SHELL_SUBCMD_DICT_SET_CREATE(sub_trigger_event, trigger_cmd_handler,
			     (activated, MSG_TRIGGER_EVENT_ACTIVATED_TAG, "activated"),
			     (initiated, MSG_TRIGGER_EVENT_INITIATED_TAG, "initiated"),
			     (ready, MSG_TRIGGER_EVENT_READY_TAG, "ready"));

SHELL_CMD_REGISTER(trigger, &sub_trigger_event, "Indicator action", NULL);
#endif /* CONFIG_SHELL */
