#include <zephyr/kernel.h>

int spiram_memtest(volatile uint8_t *mem, size_t mem_size, uint8_t byte)
{
	for (size_t i = 0; i < mem_size; ++i) {
		mem[i] = byte;
	}

	for (size_t i = 0; i < mem_size; ++i) {
		if (mem[i] != byte) {
			return -EBADF;
		}
	}

	return 0;
}

void spiram_memtest_thread(void *ptr1, void *ptr2, void *ptr3)
{
	int ret;
	uint8_t byte = 0xAA;

	printk("Memtest SPIRAM:\n");
	for (size_t mem_size = KB(100); mem_size < MB(1); mem_size += KB(100), ++byte) {
		uint8_t mem[mem_size];
		ret = spiram_memtest(mem, mem_size, byte);
		if (ret != 0) {
			printk("Error! Test failed for %zu\n", mem_size);
		}
		printk("Bytes to write and read %zu -> OK\n", mem_size);
	}
	while (1) {
		k_sleep(K_FOREVER);
	}
}

K_THREAD_DEFINE(memtest_thread_id, KB(1536), spiram_memtest_thread, NULL, NULL, NULL, 9, 0, 0);
