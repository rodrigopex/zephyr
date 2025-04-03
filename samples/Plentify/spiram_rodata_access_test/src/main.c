/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/kernel/thread_stack.h>

#define APP_THREAD_STACK_SIZE 65536
#define RODATA_ARRAY_SIZE     (16 << 10) // 32768 // 131071

uint32_t rodata_array[RODATA_ARRAY_SIZE] Z_GENERIC_SECTION(.ext_ram.bss);
// = {
// #include "./lookup_table_values.def"
// };

void spiram_memtest_thread(void *ptr1, void *ptr2, void *ptr3)
{
	printk("Memtest SPIRAM: %p\n", rodata_array);
	for (uint32_t i = 18312 /*RODATA_ARRAY_SIZE - 1*/; i >= 0; i -= 1) {
		printk("i = %u\n", i);
		if (rodata_array[i] != i) {
			printk("Error! Test failed for %u, value=%u\n", i, rodata_array[i]);
			break;
		}
	}
	printk("Test ok!\n");

	while (1) {
		k_sleep(K_FOREVER);
	}
}

Z_KERNEL_STACK_DEFINE_IN(my_stack_area, APP_THREAD_STACK_SIZE, Z_GENERIC_SECTION(.ext_ram.bss));
// K_THREAD_STACK_DEFINE(my_stack_area, APP_THREAD_STACK_SIZE);

struct k_thread my_thread_data;

int main(void)
{
	for (uint32_t i = 0; i < RODATA_ARRAY_SIZE; i += 1) {
		// printk("set rodata_array[%d] = ", i);
		rodata_array[i] = i;
		if (rodata_array[i] != i) {
			printk("Could not write to %u, value=%u\n", i, rodata_array[i]);
			break;
		}
		// printk("%d\n", rodata_array[i]);
	}

	printk("Starting SPIRAM memtest rodata_array access on board %s\n", CONFIG_BOARD_TARGET);

	k_tid_t my_tid = k_thread_create(&my_thread_data, my_stack_area,
					 K_THREAD_STACK_SIZEOF(my_stack_area),
					 spiram_memtest_thread, NULL, NULL, NULL, 3, 0, K_NO_WAIT);
	return 0;
}
