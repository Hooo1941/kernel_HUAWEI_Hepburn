// SPDX-License-Identifier: GPL-2.0
/*
 * direct_charge_charging_info.h
 *
 * charging info for direct charge module
 *
 * Copyright (c) 2021-2021 Huawei Technologies Co., Ltd.
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

#ifndef _DIRECT_CHARGE_CHARGING_INFO_H_
#define _DIRECT_CHARGE_CHARGING_INFO_H_

#include <chipset_common/hwpower/direct_charge/direct_charge_ic.h>

#define DEVICE_CLOSE_STATUS_Y     1
#define DEVICE_CLOSE_STATUS_N     0

enum dc_info_type {
	VOLT_RATIO,
	IBAT_TH_HIGH,
	INIT_DELT_VSET,
	MAX_VBAT,
	MIN_VBAT,
	MIN_IBAT_TH,
};

enum direct_charge_info_type {
	CC_CABLE_DETECT_OK,
};

#ifdef CONFIG_DIRECT_CHARGER
int dc_get_gain_ibus(void);
int dc_get_device_ibus(int *value);
void dc_set_stop_charging_flag(bool flag);
int dc_get_stop_charging_flag(void);
unsigned int dc_info_get_val_with_mode(int local_mode, int type);
int direct_charge_get_vmax(unsigned int *val);
int direct_charge_is_failed(void);
int direct_charge_get_info(enum direct_charge_info_type type, int *value);
int direct_charge_get_ibus(void);
int direct_charge_get_vbus(void);
int direct_charge_get_bat_voltage(int *value);
int direct_charge_get_bat_current(int *value);
int direct_charge_get_bat_sys_voltage(int *value);
int direct_charge_get_device_vbus(int *value);
int direct_charge_get_device_temp(int *value);
int direct_charge_get_device_close_status(void);
int direct_charge_get_vbus_vbat(int *vbus, int *vbat);
int direct_charge_get_working_mode(void);
void direct_charge_set_local_mode(int set, unsigned int dc_mode);
unsigned int direct_charge_get_local_mode(void);
unsigned int direct_charge_update_local_mode(unsigned int mode);
#else
static inline int dc_get_gain_ibus(void)
{
	return 0;
}

static inline void dc_set_stop_charging_flag(bool flag)
{
}

static inline int dc_get_stop_charging_flag(void)
{
	return -EINVAL;
}

static inline unsigned int dc_info_get_val_with_mode(int local_mode, int type)
{
	return 0;
}

static inline int direct_charge_get_vmax(unsigned int *val)
{
	return 0;
}

static inline int direct_charge_is_failed(void)
{
	return -EINVAL;
}

static inline int direct_charge_get_info(
	enum direct_charge_info_type type, int *value)
{
	return -EINVAL;
}

static inline int direct_charge_get_ibus(void)
{
	return 0;
}

static inline int direct_charge_get_vbus(void)
{
	return 0;
}

static inline int direct_charge_get_bat_voltage(int *value)
{
	return -EINVAL;
}

static inline int direct_charge_get_bat_current(int *value)
{
	return 0;
}

static inline int direct_charge_get_bat_sys_voltage(int *value)
{
	return 0;
}

static inline int direct_charge_get_device_vbus(int *value)
{
	return -EINVAL;
}

static inline int direct_charge_get_device_temp(int *value)
{
	return -EINVAL;
}

static inline int direct_charge_get_device_close_status(void)
{
	return -EINVAL;
}

static inline int direct_charge_get_vbus_vbat(int *vbus, int *vbat)
{
	return -EINVAL;
}

static inline int direct_charge_get_working_mode(void)
{
	return 0;
}

static inline void direct_charge_set_local_mode(int set, unsigned int dc_mode)
{
}

static inline unsigned int direct_charge_get_local_mode(void)
{
	return 0;
}

static inline unsigned int direct_charge_update_local_mode(unsigned int mode)
{
	return 0;
}
#endif /* CONFIG_DIRECT_CHARGER */

#endif /* _DIRECT_CHARGE_CHARGING_INFO_H_ */
