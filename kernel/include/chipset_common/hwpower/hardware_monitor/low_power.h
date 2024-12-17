/* SPDX-License-Identifier: GPL-2.0 */
/*
 * low_power.h
 *
 * lower power control driver
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

#ifndef _LOW_POWER_H_
#define _LOW_POWER_H_

#include <linux/errno.h>
#include <linux/notifier.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/slab.h>

#define LPM_ENABLE             1
#define LPM_DISABLE            0
#define LPM_CHG_ENABLE         1
#define LPM_CHG_DISABLE        0

#define LPM_DFLT_WORK_INTERVAL 5000 /* default: 5s */
#define LPM_ECM_WORK_INTERVAL  80 /* 80ms */
#define LPM_FB_WORK_DELAY      5000 /* 5s */

#define LPM_IBAT_EN_BST_TH     150 /* idischrg>th, enable buckboost */
#define LPM_IBAT_DIS_BST_TH    100 /* idischrg<th, disable buckboost */

enum lpm_sysfs_type {
	LPM_SYSFS_BEGIN = 0,
	LPM_SYSFS_SUPPORT_ECM = LPM_SYSFS_BEGIN,
	LPM_SYSFS_TRIGGER_ECM,
	LPM_SYSFS_GSM_ECM,
	LPM_SYSFS_BOOST_ECM,
	LPM_SYSFS_END,
};

enum lpm_screen_state {
	LPM_SCREEN_ON,
	LPM_SCREEN_OFF,
};

enum lpm_bst_type {
	LPM_BST_TYPE_BEGIN = 0,
	LPM_BST_TYPE_CHG_EN,
	LPM_BST_TYPE_Q4,
	LPM_BST_TYPE_NOOP, // boosted by other modules, low power no-op
	LPM_BST_TYPE_END,
};

enum lpm_mode_event {
	LPM_EVENT_BEGIN = 0,
	LPM_EVENT_DEFAULT = LPM_EVENT_BEGIN,
	LPM_EVENT_UNDER_VOLT,
	LPM_EVENT_EXIT_ECM,
	LPM_EVENT_END,
};

struct emergency_mode_para {
	u32 vbat_bst_th;
	u32 vbat_shutdown_th;
	u32 vbat_gsm;
	u32 trigger_status;
	u32 gsm_status;
	u32 delay_time;
	int event_type;
	bool bst_vsys;
};

enum ecm_trigger_type {
	ECM_TRIGGER_BEGIN = 0,
	ECM_TRIGGER_IDLE = ECM_TRIGGER_BEGIN,
	ECM_TRIGGER_CN,
	ECM_TRIGGER_OVERSEA,
	ECM_TRIGGER_END,
};

enum ecm_gsm_type {
	ECM_GSM_BEGIN = 0,
	ECM_GSM_IDLE = ECM_GSM_BEGIN,
	ECM_GSM_WORKING,
	ECM_GSM_END,
};

enum ecm_boost_type {
	ECM_BOOST_BEGIN = 0,
	ECM_BOOST_IDLE = ECM_BOOST_BEGIN,
	ECM_BOOST_WORKING,
	ECM_BOOST_END,
};

struct low_temp_mode_para {
	int temp_th;
	u32 soc_th;
	int monitor_type;
	bool bst_vsys;
};

enum ltm_mon_type {
	LTM_MONITOR_BEGIN = 0,
	LTM_MONITOR_IDLE = LTM_MONITOR_BEGIN,
	LTM_MONITOR_WORKING,
	LTM_MONITOR_EXIT,
	LTM_MONITOR_END,
};

enum lpm_bbst_vol {
	LPM_BBST_ENTER_VOL = 0,
	LPM_BBST_EXIT_VOL,
	LPM_BBST_VOL_END,
};

struct low_power_dev {
	struct device *dev;
	struct delayed_work lpm_work;
	struct notifier_block ui_cap_nb;
	struct notifier_block plugged_nb;
	struct notifier_block wltx_dping_nb;
	struct notifier_block fb_nb;
	struct wakeup_source *wakelock;
	int gpio_bst_vsys_sw;
	int gpio_bst_chg_sw;
	int boost_type;
	int vbusin_pssw_type;
	int screen_state;
	int ui_capacity;
	u32 pinctrl_len;
	u32 support_ecm;
	u32 support_ltm;
	int icost_bst;
	int idischrg_en_bst_th;
	int idischrg_dis_bst_th;
	struct emergency_mode_para ecm;
	struct low_temp_mode_para ltm;
	bool boost_vsys_status;
	bool plugin_status;
	bool wltx_dping_status;
	unsigned int work_interval;
	unsigned int lpm_bbst_vout[LPM_BBST_VOL_END];
};

#endif /* _LOW_POWER_H_ */
