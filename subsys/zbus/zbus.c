/*
 * Copyright (c) 2022 Rodrigo Peixoto <rodrigopex@gmail.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/buf.h>
#include <zephyr/zbus/zbus.h>
LOG_MODULE_REGISTER(zbus, CONFIG_ZBUS_LOG_LEVEL);

#if defined(CONFIG_ZBUS_MSG_SUBSCRIBER)

NET_BUF_POOL_HEAP_DEFINE(_zbus_msg_subscribers_pool, CONFIG_ZBUS_MSG_SUBSCRIBER_NET_BUF_POOL_SIZE,
			 0, NULL);

#endif /* CONFIG_ZBUS_MSG_SUBSCRIBER */

k_timeout_t _zbus_timeout_remainder(uint64_t end_ticks)
{
	int64_t now_ticks = sys_clock_tick_get();

	return K_TICKS((k_ticks_t)MAX(end_ticks - now_ticks, 0));
}

static inline void _zbus_notify_immediate(const struct zbus_channel *chan, uint64_t end_ticks)
{
	__ASSERT(chan != NULL, "chan is required");

	struct zbus_observer_node *obs_nd, *tmp;

	SYS_SLIST_FOR_EACH_CONTAINER_SAFE(chan->observers, obs_nd, tmp, node) {

		__ASSERT(obs_nd != NULL, "observer node is NULL");

		if (obs_nd->obs->enabled && (obs_nd->obs->callback != NULL)) {
			obs_nd->obs->callback(chan);
		}
	}

#if defined(CONFIG_ZBUS_MSG_SUBSCRIBER)
	/* Notify message subscribers */
	struct net_buf *buf =
		net_buf_alloc_len(&_zbus_msg_subscribers_pool,
				  sizeof(const struct zbus_channel *) + zbus_chan_msg_size(chan),
				  _zbus_timeout_remainder(end_ticks));

	_ZBUS_ASSERT(buf != NULL,
		     "net_buf zbus_msg_subscribers_pool is unavailable or heap is full");

	net_buf_add_mem(buf, zbus_chan_msg(chan), zbus_chan_msg_size(chan));
	net_buf_add_mem(buf, &chan, sizeof(const struct zbus_channel *));

	SYS_SLIST_FOR_EACH_CONTAINER_SAFE(chan->observers, obs_nd, tmp, node) {
		if (obs_nd->obs->enabled && (obs_nd->obs->message_fifo != NULL)) {
			struct net_buf *cloned_buf =
				net_buf_clone(buf, _zbus_timeout_remainder(end_ticks));

			_ZBUS_ASSERT(cloned_buf != NULL,
				     "net_buf zbus_msg_subscribers_pool is full or unavailable");
			net_buf_put(obs_nd->obs->message_fifo, cloned_buf);
		}
	}
	net_buf_unref(buf);

#endif /* CONFIG_ZBUS_MSG_SUBSCRIBER */
}

static inline int _zbus_notify_subscribers(const struct zbus_channel *chan, uint64_t end_ticks)
{
	__ASSERT(chan != NULL, "chan is required");

	int last_error = 0, err;
	struct zbus_observer_node *obs_nd, *tmp;

	SYS_SLIST_FOR_EACH_CONTAINER_SAFE(chan->observers, obs_nd, tmp, node) {

		__ASSERT(obs_nd != NULL, "observer node is NULL");

		if (obs_nd->obs->enabled && (obs_nd->obs->notification_queue != NULL)) {
			err = k_msgq_put(obs_nd->obs->notification_queue, &chan,
					 _zbus_timeout_remainder(end_ticks));

			_ZBUS_ASSERT(err == 0,
				     "could not deliver notification to observer %s. Error code %d",
				     _ZBUS_OBS_NAME(obs_nd->obs), err);

			if (err) {
				last_error = err;
			}
		}
	}

	return last_error;
}

static void _zbus_observer_priority_set(const struct zbus_observer *obs, int prio)
{
	__ASSERT(obs != NULL, "obs is required");

	if (*obs->priority && *obs->priority > prio) {
		*obs->priority = prio;
	}
}

static void _zbus_chan_highest_priority_observer_update(const struct zbus_channel *chan)
{
	__ASSERT(chan != NULL, "chan is required");

	struct zbus_observer_node *obs_nd, *tmp;

	SYS_SLIST_FOR_EACH_CONTAINER_SAFE(chan->observers, obs_nd, tmp, node) {

		__ASSERT(obs_nd != NULL, "observer node is NULL");

		if (obs_nd->obs->enabled && (obs_nd->obs->priority > 0)) {
			if (*chan->highest_observer_priority > *obs_nd->obs->priority) {
				*chan->highest_observer_priority = *obs_nd->obs->priority;
			}
		}
	}
}

static int _zbus_chan_smart_lock(const struct zbus_channel *chan, k_timeout_t timeout, int *prio)
{
	if (k_is_in_isr()) {
		return k_sem_take(chan->sem, K_NO_WAIT);
	}

	int current_thread_priority = k_thread_priority_get(k_current_get());

	_zbus_chan_highest_priority_observer_update(chan);

	if (current_thread_priority > *chan->highest_observer_priority) {
		*prio = current_thread_priority;

		LOG_DBG("Elevating publisher priority from %d to %d", current_thread_priority,
			*chan->highest_observer_priority);
		int p = *chan->highest_observer_priority - 1;
		k_thread_priority_set(k_current_get(), MAX(p, 0));
	}

	int err = k_sem_take(chan->sem, timeout);
	if (err) {
		LOG_ERR("Something went wrong. Restoring publisher priority");

		k_thread_priority_set(k_current_get(), current_thread_priority);

		return err;
	}

	return 0;
}

