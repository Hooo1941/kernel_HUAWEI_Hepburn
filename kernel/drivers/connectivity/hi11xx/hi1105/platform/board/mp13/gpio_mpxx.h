/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description: gpio_hi110x.c header file
 * Author: @CompanyNameTag
 */

#ifndef GPIO_MPXX_H
#define GPIO_MPXX_H
#include <linux/platform_device.h>

#include "plat_gpio.h"
#include "oal_types.h"
int32_t mpxx_gpio_init(struct platform_device *pdev);
void mpxx_gpio_remove(struct platform_device *pdev);
#endif

