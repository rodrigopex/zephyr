#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/util.h>
#include <zephyr/logging/log.h>

#include "ifaces/indicator.h"
#include "ifaces/trigger.h"

LOG_MODULE_DECLARE(app, CONFIG_APP_LOG_LEVEL);

static struct {
	struct gpio_dt_spec led;
	bool is_on;
	int pulse_duration;
} self = {.led = GPIO_DT_SPEC_GET_OR(DT_ALIAS(led0), gpios, {0}),
	  .is_on = false,
	  .pulse_duration = 100};

ZBUS_MSG_SUBSCRIBER_DEFINE(msub_led_indicator);

ZBUS_CHAN_ADD_OBS(chan_trigger_event, msub_led_indicator, 3);
ZBUS_CHAN_ADD_OBS(chan_indicator_command, msub_led_indicator, 3);

static inline void update_led_state(void)
{
	gpio_pin_set_dt(&self.led, self.is_on);
}

static inline void toggle(void)
{
	self.is_on = !self.is_on;
	update_led_state();
}

static inline void pulse(void)
{
	toggle();
	k_msleep(self.pulse_duration);
	toggle();
}

void led_thread(void)
{
	int err;

	if (self.led.port && !gpio_is_ready_dt(&self.led)) {
		zbus_chan_pub(&chan_indicator_event,
			      MSG_INDICATOR_EVT(.type = INDICATOR_EVENT_FAILED,
						.error = {.code = -ENODEV}),
			      K_MSEC(500));

		return;
	}

	err = gpio_pin_configure_dt(&self.led, GPIO_OUTPUT);
	if (err != 0) {
		zbus_chan_pub(
			&chan_indicator_event,
			MSG_INDICATOR_EVT(.type = INDICATOR_EVENT_FAILED, .error = {.code = err}),
			K_MSEC(500));

		return;
	}

	gpio_pin_set_dt(&self.led, 0);

	LOG_INF("Set up LED at %s pin %d", self.led.port->name, self.led.pin);

	zbus_chan_pub(&chan_indicator_event, MSG_INDICATOR_EVT(.type = INDICATOR_EVENT_READY),
		      K_MSEC(500));

	const struct zbus_channel *chan;

	union {
		struct msg_indicator_command indicator_cmd;
		struct msg_trigger_event trigger_evt;
	} msg;

	zbus_obs_attach_to_thread(&msub_led_indicator);

	while (1) {
		zbus_sub_wait_msg(&msub_led_indicator, &chan, &msg, K_FOREVER);
		if (chan == &chan_trigger_event) {
			switch (msg.trigger_evt.type) {
			case TRIGGER_EVENT_ACTIVATED:
				pulse();
				break;
			default:
				/* Trigger event discarded */
				break;
			}

		} else if (chan == &chan_indicator_command) {
			switch (msg.indicator_cmd.type) {
			case INDICATOR_CMD_OFF:
				self.is_on = false;
				update_led_state();
				break;
			case INDICATOR_CMD_ON:
				self.is_on = true;
				update_led_state();
				break;
			case INDICATOR_CMD_TOGGLE:
				toggle();
				break;
			case INDICATOR_CMD_PULSE:
				pulse();
				break;
			case INDICATOR_CMD_GET_PULSE_DURATION: {
				zbus_chan_pub(
					&chan_indicator_response,
					MSG_INDICATOR_RSP(.type = INDICATOR_RSP_PULSE_DURATION,
							  .pulse = {.duration =
									    self.pulse_duration}),
					K_MSEC(250));
				break;
			}
			case INDICATOR_CMD_SET_PULSE_DURATION: {
				self.pulse_duration = msg.indicator_cmd.pulse.duration;
				zbus_chan_pub(
					&chan_indicator_response,
					MSG_INDICATOR_RSP(.type = INDICATOR_RSP_PULSE_DURATION,
							  .pulse = {.duration =
									    self.pulse_duration}),
					K_MSEC(250));
				break;
			}
			default:
				LOG_ERR("Indicator action invalid");
			}
		}
	}
}

K_THREAD_DEFINE(led_thread_id, 1024, led_thread, NULL, NULL, NULL, 4, 0, 0);
