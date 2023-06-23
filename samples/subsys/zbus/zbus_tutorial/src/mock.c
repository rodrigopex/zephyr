/*
 * Copyright (c) 2023 Rodrigo Peixoto <rodrigopex@gmail.com>
 * SPDX-License-Identifier: Apache-2.0
 */
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/zbus/zbus.h>
LOG_MODULE_DECLARE(zbus, CONFIG_ZBUS_LOG_LEVEL);

ZBUS_CHAN_DECLARE(chan_sensor_out__data, chan_lora_in__payload, chan_lora_out__transmission_done);

void mock(const struct zbus_channel *chan)
{
	LOG_INF("Data received on channel %s", zbus_chan_name(chan));
	LOG_HEXDUMP_INF(zbus_chan_const_msg(chan), zbus_chan_msg_size(chan), "Msg content:");
}

ZBUS_LISTENER_DEFINE(lis_mock, mock);

ZBUS_CHAN_ADD_OBS(chan_sensor_out__data, lis_mock, 3);
ZBUS_CHAN_ADD_OBS(chan_lora_in__payload, lis_mock, 3);
ZBUS_CHAN_ADD_OBS(chan_lora_out__transmission_done, lis_mock, 3);
