// SPDX-License-Identifier: GPL-2.0
/*
 * sc8571.c
 *
 * sc8571 direct charge driver
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

#include "sc8571.h"
#include "sc8571_i2c.h"
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/common_module/power_event_ne.h>
#include <chipset_common/hwpower/common_module/power_pinctrl.h>
#include <chipset_common/hwpower/common_module/power_gpio.h>
#include <chipset_common/hwpower/common_module/power_delay.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_device_id.h>
#include <chipset_common/hwpower/common_module/power_common_macro.h>
#include <huawei_platform/power/battery_voltage.h>

#define HWLOG_TAG sc8571_chg
HWLOG_REGIST();

static int sc8571_set_bq25980_otg_mode(struct sc8571_device_info *di, bool enable)
{
	int ret;

	u8 otg_en = enable ? SC8571_OTG_ENABLE : SC8571_OTG_DISABLE;
	u8 dis_acdrv_both = di->ovp_mos_switch_off ? SC8571_DIS_ACDRV_BOTH_ON : SC8571_DIS_ACDRV_BOTH_OFF;
	u8 ovp_mos_on = di->ovp_mos_switch_off ? SC8571_ACDRV1_OFF : SC8571_ACDRV1_ON;

	ret = sc8571_write_mask(di, SC8571_CTRL2_REG,
		SC8571_OTG_EN_MASK, SC8571_OTG_EN_SHIFT, otg_en);
	if (enable) {
		ret += sc8571_write_mask(di, SC8571_CTRL2_REG,
			SC8571_DIS_ACDRV_BOTH_MASK, SC8571_DIS_ACDRV_BOTH_SHIFT, ovp_mos_on);
		ret += sc8571_write_mask(di, SC8571_CTRL2_REG,
			SC8571_ACDRV1_STAT_MASK, SC8571_ACDRV1_STAT_SHIFT, ovp_mos_on);
	} else {
		ret += sc8571_write_mask(di, SC8571_CTRL2_REG,
			SC8571_DIS_ACDRV_BOTH_MASK, SC8571_DIS_ACDRV_BOTH_SHIFT, SC8571_DIS_ACDRV_BOTH_OFF);
		ret += sc8571_write_mask(di, SC8571_CTRL2_REG,
			SC8571_ACDRV1_STAT_MASK, SC8571_ACDRV1_STAT_SHIFT, SC8571_ACDRV1_OFF);
	}
	if (ret)
		hwlog_err("otg enable failed, enable = %d\n", enable);

	return ret;
}

static int sc8571_set_otg_mode(struct sc8571_device_info *di, bool enable)
{
	int ret;

	u8 manual_en = enable ? SC8571_ACDRV_MANUAL_EN : SC8571_ACDRV_MANUAL_OFF;
	u8 ovp_mos_on = di->ovp_mos_switch_off ? SC8571_ACDRV1_EN_DISABLE : SC8571_ACDRV1_EN_ENABLE;

	ret = sc8571_write_mask(di, SC8571_INTERNAL1_REG,
		SC8571_ACDRV_MANUAL_EN_MASK, SC8571_ACDRV_MANUAL_EN_SHIFT, manual_en);
	if (enable) {
		ret += sc8571_write_mask(di, SC8571_CTRL2_REG,
			SC8571_OTG_EN_MASK, SC8571_OTG_EN_SHIFT, SC8571_OTG_ENABLE);
		ret += sc8571_write_mask(di, SC8571_INTERNAL1_REG,
			SC8571_ACDRV1_EN_MASK, SC8571_ACDRV1_EN_SHIFT, ovp_mos_on);
	} else {
		ret += sc8571_write_mask(di, SC8571_VAC_OVP_REG,
			SC8571_VAC1_PD_EN_MASK, SC8571_VAC1_PD_EN_SHIFT, SC8571_VAC1_PD_ENABLE);
		ret += sc8571_write_mask(di, SC8571_INTERNAL1_REG,
			SC8571_ACDRV1_EN_MASK, SC8571_ACDRV1_EN_SHIFT, SC8571_ACDRV1_EN_DISABLE);
		ret += sc8571_write_mask(di, SC8571_CTRL2_REG,
			SC8571_OTG_EN_MASK, SC8571_OTG_EN_SHIFT, SC8571_OTG_DISABLE);
	}
	if (ret)
		hwlog_err("otg enable failed, enable = %d\n", enable);

	return ret;
}

static void sc8571_otg_enable(struct sc8571_device_info *di, bool enable)
{
	if (di->device_id == SWITCHCAP_BQ25980)
		sc8571_set_bq25980_otg_mode(di, enable);
	else
		sc8571_set_otg_mode(di, enable);
}

static int sc8571_discharge(int enable, void *dev_data)
{
	return 0;
}

static int sc8571_is_device_close(void *dev_data)
{
	u8 val = 0;
	struct sc8571_device_info *di = dev_data;

	if (!di)
		return -EPERM;

	if (sc8571_read_byte(di, SC8571_STAT5_REG, &val))
		return -EIO;

	if (val & SC8571_CP_SWITCHING_STAT_MASK)
		return 0;

	return 1; /* 1:ic is closed */
}

static int sc8571_get_device_id(void *dev_data)
{
	u8 dev_id = 0;
	int ret;
	struct sc8571_device_info *di = dev_data;

	if (!di)
		return -EPERM;

	if (di->first_rd)
		return di->device_id;

	ret = sc8571_read_byte(di, SC8571_DEVICE_ID_REG, &dev_id);
	if (ret) {
		hwlog_err("get_device_id read fail\n");
		return -EPERM;
	}
	di->first_rd = true;
	hwlog_info("get_device_id [0x%x]=0x%x\n", SC8571_DEVICE_ID_REG, dev_id);

	switch (dev_id) {
	case SC8571_DEVICE_ID_VALUE:
		di->device_id = SWITCHCAP_SC8571;
		break;
	case BQ25980_DEVICE_ID_VALUE:
		di->device_id = SWITCHCAP_BQ25980;
		break;
	default:
		di->device_id = DC_DEVICE_ID_END;
		break;
	}
	hwlog_info("device_id=0x%x\n", di->device_id);

	return di->device_id;
}

static int sc8571_get_sc_max_ibat(void *dev_data, int *ibat)
{
	struct sc8571_device_info *di = dev_data;

	if (!di || !ibat)
		return -EINVAL;

	if (di->sc8571_sc_para.max_ibat <= 0) {
		hwlog_err("sc_max_ibat read fail\n");
		return -EPERM;
	}

	*ibat = di->sc8571_sc_para.max_ibat;
	hwlog_info("sc_max_ibat=%d\n", di->sc8571_sc_para.max_ibat);

	return 0;
}

static int sc8571_get_lvc_max_ibat(void *dev_data, int *ibat)
{
	struct sc8571_device_info *di = dev_data;

	if (!di || !ibat)
		return -EINVAL;

	if (di->sc8571_lvc_para.max_ibat <= 0) {
		hwlog_err("lvc_max_ibat read fail\n");
		return -EPERM;
	}

	*ibat = di->sc8571_lvc_para.max_ibat;
	hwlog_info("lvc_max_ibat=%d\n", di->sc8571_lvc_para.max_ibat);

	return 0;
}

static int sc8571_get_max_vbat_mv(void *dev_data)
{
	struct sc8571_device_info *di = dev_data;
	if (!di)
		return -ENODEV;

	return hw_battery_voltage(BAT_ID_MAX);
}

