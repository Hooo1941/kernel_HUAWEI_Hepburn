// SPDX-License-Identifier: GPL-2.0
/*
 * buck_charge_ic_manager.c
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

#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/math64.h>
#include <chipset_common/hwpower/common_module/power_common_macro.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/buck_charge/buck_charge_ic_manager.h>
#include <chipset_common/hwpower/charger/charger_common_interface.h>
#include <huawei_platform/hwpower/common_module/power_platform.h>

#define HWLOG_TAG buck_charge_ic_manager
HWLOG_REGIST();

int charge_init_chip(struct charge_init_data *data)
{
	int ret;
	struct charge_device_ops *temp_ops = bc_ic_get_ic_ops();

	if (!temp_ops || !temp_ops->chip_init) {
		hwlog_err("ops or chip_init is null\n");
		return -EPERM;
	}

	ret = temp_ops->chip_init(data);
	hwlog_info("init_chip: charger_type=%u vbus=%d ret=%d\n",
		data->charger_type, data->vbus, ret);
	return ret;
}

int charger_set_hiz(int enable)
{
	int ret;
	struct charge_device_ops *temp_ops = bc_ic_get_ic_ops();

	if (!temp_ops || !temp_ops->set_charger_hiz) {
		hwlog_err("ops or set_charger_hiz is null\n");
		return -EPERM;
	}

	ret = temp_ops->set_charger_hiz(enable);
	hwlog_info("set_hiz: enable=%d ret=%d\n", enable, ret);
	return ret;
}

int charge_set_vbus_vset(u32 volt)
{
	int ret;
	struct charge_device_ops *temp_ops = bc_ic_get_ic_ops();

	if (!temp_ops || !temp_ops->set_vbus_vset) {
		hwlog_err("ops or set_vbus_vset is null\n");
		return -EPERM;
	}

	ret = temp_ops->set_vbus_vset(volt);
	hwlog_info("set_vbus_vset: volt=%u ret=%d\n", volt, ret);
	return ret;
}

int charge_set_mivr(u32 volt)
{
	int ret;
	struct charge_device_ops *temp_ops = bc_ic_get_ic_ops();

	if (!temp_ops || !temp_ops->set_mivr) {
		hwlog_err("ops or set_mivr is null\n");
		return -EPERM;
	}

	ret = temp_ops->set_mivr(volt);
	hwlog_info("set_mivr: volt=%u ret=%d\n", volt, ret);
	return ret;
}

int charge_set_batfet_disable(int val)
{
	int ret;
	struct charge_device_ops *temp_ops = bc_ic_get_ic_ops();

	if (!temp_ops || !temp_ops->set_batfet_disable) {
		hwlog_err("ops or set_batfet_disable is null\n");
		return -EPERM;
	}

	ret = temp_ops->set_batfet_disable(val);
	hwlog_info("set_batfet_disable: val=%d ret=%d\n", val, ret);
	return ret;
}

int charge_set_batfet_disable_delay(int val)
{
	int ret;
	struct charge_device_ops *temp_ops = bc_ic_get_ic_ops();

	if (!temp_ops || !temp_ops->set_batfet_disable_delay) {
		hwlog_err("ops or set_batfet_disable_delay is null\n");
		return -EPERM;
	}

	ret = temp_ops->set_batfet_disable_delay(val);
	hwlog_info("set_batfet_disable_delay: val=%d ret=%d\n", val, ret);
	return ret;
}

int charge_set_watchdog(int time)
{
	int ret;
	struct charge_device_ops *temp_ops = bc_ic_get_ic_ops();

	if (!temp_ops || !temp_ops->set_watchdog_timer) {
		hwlog_err("ops or set_watchdog_timer is null\n");
		return -EPERM;
	}

	ret = temp_ops->set_watchdog_timer(time);
	hwlog_info("set_watchdog: time=%d ret=%d\n", time, ret);
	return ret;
}

int charge_reset_watchdog(void)
{
	int ret;
	struct charge_device_ops *temp_ops = bc_ic_get_ic_ops();

	if (!temp_ops || !temp_ops->reset_watchdog_timer) {
		hwlog_err("ops or reset_watchdog_timer is null\n");
		return -EPERM;
	}

	ret = temp_ops->reset_watchdog_timer();
	hwlog_info("reset_watchdog: ret=%d\n", ret);
	return ret;
}

void charge_kick_watchdog(void)
{
	if (charge_reset_watchdog())
		hwlog_err("kick watchdog timer fail\n");
	else
		hwlog_info("kick watchdog timer ok\n");
	power_platform_charge_feed_sys_wdt(CHARGE_SYS_WDT_TIMEOUT);
}

void charge_disable_watchdog(void)
{
	if (charge_set_watchdog(CHAGRE_WDT_DISABLE))
		hwlog_err("disable watchdog timer fail\n");
	else
		hwlog_info("disable watchdog timer ok\n");
	power_platform_charge_stop_sys_wdt();
}

int charge_get_vusb(void)
{
	int vusb_vol = 0;
	struct charge_device_ops *temp_ops = bc_ic_get_ic_ops();

	if (!temp_ops || !temp_ops->get_vusb) {
		hwlog_err("ops or get_vusb is null\n");
		return -EPERM;
	}

	if (temp_ops->get_vusb(&vusb_vol))
		return -EPERM;

	return vusb_vol;
}

int charge_get_vbus(int *vbus)
{
	int ret;
	struct charge_device_ops *temp_ops = bc_ic_get_ic_ops();

	if (!vbus || !temp_ops || !temp_ops->get_vbus) {
		hwlog_err("ops or get_vbus is null\n");
		return -EPERM;
	}

	ret = temp_ops->get_vbus(vbus);
	hwlog_info("get_vbus: vbus=%u ret=%d\n", *vbus, ret);
	return ret;
}

int charge_get_ibus(void)
{
	struct charge_device_ops *temp_ops = bc_ic_get_ic_ops();

	if (!temp_ops || !temp_ops->get_ibus) {
		hwlog_err("ops or get_ibus is null\n");
		return -EPERM;
	}

	return temp_ops->get_ibus();
}

int charge_get_iin_set(void)
{
	struct charge_device_ops *temp_ops = bc_ic_get_ic_ops();

	if (!temp_ops || !temp_ops->get_iin_set) {
		hwlog_err("ops or get_iin_set is null\n");
		return CHARGE_CURRENT_0500_MA;
	}
	return temp_ops->get_iin_set();
}

int charge_get_charging_state(unsigned int *state)
{
	struct charge_device_ops *temp_ops = bc_ic_get_ic_ops();

	if (!state || !temp_ops || !temp_ops->get_charge_state) {
		hwlog_err("ops or get_charge_state is null\n");
		return -EPERM;
	}

	return temp_ops->get_charge_state(state);
}

int charge_set_dev_iin(int iin)
{
	struct charge_device_ops *temp_ops = bc_ic_get_ic_ops();

	if (!temp_ops || !temp_ops->set_input_current) {
		hwlog_err("ops or set_dev_iin is null\n");
		return -EPERM;
	}

	return temp_ops->set_input_current(iin);
}

int charge_check_input_dpm_state(void)
{
	struct charge_device_ops *temp_ops = bc_ic_get_ic_ops();

	if (!temp_ops || !temp_ops->check_input_dpm_state) {
		hwlog_err("ops or check_input_dpm_state is null\n");
		return -EPERM;
	}

	return temp_ops->check_input_dpm_state();
}

int charge_check_charger_plugged(void)
{
	struct charge_device_ops *temp_ops = bc_ic_get_ic_ops();

	if (!temp_ops || !temp_ops->check_charger_plugged) {
		hwlog_err("ops or check_charger_plugged is null\n");
		return -EPERM;
	}

	return temp_ops->check_charger_plugged();
}

int charge_get_vsys(int *vsys_vol)
{
	struct charge_device_ops *temp_ops = bc_ic_get_ic_ops();

	if (!vsys_vol || !temp_ops || !temp_ops->get_vsys) {
		hwlog_err("ops or get_vsys is null\n");
		return -EPERM;
	}

	return temp_ops->get_vsys(vsys_vol);
}

int charge_set_ship_mode_enable(bool enable)
{
	struct charge_device_ops *temp_ops = bc_ic_get_ic_ops();

	if (!temp_ops || !temp_ops->set_ship_mode_enable) {
		hwlog_err("ops or set_ship_mode_enable is null\n");
		return -EPERM;
	}

	return temp_ops->set_ship_mode_enable(enable);
}

int charge_set_low_power_mode_enable(bool enable)
{
	struct charge_device_ops *temp_ops = bc_ic_get_ic_ops();

	if (!temp_ops || !temp_ops->set_low_power_mode_enable) {
		hwlog_err("ops or set_low_power_mode_enable is null\n");
		return -EPERM;
	}

	return temp_ops->set_low_power_mode_enable(enable);
}

int charge_vote_for_fcc(struct power_vote_object *obj, void *data, int fcc_ma, const char *client)
{
	struct charge_device_ops *temp_ops = bc_ic_get_ic_ops();

	if (!temp_ops || !temp_ops->vote_for_fcc) {
		hwlog_err("ops or vote_for_fcc is null\n");
		return -EPERM;
	}

	return temp_ops->vote_for_fcc(obj, data, fcc_ma, client);
}

int charge_vote_for_usb_icl(struct power_vote_object *obj, void *data, int icl_ma, const char *client)
{
	struct charge_device_ops *temp_ops = bc_ic_get_ic_ops();

	if (!temp_ops || !temp_ops->vote_for_usb_icl) {
		hwlog_err("ops or vote_for_usb_icl is null\n");
		return -EPERM;
	}

	return temp_ops->vote_for_usb_icl(obj, data, icl_ma, client);
}

int charge_vote_for_vterm(struct power_vote_object *obj, void *data, int vterm_mv, const char *client)
{
	struct charge_device_ops *temp_ops = bc_ic_get_ic_ops();

	if (!temp_ops || !temp_ops->vote_for_vterm) {
		hwlog_err("ops or vote_for_vterm is null\n");
		return -EPERM;
	}

	return temp_ops->vote_for_vterm(obj, data, vterm_mv, client);
}

int charge_vote_for_iterm(struct power_vote_object *obj, void *data, int iterm_ma, const char *client)
{
	struct charge_device_ops *temp_ops = bc_ic_get_ic_ops();

	if (!temp_ops || !temp_ops->vote_for_iterm) {
		hwlog_err("ops or vote_for_iterm is null\n");
		return -EPERM;
	}

	return temp_ops->vote_for_iterm(obj, data, iterm_ma, client);
}

int charge_vote_for_dis_chg(struct power_vote_object *obj, void *data, int dis_chg, const char *client)
{
	struct charge_device_ops *temp_ops = bc_ic_get_ic_ops();

	if (!temp_ops || !temp_ops->vote_for_dis_chg) {
		hwlog_err("ops or vote_for_dis_chg is null\n");
		return -EPERM;
	}

	return temp_ops->vote_for_dis_chg(obj, data, dis_chg, client);
}

int charge_get_dieid_for_nv(u8 *dieid, unsigned int len)
{
	int ret;
	struct charge_device_ops *temp_ops = bc_ic_get_ic_ops();

	if (!temp_ops || !temp_ops->get_dieid_for_nv) {
		hwlog_err("ops or get_dieid_for_nv is null\n");
		return -EPERM;
	}

	ret = temp_ops->get_dieid_for_nv(dieid, len);
	hwlog_info("get_dieid_for_nv: dieid=%u len=%d ret=%d\n",
		*dieid, len, ret);
	return ret;
}

int charge_set_pretrim_code(int val)
{
	int ret;
	struct charge_device_ops *temp_ops = bc_ic_get_ic_ops();

	if (!temp_ops || !temp_ops->set_pretrim_code) {
		hwlog_err("ops or set_pretrim_code is null\n");
		return -EPERM;
	}

	ret = temp_ops->set_pretrim_code(val);
	hwlog_info("set_pretrim_code: val=%d ret=%d\n", val, ret);
	return ret;
}

int charge_set_charge_enable(int enable)
{
	struct charge_device_ops *temp_ops = bc_ic_get_ic_ops();

	if (!temp_ops || !temp_ops->set_charge_enable) {
		hwlog_err("set_charge_enable is null!\n");
		return -EPERM;
	}

	return temp_ops->set_charge_enable(enable);
}

int charge_set_dpm_enable(int enable)
{
	struct charge_device_ops *temp_ops = bc_ic_get_ic_ops();

	if (!temp_ops || !temp_ops->set_dpm_enable) {
		hwlog_err("set_charge_enable is null!\n");
		return -EPERM;
	}

	return temp_ops->set_dpm_enable(enable);
}

int charge_set_charge_current(int value)
{
	struct charge_device_ops *temp_ops = bc_ic_get_ic_ops();

	if (!temp_ops || !temp_ops->set_charge_current) {
		hwlog_err("set_charge_current is null!\n");
		return -EPERM;
	}

	return temp_ops->set_charge_current(value);
}

int charge_set_covn_start(int enable)
{
	struct charge_device_ops *temp_ops = bc_ic_get_ic_ops();

	if (!temp_ops || !temp_ops->set_covn_start) {
		hwlog_err("set_covn_start is null!\n");
		return -EPERM;
	}

	return temp_ops->set_covn_start(enable);
}

int charge_turn_on_ico(struct ico_input *ico_in, struct ico_output *ico_out)
{
	struct charge_device_ops *temp_ops = bc_ic_get_ic_ops();

	if (!temp_ops || !temp_ops->turn_on_ico) {
		hwlog_err("turn_on_ico is null!\n");
		return -EPERM;
	}

	return temp_ops->turn_on_ico(ico_in, ico_out);
}

int charge_rboost_buck_limit(void)
{
	struct charge_device_ops *temp_ops = bc_ic_get_ic_ops();

	if (!temp_ops || !temp_ops->rboost_buck_limit) {
		hwlog_err("rboost_buck_limit is null!\n");
		return -EPERM;
	}

	return temp_ops->rboost_buck_limit();
}

int charge_set_dpm_voltage(int value)
{
	struct charge_device_ops *temp_ops = bc_ic_get_ic_ops();

	if (!temp_ops || !temp_ops->set_dpm_voltage) {
		hwlog_err("set_dpm_voltage is null!\n");
		return -EPERM;
	}

	return temp_ops->set_dpm_voltage(value);
}

int charge_check_input_vdpm_state(void)
{
	struct charge_device_ops *temp_ops = bc_ic_get_ic_ops();

	if (!temp_ops || !temp_ops->check_input_vdpm_state) {
		hwlog_err("check_input_vdpm_state is null!\n");
		return -EPERM;
	}

	return temp_ops->check_input_vdpm_state();
}

int charge_check_input_idpm_state(void)
{
	struct charge_device_ops *temp_ops = bc_ic_get_ic_ops();

	if (!temp_ops || !temp_ops->check_input_idpm_state) {
		hwlog_err("check_input_idpm_state is null!\n");
		return -EPERM;
	}

	return temp_ops->check_input_idpm_state();
}

int charge_set_term_enable(int enable)
{
	struct charge_device_ops *temp_ops = bc_ic_get_ic_ops();

	if (!temp_ops || !temp_ops->set_term_enable) {
		hwlog_err("set_term_enable is null!\n");
		return -EPERM;
	}

	return temp_ops->set_term_enable(enable);
}

int charge_set_terminal_voltage(int value)
{
	struct charge_device_ops *temp_ops = bc_ic_get_ic_ops();

	if (!temp_ops || !temp_ops->set_terminal_voltage) {
		hwlog_err("set_terminal_voltage is null!\n");
		return -EPERM;
	}

	return temp_ops->set_terminal_voltage(value);
}

int charge_set_terminal_current(int value)
{
	struct charge_device_ops *temp_ops = bc_ic_get_ic_ops();

	if (!temp_ops || !temp_ops->set_terminal_current) {
		hwlog_err("set_terminal_current is null!\n");
		return -EPERM;
	}

	return temp_ops->set_terminal_current(value);
}

int charge_get_charge_enable_status(void)
{
	struct charge_device_ops *temp_ops = bc_ic_get_ic_ops();

	if (!temp_ops || !temp_ops->get_charge_enable_status) {
		hwlog_err("get_charge_enable_status is null!\n");
		return -EPERM;
	}

	return temp_ops->get_charge_enable_status();
}

int charge_set_adc_conv_rate(int rate_mode)
{
	struct charge_device_ops *temp_ops = bc_ic_get_ic_ops();

	if (!temp_ops || !temp_ops->set_adc_conv_rate) {
		hwlog_err("set_adc_conv_rate is null!\n");
		return -EPERM;
	}

	return temp_ops->set_adc_conv_rate(rate_mode);
}

int charge_stop_charge_config(void)
{
	struct charge_device_ops *temp_ops = bc_ic_get_ic_ops();

	if (!temp_ops || !temp_ops->stop_charge_config) {
		hwlog_err("stop_charge_config is null!\n");
		return -EPERM;
	}

	return temp_ops->stop_charge_config();
}

int charge_get_charge_current(void)
{
	struct charge_device_ops *temp_ops = bc_ic_get_ic_ops();

	if (!temp_ops || !temp_ops->get_charge_current) {
		hwlog_err("get_charge_current is null!\n");
		return -EPERM;
	}

	return temp_ops->get_charge_current();
}

int charge_get_terminal_voltage(void)
{
	struct charge_device_ops *temp_ops = bc_ic_get_ic_ops();

	if (!temp_ops || !temp_ops->get_terminal_voltage) {
		hwlog_err("get_terminal_voltage is null!\n");
		return -EPERM;
	}

	return temp_ops->get_terminal_voltage();
}

int charge_soft_vbatt_ovp_protect(void)
{
	struct charge_device_ops *temp_ops = bc_ic_get_ic_ops();

	if (!temp_ops || !temp_ops->soft_vbatt_ovp_protect) {
		hwlog_err("soft_vbatt_ovp_protect is null!\n");
		return -EPERM;
	}

	return temp_ops->soft_vbatt_ovp_protect();
}

int charge_get_dieid(char *dieid, unsigned int len)
{
	struct charge_device_ops *temp_ops = bc_ic_get_ic_ops();

	if (!temp_ops || !temp_ops->get_dieid) {
		hwlog_err("get_dieid is null!\n");
		return -EPERM;
	}

	return temp_ops->get_dieid(dieid, len);
}

int charge_get_vbat_sys(void)
{
	struct charge_device_ops *temp_ops = bc_ic_get_ic_ops();

	if (!temp_ops || !temp_ops->get_vbat_sys) {
		hwlog_err("get_vbat_sys is null!\n");
		return -EPERM;
	}

	return temp_ops->get_vbat_sys();
}

int charge_get_charger_state(void)
{
	struct charge_device_ops *temp_ops = bc_ic_get_ic_ops();

	if (!temp_ops || !temp_ops->get_charger_state) {
		hwlog_err("get_charger_state is null!\n");
		return -EPERM;
	}

	return temp_ops->get_charger_state();
}

int charge_set_force_term_enable(int enable)
{
	struct charge_device_ops *temp_ops = bc_ic_get_ic_ops();

	if (!temp_ops || !temp_ops->set_force_term_enable) {
		hwlog_err("set_force_term_enable is null!\n");
		return -EPERM;
	}

	return temp_ops->set_force_term_enable(enable);
}

int charge_dev_check(void)
{
	struct charge_device_ops *temp_ops = bc_ic_get_ic_ops();

	if (!temp_ops || !temp_ops->dev_check) {
		hwlog_err("dev_check is null!\n");
		return -EPERM;
	}

	return temp_ops->dev_check();
}
