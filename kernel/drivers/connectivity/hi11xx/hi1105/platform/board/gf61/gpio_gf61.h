/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description: gpio_gf61.c header file
 * Author: @CompanyNameTag
 */

#ifndef GPIO_GF61_H
#define GPIO_GF61_H
#include <linux/platform_device.h>

#include "plat_gpio.h"
#include "oal_types.h"
int32_t gf61_gpio_init(struct platform_device *pdev);
void gf61_gpio_remove(struct platform_device *pdev);
#endif
