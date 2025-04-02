#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <string.h>
#include <zephyr/drivers/flash.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/fs/nvs.h>

static struct nvs_fs fs;

#define NVS_PARTITION        storage_partition
#define NVS_PARTITION_DEVICE FIXED_PARTITION_DEVICE(NVS_PARTITION)
#define NVS_PARTITION_OFFSET FIXED_PARTITION_OFFSET(NVS_PARTITION)

#define STRING_ID 1

#if defined(CONFIG_SAMPLE_USE_SPIRAM)
#define RAM_SECTION __attribute__((section(".ext_ram.bss")))
#else
#define RAM_SECTION
#endif

#define BUF_SIZE                       (128) /* 3.5KB */
#define FLASH_READ_AMOUNT_BEFORE_WRITE 10000

RAM_SECTION int rc;
RAM_SECTION char buf[BUF_SIZE];

void nvs_sample_thread(void *p1, void *p2, void *p3)
{
	printk(" ===> Starting NVS sample\n");
	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	rc = 0;

	uint64_t *counter = (uint64_t *)buf;
	*counter = 0;

	for (int i = 8; i < BUF_SIZE; ++i) {
		buf[i] = i;
	}
	struct flash_pages_info info;

	/* define the nvs file system by settings with:
	 *	sector_size equal to the pagesize,
	 *	3 sectors
	 *	starting at NVS_PARTITION_OFFSET
	 */
	fs.flash_device = NVS_PARTITION_DEVICE;
	if (!device_is_ready(fs.flash_device)) {
		printk("Flash device %s is not ready\n", fs.flash_device->name);
		return;
	}
	fs.offset = NVS_PARTITION_OFFSET;
	rc = flash_get_page_info_by_offs(fs.flash_device, fs.offset, &info);
	if (rc) {
		printk("Unable to get page info, rc=%d\n", rc);
		return;
	}
	fs.sector_size = info.size;
	fs.sector_count = 48U;

	rc = nvs_mount(&fs);
	if (rc) {
		printk("Flash Init failed, rc=%d\n", rc);
		return;
	}
	rc = nvs_read(&fs, STRING_ID, &buf, sizeof(buf));
	if (rc > 0) {
		printk("Found counter: %" PRId64 "\n", *counter);
	}

	while (1) {
		printk(" ~~> Reading flash (reading amount: %d)\n", FLASH_READ_AMOUNT_BEFORE_WRITE);

		*counter += 1;

		for (int i = 8; i < BUF_SIZE; ++i) {
			buf[i] = *counter;
		}

		(void)nvs_write(&fs, STRING_ID, &buf, sizeof(buf));

		for (int i = 0; i < FLASH_READ_AMOUNT_BEFORE_WRITE; ++i) {
			rc = nvs_read(&fs, STRING_ID, &buf, sizeof(buf));
			k_yield();
		}

		if (rc > 0) { /* item was found, show it */
			printk("Counter: %" PRId64 "\n", *counter);
		} else {
			printk("Could not read flash. err=%d\n", rc);
		}

		k_msleep(100);
	}
}

K_THREAD_DEFINE(nvs_sample_thread_id, 4096, nvs_sample_thread, NULL, NULL, NULL, 3, 0, 5000);