static int sc8571_get_vbat_mv(void *dev_data)
{
	s16 data = 0;
	int ret, vbat;
	struct sc8571_device_info *di = dev_data;

	if (!di)
		return -ENODEV;

	ret = sc8571_read_word(di, SC8571_VBAT_ADC1_REG, &data);
	if (ret)
		return -EIO;

	if (di->device_id == SWITCHCAP_BQ25980)
		vbat = (int)data * BQ25980_VBAT_ADC_STEP / BQ25980_BASE_RATIO_UNIT;
	else
		/* VBAT ADC STEP: 2.5mV */
		vbat = (int)data * 25 / 10;

	hwlog_info("VBAT_ADC=0x%x, vbat=%d\n", data, vbat);
	return vbat;
}

static int sc8571_get_ibat_ma(int *ibat, void *dev_data)
{
	int ret;
	s16 data = 0;
	struct sc8571_device_info *di = dev_data;

	if (!di || !ibat)
		return -EINVAL;

	ret = sc8571_read_word(di, SC8571_IBAT_ADC1_REG, &data);
	if (ret)
		return -EIO;

	if (di->device_id == SWITCHCAP_BQ25980)
		*ibat = (int)data * di->sense_r_config;
	else
		/* IBAT ADC STEP: 3.125mA */
		*ibat = ((int)data * 3125 / 1000) * di->sense_r_config;

	*ibat /= di->sense_r_actual;

	hwlog_info("IBAT_ADC=0x%x, ibat=%d\n", data, *ibat);
	return 0;
}

static int sc8571_get_ibus_ma(int *ibus, void *dev_data)
{
	s16 data = 0;
	int ret;
	struct sc8571_device_info *di = dev_data;

	if (!di || !ibus)
		return -EINVAL;

	ret = sc8571_read_word(di, SC8571_IBUS_ADC1_REG, &data);
	if (ret)
		return -EIO;

	if (di->device_id == SWITCHCAP_BQ25980)
		*ibus = (int)data * BQ25980_IBUS_ADC_STEP / BQ25980_BASE_RATIO_UNIT;
	else
		/* IBUS ADC STEP: 2.5mA */
		*ibus = (int)data * 25 / 10;

	hwlog_info("IBUS_ADC=0x%x, ibus=%d\n", data, *ibus);
	return 0;
}

static int sc8571_get_vbus_mv(int *vbus, void *dev_data)
{
	int ret;
	s16 data = 0;
	struct sc8571_device_info *di = dev_data;

	if (!di || !vbus)
		return -EINVAL;

	ret = sc8571_read_word(di, SC8571_VBUS_ADC1_REG, &data);
	if (ret)
		return -EIO;

	if (di->device_id == SWITCHCAP_BQ25980)
		*vbus = (int)data * BQ25980_VBUS_ADC_STEP / BQ25980_BASE_RATIO_UNIT;
	else
		/* VBUS ADC STEP: 6.25mV */
		*vbus = (int)data * 625 / 100;

	hwlog_info("VBUS_ADC=0x%x, vbus=%d\n", data, *vbus);
	return 0;
}

static int sc8571_get_vusb_mv(int *vusb, void *dev_data)
{
	int ret;
	s16 data = 0;
	struct sc8571_device_info *di = dev_data;

	if (!di || !vusb)
		return -EINVAL;

	ret = sc8571_read_word(di, SC8571_VAC2_ADC1_REG, &data);
	if (ret)
		return -EIO;

	if (di->device_id == SWITCHCAP_BQ25980)
		*vusb = (int)data * BQ25980_VAC_ADC_STEP / BQ25980_BASE_RATIO_UNIT;
	else
		/* VUSB ADC STEP: 6.25mV */
		*vusb = (int)data * 625 / 100;

	hwlog_info("VUSB_ADC=0x%x, vusb=%d\n", data, *vusb);
	return 0;
}

static int sc8571_get_device_temp(int *temp, void *dev_data)
{
	s16 data = 0;
	int ret;
	struct sc8571_device_info *di = dev_data;

	if (!di || !temp)
		return -EINVAL;

	ret = sc8571_read_word(di, SC8571_TDIE_ADC1_REG, &data);
	if (ret)
		return -EIO;

	if (di->device_id == SWITCHCAP_BQ25980)
		*temp = (int)data * BQ25980_TDIE_ADC_STEP / BQ25980_BASE_RATIO_UNIT;
	else
		/* TDIE_ADC STEP: 0.5C */
		*temp = (int)data * 5 / 10;

	hwlog_info("TDIE_ADC=0x%x, temp=%d\n", data, *temp);
	return 0;
}

static void sc8571_get_bq25980_watchdog_ms(int time, u8 *val)
{
	if (time >= BQ25980_WATCHDOG_CONFIG_TIMING_30000MS)
		*val = BQ25980_WATCHDOG_SET_30000MS;
	else if (time >= BQ25980_WATCHDOG_CONFIG_TIMING_5000MS)
		*val = BQ25980_WATCHDOG_SET_5000MS;
	else if (time >= BQ25980_WATCHDOG_CONFIG_TIMING_1000MS)
		*val = BQ25980_WATCHDOG_SET_1000MS;
	else if (time >= BQ25980_WATCHDOG_CONFIG_TIMING_500MS)
		*val = BQ25980_WATCHDOG_SET_500MS;
	else
		*val = BQ25980_WATCHDOG_SET_30000MS;
}

static void sc8571_get_watchdog_ms(int time, u8 *val)
{
	if (time >= SC8571_WATCHDOG_CONFIG_TIMING_39000MS)
		*val = SC8571_WATCHDOG_39000MS;
	else if (time >= SC8571_WATCHDOG_CONFIG_TIMING_6500MS)
		*val = SC8571_WATCHDOG_6500MS;
	else if (time >= SC8571_WATCHDOG_CONFIG_TIMING_1300MS)
		*val = SC8571_WATCHDOG_1300MS;
	else if (time >= SC8571_WATCHDOG_CONFIG_TIMING_650MS)
		*val = SC8571_WATCHDOG_650MS;
	else
		*val = SC8571_WATCHDOG_39000MS;
}

static int sc8571_config_watchdog_ms(int time, void *dev_data)
{
	int ret;
	u8 val;
	u8 reg = 0;
	struct sc8571_device_info *di = dev_data;

	if (!di)
		return -ENODEV;

	if (di->device_id == SWITCHCAP_BQ25980)
		sc8571_get_bq25980_watchdog_ms(time, &val);
	else
		sc8571_get_watchdog_ms(time, &val);

	ret = sc8571_write_mask(di, SC8571_CTRL3_REG, SC8571_WATCHDOG_MASK,
		SC8571_WATCHDOG_SHIFT, val);
	ret += sc8571_read_byte(di, SC8571_CTRL3_REG, &reg);
	if (ret)
		return -EIO;

	hwlog_info("config_watchdog_ms [0x%x]=0x%x\n", SC8571_CTRL3_REG, reg);
	return 0;
}

static int sc8571_kick_watchdog_ms(void *dev_data)
{
	return 0;
}

static int sc8571_config_vbat_ovp_th_mv(struct sc8571_device_info *di, int ovp_th)
{
	u8 value;
	int ret;

	if (ovp_th < SC8571_VBAT_OVP_BASE)
		ovp_th = SC8571_VBAT_OVP_BASE;

	if (ovp_th > SC8571_VBAT_OVP_MAX)
		ovp_th = SC8571_VBAT_OVP_MAX;

	value = (u8)((ovp_th - SC8571_VBAT_OVP_BASE) / SC8571_VBAT_OVP_STEP);
	ret = sc8571_write_mask(di, SC8571_VBAT_OVP_REG, SC8571_VBAT_OVP_MASK,
		SC8571_VBAT_OVP_SHIFT, value);
	ret += sc8571_read_byte(di, SC8571_VBAT_OVP_REG, &value);
	if (ret)
		return -EIO;

	hwlog_info("config_vbat_ovp_threshold_mv [0x%x]=0x%x\n",
		SC8571_VBAT_OVP_REG, value);
	return 0;
}

