// SPDX-License-Identifier: GPL-2.0
/*
 * nu2205.c
 *
 * nu2205 direct charge driver
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

#include "nu2205.h"
#include "nu2205_i2c.h"
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/common_module/power_event_ne.h>
#include <chipset_common/hwpower/common_module/power_pinctrl.h>
#include <chipset_common/hwpower/common_module/power_gpio.h>
#include <chipset_common/hwpower/common_module/power_delay.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_device_id.h>
#include <chipset_common/hwpower/common_module/power_common_macro.h>
#include <huawei_platform/power/battery_voltage.h>

#define HWLOG_TAG nu2205_chg
HWLOG_REGIST();

static int nu2205_discharge(int enable, void *dev_data)
{
	return 0;
}

static int nu2205_is_device_close(void *dev_data)
{
	u8 val = 0;
	struct nu2205_device_info *di = dev_data;

	if (!di)
		return -EPERM;

	if (nu2205_read_byte(di, NU2205_CHG_CTL_REG, &val))
		return -EIO;

	if (val & NU2205_CHARGE_EN_MASK)
		return 0;

	return 1; /* 1:ic is closed */
}

static int nu2205_get_device_id(void *dev_data)
{
	u8 dev_id = 0;
	int ret;
	struct nu2205_device_info *di = dev_data;

	if (!di)
		return -EPERM;

	if (di->first_rd)
		return di->device_id;

	ret = nu2205_read_byte(di, NU2205_DEVICE_ID_REG, &dev_id);
	if (ret) {
		hwlog_err("get_device_id read fail\n");
		return -EPERM;
	}
	di->first_rd = true;
	hwlog_info("get_device_id [0x%x]=0x%x\n", NU2205_DEVICE_ID_REG, dev_id);

	if (dev_id == NU2205_DEVICE_ID_VALUE)
		di->device_id = SWITCHCAP_NU2205;
	else
		di->device_id = DC_DEVICE_ID_END;

	hwlog_info("device_id=0x%x\n", di->device_id);

	return di->device_id;
}

static int nu2205_get_sc_max_ibat(void *dev_data, int *ibat)
{
	struct nu2205_device_info *di = dev_data;

	if (!di || !ibat)
		return -EINVAL;

	if (di->nu2205_sc_para.max_ibat <= 0) {
		hwlog_err("sc_max_ibat read fail\n");
		return -EPERM;
	}

	*ibat = di->nu2205_sc_para.max_ibat;
	hwlog_info("sc_max_ibat=%d\n", di->nu2205_sc_para.max_ibat);

	return 0;
}

static int nu2205_get_lvc_max_ibat(void *dev_data, int *ibat)
{
	struct nu2205_device_info *di = dev_data;

	if (!di || !ibat)
		return -EINVAL;

	if (di->nu2205_lvc_para.max_ibat <= 0) {
		hwlog_err("lvc_max_ibat read fail\n");
		return -EPERM;
	}

	*ibat = di->nu2205_lvc_para.max_ibat;
	hwlog_info("lvc_max_ibat=%d\n", di->nu2205_lvc_para.max_ibat);

	return 0;
}

static int nu2205_get_max_vbat_mv(void *dev_data)
{
	struct nu2205_device_info *di = dev_data;

	if (!di)
		return -ENODEV;

	return hw_battery_voltage(BAT_ID_MAX);
}

static int nu2205_get_vbat_mv(void *dev_data)
{
	s16 data = 0;
	int vbat;
	struct nu2205_device_info *di = dev_data;

	if (!di)
		return -ENODEV;

	if (nu2205_read_word(di, NU2205_VBAT_ADC1_REG, &data))
		return -EIO;

	vbat = (int)data;

	hwlog_info("VBAT_ADC=0x%x, vbat=%d\n", data, vbat);
	return vbat;
}

static int nu2205_get_ibat_ma(int *ibat, void *dev_data)
{
	s16 data = 0;
	struct nu2205_device_info *di = dev_data;

	if (!di || !ibat)
		return -EINVAL;

	if (nu2205_read_word(di, NU2205_IBAT_ADC1_REG, &data))
		return -EIO;

	*ibat = (int)data * di->sense_r_config;
	*ibat /= di->sense_r_actual;

	hwlog_info("IBAT_ADC=0x%x, ibat=%d\n", data, *ibat);
	return 0;
}

static int nu2205_get_ibus_ma(int *ibus, void *dev_data)
{
	s16 data = 0;
	struct nu2205_device_info *di = dev_data;

	if (!di || !ibus)
		return -EINVAL;

	if (nu2205_read_word(di, NU2205_IBUS_ADC1_REG, &data))
		return -EIO;

	*ibus = (int)data;

	hwlog_info("IBUS_ADC=0x%x, ibus=%d\n", data, *ibus);
	return 0;
}

static int nu2205_get_vbus_mv(int *vbus, void *dev_data)
{
	s16 data = 0;
	struct nu2205_device_info *di = dev_data;

	if (!di || !vbus)
		return -EINVAL;

	if (nu2205_read_word(di, NU2205_VBUS_ADC1_REG, &data))
		return -EIO;

	*vbus = (int)data;

	hwlog_info("VBUS_ADC=0x%x, vbus=%d\n", data, *vbus);
	return 0;
}

static int nu2205_get_vusb_mv(int *vusb, void *dev_data)
{
	s16 data = 0;
	struct nu2205_device_info *di = dev_data;

	if (!di || !vusb)
		return -EINVAL;

	if (nu2205_read_word(di, NU2205_VAC1_ADC1_REG, &data))
		return -EIO;

	*vusb = (int)data;

	hwlog_info("VUSB_ADC=0x%x, vusb=%d\n", data, *vusb);
	return 0;
}

static int nu2205_get_device_temp(int *temp, void *dev_data)
{
	s16 data = 0;
	struct nu2205_device_info *di = dev_data;

	if (!di || !temp)
		return -EINVAL;

	if (nu2205_read_word(di, NU2205_TDIE_ADC1_REG, &data))
		return -EIO;

	/* TDIE_ADC STEP: 0.5C */
	*temp = (int)data * 5 / 10;

	hwlog_info("TDIE_ADC=0x%x, temp=%d\n", data, *temp);
	return 0;
}

