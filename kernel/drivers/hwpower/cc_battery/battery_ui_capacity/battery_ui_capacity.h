/* SPDX-License-Identifier: GPL-2.0 */
/*
 * battery_ui_capacity.h
 *
 * huawei battery ui capacity
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

#ifndef _BATTERY_UI_CAPACITY_H_
#define _BATTERY_UI_CAPACITY_H_

#include <linux/workqueue.h>
#include <linux/device.h>
#include <chipset_common/hwpower/common_module/power_supply.h>
#include <chipset_common/hwpower/coul/coul_interface.h>
#include "../battery_core.h"

#define BUC_WINDOW_LEN                      10
#define BUC_VBAT_MIN                        3450
#define BUC_CAPACITY_EMPTY                  0
#define BUC_CAPACITY_FULL                   100
#define BUC_SOC_CALIBRATION_PARA_LEVEL      2
#define BUC_SOC_JUMP_THRESHOLD              2
#define BUC_RECHG_PROTECT_VOLT_THRESHOLD    150
#define BUC_CURRENT_THRESHOLD               10
#define BUC_LOWTEMP_THRESHOLD               (-50)
#define BUC_LOW_CAPACITY_THRESHOLD          2
#define BUC_LOW_CAPACITY                    5
#define BUC_DEFAULT_CAPACITY                50
#define BUC_CAPACITY_AMPLIFY                100
#define BUC_CAPACITY_DIVISOR                100
#define BUC_CONVERT_VOLTAGE                 12
#define BUC_CONVERT_CURRENT                 100
#define BUC_WORK_INTERVAL_NORMAL            10000
#define BUC_WORK_INTERVAL_LOWTEMP           5000
#define BUC_WORK_INTERVAL_CHARGING          5000
#define BUC_WORK_INTERVAL_FULL              30000
#define BUC_CHG_FORCE_FULL_SOC_THRESHOLD    95
#define BUC_CHG_FORCE_FULL_TIME             24
#define BUC_WAIT_INTERVAL                   100
#define BUC_WAIT_MAX_TIME                   120000
#define BUC_WAIT_MAGIC_10                   10
#define BUC_WAIT_MAGIC_1                    1
#define BUC_SOC_VARY_LOW                    10
#define BUC_SOC_VARY_HIGH                   90
#define BUC_SOC_MONITOR_TEMP_MIN            100
#define BUC_SOC_MONITOR_TEMP_MAX            450
#define BUC_SOC_MONITOR_INTERVAL            60000
#define BUC_DSM_BUF_SIZE                    256
#define BUC_DEFAULT_SOC_MONITOR_LIMIT       15
#define BUC_CAP_JUMP_TH                     7

enum coul_status {
	BUC_STATUS_START = 0,
	BUC_STATUS_RUNNING,
	BUC_STATUS_WAKEUP,
};

struct bat_ui_vbat_para {
	int volt;
	int soc;
};

struct bat_ui_soc_calibration_para {
	int soc;
	int volt;
};

struct bat_ui_capacity_device {
	struct device *dev;
	struct delayed_work update_work;
	struct delayed_work wait_work;
	struct wakeup_source *wakelock;
	struct mutex update_lock;
	int charge_status;
	int ui_cap_zero_offset;
	int soc_at_term;
	int soc_monitor_limit;
	int soc_monitor_flag;
	int soc_monitor_cnt;
	int ui_capacity;
	int bat_exist;
	int bat_volt;
	int bat_cur;
	int bat_temp;
	int prev_soc;
	int bat_max_volt;
	int ui_prev_capacity;
	int capacity_sum;
	int vth_correct_en;
	int chg_force_full_soc_thld;
	int chg_force_full_count;
	int chg_force_full_wait_time;
	int capacity_filter_count;
	int wait_cnt;
	int wait_max_time;
	u16 monitoring_interval;
	int interval_charging;
	int interval_normal;
	int interval_lowtemp;
	u32 disable_pre_vol_check;
	struct notifier_block event_nb;
	int capacity_filter[BUC_WINDOW_LEN];
	int filter_len;
	struct bat_ui_soc_calibration_para vth_soc_calibration_data[BUC_SOC_CALIBRATION_PARA_LEVEL];
	/* correct shutdown soc */
	int correct_shutdown_soc_en;
	int shutdown_flag;
	int shutdown_cap;
	int shutdown_gap;
	int shutdown_vth;
	int cap_jump_th;
};

#ifdef CONFIG_HUAWEI_BATTERY_UI_CAPACTIY
int bat_ui_capacity(void);
int bat_ui_raw_capacity(void);
int bat_fake_cap_filter(int cap);
#else
static inline int bat_ui_capacity(void)
{
	return coul_interface_get_battery_capacity(bat_core_get_coul_type());
}

static inline int bat_ui_raw_capacity(void)
{
	return coul_interface_get_battery_capacity(bat_core_get_coul_type());
}

static inline int bat_fake_cap_filter(int cap)
{
	return cap;
}
#endif /* CONFIG_HUAWEI_BATTERY_UI_CAPACTIY */

#endif /* _BATTERY_UI_CAPACITY_H_ */
