/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description: MP16c ssi host entry function declaration.
 * Author: @CompanyNameTag
 */

#ifndef __SSI_MP16C_H__
#define __SSI_MP16C_H__

#ifdef _PRE_CONFIG_GPIO_TO_SSI_DEBUG
int mp16c_ssi_device_regs_dump(unsigned long long module_set);
int mp16c_ssi_check_subsys_is_work(void);
void mp16c_ssi_show_mdpll_lock_status(void);
void mp16c_ssi_dump_gpio(void);
#endif /* #ifdef __PRE_CONFIG_GPIO_TO_SSI_DEBUG */
#endif /* #ifndef __SSI_MP16C_H__ */
