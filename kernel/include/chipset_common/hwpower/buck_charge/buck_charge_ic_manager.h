// SPDX-License-Identifier: GPL-2.0
/*
 * buck_charge_ic_manager.h
 *
 * buck charge ic management interface
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

#ifndef _BUCK_CHARGE_IC_MANAGER_H_
#define _BUCK_CHARGE_IC_MANAGER_H_

#include <chipset_common/hwpower/buck_charge/buck_charge_ic.h>

#define charge_ops_valid(_name) \
({				\
	struct charge_device_ops *_temp_ops = bc_ic_get_ic_ops();	\
	_temp_ops && _temp_ops->_name;			\
})

#if (defined(CONFIG_HUAWEI_CHARGER_AP) || defined(CONFIG_HUAWEI_CHARGER) || defined(CONFIG_FCP_CHARGER))
int charge_init_chip(struct charge_init_data *data);
int charger_set_hiz(int enable);
int charge_set_vbus_vset(u32 volt);
/* mivr: minimum input voltage regulation */
int charge_set_mivr(u32 volt);
int charge_set_batfet_disable(int val);
int charge_set_batfet_disable_delay(int val);
int charge_set_watchdog(int time);
int charge_reset_watchdog(void);
void charge_kick_watchdog(void);
void charge_disable_watchdog(void);
int charge_get_vusb(void);
int charge_get_vbus(int *vbus);
int charge_get_ibus(void);
int charge_get_iin_set(void);
int charge_get_charging_state(unsigned int *state);
int charge_set_dev_iin(int iin);
int charge_check_input_dpm_state(void);
int charge_check_charger_plugged(void);
int charge_get_vsys(int *vsys_vol);
int charge_set_ship_mode_enable(bool enable);
int charge_set_low_power_mode_enable(bool enable);
int charge_vote_for_fcc(struct power_vote_object *obj, void *data, int fcc_ma, const char *client);
int charge_vote_for_usb_icl(struct power_vote_object *obj, void *data, int icl_ma, const char *client);
int charge_vote_for_vterm(struct power_vote_object *obj, void *data, int vterm_mv, const char *client);
int charge_vote_for_iterm(struct power_vote_object *obj, void *data, int iterm_ma, const char *client);
int charge_vote_for_dis_chg(struct power_vote_object *obj, void *data, int dis_chg, const char *client);
int charge_get_dieid_for_nv(u8 *dieid, unsigned int len);
int charge_set_pretrim_code(int val);
int charge_set_charge_enable(int enable);
int charge_set_dpm_enable(int enable);
int charge_set_charge_current(int value);
int charge_set_covn_start(int enable);
int charge_turn_on_ico(struct ico_input *ico_in, struct ico_output *ico_out);
int charge_rboost_buck_limit(void);
int charge_set_dpm_voltage(int value);
int charge_check_input_vdpm_state(void);
int charge_check_input_idpm_state(void);
int charge_set_term_enable(int enable);
int charge_set_terminal_voltage(int value);
int charge_set_terminal_current(int value);
int charge_get_charge_enable_status(void);
int charge_set_adc_conv_rate(int rate_mode);
int charge_stop_charge_config(void);
int charge_get_charge_current(void);
int charge_get_terminal_voltage(void);
int charge_soft_vbatt_ovp_protect(void);
int charge_get_dieid(char *dieid, unsigned int len);
int charge_get_vbat_sys(void);
int charge_get_charger_state(void);
int charge_set_force_term_enable(int enable);
int charge_dev_check(void);
#else
static inline int charger_set_hiz(int enable)
{
	return -ENOTSUPP;
}

static inline int charge_get_iin_set(void)
{
	return CHARGE_CURRENT_0500_MA;
}

static inline int charge_set_dev_iin(int iin)
{
	return -ENOTSUPP;
}

static inline int charge_get_charging_state(unsigned int *state)
{
	return -ENOTSUPP;
}

static inline int charge_set_vbus_vset(u32 volt)
{
	return -ENOTSUPP;
}

/* mivr: minimum input voltage regulation */
static inline int charge_set_mivr(u32 volt)
{
	return -ENOTSUPP;
}

static inline int charge_set_batfet_disable(int val)
{
	return -ENOTSUPP;
}

static inline int charge_set_batfet_disable_delay(int val)
{
	return -ENOTSUPP;
}

