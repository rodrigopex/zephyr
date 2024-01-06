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

			if (msg.indicator_evt.type == INDICATOR_EVENT_READY) {
				++system_status;

				LOG_INF("Indicator module...[ok]");
			} else {
				LOG_WRN("Indicator module...[failed: %d]",
					msg.indicator_evt.error.code);
			}
		} else if (chan == &chan_trigger_event) {

			if (msg.trigger_evt.type == TRIGGER_EVENT_READY) {
				++system_status;
				LOG_INF("Trigger module...[ok]");

			} else {
				LOG_WRN("Trigger module...[failed: %d]\n",
					msg.indicator_evt.error.code);
			}
		}

		if (system_status == 2) {
			zbus_obs_set_enable(&msub_post, false);

			break;
		}
	}

	return 0;
}
