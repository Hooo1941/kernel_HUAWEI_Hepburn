// SPDX-License-Identifier: GPL-2.0
/*
 * direct_charge_charging_info.c
 *
 * charging info for direct charge module
 *
 * Copyright (c) 2021-2021 Huawei Technologies Co., Ltd.
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

#include <chipset_common/hwpower/battery/battery_temp.h>
#include <chipset_common/hwpower/common_module/power_common_macro.h>
#include <chipset_common/hwpower/common_module/power_supply.h>
#include <chipset_common/hwpower/common_module/power_supply_interface.h>
#include <chipset_common/hwpower/direct_charge/direct_charger.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_ic_manager.h>
#include <chipset_common/hwpower/wireless_charge/wireless_tx_ic_intf.h>
#include <huawei_platform/power/wireless/wireless_transmitter.h>
#include <huawei_platform/power/battery_voltage.h>
#include <huawei_platform/hwpower/common_module/power_platform.h>
#include <huawei_platform/hwpower/common_module/power_platform_macro.h>

#define HWLOG_TAG direct_charge_charging_info
HWLOG_REGIST();

static unsigned int g_dc_mode;

int dc_get_gain_ibus(void)
{
	int gain_ibus = 0;
	u16 itx = 0;
	int iuem;
	struct direct_charge_device *di = direct_charge_get_di();

	if (!di)
		goto exit;

	if (di->need_check_tx_iin && wireless_tx_get_tx_open_flag()) {
		(void)wltx_ic_get_iin(WLTRX_IC_MAIN, &itx);
		hwlog_info("Itx=%u\n", itx);
		gain_ibus += itx;
	}

	iuem = (int)power_platform_get_uem_leak_current();
	hwlog_info("Iuem=%u\n", iuem);
	gain_ibus += iuem;
exit:
	return gain_ibus;
}

int dc_get_device_ibus(int *value)
{
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di || (l_di->working_mode == UNDEFINED_MODE))
		return -1;

	if (dcm_get_ic_ibus(l_di->working_mode, l_di->cur_mode, value)) {
		*value = 0;
		dc_ctrl_fill_eh_buf_and_set_err_flag_true(DC_EH_GET_DEVICE_IBUS, NULL);
		return -1;
	}

	return 0;
}

void dc_set_stop_charging_flag(bool flag)
{
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return;

	hwlog_info("set stop_charging_flag_error %s\n", flag ? "true" : "false");
	l_di->stop_charging_flag_error = flag;
}

int dc_get_stop_charging_flag(void)
{
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return -1;

	if (l_di->stop_charging_flag_error) {
		hwlog_info("stop_charging_flag_error is true\n");
		return -1;
	}

	return 0;
}

unsigned int dc_info_get_val_with_mode(int local_mode, int type)
{
	struct direct_charge_device *l_di = direct_charge_get_di_by_mode(local_mode);

	if (!l_di)
		return 0;

	switch (type) {
	case VOLT_RATIO:
		return l_di->dc_volt_ratio;
	case IBAT_TH_HIGH:
		return l_di->cur_ibat_th_high;
	case INIT_DELT_VSET:
		return l_di->init_delt_vset;
	case MAX_VBAT:
		return l_di->max_dc_bat_vol;
	case MIN_VBAT:
		return l_di->min_dc_bat_vol;
	case MIN_IBAT_TH:
		return l_di->min_ibat_th;
	default:
		break;
	}

	return 0;
}

int direct_charge_get_vmax(unsigned int *val)
{
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return -ENODEV;

	if (l_di->stage_size > 0)
		*val = l_di->orig_volt_para[l_di->stage_size - 1].vol_th;

	hwlog_info("stage_size=%d, vmax=%d\n", l_di->stage_size,
		l_di->orig_volt_para[l_di->stage_size - 1]);
	return 0;
}

int direct_charge_is_failed(void)
{
	struct direct_charge_device *lvc_di = NULL;
	struct direct_charge_device *sc_di = NULL;
	struct direct_charge_device *sc4_di = NULL;

	lvc_get_di(&lvc_di);
	sc_get_di(&sc_di);
	sc4_get_di(&sc4_di);

	if (!lvc_di && !sc_di && !sc4_di) {
		hwlog_info("lvc_di sc_di and sc4_di is NULL\n");
		return 1;
	}

	if (direct_charge_get_can_enter_status() == false) {
		hwlog_info("can not enter dc\n");
		return 1;
	}

	if (lvc_di) {
		if (lvc_di->error_cnt >= DC_ERR_CNT_MAX) {
			hwlog_info("lvc error exceed retry max\n");
			return 1;
		} else {
			return 0;
		}
	}

	if (sc_di) {
		if (sc_di->error_cnt >= DC_ERR_CNT_MAX) {
			hwlog_info("sc error exceed retry max\n");
			return 1;
		} else {
			return 0;
		}
	}

	if (sc4_di) {
		if (sc4_di->error_cnt >= DC_ERR_CNT_MAX) {
			hwlog_info("sc4 error exceed retry max\n");
			return 1;
		} else {
			return 0;
		}
	}

	return 0;
}

int direct_charge_get_info(enum direct_charge_info_type type, int *value)
{
	struct direct_charge_device *l_di = direct_charge_get_di();
	int ret;

	if (!l_di || !value)
		return -ENODEV;

	switch (type) {
	case CC_CABLE_DETECT_OK:
		if (dc_get_cable_inherent_info(DC_CABLE_DETECT_OK) == CABLE_DETECT_NOK)
			*value = 0;
		else
			*value = 1;
		ret = 0;
		break;
	default:
		ret = -EINVAL;
		break;
	}
	return ret;
}

int direct_charge_get_ibus(void)
{
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return 0;

	return l_di->iadapt;
}

int direct_charge_get_vbus(void)
{
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return 0;

	return l_di->vadapt;
}

int direct_charge_get_bat_voltage(int *value)
{
	int btb_vol;
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return -ENODEV;

	/* last cc&cv use the higher vbat */
	if (l_di->use_higher_vbat && (l_di->cur_stage >= (DOUBLE_SIZE * l_di->stage_size - 2)))
		btb_vol = dcm_get_ic_max_vbtb_with_comp(l_di->working_mode,
			l_di->comp_para.vbat_comp);
	else
		btb_vol = dcm_get_ic_vbtb_with_comp(l_di->working_mode, l_di->cur_mode,
			l_di->comp_para.vbat_comp);

	if (btb_vol <= 0) {
		*value = 0;
		dc_ctrl_fill_eh_buf_and_set_err_flag_true(DC_EH_GET_VBAT, NULL);
		return -EINVAL;
	}

	*value = btb_vol;
	return 0;
}

