/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>

#if defined(CONFIG_MEMTEST_INLINE)
#define TEST_SCOPE "INLINE"
#elif defined(CONFIG_MEMTEST_BSS)
#define TEST_SCOPE "BSS"
#else
#define TEST_SCOPE "HEAP"
#endif

int main(void)
{
	printk("Starting SPIRAM memtest (" TEST_SCOPE ") on board %s\n", CONFIG_BOARD_TARGET);

	return 0;
}
