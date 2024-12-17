/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description: ssi_mp16.c header file
 * Author: @CompanyNameTag
 */

#ifndef SSI_MP16_H
#define SSI_MP16_H

#ifdef _PRE_CONFIG_GPIO_TO_SSI_DEBUG
int mp16_ssi_read_wcpu_pc_lr_sp(void);
int mp16_ssi_read_bcpu_pc_lr_sp(void);
int mp16_ssi_read_gcpu_pc_lr_sp(void);
int mp16_ssi_read_device_arm_register(void);
int mp16_ssi_device_regs_dump(unsigned long long module_set);
int mp16_ssi_check_subsys_is_work(void);
void mp16_ssi_dump_gpio_regs(void);
#endif /* #ifdef __PRE_CONFIG_GPIO_TO_SSI_DEBUG */

#endif /* #ifndef __SSI_MP16_H__ */
