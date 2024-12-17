/*
 * Huawei thermal watch header
 *
 * Copyright (c) 2023-2023 Huawei Technologies Co., Ltd.
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
#ifndef HW_THERMAL_WATCH_H
#define HW_THERMAL_WATCH_H
#include <linux/module.h>
#include <linux/err.h>
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/device.h>
#include <asm/page.h>
#include <linux/power_supply.h>
#include <linux/thermal.h>
#include <securec.h>

#define BATTERY_NAME		"battery"
#define IS_TRANSFER		1
#define PSY_BATTERY_NAME	"battery"
#define PSY_USB_NAME		"usb"
#define CHAGING_STAT		"Charging"
#define DISCHAGING_STAT		"Discharging"

enum {
	THERMAL_INFO_CHG_ON = 0,
	THERMAL_INFO_BAT_STAT = 1,
	THERMAL_INFO_IBUS = 2,
	THERMAL_INFO_VBUS = 3,
	THERMAL_INFO_IBAT = 4,
	THERMAL_INFO_VBAT = 5,
	THERMAL_INFO_CAP = 6,
};

struct hw_shell_t {
	int flag;
	int is_transfer;
};

#endif
