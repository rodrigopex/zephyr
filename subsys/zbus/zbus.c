/*
 * Copyright (c) 2022 Rodrigo Peixoto <rodrigopex@gmail.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/sys/iterable_sections.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/buf.h>
#include <zephyr/zbus/zbus.h>
LOG_MODULE_REGISTER(zbus, CONFIG_ZBUS_LOG_LEVEL);

#if defined(CONFIG_ZBUS_MSG_SUBSCRIBER)

NET_BUF_POOL_HEAP_DEFINE(_zbus_msg_subscribers_pool, CONFIG_ZBUS_MSG_SUBSCRIBER_NET_BUF_POOL_SIZE,
			 0, NULL);

#endif /* CONFIG_ZBUS_MSG_SUBSCRIBER */
int _zbus_init(void)
{
	const struct zbus_channel *curr = NULL;
	const struct zbus_channel *prev = NULL;

	STRUCT_SECTION_FOREACH(zbus_channel_observation, observation) {
		curr = observation->chan;

		if (prev != curr) {
			if (prev == NULL) {
				*curr->observers_start_idx = 0;
				*curr->observers_end_idx = 0;
			} else {
				*curr->observers_start_idx = *prev->observers_end_idx;
				*curr->observers_end_idx = *prev->observers_end_idx;
			}
			prev = curr;
		}

		++(*curr->observers_end_idx);
	}
	return 0;
}
SYS_INIT(_zbus_init, APPLICATION, CONFIG_ZBUS_CHANNELS_SYS_INIT_PRIORITY);

k_timeout_t _zbus_timeout_remainder(uint64_t end_ticks)
{
	int64_t now_ticks = sys_clock_tick_get();

	return K_TICKS((k_ticks_t)MAX(end_ticks - now_ticks, 0));
}

static inline int _zbus_vded_exec(const struct zbus_channel *chan, uint64_t end_ticks)
{
	int last_error = 0;

	__ASSERT(chan != NULL, "chan is required");

	struct zbus_channel_observation *observation;

#if defined(CONFIG_ZBUS_MSG_SUBSCRIBER)
	/* Notify message subscribers */
	struct net_buf *buf =
		net_buf_alloc_len(&_zbus_msg_subscribers_pool,
				  sizeof(const struct zbus_channel *) + zbus_chan_msg_size(chan),
				  _zbus_timeout_remainder(end_ticks));

	_ZBUS_ASSERT(buf != NULL, "net_buf zbus_msg_subscribers_pool is "
				  "unavailable or heap is full");

	net_buf_add_mem(buf, zbus_chan_msg(chan), zbus_chan_msg_size(chan));
	net_buf_add_mem(buf, &chan, sizeof(const struct zbus_channel *));

#endif /* CONFIG_ZBUS_MSG_SUBSCRIBER */

	for (int16_t i = *chan->observers_start_idx, limit = *chan->observers_end_idx; i < limit;
	     ++i) {
		STRUCT_SECTION_GET(zbus_channel_observation, i, &observation);

		__ASSERT(observation != NULL, "observation must be not NULL");

		if (!observation->obs->enabled) {
			continue;
		}

		if (observation->obs->callback != NULL) {
			observation->obs->callback(chan);
		}

#if defined(CONFIG_ZBUS_MSG_SUBSCRIBER)
		else if (observation->obs->message_fifo != NULL) {
			struct net_buf *cloned_buf =
				net_buf_clone(buf, _zbus_timeout_remainder(end_ticks));

			_ZBUS_ASSERT(cloned_buf != NULL,
				     "net_buf zbus_msg_subscribers_pool is full or unavailable");
			net_buf_put(observation->obs->message_fifo, cloned_buf);
		}
#endif /* CONFIG_ZBUS_MSG_SUBSCRIBER */

		else if (observation->obs->notification_queue != NULL) {
			int err = k_msgq_put(observation->obs->notification_queue, &chan,
					     _zbus_timeout_remainder(end_ticks));

			_ZBUS_ASSERT(err == 0,
				     "could not deliver notification to observer %s. Error code %d",
				     _ZBUS_OBS_NAME(observation->obs), err);

			if (err) {
				last_error = err;
			}
		}
	}

#if defined(CONFIG_ZBUS_RUNTIME_OBSERVERS)
	struct zbus_observer_node *obs_nd, *tmp;

	SYS_SLIST_FOR_EACH_CONTAINER_SAFE(chan->observers, obs_nd, tmp, node) {

		__ASSERT(obs_nd != NULL, "observer node is NULL");

		if (!obs_nd->obs->enabled) {
			continue;
		}

		if (obs_nd->obs->callback != NULL) {
			obs_nd->obs->callback(chan);
		}

#if defined(CONFIG_ZBUS_MSG_SUBSCRIBER)
		else if (obs_nd->obs->message_fifo != NULL) {

			struct net_buf *cloned_buf =
				net_buf_clone(buf, _zbus_timeout_remainder(end_ticks));

			_ZBUS_ASSERT(cloned_buf != NULL, "net_buf zbus_msg_subscribers_pool is "
							 "full or unavailable");
			net_buf_put(obs_nd->obs->message_fifo, cloned_buf);

		}
#endif /* CONFIG_ZBUS_MSG_SUBSCRIBER */

		else if (obs_nd->obs->notification_queue != NULL) {
			last_error = k_msgq_put(obs_nd->obs->notification_queue, &chan,
						_zbus_timeout_remainder(end_ticks));
		}
	}

#if defined(CONFIG_ZBUS_MSG_SUBSCRIBER)
	net_buf_unref(buf);
#endif /* CONFIG_ZBUS_MSG_SUBSCRIBER */

#endif /* CONFIG_ZBUS_RUNTIME_OBSERVERS */

	return last_error;
}