static int nu2205_config_watchdog_ms(int time, void *dev_data)
{
	int ret;
	u8 val;
	u8 reg = 0;
	struct nu2205_device_info *di = dev_data;

	if (!di)
		return -ENODEV;

	if (time >= NU2205_WTD_CONFIG_TIMING_30000MS)
		val = NU2205_WTD_SET_30000MS;
	else if (time >= NU2205_WTD_CONFIG_TIMING_5000MS)
		val = NU2205_WTD_SET_5000MS;
	else if (time >= NU2205_WTD_CONFIG_TIMING_1000MS)
		val = NU2205_WTD_SET_1000MS;
	else if (time >= NU2205_WTD_CONFIG_TIMING_500MS)
		val = NU2205_WTD_SET_500MS;
	else
		val = NU2205_WTD_SET_30000MS;

	ret = nu2205_write_mask(di, NU2205_CONTROL_REG, NU2205_WATCHDOG_CONFIG_MASK,
		NU2205_WATCHDOG_CONFIG_SHIFT, val);
	ret += nu2205_read_byte(di, NU2205_CONTROL_REG, &reg);
	if (ret)
		return -EIO;

	hwlog_info("config_watchdog_ms [0x%x]=0x%x\n", NU2205_CONTROL_REG, reg);
	return 0;
}

static int nu2205_kick_watchdog_ms(void *dev_data)
{
	return 0;
}

static int nu2205_config_vbat_ovp_th_mv(struct nu2205_device_info *di, int ovp_th)
{
	u8 value;
	int ret;

	if (ovp_th < NU2205_VBAT_OVP_BASE)
		ovp_th = NU2205_VBAT_OVP_BASE;

	if (ovp_th > NU2205_VBAT_OVP_MAX)
		ovp_th = NU2205_VBAT_OVP_MAX;

	value = (u8)((ovp_th - NU2205_VBAT_OVP_BASE) / NU2205_VBAT_OVP_STEP);
	ret = nu2205_write_mask(di, NU2205_VBAT_OVP_REG, NU2205_VBAT_OVP_MASK,
		NU2205_VBAT_OVP_SHIFT, value);
	ret += nu2205_read_byte(di, NU2205_VBAT_OVP_REG, &value);
	if (ret)
		return -EIO;

	hwlog_info("config_vbat_ovp_threshold_mv [0x%x]=0x%x\n",
		NU2205_VBAT_OVP_REG, value);
	return 0;
}

static int nu2205_config_ibat_ocp_th_ma(struct nu2205_device_info *di, int ocp_th)
{
	u8 value;
	int ret;

	if (ocp_th < NU2205_IBAT_OCP_BASE)
		ocp_th = NU2205_IBAT_OCP_BASE;

	if (ocp_th > NU2205_IBAT_OCP_MAX)
		ocp_th = NU2205_IBAT_OCP_MAX;

	value = (u8)((ocp_th - NU2205_IBAT_OCP_BASE) / NU2205_IBAT_OCP_STEP);
	ret = nu2205_write_mask(di, NU2205_IBAT_OCP_REG, NU2205_IBAT_OCP_MASK,
		NU2205_IBAT_OCP_SHIFT, value);
	ret += nu2205_read_byte(di, NU2205_IBAT_OCP_REG, &value);
	if (ret)
		return -EIO;

	hwlog_info("config_ibat_ocp_threshold_ma [0x%x]=0x%x\n",
		NU2205_IBAT_OCP_REG, value);
	return 0;
}

static int nu2205_config_vac_ovp_th_mv(struct nu2205_device_info *di, int ovp_th, int mode)
{
	u8 value;
	int ret;

	switch (mode) {
	case NU2205_CHARGE_FBYPASS_MODE:
		if (ovp_th < NU2205_AC_OVP_FBYPASS_BASE)
			ovp_th = NU2205_AC_OVP_FBYPASS_BASE;

		if (ovp_th > NU2205_AC_OVP_FBYPASS_MAX)
			ovp_th = NU2205_AC_OVP_FBYPASS_MAX;

		value = (u8)((ovp_th - NU2205_AC_OVP_FBYPASS_BASE) / NU2205_AC_OVP_FBYPASS_STEP);
		break;
	case NU2205_CHARGE_F42SC_MODE:
		if (ovp_th < NU2205_AC_OVP_F42SC_BASE)
			ovp_th = NU2205_AC_OVP_F42SC_BASE;

		if (ovp_th > NU2205_AC_OVP_F42SC_MAX)
			ovp_th = NU2205_AC_OVP_F42SC_MAX;

		value = (u8)((ovp_th - NU2205_AC_OVP_F42SC_BASE) / NU2205_AC_OVP_F42SC_STEP);
		break;
	default:
		value = NU2205_AC_OVP_DFT;
		break;
	}

	ret = nu2205_write_mask(di, NU2205_AC1_OVP_REG, NU2205_AC1_OVP_MASK,
		NU2205_AC1_OVP_SHIFT, value);
	ret += nu2205_read_byte(di, NU2205_AC1_OVP_REG, &value);
	if (ret)
		return -EIO;

	hwlog_info("config_ac1_ovp_threshold_mv [0x%x]=0x%x\n",
		NU2205_AC1_OVP_REG, value);

	ret += nu2205_write_mask(di, NU2205_AC2_OVP_REG, NU2205_AC2_OVP_MASK,
		NU2205_AC2_OVP_SHIFT, value);

	ret += nu2205_read_byte(di, NU2205_AC2_OVP_REG, &value);
	if (ret)
		return -EIO;

	hwlog_info("config_ac2_ovp_threshold_mv [0x%x]=0x%x\n",
		NU2205_AC2_OVP_REG, value);
	return 0;
}

static int nu2205_config_vbus_ovp_th_mv(struct nu2205_device_info *di, int ovp_th)
{
	u8 value;
	int ret;

	if (ovp_th < NU2205_VBUS_OVP_F42SC_BASE)
		ovp_th = NU2205_VBUS_OVP_F42SC_BASE;

	if (ovp_th > NU2205_VBUS_OVP_F42SC_MAX)
		ovp_th = NU2205_VBUS_OVP_F42SC_MAX;

	value = (u8)((ovp_th - NU2205_VBUS_OVP_F42SC_BASE) / NU2205_VBUS_OVP_F42SC_STEP);
	ret = nu2205_write_mask(di, NU2205_VBUS_OVP_REG, NU2205_VBUS_OVP_MASK,
		NU2205_VBUS_OVP_SHIFT, value);
	ret += nu2205_read_byte(di, NU2205_VBUS_OVP_REG, &value);
	if (ret)
		return -EIO;

	hwlog_info("config_vbus_ovp_threshole_mv [0x%x]=0x%x\n",
		NU2205_VBUS_OVP_REG, value);
	return 0;
}

