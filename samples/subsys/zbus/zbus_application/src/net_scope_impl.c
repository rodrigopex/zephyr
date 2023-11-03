/*
 * Copyright (c) 2023 Rodrigo Peixoto <rodrigopex@gmail.com>
 * SPDX-License-Identifier: Apache-2.0
 */
#include "calculator_scope.h"
#include "net_scope.h"
#include <zephyr/kernel.h>
#include <zephyr/zbus/zbus.h>

ZBUS_MSG_SUBSCRIBER_DEFINE(msub_net);

ZBUS_CHAN_ADD_OBS(chan_calculator_out__command_result, msub_net, 3);

static void net_thread(void)
{
	int err;
	const struct zbus_channel *chan;
	struct msg_calculator__command_result calc_res;
	struct msg_net__transmission_result trans_res;

	while (1) {
		if (zbus_sub_wait_msg(&msub_net, &chan, &calc_res, K_FOREVER)) {
			k_oops();
		}

		if (calc_res.return_code) {
			err = calc_res.return_code;
			printk("ERR(%d)\n", calc_res.return_code);
		} else {
			err = 0;
			printk("%d\n", calc_res.result);
		}

		trans_res.return_code = err;

		zbus_chan_pub(&chan_net_out__transmission_done, &trans_res, K_FOREVER);
	}
}

K_THREAD_DEFINE(net_thread_id, 1024, net_thread, NULL, NULL, NULL, 3, 0, 0);
