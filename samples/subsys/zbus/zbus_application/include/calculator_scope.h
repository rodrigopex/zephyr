/*
 * Copyright (c) 2023 Rodrigo Peixoto <rodrigopex@gmail.com>
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef _CALCULATOR_SCOPE_H_
#define _CALCULATOR_SCOPE_H_

#include "zephyr/zbus/zbus.h"

struct msg_calculator__command {
	char op;
	int param1;
	int param2;
};

ZBUS_CHAN_DECLARE(chan_calculator_in__command);

struct msg_calculator__command_result {
	int result;
	int return_code;
};

ZBUS_CHAN_DECLARE(chan_calculator_out__command_result);

#endif /* _CALCULATOR_SCOPE_H_ */
