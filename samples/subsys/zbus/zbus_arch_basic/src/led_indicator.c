#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/util.h>
#include <zephyr/logging/log.h>

#include "nano_services/indicator.h"
#include "nano_services/trigger.h"

LOG_MODULE_DECLARE(app, CONFIG_APP_LOG_LEVEL);

static struct {
	struct gpio_dt_spec led;
	bool is_on;
	int pulse_duration;
} self = {.led = GPIO_DT_SPEC_GET_OR(DT_ALIAS(led0), gpios, {0}),
	  .is_on = false,
	  .pulse_duration = 100};

ZBUS_MSG_SUBSCRIBER_DEFINE(msub_led_indicator);

ZBUS_CHAN_ADD_OBS(chan_trigger_evt, msub_led_indicator, 3);
ZBUS_CHAN_ADD_OBS(chan_indicator_cmd, msub_led_indicator, 3);

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

static inline int send_state_reponse()
{
	return zbus_chan_pub(&chan_indicator_rsp,
			     MSG_INDICATOR_RSP(.which_indicator_rsp = MSG_INDICATOR_RSP_STATE_TAG,
					       .state = {.is_on = self.is_on}),
			     K_MSEC(250));
}
static inline int send_pulse_config_reponse()
{
	return zbus_chan_pub(
		&chan_indicator_rsp,
		MSG_INDICATOR_RSP(.which_indicator_rsp = MSG_INDICATOR_RSP_PULSE_CONFIG_TAG,
				  .pulse_config = {.duration = self.pulse_duration}),
		K_MSEC(250));
}

void led_thread(void)
{
	int err;

	if (self.led.port && !gpio_is_ready_dt(&self.led)) {
		zbus_chan_pub(&chan_indicator_evt,
			      MSG_INDICATOR_EVT(.which_indicator_evt = MSG_INDICATOR_EVT_FAILED_TAG,
						.failed = {.error_code = -ENODEV}),
			      K_MSEC(500));

		return;
	}

	err = gpio_pin_configure_dt(&self.led, GPIO_OUTPUT);
	if (err != 0) {
		zbus_chan_pub(&chan_indicator_evt,
			      MSG_INDICATOR_EVT(.which_indicator_evt = MSG_INDICATOR_EVT_FAILED_TAG,
						.failed = {.error_code = err}),
			      K_MSEC(500));

		return;
	}

	gpio_pin_set_dt(&self.led, 0);

	LOG_INF("Set up LED at %s pin %d", self.led.port->name, self.led.pin);

	zbus_chan_pub(&chan_indicator_evt,
		      MSG_INDICATOR_EVT(.which_indicator_evt = MSG_INDICATOR_EVT_READY_TAG),
		      K_MSEC(500));

	const struct zbus_channel *chan;

	union {
		struct msg_indicator_cmd indicator_cmd;
		struct msg_trigger_evt trigger_evt;
	} msg;

	zbus_obs_attach_to_thread(&msub_led_indicator);

	while (1) {
		zbus_sub_wait_msg(&msub_led_indicator, &chan, &msg, K_FOREVER);
		if (chan == &chan_trigger_evt) {
			switch (msg.trigger_evt.which_trigger_evt) {
			case MSG_TRIGGER_EVT_ACTIVATED_TAG:
				pulse();
				break;
			default:
				/* Trigger event discarded */
				break;
			}

		} else if (chan == &chan_indicator_cmd) {
			switch (msg.indicator_cmd.which_indicator_cmd) {
			case MSG_INDICATOR_CMD_OFF_TAG:
				self.is_on = false;
				update_led_state();
				send_state_reponse();
				break;
			case MSG_INDICATOR_CMD_ON_TAG:
				self.is_on = true;
				update_led_state();
				send_state_reponse();
				break;
			case MSG_INDICATOR_CMD_TOGGLE_TAG:
				toggle();
				send_state_reponse();
				break;
			case MSG_INDICATOR_CMD_PULSE_TAG:
				pulse();
				send_state_reponse();
				break;
			case MSG_INDICATOR_CMD_GET_PULSE_CONFIG_TAG: {
				send_pulse_config_reponse();
				break;
			}
			case MSG_INDICATOR_CMD_SET_PULSE_CONFIG_TAG: {
				self.pulse_duration = msg.indicator_cmd.set_pulse_config.duration;
				send_pulse_config_reponse();
				break;
			}
			default:
				LOG_ERR("Indicator action invalid %d",
					msg.indicator_cmd.which_indicator_cmd);
			}
		}
	}
}

K_THREAD_DEFINE(led_thread_id, 2048, led_thread, NULL, NULL, NULL, 4, 0, 0);
