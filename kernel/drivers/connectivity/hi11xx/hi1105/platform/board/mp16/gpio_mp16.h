/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description: gpio_mp16.c header file
 * Author: @CompanyNameTag
 */

#ifndef _GPIO_MP16_H_
#define _GPIO_MP16_H_
#include <linux/platform_device.h>

#include "plat_gpio.h"
#include "oal_types.h"
int32_t mp16_gpio_init(struct platform_device *pdev);
void mp16_gpio_remove(struct platform_device *pdev);
#endif
