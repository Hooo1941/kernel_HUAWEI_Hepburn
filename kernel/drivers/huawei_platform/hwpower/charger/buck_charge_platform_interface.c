// SPDX-License-Identifier: GPL-2.0
/*
 * buck_charge_platform_interface.c
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

#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/power_supply.h>
#include <linux/sysfs.h>
#include <chipset_common/hwpower/battery/battery_temp.h>
#include <chipset_common/hwpower/buck_charge/buck_charge_platform_interface.h>
#include <chipset_common/hwpower/buck_charge/buck_charge_sysfs.h>
#include <chipset_common/hwpower/buck_charge/buck_charge_ic_manager.h>
#include <chipset_common/hwpower/charger/charger_common_interface.h>
#include <chipset_common/hwpower/charger/charger_detection.h>
#include <chipset_common/hwpower/common_module/power_event_ne.h>
#include <chipset_common/hwpower/common_module/power_interface.h>
#include <chipset_common/hwpower/common_module/power_log.h>
#include <huawei_platform/hwpower/common_module/power_platform.h>
#include <chipset_common/hwpower/common_module/power_supply_interface.h>
#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/log/hwlog_kernel.h>
#include <huawei_platform/power/huawei_charger.h>

#define HWLOG_TAG buck_charge_platform_interface
HWLOG_REGIST();

static struct charge_core_data *g_core_data;

unsigned int buck_charge_get_charge_core_data(int info_name)
{
	unsigned int value = 0;

	switch (info_name) {
	case BUCK_CHARGE_CORE_VTERM_RT:
		value = coul_drv_battery_vbat_max();
		break;
	case BUCK_CHARGE_CORE_IIN_MAX:
		value = g_core_data->iin_max;
		break;
	case BUCK_CHARGE_CORE_ICHG_MAX:
		value = g_core_data->ichg_max;
		break;
	case BUCK_CHARGE_CORE_ITERM:
		value = g_core_data->iterm;
		break;
	case BUCK_CHARGE_CORE_VTERM:
		value = g_core_data->vterm;
		break;
	default:
		break;
	}

	return value;
}

int buck_charge_get_vsys(void)
{
	if (charge_ops_valid(get_vbat_sys))
		return charge_get_vbat_sys();

	return 0;
}

int buck_charge_get_hiz_mode(int hiz_mode)
{
	return charge_get_hiz_state();
}

int buck_charge_get_mains_online(void)
{
	return 0;
}

int buck_charge_set_charge_en_flag(unsigned int val, unsigned int *charge_limit)
{
	int ret;

	if ((val < 0) || (val > 1))
		return -EINVAL;

	ret = set_charger_disable_flags(
		val ? CHARGER_CLEAR_DISABLE_FLAGS : CHARGER_SET_DISABLE_FLAGS, CHARGER_SYS_NODE);
	if (ret < 0)
		return -EINVAL;

	*charge_limit = TRUE;

	return 0;
}


int buck_charge_set_charge_en(unsigned int val, unsigned int charge_enable)
{
	int ret;
	int batt_temp = DEFAULT_NORMAL_TEMP;
	enum charge_status_event events;
	enum chip_charger_type type;

	/*
	 * why should send events in this command?
	 * because it will get the /sys/class/power_supply/Battery/status immediately
	 * to check if the enable/disable command set successfully or not in some product line station
	 */
	bat_temp_get_rt_temperature(BAT_TEMP_MIXED, &batt_temp);
	hwlog_info("dcp runningtest set charge enable=%d\n", charge_enable);
	if (charge_enable) {
		if (((batt_temp > BATT_EXIST_TEMP_LOW) && (batt_temp <= NO_CHG_TEMP_LOW)) ||
			(batt_temp >= NO_CHG_TEMP_HIGH)) {
			hwlog_err("battery temp=%d, abandon enable_charge\n", batt_temp);
			return -EINVAL;
		}
	}

	type = chip_get_charger_type();
	if ((type == PLEASE_PROVIDE_POWER) && (pd_judge_is_cover() == false) &&
		(charge_get_charger_type() != CHARGER_TYPE_WIRELESS)) {
		hwlog_err("hisi charge type=%d, charger_type=%u,"
			"abandon enable_charge\n", type, charge_get_charger_type());
		return -EINVAL;
	}

	charge_set_charge_enable(charge_enable);
#ifdef CONFIG_DIRECT_CHARGER
	if (direct_charge_in_charging_stage() == DC_NOT_IN_CHARGING_STAGE) {
#endif
		if (pmic_get_vbus_status() && (charge_get_charger_type() != CHARGER_REMOVED)) {
			if (charge_enable)
				events = VCHRG_START_CHARGING_EVENT;
			else
				events = VCHRG_NOT_CHARGING_EVENT;
			charge_send_uevent(events);
		}
#ifdef CONFIG_DIRECT_CHARGER
	}
#endif

	return 0;
}