static int nu2205_config_ibus_ocp_th_ma(struct nu2205_device_info *di, int ocp_th)
{
	u8 value;
	int ret;

	if (ocp_th < NU2205_IBUS_OCP_BASE)
		ocp_th = NU2205_IBUS_OCP_BASE;

	if (ocp_th > NU2205_IBUS_OCP_MAX)
		ocp_th = NU2205_IBUS_OCP_MAX;

	value = (u8)((ocp_th - NU2205_IBUS_OCP_BASE) / NU2205_IBUS_OCP_STEP);
	ret = nu2205_write_mask(di, NU2205_IBUS_OCP_UCP_REG,
		NU2205_IBUS_OCP_MASK, NU2205_IBUS_OCP_SHIFT, value);
	ret += nu2205_read_byte(di, NU2205_IBUS_OCP_UCP_REG, &value);
	if (ret)
		return -EIO;

	hwlog_info("config_ibus_ocp_threshold_ma [0x%x]=0x%x\n",
		NU2205_IBUS_OCP_UCP_REG, value);
	return 0;
}

static int nu2205_config_switching_frequency(int data, struct nu2205_device_info *di)
{
	int freq, freq_shift, ret;

	switch (data) {
	case NU2205_SW_FREQ_400KHZ:
		freq = NU2205_FSW_SET_SW_FREQ_400KHZ;
		freq_shift = NU2205_SW_FREQ_SHIFT_NORMAL;
		break;
	case NU2205_SW_FREQ_500KHZ:
		freq = NU2205_FSW_SET_SW_FREQ_500KHZ;
		freq_shift = NU2205_SW_FREQ_SHIFT_NORMAL;
		break;
	case NU2205_SW_FREQ_600KHZ:
		freq = NU2205_FSW_SET_SW_FREQ_600KHZ;
		freq_shift = NU2205_SW_FREQ_SHIFT_NORMAL;
		break;
	case NU2205_SW_FREQ_700KHZ:
		freq = NU2205_FSW_SET_SW_FREQ_700KHZ;
		freq_shift = NU2205_SW_FREQ_SHIFT_NORMAL;
		break;
	case NU2205_SW_FREQ_800KHZ:
		freq = NU2205_FSW_SET_SW_FREQ_800KHZ;
		freq_shift = NU2205_SW_FREQ_SHIFT_NORMAL;
		break;
	case NU2205_SW_FREQ_900KHZ:
		freq = NU2205_FSW_SET_SW_FREQ_900KHZ;
		freq_shift = NU2205_SW_FREQ_SHIFT_NORMAL;
		break;
	default:
		freq = NU2205_FSW_SET_SW_FREQ_500KHZ;
		freq_shift = NU2205_SW_FREQ_SHIFT_NORMAL;
		break;
	}

	ret = nu2205_write_mask(di, NU2205_CONTROL_REG, NU2205_FSW_SET_MASK,
		NU2205_FSW_SET_SHIFT, freq);
	ret += nu2205_write_mask(di, NU2205_CHG_CTL_REG, NU2205_FREQ_SHIFT_MASK,
		NU2205_FREQ_SHIFT_SHIFT, freq_shift);
	if (ret)
		return -EIO;

	hwlog_info("config_switching_frequency [0x%x]=0x%x\n",
		NU2205_CONTROL_REG, freq);
	hwlog_info("config_adjustable_switching_frequency [0x%x]=0x%x\n",
		NU2205_CHG_CTL_REG, freq_shift);

	return 0;
}

static int nu2205_congfig_ibat_sns_res(struct nu2205_device_info *di)
{
	int ret;
	u8 value;

	if (di->sense_r_config == SENSE_R_2_MOHM)
		value = NU2205_IBAT_SNS_RES_2MOHM;
	else
		value = NU2205_IBAT_SNS_RES_1MOHM;

	ret = nu2205_write_mask(di, NU2205_IBUS_UCP_RCP_THRESHOLD_REG,
		NU2205_IBAT_SNS_RES_MASK, NU2205_IBAT_SNS_RES_SHIFT, value);
	if (ret)
		return -EIO;

	hwlog_info("congfig_ibat_sns_res=%d\n", di->sense_r_config);
	return 0;
}

static int nu2205_threshold_reg_init(struct nu2205_device_info *di, u8 mode)
{
	int ret, ibus_ocp, ibat_ocp, vbat_ovp, fsw;

	if (mode == NU2205_CHARGE_FBYPASS_MODE) {
		ibus_ocp = di->nu2205_lvc_para.ibus_ocp;
		ibat_ocp = di->nu2205_lvc_para.ibat_ocp;
		vbat_ovp = di->nu2205_lvc_para.vbat_ovp;
		fsw = di->nu2205_lvc_para.switching_frequency;
	} else if (mode == NU2205_CHARGE_F42SC_MODE) {
		ibus_ocp = di->nu2205_sc_para.ibus_ocp;
		ibat_ocp = di->nu2205_sc_para.ibat_ocp;
		vbat_ovp = di->nu2205_sc_para.vbat_ovp;
		fsw = di->nu2205_sc_para.switching_frequency;
	} else {
		ibus_ocp = NU2205_SC_IBUS_OCP_TH_INIT;
		ibat_ocp = NU2205_IBAT_OCP_TH_INIT;
		vbat_ovp = NU2205_VBAT_OVP_TH_INIT;
		fsw = NU2205_SW_FREQ_500KHZ;
	}
	ret = nu2205_config_vac_ovp_th_mv(di, NU2205_VAC_OVP_TH_INIT, mode);
	ret += nu2205_config_vbus_ovp_th_mv(di, NU2205_VBUS_OVP_TH_INIT);
	ret += nu2205_config_ibus_ocp_th_ma(di, ibus_ocp);
	ret += nu2205_config_vbat_ovp_th_mv(di, vbat_ovp);
	ret += nu2205_config_ibat_ocp_th_ma(di, ibat_ocp);
	if (ret) {
		hwlog_err("protect threshold init fail\n");
		return ret;
	}

	ret = nu2205_config_switching_frequency(fsw, di);
	if (ret)
		hwlog_err("config switching frequency fail\n");

	return ret;
}

static int nu2205_lvc_charge_enable(int enable, void *dev_data)
{
	int ret = 0;
	u8 ctrl_reg = 0;
	u8 mode = NU2205_CHARGE_FBYPASS_MODE;
	u8 chg_en = enable ? NU2205_CHG_ENABLE : NU2205_CHG_DISABLE;
	struct nu2205_device_info *di = dev_data;

	if (!di)
		return -ENODEV;
	if (chg_en)
		ret = nu2205_threshold_reg_init(di, mode);

	ret += nu2205_write_mask(di, NU2205_CHG_CTL_REG, NU2205_CHARGE_EN_MASK,
		NU2205_CHARGE_EN_SHIFT, NU2205_CHG_DISABLE);
	ret += nu2205_write_mask(di, NU2205_CHG_CTL_REG, NU2205_CHARGE_MODE_MASK,
		NU2205_CHARGE_MODE_SHIFT, mode);
	ret += nu2205_write_mask(di, NU2205_CHG_CTL_REG, NU2205_CHARGE_EN_MASK,
		NU2205_CHARGE_EN_SHIFT, chg_en);
	ret += nu2205_read_byte(di, NU2205_CHG_CTL_REG, &ctrl_reg);
	if (ret)
		return -EIO;

	hwlog_info("ic_role=%d, lvc_charge_enable [0x%x]=0x%x\n",
		di->ic_role, NU2205_CHG_CTL_REG, ctrl_reg);

	return 0;
}

