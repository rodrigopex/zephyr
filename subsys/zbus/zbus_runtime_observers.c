/*
 * Copyright (c) 2022 Rodrigo Peixoto <rodrigopex@gmail.com>
 * SPDX-License-Identifier: Apache-2.0
 */
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/zbus/zbus.h>

LOG_MODULE_DECLARE(zbus, CONFIG_ZBUS_LOG_LEVEL);

int zbus_chan_add_obs(const struct zbus_channel *chan, const struct zbus_observer *obs,
		      k_timeout_t timeout)
{
	int err;
	struct zbus_observer_node *obs_nd, *tmp;

	_ZBUS_ASSERT(!k_is_in_isr(), "ISR blocked");
	_ZBUS_ASSERT(chan != NULL, "chan is required");
	_ZBUS_ASSERT(obs != NULL, "obs is required");

	err = k_sem_take(chan->sem, timeout);
	if (err) {
		return err;
	}

	/* Check if the observer is already a runtime observer */
	SYS_SLIST_FOR_EACH_CONTAINER_SAFE(chan->observers, obs_nd, tmp, node) {
		if (obs_nd->obs == obs) {
			k_sem_give(chan->sem);

			return -EALREADY;
		}
	}

	struct zbus_observer_node *new_obs_nd = k_malloc(sizeof(struct zbus_observer_node));
	if (new_obs_nd == NULL) {
		LOG_ERR("Could not allocate memory on runtime observers pool\n");

		k_sem_give(chan->sem);

		return err;
	}

	new_obs_nd->obs = obs;

	sys_slist_append(chan->observers, &new_obs_nd->node);

	k_sem_give(chan->sem);

	return 0;
}

int zbus_chan_rm_obs(const struct zbus_channel *chan, const struct zbus_observer *obs,
		     k_timeout_t timeout)
{
	int err;
	struct zbus_observer_node *obs_nd, *tmp;
	struct zbus_observer_node *prev_obs_nd = NULL;

	_ZBUS_ASSERT(!k_is_in_isr(), "ISR blocked");
	_ZBUS_ASSERT(chan != NULL, "chan is required");
	_ZBUS_ASSERT(obs != NULL, "obs is required");

	err = k_sem_take(chan->sem, timeout);
	if (err) {
		return err;
	}

	SYS_SLIST_FOR_EACH_CONTAINER_SAFE(chan->observers, obs_nd, tmp, node) {
		if (obs_nd->obs == obs) {
			sys_slist_remove(chan->observers, &prev_obs_nd->node, &obs_nd->node);

			k_free(obs_nd);

			k_sem_give(chan->sem);

			return 0;
		}

		prev_obs_nd = obs_nd;
	}

	k_sem_give(chan->sem);

	return -ENODATA;
}
