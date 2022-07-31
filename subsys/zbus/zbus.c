/*
 * Copyright (c) 2022 Rodrigo Peixoto <rodrigopex@gmail.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/printk.h>
#include <zephyr/zbus/zbus.h>
LOG_MODULE_REGISTER(zbus, CONFIG_ZBUS_LOG_LEVEL);

#if defined(CONFIG_THREAD_NAME)
#define _ZBUS_CURRENT_THREAD_INFO k_thread_name_get(k_current_get()), k_current_get()
#else
#define _ZBUS_CURRENT_THREAD_INFO "_", k_current_get()
#endif /* CONFIG_THREAD_NAME */

k_timeout_t _zbus_timeout_remainder(uint64_t end_ticks)
{
	int64_t now_ticks = sys_clock_tick_get();

	return K_TICKS((k_ticks_t)MAX(end_ticks - now_ticks, 0));
}

void __weak _zbus_notify_runtime_listeners(struct zbus_channel *chan)
{
}

int __weak _zbus_notify_runtime_subscribers(struct zbus_channel *chan, uint64_t end_ticks)
{
	return 0;
}

static void _zbus_notify_listeners(struct zbus_channel *chan)
{
	for (const struct zbus_observer **obs = chan->observers; *obs != NULL; ++obs) {
		if ((*obs)->enabled && ((*obs)->callback != NULL)) {
			(*obs)->callback(chan);
		}
	}
	_zbus_notify_runtime_listeners(chan);
}

static int _zbus_notify_subscribers(struct zbus_channel *chan, uint64_t end_ticks)
{
	int err = 0;

	for (const struct zbus_observer **obs = chan->observers; *obs != NULL; ++obs) {
		if ((*obs)->enabled && ((*obs)->queue != NULL)) {
			err = k_msgq_put((*obs)->queue, &chan, _zbus_timeout_remainder(end_ticks));
			if (err) {
				return err;
			}
		}
	}
	return _zbus_notify_runtime_subscribers(chan, end_ticks);
}

int zbus_chan_pub(struct zbus_channel *chan, const void *msg, k_timeout_t timeout)
{
	int err;
	uint64_t end_ticks = sys_clock_timeout_end_calc(timeout);

	_ZBUS_ASSERT(!k_is_in_isr(), "zbus cannot be used inside ISRs");
	_ZBUS_ASSERT(chan != NULL, "chan is required");
	_ZBUS_ASSERT(msg != NULL, "msg is required");

	if (chan->read_only != 0) {
		return -EPERM;
	}

	if (chan->validator != NULL && !chan->validator(msg, chan->message_size)) {
		return -ENOMSG;
	}

	err = k_mutex_lock(chan->mutex, timeout);
	if (err) {
		return err;
	}

	memcpy(chan->message, msg, chan->message_size);

	_zbus_notify_listeners(chan);
	err = _zbus_notify_subscribers(chan, end_ticks);

	k_mutex_unlock(chan->mutex);

	return err;
}

int zbus_chan_read(const struct zbus_channel *chan, void *msg, k_timeout_t timeout)
{
	int err;

	_ZBUS_ASSERT(!k_is_in_isr(), "zbus cannot be used inside ISRs");
	_ZBUS_ASSERT(chan != NULL, "chan is required");
	_ZBUS_ASSERT(msg != NULL, "msg is required");

	err = k_mutex_lock(chan->mutex, timeout);
	if (err) {
		return err;
	}

	memcpy(msg, chan->message, chan->message_size);

	k_mutex_unlock(chan->mutex);

	return err;
}

int zbus_chan_notify(struct zbus_channel *chan, k_timeout_t timeout)
{
	int err;
	uint64_t end_ticks = sys_clock_timeout_end_calc(timeout);

	_ZBUS_ASSERT(!k_is_in_isr(), "zbus cannot be used inside ISRs");
	_ZBUS_ASSERT(chan != NULL, "chan is required");

	if (chan->read_only != 0) {
		return -EPERM;
	}

	err = k_mutex_lock(chan->mutex, timeout);
	if (err) {
		return err;
	}

	_zbus_notify_listeners(chan);
	err = _zbus_notify_subscribers(chan, end_ticks);

	k_mutex_unlock(chan->mutex);

	return err;
}

int zbus_chan_claim(const struct zbus_channel *chan, k_timeout_t timeout)
{
	int err;

	_ZBUS_ASSERT(!k_is_in_isr(), "zbus cannot be used inside ISRs");
	_ZBUS_ASSERT(chan != NULL, "chan is required");

	err = k_mutex_lock(chan->mutex, timeout);
	if (err) {
		return err;
	}

	return 0;
}

int zbus_chan_finish(const struct zbus_channel *chan)
{
	int err;

	_ZBUS_ASSERT(!k_is_in_isr(), "zbus cannot be used inside ISRs");
	_ZBUS_ASSERT(chan != NULL, "chan is required");

	err = k_mutex_unlock(chan->mutex);

	return err;
}

#if defined(CONFIG_ZBUS_CHANNEL_NAME)
const char *zbus_chan_name(const struct zbus_channel *chan)
{
	return chan->name;
}
#endif

void *zbus_chan_msg(struct zbus_channel *chan)
{
	if (chan->read_only) {
		LOG_ERR("Invalid read-only message access.");
		return NULL;
	}

	return chan->message;
}

void *zbus_chan_const_msg(const struct zbus_channel *chan)
{
	return chan->message;
}

uint16_t zbus_chan_msg_size(const struct zbus_channel *chan)
{
	return chan->message_size;
}

#if CONFIG_ZBUS_CHANNEL_USER_DATA_SIZE > 0
void *zbus_chan_user_data(const struct zbus_channel *chan)
{
	return chan->user_data;
}
#endif

int zbus_obs_set_enable(struct zbus_observer *obs, bool enabled)
{
	_ZBUS_ASSERT(obs != NULL, "obs is required");

	obs->enabled = enabled;

	return 0;
}

#if defined(CONFIG_ZBUS_OBSERVER_NAME)
const char *zbus_obs_name(const struct zbus_observer *obs)
{
	return obs->name;
}
#endif

int zbus_sub_wait(struct zbus_observer *sub, struct zbus_channel **chan, k_timeout_t timeout)
{
	_ZBUS_ASSERT(!k_is_in_isr(), "zbus cannot be used inside ISRs");
	_ZBUS_ASSERT(sub != NULL, "sub is required");
	_ZBUS_ASSERT(chan != NULL, "chan is required");

	if (sub->queue == NULL) {
		return -EINVAL;
	}

	return k_msgq_get(sub->queue, chan, timeout);
}