static int nu2205_sc_charge_enable(int enable, void *dev_data)
{
	int ret = 0;
	u8 ctrl_reg = 0;
	u8 mode = NU2205_CHARGE_F42SC_MODE;
	u8 chg_en = enable ? NU2205_CHG_ENABLE : NU2205_CHG_DISABLE;
	struct nu2205_device_info *di = dev_data;

	if (!di)
		return -ENODEV;
	if (chg_en)
		ret = nu2205_threshold_reg_init(di, mode);

	ret += nu2205_write_mask(di, NU2205_CHG_CTL_REG, NU2205_CHARGE_EN_MASK,
		NU2205_CHARGE_EN_SHIFT, NU2205_CHG_DISABLE);
	ret += nu2205_write_mask(di, NU2205_CHG_CTL_REG, NU2205_CHARGE_MODE_MASK,
		NU2205_CHARGE_MODE_SHIFT, mode);
	ret += nu2205_write_mask(di, NU2205_CHG_CTL_REG, NU2205_CHARGE_EN_MASK,
		NU2205_CHARGE_EN_SHIFT, chg_en);
	ret += nu2205_read_byte(di, NU2205_CHG_CTL_REG, &ctrl_reg);
	if (ret)
		return -EIO;

	hwlog_info("ic_role=%d, sc_charge_enable [0x%x]=0x%x\n",
		di->ic_role, NU2205_CHG_CTL_REG, ctrl_reg);

	return 0;
}

static int nu2205_reg_reset(struct nu2205_device_info *di)
{
	int ret;
	u8 ctrl_reg = 0;

	ret = nu2205_write_mask(di, NU2205_CONTROL_REG,
		NU2205_REG_RST_MASK, NU2205_REG_RST_SHIFT, NU2205_REG_RST_ENABLE);
	if (ret)
		return -EIO;

	power_usleep(DT_USLEEP_1MS);
	ret = nu2205_read_byte(di, NU2205_CONTROL_REG, &ctrl_reg);
	if (ret)
		return -EIO;

	hwlog_info("reg_reset [0x%x]=0x%x\n", NU2205_CONTROL_REG, ctrl_reg);
	return 0;
}

static int nu2205_chip_init(void *dev_data)
{
	return 0;
}

static int nu2205_disable_alm_interrupt(void *dev_data)
{
	int ret;
	struct nu2205_device_info *di = dev_data;

	ret = nu2205_write_mask(di, NU2205_INT_MASK_REG, NU2205_BAT_OVP_ALM_FLAG_MASK,
		NU2205_BAT_OVP_ALM_MASK_SHIFT, NU2205_ALM_DISABLE);
	ret += nu2205_write_mask(di, NU2205_INT_MASK_REG, NU2205_BAT_OCP_ALM_FLAG_MASK,
		NU2205_BAT_OCP_ALM_MASK_SHIFT, NU2205_ALM_DISABLE);
	ret += nu2205_write_mask(di, NU2205_INT_MASK_REG, NU2205_BUS_OVP_ALM_MASK_MASK,
		NU2205_BUS_OVP_ALM_MASK_SHIFT, NU2205_ALM_DISABLE);
	ret += nu2205_write_mask(di, NU2205_INT_MASK_REG, NU2205_BUS_OCP_ALM_MASK_MASK,
		NU2205_BUS_OCP_ALM_MASK_SHIFT, NU2205_ALM_DISABLE);
	ret += nu2205_write_mask(di, NU2205_FLT_MASK_REG, NU2205_TDIE_ALM_MASK_MASK,
		NU2205_TDIE_ALM_MASK_SHIFT, NU2205_ALM_DISABLE);
	ret += nu2205_write_mask(di, NU2205_FLT_MASK_REG, NU2205_TS_ALM_MASK_MASK,
		NU2205_TS_ALM_MASK_SHIFT, NU2205_ALM_DISABLE);
	if (ret)
		hwlog_err("disable_alm_interrupt fail\n");

	return ret;
}

static int nu2205_reg_init(struct nu2205_device_info *di)
{
	int ret;

	ret = nu2205_write_byte(di, NU2205_CONTROL_REG, NU2205_CONTROL_REG_INIT);
	ret += nu2205_threshold_reg_init(di, NU2205_CHARGE_F42SC_MODE);
	ret += nu2205_congfig_ibat_sns_res(di);
	ret += nu2205_disable_alm_interrupt(di);
	ret += nu2205_write_mask(di, NU2205_ADC_CTRL_REG,
		NU2205_ADC_CTRL_EN_MASK, NU2205_ADC_CTRL_EN_SHIFT, NU2205_ADC_ENABLE);
	/* set IBUS_UCP_DEGLITCH 5ms */
	ret += nu2205_write_mask(di, NU2205_DEGLITCH_REG, NU2205_IBUS_UCP_FALL_DG_SET_MASK,
		NU2205_IBUS_UCP_FALL_DG_SET_SHIFT, NU2205_IBUS_UCP_FALL_DG_SET_5MS);
	ret += nu2205_write_mask(di, NU2205_FLT_MASK_REG, NU2205_TS_FLT_MASK_MASK,
		NU2205_TS_FLT_MASK_SHIFT, NU2205_TS_FLT_DISABLE);
	/* close TS protection */
	ret += nu2205_write_mask(di, NU2205_CHG_CTL_REG, NU2205_TSBUS_DIS_MASK,
		NU2205_TSBUS_DIS_SHIFT, NU2205_TS_PROTECTION_DISENABLE);
	ret += nu2205_write_mask(di, NU2205_CHG_CTL_REG, NU2205_TSBAT_DIS_MASK,
		NU2205_TSBAT_DIS_SHIFT, NU2205_TS_PROTECTION_DISENABLE);
	if (ret)
		hwlog_err("reg_init fail\n");

	return ret;
}

static int nu2205_enable_adc(int enable, void *dev_data)
{
	int ret;
	u8 reg = 0;
	u8 value = enable ? NU2205_ADC_ENABLE : NU2205_ADC_DISABLE;
	struct nu2205_device_info *di = dev_data;

	if (!di)
		return -ENODEV;

	ret = nu2205_write_mask(di, NU2205_ADC_CTRL_REG, NU2205_ADC_CTRL_EN_MASK,
		NU2205_ADC_CTRL_EN_SHIFT, value);
	if (ret)
		return -EPERM;

	ret = nu2205_read_byte(di, NU2205_ADC_CTRL_REG, &reg);
	if (ret)
		return -EPERM;

	hwlog_info("adc_enable [0x%x]=0x%x\n", NU2205_ADC_CTRL_REG, reg);
	return 0;
}

