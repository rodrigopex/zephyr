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

void spiram_memtest_thread(void *thread_id, void *ptr2, void *ptr3)
{
	ARG_UNUSED(ptr2);
	ARG_UNUSED(ptr3);

	uintptr_t thread = (uintptr_t)thread_id;
	int ret;
	uint8_t byte = 0xAA;

	uint8_t *mem = k_malloc(CONFIG_TEST_MEM_SIZE);

	__ASSERT(mem != NULL, "Could not malloc the requested size");

	const size_t mem_limit = CONFIG_TEST_MEM_SIZE;
	size_t mem_size = CONFIG_MEM_INCREASE_STEP;
	for (; mem_size < mem_limit; mem_size += CONFIG_MEM_INCREASE_STEP,
				     ++byte IF_ENABLED(CONFIG_PARALLEL, (, k_yield()))) {

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
		printk("Thread %lu: OK, memtest for %zuKB\n", thread, mem_size >> 10);
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
