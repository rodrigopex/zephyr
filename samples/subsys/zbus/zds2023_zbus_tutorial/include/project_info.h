/*
 * Copyright (c) 2023 Rodrigo Peixoto <rodrigopex@gmail.com>
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef _PROJECT_INFO_H_
#define _PROJECT_INFO_H_

#include <stdint.h>

struct project_info_msg {
	const struct {
		uint8_t major;
		uint8_t minor;
		uint8_t patch;
	} firmware_version;
	const struct {
		char major;
		char minor[4];
	} hardware_version;
	const char serial_number[32];
	const char model[16];
};

#endif // _PROJECT_INFO_H_
