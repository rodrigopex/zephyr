/*
 * Copyright (c) 2023 Rodrigo Peixoto <rodrigopex@gmail.com>
 * SPDX-License-Identifier: Apache-2.0
 */
#include "app_info.h"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/init.h>
#include <zephyr/zbus/zbus.h>
LOG_MODULE_DECLARE(zbus, CONFIG_ZBUS_LOG_LEVEL);

ZBUS_CHAN_DECLARE(chan_app_info, chan_sensor_out__data, chan_sensor_in__fetch,
		  chan_lora_in__payload, chan_lora_out__transmission_done);

struct k_work start_trigger_work;

void execute_start_trigger(struct k_work *work)
{
	zbus_chan_notify(&chan_sensor_in__fetch, K_MSEC(250));
}

void trigger_timer_handler(struct k_timer *dummy)
{
	k_work_submit(&start_trigger_work);
}

K_TIMER_DEFINE(trigger_timer, trigger_timer_handler, NULL);

int main(void)
{
	/* Pretend to fetch some data from external source to describe hardware and the samples
	 * timeout */
	//...
	zbus_chan_claim(&chan_app_info, K_FOREVER);

	struct app_info_msg *app_info = (struct app_info_msg *)zbus_chan_msg(&chan_app_info);

	app_info->hardware_version.major = 'P';

	strncpy(app_info->hardware_version.minor, "07X", 4);

	app_info->sample_timeout = 3000;

	bool *locked = zbus_chan_user_data(&chan_app_info);
	*locked = true;

	zbus_chan_finish(&chan_app_info);

	/* Is using app_info safe?!?! */
	LOG_INF("%s (serial number:%s)", app_info->model, app_info->serial_number);

	LOG_INF(" - Firmware v%u.%u.%u", app_info->firmware_version.major,
		app_info->firmware_version.minor, app_info->firmware_version.patch);

	LOG_INF(" - Hardware %c%s", app_info->hardware_version.major,
		app_info->hardware_version.minor);

	/* while (1) { */
	/* 	zbus_chan_notify(&start_trigger_chan, K_FOREVER); */
	/**/
	/* 	k_msleep(2000); */
	/* } */

	k_work_init(&start_trigger_work, execute_start_trigger);

	k_timer_start(&trigger_timer, K_MSEC(app_info->sample_timeout),
		      K_MSEC(app_info->sample_timeout));

	LOG_WRN("app_info pub (err == -ENOMSG) -> %d",
		zbus_chan_pub(&chan_app_info, app_info, K_FOREVER) == -ENOMSG);

	return 0;
}
