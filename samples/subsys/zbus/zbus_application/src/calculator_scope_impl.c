/*
 * Copyright (c) 2023 Rodrigo Peixoto <rodrigopex@gmail.com>
 * SPDX-License-Identifier: Apache-2.0
 */
#include "calculator_scope.h"
#include "zephyr/sys/printk.h"
#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/zbus/zbus.h>

ZBUS_MSG_SUBSCRIBER_DEFINE(msub_calculator);

ZBUS_CHAN_ADD_OBS(chan_calculator_in__command, msub_calculator, 3);

void __calculator_command_executor(const struct msg_calculator__command *cmd,
				   struct msg_calculator__command_result *cmd_res)
{
	cmd_res->result = 0;
	cmd_res->return_code = 0;

	switch (cmd->op) {
	case '+':
		cmd_res->result = cmd->param1 + cmd->param2;
		break;
	case '-':
		cmd_res->result = cmd->param1 - cmd->param2;
		break;
	case '*':
		cmd_res->result = cmd->param1 * cmd->param2;
		break;
	case '/':
		if (cmd->param2 == 0) {
			cmd_res->return_code = -EFAULT;
		} else {
			cmd_res->result = cmd->param1 / cmd->param2;
		}
		break;
	default:
		cmd_res->return_code = -EINVAL;
	}
}

static void calculator_thread(void)
{
	const struct zbus_channel *chan;
	struct msg_calculator__command cmd = {0};

	while (1) {
		if (zbus_sub_wait_msg(&msub_calculator, &chan, &cmd, K_FOREVER)) {
			k_oops();
		}

		struct msg_calculator__command_result cmd_res;

		__calculator_command_executor(&cmd, &cmd_res);

		zbus_chan_pub(&chan_calculator_out__command_result, &cmd_res, K_FOREVER);
	}
}

K_THREAD_DEFINE(calculator_thread_id, 1024, calculator_thread, NULL, NULL, NULL, 3, 0, 0);
