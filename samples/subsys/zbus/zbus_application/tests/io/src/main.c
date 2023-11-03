/*
 * Copyright (c) 2022 Rodrigo Peixoto <rodrigopex@gmail.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#include "calculator_scope.h"

#include <zephyr/kernel.h>
#include <zephyr/zbus/zbus.h>
#include <zephyr/ztest.h>

ZBUS_MSG_SUBSCRIBER_DEFINE(msub_io_test);

ZBUS_CHAN_ADD_OBS(chan_calculator_in__command, msub_io_test, 3);

#define HISTORY_SIZE 5

static bool calculator_command__is_op_valid(struct msg_calculator__command *cmd)
{
	return (cmd->op == '+') || (cmd->op == '-') || (cmd->op == '*') || (cmd->op == '/');
}

static bool calculator_command__is_equal(struct msg_calculator__command *cmd1,
					 struct msg_calculator__command *cmd2)
{
	if (cmd1->op != cmd2->op) {
		return false;
	}
	if (cmd1->param1 != cmd2->param1) {
		return false;
	}
	if (cmd1->param2 != cmd2->param2) {
		return false;
	}
	return true;
}

/* WARN: Potential flaky test. */
ZTEST(io, test_io_generation)
{
	struct msg_calculator__command cmds[HISTORY_SIZE] = {{0}};

	const struct zbus_channel *chan;
	for (int i = 0; i < HISTORY_SIZE; ++i) {
		zassert_true(0 == zbus_sub_wait_msg(&msub_io_test, &chan, &cmds[i], K_FOREVER),
			     NULL);

		zassert_true(true == calculator_command__is_op_valid(&cmds[i]), NULL);
	}

	for (int i = 0; i < HISTORY_SIZE; ++i) {
		for (int j = i + 1; j < HISTORY_SIZE; ++j) {
			zassert_false(calculator_command__is_equal(&cmds[i], &cmds[j]), NULL);
		}
	}
}

ZTEST_SUITE(io, NULL, NULL, NULL, NULL, NULL);
