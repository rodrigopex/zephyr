#include <zephyr/zbus/zbus.h>

#include "ifaces/indicator.h"
#include "ifaces/trigger.h"

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(app, CONFIG_APP_LOG_LEVEL);

ZBUS_MSG_SUBSCRIBER_DEFINE(msub_post);

ZBUS_CHAN_ADD_OBS(chan_indicator_event, msub_post, 3);
ZBUS_CHAN_ADD_OBS(chan_trigger_event, msub_post, 3);

int main(void)
{
	int system_status = 0;

	union {
		struct msg_trigger_event trigger_evt;
		struct msg_indicator_event indicator_evt;
	} msg;

	const struct zbus_channel *chan;

	while (1) {
		zbus_sub_wait_msg(&msub_post, &chan, &msg, K_FOREVER);

		if (chan == &chan_indicator_event) {

			if (msg.indicator_evt.which_indicator_evt ==
			    MSG_INDICATOR_EVENT_READY_TAG) {
				++system_status;

				LOG_INF("Indicator service...[ok]");
			} else {
				LOG_WRN("Indicator service...[failed: %d]",
					msg.indicator_evt.failed.error_code);
			}
		} else if (chan == &chan_trigger_event) {

			if (msg.trigger_evt.which_trigger_evt == MSG_TRIGGER_EVENT_READY_TAG) {
				++system_status;
				LOG_INF("Trigger service...[ok]");

			} else {
				LOG_WRN("Trigger service...[failed: %d]\n",
					msg.trigger_evt.failed.error_code);
			}
		}

		if (system_status == 2) {
			zbus_obs_set_enable(&msub_post, false);

			break;
		}
	}

	return 0;
}