static int nu2205_charge_init(void *dev_data)
{
	struct nu2205_device_info *di = dev_data;

	if (!di)
		return -ENODEV;

	if (nu2205_reg_init(di))
		return -EINVAL;

	di->init_finished = true;
	return 0;
}

static int nu2205_reg_reset_and_init(void *dev_data)
{
	int ret;
	struct nu2205_device_info *di = dev_data;

	if (!di)
		return -ENODEV;

	hwlog_info("reg reset and init enter\n");
	ret = nu2205_reg_reset(di);
	ret += nu2205_reg_init(di);

	return ret;
}

static int nu2205_charge_exit(void *dev_data)
{
	int ret;
	struct nu2205_device_info *di = dev_data;

	if (!di)
		return -ENODEV;

	ret = nu2205_sc_charge_enable(NU2205_SWITCHCAP_DISABLE, di);
	di->init_finished = false;
	di->int_notify_enabled = false;

	return ret;
}

static int nu2205_batinfo_exit(void *dev_data)
{
	return 0;
}

static int nu2205_batinfo_init(void *dev_data)
{
	struct nu2205_device_info *di = dev_data;

	if (!di)
		return -ENODEV;

	return nu2205_chip_init(di);
}

/* print the register head in charging process */
static int nu2205_register_head(char *buffer, int size, void *dev_data)
{
	struct nu2205_device_info *di = dev_data;

	if (!di || !buffer)
		return -EINVAL;

	snprintf(buffer, size,
		"dev       mode   Ibus   Vbus   Vusb   Ibat   Vbat   Temp   ");

	return 0;
}

/* print the register value in charging process */
static int nu2205_value_dump(char *buffer, int size, void *dev_data)
{
	struct nu2205_dump_data data = { 0 };
	char buff[NU2205_BUF_LEN] = { 0 };
	u8 val = 0;
	struct nu2205_device_info *di = dev_data;

	if (!di || !buffer)
		return -EINVAL;

	(void)nu2205_get_vbus_mv(&data.vbus, di);
	(void)nu2205_get_ibat_ma(&data.ibat, di);
	(void)nu2205_get_ibus_ma(&data.ibus, di);
	(void)nu2205_get_device_temp(&data.temp, di);
	(void)nu2205_get_vusb_mv(&data.vusb, di);
	(void)nu2205_read_byte(di, NU2205_CHG_CTL_REG, &val);

	snprintf(buff, sizeof(buff), "%-10s", di->name);
	strncat(buffer, buff, strlen(buff));

	if (nu2205_is_device_close(di))
		snprintf(buff, sizeof(buff), "%s", "OFF    ");
	else if (((val & NU2205_CHARGE_MODE_MASK) >> NU2205_CHARGE_MODE_SHIFT) ==
		NU2205_CHARGE_FBYPASS_MODE)
		snprintf(buff, sizeof(buff), "%s", "LVC    ");
	else if (((val & NU2205_CHARGE_MODE_MASK) >> NU2205_CHARGE_MODE_SHIFT) ==
		NU2205_CHARGE_F42SC_MODE)
		snprintf(buff, sizeof(buff), "%s", "SC     ");

	strncat(buffer, buff, strlen(buff));
	snprintf(buff, sizeof(buff), "%-7d%-7d%-7d%-7d%-7d%-7d",
		data.ibus, data.vbus, data.vusb, data.ibat, nu2205_get_vbat_mv(di), data.temp);
	strncat(buffer, buff, strlen(buff));

	return 0;
}

static struct dc_ic_ops g_nu2205_lvc_ops = {
	.dev_name = "nu2205",
	.ic_init = nu2205_charge_init,
	.ic_exit = nu2205_charge_exit,
	.ic_enable = nu2205_lvc_charge_enable,
	.ic_discharge = nu2205_discharge,
	.is_ic_close = nu2205_is_device_close,
	.get_ic_id = nu2205_get_device_id,
	.config_ic_watchdog = nu2205_config_watchdog_ms,
	.kick_ic_watchdog = nu2205_kick_watchdog_ms,
	.ic_reg_reset_and_init = nu2205_reg_reset_and_init,
	.get_max_ibat = nu2205_get_lvc_max_ibat,
};

static struct dc_ic_ops g_nu2205_sc_ops = {
	.dev_name = "nu2205",
	.ic_init = nu2205_charge_init,
	.ic_exit = nu2205_charge_exit,
	.ic_enable = nu2205_sc_charge_enable,
	.ic_discharge = nu2205_discharge,
	.is_ic_close = nu2205_is_device_close,
	.get_ic_id = nu2205_get_device_id,
	.config_ic_watchdog = nu2205_config_watchdog_ms,
	.kick_ic_watchdog = nu2205_kick_watchdog_ms,
	.ic_reg_reset_and_init = nu2205_reg_reset_and_init,
	.get_max_ibat = nu2205_get_sc_max_ibat,
};

static struct dc_batinfo_ops g_nu2205_batinfo_ops = {
	.init = nu2205_batinfo_init,
	.exit = nu2205_batinfo_exit,
	.get_bat_btb_voltage = nu2205_get_max_vbat_mv,
	.get_bat_package_voltage = nu2205_get_max_vbat_mv,
	.get_vbus_voltage = nu2205_get_vbus_mv,
	.get_bat_current = nu2205_get_ibat_ma,
	.get_ic_ibus = nu2205_get_ibus_ma,
	.get_ic_temp = nu2205_get_device_temp,
};

static struct power_log_ops g_nu2205_log_ops = {
	.dev_name = "nu2205",
	.dump_log_head = nu2205_register_head,
	.dump_log_content = nu2205_value_dump,
};

static void nu2205_init_ops_dev_data(struct nu2205_device_info *di)
{
	memcpy(&di->lvc_ops, &g_nu2205_lvc_ops, sizeof(struct dc_ic_ops));
	di->lvc_ops.dev_data = (void *)di;
	memcpy(&di->sc_ops, &g_nu2205_sc_ops, sizeof(struct dc_ic_ops));
	di->sc_ops.dev_data = (void *)di;
	memcpy(&di->batinfo_ops, &g_nu2205_batinfo_ops, sizeof(struct dc_batinfo_ops));
	di->batinfo_ops.dev_data = (void *)di;
	memcpy(&di->log_ops, &g_nu2205_log_ops, sizeof(struct power_log_ops));
	di->log_ops.dev_data = (void *)di;

	if (!di->ic_role) {
		snprintf(di->name, CHIP_DEV_NAME_LEN, "nu2205");
	} else {
		snprintf(di->name, CHIP_DEV_NAME_LEN, "nu2205_%d", di->ic_role);
		di->lvc_ops.dev_name = di->name;
		di->sc_ops.dev_name = di->name;
		di->log_ops.dev_name = di->name;
	}
}

