/*
 * Copyright (c) 2018 Ayna.tech 
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>

#include <device.h>
#include <adc.h>
#include <misc/printk.h>

#define NUMBER_OF_SAMPLES 8

struct adc_data_info {
	struct device *dev;
	struct adc_seq_entry sample;
	struct adc_seq_table table;
	s16_t buffer[NUMBER_OF_SAMPLES];
};

void main(void)
{
	struct adc_data_info adc;
	adc.sample.sampling_delay = 10;
	adc.sample.channel_id = 0;
	adc.sample.buffer = (u8_t *) &adc.buffer;
	adc.sample.buffer_length = NUMBER_OF_SAMPLES * sizeof(s16_t);

	adc.table.entries = &adc.sample;
	adc.table.num_entries = 1;
    
	printk("Test ADC driver\n");
	adc.dev = device_get_binding(CONFIG_ADC_0_NAME);
	if(!adc.dev) {
	    printk("Could not load the ADC driver\n");
	    return;
	}
	adc_enable(adc.dev);
	while (1) {
	    adc_read(adc.dev, &adc.table);
		printk("adc.buffer = [");
		for(int i = 0; i < NUMBER_OF_SAMPLES; ++i) {
		    printk("%d,", adc.buffer[i]);
		}
		printk("\b]\n");
		k_sleep(K_SECONDS(1));
	}
}
