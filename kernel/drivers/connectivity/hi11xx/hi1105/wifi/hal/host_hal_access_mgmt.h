/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : Host访问Device控制
 * 作    者 : wifi
 * 创建日期 : 2023年2月13日
 */

#ifndef HOST_HAL_ACCESS_MGMT_H
#define HOST_HAL_ACCESS_MGMT_H

#include "host_hal_device.h"

#define HAL_HOST_READL_INVALID_VAL 0xffffffff

void hal_host_forbid_access_device(void);
void hal_host_approve_access_device(void);
void hal_host_access_device_init(void);
uint32_t hal_host_readl(uintptr_t addr);
void hal_host_writel(uint32_t val, uintptr_t addr);
uint32_t hal_pm_try_wakeup_dev_lock(void);
void hal_pm_try_wakeup_forbid_sleep(void);
void hal_pm_try_wakeup_allow_sleep(void);
void hal_pm_try_wakeup_reset_forbid_sleep(void);
uint32_t hal_host_readl_irq(uintptr_t addr);
void hal_host_writel_irq(uint32_t val, uintptr_t addr);
void hal_pm_check_forbid_sleep_in_timer(void);
#endif
