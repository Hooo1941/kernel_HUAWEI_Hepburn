/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description: ssi_gf61.c header file
 * Author: @CompanyNameTag
 */

#ifndef SSI_GF61_H
#define SSI_GF61_H

#ifdef _PRE_CONFIG_GPIO_TO_SSI_DEBUG
int gf61_ssi_device_regs_dump(unsigned long long module_set);
int gf61_ssi_check_subsys_is_work(void);
void gf61_ssi_dump_gpio_regs(void);
int gf61_ssi_test(unsigned int addr, unsigned int value);
#endif /* #ifdef __PRE_CONFIG_GPIO_TO_SSI_DEBUG */

#endif /* #ifndef __SSI_GF61_H__ */
