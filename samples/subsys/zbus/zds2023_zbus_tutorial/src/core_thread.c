/*
 * Copyright (c) 2023 Rodrigo Peixoto <rodrigopex@gmail.com>
 * SPDX-License-Identifier: Apache-2.0
 */
#include "messages.h"

#include <zephyr/logging/log.h>
#include <zephyr/kernel.h>
#include <zephyr/zbus/zbus.h>
#include <zephyr/init.h>
LOG_MODULE_DECLARE(zbus, CONFIG_ZBUS_LOG_LEVEL);

ZBUS_CHAN_DECLARE(sensor_data_chan, payload_chan);

#if defined(CONFIG_CORE_AS_LISTENER)

void core_listener_callback(const struct zbus_channel *chan)
{
	struct sensor_data_msg *sdata = (struct sensor_data_msg *)zbus_chan_const_msg(chan);

	uint64_t payload = sdata->x + sdata->y + sdata->z;

	zbus_chan_pub(&payload_chan, &payload, K_NO_WAIT);
}

ZBUS_LISTENER_DEFINE(core_lis, core_listener_callback);

int core_listener_init(void)
{
	return zbus_chan_add_obs(&sensor_data_chan, &core_lis, K_NO_WAIT);
}

SYS_INIT(core_listener_init, APPLICATION, 3);

#else

ZBUS_SUBSCRIBER_DEFINE(core_thread_sub, 4);

void core_thread()
{
	LOG_INF("Core thread started!");

	zbus_chan_add_obs(&sensor_data_chan, &core_thread_sub, K_NO_WAIT);

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

#endif
