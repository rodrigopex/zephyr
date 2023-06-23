/*
 * Copyright (c) 2023 Rodrigo Peixoto <rodrigopex@gmail.com>
 * SPDX-License-Identifier: Apache-2.0
 */
#include "messages.h"

#include <zephyr/logging/log.h>
#include <zephyr/kernel.h>
#include <zephyr/zbus/zbus.h>
#include <zephyr/random/random.h>
LOG_MODULE_DECLARE(zbus, CONFIG_ZBUS_LOG_LEVEL);

ZBUS_CHAN_DEFINE(chan_lora_in__payload,     /* Channel name */
		 uint64_t,                  /* Message type */
		 NULL,                      /* User data */
		 NULL,                      /* Validator */
		 ZBUS_OBSERVERS(msub_lora), /* Observers */
		 0);

ZBUS_CHAN_DEFINE(chan_lora_out__transmission_done, /* Channel name */
		 bool,                             /* Message type */
		 NULL,                             /* User data */
		 NULL,                             /* Validator */
		 ZBUS_OBSERVERS_EMPTY,             /* Observers */
		 false);

ZBUS_MSG_SUBSCRIBER_DEFINE(msub_lora);

void lora_thread()
{
	LOG_INF("Lora thread started!");

	const struct zbus_channel *chan;

	bool transmission_done;

	uint64_t payload;

	while (!zbus_sub_wait_msg(&msub_lora, &chan, &payload, K_FOREVER)) {

		/* To simulate the transmission delay. In a imaginary scenario, if the transmission
		 * takes more than 500 ms, it will fail.*/
		int32_t rand_delay = sys_rand32_get() % 1000;

		LOG_INF(" ---> Sending packet to the network server...(%d ms)", rand_delay);
		k_msleep(rand_delay);

		transmission_done = (rand_delay % 2) == 0;

		zbus_chan_pub(&chan_lora_out__transmission_done, &transmission_done, K_MSEC(500));
	}
}

K_THREAD_DEFINE(lora_thread_id, 1024, lora_thread, NULL, NULL, NULL, 4, 0, 0);
