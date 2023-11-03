/*
 * Copyright (c) 2023 Rodrigo Peixoto <rodrigopex@gmail.com>
 * SPDX-License-Identifier: Apache-2.0
 */
#include "calculator_scope.h"
#include <stdlib.h>
#include <zephyr/kernel.h>
#include <zephyr/zbus/zbus.h>
#include <zephyr/random/random.h>
#include <zephyr/shell/shell.h>

ZBUS_MSG_SUBSCRIBER_DEFINE(msub_io_calculator__command_result);

ZBUS_CHAN_ADD_OBS(chan_calculator_out__command_result, msub_io_calculator__command_result, 1);

static int cmd_eval(const struct shell *sh, size_t argc, char **argv)
{
	int err;
	const struct zbus_channel *chan;
	struct msg_calculator__command_result cmd_res;

	struct msg_calculator__command cmd = {
		.op = argv[2][0], .param1 = atoi(argv[1]), .param2 = atoi(argv[3])};

	err = zbus_chan_pub(&chan_calculator_in__command, &cmd, K_FOREVER);
	if (err) {
		return err;
	}

	err = zbus_sub_wait_msg(&msub_io_calculator__command_result, &chan, &cmd_res, K_MSEC(500));
	if (err) {
		return err;
	}

	shell_print(sh, "%d %c %d = %d", cmd.param1, cmd.op, cmd.param2, cmd_res.result);

	return 0;
}

SHELL_CMD_ARG_REGISTER(eval, NULL, "Calculator command", cmd_eval, 4, 0);
