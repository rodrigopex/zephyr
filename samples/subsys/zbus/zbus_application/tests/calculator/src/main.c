/*
 * Copyright (c) 2022 Rodrigo Peixoto <rodrigopex@gmail.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#include "calculator_scope.h"

#include <zephyr/kernel.h>
#include <zephyr/zbus/zbus.h>
#include <zephyr/ztest.h>
#include <zephyr/random/random.h>

ZBUS_MSG_SUBSCRIBER_DEFINE(msub_calculator_command_result);

ZBUS_CHAN_ADD_OBS(chan_calculator_out__command_result, msub_calculator_command_result, 3);

#define ITERATION_COUNT 5000

ZTEST(calculator, test_sum_sub_mul)
{
	int expected_result;

	const struct zbus_channel *chan;
	struct msg_calculator__command_result cmd_res = {0};
	char *ops = (char[]){'+', '-', '*'};
	char *end = ops + 3;
	for (; ops < end; ++ops) {
		for (int i = 0; i < ITERATION_COUNT; ++i) {

			struct msg_calculator__command cmd = {
				.op = *ops, .param1 = sys_rand32_get(), .param2 = sys_rand32_get()};

			expected_result = cmd.op == '+'
						  ? cmd.param1 + cmd.param2
						  : (cmd.op == '-' ? (cmd.param1 - cmd.param2)
								   : (cmd.param1 * cmd.param2));

			zassert_true(0 ==
				     zbus_chan_pub(&chan_calculator_in__command, &cmd, K_FOREVER));

			zassert_true(0 == zbus_sub_wait_msg(&msub_calculator_command_result, &chan,
							    &cmd_res, K_FOREVER));

			zassert_true(0 == cmd_res.return_code);

			zassert_true(expected_result == cmd_res.result, "%d %c %d = (%d == %d)",
				     cmd.param1, cmd.op, cmd.param2, expected_result,
				     cmd_res.result);
		}
	}
}

ZTEST(calculator, test_div)
{
	int expected_result;
	const struct zbus_channel *chan;
	struct msg_calculator__command_result cmd_res;

	/*Division by zero */
	struct msg_calculator__command cmd = {.op = '/', .param1 = sys_rand32_get(), .param2 = 0};

	zassert_true(0 == zbus_chan_pub(&chan_calculator_in__command, &cmd, K_FOREVER));

	zassert_true(0 == zbus_sub_wait_msg(&msub_calculator_command_result, &chan, &cmd_res,
					    K_FOREVER));

	zassert_true(-EFAULT == cmd_res.return_code);

	/* Random numbers */
	for (int i = 0; i < ITERATION_COUNT; ++i) {

		struct msg_calculator__command cmd = {
			.op = '/', .param1 = sys_rand32_get(), .param2 = sys_rand32_get()};

		zassert_true(0 == zbus_chan_pub(&chan_calculator_in__command, &cmd, K_FOREVER));

		zassert_true(0 == zbus_sub_wait_msg(&msub_calculator_command_result, &chan,
						    &cmd_res, K_FOREVER));
		if (cmd.param2 == 0) {
			zassert_true(-EFAULT == cmd_res.return_code);
		} else {
			expected_result = cmd.param1 / cmd.param2;

			zassert_true(0 == cmd_res.return_code);

			zassert_true(expected_result == cmd_res.result);
		}
	}
}

ZTEST_SUITE(calculator, NULL, NULL, NULL, NULL, NULL);
