/*
 * Copyright (c) 2023 Rodrigo Peixoto <rodrigopex@gmail.com>
 * SPDX-License-Identifier: Apache-2.0
 */
#include "app_info.h"

#include <zephyr/kernel.h>
#include <zephyr/zbus/zbus.h>

static bool locked = false;

static bool validator(const void *msg, size_t msg_size);

ZBUS_CHAN_DEFINE(chan_app_info,       /* Channel name */
		 struct app_info_msg, /* Message type */

		 validator,            /* Validator */
		 &locked,              /* User data */
		 ZBUS_OBSERVERS_EMPTY, /* Observers */
		 ZBUS_MSG_INIT(.firmware_version = {0, 1, 2}, .hardware_version = {'V', "01A"},
			       .serial_number = "bNojO6UjtM0gIflI9HcAVPDqF7vX1aH7",
			       .model = "ZBusIoT1"));

static bool validator(const void *msg, size_t msg_size)
{
	bool *locked = zbus_chan_user_data(&chan_app_info);
	return (*locked == false);
}
