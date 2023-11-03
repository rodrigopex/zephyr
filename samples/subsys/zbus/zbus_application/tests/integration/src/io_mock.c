/*
 * Copyright (c) 2023 Rodrigo Peixoto <rodrigopex@gmail.com>
 * SPDX-License-Identifier: Apache-2.0
 */
#include "calculator_scope.h"
#include <zephyr/kernel.h>
#include <zephyr/zbus/zbus.h>
#include <zephyr/random/random.h>

static void io_thread(void)
{
	const char ops[] = "+-*/";

	while (1) {
		struct msg_calculator__command cmd = {.op = ops[sys_rand32_get() % 4],
						      .param1 = sys_rand32_get() % 25,
						      .param2 = sys_rand32_get() % 25};

		printk("\n%d %c %d =", cmd.param1, cmd.op, cmd.param2);

		zbus_chan_pub(&chan_calculator_in__command, &cmd, K_FOREVER);

		k_msleep(1000);
	}
}

K_THREAD_DEFINE(io_thread_id, 1024, io_thread, NULL, NULL, NULL, 3, 0, 0);
