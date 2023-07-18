/*
 * Copyright (c) 2022 Rodrigo Peixoto <rodrigopex@gmail.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdint.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/zbus/zbus.h>
LOG_MODULE_REGISTER(msg_sample, 4);

struct acc_msg {
	int x;
	int y;
	int z;
};

ZBUS_CHAN_DEFINE(acc_data_chan,  /* Name */
		 struct acc_msg, /* Message type */

		 NULL, /* Validator */
		 NULL, /* User data */
		 ZBUS_OBSERVERS(bar_sub01, bar_msg_sub01, bar_msg_sub02, bar_msg_sub03,
				bar_msg_sub04, bar_msg_sub05, bar_msg_sub06, bar_msg_sub07,
				bar_msg_sub08, bar_msg_sub09, foo_lis), /* observers */
		 ZBUS_MSG_INIT(.x = 0, .y = 0, .z = 0)                  /* Initial value */
);

ZBUS_CHAN_DEFINE(acc_data2_chan, /* Name */
		 struct acc_msg, /* Message type */

		 NULL,                                   /* Validator */
		 NULL,                                   /* User data */
		 ZBUS_OBSERVERS(bar_msg_sub09, foo_lis), /* observers */
		 ZBUS_MSG_INIT(.x = 0, .y = 0, .z = 0)   /* Initial value */
);

static void listener_callback_example(const struct zbus_channel *chan)
{
	const struct acc_msg *acc = zbus_chan_const_msg(chan);

	LOG_INF("From listener -> Acc x=%d, y=%d, z=%d", acc->x, acc->y, acc->z);
}
ZBUS_LISTENER_DEFINE(foo_lis, listener_callback_example);

static void aaa_listener_callback_example(const struct zbus_channel *chan)
{
	const struct acc_msg *acc = zbus_chan_const_msg(chan);

	LOG_INF("From aaa listener -> Acc x=%d, y=%d, z=%d", acc->x, acc->y, acc->z);
}
ZBUS_LISTENER_DEFINE(aaa_lis, aaa_listener_callback_example);

ZBUS_MSG_SUBSCRIBER_DEFINE(bar_msg_sub01);
ZBUS_MSG_SUBSCRIBER_DEFINE(bar_msg_sub02);
ZBUS_MSG_SUBSCRIBER_DEFINE(bar_msg_sub03);
ZBUS_MSG_SUBSCRIBER_DEFINE(bar_msg_sub04);
ZBUS_MSG_SUBSCRIBER_DEFINE(bar_msg_sub05);
ZBUS_MSG_SUBSCRIBER_DEFINE(bar_msg_sub06);
ZBUS_MSG_SUBSCRIBER_DEFINE(bar_msg_sub07);
ZBUS_MSG_SUBSCRIBER_DEFINE(bar_msg_sub08);
ZBUS_MSG_SUBSCRIBER_DEFINE(bar_msg_sub09);
ZBUS_MSG_SUBSCRIBER_DEFINE(bar_msg_sub10);
ZBUS_MSG_SUBSCRIBER_DEFINE(bar_msg_sub11);
ZBUS_MSG_SUBSCRIBER_DEFINE(bar_msg_sub12);
ZBUS_MSG_SUBSCRIBER_DEFINE(bar_msg_sub13);
ZBUS_MSG_SUBSCRIBER_DEFINE(bar_msg_sub14);
ZBUS_MSG_SUBSCRIBER_DEFINE(bar_msg_sub15);
ZBUS_MSG_SUBSCRIBER_DEFINE(bar_msg_sub16);

ZBUS_SUBSCRIBER_DEFINE(bar_sub01, 4);
ZBUS_SUBSCRIBER_DEFINE(bar_sub02, 4);