static void _zbus_chan_highest_priority_observer_update(const struct zbus_channel *chan)
{
	__ASSERT(chan != NULL, "chan is required");

	struct zbus_channel_observation *observation;

	*chan->highest_observer_priority = K_LOWEST_APPLICATION_THREAD_PRIO;

	for (int16_t i = *chan->observers_start_idx, limit = *chan->observers_end_idx; i < limit;
	     ++i) {
		STRUCT_SECTION_GET(zbus_channel_observation, i, &observation);

		if (observation->obs->enabled && (observation->obs->priority > 0)) {
			if (*chan->highest_observer_priority > *observation->obs->priority) {
				*chan->highest_observer_priority = *observation->obs->priority;
			}
		}
	}
}

static inline int _zbus_chan_smart_lock(const struct zbus_channel *chan, k_timeout_t timeout,
					int *prio)
{
	if (k_is_in_isr()) {
		return k_sem_take(chan->sem, K_NO_WAIT);
	} else if (IS_ENABLED(CONFIG_ZBUS_SMART_LOCK)) {
		int current_thread_priority = k_thread_priority_get(k_current_get());

		if (current_thread_priority > *chan->highest_observer_priority) {
			*prio = current_thread_priority;

			int p = *chan->highest_observer_priority - 1;

			LOG_DBG("Elevating publisher priority from %d to %d",
				current_thread_priority, MAX(p, 0));

			k_thread_priority_set(k_current_get(), MAX(p, 0));
		} else {
			*prio = -1;
		}

		int err = k_sem_take(chan->sem, timeout);
		if (err) {
			LOG_ERR("Something went wrong. Restoring publisher priority");

			k_thread_priority_set(k_current_get(), current_thread_priority);

			return err;
		}
	} else {
		return k_sem_take(chan->sem, timeout);
	}

	return 0;
}

static inline void _zbus_chan_smart_unlock(const struct zbus_channel *chan, int prio)
{
	k_sem_give(chan->sem);

	if (IS_ENABLED(CONFIG_ZBUS_SMART_LOCK) && !k_is_in_isr() && prio >= 0) {
		__ASSERT_NO_MSG((k_thread_priority_get(k_current_get()) + 1) ==
				*chan->highest_observer_priority);

		LOG_DBG("Restoring publisher priority from %d to %d",
			k_thread_priority_get(k_current_get()), prio);

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

	int context_priority = K_LOWEST_APPLICATION_THREAD_PRIO;

	err = _zbus_chan_smart_lock(chan, timeout, &context_priority);
	if (err) {
		return err;
	}

	memcpy(chan->message, msg, chan->message_size);

	err = _zbus_vded_exec(chan, end_ticks);

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

	int context_priority = K_LOWEST_APPLICATION_THREAD_PRIO;

	err = _zbus_chan_smart_lock(chan, timeout, &context_priority);
	if (err) {
		return err;
	}

	err = _zbus_vded_exec(chan, end_ticks);

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

#if defined(CONFIG_ZBUS_SMART_LOCK)
int zbus_obs_thread_attach(const struct zbus_observer *obs)
{
	int prio = k_thread_priority_get(k_current_get());

	_ZBUS_ASSERT(!k_is_in_isr(), "zbus subscribers cannot be used inside ISRs");
	_ZBUS_ASSERT(obs != NULL, "obs is required");

	if (*obs->priority && *obs->priority > prio) {
		*obs->priority = prio;
	}

	STRUCT_SECTION_FOREACH(zbus_channel_observation, observation) {
		if (observation->obs == obs) {
			_zbus_chan_highest_priority_observer_update(observation->chan);
		}
	}

	return 0;
}
#endif /* CONFIG_ZBUS_SMART_LOCK */

int zbus_obs_set_enable(struct zbus_observer *obs, bool enabled)
{
	_ZBUS_ASSERT(obs != NULL, "obs is required");

	obs->enabled = enabled;

	STRUCT_SECTION_FOREACH(zbus_channel_observation, observation) {
		if (observation->obs == obs) {
			_zbus_chan_highest_priority_observer_update(observation->chan);
		}
	}

	return 0;
}
