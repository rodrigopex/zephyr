/*
 * Copyright (c) 2023 Rodrigo Peixoto <rodrigopex@gmail.com>
 * SPDX-License-Identifier: Apache-2.0
 */
#include "messages.h"

#include <zephyr/logging/log.h>
#include <zephyr/kernel.h>
#include <zephyr/zbus/zbus.h>
LOG_MODULE_DECLARE(zbus, CONFIG_ZBUS_LOG_LEVEL);

ZBUS_CHAN_DECLARE(payload_chan, transmission_done_chan);

ZBUS_SUBSCRIBER_DEFINE(lora_thread_sub, 4);

void lora_thread()
{
	LOG_INF("Lora thread started!");

	const struct zbus_channel *chan;

	uint64_t payload = 0;

	bool transmission_done = true;

	while (!zbus_sub_wait(&lora_thread_sub, &chan, K_FOREVER)) {

		int err = zbus_chan_read(&payload_chan, &payload, K_MSEC(500));
		if (err) {
			LOG_WRN("Could not read the channel. Error code: %d", err);
		} else {
			/* To simulate the transmission delay */
			k_msleep(1000);

			zbus_chan_pub(&transmission_done_chan, &transmission_done, K_MSEC(500));
		}
	}
}

K_THREAD_DEFINE(lora_thread_id, 1024, lora_thread, NULL, NULL, NULL, 4, 0, 0);