static void nu2205_ops_register(struct nu2205_device_info *di)
{
	int ret;

	nu2205_init_ops_dev_data(di);

	ret = dc_ic_ops_register(LVC_MODE, di->ic_role, &di->lvc_ops);
	ret += dc_ic_ops_register(SC_MODE, di->ic_role, &di->sc_ops);
	ret += dc_batinfo_ops_register(di->ic_role, &di->batinfo_ops, di->device_id);
	if (ret)
		hwlog_err("sysinfo ops register fail\n");

	power_log_ops_register(&di->log_ops);
}

static void nu2205_fault_event_notify(unsigned long event, void *data)
{
	power_event_anc_notify(POWER_ANT_DC_FAULT, event, data);
}

static void nu2205_interrupt_handle(struct nu2205_device_info *di,
	struct nty_data *data, u8 *flag)
{
	int val = 0;
	u8 mode = 0;
	if (flag[NU2205_IRQ_AC1_OVP] & NU2205_AC1_OVP_FLAG_MASK) {
		hwlog_info("AC1 OVP happened\n", val);
		nu2205_fault_event_notify(POWER_NE_DC_FAULT_AC_OVP, data);
	} else if (flag[NU2205_IRQ_AC2_OVP] & NU2205_AC2_OVP_FLAG_MASK) {
		hwlog_info("AC2 OVP happened\n", val);
		nu2205_fault_event_notify(POWER_NE_DC_FAULT_AC_OVP, data);
	} else if (flag[NU2205_IRQ_FLT_FLAG] & NU2205_BAT_OVP_FLT_FLAG_MASK) {
		val = nu2205_get_vbat_mv(di);
		hwlog_info("BAT OVP happened, vbat=%d\n", val);
		if (val >= NU2205_VBAT_OVP_TH_INIT)
			nu2205_fault_event_notify(POWER_NE_DC_FAULT_VBAT_OVP, data);
	} else if (flag[NU2205_IRQ_FLT_FLAG] & NU2205_BAT_OCP_FLT_FLAG_MASK) {
		nu2205_get_ibat_ma(&val, di);
		hwlog_info("BAT OCP happened,ibat=%d\n", val);
		if (val >= NU2205_IBAT_OCP_TH_INIT)
			nu2205_fault_event_notify(POWER_NE_DC_FAULT_IBAT_OCP, data);
	} else if (flag[NU2205_IRQ_FLT_FLAG] & NU2205_BUS_OVP_FLT_FLAG_SHIFT) {
		nu2205_get_vbus_mv(&val, di);
		hwlog_info("BUS OVP happened,vbus=%d\n", val);
		if (val >= NU2205_VBUS_OVP_TH_INIT)
			nu2205_fault_event_notify(POWER_NE_DC_FAULT_VBUS_OVP, data);
	} else if (flag[NU2205_IRQ_FLT_FLAG] & NU2205_BUS_OCP_FLT_FLAG_SHIFT) {
		nu2205_get_ibus_ma(&val, di);
		hwlog_info("BUS OCP happened,ibus=%d\n", val);
		(void)nu2205_read_byte(di, NU2205_CHG_CTL_REG, &mode);
		mode = (mode & NU2205_CHARGE_MODE_MASK) >> NU2205_CHARGE_MODE_SHIFT;
		if (((val >= NU2205_LVC_IBUS_OCP_TH_INIT) && (mode == NU2205_CHARGE_FBYPASS_MODE)) ||
			((val >= NU2205_SC_IBUS_OCP_TH_INIT) && (mode == NU2205_CHARGE_F42SC_MODE)))
			nu2205_fault_event_notify(POWER_NE_DC_FAULT_IBUS_OCP, data);
	} else if (flag[NU2205_IRQ_FLT_FLAG] & NU2205_BUS_RCP_FLT_FLAG_MASK) {
		hwlog_info("BUS RCP happened\n");
		nu2205_fault_event_notify(POWER_NE_DC_FAULT_REVERSE_OCP, data);
	} else if (flag[NU2205_IRQ_VOUT_OVP] & NU2205_VOUT_OVP_FLAG_MASK) {
		hwlog_info("VOUT OVP happened\n");
	}
}

static void nu2205_interrupt_work(struct work_struct *work)
{
	u8 flag[NU2205_IRQ_END] = { 0 };
	struct nu2205_device_info *di = NULL;
	struct nty_data *data = NULL;

	if (!work)
		return;

	di = container_of(work, struct nu2205_device_info, irq_work);
	if (!di || !di->client) {
		hwlog_err("di is null\n");
		return;
	}

	(void)nu2205_read_byte(di, NU2205_AC1_OVP_REG, &flag[NU2205_IRQ_AC1_OVP]);
	(void)nu2205_read_byte(di, NU2205_AC2_OVP_REG, &flag[NU2205_IRQ_AC2_OVP]);
	(void)nu2205_read_byte(di, NU2205_FLT_FLAG_REG, &flag[NU2205_IRQ_FLT_FLAG]);
	(void)nu2205_read_byte(di, NU2205_IBUS_OCP_UCP_REG, &flag[NU2205_IRQ_IBUS_UCP_RISE_FLAG]);
	(void)nu2205_read_byte(di, NU2205_IBUS_OCP_ALM_REG, &flag[NU2205_IRQ_IBUS_UCP_FALL_FLAG]);
	(void)nu2205_read_byte(di, NU2205_CONVERTER_STATE_REG, &flag[NU2205_IRQ_CONVERTER_STATE]);
	(void)nu2205_read_byte(di, NU2205_VOUT_OVP_REG, &flag[NU2205_IRQ_VOUT_OVP]);
	(void)nu2205_read_byte(di, NU2205_VBUS2VOUT_UVP_OVP_REG, &flag[NU2205_IRQ_VBUS2VOUT_UVP_OVP]);
	(void)nu2205_read_byte(di, NU2205_CONTROL_REG, &flag[NU2205_IRQ_WD_TIMEOUT]);

	data = &(di->notify_data);
	data->addr = di->client->addr;

	if (di->int_notify_enabled)
		nu2205_interrupt_handle(di, data, flag);

	if (flag[NU2205_IRQ_AC1_OVP])
		hwlog_info("interrupt_reg[0x%x]=0x%x\n", NU2205_AC1_OVP_REG, flag[NU2205_IRQ_AC1_OVP]);
	if (flag[NU2205_IRQ_AC2_OVP])
		hwlog_info("interrupt_reg[0x%x]=0x%x\n", NU2205_AC2_OVP_REG, flag[NU2205_IRQ_AC2_OVP]);
	if (flag[NU2205_IRQ_FLT_FLAG])
		hwlog_info("interrupt_reg[0x%x]=0x%x\n", NU2205_FLT_FLAG_REG, flag[NU2205_IRQ_FLT_FLAG]);
	if (flag[NU2205_IRQ_IBUS_UCP_RISE_FLAG])
		hwlog_info("interrupt_reg[0x%x]=0x%x\n",
			NU2205_IBUS_OCP_UCP_REG, flag[NU2205_IRQ_IBUS_UCP_RISE_FLAG]);
	if (flag[NU2205_IRQ_IBUS_UCP_FALL_FLAG])
		hwlog_info("interrupt_reg[0x%x]=0x%x\n",
			NU2205_IBUS_OCP_ALM_REG, flag[NU2205_IRQ_IBUS_UCP_FALL_FLAG]);
	if (flag[NU2205_IRQ_CONVERTER_STATE])
		hwlog_info("interrupt_reg[0x%x]=0x%x\n",
			NU2205_CONVERTER_STATE_REG, flag[NU2205_IRQ_CONVERTER_STATE]);
	if (flag[NU2205_IRQ_VOUT_OVP])
		hwlog_info("interrupt_reg[0x%x]=0x%x\n",
			NU2205_VOUT_OVP_REG, flag[NU2205_IRQ_VOUT_OVP]);
	if (flag[NU2205_IRQ_VBUS2VOUT_UVP_OVP])
		hwlog_info("interrupt_reg[0x%x]=0x%x\n",
			NU2205_VBUS2VOUT_UVP_OVP_REG, flag[NU2205_IRQ_VBUS2VOUT_UVP_OVP]);
	if (flag[NU2205_IRQ_WD_TIMEOUT])
		hwlog_info("interrupt_reg[0x%x]=0x%x\n",
			NU2205_CONTROL_REG, flag[NU2205_IRQ_WD_TIMEOUT]);

	enable_irq(di->irq_int);
}