int direct_charge_get_bat_current(int *value)
{
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return -ENODEV;

	if (dcm_get_total_ibat(l_di->working_mode, l_di->cur_mode, value)) {
		*value = 0;
		dc_ctrl_fill_eh_buf_and_set_err_flag_true(DC_EH_GET_IBAT, NULL);
		return -EINVAL;
	}

	return 0;
}

int direct_charge_get_bat_sys_voltage(int *value)
{
	/* default one battery */
	if (hw_battery_get_series_num() == 1)
		return direct_charge_get_bat_voltage(value);

	/* get total voltage for multi battery */
	*value = hw_battery_voltage(BAT_ID_ALL);
	return 0;
}

int direct_charge_get_device_vbus(int *value)
{
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return -ENODEV;

	if (dcm_get_ic_vbus(l_di->working_mode, l_di->cur_mode, value)) {
		*value = 0;
		dc_ctrl_fill_eh_buf_and_set_err_flag_true(DC_EH_GET_DEVICE_VBUS, NULL);
		return -EINVAL;
	}

	return 0;
}

int direct_charge_get_device_temp(int *value)
{
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return -ENODEV;

	if (dcm_get_ic_temp(l_di->working_mode, l_di->cur_mode, value)) {
		*value = 0;
		dc_ctrl_fill_eh_buf_and_set_err_flag_true(DC_EH_GET_DEVICE_TEMP, NULL);
		return -EINVAL;
	}

	return 0;
}

