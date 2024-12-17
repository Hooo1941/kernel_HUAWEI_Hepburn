/**
 * Copyright (c) @CompanyNameMagicTag 2022-2023. All rights reserved.
 *
 * Description: MP16c gpio board-level implementation header file.
 * Author: @CompanyNameTag
 */

#ifndef _GPIO_MP16C_H_
#define _GPIO_MP16C_H_
#include <linux/platform_device.h>

#include "plat_gpio.h"
#include "oal_types.h"

void gugong_clk_power_on(uint32_t subsys);
void gugong_clk_power_off(void);
int32_t mp16c_gpio_init(struct platform_device *pdev);
void mp16c_gpio_remove(struct platform_device *pdev);
void board_clk_config_reset(void);
void gugong_power_off_with_chip_on(void);
uint32_t is_gugong_status_mp16c(void);
void gugong_check_subsys_power_on(uint32_t subsys);
#endif
