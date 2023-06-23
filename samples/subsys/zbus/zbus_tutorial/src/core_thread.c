/*
 * Copyright (c) 2023 Rodrigo Peixoto <rodrigopex@gmail.com>
 * SPDX-License-Identifier: Apache-2.0
 */
#include "messages.h"
#include <stdint.h>

#include <zephyr/logging/log.h>
#include <zephyr/kernel.h>
#include <zephyr/zbus/zbus.h>
#include <zephyr/init.h>
LOG_MODULE_DECLARE(zbus, CONFIG_ZBUS_LOG_LEVEL);

ZBUS_CHAN_DECLARE(chan_sensor_out__data, chan_lora_in__payload, chan_lora_out__transmission_done);

ZBUS_MSG_SUBSCRIBER_DEFINE(msub_core);

ZBUS_CHAN_ADD_OBS(chan_sensor_out__data, msub_core, 3);
ZBUS_CHAN_ADD_OBS(chan_lora_out__transmission_done, msub_core, 3);

void core_thread()
{
	LOG_INF("Core thread started!");

	const struct zbus_channel *chan;

	union {
		struct sensor_data_msg sensor_data;
		bool transmission_done;
	} core_msg = {0};

	uint64_t payload = 0;
	bool retry = true;

	while (!zbus_sub_wait_msg(&msub_core, &chan, &core_msg, K_FOREVER)) {
		if (chan == &chan_sensor_out__data) {
			payload = core_msg.sensor_data.x + core_msg.sensor_data.y +
				  core_msg.sensor_data.y;

			retry = 1;
			zbus_chan_pub(&chan_lora_in__payload, &payload, K_MSEC(500));
		} else {
			if (core_msg.transmission_done) {
				/* Do nothing! */
			} else if ((core_msg.transmission_done == false) && retry) {
				LOG_WRN("Retrying to send the payload");
				retry = false;
				zbus_chan_notify(&chan_lora_in__payload, K_MSEC(500));
			} else {
				LOG_ERR("Could not send the payload");
			}
		}
	}
}

K_THREAD_DEFINE(core_thread_id, 1024, core_thread, NULL, NULL, NULL, 3, 0, 0);