static int sc8571_config_ibat_ocp_th_ma(struct sc8571_device_info *di, int ocp_th)
{
	u8 value;
	int ret;
	int ibat_ocp_base = (di->device_id == SWITCHCAP_BQ25980) ?
		BQ25980_IBAT_OCP_BASE : SC8571_IBAT_OCP_BASE;

	if (ocp_th < ibat_ocp_base)
		ocp_th = ibat_ocp_base;

	if (ocp_th > SC8571_IBAT_OCP_MAX)
		ocp_th = SC8571_IBAT_OCP_MAX;

	value = (u8)((ocp_th - SC8571_IBAT_OCP_BASE) / SC8571_IBAT_OCP_STEP);
	ret = sc8571_write_mask(di, SC8571_IBAT_OCP_REG, SC8571_IBAT_OCP_MASK,
		SC8571_IBAT_OCP_SHIFT, value);
	ret += sc8571_read_byte(di, SC8571_IBAT_OCP_REG, &value);
	if (ret)
		return -EIO;

	hwlog_info("config_ibat_ocp_threshold_ma [0x%x]=0x%x\n",
		SC8571_IBAT_OCP_REG, value);
	return 0;
}

static void sc8571_get_bq25980_vac_ovp(int ovp_th, u8 *value)
{
	switch (ovp_th) {
	case BQ25980_VAC_OVP_6500MV:
		*value = BQ25980_VAC_OVP_SET_6500MV;
		break;
	case BQ25980_VAC_OVP_10500MV:
		*value = BQ25980_VAC_OVP_SET_10500MV;
		break;
	case BQ25980_VAC_OVP_12000MV:
		*value = BQ25980_VAC_OVP_SET_12000MV;
		break;
	case BQ25980_VAC_OVP_14000MV:
		*value = BQ25980_VAC_OVP_SET_14000MV;
		break;
	case BQ25980_VAC_OVP_16000MV:
		*value = BQ25980_VAC_OVP_SET_16000MV;
		break;
	case BQ25980_VAC_OVP_18000MV:
		*value = BQ25980_VAC_OVP_SET_18000MV;
		break;
	case BQ25980_VAC_OVP_22000MV:
		*value = BQ25980_VAC_OVP_SET_22000MV;
		break;
	case BQ25980_VAC_OVP_24000MV:
		*value = BQ25980_VAC_OVP_SET_24000MV;
		break;
	default:
		*value = BQ25980_VAC_OVP_SET_6500MV;
		break;
	}
}

static void sc8571_get_vac_ovp(int ovp_th, u8 *value)
{
	if (ovp_th < SC8571_VAC1_OVP_BASE)
		ovp_th = SC8571_VAC1_OVP_BASE;

	if (ovp_th > SC8571_VAC1_OVP_MAX)
		ovp_th = SC8571_VAC1_OVP_MAX;

	*value = (u8)((ovp_th - SC8571_VAC1_OVP_BASE) / SC8571_VAC1_OVP_STEP);
}

static int sc8571_config_vac_ovp_th_mv(struct sc8571_device_info *di, int ovp_th)
{
	u8 value;
	int ret;

	if (di->device_id == SWITCHCAP_BQ25980)
		sc8571_get_bq25980_vac_ovp(ovp_th, &value);
	else
		sc8571_get_vac_ovp(ovp_th, &value);

	ret = sc8571_write_mask(di, SC8571_VAC_OVP_REG, SC8571_VAC2_OVP_MASK,
		SC8571_VAC2_OVP_SHIFT, value);
	ret += sc8571_write_mask(di, SC8571_VAC_OVP_REG, SC8571_VAC1_OVP_MASK,
		SC8571_VAC1_OVP_SHIFT, value);
	ret += sc8571_read_byte(di, SC8571_VAC_OVP_REG, &value);
	if (ret)
		return -EIO;

	hwlog_info("config_ac_ovp_threshold_mv [0x%x]=0x%x\n",
		SC8571_VAC_OVP_REG, value);
	return 0;
}

static int sc8571_config_vbus_ovp_th_mv(struct sc8571_device_info *di, int ovp_th)
{
	u8 value;
	int ret;
	int vbus_ovp_max = (di->device_id == SWITCHCAP_BQ25980) ?
		BQ25980_VBUS_OVP_2_1_MAX : SC8571_VBUS_OVP_2_1_MAX;

	if (ovp_th < SC8571_VBUS_OVP_2_1_BASE)
		ovp_th = SC8571_VBUS_OVP_2_1_BASE;

	if (ovp_th > vbus_ovp_max)
		ovp_th = vbus_ovp_max;

	value = (u8)((ovp_th - SC8571_VBUS_OVP_2_1_BASE) / SC8571_VBUS_OVP_2_1_STEP);
	ret = sc8571_write_mask(di, SC8571_VBUS_OVP_REG, SC8571_VBUS_OVP_MASK,
		SC8571_VBUS_OVP_SHIFT, value);
	ret += sc8571_read_byte(di, SC8571_VBUS_OVP_REG, &value);
	if (ret)
		return -EIO;

	hwlog_info("config_vbus_ovp_threshole_mv [0x%x]=0x%x\n",
		SC8571_VBUS_OVP_REG, value);
	return 0;
}

static void sc8571_get_bq25980_ibus_ocp(int ocp_th, u8 *value, u8 mode)
{
	int ibus_ocp_max = (mode == SC8571_CHARGE_MODE_1_1) ?
		BQ25980_IBUS_OCP_1_1_MAX : BQ25980_IBUS_OCP_2_1_MAX;

	if (ocp_th < BQ25980_IBUS_OCP_BASE)
		ocp_th = BQ25980_IBUS_OCP_BASE;

	if (ocp_th > ibus_ocp_max)
		ocp_th = ibus_ocp_max;

	*value = (u8)((ocp_th - BQ25980_IBUS_OCP_BASE) / BQ25980_IBUS_OCP_STEP);
}

static void sc8571_get_ibus_ocp(int ocp_th, u8 *value)
{
	if (ocp_th < SC8571_IBUS_OCP_BASE)
		ocp_th = SC8571_IBUS_OCP_BASE;

	if (ocp_th > SC8571_IBUS_OCP_MAX)
		ocp_th = SC8571_IBUS_OCP_MAX;

	*value = (u8)((ocp_th - SC8571_IBUS_OCP_BASE) / SC8571_IBUS_OCP_STEP);
}

static int sc8571_config_ibus_ocp_th_ma(struct sc8571_device_info *di, int ocp_th, u8 mode)
{
	u8 value;
	int ret;

	if (di->device_id == SWITCHCAP_BQ25980)
		sc8571_get_bq25980_ibus_ocp(ocp_th, &value, mode);
	else
		sc8571_get_ibus_ocp(ocp_th, &value);

	ret = sc8571_write_mask(di, SC8571_IBUS_OCP_REG,
		SC8571_IBUS_OCP_MASK, SC8571_IBUS_OCP_SHIFT, value);
	ret += sc8571_read_byte(di, SC8571_IBUS_OCP_REG, &value);
	if (ret)
		return -EIO;

	hwlog_info("config_ibus_ocp_threshold_ma [0x%x]=0x%x\n",
		SC8571_IBUS_OCP_REG, value);
	return 0;
}

