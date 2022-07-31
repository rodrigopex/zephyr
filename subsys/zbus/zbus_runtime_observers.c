/*
 * Copyright (c) 2022 Rodrigo Peixoto <rodrigopex@gmail.com>
 * SPDX-License-Identifier: Apache-2.0
 */
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/zbus/zbus.h>
LOG_MODULE_DECLARE(zbus, CONFIG_ZBUS_LOG_LEVEL);

struct zbus_observer_node {
	sys_snode_t node;
	struct zbus_observer *obs;
};

static struct zbus_observer_node _zbus_runtime_obs_pool[CONFIG_ZBUS_RUNTIME_OBSERVERS_POOL_SIZE];
static uint8_t pool_watermark;
static uint8_t pool_used;

void _zbus_notify_runtime_listeners(struct zbus_channel *chan)
{
	struct zbus_observer_node *obs_nd, *tmp;

	SYS_SLIST_FOR_EACH_CONTAINER_SAFE(&chan->runtime_observers, obs_nd, tmp, node) {
		__ASSERT(obs_nd != NULL, "observer node is NULL");
		if (obs_nd->obs->enabled && (obs_nd->obs->callback != NULL)) {
			obs_nd->obs->callback(chan);
		}
	}
}

int _zbus_notify_runtime_subscribers(struct zbus_channel *chan, uint64_t end_ticks)
{
	int err = 0;
	struct zbus_observer_node *obs_nd, *tmp;

	SYS_SLIST_FOR_EACH_CONTAINER_SAFE(&chan->runtime_observers, obs_nd, tmp, node) {
		__ASSERT(obs_nd != NULL, "observer node is NULL");
		if (obs_nd->obs->enabled && (obs_nd->obs->queue != NULL)) {
			err = k_msgq_put(obs_nd->obs->queue, &chan,
					 _zbus_timeout_remainder(end_ticks));
			if (err) {
				return err;
			}
		}
	}
	return err;
}

uint8_t zbus_runtime_obs_pool_available(void)
{
	return CONFIG_ZBUS_RUNTIME_OBSERVERS_POOL_SIZE - pool_used;
}

uint8_t zbus_runtime_obs_pool_watermark(void)
{
	return pool_watermark;
}

int zbus_chan_add_obs(struct zbus_channel *chan, struct zbus_observer *obs, k_timeout_t timeout)
{
	int err;
	struct zbus_observer_node *obs_nd, *tmp;

	_ZBUS_ASSERT(!k_is_in_isr(), "ISR blocked");
	_ZBUS_ASSERT(chan != NULL, "chan is required");
	_ZBUS_ASSERT(obs != NULL, "obs is required");

	if (chan->read_only != 0) {
		return -EPERM;
	}

	/* Check if the observer is already a static observer */
	for (const struct zbus_observer **static_obs = chan->observers; *static_obs != NULL;
	     ++static_obs) {
		if (*static_obs == obs) {
			return -EEXIST;
		}
	}

	err = k_mutex_lock(chan->mutex, timeout);
	if (err) {
		return err;
	}

	/* Check if the observer is already a runtime observer */
	SYS_SLIST_FOR_EACH_CONTAINER_SAFE(&chan->runtime_observers, obs_nd, tmp, node) {
		__ASSERT(obs_nd != NULL, "observer node is NULL");
		if (obs_nd->obs == obs) {
			k_mutex_unlock(chan->mutex);
			return -EALREADY;
		}
	}

	for (int i = 0; i < CONFIG_ZBUS_RUNTIME_OBSERVERS_POOL_SIZE; ++i) {
		if (_zbus_runtime_obs_pool[i].obs == NULL) {
			_zbus_runtime_obs_pool[i].obs = obs;
			sys_slist_append(&chan->runtime_observers, &_zbus_runtime_obs_pool[i].node);

			k_mutex_unlock(chan->mutex);

			++pool_used;
			if (pool_used > pool_watermark) {
				pool_watermark = pool_used;
			}

			return 0;
		}
	}

	k_mutex_unlock(chan->mutex);

	return -ENOBUFS;
}

int zbus_chan_rm_obs(struct zbus_channel *chan, struct zbus_observer *obs, k_timeout_t timeout)
{
	int err;
	struct zbus_observer_node *obs_nd, *tmp;
	struct zbus_observer_node *prev_obs_nd = NULL;

	_ZBUS_ASSERT(!k_is_in_isr(), "ISR blocked");
	_ZBUS_ASSERT(chan != NULL, "chan is required");
	_ZBUS_ASSERT(obs != NULL, "obs is required");

	if (chan->read_only != 0) {
		return -EPERM;
	}

	err = k_mutex_lock(chan->mutex, timeout);
	if (err) {
		return err;
	}

	SYS_SLIST_FOR_EACH_CONTAINER_SAFE(&chan->runtime_observers, obs_nd, tmp, node) {
		__ASSERT(obs_nd != NULL, "observer node is NULL");
		if (obs_nd->obs == obs) {
			obs_nd->obs = NULL;
			sys_slist_remove(&chan->runtime_observers, &prev_obs_nd->node,
					 &obs_nd->node);

			k_mutex_unlock(chan->mutex);

			--pool_used;

			return 0;
		}
		prev_obs_nd = obs_nd;
	}

	k_mutex_unlock(chan->mutex);

	return -ENODATA;
}
