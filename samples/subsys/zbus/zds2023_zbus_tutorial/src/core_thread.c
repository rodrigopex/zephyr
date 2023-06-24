/*
 * Copyright (c) 2023 Rodrigo Peixoto <rodrigopex@gmail.com>
 * SPDX-License-Identifier: Apache-2.0
 */
#include "messages.h"

#include <zephyr/logging/log.h>
#include <zephyr/kernel.h>
#include <zephyr/zbus/zbus.h>
LOG_MODULE_DECLARE(zbus, CONFIG_ZBUS_LOG_LEVEL);

ZBUS_CHAN_DECLARE(sensor_data_chan, payload_chan);

ZBUS_SUBSCRIBER_DEFINE(core_thread_sub, 4);

void core_thread()
{
	LOG_INF("Core thread started!");

	const struct zbus_channel *chan;

	struct sensor_data_msg sdata = {.x = 0, .y = 0, .z = 0};

	uint64_t payload = 0;

	while (!zbus_sub_wait(&core_thread_sub, &chan, K_FOREVER)) {

		int err = zbus_chan_read(&sensor_data_chan, &sdata, K_MSEC(500));
		if (err) {
			LOG_WRN("Could not read the channel. Error code: %d", err);
		} else {
			payload = sdata.x + sdata.y + sdata.z;

			zbus_chan_pub(&payload_chan, &payload, K_MSEC(500));
		}
	}
}

K_THREAD_DEFINE(core_thread_id, 1024, core_thread, NULL, NULL, NULL, 3, 0, 0);