static void sc8571_get_bq25980_frequency(int data, int *freq, int *freq_shift)
{
	switch (data) {
	case BQ25980_SW_FREQ_187P5KHZ:
		*freq = BQ25980_FSW_SET_SW_FREQ_187P5KHZ;
		*freq_shift = BQ25980_FREQ_SHIFT_NORMAL;
		break;
	case BQ25980_SW_FREQ_250KHZ:
		*freq = BQ25980_FSW_SET_SW_FREQ_250KHZ;
		*freq_shift = BQ25980_FREQ_SHIFT_NORMAL;
		break;
	case BQ25980_SW_FREQ_300KHZ:
		*freq = BQ25980_FSW_SET_SW_FREQ_300KHZ;
		*freq_shift = BQ25980_FREQ_SHIFT_NORMAL;
		break;
	case BQ25980_SW_FREQ_375KHZ:
		*freq = BQ25980_FSW_SET_SW_FREQ_375KHZ;
		*freq_shift = BQ25980_FREQ_SHIFT_NORMAL;
		break;
	case BQ25980_SW_FREQ_500KHZ:
		*freq = BQ25980_FSW_SET_SW_FREQ_500KHZ;
		*freq_shift = BQ25980_FREQ_SHIFT_NORMAL;
		break;
	case BQ25980_SW_FREQ_750KHZ:
		*freq = BQ25980_FSW_SET_SW_FREQ_750KHZ;
		*freq_shift = BQ25980_FREQ_SHIFT_NORMAL;
		break;
	default:
		*freq = BQ25980_FSW_SET_SW_FREQ_500KHZ;
		*freq_shift = BQ25980_FREQ_SHIFT_NORMAL;
		break;
	}
}

static void sc8571_get_frequency(int data, int *freq, int *freq_shift)
{
	switch (data) {
	case SC8571_SW_FREQ_300KHZ:
		*freq  = SC8571_FSW_SET_300KHZ;
		*freq_shift = SC8571_FREQ_SHIFT_NORMINAL;
		break;
	case SC8571_SW_FREQ_350KHZ:
		*freq  = SC8571_FSW_SET_350KHZ;
		*freq_shift = SC8571_FREQ_SHIFT_NORMINAL;
		break;
	case SC8571_SW_FREQ_400KHZ:
		*freq  = SC8571_FSW_SET_400KHZ;
		*freq_shift = SC8571_FREQ_SHIFT_NORMINAL;
		break;
	case SC8571_SW_FREQ_450KHZ:
		*freq  = SC8571_FSW_SET_450KHZ;
		*freq_shift = SC8571_FREQ_SHIFT_NORMINAL;
		break;
	case SC8571_SW_FREQ_500KHZ:
		*freq = SC8571_FSW_SET_500KHZ;
		*freq_shift = SC8571_FREQ_SHIFT_NORMINAL;
		break;
	case SC8571_SW_FREQ_550KHZ:
		*freq  = SC8571_FSW_SET_550KHZ;
		*freq_shift = SC8571_FREQ_SHIFT_NORMINAL;
		break;
	case SC8571_SW_FREQ_600KHZ:
		*freq  = SC8571_FSW_SET_600KHZ;
		*freq_shift = SC8571_FREQ_SHIFT_NORMINAL;
		break;
	case SC8571_SW_FREQ_750KHZ:
		*freq  = SC8571_FSW_SET_750KHZ;
		*freq_shift = SC8571_FREQ_SHIFT_NORMINAL;
		break;
	default:
		*freq  = SC8571_FSW_SET_500KHZ;
		*freq_shift = SC8571_FREQ_SHIFT_NORMINAL;
		break;
	}
}

static int sc8571_config_switching_frequency(int data, struct sc8571_device_info *di)
{
	int freq;
	int freq_shift;
	int ret;

	if (di->device_id == SWITCHCAP_BQ25980)
		sc8571_get_bq25980_frequency(data, &freq, &freq_shift);
	else
		sc8571_get_frequency(data, &freq, &freq_shift);

	ret = sc8571_write_mask(di, SC8571_CTRL3_REG, SC8571_FSW_SET_MASK,
		SC8571_FSW_SET_SHIFT, freq);
	ret += sc8571_write_mask(di, SC8571_CTRL5_REG, SC8571_FREQ_SHIFT_MASK,
		SC8571_FREQ_SHIFT_SHIFT, freq_shift);
	if (ret)
		return -EIO;

	hwlog_info("config_switching_freq [0x%x]=0x%x\n", SC8571_CTRL3_REG, freq);
	hwlog_info("config_adjustable_switching_freq [0x%x]=0x%x\n", SC8571_CTRL5_REG, freq_shift);

	return 0;
}

static int sc8571_congfig_ibat_sns_res(struct sc8571_device_info *di)
{
	int ret;
	u8 value;

	if (di->sense_r_config == SENSE_R_5_MOHM)
		value = SC8571_IBAT_SNS_RES_5MHM;
	else
		value = SC8571_IBAT_SNS_RES_2MHM;

	ret = sc8571_write_mask(di, SC8571_CTRL4_REG,
		SC8571_IBAT_SNS_RES_MASK,
		SC8571_IBAT_SNS_RES_SHIFT, value);
	if (ret)
		return -EIO;

	hwlog_info("congfig_ibat_sns_res=%d\n", di->sense_r_config);
	return 0;
}

static int sc8571_threshold_reg_init(struct sc8571_device_info *di, u8 mode)
{
	int ret, ibus_ocp, ibat_ocp, vbat_ovp, fsw;

	if (mode == SC8571_CHARGE_MODE_1_1) {
		ibus_ocp = di->sc8571_lvc_para.ibus_ocp;
		ibat_ocp = di->sc8571_lvc_para.ibat_ocp;
		vbat_ovp = di->sc8571_lvc_para.vbat_ovp;
		fsw = di->sc8571_lvc_para.switching_frequency;
	} else if (mode == SC8571_CHARGE_MODE_2_1) {
		ibus_ocp = di->sc8571_sc_para.ibus_ocp;
		ibat_ocp = di->sc8571_sc_para.ibat_ocp;
		vbat_ovp = di->sc8571_sc_para.vbat_ovp;
		fsw = di->sc8571_sc_para.switching_frequency;
	} else {
		ibus_ocp = SC8571_IBUS_OCP_TH_INIT;
		ibat_ocp = SC8571_IBAT_OCP_TH_INIT;
		vbat_ovp = SC8571_VBAT_OVP_TH_INIT;
		fsw = SC8571_SW_FREQ_500KHZ;
	}
	ret = sc8571_config_vac_ovp_th_mv(di, SC8571_VAC_OVP_TH_INIT);
	ret += sc8571_config_vbus_ovp_th_mv(di, SC8571_VBUS_OVP_TH_INIT);
	ret += sc8571_config_ibus_ocp_th_ma(di, ibus_ocp, mode);
	ret += sc8571_config_vbat_ovp_th_mv(di, vbat_ovp);
	ret += sc8571_config_ibat_ocp_th_ma(di, ibat_ocp);
	if (ret) {
		hwlog_err("protect threshold init fail\n");
		return ret;
	}

	ret = sc8571_config_switching_frequency(fsw, di);
	if (ret)
		hwlog_err("config switching frequency fail\n");

	return ret;
}

static int sc8571_set_bq25980_chg_config(void *dev_data)
{
	int ret;
	struct sc8571_device_info *di = dev_data;

	if (!di)
		return -ENODEV;

	ret = sc8571_write_mask(di, BQ25980_CHARGER_CONTROL_1_REG, BQ25980_IBUS_UCP_MASK,
		BQ25980_IBUS_UCP_SHIFT, BQ25980_IBUS_UCP_250MA);
	ret += sc8571_write_mask(di, BQ25980_CHARGER_CONFIG_1_REG, BQ25980_CHARGER_CONFIG_1_MASK,
		BQ25980_CHARGER_CONFIG_1_SHIFT, BQ25980_CHARGER_CONFIG_ENABLE);
	ret += sc8571_write_mask(di, BQ25980_CHARGER_CONTROL_1_REG, BQ25980_CHARGER_CONFIG_2_MASK,
		BQ25980_CHARGER_CONFIG_2_SHIFT, BQ25980_CHARGER_CONFIG_ENABLE);
	ret += sc8571_write_mask(di, BQ25980_CHARGER_CONTROL_1_REG, BQ25980_CHARGER_CONFIG_3_MASK,
		BQ25980_CHARGER_CONFIG_3_SHIFT, BQ25980_CHARGER_CONFIG_ENABLE);
	ret += sc8571_write_mask(di, BQ25980_CHARGER_CONFIG_4_REG, BQ25980_CHARGER_CONFIG_4_MASK,
		BQ25980_CHARGER_CONFIG_4_SHIFT, BQ25980_CHARGER_CONFIG_ENABLE);
	if (ret)
		hwlog_err("set charge config fail\n");

	return ret;
}

