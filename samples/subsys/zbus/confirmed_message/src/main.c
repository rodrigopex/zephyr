/*
 * Copyright (c) 2022 Rodrigo Peixoto <rodrigopex@gmail.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#include "messages.h"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/zbus/zbus.h>
LOG_MODULE_DECLARE(zbus, CONFIG_ZBUS_LOG_LEVEL);

bool internal_chan_validator(const void *msg, size_t msg_size)
{
	ARG_UNUSED(msg_size);

	const struct payload_msg *p = msg;

	if (p->sequence >= 0) {
		return true;
	}

	return false;
}

ZBUS_CHAN_DEFINE(internal_chan,  /* Name */
		 false,		     /* Read only */
		 struct payload_msg, /* Message type */

		 internal_chan_validator, /* Validator */
		 ZBUS_OBSERVERS_EMPTY,	      /* observers */
		 ZBUS_MSG_INIT(0)	      /* Initial value */
);

ZBUS_CHAN_DEFINE(internal_ack_chan, /* Name */
		 false,			/* Read only */
		 struct ack_msg,	/* Message type */

		 NULL,				   /* Validator */
		 ZBUS_OBSERVERS(internal_ack_lis), /* observers */
		 ZBUS_MSG_INIT(0)		   /* Initial value */
);

K_SEM_DEFINE(internal_chan_sem, 1, 1);

static struct payload_msg global_payload;

static uint8_t global_ack_ids;

static void internal_ack_listener_cb(const struct zbus_channel *chan)
{
	const struct ack_msg *am = zbus_chan_const_msg(chan);

	__ASSERT(am->sequence == global_payload.sequence,
		 "Sequence must be the same. Found left = %d and right = %d", am->sequence,
		 global_payload.sequence);

	LOG_INF("----------------------------------");
	LOG_INF("Critical %d sent ACK!", am->id + 1);

	WRITE_BIT(global_ack_ids, am->id, 1);

	if (global_ack_ids == 0b00011111) {
		global_ack_ids = 0;

		++global_payload.sequence;

		k_sem_give(&internal_chan_sem);
	}
}

ZBUS_LISTENER_DEFINE(internal_ack_lis, internal_ack_listener_cb);

ZBUS_OBS_DECLARE(critical1_sub, critical2_sub, critical3_sub, critical4_sub, critical5_sub);

void main(void)
{
	memset(global_payload.data, 0, ARRAY_SIZE(global_payload.data));
	global_ack_ids = 0;

	zbus_chan_add_obs(&internal_chan, &critical1_sub, K_MSEC(100));
	zbus_chan_add_obs(&internal_chan, &critical2_sub, K_MSEC(100));
	zbus_chan_add_obs(&internal_chan, &critical3_sub, K_MSEC(100));
	zbus_chan_add_obs(&internal_chan, &critical4_sub, K_MSEC(100));
	zbus_chan_add_obs(&internal_chan, &critical5_sub, K_MSEC(100));

	for (int i = 0; i < 10; ++i) {
		global_payload.data[i] = i;

		if (!k_sem_take(&internal_chan_sem, K_FOREVER)) {
			LOG_INF("----------------------------------");
			LOG_INF(" *** USB sending message sequence %d to interested tasks "
				"(on-to-many) *** ",
				global_payload.sequence);

			zbus_chan_pub(&internal_chan, &global_payload, K_SECONDS(1));
		}
	}

	if (!k_sem_take(&internal_chan_sem, K_FOREVER)) {
		LOG_INF("Sample finished with success. %d "
			"confirmed messages sent and confirmed",
			global_payload.sequence);
	}
}
