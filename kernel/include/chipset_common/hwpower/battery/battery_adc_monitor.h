/* SPDX-License-Identifier: GPL-2.0 */
/*
 * battery_adc_monitor.h
 *
 * battery adc monitor driver
 *
 * Copyright (c) 2024-2024 Huawei Technologies Co., Ltd.
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

#ifndef _BATTERY_ADC_MONITOR_H_
#define _BATTERY_ADC_MONITOR_H_

#include <linux/bitops.h>
#include <linux/errno.h>
#include <huawei_platform/hwpower/common_module/power_platform.h>

enum batt_id {
	BATT_ID_MAIN = 0,
	BATT_ID_AUX,
	BATT_ID_MAX,
};

enum prop_type {
	BATT_CURRENT = 0,
	BATT_VOLTAGE,
	BATT_PROP_MAX,
};

typedef int (*battery_adc_monitor_get_func)(void *data);

struct batt_adc_monitor_ops {
	int (*get_ibat)(void *data);
	void *ibat_data;
	int (*get_vbat)(void *data);
	void *vbat_data;
};

#ifdef CONFIG_HUAWEI_BATTERY_ADC_MONITOR
int batt_adc_monitor_get_prop(enum batt_id id, enum prop_type prop, int *cur);
int batt_adc_monitor_ops_register(enum batt_id id, enum prop_type prop,
	battery_adc_monitor_get_func func, void *data);
#else
static inline int batt_adc_monitor_get_prop(enum batt_id id, enum prop_type prop, int *cur)
{
	return -ENODEV;
}

static inline int batt_adc_monitor_ops_register(enum batt_id id, enum prop_type prop,
	battery_adc_monitor_get_func func, void *data)
{
	return -ENODEV;
}
#endif /* CONFIG_HUAWEI_BATTERY_ADC_MONITOR */

#endif /* _BATTERY_ADC_MONITOR_H_ */
