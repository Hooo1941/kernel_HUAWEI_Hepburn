/* SPDX-License-Identifier: GPL-2.0 */
/*
 * wireless_rx_sysfs.h
 *
 * common interface for wireless_rx_sysfs
 *
 * Copyright (c) 2022-2022 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#ifndef _WIRELESS_RX_SYSFS_H_
#define _WIRELESS_RX_SYSFS_H_

#include <linux/types.h>

#define WLRX_SYSFS_THERMAL_FORCE_FAN_FULL_SPEED    BIT(0)
#define WLRX_SYSFS_THERMAL_EXIT_SC_MODE            BIT(1)

#ifdef CONFIG_WIRELESS_CHARGER
void wlrx_sysfs_charge_para_init(unsigned int drv_type);
u8 wlrx_sysfs_get_support_mode(unsigned int drv_type);
bool wlrx_sysfs_ignore_fan_ctrl(unsigned int drv_type);
u8 wlrx_sysfs_get_thermal_ctrl(unsigned int drv_type);
#else
static inline void wlrx_sysfs_charge_para_init(unsigned int drv_type)
{
}

static inline void u8 wlrx_sysfs_get_support_mode(unsigned int drv_type)
{
	return 0;
}

static inline bool wlrx_sysfs_ignore_fan_ctrl(unsigned int drv_type)
{
	return false;
}

static inline u8 wlrx_sysfs_get_thermal_ctrl(unsigned int drv_type)
{
	return 0;
}
#endif /* CONFIG_WIRELESS_CHARGER */

#endif /* _WIRELESS_RX_SYSFS_H_ */