static void _zbus_chan_smart_unlock(const struct zbus_channel *chan, int prio)
{
	k_sem_give(chan->sem);

	if (!k_is_in_isr()) {
		LOG_DBG("Restoring publisher priority from %d to %d",
			*chan->highest_observer_priority, prio);

		k_thread_priority_set(k_current_get(), prio);
	}
}

int zbus_chan_pub(const struct zbus_channel *chan, const void *msg, k_timeout_t timeout)
{
	int err;

	if (k_is_in_isr()) {
		timeout = K_NO_WAIT;
	}

	uint64_t end_ticks = sys_clock_timeout_end_calc(timeout);

	_ZBUS_ASSERT(chan != NULL, "chan is required");
	_ZBUS_ASSERT(msg != NULL, "msg is required");

	if (chan->validator != NULL && !chan->validator(msg, chan->message_size)) {
		return -ENOMSG;
	}

	int context_priority = CONFIG_NUM_PREEMPT_PRIORITIES - 1;

	err = _zbus_chan_smart_lock(chan, timeout, &context_priority);
	if (err) {
		return err;
	}

	memcpy(chan->message, msg, chan->message_size);

	_zbus_notify_immediate(chan, end_ticks);

	err = _zbus_notify_subscribers(chan, end_ticks);

	_zbus_chan_smart_unlock(chan, context_priority);

	return err;
}

int zbus_chan_read(const struct zbus_channel *chan, void *msg, k_timeout_t timeout)
{
	int err;

	_ZBUS_ASSERT(chan != NULL, "chan is required");
	_ZBUS_ASSERT(msg != NULL, "msg is required");

	err = k_sem_take(chan->sem, timeout);
	if (err) {
		return err;
	}

	memcpy(msg, chan->message, chan->message_size);

	k_sem_give(chan->sem);

	return 0;
}

int zbus_chan_notify(const struct zbus_channel *chan, k_timeout_t timeout)
{
	int err;

	_ZBUS_ASSERT(chan != NULL, "chan is required");

	if (k_is_in_isr()) {
		timeout = K_NO_WAIT;
	}

	uint64_t end_ticks = sys_clock_timeout_end_calc(timeout);

	int context_priority = CONFIG_NUM_PREEMPT_PRIORITIES - 1;

	err = _zbus_chan_smart_lock(chan, timeout, &context_priority);
	if (err) {
		return err;
	}

	_zbus_notify_immediate(chan, end_ticks);

	err = _zbus_notify_subscribers(chan, end_ticks);

	_zbus_chan_smart_unlock(chan, context_priority);

	return err;
}

int zbus_chan_claim(const struct zbus_channel *chan, k_timeout_t timeout)
{
	_ZBUS_ASSERT(chan != NULL, "chan is required");

	if (k_is_in_isr()) {
		timeout = K_NO_WAIT;
	}

	return k_sem_take(chan->sem, timeout);
}

int zbus_chan_finish(const struct zbus_channel *chan)
{
	_ZBUS_ASSERT(chan != NULL, "chan is required");

	k_sem_give(chan->sem);

	return 0;
}

int zbus_sub_wait(const struct zbus_observer *sub, const struct zbus_channel **chan,
		  k_timeout_t timeout)
{
	_ZBUS_ASSERT(!k_is_in_isr(), "zbus subscribers cannot be used inside ISRs");
	_ZBUS_ASSERT(sub != NULL, "sub is required");
	_ZBUS_ASSERT(chan != NULL, "chan is required");

	_zbus_observer_priority_set(sub, k_thread_priority_get(k_current_get()));

	if (sub->notification_queue == NULL) {
		return -EINVAL;
	}

	return k_msgq_get(sub->notification_queue, chan, timeout);
}

#if defined(CONFIG_ZBUS_MSG_SUBSCRIBER)

int zbus_sub_wait_msg(const struct zbus_observer *sub, const struct zbus_channel **chan, void *msg,
		      k_timeout_t timeout)
{
	_ZBUS_ASSERT(!k_is_in_isr(), "zbus subscribers cannot be used inside ISRs");
	_ZBUS_ASSERT(sub != NULL, "sub is required");
	_ZBUS_ASSERT(chan != NULL, "chan is required");
	_ZBUS_ASSERT(msg != NULL, "msg is required");

	_zbus_observer_priority_set(sub, k_thread_priority_get(k_current_get()));

	if (sub->message_fifo == NULL) {
		return -EINVAL;
	}

	struct net_buf *buf = net_buf_get(sub->message_fifo, timeout);
	if (buf == NULL) {
		return -ENODATA;
	}

	memcpy(chan, net_buf_remove_mem(buf, sizeof(const struct zbus_channel *)),
	       sizeof(const struct zbus_channel *));

	if (*chan == NULL) {
		net_buf_unref(buf);

		return -EILSEQ;
	}

	memcpy(msg, net_buf_remove_mem(buf, zbus_chan_msg_size(*chan)), zbus_chan_msg_size(*chan));

	net_buf_unref(buf);

	return 0;
}

#endif /* CONFIG_ZBUS_MSG_SUBSCRIBER */
