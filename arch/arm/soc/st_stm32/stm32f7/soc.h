/*
 * Copyright (c) 2018 Yurii Hamann
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file SoC configuration macros for the ST STM32F7 family processors.
 *
 * Based on reference manual:
 * RM0385 Reference manual STM32F75xxx and STM32F74xxx
 *   advanced ARM(r)-based 32-bit MCUs
 *
 * Chapter 2.2.2: Memory map and register boundary addresses
 */

#ifndef _STM32F7_SOC_H_
#define _STM32F7_SOC_H_

#define GPIO_REG_SIZE         0x400
/* base address for where GPIO registers start */
#define GPIO_PORTS_BASE       (GPIOA_BASE)

#ifndef _ASMLANGUAGE

#include <device.h>
#include <misc/util.h>
#include <stm32f7xx.h>

#ifdef CONFIG_CLOCK_CONTROL_STM32_CUBE
#include <stm32f7xx_ll_utils.h>
#include <stm32f7xx_ll_bus.h>
#include <stm32f7xx_ll_rcc.h>
#include <stm32f7xx_ll_system.h>
#endif /* CONFIG_CLOCK_CONTROL_STM32_CUBE */

#ifdef CONFIG_SERIAL_HAS_DRIVER
#include <stm32f7xx_ll_usart.h>
#endif

#endif /* !_ASMLANGUAGE */

#endif /* _STM32F7_SOC_H_ */
