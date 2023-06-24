/*
 * Copyright (c) 2023 Rodrigo Peixoto <rodrigopex@gmail.com>
 * SPDX-License-Identifier: Apache-2.0
 */
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/zbus/zbus.h>
LOG_MODULE_DECLARE(zbus, CONFIG_ZBUS_LOG_LEVEL);

void mock(const struct zbus_channel *chan)
{
	LOG_INF("Data received on channel %s", zbus_chan_name(chan));
	LOG_HEXDUMP_INF(zbus_chan_const_msg(chan), zbus_chan_msg_size(chan), "Msg content:");
}

ZBUS_LISTENER_DEFINE(mock_lis, mock);
