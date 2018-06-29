/* saadc_nrf52.c - NRF52 ADC driver */

/*
 * Copyright (c) 2018 Ayna.tech
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <errno.h>

#include <init.h>
#include <kernel.h>
#include <board.h>
#include <adc.h>

#include <nrfx.h>
#include <nrfx_saadc.h>

#include <logging/sys_log.h>

struct adc_info {
    nrfx_saadc_config_t saadc_config;
    nrf_saadc_channel_config_t channel_config;
};

static nrfx_err_t ret;

void __saadc_callback(nrfx_saadc_evt_t const * p_event)
{
    ARG_UNUSED(p_event);
}

static void saadc_nrf52_enable(struct device *dev)
{
	ARG_UNUSED(dev);
}

static void saadc_nrf52_disable(struct device *dev)
{
	ARG_UNUSED(dev);
	nrfx_saadc_uninit();
}

int __saadc_capture_sample(nrf_saadc_input_t pin, s16_t *sample_buffer)
{
    nrfx_err_t ret;
    ret = nrfx_saadc_sample_convert(pin, sample_buffer);
    if(ret != NRFX_SUCCESS) {
        SYS_LOG_ERR("Could not convert buffer. Error code: 0x%X", ret);
        return -EIO; /* Some thing gone wrong. */
    }
    return 0;
}

static int saadc_nrf52_read(struct device *dev, struct adc_seq_table *seq_tbl)
{
	struct adc_info *info = dev->driver_data;
	for (int i = 0; i < seq_tbl->num_entries; i++) {
		u8_t n_samples = (seq_tbl->entries[i].buffer_length)/sizeof(s16_t);
        s16_t *sample_buffer = (s16_t *) seq_tbl->entries[i].buffer;
        for (int j = 0; j < n_samples; ++j) {
            if(__saadc_capture_sample(info->channel_config.pin_p, &sample_buffer[j])) {
                return -EIO;
            }
            k_sleep(seq_tbl->entries[i].sampling_delay);
        }
	}
	return 0;
}

static const struct adc_driver_api api_funcs = {
	.enable  = saadc_nrf52_enable,
	.disable = saadc_nrf52_disable,
	.read    = saadc_nrf52_read,
};

static int saadc_nrf52_init(struct device *dev)
{
    struct adc_info *info = dev->driver_data;
    /* SAADC initial */
    ret = nrfx_saadc_init(&info->saadc_config, __saadc_callback);
    if(ret != NRFX_SUCCESS) {
        SYS_LOG_ERR("Could not initialize the SAADC. Error code: 0x%X", ret);
        return -EINVAL;
    }
    /* SAADC Channel Setting */
    ret = nrfx_saadc_channel_init(0, &info->channel_config);    /* Channel 0 */
    if(ret != NRFX_SUCCESS) {
        SYS_LOG_ERR("Could not initialize channel 0. Error code: 0x%X", ret);
        return -EINVAL;
    }
 	return 0;
}

static struct adc_info adc_info_dev = {
    /* SAADC Register initial parameter */
    .saadc_config = {
        .resolution         = (nrf_saadc_resolution_t)NRFX_SAADC_CONFIG_RESOLUTION,
        .oversample         = (nrf_saadc_oversample_t) 0,
        .interrupt_priority = NRFX_SAADC_CONFIG_IRQ_PRIORITY,
        .low_power_mode     = NRFX_SAADC_CONFIG_LP_MODE
    },
    /* SAADC Channel Setting parameter */
    .channel_config = {
        .resistor_p = NRF_SAADC_RESISTOR_DISABLED,
        .resistor_n = NRF_SAADC_RESISTOR_DISABLED,
        .gain       = NRF_SAADC_GAIN1_4,
        .reference  = NRF_SAADC_REFERENCE_VDD4,
        .acq_time   = NRF_SAADC_ACQTIME_5US,
        .mode       = NRF_SAADC_MODE_SINGLE_ENDED,
        .burst      = NRF_SAADC_BURST_DISABLED,
        .pin_p      = (nrf_saadc_input_t)(CONFIG_NRFX_SAADC_PIN + 1),
        .pin_n      = NRF_SAADC_INPUT_DISABLED
    }
};

DEVICE_AND_API_INIT(saadc_nrf52, CONFIG_ADC_0_NAME, &saadc_nrf52_init,
		    &adc_info_dev, NULL,
		    POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,
		    (void *)&api_funcs);