static int sc8571_lvc_charge_enable(int enable, void *dev_data)
{
	int ret = 0;
	u8 ctrl1_reg = 0;
	u8 mode = SC8571_CHARGE_MODE_1_1;
	u8 chg_en = enable ? SC8571_CHG_ENABLE : SC8571_CHG_DISABLE;
	struct sc8571_device_info *di = dev_data;

	if (!di)
		return -ENODEV;

	ret = sc8571_write_mask(di, SC8571_CTRL2_REG, SC8571_CHG_EN_MASK,
		SC8571_CHG_EN_SHIFT, SC8571_CHG_DISABLE);
	if (di->device_id == SWITCHCAP_BQ25980)
		ret += sc8571_set_bq25980_chg_config(di);
	ret += sc8571_write_mask(di, SC8571_CTRL2_REG, SC8571_CHARGE_MODE_MASK,
		SC8571_CHARGE_MODE_SHIFT, mode);
	if (chg_en)
		ret += sc8571_threshold_reg_init(di, mode);
	ret += sc8571_write_mask(di, SC8571_CTRL2_REG, SC8571_CHG_EN_MASK,
		SC8571_CHG_EN_SHIFT, chg_en);
	ret += sc8571_read_byte(di, SC8571_CTRL2_REG, &ctrl1_reg);
	if (ret)
		return -EIO;

	hwlog_info("ic_role=%d, charge_enable [0x%x]=0x%x\n",
		di->ic_role, SC8571_CTRL2_REG, ctrl1_reg);

	return 0;
}

static int sc8571_sc_charge_enable(int enable, void *dev_data)
{
	int ret = 0;
	u8 ctrl1_reg = 0;
	u8 mode = SC8571_CHARGE_MODE_2_1;
	u8 chg_en = enable ? SC8571_CHG_ENABLE : SC8571_CHG_DISABLE;
	struct sc8571_device_info *di = dev_data;

	if (!di)
		return -ENODEV;

	ret = sc8571_write_mask(di, SC8571_CTRL2_REG, SC8571_CHG_EN_MASK,
		SC8571_CHG_EN_SHIFT, SC8571_CHG_DISABLE);
	if (di->device_id == SWITCHCAP_BQ25980)
		ret += sc8571_set_bq25980_chg_config(di);
	ret += sc8571_write_mask(di, SC8571_CTRL2_REG, SC8571_CHARGE_MODE_MASK,
		SC8571_CHARGE_MODE_SHIFT, mode);
	if (chg_en)
		ret += sc8571_threshold_reg_init(di, mode);
	ret += sc8571_write_mask(di, SC8571_CTRL2_REG, SC8571_CHG_EN_MASK,
		SC8571_CHG_EN_SHIFT, chg_en);
	ret += sc8571_read_byte(di, SC8571_CTRL2_REG, &ctrl1_reg);
	if (ret)
		return -EIO;

	hwlog_info("ic_role=%d, charge_enable [0x%x]=0x%x\n",
		di->ic_role, SC8571_CTRL2_REG, ctrl1_reg);

	return 0;
}

static int sc8571_reg_reset(struct sc8571_device_info *di)
{
	int ret;
	u8 ctrl1_reg = 0;

	ret = sc8571_write_mask(di, SC8571_CTRL2_REG,
		SC8571_REG_RST_MASK, SC8571_REG_RST_SHIFT,
		SC8571_REG_RST_RESET);
	if (ret)
		return -EIO;

	power_usleep(DT_USLEEP_1MS);
	ret = sc8571_read_byte(di, SC8571_CTRL2_REG, &ctrl1_reg);
	ret += sc8571_config_vac_ovp_th_mv(di, SC8571_VAC_OVP_TH_INIT);
	if (ret)
		return -EIO;

	hwlog_info("reg_reset [0x%x]=0x%x\n", SC8571_CTRL2_REG, ctrl1_reg);
	return 0;
}

static int sc8571_chip_init(void *dev_data)
{
	return 0;
}

static int sc8571_reg_init(struct sc8571_device_info *di)
{
	int ret;

	ret = sc8571_write_byte(di, SC8571_CTRL3_REG, SC8571_CONTROL_REG_INIT);
	if (di->device_id == SWITCHCAP_SC8571)
		ret += sc8571_write_byte(di, SC8571_PMID2OUT_OVP_UVP_REG,
			SC8571_PMID2OUT_OVP_UVP_INIT);
	ret += sc8571_threshold_reg_init(di, SC8571_CHARGE_MODE_2_1);
	ret += sc8571_congfig_ibat_sns_res(di);
	ret += sc8571_write_mask(di, SC8571_ADC_CTRL_REG,
		SC8571_ADC_EN_MASK, SC8571_ADC_EN_SHIFT, SC8571_ADC_ENABLE);
	/* set IBUS_UCP_DEGLITCH 5ms */
	ret += sc8571_write_mask(di, SC8571_CTRL4_REG, SC8571_IBUS_UCP_FALL_DEG_MASK,
		SC8571_IBUS_UCP_FALL_DEG_SHIFT, SC8571_IBUS_UCP_FALL_5MS);
	ret += sc8571_write_mask(di, SC8571_TEPMP_CTRL_REG, SC8571_TSBUS_FLT_DIS_MASK,
		SC8571_TSBUS_FLT_DIS_SHIFT, SC8571_TSBUS_FLT_DISABLE);
	ret += sc8571_write_mask(di, SC8571_TEPMP_CTRL_REG, SC8571_TSBAT_FLT_DIS_MASK,
		SC8571_TSBAT_FLT_DIS_SHIFT, SC8571_TSBAT_FLT_DISABLE);
	/* disable alarm interupt */
	ret += sc8571_write_mask(di, SC8571_MASK1_REG, SC8571_BUS_OVP_ALM_MASK,
		SC8571_BUS_OVP_ALM_MASK_SHIFT, SC8571_BUS_OVP_ALM_MASK_DISABLE);
	ret += sc8571_write_mask(di, SC8571_MASK1_REG, SC8571_BAT_OCP_ALM_MASK,
		SC8571_BAT_OCP_ALM_MASK_SHIFT, SC8571_BAT_OCP_ALM_DISALLE);
	ret += sc8571_write_mask(di, SC8571_MASK1_REG, SC8571_BAT_OVP_ALM_MASK,
		SC8571_BAT_OVP_ALM_MASK_SHIFT, SC8571_BAT_OVP_ALM_MASK_DISABLE);
	ret += sc8571_write_mask(di, SC8571_MASK4_REG, SC8571_TDIE_ALM_MASK,
		SC8571_TDIE_ALM_MASK_SHIFT, SC8571_TDIE_ALM_DISABLE);
	ret += sc8571_write_mask(di, SC8571_MASK4_REG, SC8571_TSBUS_TSBAT_ALM_MASK,
		SC8571_TSBUS_TSBAT_ALM_MASK_SHIFT, SC8571_TSBUS_TSBAT_ALM_DISABLE);
	if (ret)
		hwlog_err("reg_init fail\n");

	return ret;
}

