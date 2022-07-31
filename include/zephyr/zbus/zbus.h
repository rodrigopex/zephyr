/*
 * Copyright (c) 2022 Rodrigo Peixoto <rodrigopex@gmail.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_INCLUDE_ZBUS_H_
#define ZEPHYR_INCLUDE_ZBUS_H_

#include <string.h>

#include <zephyr/kernel.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Zbus API
 * @defgroup zbus_apis Zbus APIs
 * @{
 */

/**
 * @brief Type used to represent a channel.
 *
 * Every channel has a zbus_channel structure associated used to control the channel
 * access and usage.
 */
struct zbus_channel {
#if defined(CONFIG_ZBUS_CHANNEL_NAME) || defined(__DOXYGEN__)
	/** Channel name. */
	const char *name;
#endif
	/** Read only flag. Indicates the channel cannot receive publications. */
	const bool read_only;

	/** Message size. Represents the channel's message size. */
	const uint16_t message_size;

#if (CONFIG_ZBUS_CHANNEL_USER_DATA_SIZE > 0) || defined(__DOXYGEN__)
	/** User data available to extend zbus features. The channel must be claimed before
	 * using this field.
	 */
	void *user_data;

#endif /* CONFIG_ZBUS_CHANNEL_USER_DATA_SIZE */

	/** Message reference. Represents the message's reference that points to the actual
	 * shared memory region.
	 */
	void *message;

	/** Message validator. Stores the reference to the function to check the message
	 * validity before actually performing the publishing. No invalid messages can be
	 * published. Every message is valid when this field is empty.
	 */
	bool (*validator)(const void *msg, size_t msg_size);

	/** Access control mutex. Points to the mutex used to avoid race conditions
	 * for accessing the channel.
	 */
	struct k_mutex *mutex;
	/** Channel observer list. Represents the channel's observers list, it can be empty or
	 * have listeners and subscribers mixed in any sequence.
	 */
	const struct zbus_observer **observers;

#if (CONFIG_ZBUS_RUNTIME_OBSERVERS_POOL_SIZE > 0) || defined(__DOXYGEN__)
	/** Dynamic channel observer list. Represents the channel's observers list, it can be empty
	 * or have listeners and subscribers mixed in any sequence. It can be changed in runtime.
	 */
	sys_slist_t runtime_observers;
#endif /* CONFIG_ZBUS_RUNTIME_OBSERVERS_POOL_SIZE  */
};

/**
 * @brief Type used to represent an observer.
 *
 * Every observer has an representation structure containing the relevant information.
 * An observer is a code portion interested in some channel. The observer can be notified
 * synchronously or asynchronously and it is called listener and subscriber respectively.
 * The observer can be enabled or disabled during runtime by change the enabled boolean
 * field of the structure. The listeners have a callback function that is executed by the
 * bus with the index of the changed channel as argument when the notification is sent.
 * The subscribers have a message queue where the bus enqueues the index of the changed
 * channel when a notification is sent.
 *
 * @see zbus_obs_set_enable function to properly change the observer's enabled field.
 *
 */
struct zbus_observer {
#if defined(CONFIG_ZBUS_OBSERVER_NAME) || defined(__DOXYGEN__)
	/** Observer name. */
	const char *name;
#endif
	/** Enabled flag. Indicates if observer is receiving notification. */
	bool enabled;
	/** Observer message queue. It turns the observer into a subscriber. */
	struct k_msgq *queue;
	/** Observer callback function. It turns the observer into a listener. */
	void (*callback)(const struct zbus_channel *chan);
};

/** @cond INTERNAL_HIDDEN */

