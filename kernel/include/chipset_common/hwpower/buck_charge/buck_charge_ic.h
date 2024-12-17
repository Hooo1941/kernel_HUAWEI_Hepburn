// SPDX-License-Identifier: GPL-2.0
/*
 * buck_charge_ic.h
 *
 * buck charge ic module
 *
 * Copyright (c) 2022-2023 Huawei Technologies Co., Ltd.
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

#ifndef _BUCK_CHARGE_IC_H_
#define _BUCK_CHARGE_IC_H_

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <chipset_common/hwpower/buck_charge/buck_charge_state.h>
#include <chipset_common/hwpower/common_module/power_vote.h>

/* define watchdog timer of charger */
#define CHAGRE_WDT_DISABLE                0
#define CHAGRE_WDT_40S                    40
#define CHAGRE_WDT_80S                    80
#define CHAGRE_WDT_160S                   160
#define CHARGE_SYS_WDT_TIMEOUT            180

/* define ic state of charger */
#define CHARGE_IC_GOOD                    0
#define CHARGE_IC_BAD                     1

/* define charging state of charger */
#define CHAGRE_STATE_NORMAL               0x00
#define CHAGRE_STATE_VBUS_OVP             0x01
#define CHAGRE_STATE_NOT_PG               0x02
#define CHAGRE_STATE_WDT_FAULT            0x04
#define CHAGRE_STATE_BATT_OVP             0x08
#define CHAGRE_STATE_CHRG_DONE            0x10
#define CHAGRE_STATE_INPUT_DPM            0x20
#define CHAGRE_STATE_NTC_FAULT            0x40
#define CHAGRE_STATE_CV_MODE              0x80

/* define charging voltage gears of charger */
#define CHARGE_VOLTAGE_4360_MV            4360
#define CHARGE_VOLTAGE_4520_MV            4520
#define CHARGE_VOLTAGE_4600_MV            4600
#define CHARGE_VOLTAGE_5000_MV            5000
#define CHARGE_VOLTAGE_6300_MV            6300
#define CHARGE_VOLTAGE_6500_MV            6500

/* type must be appended and unchangeable */
enum buck_ic_type {
	BUCK_IC_TYPE_BEGIN = 0,
	BUCK_IC_TYPE_PLATFORM = BUCK_IC_TYPE_BEGIN,
	BUCK_IC_TYPE_THIRDPARTY,
	BUCK_IC_TYPE_END,
};

struct ico_input {
	unsigned int charger_type;
	unsigned int iin_max;
	unsigned int ichg_max;
	unsigned int vterm;
};

struct ico_output {
	unsigned int input_current;
	unsigned int charge_current;
};

struct charge_init_data {
	unsigned int charger_type;
	int vbus;
};

struct charge_device_ops {
	int (*chip_init)(struct charge_init_data *init_crit);
	int (*set_adc_conv_rate)(int rate_mode);
	int (*set_input_current)(int value);
	void (*set_input_current_thermal)(int val1, int val2);
	int (*set_charge_current)(int value);
	void (*set_charge_current_thermal)(int val1, int val2);
	int (*dev_check)(void);
	int (*set_terminal_voltage)(int value);
	int (*set_dpm_voltage)(int value);
	int (*set_terminal_current)(int value);
	int (*set_charge_enable)(int enable);
	int (*get_charge_enable_status)(void);
	int (*set_otg_enable)(int enable);
	int (*set_term_enable)(int enable);
	int (*get_charge_state)(unsigned int *state);
	int (*reset_watchdog_timer)(void);
	int (*set_watchdog_timer)(int value);
	int (*set_batfet_disable)(int disable);
	int (*set_batfet_disable_delay)(int delay);
	int (*get_ibus)(void);
	int (*get_vbus)(unsigned int *value);
	int (*check_charger_plugged)(void);
	int (*check_input_dpm_state)(void);
	int (*check_input_vdpm_state)(void);
	int (*check_input_idpm_state)(void);
	int (*set_covn_start)(int enable);
	int (*set_charger_hiz)(int enable);
	int (*set_otg_current)(int value);
	int (*stop_charge_config)(void);
	int (*set_otg_switch_mode_enable)(int enable);
	int (*get_vbat_sys)(void);
	int (*set_vbus_vset)(u32);
	int (*set_mivr)(u32);
	int (*set_uvp_ovp)(void);
	int (*turn_on_ico)(struct ico_input *, struct ico_output *);
	int (*set_force_term_enable)(int enable);
	int (*get_charger_state)(void);
	int (*soft_vbatt_ovp_protect)(void);
	int (*rboost_buck_limit)(void);
	int (*get_charge_current)(void);
	int (*get_iin_set)(void);
	int (*set_boost_voltage)(int voltage);
	int (*get_dieid)(char *dieid, unsigned int len);
	int (*get_vbat)(void);
	int (*get_terminal_voltage)(void);
	int (*get_vusb)(int *value);
	int (*set_pretrim_code)(int val);
	int (*get_dieid_for_nv)(u8 *dieid, unsigned int len);
	int (*get_vsys)(int *value);
	int (*set_ship_mode_enable)(bool enable);
	int (*set_low_power_mode_enable)(bool enable);
	int (*set_dpm_enable)(bool enable);
	/* ops for buck charge vote */
	int (*vote_for_fcc)(struct power_vote_object *obj, void *data, int fcc_ma, const char *client);
	int (*vote_for_usb_icl)(struct power_vote_object *obj, void *data, int icl_ma, const char *client);
	int (*vote_for_vterm)(struct power_vote_object *obj, void *data, int vterm_mv, const char *client);
	int (*vote_for_iterm)(struct power_vote_object *obj, void *data, int iterm_ma, const char *client);
	int (*vote_for_dis_chg)(struct power_vote_object *obj, void *data, int dis_chg, const char *client);
};

struct bc_ic_dev {
	struct device *dev;
	int ic_type;
	bool probe_finish;
	int convert_ratio;
};

#if (defined(CONFIG_HUAWEI_CHARGER_AP) || defined(CONFIG_HUAWEI_CHARGER))
int charge_ops_register(struct charge_device_ops *ops, int index);
struct charge_device_ops *bc_ic_get_ic_ops(void);
int bc_ic_get_convert_ratio(void);
#else
static inline int charge_ops_register(struct charge_device_ops *ops, int index)
{
	return -EPERM;
}

static inline struct charge_device_ops *bc_ic_get_ic_ops(void)
{
	return NULL;
}

static inline int bc_ic_get_convert_ratio(void)
{
	return 1;
}
#endif /* CONFIG_HUAWEI_CHARGER_AP || CONFIG_HUAWEI_CHARGER */

#endif /* _BUCK_CHARGE_IC_H_ */