static inline int charge_set_watchdog(int time)
{
	return -ENOTSUPP;
}

static inline int charge_reset_watchdog(void)
{
	return -ENOTSUPP;
}

static inline void charge_kick_watchdog(void)
{
}

static inline void charge_disable_watchdog(void)
{
}

static inline int charge_get_ibus(void)
{
	return 0;
}

static inline int charge_get_vbus(int *vbus)
{
	return -ENOTSUPP;
}

static inline int charge_get_vusb(void)
{
	return -EINVAL;
}

static inline int charge_init_chip(struct charge_init_data *data)
{
	return -ENOTSUPP;
}

static inline int charge_check_charger_plugged(void)
{
	return 0;
}

static inline int charge_get_vsys(int *vsys_vol)
{
	return -ENOTSUPP;
}

static inline int charge_set_ship_mode_enable(bool enable)
{
	return -ENOTSUPP;
}

static inline int charge_set_low_power_mode_enable(bool enable)
{
	return -ENOTSUPP;
}

static inline int charge_vote_for_fcc(struct power_vote_object *obj, void *data, int fcc_ma, const char *client)
{
	return 0;
}

static inline int charge_vote_for_usb_icl(struct power_vote_object *obj, void *data, int icl_ma, const char *client)
{
	return 0;
}

static inline int charge_vote_for_vterm(struct power_vote_object *obj, void *data, int vterm_mv, const char *client)
{
	return 0;
}

static inline int charge_vote_for_iterm(struct power_vote_object *obj, void *data, int iterm_ma, const char *client)
{
	return 0;
}

static inline int charge_vote_for_dis_chg(struct power_vote_object *obj, void *data, int dis_chg, const char *client)
{
	return 0;
}

static inline int charge_get_dieid_for_nv(u8 *dieid, unsigned int len)
{
	return -ENOTSUPP;
}

static inline int charge_set_pretrim_code(int val)
{
	return -ENOTSUPP;
}

static inline int charge_set_charge_enable(int enable)
{
	return -ENOTSUPP;
}

static inline int charge_set_dpm_enable(int enable)
{
	return -ENOTSUPP;
}

static inline int charge_set_charge_current(int value)
{
	return -ENOTSUPP;
}

static inline int charge_set_covn_start(int enable)
{
	return -ENOTSUPP;
}

static inline int charge_turn_on_ico(struct ico_input *ico_in, struct ico_output *ico_out)
{
	return -ENOTSUPP;
}

static inline int charge_rboost_buck_limit(void)
{
	return -ENOTSUPP;
}

static inline int charge_set_dpm_voltage(int value)
{
	return -ENOTSUPP;
}

static inline int charge_check_input_vdpm_state(void)
{
	return 0;
}

static inline int charge_check_input_idpm_state(void)
{
	return 0;
}

static inline int charge_set_term_enable(int enable)
{
	return -ENOTSUPP;
}

static inline int charge_set_terminal_voltage(int value)
{
	return -ENOTSUPP;
}

static inline int charge_set_terminal_current(int value)
{
	return -ENOTSUPP;
}

static inline int charge_get_charge_enable_status(void)
{
	return 0;
}

static inline int charge_set_adc_conv_rate(int rate_mode)
{
	return -ENOTSUPP;
}

static inline int charge_stop_charge_config(void)
{
	return -ENOTSUPP;
}

static inline int charge_get_charge_current(void)
{
	return 0;
}

static inline int charge_get_terminal_voltage(void)
{
	return 0;
}

static inline int charge_soft_vbatt_ovp_protect(void)
{
	return -ENOTSUPP;
}

static inline int charge_get_dieid(char *dieid, unsigned int len)
{
	return -ENOTSUPP;
}

static inline int charge_get_vbat_sys(void)
{
	return 0;
}

static inline int charge_get_charger_state(void)
{
	return 0;
}

static inline int charge_set_force_term_enable(int enable)
{
	return -ENOTSUPP;
}

static inline int charge_dev_check(void)
{
	return -ENOTSUPP;
}

#endif /* CONFIG_HUAWEI_CHARGER_AP || CONFIG_HUAWEI_CHARGER || CONFIG_FCP_CHARGER */

#endif /* _BUCK_CHARGE_IC_MANAGER_H_ */
