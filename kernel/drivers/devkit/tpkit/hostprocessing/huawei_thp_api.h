/*
 * lcd_kit_core.h
 *
 * lcdkit core function for lcdkit head file
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
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

#ifndef _THP_API_H_
#define _THP_API_H_
#include <linux/types.h>
#include <linux/kobject.h>

enum lcd_kit_type {
	TS_GESTURE_FUNCTION,
};

/* TS Event */
enum lcd_kit_ts_pm_type {
	TS_BEFORE_SUSPEND = 0,
	TS_SUSPEND_DEVICE,
	TS_RESUME_DEVICE,
	TS_AFTER_RESUME,
	TS_EARLY_SUSPEND,
	TS_IC_SHUT_DOWN,
	TS_2ND_POWER_OFF,
};

/* tp kit ops, provide to ts kit module register */
struct ts_kit_ops {
	int (*ts_power_notify)(enum lcd_kit_ts_pm_type type, int sync);
	int (*get_tp_status_by_type)(int type, int *status);
	int (*read_otp_gamma)(u8 *buf, int len);
	int (*send_esd_event)(u32 val);
	bool (*get_tp_proxmity)(void);
	int (*ts_multi_power_notify)(enum lcd_kit_ts_pm_type type,
		int sync, int panel_index);
	bool (*get_afe_status)(struct timespec64 *record_tv);
	int (*get_sn_code)(char *sn_code, unsigned int len);
};

int ts_kit_ops_register(struct ts_kit_ops *ops);
int ts_kit_ops_unregister(struct ts_kit_ops *ops);
struct ts_kit_ops *ts_kit_get_ops(void);
#endif