static void msg_subscriber_task(void *sub)
{
	const struct zbus_channel *chan;

	struct acc_msg acc;

	const struct zbus_observer *subscriber = sub;

	zbus_obs_thread_attach(subscriber);

	while (!zbus_sub_wait_msg(subscriber, &chan, &acc, K_FOREVER)) {
		if (&acc_data_chan != chan && &acc_data2_chan != chan) {
			LOG_ERR("Wrong channel %p!", chan);

			continue;
		}
		LOG_INF("From msg subscriber %s -> Acc x=%d, y=%d, z=%d", zbus_obs_name(subscriber),
			acc.x, acc.y, acc.z);
	}
}

K_THREAD_DEFINE(subscriber_task_id1, CONFIG_MAIN_STACK_SIZE, msg_subscriber_task, &bar_msg_sub01,
		NULL, NULL, 3, 0, 0);
K_THREAD_DEFINE(subscriber_task_id2, CONFIG_MAIN_STACK_SIZE, msg_subscriber_task, &bar_msg_sub02,
		NULL, NULL, 3, 0, 0);
K_THREAD_DEFINE(subscriber_task_id3, CONFIG_MAIN_STACK_SIZE, msg_subscriber_task, &bar_msg_sub03,
		NULL, NULL, 3, 0, 0);
K_THREAD_DEFINE(subscriber_task_id4, CONFIG_MAIN_STACK_SIZE, msg_subscriber_task, &bar_msg_sub04,
		NULL, NULL, 3, 0, 0);
K_THREAD_DEFINE(subscriber_task_id5, CONFIG_MAIN_STACK_SIZE, msg_subscriber_task, &bar_msg_sub05,
		NULL, NULL, 3, 0, 0);
K_THREAD_DEFINE(subscriber_task_id6, CONFIG_MAIN_STACK_SIZE, msg_subscriber_task, &bar_msg_sub06,
		NULL, NULL, 3, 0, 0);
K_THREAD_DEFINE(subscriber_task_id7, CONFIG_MAIN_STACK_SIZE, msg_subscriber_task, &bar_msg_sub07,
		NULL, NULL, 3, 0, 0);
K_THREAD_DEFINE(subscriber_task_id8, CONFIG_MAIN_STACK_SIZE, msg_subscriber_task, &bar_msg_sub08,
		NULL, NULL, 3, 0, 0);
K_THREAD_DEFINE(subscriber_task_id9, CONFIG_MAIN_STACK_SIZE, msg_subscriber_task, &bar_msg_sub09,
		NULL, NULL, 3, 0, 0);
K_THREAD_DEFINE(subscriber_task_id10, CONFIG_MAIN_STACK_SIZE, msg_subscriber_task, &bar_msg_sub10,
		NULL, NULL, 3, 0, 0);
K_THREAD_DEFINE(subscriber_task_id11, CONFIG_MAIN_STACK_SIZE, msg_subscriber_task, &bar_msg_sub11,
		NULL, NULL, 3, 0, 0);
K_THREAD_DEFINE(subscriber_task_id12, CONFIG_MAIN_STACK_SIZE, msg_subscriber_task, &bar_msg_sub12,
		NULL, NULL, 3, 0, 0);
K_THREAD_DEFINE(subscriber_task_id13, CONFIG_MAIN_STACK_SIZE, msg_subscriber_task, &bar_msg_sub13,
		NULL, NULL, 3, 0, 0);
K_THREAD_DEFINE(subscriber_task_id14, CONFIG_MAIN_STACK_SIZE, msg_subscriber_task, &bar_msg_sub14,
		NULL, NULL, 3, 0, 0);
K_THREAD_DEFINE(subscriber_task_id15, CONFIG_MAIN_STACK_SIZE, msg_subscriber_task, &bar_msg_sub15,
		NULL, NULL, 3, 0, 0);
K_THREAD_DEFINE(subscriber_task_id16, CONFIG_MAIN_STACK_SIZE, msg_subscriber_task, &bar_msg_sub16,
		NULL, NULL, 3, 0, 0);

