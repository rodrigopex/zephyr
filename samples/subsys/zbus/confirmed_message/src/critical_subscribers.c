/*
 * Copyright (c) 2022 Rodrigo Peixoto <rodrigopex@gmail.com>
 * SPDX-License-Identifier: Apache-2.0
 */
#include "messages.h"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/zbus/zbus.h>

LOG_MODULE_DECLARE(zbus, CONFIG_ZBUS_LOG_LEVEL);

ZBUS_CHAN_DECLARE(internal_chan, internal_ack_chan);

ZBUS_SUBSCRIBER_DEFINE(critical1_sub, 4);

ZBUS_SUBSCRIBER_DEFINE(critical2_sub, 4);

ZBUS_SUBSCRIBER_DEFINE(critical3_sub, 4);

ZBUS_SUBSCRIBER_DEFINE(critical4_sub, 4);

ZBUS_SUBSCRIBER_DEFINE(critical5_sub, 4);

static void sub_task(void *sub, void *id)
{
	struct zbus_channel *chan;
	struct payload_msg local_payload;
	struct zbus_observer *subscriber = sub;
	enum critical_sub_id *sub_id = id;

	while (!zbus_sub_wait(subscriber, &chan, K_FOREVER)) {
		if (chan == &internal_chan) {
			int err = zbus_chan_read(&internal_chan, &local_payload, K_MSEC(300));
			__ASSERT(!err, "Channel must be read");

			LOG_INF("----------------------------------");
			LOG_INF("%s received sequence: %d", subscriber->name,
				local_payload.sequence);
			LOG_HEXDUMP_INF(local_payload.data, ARRAY_SIZE(local_payload.data),
					"Data:");

			struct ack_msg am = {.sequence = local_payload.sequence, .id = *sub_id};

			zbus_chan_pub(&internal_ack_chan, &am, K_MSEC(100));
		}
	}
}
enum critical_sub_id critical1_id = CRITICAL1;
K_THREAD_DEFINE(subscriber1_task_id, 512, sub_task, &critical1_sub, &critical1_id, NULL, 3, 0, 0);

enum critical_sub_id critical2_id = CRITICAL2;
K_THREAD_DEFINE(subscriber2_task_id, 512, sub_task, &critical2_sub, &critical2_id, NULL, 3, 0, 0);

enum critical_sub_id critical3_id = CRITICAL3;
K_THREAD_DEFINE(subscriber3_task_id, 512, sub_task, &critical3_sub, &critical3_id, NULL, 3, 0, 0);

enum critical_sub_id critical4_id = CRITICAL4;
K_THREAD_DEFINE(subscriber4_task_id, 512, sub_task, &critical4_sub, &critical4_id, NULL, 3, 0, 0);

enum critical_sub_id critical5_id = CRITICAL5;
K_THREAD_DEFINE(subscriber5_task_id, 512, sub_task, &critical5_sub, &critical5_id, NULL, 3, 0, 0);