static int sc8571_enable_adc(int enable, void *dev_data)
{
	int ret;
	u8 reg = 0;
	u8 value = enable ? SC8571_ADC_ENABLE : SC8571_ADC_DISABLE;
	struct sc8571_device_info *di = dev_data;

	if (!di)
		return -ENODEV;

	ret = sc8571_write_mask(di, SC8571_ADC_CTRL_REG,
		SC8571_ADC_EN_MASK, SC8571_ADC_EN_SHIFT, value);
	if (ret)
		return -EPERM;

	ret = sc8571_read_byte(di, SC8571_ADC_CTRL_REG, &reg);
	if (ret)
		return -EPERM;

	hwlog_info("adc_enable [0x%x]=0x%x\n", SC8571_ADC_CTRL_REG, reg);
	return 0;
}

static int sc8571_charge_init(void *dev_data)
{
	struct sc8571_device_info *di = dev_data;

	if (!di)
		return -ENODEV;

	if (sc8571_reg_init(di))
		return -EINVAL;

	di->init_finished = true;
	return 0;
}

static int sc8571_reg_reset_and_init(void *dev_data)
{
	int ret;
	struct sc8571_device_info *di = dev_data;

	if (!di)
		return -ENODEV;

	hwlog_info("reg reset and init enter\n");
	ret = sc8571_reg_reset(di);
	ret += sc8571_reg_init(di);

	return ret;
}

static int sc8571_charge_exit(void *dev_data)
{
	int ret;
	struct sc8571_device_info *di = dev_data;

	if (!di)
		return -ENODEV;

	ret = sc8571_sc_charge_enable(SC8571_SWITCHCAP_DISABLE, di);
	di->init_finished = false;
	di->int_notify_enabled = false;

	return ret;
}

static int sc8571_batinfo_exit(void *dev_data)
{
	return 0;
}

static int sc8571_batinfo_init(void *dev_data)
{
	struct sc8571_device_info *di = dev_data;

	if (!di)
		return -ENODEV;

	return sc8571_chip_init(di);
}

/* print the register head in charging process */
static int sc8571_register_head(char *buffer, int size, void *dev_data)
{
	struct sc8571_device_info *di = dev_data;

	if (!di || !buffer)
		return -EINVAL;

	snprintf(buffer, size,
		"dev       mode   Ibus   Vbus   Vusb   Ibat   Vbat   Temp   ");

	return 0;
}

/* print the register value in charging process */
static int sc8571_dump_reg(char *buffer, int size, void *dev_data)
{
	struct sc8571_dump_data data = { 0 };
	char buff[SC8571_BUF_LEN] = { 0 };
	u8 reg = 0;
	struct sc8571_device_info *di = dev_data;

	if (!di || !buffer)
		return -EINVAL;

	(void)sc8571_get_vbus_mv(&data.vbus, di);
	(void)sc8571_get_ibat_ma(&data.ibat, di);
	(void)sc8571_get_ibus_ma(&data.ibus, di);
	(void)sc8571_get_device_temp(&data.temp, di);
	(void)sc8571_get_vusb_mv(&data.vusb, di);
	(void)sc8571_read_byte(di, SC8571_CTRL2_REG, &reg);

	snprintf(buff, sizeof(buff), "%-10s", di->name);
	strncat(buffer, buff, strlen(buff));

	if (sc8571_is_device_close(di))
		snprintf(buff, sizeof(buff), "%s", "OFF    ");
	else if (((reg & SC8571_CHARGE_MODE_MASK) >> SC8571_CHARGE_MODE_SHIFT) ==
		SC8571_CHARGE_MODE_1_1)
		snprintf(buff, sizeof(buff), "%s", "LVC    ");
	else if (((reg & SC8571_CHARGE_MODE_MASK) >> SC8571_CHARGE_MODE_SHIFT) ==
		SC8571_CHARGE_MODE_2_1)
		snprintf(buff, sizeof(buff), "%s", "SC     ");

	strncat(buffer, buff, strlen(buff));
	snprintf(buff, sizeof(buff), "%-7d%-7d%-7d%-7d%-7d%-7d",
		data.ibus, data.vbus, data.vusb, data.ibat, sc8571_get_vbat_mv(di), data.temp);
	strncat(buffer, buff, strlen(buff));

	return 0;
}

static struct dc_ic_ops g_sc8571_lvc_ops = {
	.dev_name = "sc8571",
	.ic_init = sc8571_charge_init,
	.ic_exit = sc8571_charge_exit,
	.ic_enable = sc8571_lvc_charge_enable,
	.ic_discharge = sc8571_discharge,
	.is_ic_close = sc8571_is_device_close,
	.get_ic_id = sc8571_get_device_id,
	.config_ic_watchdog = sc8571_config_watchdog_ms,
	.kick_ic_watchdog = sc8571_kick_watchdog_ms,
	.ic_reg_reset_and_init = sc8571_reg_reset_and_init,
	.get_max_ibat = sc8571_get_lvc_max_ibat,
};

static struct dc_ic_ops g_sc8571_sc_ops = {
	.dev_name = "sc8571",
	.ic_init = sc8571_charge_init,
	.ic_exit = sc8571_charge_exit,
	.ic_enable = sc8571_sc_charge_enable,
	.ic_discharge = sc8571_discharge,
	.is_ic_close = sc8571_is_device_close,
	.get_ic_id = sc8571_get_device_id,
	.config_ic_watchdog = sc8571_config_watchdog_ms,
	.kick_ic_watchdog = sc8571_kick_watchdog_ms,
	.ic_reg_reset_and_init = sc8571_reg_reset_and_init,
	.get_max_ibat = sc8571_get_sc_max_ibat,
};

static struct dc_batinfo_ops g_sc8571_batinfo_ops = {
	.init = sc8571_batinfo_init,
	.exit = sc8571_batinfo_exit,
	.get_bat_btb_voltage = sc8571_get_max_vbat_mv,
	.get_bat_package_voltage = sc8571_get_max_vbat_mv,
	.get_vbus_voltage = sc8571_get_vbus_mv,
	.get_bat_current = sc8571_get_ibat_ma,
	.get_ic_ibus = sc8571_get_ibus_ma,
	.get_ic_temp = sc8571_get_device_temp,
};

static struct power_log_ops g_sc8571_log_ops = {
	.dev_name = "sc8571",
	.dump_log_head = sc8571_register_head,
	.dump_log_content = sc8571_dump_reg,
};

static void sc8571_init_ops_dev_data(struct sc8571_device_info *di)
{
	memcpy(&di->lvc_ops, &g_sc8571_lvc_ops, sizeof(struct dc_ic_ops));
	di->lvc_ops.dev_data = (void *)di;
	memcpy(&di->sc_ops, &g_sc8571_sc_ops, sizeof(struct dc_ic_ops));
	di->sc_ops.dev_data = (void *)di;
	memcpy(&di->batinfo_ops, &g_sc8571_batinfo_ops, sizeof(struct dc_batinfo_ops));
	di->batinfo_ops.dev_data = (void *)di;
	memcpy(&di->log_ops, &g_sc8571_log_ops, sizeof(struct power_log_ops));
	di->log_ops.dev_data = (void *)di;

	if (!di->ic_role) {
		snprintf(di->name, CHIP_DEV_NAME_LEN, "sc8571");
	} else {
		snprintf(di->name, CHIP_DEV_NAME_LEN, "sc8571_%d", di->ic_role);
		di->lvc_ops.dev_name = di->name;
		di->sc_ops.dev_name = di->name;
		di->log_ops.dev_name = di->name;
	}
}