static irqreturn_t nu2205_interrupt(int irq, void *_di)
{
	struct nu2205_device_info *di = _di;

	if (!di)
		return IRQ_HANDLED;

	if (di->init_finished)
		di->int_notify_enabled = true;

	hwlog_info("int happened\n");

	disable_irq_nosync(di->irq_int);
	schedule_work(&di->irq_work);

	return IRQ_HANDLED;
}

static int nu2205_irq_init(struct nu2205_device_info *di,
	struct device_node *np)
{
	int ret;

	INIT_WORK(&di->irq_work, nu2205_interrupt_work);
	ret = power_gpio_config_interrupt(np,
		"intr_gpio", "nu2205_gpio_int", &di->gpio_int, &di->irq_int);
	if (ret)
		return ret;

	ret = request_irq(di->irq_int, nu2205_interrupt,
		IRQF_TRIGGER_FALLING, "nu2205_int_irq", di);
	if (ret) {
		hwlog_err("gpio irq request fail\n");
		di->irq_int = -1;
		gpio_free(di->gpio_int);
		return ret;
	}

	enable_irq_wake(di->irq_int);
	return 0;
}

static void nu2205_parse_dts(struct device_node *np,
	struct nu2205_device_info *di)
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

static void nu2205_mode_para_init(struct nu2205_device_info *di)
{
	di->nu2205_lvc_para.ibus_ocp = NU2205_LVC_IBUS_OCP_TH_INIT;
	di->nu2205_lvc_para.ibat_ocp = NU2205_IBAT_OCP_TH_INIT;
	di->nu2205_lvc_para.vbat_ovp = NU2205_VBAT_OVP_TH_INIT;
	di->nu2205_lvc_para.switching_frequency = NU2205_SW_FREQ_500KHZ;
	di->nu2205_sc_para.ibus_ocp = NU2205_SC_IBUS_OCP_TH_INIT;
	di->nu2205_sc_para.ibat_ocp = NU2205_IBAT_OCP_TH_INIT;
	di->nu2205_sc_para.vbat_ovp = NU2205_VBAT_OVP_TH_INIT;
	di->nu2205_sc_para.switching_frequency = NU2205_SW_FREQ_500KHZ;
}

static int nu2205_parse_mode_para_dts(struct device_node *np,
	struct nu2205_mode_para *data, const char *name, int id)
{
	int array_len, col, row, idata;
	int index = -1; /* -1 : illegal value */
	const char *device_name = dc_get_device_name_without_mode(id);
	const char *tmp_string = NULL;

	array_len = power_dts_read_count_strings(power_dts_tag(HWLOG_TAG), np,
		name, NU2205_COMP_MAX_NUM, NU2205_INFO_TOTAL);
	if (array_len < 0)
		return -EPERM;

	for (row = 0; row < array_len / NU2205_INFO_TOTAL; row++) {
		col = row * NU2205_INFO_TOTAL + NU2205_INFO_IC_NAME;
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
		col = index * NU2205_INFO_TOTAL + NU2205_INFO_MAX_IBAT;
		power_dts_read_string_index(power_dts_tag(HWLOG_TAG), np, name, col, &tmp_string);
		kstrtoint(tmp_string, POWER_BASE_DEC, &idata);
		data->max_ibat = idata;
		col = index * NU2205_INFO_TOTAL + NU2205_INFO_IBUS_OCP;
		power_dts_read_string_index(power_dts_tag(HWLOG_TAG), np, name, col, &tmp_string);
		kstrtoint(tmp_string, POWER_BASE_DEC, &idata);
		data->ibus_ocp = idata;
		col = index * NU2205_INFO_TOTAL + NU2205_INFO_IBAT_OCP;
		power_dts_read_string_index(power_dts_tag(HWLOG_TAG), np, name, col, &tmp_string);
		kstrtoint(tmp_string, POWER_BASE_DEC, &idata);
		data->ibat_ocp = idata;
		col = index * NU2205_INFO_TOTAL + NU2205_INFO_VBAT_OVP;
		power_dts_read_string_index(power_dts_tag(HWLOG_TAG), np, name, col, &tmp_string);
		kstrtoint(tmp_string, POWER_BASE_DEC, &idata);
		data->vbat_ovp = idata;
		col = index * NU2205_INFO_TOTAL + NU2205_INFO_SW_FREQ;
		power_dts_read_string_index(power_dts_tag(HWLOG_TAG), np, name, col, &tmp_string);
		kstrtoint(tmp_string, POWER_BASE_DEC, &idata);
		data->switching_frequency = idata;
	}

	return 0;
}

