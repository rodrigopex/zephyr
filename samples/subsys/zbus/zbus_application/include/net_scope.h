/*
 * Copyright (c) 2023 Rodrigo Peixoto <rodrigopex@gmail.com>
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef _NET_SCOPE_H_
#define _NET_SCOPE_H_

#include <zephyr/zbus/zbus.h>

struct msg_net__transmission_result {
	int return_code;
};

ZBUS_CHAN_DECLARE(chan_net_out__transmission_done);

#endif /* _NET_SCOPE_H_ */
