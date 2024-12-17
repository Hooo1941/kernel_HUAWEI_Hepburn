/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description: ssi_mp13.c header file
 * Author: @CompanyNameTag
 */

#ifndef SSI_MP13_H
#define SSI_MP13_H

#ifdef _PRE_CONFIG_GPIO_TO_SSI_DEBUG
int ssi_read_device_arm_register(int trace_en);
int ssi_device_regs_dump(unsigned long long module_set);
int ssi_check_subsys_is_work(void);
void ssi_dump_gpio_regs(void);
#endif /* #ifdef _PRE_CONFIG_GPIO_TO_SSI_DEBUG */

#endif /* #ifndef SSI_MP13_H */
