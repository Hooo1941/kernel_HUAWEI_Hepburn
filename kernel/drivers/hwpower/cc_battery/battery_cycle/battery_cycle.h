// SPDX-License-Identifier: GPL-2.0
/*
 * battery_cycle.c
 *
 * driver adapter for cycle count store
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

#ifndef _BATTERY_CYCLE_H_
#define _BATTERY_CYCLE_H_

#include <linux/list.h>
#include <linux/device.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/workqueue.h>
#include <linux/sysfs.h>
#include <linux/mutex.h>

#define BAT_CYC_SN_LEN 				16
#define BAT_CYC_SN_STR_LEN 			(BAT_CYC_SN_LEN + 1)
#define BAT_CYC_SN_BUFF_LEN 			64
#define BAT_CYC_SN_NO_CT_STR 			"__NA__"
#define BAT_CYC_SN_RM_STR 			"__RM__"
#define BAT_CYC_NV_REC_CNT 			3
#define BAT_CYC_NV_VER				1
#define BAT_CYC_UPDATE_INC_THR 			100
#define BAT_CYC_FT_REMIND_CYC_THR 		200
#define BAT_CYC_FT_OP_BUF_SIZE 			32
#define BAT_CYC_FT_RST_MAX_CYCLE		30
#define BAT_CYC_EVENT_NOTIFY_SIZE 		64
#define BAT_CYC_SYSFS_BUFF_SIZE			128
#define BAT_CYC_INIT_RETRY_DELAY		1000
#define BAT_CYC_INIT_MAX_RETRY			300
#define BAT_CYC_UNIT				100

enum bat_cycle_ft_remind_mode {
	BAT_CYC_FT_REMIND_MODE_BEGIN = 0,
	BAT_CYC_FT_REMIND_MODE_AUTO = BAT_CYC_FT_REMIND_MODE_BEGIN,
	BAT_CYC_FT_REMIND_MODE_ALWAYS,
	BAT_CYC_FT_REMIND_MODE_END,
};

enum bat_cycle_ft_oper_type {
	BAT_CYC_FT_OPER_BEGIN = 0,
	BAT_CYC_FT_OPER_CLR = BAT_CYC_FT_OPER_BEGIN,
	BAT_CYC_FT_OPER_RST,
	BAT_CYC_FT_OPER_END,
};

enum bat_cycle_ft_oper_res_type {
	BAT_CYC_FT_OPER_RES_SUCCESS		= 0,
	BAT_CYC_FT_OPER_RES_NO_NEED		= 1,
	BAT_CYC_FT_OPER_RES_FUNC_ERR		= -500,
	BAT_CYC_FT_OPER_RES_FORBID_RST_TWICE	= -1,
	BAT_CYC_FT_OPER_RES_CURR_CYC_TOO_LARGE	= -2,
	BAT_CYC_FT_OPER_RES_NO_VALID_BACKUP	= -3,
};

enum bat_cycle_battery_type {
	BAT_CYC_BATTERY_TYPE_BEGIN = 0,
	BAT_CYC_BATTERY_TYPE_RESERVED = BAT_CYC_BATTERY_TYPE_BEGIN,
	BAT_CYC_BATTERY_TYPE_1_BATT,
	BAT_CYC_BATTERY_TYPE_2_BATT,
	BAT_CYC_BATTERY_TYPE_END,
};

struct bat_cycle_sn_pair {
	char sn[BAT_CYC_SN_STR_LEN];
	uint32_t cycle;
};

struct bat_cycle_nv_data {
	int8_t index;
	unsigned int ver;
	int last_cycle;
	struct bat_cycle_sn_pair records[BAT_CYC_NV_REC_CNT];
};

struct bat_cycle_ft_operate {
	char type[BAT_CYC_FT_OP_BUF_SIZE];
	int (*operate)(void);
};

struct bat_cycle_device {
	struct device *dev;
	struct delayed_work init_work;
	int init_retry_times;
	struct bat_cycle_nv_data data;
	struct mutex nv_write_lock;
	int initialized;
	int coul_cycle;
	int last_cycle;
	int battery_type;
	char batt_sn[BAT_CYC_SN_STR_LEN];
	int batt_has_sn;
	int battery_removed;
	int ft_entered;
	int ft_restore_count;
	int ft_oper_res;
	int ft_remind_mode;
};

#endif /* _BATTERY_CYCLE_H_ */