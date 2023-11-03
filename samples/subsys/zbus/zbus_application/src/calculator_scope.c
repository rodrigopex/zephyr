/*
 * Copyright (c) 2022 Rodrigo Peixoto <rodrigopex@gmail.com>
 * SPDX-License-Identifier: Apache-2.0
 */
#include "calculator_scope.h"
#include <zephyr/kernel.h>
#include <zephyr/zbus/zbus.h>

ZBUS_CHAN_DEFINE(chan_calculator_in__command, struct msg_calculator__command, NULL, NULL,
		 ZBUS_OBSERVERS_EMPTY, ZBUS_MSG_INIT(0));

ZBUS_CHAN_DEFINE(chan_calculator_out__command_result, struct msg_calculator__command_result, NULL,
		 NULL, ZBUS_OBSERVERS_EMPTY, ZBUS_MSG_INIT(0));
