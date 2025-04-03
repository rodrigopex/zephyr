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
	ARG_UNUSED(ptr2);
	ARG_UNUSED(ptr3);

	uintptr_t thread = (uintptr_t)ptr1;
	int ret;
	uint8_t byte = 0xAA;

	const size_t mem_limit = k_current_get()->stack_info.size - KB(2);

	for (size_t mem_size = CONFIG_MEM_INCREASE_STEP; mem_size < mem_limit;
	     mem_size += CONFIG_MEM_INCREASE_STEP,
		    ++byte IF_ENABLED(CONFIG_PARALLEL, (, k_yield()))) {
		uint8_t mem[mem_size];

		IF_ENABLED(CONFIG_VERBOSE,(
			printk("[%lu] Address: start=%p, end=%p, size=%zu\n", thread,
			       (void *)&mem[0], (void *)&mem[mem_size - 1],
			       (size_t)(&mem[mem_size - 1] - &mem[0]));
		))

		ret = spiram_memtest(mem, mem_size, byte);
		if (ret != 0) {
			printk("[%lu] Error! Test failed for %zu\n", thread, mem_size);
			break;
		}

		IF_ENABLED(CONFIG_VERBOSE,(
			printk("[%lu] Bytes to write and read %zu -> OK\n", thread, mem_size);
	     ))
	}
	if (ret == 0) {
		printk("Thread %lu: OK, memtest for %zuKB\n", thread, mem_limit >> 10);
	}
	return;
}

K_THREAD_DEFINE(memtest_thread_id_01, CONFIG_STACK_SIZE >> 2, spiram_memtest_thread, 1, NULL, NULL,
		9, 0, 0);
K_THREAD_DEFINE(memtest_thread_id_02, CONFIG_STACK_SIZE >> 1, spiram_memtest_thread, 2, NULL, NULL,
		9, 0, 0);
K_THREAD_DEFINE(memtest_thread_id_03, CONFIG_STACK_SIZE, spiram_memtest_thread, 3, NULL, NULL, 9, 0,
		0);
K_THREAD_DEFINE(memtest_thread_id_04, CONFIG_STACK_SIZE, spiram_memtest_thread, 4, NULL, NULL, 9, 0,
		0);
K_THREAD_DEFINE(memtest_thread_id_05, CONFIG_STACK_SIZE, spiram_memtest_thread, 5, NULL, NULL, 9, 0,
		0);
K_THREAD_DEFINE(memtest_thread_id_06, CONFIG_STACK_SIZE, spiram_memtest_thread, 6, NULL, NULL, 9, 0,
		0);
// K_THREAD_DEFINE(memtest_thread_id_07, CONFIG_STACK_SIZE >> 2, spiram_memtest_thread, 7, NULL,
// NULL,
// 9, 0, 0);
// K_THREAD_DEFINE(memtest_thread_id_08, CONFIG_STACK_SIZE << 1,
// spiram_memtest_thread, 5, NULL, NULL, 		9, 0, 0);
