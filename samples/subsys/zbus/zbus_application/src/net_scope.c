/*
 * Copyright (c) 2023 Rodrigo Peixoto <rodrigopex@gmail.com>
 * SPDX-License-Identifier: Apache-2.0
 */
#include "net_scope.h"
#include <zephyr/kernel.h>
#include <zephyr/zbus/zbus.h>

ZBUS_CHAN_DEFINE(chan_net_out__transmission_done, struct msg_net__transmission_result, NULL, NULL,
		 ZBUS_OBSERVERS_EMPTY, ZBUS_MSG_INIT(0));