static void subscriber_task(void *sub)
{
	const struct zbus_channel *chan;

	struct acc_msg acc;

	const struct zbus_observer *subscriber = sub;

	zbus_obs_thread_attach(subscriber);

	while (!zbus_sub_wait(subscriber, &chan, K_FOREVER)) {
		if (&acc_data_chan != chan && &acc_data2_chan != chan) {
			LOG_ERR("Wrong channel %p!", chan);

			continue;
		}
		zbus_chan_read(chan, &acc, K_MSEC(250));

		LOG_INF("From subscriber %s -> Acc x=%d, y=%d, z=%d", zbus_obs_name(subscriber),
			acc.x, acc.y, acc.z);
	}
}

K_THREAD_DEFINE(subscriber_task_id17, CONFIG_MAIN_STACK_SIZE, subscriber_task, &bar_sub01, NULL,
		NULL, 2, 0, 0);
K_THREAD_DEFINE(subscriber_task_id18, CONFIG_MAIN_STACK_SIZE, subscriber_task, &bar_sub02, NULL,
		NULL, 3, 0, 0);

void my_timer_handler(struct k_timer *dummy)
{
	struct acc_msg acc = {.x = 0, .y = 0, .z = 0};

	zbus_chan_pub(&acc_data_chan, &acc, K_SECONDS(1));
}

K_TIMER_DEFINE(my_timer, my_timer_handler, NULL);

void my_timer_handler2(struct k_timer *dummy)
{
	struct acc_msg acc = {.x = 0, .y = 0, .z = 0};

	LOG_INF(" *** ISR: Publishing to %s channel", zbus_chan_name(&acc_data2_chan));
	zbus_chan_pub(&acc_data2_chan, &acc, K_SECONDS(1));
}

K_TIMER_DEFINE(my_timer2, my_timer_handler2, NULL);

ZBUS_CHAN_ADD_OBS(acc_data_chan, bar_sub02, 1);
ZBUS_CHAN_ADD_OBS(acc_data_chan, bar_msg_sub10, 3);
ZBUS_CHAN_ADD_OBS(acc_data_chan, bar_msg_sub11, 0);
ZBUS_CHAN_ADD_OBS(acc_data_chan, bar_msg_sub12, 2);

int main(void)
{
	int err;

	err = zbus_chan_add_obs(&acc_data_chan, &aaa_lis, K_NO_WAIT);
	__ASSERT_NO_MSG(err == 0);
	err = zbus_chan_add_obs(&acc_data_chan, &bar_msg_sub13, K_NO_WAIT);
	__ASSERT_NO_MSG(err == 0);
	err = zbus_chan_add_obs(&acc_data_chan, &bar_msg_sub14, K_NO_WAIT);
	__ASSERT_NO_MSG(err == 0);
	err = zbus_chan_add_obs(&acc_data_chan, &bar_msg_sub15, K_NO_WAIT);
	__ASSERT_NO_MSG(err == 0);
	err = zbus_chan_add_obs(&acc_data_chan, &bar_msg_sub16, K_NO_WAIT);
	__ASSERT_NO_MSG(err == 0);

	struct acc_msg acc = {.x = 1, .y = 10, .z = 100};

	printk("acc1 start %d, end %d\n", *acc_data_chan.observers_start_idx,
	       *acc_data_chan.observers_end_idx);
	printk("acc2 start %d, end %d\n", *acc_data2_chan.observers_start_idx,
	       *acc_data2_chan.observers_end_idx);
	/* start periodic timer that expires once every second */

	k_timer_start(&my_timer, K_SECONDS(3), K_SECONDS(3));
	k_timer_start(&my_timer2, K_SECONDS(2), K_SECONDS(2));

	while (1) {
		zbus_chan_pub(&acc_data_chan, &acc, K_SECONDS(1));
		acc.x += 1;
		acc.y += 10;
		acc.z += 100;

		k_msleep(1000);
	}

	return 0;
}
