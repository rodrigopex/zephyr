/*
 * Copyright (c) 2023 Rodrigo Peixoto <rodrigopex@gmail.com>
 * SPDX-License-Identifier: Apache-2.0
 */
#include "project_info.h"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/zbus/zbus.h>
LOG_MODULE_DECLARE(zbus, CONFIG_ZBUS_LOG_LEVEL);

ZBUS_CHAN_DECLARE(project_info_chan);

int main(void)
{
	struct project_info_msg *prj_info =
		(struct project_info_msg *)zbus_chan_const_msg(&project_info_chan);

	LOG_INF("%s (serial:%s)", prj_info->model, prj_info->serial_number);

	LOG_INF(" - Firmware v%u.%u.%u", prj_info->firmware_version.major,
		prj_info->firmware_version.minor, prj_info->firmware_version.patch);

	LOG_INF(" - Hardware %c%s", prj_info->hardware_version.major,
		prj_info->hardware_version.minor);

	return 0;
}
