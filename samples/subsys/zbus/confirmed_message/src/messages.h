/*
 * Copyright (c) 2022 Rodrigo Peixoto <rodrigopex@gmail.com>
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef MESSAGES_H
#define MESSAGES_H
#include <stdint.h>

#include <zephyr/kernel.h>

struct payload_msg {
	int sequence;
	uint8_t data[16];
};

enum critical_sub_id {
	CRITICAL1,
	CRITICAL2,
	CRITICAL3,
	CRITICAL4,
	CRITICAL5,
};

struct ack_msg {
	uint8_t sequence;
	enum critical_sub_id id;
};
#endif /* MESSAGES_H */