#if defined(CONFIG_ZBUS_ASSERT_MOCK)
#define _ZBUS_ASSERT(_cond, _fmt, ...)                                                             \
	({                                                                                         \
		if (!(_cond)) {                                                                    \
			LOG_ERR(_fmt, ##__VA_ARGS__);                                              \
			return -EFAULT;                                                            \
		}                                                                                  \
	})
#else
#define _ZBUS_ASSERT(_cond, _fmt, ...) __ASSERT(_cond, _fmt, ##__VA_ARGS__)
#endif

#if defined(CONFIG_ZBUS_CHANNEL_NAME)
#define ZBUS_CHANNEL_NAME_INIT(_name) .name = #_name,
#else
#define ZBUS_CHANNEL_NAME_INIT(_name)
#endif

#if defined(CONFIG_ZBUS_OBSERVER_NAME)
#define ZBUS_OBSERVER_NAME_INIT(_name) .name = #_name,
#else
#define ZBUS_OBSERVER_NAME_INIT(_name)
#endif

#if (CONFIG_ZBUS_CHANNEL_USER_DATA_SIZE > 0)
#define ZBUS_USER_DATA_INIT                                                                        \
	.user_data = (void *)((uint8_t[CONFIG_ZBUS_CHANNEL_USER_DATA_SIZE]) {0}), /* User data */
#else
#define ZBUS_USER_DATA_INIT /* No user data */
#endif

#if CONFIG_ZBUS_RUNTIME_OBSERVERS_POOL_SIZE > 0
#define ZBUS_RUNTIME_OBSERVERS_LIST_INIT                                                           \
	.runtime_observers = SYS_SLIST_STATIC_INIT(&_name.runtime_observers),
#else
#define ZBUS_RUNTIME_OBSERVERS_LIST_INIT /* No user data */
#endif

#if defined(CONFIG_ZBUS_STRUCTS_ITERABLE_ACCESS)
#define _ZBUS_STRUCT_DECLARE(_type, _name) STRUCT_SECTION_ITERABLE(_type, _name)
#else
#define _ZBUS_STRUCT_DECLARE(_type, _name) struct _type _name
#endif /* CONFIG_ZBUS_STRUCTS_ITERABLE_ACCESS */

#define _ZBUS_OBS_EXTERN(_name) extern struct zbus_observer _name

#define _ZBUS_CHAN_EXTERN(_name) extern struct zbus_channel _name

#define ZBUS_REF(_value) &(_value)

k_timeout_t _zbus_timeout_remainder(uint64_t end_ticks);
/** @endcond */

/**
 * @def ZBUS_OBS_DECLARE
 * This macro list the observers to be used in a file. Internally, it declares the observers with
 * the extern statement. Note it is only necessary when the observers are declared outside the file.
 */
#define ZBUS_OBS_DECLARE(...) FOR_EACH(_ZBUS_OBS_EXTERN, (;), __VA_ARGS__)

/**
 * @def ZBUS_CHAN_DECLARE
 * This macro list the channels to be used in a file. Internally, it declares the channels with the
 * extern statement. Note it is only necessary when the channels are declared outside the file.
 */
#define ZBUS_CHAN_DECLARE(...) FOR_EACH(_ZBUS_CHAN_EXTERN, (;), __VA_ARGS__)

/**
 * @def ZBUS_OBSERVERS_EMPTY
 * This macro indicates the channel has no observers.
 */
#define ZBUS_OBSERVERS_EMPTY

/**
 * @def ZBUS_OBSERVERS
 * This macro indicates the channel has listed observers. Note the sequence of observer notification
 * will follow the same as listed.
 */
#define ZBUS_OBSERVERS(...) __VA_ARGS__

/**
 * @brief Zbus channel definition.
 *
 * This macro defines a channel.
 *
 * @param _name The channel's name.
 * @param _read_only Flag indicates the channel is read-only.
 * @param _type The Message type. It must be a struct or union.
 * @param _validator The validator function.
 *
 * @see struct zbus_channel
 * @param _observers The observers list. The sequence indicates the priority of the observer. The
 * first the highest priority.
 * @param _init_val The message initialization.
 */
#define ZBUS_CHAN_DEFINE(_name, _read_only, _type, _validator, _observers, _init_val)              \
	static _type _CONCAT(_zbus_message_, _name) = _init_val;                                   \
	K_MUTEX_DEFINE(_CONCAT(_zbus_mutex_, _name));                                              \
	FOR_EACH_NONEMPTY_TERM(_ZBUS_OBS_EXTERN, (;), _observers)                                 \
	_ZBUS_STRUCT_DECLARE(zbus_channel, _name) = {                                              \
		ZBUS_CHANNEL_NAME_INIT(_name)		       /* Name */                          \
			.read_only = (_read_only),	       /* Read only flag */                \
		ZBUS_USER_DATA_INIT			       /* User data */                     \
			ZBUS_RUNTIME_OBSERVERS_LIST_INIT       /* Runtime observer list */         \
				.message_size = sizeof(_type), /* Message size */                  \
		.message = &_CONCAT(_zbus_message_, _name),    /* Reference to the message */      \
		.validator = (_validator),		       /* Validator function */            \
		.mutex = &_CONCAT(_zbus_mutex_, _name),	       /* Channel's Mutex */               \
		.observers = (const struct zbus_observer **)(struct zbus_observer *[]){            \
			FOR_EACH_NONEMPTY_TERM(ZBUS_REF, (,), _observers)                        \
				NULL}} /* Static observer list */

/**
 * @brief Initialize a message.
 *
 * This macro initializes a message by passing the values to initialize the message struct
 * or union.
 *
 * @param[in] _val Variadic with the initial values. ``ZBUS_INIT(0)`` means ``{0}``, as
 * ZBUS_INIT(.a=10, .b=30) means ``{.a=10, .b=30}``.
 */
#define ZBUS_MSG_INIT(_val, ...)                                                                   \
	{                                                                                          \
		_val, ##__VA_ARGS__                                                                \
	}

/**
 * @brief Define and initialize a subscriber.
 *
 * This macro defines an observer of subscriber type. It defines a message queue where the
 * subscriber will receive the notification asynchronously, and initialize the ``struct
 * zbus_observer`` defining the subscriber.
 *
 * @param[in] _name The subscriber's name.
 * @param[in] _queue_size The notification queue's size.
 */
#define ZBUS_SUBSCRIBER_DEFINE(_name, _queue_size)                                                 \
	K_MSGQ_DEFINE(_zbus_observer_queue_##_name, sizeof(struct zbus_channel *), _queue_size,    \
		      sizeof(struct zbus_channel *));                                              \
	_ZBUS_STRUCT_DECLARE(zbus_observer,                                                        \
			     _name) = {ZBUS_OBSERVER_NAME_INIT(_name) /* Name field */             \
					       .enabled = true,                                    \
				       .queue = &_zbus_observer_queue_##_name, .callback = NULL}

/**
 * @brief Define and initialize a listener.
 *
 * This macro defines an observer of listener type. This macro establishes the callback where the
 * listener will be notified synchronously, and initialize the ``struct zbus_observer`` defining the
 * listener.
 *
 * @param[in] _name The listener's name.
 * @param[in] _cb The callback function.
 */
#define ZBUS_LISTENER_DEFINE(_name, _cb)                                                           \
	_ZBUS_STRUCT_DECLARE(zbus_observer,                                                        \
			     _name) = {ZBUS_OBSERVER_NAME_INIT(_name) /* Name field */             \
					       .enabled = true,                                    \
				       .queue = NULL, .callback = (_cb)}

/**
 *
 * @brief Publish to a channel
 *
 * This routine publishes a message to a channel.
 *
 * @param chan The channel's reference.
 * @param msg Reference to the message where the publish function copies the channel's
 * message data from.
 * @param timeout Waiting period to publish the channel,
 *                or one of the special values K_NO_WAIT and K_FOREVER.
 *
 * @retval 0 Channel published.
 * @retval -EPERM The channel is read-only or validator indicates invalid message. Note if the
 * message validator is set and the message is not valid the return is -EPERM as well.
 * @retval -EBUSY The channel is busy.
 * @retval -EAGAIN Waiting period timed out.
 * @retval -EINVAL Some parameter is invalid.
 */
int zbus_chan_pub(struct zbus_channel *chan, const void *msg, k_timeout_t timeout);

/**
 * @brief Read a channel
 *
 * This routine reads a message from a channel.
 *
 * @param[in] chan The channel's reference.
 * @param[out] msg Reference to the message where the read function copies the channel's
 * message data to.
 * @param[in] timeout Waiting period to read the channel,
 *                or one of the special values K_NO_WAIT and K_FOREVER.
 *
 * @retval 0 Channel read.
 * @retval -EBUSY The channel is busy.
 * @retval -EAGAIN Waiting period timed out.
 * @retval -EINVAL Some parameter is invalid.
 */
int zbus_chan_read(const struct zbus_channel *chan, void *msg, k_timeout_t timeout);

/**
 * @brief Claim a channel
 *
 * This routine claims a channel. During the claiming period the channel is blocked for publishing,
 * reading, notifying or claiming again. Finishing is the only available action.
 *
 * @warning After calling this routine, the channel cannot be used by other
 * thread until the zbus_chan_finish routine is performed.
 *
 * @warning This routine should only be called once before a zbus_chan_finish.
 *
 * @param[in] chan The channel's reference.
 * @param[in] timeout Waiting period to claim the channel,
 *                or one of the special values K_NO_WAIT and K_FOREVER.
 *
 * @retval 0 Channel claimed.
 * @retval -EBUSY The channel is busy.
 * @retval -EAGAIN Waiting period timed out.
 * @retval -EINVAL Some parameter is invalid.
 */
int zbus_chan_claim(const struct zbus_channel *chan, k_timeout_t timeout);

/**
 * @brief Finish a channel claim.
 *
 * This routine finishes a channel claim. After calling this routine with success, the channel will
 * be able to be used by other thread.
 *
 * @warning This routine must only be used after a zbus_chan_claim.
 *
 * @param chan The channel's reference.
 *
 * @retval 0 Channel finished.
 * @retval -EPERM The channel was claimed by other thread.
 * @retval -EINVAL The channel's mutex is not locked.
 */
int zbus_chan_finish(const struct zbus_channel *chan);

/**
 * @brief Force a channel notification.
 *
 * This routine forces the event dispatcher to notify the channel's observers even if the message
 * has no changes. Note this function could be useful after claiming/finishing actions.
 *
 * @param chan The channel's reference.
 * @param timeout Waiting period to notify the channel,
 *                or one of the special values K_NO_WAIT and K_FOREVER.
 *
 * @retval 0 Channel notified.
 * @retval -EPERM The current thread does not own the channel.
 * @retval -EINVAL Some parameter is invalid.
 * @retval -EBUSY The channel's mutex returned without waiting.
 * @retval -EAGAIN Timeout to acquiring the channel's mutex.
 */
int zbus_chan_notify(struct zbus_channel *chan, k_timeout_t timeout);

#if defined(CONFIG_ZBUS_CHANNEL_NAME) || defined(__DOXYGEN__)
/**
 * @brief Get the channel's name.
 *
 * This routine returns the channel's name reference.
 *
 * @param chan The channel's reference.
 *
 * @return Channel's name reference.
 */
const char *zbus_chan_name(const struct zbus_channel *chan);
#endif

/**
 * @brief Get the reference for a channel message directly.
 *
 * This routine returns the reference of a channel message.
 *
 * @warning This function cannot used directly for read-only channels.
 * @warning This function must only be used directly for acquired (locked by mutex) channels. This
 * can be done inside a listener for the receiving channel or after claim a channel.
 *
 * @param chan The channel's reference.
 *
 * @return Channel's message reference.
 */
void *zbus_chan_msg(struct zbus_channel *chan);

/**
 * @brief Get a constant reference for a channel message directly.
 *
 * This routine returns a constant reference of a channel message. This should be used
 * inside listeners to access the message directly. In this way zbus prevents the listener of
 * changing the notifying channel's message during the notification process.
 *
 * @note This function can be used directly for read-only channels.
 * @warning This function must only be used directly for acquired (locked by mutex) channels. This
 * can be done inside a listener for the receiving channel or after claim a channel.
 *
 * @param chan The channel's constant reference.
 *
 * @return A constant channel's message reference.
 */
void *zbus_chan_const_msg(const struct zbus_channel *chan);

/**
 * @brief Get the channel's message size.
 *
 * This routine returns the channel's message size.
 *
 * @param chan The channel's reference.
 *
 * @return Channel's message size.
 */
uint16_t zbus_chan_msg_size(const struct zbus_channel *chan);

#if (CONFIG_ZBUS_CHANNEL_USER_DATA_SIZE > 0) || defined(__DOXYGEN__)
/**
 * @brief Get the channel's user data.
 *
 * This routine returns the channel's user data.
 *
 * @param chan The channel's reference.
 *
 * @return Channel's user data.
 */
void *zbus_chan_user_data(const struct zbus_channel *chan);
#endif

#if (CONFIG_ZBUS_RUNTIME_OBSERVERS_POOL_SIZE > 0) || defined(__DOXYGEN__)
/**
 * @brief Add an observer to a channel.
 *
 * This routine adds an observer to the channel.
 *
 * @param chan The channel's reference.
 * @param obs The observer's reference to be added.
 * @param timeout Waiting period to add an observer,
 *                or one of the special values K_NO_WAIT and K_FOREVER.
 *
 * @retval 0 Observer added to the channel.
 * @retval -EINVAL Some parameter is invalid.
 */
int zbus_chan_add_obs(struct zbus_channel *chan, struct zbus_observer *obs, k_timeout_t timeout);

/**
 * @brief Remove an observer from a channel.
 *
 * This routine removes an observer to the channel.
 *
 * @param chan The channel's reference.
 * @param obs The observer's reference to be removed.
 * @param timeout Waiting period to remove an observer,
 *                or one of the special values K_NO_WAIT and K_FOREVER.
 *
 * @retval 0 Observer removed to the channel.
 * @retval -EINVAL Some parameter is invalid.
 */
int zbus_chan_rm_obs(struct zbus_channel *chan, struct zbus_observer *obs, k_timeout_t timeout);

/**
 * @brief Retrieve the numbers of available obervers in the runtime observers pool
 *
 * This routine counts how many nodes are available in runtime observers pool.
 *
 * @return The quantity of observers available in runtime observers pool.
 */
uint8_t zbus_runtime_obs_pool_available(void);

/**
 * @brief Retrieve the maximum number os observers used from the pool
 *
 * This routine indicates the maximum count of used observers from the pool during runtime.
 * It keeps it only in RAM.
 *
 * @return The maximum number of observers used during execution.
 */
uint8_t zbus_runtime_obs_pool_watermark(void);

#endif /* CONFIG_ZBUS_RUNTIME_OBSERVERS_REGISTRATION */

/**
 * @brief Change the observer state.
 *
 * This routine changes the observer state. A channel when disabled will not receive
 * notifications from the event dispatcher.
 *
 * @param[in] obs The observer's reference.
 * @param[in] enabled State to be. When false the observer stops to receive notifications.
 *
 * @retval 0 Observer set enable.
 * @retval -EINVAL Some parameter is invalid.
 */
int zbus_obs_set_enable(struct zbus_observer *obs, bool enabled);

#if defined(CONFIG_ZBUS_OBSERVER_NAME) || defined(__DOXYGEN__)
/**
 * @brief Get the observer's name.
 *
 * This routine returns the observer's name reference.
 *
 * @param obs The observer's reference.
 *
 * @return The observer's name reference.
 */
const char *zbus_obs_name(const struct zbus_observer *obs);
#endif
/**
 * @brief Wait for a channel notification.
 *
 * This routine makes the subscriber to wait a notification. The notification comes as a channel
 * reference.
 *
 * @param sub The subscriber's reference.
 * @param chan The notification channel's reference.
 * @param timeout Waiting period for a notification arrival,
 *                or one of the special values K_NO_WAIT and K_FOREVER.
 *
 * @retval 0 Notification received.
 * @retval -EBUSY The subscriber queue is busy.
 * @retval -EAGAIN Waiting period timed out.
 * @retval -EINVAL Some parameter is invalid.
 */
int zbus_sub_wait(struct zbus_observer *sub, struct zbus_channel **chan, k_timeout_t timeout);

#if defined(CONFIG_ZBUS_STRUCTS_ITERABLE_ACCESS) || defined(__DOXYGEN__)
/**
 *
 * @brief Iterate over channels.
 *
 * Enables the developer to iterate over the channels giving to this function an
 * iterator_func which is called for each channel. If the iterator_func returns false all
 * the iteration stops.
 *
 * @retval true Iterator executed for all channels.
 * @retval false Iterator could not be executed. Some iterate returned false.
 */
bool zbus_iterate_over_channels(bool (*iterator_func)(struct zbus_channel *chan));

/**
 *
 * @brief Iterate over observers.
 *
 * Enables the developer to iterate over the observers giving to this function an
 * iterator_func which is called for each observer. If the iterator_func returns false all
 * the iteration stops.
 *
 * @retval true Iterator executed for all channels.
 * @retval false Iterator could not be executed. Some iterate returned false.
 */
bool zbus_iterate_over_observers(bool (*iterator_func)(struct zbus_observer *obs));
#endif /* CONFIG_ZBUS_STRUCTS_ITERABLE_ACCESS */
       /**
	* @}
	*/

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_ZBUS_H_ */