static void sc8571_ops_register(struct sc8571_device_info *di)
{
	int ret;

	sc8571_init_ops_dev_data(di);

	ret = dc_ic_ops_register(LVC_MODE, di->ic_role, &di->lvc_ops);
	ret += dc_ic_ops_register(SC_MODE, di->ic_role, &di->sc_ops);
	ret += dc_batinfo_ops_register(di->ic_role, &di->batinfo_ops, di->device_id);
	if (ret)
		hwlog_err("sysinfo ops register fail\n");

	power_log_ops_register(&di->log_ops);
}

static void sc8571_fault_event_notify(unsigned long event, void *data)
{
	power_event_anc_notify(POWER_ANT_DC_FAULT, event, data);
}

static void sc8571_interrupt_handle(struct sc8571_device_info *di,
	struct nty_data *data)
{
	int val = 0;
	u8 flag1 = data->event1;
	u8 flag2 = data->event2;
	u8 flag3 = data->event3;

	if (flag3 & SC8571_VAC1_OVP_FLAG_MASK) {
		hwlog_info("VAC1 OVP happened\n");
		sc8571_fault_event_notify(POWER_NE_DC_FAULT_AC_OVP, data);
	} else if (flag3 & SC8571_VAC2_OVP_FLAG_MASK) {
		hwlog_info("VAC2 OVP happened\n");
		sc8571_fault_event_notify(POWER_NE_DC_FAULT_AC_OVP, data);
	} else if (flag1 & SC8571_VBAT_OVP_FLAG_MASK) {
		val = sc8571_get_vbat_mv(di);
		hwlog_info("BAT OVP happened, vbat=%d\n", val);
		if (val >= SC8571_VBAT_OVP_TH_INIT)
			sc8571_fault_event_notify(POWER_NE_DC_FAULT_VBAT_OVP, data);
	} else if (flag1 & SC8571_IBAT_OCP_FLAG_MASK) {
		sc8571_get_ibat_ma(&val, di);
		hwlog_info("BAT OCP happened, ibat=%d\n", val);
		if (val >= SC8571_IBAT_OCP_TH_INIT)
			sc8571_fault_event_notify(POWER_NE_DC_FAULT_IBAT_OCP, data);
	} else if (flag1 & SC8571_VBUS_OVP_FLAG_MASK) {
		sc8571_get_vbus_mv(&val, di);
		hwlog_info("BUS OVP happened, vbus=%d\n", val);
		if (val >= SC8571_VBUS_OVP_TH_INIT)
			sc8571_fault_event_notify(POWER_NE_DC_FAULT_VBUS_OVP, data);
	} else if (flag2 & SC8571_IBUS_OCP_FLAG_MASK) {
		sc8571_get_ibus_ma(&val, di);
		hwlog_info("BUS OCP happened, ibus=%d\n", val);
		if (val >= SC8571_IBUS_OCP_TH_INIT)
		sc8571_fault_event_notify(POWER_NE_DC_FAULT_IBUS_OCP, data);
	} else if (flag1 & SC8571_OUT_OVP_FLAG_MASK) {
		hwlog_info("VOUT OVP happened\n");
	}
}

static void sc8571_interrupt_work(struct work_struct *work)
{
	u8 flag[6] = { 0 };
	struct sc8571_device_info *di = NULL;
	struct nty_data *data = NULL;

	hwlog_info("sc8571_interrupt_work\n");
	if (!work)
		return;

	di = container_of(work, struct sc8571_device_info, irq_work);
	if (!di || !di->client) {
		hwlog_err("di is null\n");
		return;
	}

	(void)sc8571_read_byte(di, SC8571_FLAG1_REG, &flag[1]);
	(void)sc8571_read_byte(di, SC8571_FLAG2_REG, &flag[2]);
	(void)sc8571_read_byte(di, SC8571_FLAG3_REG, &flag[3]);
	(void)sc8571_read_byte(di, SC8571_FLAG4_REG, &flag[4]);
	(void)sc8571_read_byte(di, SC8571_FLAG5_REG, &flag[5]);

	data = &(di->notify_data);
	data->event1 = flag[1];
	data->event2 = flag[2];
	data->event3 = flag[3];
	data->addr = di->client->addr;

	if (di->int_notify_enabled)
		sc8571_interrupt_handle(di, data);

	if (flag[1])
		hwlog_info("interrupt reg[0x%x]=0x%x\n", SC8571_FLAG1_REG, flag[1]);
	if (flag[2])
		hwlog_info("interrupt reg[0x%x]=0x%x\n", SC8571_FLAG2_REG, flag[2]);
	if (flag[3])
		hwlog_info("interrupt reg[0x%x]=0x%x\n", SC8571_FLAG3_REG, flag[3]);
	if (flag[4])
		hwlog_info("interrupt reg[0x%x]=0x%x\n", SC8571_FLAG4_REG, flag[4]);
	if (flag[5])
		hwlog_info("interrupt reg[0x%x]=0x%x\n", SC8571_FLAG5_REG, flag[5]);

	enable_irq(di->irq_int);
}

static irqreturn_t sc8571_interrupt(int irq, void *_di)
{
	struct sc8571_device_info *di = _di;

	if (!di)
		return IRQ_HANDLED;

	if (di->init_finished)
		di->int_notify_enabled = true;

	hwlog_info("int happened\n");

	disable_irq_nosync(di->irq_int);
	schedule_work(&di->irq_work);

	return IRQ_HANDLED;
}

static int sc8571_irq_init(struct sc8571_device_info *di,
	struct device_node *np)
{
	int ret;

	INIT_WORK(&di->irq_work, sc8571_interrupt_work);
	ret = power_gpio_config_interrupt(np,
		"intr_gpio", "sc8571_gpio_int", &di->gpio_int, &di->irq_int);
	if (ret)
		return ret;

	ret = request_irq(di->irq_int, sc8571_interrupt,
		IRQF_TRIGGER_FALLING, "sc8571_int_irq", di);
	if (ret) {
		hwlog_err("gpio irq request fail\n");
		di->irq_int = -1;
		gpio_free(di->gpio_int);
		return ret;
	}

	enable_irq_wake(di->irq_int);
	return 0;
}

static void sc8571_parse_dts(struct device_node *np,
	struct sc8571_device_info *di)
{
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "ic_role",
		(u32 *)&(di->ic_role), IC_ONE);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"sense_r_config", &di->sense_r_config, SENSE_R_5_MOHM);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"sense_r_actual", &di->sense_r_actual, SENSE_R_5_MOHM);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"ovp_mos_switch_off", &di->ovp_mos_switch_off, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"manual_otg_mode", &di->manual_otg_mode, 0);
}

static void sc8571_mode_para_init(struct sc8571_device_info *di)
{
	di->sc8571_lvc_para.ibus_ocp = SC8571_LVC_IBUS_OCP_TH_INIT;
	di->sc8571_lvc_para.ibat_ocp = SC8571_IBAT_OCP_TH_INIT;
	di->sc8571_lvc_para.vbat_ovp = SC8571_VBAT_OVP_TH_INIT;
	di->sc8571_lvc_para.switching_frequency = SC8571_SW_FREQ_500KHZ;
	di->sc8571_sc_para.ibus_ocp = SC8571_SC_IBUS_OCP_TH_INIT;
	di->sc8571_sc_para.ibat_ocp = SC8571_IBAT_OCP_TH_INIT;
	di->sc8571_sc_para.vbat_ovp = SC8571_VBAT_OVP_TH_INIT;
	di->sc8571_sc_para.switching_frequency = SC8571_SW_FREQ_500KHZ;
}

static int sc8571_parse_mode_para_dts(struct device_node *np,
	struct sc8571_mode_para *data, const char *name, int id)
{
	int array_len, col, row, idata;
	int index = -1; /* -1 : illegal value */
	const char *device_name = dc_get_device_name_without_mode(id);
	const char *tmp_string = NULL;

	array_len = power_dts_read_count_strings(power_dts_tag(HWLOG_TAG), np,
		name, SC8571_COMP_MAX_NUM, SC8571_INFO_TOTAL);
	if (array_len < 0)
		return -EPERM;

