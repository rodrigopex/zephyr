/*
 * Copyright (c) 2023 Rodrigo Peixoto <rodrigopex@gmail.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#include "calculator_scope.h"
#include "net_scope.h"

#include <zephyr/kernel.h>
#include <zephyr/zbus/zbus.h>
#include <zephyr/ztest.h>
#include <zephyr/random/random.h>

ZBUS_MSG_SUBSCRIBER_DEFINE(msub_calculator__command_result);
ZBUS_MSG_SUBSCRIBER_DEFINE(msub_net__transmission_result);
ZBUS_MSG_SUBSCRIBER_DEFINE(msub_mock__transmission_result);

ZBUS_CHAN_ADD_OBS(chan_calculator_out__command_result, msub_calculator__command_result, 3);
ZBUS_CHAN_ADD_OBS(chan_net_out__transmission_done, msub_net__transmission_result, 3);
ZBUS_CHAN_ADD_OBS(chan_net_out__transmission_done, msub_mock__transmission_result, 3);

#define ITERATION_COUNT 1000

static void calculator_mock_thread(void)
{
	const struct zbus_channel *chan;
	struct msg_net__transmission_result trans_res;

	int results[] = {0, 0, 0, 0, 0, -EINVAL, -EFAULT};

	while (1) {
		struct msg_calculator__command_result cmd_res = {
			.result = sys_rand32_get(), .return_code = results[sys_rand32_get() % 7]};

		zbus_chan_pub(&chan_calculator_out__command_result, &cmd_res, K_FOREVER);

		zbus_sub_wait_msg(&msub_mock__transmission_result, &chan, &trans_res, K_FOREVER);
	}
}

K_THREAD_DEFINE(calculator_mock_thread_id, 1024, calculator_mock_thread, NULL, NULL, NULL, 3, 0, 0);

ZTEST(net, test_transmission)
{
	const struct zbus_channel *chan;
	struct msg_calculator__command_result cmd_res;
	struct msg_net__transmission_result trans_res;

	for (int i = 0; i < ITERATION_COUNT; ++i) {

		zassert_true(0 == zbus_sub_wait_msg(&msub_calculator__command_result, &chan,
						    &cmd_res, K_FOREVER));

		zassert_true(0 == zbus_sub_wait_msg(&msub_net__transmission_result, &chan,
						    &trans_res, K_FOREVER));

		zassert_true(cmd_res.return_code == trans_res.return_code);
	}
	k_thread_abort(calculator_mock_thread_id);
}

ZTEST_SUITE(net, NULL, NULL, NULL, NULL, NULL);
