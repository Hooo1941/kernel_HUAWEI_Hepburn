/* SPDX-License-Identifier: GPL-2.0 */
/*
 * ffc_base.h
 *
 * base head file for ffc control driver
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

#ifndef _FFC_BASE_H_
#define _FFC_BASE_H_

#include <linux/init.h>
#include <linux/device.h>
#include <linux/notifier.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/bitops.h>

#define FFC_MAX_CHARGE_TERM    8
#define FFC_PARA_LEN_MAX       16

struct ffc_buck_term_para {
	int temp_low;
	int temp_high;
	int vterm_gain;
	int ichg_thre;
	int iterm;
};

enum ffc_buck_term_info {
	FFC_BUCK_TEMP_LOW,
	FFC_BUCK_TEMP_HIGH,
	FFC_BUCK_VTERM_GAIN,
	FFC_BUCK_ICHG_THRE,
	FFC_BUCK_ITERM,
	FFC_BUCK_TERM_TOTAL,
};

enum ffc_battery_type {
	BATTERY_C,
	BATTERY_SI,
};

enum ffc_bat_info {
	FFC_BAT_SN,
	FFC_BAT_TYPE,
	FFC_TERM_PARA_INDEX,
	FFC_BAT_INFO_TOTAL,
};

struct ffc_ctrl_bat_para {
	int parse_ok;
	char bat_sn[FFC_PARA_LEN_MAX];
	int bat_type;
	char buck_term_para_index[FFC_PARA_LEN_MAX];
};

struct ffc_term_para_group {
	struct ffc_buck_term_para term_para_group[FFC_MAX_CHARGE_TERM];
	struct ffc_ctrl_bat_para bat_info;
};

struct ffc_ctrl_dev {
	struct device *dev;
	struct notifier_block event_nb;
	struct notifier_block event_dc_nb;
	struct notifier_block event_fcp_nb;
	struct ffc_buck_term_para buck_term_para[FFC_MAX_CHARGE_TERM];
	bool buck_term_para_flag;
	bool term_para_select_ok;
	struct ffc_term_para_group *ffc_term_para_group;
	int group_size;
	int bat_type;
	bool dc_adp;
	bool fcp_adp;
	int fcp_support_ffc;
	u32 ffc_vterm_flag;
	int ffc_delay_cnt;
	int delay_max_times;
};

#ifdef CONFIG_HUAWEI_FFC_CONTROL
int ffc_get_buck_vterm_with_temp(struct ffc_ctrl_dev *di);
int ffc_get_buck_vterm(struct ffc_ctrl_dev *di);
int ffc_get_buck_iterm(struct ffc_ctrl_dev *di);
int ffc_get_buck_ichg_th(struct ffc_ctrl_dev *di);
void ffc_ctrl_parse_dts(struct device_node *np, struct ffc_ctrl_dev *di);
#else
static inline int ffc_get_buck_vterm_with_temp(struct ffc_ctrl_dev *di)
{
	return 0;
}

static inline int ffc_get_buck_vterm(struct ffc_ctrl_dev *di)
{
	return 0;
}

static inline int ffc_get_buck_iterm(struct ffc_ctrl_dev *di)
{
	return 0;
}

static inline int ffc_get_buck_ichg_th(struct ffc_ctrl_dev *di)
{
	return 0;
}

static void ffc_ctrl_parse_dts(struct device_node *np, struct ffc_ctrl_dev *di)
{
}
#endif /* CONFIG_HUAWEI_FFC_CONTROL */

#endif /* _FFC_BASE_H_ */
