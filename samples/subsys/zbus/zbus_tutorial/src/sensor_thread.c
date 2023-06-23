/*
 * Copyright (c) 2023 Rodrigo Peixoto <rodrigopex@gmail.com>
 * SPDX-License-Identifier: Apache-2.0
 */
#include "messages.h"
#include <native_rtc.h>

#include <zephyr/kernel.h>
#include <zephyr/zbus/zbus.h>

#define GET_TIME_IN_NS() (native_rtc_gettime_us(RTC_CLOCK_PSEUDOHOSTREALTIME) * NSEC_PER_USEC)

ZBUS_CHAN_DEFINE(chan_sensor_in__fetch,      /* Channel name */
		 bool,                       /* Message type */
		 NULL,                       /* User data */
		 NULL,                       /* Validator */
		 ZBUS_OBSERVERS(sub_sensor), /* Observers */
		 false);

ZBUS_CHAN_DEFINE(chan_sensor_out__data,  /* Channel name */
		 struct sensor_data_msg, /* Message type */
		 NULL,                   /* User data */
		 NULL,                   /* Validator */
		 ZBUS_OBSERVERS_EMPTY,   /* Observers */
		 ZBUS_MSG_INIT(0));

ZBUS_SUBSCRIBER_DEFINE(sub_sensor, 4);

void sensor_thread()
{
	const struct zbus_channel *chan;

	struct sensor_data_msg sdata = {.x = 0, .y = 0, .z = 0};

	while (1) {
		zbus_sub_wait(&sub_sensor, &chan, K_FOREVER);

		sdata.x += 1;
		sdata.y += 10;
		sdata.z += 100;

		uint64_t start = GET_TIME_IN_NS();
		zbus_chan_pub(&chan_sensor_out__data, &sdata, K_MSEC(500));
		uint64_t delta = GET_TIME_IN_NS() - start;

		printk(" *** Publishing duration: %lluus\n", delta / 1000);
	}
}

K_THREAD_DEFINE(sensor_thread_id, 1024, sensor_thread, NULL, NULL, NULL, 3, 0, 0);