static int nu2205_otg_enable(struct nu2205_device_info *di, bool enable)
{
	int ret;

	u8 otg_en = enable ? NU2205_EN_OTG_ENABLE : NU2205_EN_OTG_DISABLE;
	u8 dis_acdrv_both = di->ovp_mos_switch_off ? NU2205_DIS_ACDRV_ENABLE : NU2205_DIS_ACDRV_DISABLE;
	u8 ovp_mos_on = di->ovp_mos_switch_off ? NU2205_ACDRV_EN_ACDRV1_DISABLE : NU2205_ACDRV_EN_ACDRV1_ENABLE;

	ret = nu2205_write_mask(di, NU2205_VAC_PRESENT_DET_REG,
		NU2205_EN_OTG_MASK, NU2205_EN_OTG_SHIFT, otg_en);
	if (enable) {
		ret += nu2205_write_mask(di, NU2205_VAC_PRESENT_DET_REG,
			NU2205_DIS_ACDRV_MASK, NU2205_DIS_ACDRV_SHIFT, dis_acdrv_both);
		ret += nu2205_write_mask(di, NU2205_ACDRV_CTRL_REG, NU2205_ACDRV_EN_ACDRV1_MASK,
			NU2205_ACDRV_EN_ACDRV1_SHIFT, ovp_mos_on);
	} else {
		ret += nu2205_write_mask(di, NU2205_VAC_PRESENT_DET_REG,
			NU2205_DIS_ACDRV_MASK, NU2205_DIS_ACDRV_SHIFT, NU2205_DIS_ACDRV_DISABLE);
		ret += nu2205_write_mask(di, NU2205_ACDRV_CTRL_REG, NU2205_ACDRV_EN_ACDRV1_MASK,
			NU2205_ACDRV_EN_ACDRV1_SHIFT, NU2205_ACDRV_EN_ACDRV1_DISABLE);
	}
	if (ret)
		hwlog_err("otg enable fail enable = %d\n", enable);

	return ret;
}

static int nu2205_otg_notifier_call(struct notifier_block *nb, unsigned long event, void *data)
{
	struct nu2205_device_info *di = container_of(nb, struct nu2205_device_info, otg_nb);

	if (!di)
		return NOTIFY_OK;

	switch (event) {
	case POWER_NE_OTG_INSERT:
		hwlog_info("otg insert event happen\n");
		nu2205_otg_enable(di, true);
		break;
	case POWER_NE_OTG_REMOVE:
		hwlog_info("otg remove event happen\n");
		nu2205_otg_enable(di, false);
		break;
	default:
		break;
	}
	return NOTIFY_OK;
}

static int nu2205_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int ret;
	struct nu2205_device_info *di = NULL;
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

	ret = nu2205_get_device_id(di);
	if ((ret < 0) || (ret == DC_DEVICE_ID_END))
		goto nu2205_fail_0;

	nu2205_parse_dts(np, di);
	nu2205_mode_para_init(di);
	nu2205_parse_mode_para_dts(np, &di->nu2205_lvc_para, "lvc_para", di->device_id);
	nu2205_parse_mode_para_dts(np, &di->nu2205_sc_para, "sc_para", di->device_id);

	(void)power_pinctrl_config(di->dev, "pinctrl-names", 1); /* 1:pinctrl-names length */

	ret = nu2205_reg_reset_and_init(di);
	if (ret)
		goto nu2205_fail_0;
	if (di->manual_otg_mode) {
		di->otg_nb.notifier_call = nu2205_otg_notifier_call;
		ret = power_event_bnc_register(POWER_BNT_OTG, &di->otg_nb);
		if (ret)
			goto nu2205_fail_0;
	}

	ret = nu2205_irq_init(di, np);
	if (ret)
		goto nu2205_fail_1;

	nu2205_ops_register(di);
	i2c_set_clientdata(client, di);
	hwlog_info("probe ok\n");
	return 0;

nu2205_fail_1:
	free_irq(di->irq_int, di);
	gpio_free(di->gpio_int);
	if (di->manual_otg_mode)
		(void)power_event_bnc_unregister(POWER_BNT_OTG, &di->otg_nb);
nu2205_fail_0:
	di->chip_already_init = 0;
	devm_kfree(&client->dev, di);

	return ret;
}

static int nu2205_remove(struct i2c_client *client)
{
	struct nu2205_device_info *di = i2c_get_clientdata(client);

	if (!di)
		return -ENODEV;

	hwlog_info("remove enter\n");
	if (di->manual_otg_mode)
		(void)power_event_bnc_unregister(POWER_BNT_OTG, &di->otg_nb);

	if (di->irq_int)
		free_irq(di->irq_int, di);
	if (di->gpio_int)
		gpio_free(di->gpio_int);

	nu2205_reg_reset(di);
	return 0;
}

static void nu2205_shutdown(struct i2c_client *client)
{
	struct nu2205_device_info *di = i2c_get_clientdata(client);

	if (!di)
		return;

	if (di->irq_int)
		free_irq(di->irq_int, di);
	if (di->gpio_int)
		gpio_free(di->gpio_int);

	nu2205_reg_reset(di);
}

#ifdef CONFIG_PM
static int nu2205_i2c_suspend(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct nu2205_device_info *di = NULL;

	if (!client)
		return 0;

	di = i2c_get_clientdata(client);
	if (di)
		nu2205_enable_adc(0, (void *)di);

	return 0;
}

static int nu2205_i2c_resume(struct device *dev)
{
	return 0;
}

static void nu2205_i2c_complete(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct nu2205_device_info *di = NULL;

	if (!client)
		return;

	di = i2c_get_clientdata(client);
	if (!di)
		return;

	nu2205_enable_adc(1, (void *)di);
}

static const struct dev_pm_ops nu2205_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(nu2205_i2c_suspend, nu2205_i2c_resume)
	.complete = nu2205_i2c_complete,
};
#define NU2205_PM_OPS (&nu2205_pm_ops)
#else
#define NU2205_PM_OPS (NULL)
#endif /* CONFIG_PM */

MODULE_DEVICE_TABLE(i2c, nu2205);
static const struct of_device_id nu2205_of_match[] = {
	{
		.compatible = "sc8571",
		.data = NULL,
	},
	{},
};

static const struct i2c_device_id nu2205_i2c_id[] = {
	{ "sc8571", 0 }, {}
};

static struct i2c_driver nu2205_driver = {
	.probe = nu2205_probe,
	.remove = nu2205_remove,
	.shutdown = nu2205_shutdown,
	.id_table = nu2205_i2c_id,
	.driver = {
		.owner = THIS_MODULE,
		.name = "nu2205",
		.of_match_table = of_match_ptr(nu2205_of_match),
		.pm = NU2205_PM_OPS,
	},
};

static int __init nu2205_init(void)
{
	return i2c_add_driver(&nu2205_driver);
}

static void __exit nu2205_exit(void)
{
	i2c_del_driver(&nu2205_driver);
}

module_init(nu2205_init);
module_exit(nu2205_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("nu2205 module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
