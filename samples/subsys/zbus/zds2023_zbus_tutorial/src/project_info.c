/*
 * Copyright (c) 2023 Rodrigo Peixoto <rodrigopex@gmail.com>
 * SPDX-License-Identifier: Apache-2.0
 */
#include "project_info.h"

#include <zephyr/kernel.h>
#include <zephyr/zbus/zbus.h>

ZBUS_CHAN_DEFINE(project_info_chan,       /* Channel name */
		 struct project_info_msg, /* Message type */
		 NULL,                    /* User data */
		 NULL,                    /* Validator */
		 ZBUS_OBSERVERS_EMPTY,    /* Observers */
		 ZBUS_MSG_INIT(.firmware_version = {0, 1, 2}, .hardware_version = {'V', "01A"},
			       .serial_number = "bNojO6UjtM0gIflI9HcAVPDqF7vX1aH7",
			       .model = "ZBusIoT1"));