	for (row = 0; row < array_len / SC8571_INFO_TOTAL; row++) {
		col = row * SC8571_INFO_TOTAL + SC8571_INFO_IC_NAME;
		if (power_dts_read_string_index(power_dts_tag(HWLOG_TAG), np, name, col, &tmp_string))
			return -EPERM;

		if (!strcmp(tmp_string, device_name)) {
			strncpy(data->ic_name, tmp_string, CHIP_DEV_NAME_LEN - 1);
			index = row;
			break;
		}

		if (!strcmp(tmp_string, "default")) {
			strncpy(data->ic_name, tmp_string, CHIP_DEV_NAME_LEN - 1);
			index = row;
		}
	}

	if (index >= 0) {
		col = index * SC8571_INFO_TOTAL + SC8571_INFO_MAX_IBAT;
		power_dts_read_string_index(power_dts_tag(HWLOG_TAG), np, name, col, &tmp_string);
		kstrtoint(tmp_string, POWER_BASE_DEC, &idata);
		data->max_ibat = idata;
		col = index * SC8571_INFO_TOTAL + SC8571_INFO_IBUS_OCP;
		power_dts_read_string_index(power_dts_tag(HWLOG_TAG), np, name, col, &tmp_string);
		kstrtoint(tmp_string, POWER_BASE_DEC, &idata);
		data->ibus_ocp = idata;
		col = index * SC8571_INFO_TOTAL + SC8571_INFO_IBAT_OCP;
		power_dts_read_string_index(power_dts_tag(HWLOG_TAG), np, name, col, &tmp_string);
		kstrtoint(tmp_string, POWER_BASE_DEC, &idata);
		data->ibat_ocp = idata;
		col = index * SC8571_INFO_TOTAL + SC8571_INFO_VBAT_OVP;
		power_dts_read_string_index(power_dts_tag(HWLOG_TAG), np, name, col, &tmp_string);
		kstrtoint(tmp_string, POWER_BASE_DEC, &idata);
		data->vbat_ovp = idata;
		col = index * SC8571_INFO_TOTAL + SC8571_INFO_SW_FREQ;
		power_dts_read_string_index(power_dts_tag(HWLOG_TAG), np, name, col, &tmp_string);
		kstrtoint(tmp_string, POWER_BASE_DEC, &idata);
		data->switching_frequency = idata;
	}

	return 0;
}

static int sc8571_otg_notifier_call(struct notifier_block *nb, unsigned long event, void *data)
{
	struct sc8571_device_info *di = container_of(nb, struct sc8571_device_info, otg_nb);

	if (!di)
		return NOTIFY_OK;

	switch (event) {
	case POWER_NE_OTG_INSERT:
		hwlog_info("otg insert event happen\n");
		sc8571_otg_enable(di, true);
		break;
	case POWER_NE_OTG_REMOVE:
		hwlog_info("otg remove event happen\n");
		sc8571_otg_enable(di, false);
		break;
	default:
		break;
	}
	return NOTIFY_OK;
}

static int sc8571_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int ret;
	struct sc8571_device_info *di = NULL;
	struct device_node *np = NULL;

	if (!client || !client->dev.of_node || !id)
		return -ENODEV;

	di = devm_kzalloc(&client->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	di->dev = &client->dev;
	np = di->dev->of_node;
	di->client = client;
	di->chip_already_init = 1;

	ret = sc8571_get_device_id(di);
	if ((ret < 0) || (ret == DC_DEVICE_ID_END))
		goto sc8571_fail_0;

	sc8571_parse_dts(np, di);
	sc8571_mode_para_init(di);
	sc8571_parse_mode_para_dts(np, &di->sc8571_lvc_para, "lvc_para", di->device_id);
	sc8571_parse_mode_para_dts(np, &di->sc8571_sc_para, "sc_para", di->device_id);

	(void)power_pinctrl_config(di->dev, "pinctrl-names", 1); /* 1:pinctrl-names length */

	ret = sc8571_reg_reset_and_init(di);
	if (ret)
		goto sc8571_fail_0;
	if (di->manual_otg_mode) {
		di->otg_nb.notifier_call = sc8571_otg_notifier_call;
		ret = power_event_bnc_register(POWER_BNT_OTG, &di->otg_nb);
		if (ret)
			goto sc8571_fail_0;
	}

	ret = sc8571_irq_init(di, np);
	if (ret)
		goto sc8571_fail_1;

	sc8571_ops_register(di);
	i2c_set_clientdata(client, di);
	hwlog_info("probe ok\n");
	return 0;

sc8571_fail_1:
	free_irq(di->irq_int, di);
	gpio_free(di->gpio_int);
	if (di->manual_otg_mode)
		(void)power_event_bnc_unregister(POWER_BNT_OTG, &di->otg_nb);
sc8571_fail_0:
	di->chip_already_init = 0;
	devm_kfree(&client->dev, di);

	return ret;
}

static int sc8571_remove(struct i2c_client *client)
{
	struct sc8571_device_info *di = i2c_get_clientdata(client);

	if (!di)
		return -ENODEV;

	hwlog_info("remove enter\n");
	if (di->manual_otg_mode)
		(void)power_event_bnc_unregister(POWER_BNT_OTG, &di->otg_nb);
	if (di->irq_int)
		free_irq(di->irq_int, di);
	if (di->gpio_int)
		gpio_free(di->gpio_int);

	sc8571_reg_reset(di);
	return 0;
}

static void sc8571_shutdown(struct i2c_client *client)
{
	struct sc8571_device_info *di = i2c_get_clientdata(client);

	if (!di)
		return;

	if (di->irq_int)
		free_irq(di->irq_int, di);
	if (di->gpio_int)
		gpio_free(di->gpio_int);

	sc8571_reg_reset(di);
}

#ifdef CONFIG_PM
static int sc8571_i2c_suspend(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct sc8571_device_info *di = NULL;

	if (!client)
		return 0;

	di = i2c_get_clientdata(client);
	if (di)
		sc8571_enable_adc(0, (void *)di);

	return 0;
}

static int sc8571_i2c_resume(struct device *dev)
{
	return 0;
}

static void sc8571_i2c_complete(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct sc8571_device_info *di = NULL;

	if (!client)
		return;

	di = i2c_get_clientdata(client);
	if (!di)
		return;

	sc8571_enable_adc(1, (void *)di);
}

static const struct dev_pm_ops sc8571_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(sc8571_i2c_suspend, sc8571_i2c_resume)
	.complete = sc8571_i2c_complete,
};
#define SC8571_PM_OPS (&sc8571_pm_ops)
#else
#define SC8571_PM_OPS (NULL)
#endif /* CONFIG_PM */

MODULE_DEVICE_TABLE(i2c, sc8571);
static const struct of_device_id sc8571_of_match[] = {
	{
		.compatible = "sc8571",
		.data = NULL,
	},
	{},
};

static const struct i2c_device_id sc8571_i2c_id[] = {
	{ "sc8571", 0 }, {}
};

static struct i2c_driver sc8571_driver = {
	.probe = sc8571_probe,
	.remove = sc8571_remove,
	.shutdown = sc8571_shutdown,
	.id_table = sc8571_i2c_id,
	.driver = {
		.owner = THIS_MODULE,
		.name = "sc8571",
		.of_match_table = of_match_ptr(sc8571_of_match),
		.pm = SC8571_PM_OPS,
	},
};

static int __init sc8571_init(void)
{
	return i2c_add_driver(&sc8571_driver);
}

static void __exit sc8571_exit(void)
{
	i2c_del_driver(&sc8571_driver);
}

module_init(sc8571_init);
module_exit(sc8571_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("sc8571 module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
