/*
 * Copyright (c) 2022 Rodrigo Peixoto <rodrigopex@gmail.com>
 * SPDX-License-Identifier: Apache-2.0
 */
#include "messages.h"

#include <zephyr/logging/log.h>
#include <zephyr/zbus/zbus.h>
LOG_MODULE_DECLARE(zbus, CONFIG_ZBUS_LOG_LEVEL);

ZBUS_SUBSCRIBER_DEFINE(peripheral_sub, 8);

ZBUS_CHAN_DECLARE(sensor_data_chan);

ZBUS_CHAN_DEFINE(sensor_data_chan,	 /* Name */
		 false,			 /* Read only */
		 struct sensor_data_msg, /* Message type */

		 NULL,			     /* Validator */
		 ZBUS_OBSERVERS(bridge_sub), /* observers */
		 ZBUS_MSG_INIT(0)	     /* Initial value {0} */
);

ZBUS_CHAN_DEFINE(start_measurement_chan, /* Name */
		 false,			 /* Read only */
		 struct action_msg,	 /* Message type */

		 NULL,					     /* Validator */
		 ZBUS_OBSERVERS(bridge_sub, peripheral_sub), /* observers */
		 ZBUS_MSG_INIT(false)			     /* Initial value */
);

static void peripheral_thread(void)
{
	struct zbus_channel *chan;
	struct sensor_data_msg sd = {0, 0};

	while (!zbus_sub_wait(&peripheral_sub, &chan, K_FOREVER)) {
		LOG_DBG("Peripheral sending sensor data");

		sd.a += 1;
		sd.b += 10;

		zbus_chan_pub(&sensor_data_chan, &sd, K_MSEC(250));
	}
}

K_THREAD_DEFINE(peripheral_thread_id, 1024, peripheral_thread, NULL, NULL, NULL, 3, 0, 0);