int direct_charge_get_device_close_status(void)
{
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return DEVICE_CLOSE_STATUS_Y;

	if (dcm_is_ic_close(l_di->working_mode, l_di->cur_mode)) {
		dc_ctrl_fill_eh_buf_and_set_err_flag_true(DC_EH_DEVICE_IS_CLOSE, NULL);
		return DEVICE_CLOSE_STATUS_Y;
	}

	return DEVICE_CLOSE_STATUS_N;
}

int direct_charge_get_vbus_vbat(int *vbus, int *vbat)
{
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return -ENODEV;

	if (dcm_get_ic_status(l_di->working_mode, l_di->cur_mode)) {
		msleep(DC_LS_RECOVERY_DELAY);
		power_supply_get_int_prop(POWER_PLATFORM_USB_PSY_NAME,
			POWER_SUPPLY_PROP_VOLTAGE_NOW, vbus, 0, POWER_UV_PER_MV);
		power_supply_get_int_prop(POWER_PLATFORM_BAT_PSY_NAME,
			POWER_SUPPLY_PROP_VOLTAGE_NOW, vbat,
			POWER_SUPPLY_DEFAULT_VOLTAGE_NOW, POWER_UV_PER_MV);
		hwlog_err("device error, vbus=%d, vbat=%d\n", *vbus, *vbat);
		return 0;
	}

	direct_charge_get_device_vbus(vbus);
	direct_charge_get_bat_voltage(vbat);

	hwlog_info("device_vbus=%d, vbat=%d\n", *vbus, *vbat);
	return 0;
}

int direct_charge_get_working_mode(void)
{
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return UNDEFINED_MODE;

	return l_di->working_mode;
}

void direct_charge_set_local_mode(int set, unsigned int dc_mode)
{
	if (set == AND_SET)
		g_dc_mode &= dc_mode;

	if (set == OR_SET)
		g_dc_mode |= dc_mode;
}

unsigned int direct_charge_get_local_mode(void)
{
	return g_dc_mode;
}

unsigned int direct_charge_update_local_mode(unsigned int mode)
{
	unsigned int local_mode = mode;
	struct direct_charge_device *lvc_di = NULL;
	struct direct_charge_device *sc_di = NULL;
	struct direct_charge_device *sc4_di = NULL;

	lvc_get_di(&lvc_di);
	sc_get_di(&sc_di);
	sc4_get_di(&sc4_di);
	if (!lvc_di && !sc_di && !sc4_di) {
		hwlog_info("local not support direct_charge\n");
		return UNDEFINED_MODE;
	}
	hwlog_info("original local mode %u\n", mode);

	if (sc4_di) {
		if ((sc4_di->sysfs_enable_charger == 0) &&
			!(sc4_di->sysfs_mainsc_enable_charger ^ sc4_di->sysfs_auxsc_enable_charger)) {
			local_mode &= ~SC4_MODE;
			hwlog_info("disable sc4 mode, local_mode %u\n",
				local_mode);
		}
	}
	if (sc_di) {
		if ((sc_di->sysfs_enable_charger == 0) &&
			!(sc_di->sysfs_mainsc_enable_charger ^ sc_di->sysfs_auxsc_enable_charger)) {
			local_mode &= ~SC_MODE;
			hwlog_info("disable sc mode, local_mode %u\n",
				local_mode);
		}
	}
	if (lvc_di) {
		if (lvc_di->sysfs_enable_charger == 0) {
			local_mode &= ~LVC_MODE;
			hwlog_info("disable lvc mode, local_mode %u\n",
				local_mode);
		}
	}

	return local_mode;
}

