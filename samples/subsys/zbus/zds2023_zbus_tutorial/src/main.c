/*
 * Copyright (c) 2023 Rodrigo Peixoto <rodrigopex@gmail.com>
 * SPDX-License-Identifier: Apache-2.0
 */
#include "project_info.h"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/zbus/zbus.h>
LOG_MODULE_DECLARE(zbus, CONFIG_ZBUS_LOG_LEVEL);

ZBUS_CHAN_DECLARE(project_info_chan, sensor_data_chan);

ZBUS_OBS_DECLARE(sensor_thread_sub, core_thread_sub, lora_thread_sub, mock_lis);

ZBUS_CHAN_DEFINE(start_trigger_chan,                /* Channel name */
		 bool,                              /* Message type */
		 NULL,                              /* User data */
		 NULL,                              /* Validator */
		 ZBUS_OBSERVERS(sensor_thread_sub), /* Observers */
		 false);

ZBUS_CHAN_DEFINE(payload_chan,                    /* Channel name */
		 uint64_t,                        /* Message type */
		 NULL,                            /* User data */
		 NULL,                            /* Validator */
		 ZBUS_OBSERVERS(lora_thread_sub), /* Observers */
		 0);

ZBUS_CHAN_DEFINE(transmission_done_chan, /* Channel name */
		 bool,                   /* Message type */
		 NULL,                   /* User data */
		 NULL,                   /* Validator */
		 ZBUS_OBSERVERS_EMPTY,   /* Observers */
		 false);

struct k_work start_trigger_work;

void execute_start_trigger(struct k_work *work)
{
	zbus_chan_notify(&start_trigger_chan, K_MSEC(250));
}

void trigger_timer_handler(struct k_timer *dummy)
{
	k_work_submit(&start_trigger_work);
}

K_TIMER_DEFINE(trigger_timer, trigger_timer_handler, NULL);

int main(void)
{
	struct project_info_msg *prj_info =
		(struct project_info_msg *)zbus_chan_const_msg(&project_info_chan);

	LOG_INF("%s (serial:%s)", prj_info->model, prj_info->serial_number);

	LOG_INF(" - Firmware v%u.%u.%u", prj_info->firmware_version.major,
		prj_info->firmware_version.minor, prj_info->firmware_version.patch);

	LOG_INF(" - Hardware %c%s", prj_info->hardware_version.major,
		prj_info->hardware_version.minor);

	/* Adding a mock for testing the current status of the system */
	zbus_chan_add_obs(&sensor_data_chan, &core_thread_sub, K_NO_WAIT);
	zbus_chan_add_obs(&sensor_data_chan, &mock_lis, K_NO_WAIT);
	zbus_chan_add_obs(&payload_chan, &mock_lis, K_NO_WAIT);
	zbus_chan_add_obs(&transmission_done_chan, &mock_lis, K_NO_WAIT);

	/* while (1) { */
	/* 	zbus_chan_notify(&start_trigger_chan, K_FOREVER); */
	/**/
	/* 	k_msleep(2000); */
	/* } */

	k_work_init(&start_trigger_work, execute_start_trigger);

	k_timer_start(&trigger_timer, K_SECONDS(2), K_SECONDS(2));

	return 0;
}