int buck_charge_set_iin_limit(unsigned int val, unsigned int *inputcurrent)
{
	if ((val >= 1) && (val <= CHARGE_SYSFS_MIN_IIN_CURRENT)) {
		*inputcurrent = CHARGE_SYSFS_MIN_IIN_CURRENT;
	} else {
		*inputcurrent = val;
	}

	huawei_charger_input_current_handler();
	hwlog_info("set input current=%d\n", val);
	return 0;
}

int buck_charge_set_ichg_limit(unsigned int val)
{
	if (!g_core_data) {
		hwlog_err("set_ichg_limit g_core_data is null\n");
		return -EPERM;
	}

	g_core_data->ichg_bsoh = val;
	hwlog_info("set charge current is %d\n", val);
	return 0;
}

#ifndef CONFIG_HLTHERM_RUNTEST
int buck_charge_set_iin_limit_array(unsigned int val)
{
	huawei_charger_input_current_handler();
	hwlog_info("thermal set input current=%d\n", val);
	return 0;
}
#else
int buck_charge_set_iin_limit_array(unsigned int val)
{
	return 0;
}
#endif /* CONFIG_HLTHERM_RUNTEST */

int buck_charge_set_vterm_dec(unsigned int val)
{
	int vterm_max;

	if (!g_core_data) {
		hwlog_err("set_vterm_dec di is null\n");
		return -EPERM;
	}

	vterm_max = coul_drv_battery_vbat_max();
	if (vterm_max < 0) {
		hwlog_err("get vterm_max=%d fail\n", vterm_max);
		vterm_max = VTERM_MAX_DEFAULT_MV;
	}

	g_core_data->vterm_bsoh = vterm_max - val;
	HWDUBAI_LOGE("DUBAI_TAG_BATTERY_CUTOFF_VOLTAGE", "voltage=%d", g_core_data->vterm_bsoh);
	hwlog_info("set charger terminal voltage is %d\n",
		g_core_data->vterm_bsoh);
	return 0;
}

void buck_charge_set_iin_runningtest(unsigned int val, unsigned int iin_max, unsigned int *iin_rt)
{
	if ((val == 0) || (val == 1))
		*iin_rt = iin_max;
	else if ((val > 1) && (val <= CHARGE_SYSFS_MIN_IIN_CURRENT))
		*iin_rt = CHARGE_SYSFS_MIN_IIN_CURRENT;
	else
		*iin_rt = val;

	if (huawei_charger_get_input_current() > *iin_rt) {
		huawei_charger_set_input_current(*iin_rt);
		charge_set_input_current(*iin_rt);
	}
	hwlog_info("runningtest set input current=%d\n", *iin_rt);
}

void buck_charge_set_iin_thermal(unsigned int val, unsigned int iin_max, unsigned int *iin_thl_idx)
{
	if ((val == 0) || (val == 1))
		*iin_thl_idx = iin_max;
	else if ((val > 1) && (val <= CHARGE_SYSFS_MIN_IIN_CURRENT))
		*iin_thl_idx = CHARGE_SYSFS_MIN_IIN_CURRENT;
	else
		*iin_thl_idx = val;

	(void)buck_charge_set_iin_limit_array(*iin_thl_idx);
}

void buck_charge_set_iin_rt_current(unsigned int val, int *iin_rt_curr)
{
}

void buck_charge_set_adaptor_voltage(unsigned int val)
{
	charge_set_adapter_voltage(val, RESET_ADAPTER_SOURCE_SYSFS,
		PD_VOLTAGE_CHANGE_WORK_TIMEOUT);
}

void buck_charge_set_chrg_config(unsigned int val, int *chrg_config)
{
}

void buck_charge_set_factory_diag(unsigned int val)
{
}

int buck_charge_get_plugusb(char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "current state is %s\n" "usage is %s\n",
		charger_detection_get_sourcesink_type_name(),
		"echo startsink/stopsink/startsource/stopsource > plugusb\n");
}

void buck_charge_set_ichg_runningtest(unsigned int ichg_rt)
{
	if (huawei_charger_get_charge_current() > ichg_rt)
		charge_set_charge_current(ichg_rt);
	hwlog_info("runningtest set charge current=%d\n", ichg_rt);
}

void buck_charge_set_sdp_iin_limit(unsigned int val)
{
	if ((val >= SDP_IIN_USB) && power_cmdline_is_factory_mode()) {
		buck_charge_set_charge_core_iin_usb(val);
		huawei_charger_set_input_current(val);
		charge_set_input_current(val);
		hwlog_info("set sdp ibus current=%d\n", val);
	}
}

void buck_charge_set_ichg_thermal(unsigned int  val, unsigned int ichg_thl)
{
	if (huawei_charger_get_charge_current() > ichg_thl)
		charge_set_charge_current(ichg_thl);
	hwlog_info("thermal set charge current=%d\n", ichg_thl);
}

void buck_charge_set_vterm_rt(unsigned int vterm_rt)
{
	charge_set_terminal_voltage(vterm_rt);
}

void buck_charge_set_batfet_disable(long val, unsigned int batfet_disable)
{
	if (val == 1) {
		huawei_charger_set_input_current(CHARGE_CURRENT_2000_MA);
		charge_set_input_current(CHARGE_CURRENT_2000_MA);
	}

	(void)charge_set_batfet_disable(batfet_disable);
	hwlog_info("runningtest set batfet_disable=%d\n", batfet_disable);
}

void buck_charge_set_inputcurrent(unsigned int inputcurrent)
{
	huawei_charger_set_input_current(inputcurrent);
	charge_set_input_current(inputcurrent);
	hwlog_info("set input currrent=%ld\n", inputcurrent);
}

void buck_charge_set_charger_cvcal(long val, unsigned int *charger_cvcal_value)
{
	int ret;
	u8 dieid[BUCK_CHARGE_CHGDIEID_LEN] = {0};
	u8 dieid_for_nv[BUCK_CHARGE_CHGDIEID_NV_LEN] = {0};

	if (!charge_ops_valid(get_dieid_for_nv) || !charge_ops_valid(set_pretrim_code))
		return;

	ret = charge_get_dieid_for_nv(dieid, BUCK_CHARGE_CHGDIEID_LEN);
	if (ret) {
		hwlog_err("get dieid failed\n");
		return;
	}

	/* 1:the first byte of dieid_for_nv write 0 */
	memcpy(&dieid_for_nv[1], dieid, BUCK_CHARGE_CHGDIEID_LEN);
	ret = power_nv_write(POWER_NV_CHGDIEID,
		dieid_for_nv, BUCK_CHARGE_CHGDIEID_NV_LEN);
	if (ret) {
		hwlog_err("write dieid to nv failed\n");
		return;
	}

	*charger_cvcal_value = (int)val;
	ret = charge_set_pretrim_code((int)val);
	if (ret) {
		hwlog_err("set pretrim code failed\n");
		return;
	}

	hwlog_info("charger_cvcal=%d\n", (int)val);
}

void buck_charge_set_charger_cvcal_clear(long val, unsigned int *charger_cvcal_clear)
{
	int ret;
	u8 dieid_for_nv[BUCK_CHARGE_CHGDIEID_NV_LEN] = {0};

	if (!charge_ops_valid(set_pretrim_code))
		return;

	if (val == BUCK_CHARGE_CLEAR_CHARGER_CVCAL) {
		ret = power_nv_write(POWER_NV_CHGDIEID,
			dieid_for_nv, BUCK_CHARGE_CHGDIEID_NV_LEN);
		if (ret) {
			hwlog_err("clear dieid in nv failed\n");
			return;
		}

		*charger_cvcal_clear = BUCK_CHARGE_CLEAR_CHARGER_CVCAL;
		ret = charge_set_pretrim_code(0);
		if (ret) {
			hwlog_err("clear pretrim code failed\n");
			return;
		}
	} else {
		hwlog_err("invalid val, clear a and dieid failed\n");
		return;
	}

	hwlog_info("clear a and dieid success\n");
}

void buck_charge_set_plugusb(const char *buf)
{
	if (!buf)
		return;

	if (!strncmp(buf, "startsink", strlen("startsink"))) {
		hwlog_info("set_plugusb start sink\n");
		power_supply_set_int_property_value("charger_detection",
			POWER_SUPPLY_PROP_CHG_PLUGIN, POWER_SUPPLY_START_SINK);
	} else if (!strncmp(buf, "stopsink", strlen("stopsink"))) {
		hwlog_info("set_plugusb stop sink\n");
		power_supply_set_int_property_value("charger_detection",
			POWER_SUPPLY_PROP_CHG_PLUGIN, POWER_SUPPLY_STOP_SINK);
	} else if (!strncmp(buf, "startsource", strlen("startsource"))) {
		hwlog_info("set_plugusb start source\n");
		power_supply_set_int_property_value("charger_detection",
			POWER_SUPPLY_PROP_CHG_PLUGIN, POWER_SUPPLY_START_SOURCE);
	} else if (!strncmp(buf, "stopsource", strlen("stopsource"))) {
		hwlog_info("set_plugusb stop source\n");
		power_supply_set_int_property_value("charger_detection",
			POWER_SUPPLY_PROP_CHG_PLUGIN, POWER_SUPPLY_STOP_SOURCE);
	} else {
		hwlog_err("set_plugusb error param\n");
	}
}

void buck_charge_set_vr_charge_type(void)
{
	huawei_charger_process_vr_charge_event();
}

void buck_charge_set_charge_core_iin_usb(unsigned int val)
{
	if (!g_core_data) {
		hwlog_err("set_charge_core_iin_usb di is null\n");
		return;
	}

	g_core_data->iin_usb = val;
}

int buck_charge_init_charge_core_data(void)
{
	g_core_data = charge_core_get_params();
	if (!g_core_data) {
		hwlog_err("init g_core_data is null\n");
		return -EPERM;
	}

	return 0;
}
