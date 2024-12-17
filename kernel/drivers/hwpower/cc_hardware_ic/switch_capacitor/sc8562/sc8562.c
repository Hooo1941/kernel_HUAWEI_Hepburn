// SPDX-License-Identifier: GPL-2.0
/*
 * sc8562.c
 *
 * sc8562 driver
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

#include "sc8562.h"
#include "sc8562_i2c.h"
#include "sc8562_scp.h"
#include "sc8565_ovp_switch.h"
#include <linux/delay.h>
#include <securec.h>
#include <chipset_common/hwpower/common_module/power_common_macro.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/common_module/power_event_ne.h>
#include <chipset_common/hwpower/common_module/power_pinctrl.h>
#include <chipset_common/hwpower/common_module/power_gpio.h>
#include <chipset_common/hwpower/common_module/power_delay.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/common_module/power_supply.h>
#include <chipset_common/hwpower/common_module/power_supply_interface.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_device_id.h>
#include <chipset_common/hwpower/hardware_ic/charge_pump.h>
#include <chipset_common/hwpower/reverse_charge/reverse_charge_pd.h>
#include <huawei_platform/hwpower/common_module/power_platform_macro.h>

#define HWLOG_TAG sc8562_chg
HWLOG_REGIST();

#define SC8562_REG_DUMP_MAX_NUM  0x14
#define SC8562_REG_DUMP_BUF_LEN  4096

static void sc8562_dump_register(struct sc8562_device_info *di)
{
	int i, ret;
	int len = 0;
	char buff[SC8562_REG_DUMP_BUF_LEN] = { 0 };
	u8 value[SC8562_REG_DUMP_MAX_NUM] = { 0 };

	sc8562_read_block(di, SC8562_VBAT_OVP_REG, value, SC8562_REG_DUMP_MAX_NUM);
	for (i = 0; i < SC8562_REG_DUMP_MAX_NUM; i++) {
		ret = snprintf_s(buff + len, sizeof(buff) - len, sizeof(buff) - len - 1,
			"reg[0x%x]=0x%x ", SC8562_VBAT_OVP_REG + i, value[i]);
		if (ret < 0)
			return;
		len += ret;
	}
	hwlog_info("%s ic_%u %s\n", __func__, di->ic_role, buff);
}

static int sc8562_discharge(int enable, void *dev_data)
{
	return 0;
}

static int sc8562_is_device_close(void *dev_data)
{
	u8 val = 0;
	struct sc8562_device_info *di = dev_data;

	if (!di)
		return -EPERM;

	if (sc8562_read_byte(di, SC8562_CONVERTER_STATE_REG, &val))
		return -EIO;

	if (val & SC8562_CP_SWITCHING_STAT_MASK)
		return 0;

	return 1; /* 1:ic is closed */
}

static int sc8562_get_device_id(void *dev_data)
{
	u8 part_info = 0;
	int ret;
	struct sc8562_device_info *di = dev_data;
	int i;

	if (!di)
		return SC8562_DEVICE_ID_GET_FAIL;

	if (di->get_id_time == SC8562_USED)
		return di->device_id;

	for (i = 0; i < SC8562_DEVICE_ID_GET_RETRY; i++) {
		di->get_id_time = SC8562_USED;
		ret = sc8562_read_byte(di, SC8562_DEVICE_ID_REG, &part_info);
		if (ret) {
			di->get_id_time = SC8562_NOT_USED;
			hwlog_err("ic_%u get_device_id read failed\n", di->ic_role);
		} else {
			break;
		}
	}

	if (di->get_id_time == SC8562_NOT_USED)
		return SC8562_DEVICE_ID_GET_FAIL;

	if (part_info == SC8562_DEVICE_ID_SC8562)
		di->device_id = SWITCHCAP_SC8562;
	else if (part_info == SC8565_DEVICE_ID_SC8562)
		di->device_id = SWITCHCAP_SC8565;
	else
		di->device_id = SC8562_DEVICE_ID_GET_FAIL;

	hwlog_info("ic_%u get_device_id [%x]=0x%x, device_id: 0x%x\n",
		di->ic_role, SC8562_DEVICE_ID_REG, part_info, di->device_id);

	return di->device_id;
}

static int sc8562_get_vbat_mv(void *dev_data)
{
	s16 data = 0;
	int vbat;
	struct sc8562_device_info *di = dev_data;

	if (!di)
		return -EPERM;

	if (sc8562_read_word(di, SC8562_VBAT_ADC1_REG, &data))
		return -EIO;

	/* VBAT ADC LBS: 1.25mV */
	vbat = (int)data * 125 / 100;

	hwlog_info("ic_%u VBAT_ADC=0x%x, vbat=%d\n", di->ic_role, data, vbat);

	return vbat;
}

static int sc8562_get_ibat_ma(int *ibat, void *dev_data)
{
	s16 data = 0;
	int ibat_ori;
	struct sc8562_device_info *di = dev_data;

	if (!ibat || !di)
		return -EPERM;

	if (sc8562_read_word(di, SC8562_IBAT_ADC1_REG, &data))
		return -EIO;

	if (di->device_id == SWITCHCAP_SC8565)
		/* IBAT ADC LBS: 3.75mA */
		ibat_ori = (int)data * 375 / 100;
	else
		/* IBAT ADC LBS: 3.125mA */
		ibat_ori = (int)data * 3125 / 1000;

	*ibat = ibat_ori * di->sense_r_config;
	*ibat /= di->sense_r_actual;

	hwlog_info("ic_%u IBAT_ADC=0x%x ibat_ori=%d ibat=%d\n",
		di->ic_role, data, ibat_ori, *ibat);

	return 0;
}

static int sc8562_get_ibus_ma(int *ibus, void *dev_data)
{
	s16 data = 0;
	struct sc8562_device_info *di = dev_data;

	if (!di || !ibus)
		return -EPERM;

	if (sc8562_read_word(di, SC8562_IBUS_ADC1_REG, &data))
		return -EIO;

	/* IBUS ADC LBS: 1.5625mA */
	*ibus = (int)data * 6400 / 4096;

	hwlog_info("ic_%u IBUS_ADC=0x%x, ibus=%d\n", di->ic_role, data, *ibus);

	return 0;
}

static int sc8562_get_ibus(int *ibus, void *dev_data)
{
	int delta_ibus;
	struct sc8562_device_info *di = dev_data;

	if (!di || !ibus)
		return -EPERM;

	if (sc8562_get_ibus_ma(ibus, dev_data))
		return -EPERM;

	/*
	 * The IBUS ADC has two 8-bit registers, and there is a probability
	 * that a read error occurs during carry, When delta_ibus is
	 * greater than 300, the ibus is read again at a delay of 1 ms
	 */
	delta_ibus = (*ibus > di->last_ibus) ? (*ibus - di->last_ibus) : (di->last_ibus - *ibus);
	if (*ibus && di->last_ibus && (delta_ibus > SC8562_ERROR_DELTA_IBUS)) {
		power_usleep(DT_USLEEP_1MS);
		hwlog_info("ic_%u ibus=%d last_ibus=%d, read again\n", di->ic_role, *ibus, di->last_ibus);
		if (sc8562_get_ibus_ma(ibus, dev_data))
			return -EPERM;
	}
	di->last_ibus = *ibus;

	return 0;
}

int sc8562_get_vbus_mv(int *vbus, void *dev_data)
{
	s16 data = 0;
	struct sc8562_device_info *di = dev_data;

	if (!di || !vbus)
		return -EPERM;

	if (sc8562_read_word(di, SC8562_VBUS_ADC1_REG, &data))
		return -EIO;

	/* VBUS ADC LBS: 6.25mV */
	*vbus = (int)data * 6250 / 1000;

	hwlog_info("ic_%u VBUS_ADC=0x%x, vbus=%d\n", di->ic_role, data, *vbus);

	return 0;
}

int sc8562_get_vbus_uvp_status(void *dev_data)
{
	u8 reg = 0;
	int ret;
	struct sc8562_device_info *di = dev_data;

	if (!di)
		return -EPERM;

	ret = sc8562_read_byte(di, SC8562_INT_STAT_REG, &reg);
	if (ret)
		return -EIO;
	hwlog_info("ic_%u [%x]=0x%x\n", di->ic_role, SC8562_INT_STAT_REG, reg);

	return (reg & SC8562_VBUS_PRESENT_STAT_MASK) ? 0 : SC8562_VBUS_PRESENT_STAT_UVP_MODE;
}

static int sc8562_get_vusb_mv(int *vusb, void *dev_data)
{
	s16 data = 0;
	struct sc8562_device_info *di = dev_data;

	if (!vusb || !di)
		return -EPERM;

	if (sc8562_read_word(di, SC8562_VUSB_ADC1_REG, &data))
		return -EIO;

	/* VUSB_ADC LSB: 6.25mV */
	*vusb = (int)data * 625 / 100;

	hwlog_info("ic_%u VUSB_ADC=0x%x, vusb=%d\n", di->ic_role, data, *vusb);

	return 0;
}

static int sc8562_get_device_temp(int *temp, void *dev_data)
{
	s16 data = 0;
	struct sc8562_device_info *di = dev_data;

	if (!temp || !di)
		return -EPERM;

	if (sc8562_read_word(di, SC8562_TDIE_ADC1_REG, &data))
		return -EIO;

	/* TDIE_ADC LSB: 0.5C */
	*temp = (int)data * 5 / 10;

	hwlog_info("ic_%u TDIE_ADC=0x%x temp=%d\n", di->ic_role, data, *temp);

	return 0;
}

static int sc8562_get_vout_mv(int *vout, void *dev_data)
{
	s16 data = 0;
	struct sc8562_device_info *di = dev_data;

	if (!vout || !di)
		return -EPERM;

	if (sc8562_read_word(di, SC8562_VOUT_ADC1_REG, &data))
		return -EIO;

	/* VOUT_ADC LSB: 1.25mV */
	*vout = (int)data * 125 / 100;

	hwlog_info("ic_%u VOUT_ADC=0x%x, vout=%d\n", di->ic_role, data, *vout);

	return 0;
}

static int sc8562_config_watchdog_ms(int time, void *dev_data)
{
	u8 val;
	u8 reg = 0;
	int ret;
	struct sc8562_device_info *di = dev_data;

	if (!di)
		return -EPERM;

	if (di->watchdog_time && time > 0)
		time = (time > di->watchdog_time) ? time : di->watchdog_time;

	if (time >= SC8562_WD_TMR_TIMING_30000MS)
		val = SC8562_WD_TMR_30000MS;
	else if (time >= SC8562_WD_TMR_TIMING_5000MS)
		val = SC8562_WD_TMR_5000MS;
	else if (time >= SC8562_WD_TMR_TIMING_1000MS)
		val = SC8562_WD_TMR_1000MS;
	else if (time >= SC8562_WD_TMR_TIMING_500MS)
		val = SC8562_WD_TMR_500MS;
	else if (time >= SC8562_WD_TMR_TIMING_200MS)
		val = SC8562_WD_TMR_200MS;
	else
		val = SC8562_WD_TMR_TIMING_DIS;

	ret = sc8562_write_mask(di, SC8562_CTRL3_REG, SC8562_CTRL3_WD_TIMEOUT_MASK,
		SC8562_CTRL3_WD_TIMEOUT_SHIFT, val);
	ret += sc8562_read_byte(di, SC8562_CTRL3_REG, &reg);
	if (ret)
		return -EIO;

	hwlog_info("ic_%u config_watchdog_ms [%x]=0x%x\n",
		di->ic_role, SC8562_CTRL3_REG, reg);
	return 0;
}

static int sc8562_kick_watchdog_ms(void *dev_data)
{
	return 0;
}

static int sc8562_config_vbat_ovp_th_mv(struct sc8562_device_info *di, int ovp_th)
{
	u8 vbat;
	int ret;

	if (ovp_th < SC8562_VBAT_OVP_MIN)
		ovp_th = SC8562_VBAT_OVP_MIN;

	if (ovp_th > SC8562_VBAT_OVP_MAX)
		ovp_th = SC8562_VBAT_OVP_MAX;

	vbat = (u8)((ovp_th - SC8562_VBAT_OVP_MIN) / SC8562_VBAT_OVP_STEP);
	ret = sc8562_write_mask(di, SC8562_VBAT_OVP_REG, SC8562_VBAT_OVP_TH_MASK,
		SC8562_VBAT_OVP_TH_SHIFT, vbat);
	ret += sc8562_read_byte(di, SC8562_VBAT_OVP_REG, &vbat);
	if (ret)
		return -EIO;

	hwlog_info("ic_%u config_vbat_ovp_threshold_mv [%x]=0x%x\n",
		di->ic_role, SC8562_VBAT_OVP_REG, vbat);

	return 0;
}

static int sc8562_config_ibat_ocp_th_ma(struct sc8562_device_info *di, int ocp_th)
{
	u8 value;
	int ret;
	int ibat_ocp_min;
	int ibat_ocp_max;

	if (di->device_id == SWITCHCAP_SC8565) {
		ibat_ocp_min = SC8565_IBAT_OCP_MIN;
		ibat_ocp_max = SC8565_IBAT_OCP_MAX;
	} else {
		ibat_ocp_min = SC8562_IBAT_OCP_MIN;
		ibat_ocp_max = SC8562_IBAT_OCP_MAX;
	}

	if (ocp_th < ibat_ocp_min)
		ocp_th = ibat_ocp_min;

	if (ocp_th > ibat_ocp_max)
		ocp_th = ibat_ocp_max;

	value = (u8)((ocp_th - ibat_ocp_min) / SC8562_IBAT_OCP_STEP);
	ret = sc8562_write_mask(di, SC8562_IBAT_OCP_REG, SC8562_IBAT_OCP_TH_MASK,
		SC8562_IBAT_OCP_TH_SHIFT, value);
	ret += sc8562_read_byte(di, SC8562_IBAT_OCP_REG, &value);
	if (ret)
		return -EIO;

	hwlog_info("ic_%u config_ibat_ocp_threshold_ma [%x]=0x%x\n",
		di->ic_role, SC8562_IBAT_OCP_REG, value);
	return 0;
}

int sc8562_config_vusb_ovp_th_mv(struct sc8562_device_info *di, int ovp_th)
{
	u8 value;
	int ret;

	if (ovp_th < SC8562_VUSB_VWPC_OVP_MIN)
		value = SC8562_VUSB_VWPC_OVP_DEF_VAL;
	else if (ovp_th >= SC8562_VUSB_VWPC_OVP_MAX)
		value = SC8562_VUSB_VWPC_OVP_MAX_VAL;
	else
		value = (u8)((ovp_th - SC8562_VUSB_VWPC_OVP_MIN) / SC8562_VUSB_VWPC_OVP_STEP);

	ret = sc8562_write_mask(di, SC8562_VUSB_OVP_REG, SC8562_VUSB_OVP_TH_MASK,
		SC8562_VUSB_OVP_TH_SHIFT, value);
	ret += sc8562_read_byte(di, SC8562_VUSB_OVP_REG, &value);
	if (ret)
		return -EIO;

	hwlog_info("ic_%u config_vusb_ovp_threshold_mv [%x]=0x%x\n",
		di->ic_role, SC8562_VUSB_OVP_REG, value);
	return 0;
}

static int sc8562_config_vwpc_ovp_th_mv(struct sc8562_device_info *di, int ovp_th)
{
	u8 value;
	int ret;

	if (ovp_th < SC8562_VUSB_VWPC_OVP_MIN)
		value = SC8562_VUSB_VWPC_OVP_DEF_VAL;
	else if (ovp_th >= SC8562_VUSB_VWPC_OVP_MAX)
		value = SC8562_VUSB_VWPC_OVP_MAX_VAL;
	else
		value = (u8)((ovp_th - SC8562_VUSB_VWPC_OVP_MIN) / SC8562_VUSB_VWPC_OVP_STEP);

	ret = sc8562_write_mask(di, SC8562_VWPC_OVP_REG, SC8562_VWPC_OVP_TH_MASK,
		SC8562_VWPC_OVP_TH_SHIFT, value);
	ret += sc8562_read_byte(di, SC8562_VWPC_OVP_REG, &value);
	if (ret)
		return -EIO;

	hwlog_info("ic_%u config_vwpc_ovp_threshold_mv [%x]=0x%x\n",
		di->ic_role, SC8562_VWPC_OVP_REG, value);
	return 0;
}

int sc8562_config_vout_ovp_th_mv(struct sc8562_device_info *di, int ovp_th)
{
	u8 value;
	int ret;

	if (ovp_th < SC8562_VOUT_OVP_MIN)
		ovp_th = SC8562_VOUT_OVP_MIN;

	if (ovp_th > SC8562_VOUT_OVP_MAX)
		ovp_th = SC8562_VOUT_OVP_MAX;

	value = (u8)((ovp_th - SC8562_VOUT_OVP_MIN) / SC8562_VOUT_OVP_STEP);
	ret = sc8562_write_mask(di, SC8562_VOUT_VBUS_OVP_REG, SC8562_VOUT_OVP_TH_MASK,
		SC8562_VOUT_OVP_TH_SHIFT, value);
	ret += sc8562_read_byte(di, SC8562_VOUT_VBUS_OVP_REG, &value);
	if (ret)
		return -EIO;

	hwlog_info("ic_%u config_vout_ovp_threshold_mv [%x]=0x%x\n",
		di->ic_role, SC8562_VOUT_VBUS_OVP_REG, value);
	return 0;
}

int sc8562_config_vbus_ovp_th_mv(struct sc8562_device_info *di, int ovp_th, int mode)
{
	u8 value;
	int ret;

	switch (mode) {
	case SC8562_CHG_FBYPASS_MODE:
		if (ovp_th < SC8562_VBUS_OVP_FBPSC_MIN)
			ovp_th = SC8562_VBUS_OVP_FBPSC_MIN;

		if (ovp_th > SC8562_VBUS_OVP_FBPSC_MAX)
			ovp_th = SC8562_VBUS_OVP_FBPSC_MAX;

		value = (u8)((ovp_th - SC8562_VBUS_OVP_FBPSC_MIN) / SC8562_VBUS_OVP_FBPSC_STEP);
		break;
	case SC8562_CHG_F21SC_MODE:
	case SC8562_CHG_R12SC_MODE:
		if (ovp_th < SC8562_VBUS_OVP_F21SC_MIN)
			ovp_th = SC8562_VBUS_OVP_F21SC_MIN;

		if (ovp_th > SC8562_VBUS_OVP_F21SC_MAX)
			ovp_th = SC8562_VBUS_OVP_F21SC_MAX;

		value = (u8)((ovp_th - SC8562_VBUS_OVP_F21SC_MIN) / SC8562_VBUS_OVP_F21SC_STEP);
		break;
	case SC8562_CHG_F41SC_MODE:
	case SC8562_CHG_R14SC_MODE:
		if (ovp_th < SC8562_VBUS_OVP_F41SC_MIN)
			ovp_th = SC8562_VBUS_OVP_F41SC_MIN;

		if (ovp_th > SC8562_VBUS_OVP_F41SC_MAX)
			ovp_th = SC8562_VBUS_OVP_F41SC_MAX;

		value = (u8)((ovp_th - SC8562_VBUS_OVP_F41SC_MIN) / SC8562_VBUS_OVP_F41SC_STEP);
		break;
	default:
		if (ovp_th < SC8562_VBUS_OVP_F21SC_MIN)
			ovp_th = SC8562_VBUS_OVP_F21SC_MIN;

		if (ovp_th > SC8562_VBUS_OVP_F21SC_MAX)
			ovp_th = SC8562_VBUS_OVP_F21SC_MAX;

		value = (u8)((ovp_th - SC8562_VBUS_OVP_F21SC_MIN) / SC8562_VBUS_OVP_F21SC_STEP);
		break;
	}

	ret = sc8562_write_mask(di, SC8562_VOUT_VBUS_OVP_REG, SC8562_VBUS_OVP_TH_MASK,
		SC8562_VBUS_OVP_TH_SHIFT, value);
	ret += sc8562_read_byte(di, SC8562_VOUT_VBUS_OVP_REG, &value);
	if (ret)
		return -EIO;

	hwlog_info("ic_%u config_vbus_ovp_threshole_mv [%x]=0x%x\n",
		di->ic_role, SC8562_VOUT_VBUS_OVP_REG, value);
	return 0;
}

static int sc8562_config_pmid2out_uvp_th_mv(struct sc8562_device_info *di, int mode)
{
	u8 value = SC8562_PMID2OUT_UVP_DEFAULT;

	if ((mode == SC8562_CHG_R12SC_MODE)|| (mode == SC8562_CHG_R14SC_MODE))
		value = SC8562_PMID2OUT_UVP_R12;

	return sc8562_write_mask(di, SC8562_PMID2OUT_UVP_REG, SC8562_PMID2OUT_UVP_TH_MASK,
		SC8562_PMID2OUT_UVP_TH_SHIFT, value);
}

static int sc8562_config_ibus_ocp_th_ma(struct sc8562_device_info *di, int ocp_th)
{
	u8 value;
	int ret;

	if (ocp_th < SC8562_IBUS_OCP_MIN)
		ocp_th = SC8562_IBUS_OCP_MIN;

	if (ocp_th > SC8562_IBUS_OCP_MAX)
		ocp_th = SC8562_IBUS_OCP_MAX;

	value = (u8)((ocp_th - SC8562_IBUS_OCP_MIN) / SC8562_IBUS_OCP_STEP);
	ret = sc8562_write_mask(di, SC8562_IBUS_OCP_REG,
		SC8562_IBUS_OCP_TH_MASK, SC8562_IBUS_OCP_TH_SHIFT, value);
	ret += sc8562_read_byte(di, SC8562_IBUS_OCP_REG, &value);
	if (ret)
		return -EIO;

	hwlog_info("ic_%u config_ibus_ocp_threshold_ma [%x]=0x%x\n",
		di->ic_role, SC8562_IBUS_OCP_REG, value);
	return 0;
}

static int sc8562_config_switching_frequency(int data, struct sc8562_device_info *di)
{
	u8 freq;
	int freq_shift, ret;

	if (data < SC8562_SW_FREQ_MIN)
		data = SC8562_SW_FREQ_MIN;

	if (data > SC8562_SW_FREQ_MAX)
		data = SC8562_SW_FREQ_MAX;

	freq = (u8)((data - SC8562_SW_FREQ_MIN) / SC8562_SW_FREQ_STEP);
	freq_shift = SC8562_SW_FREQ_SHIFT_NORMAL;
	ret = sc8562_write_mask(di, SC8562_CTRL2_REG, SC8562_CTRL2_FSW_SET_MASK,
		SC8562_CTRL2_FSW_SET_SHIFT, freq);
	if (di->device_id == SWITCHCAP_SC8565)
		ret += sc8562_write_mask(di, SC8562_CTRL2_REG, SC8565_CTRL2_FREQ_DITHER_MASK,
			SC8565_CTRL2_FREQ_DITHER_SHIFT, freq_shift);
	else
		ret += sc8562_write_mask(di, SC8562_CTRL2_REG, SC8562_CTRL2_FREQ_DITHER_MASK,
			SC8562_CTRL2_FREQ_DITHER_SHIFT, freq_shift);
	if (ret)
		return -EIO;

	hwlog_info("ic_%u config_switching_frequency [%x]=0x%x\n",
		di->ic_role, SC8562_CTRL2_REG, freq);
	hwlog_info("ic_%u config_adjustable_switching_frequency [%x]=0x%x\n",
		di->ic_role, SC8562_CTRL2_REG, freq_shift);

	return 0;
}

static int sc8562_config_ibat_sns_res(struct sc8562_device_info *di)
{
	u8 res_config;
	int ret;

	if (di->sense_r_config == SENSE_R_1_MOHM)
		res_config = SC8562_IBAT_SNS_RES_1MOHM;
	else
		res_config = SC8562_IBAT_SNS_RES_2MOHM;

	ret = sc8562_write_mask(di, SC8562_CTRL4_REG, SC8562_CTRL4_SET_IBAT_SNS_RES_MASK,
		SC8562_CTRL4_SET_IBAT_SNS_RES_SHIFT, res_config);
	if (ret)
		return -EIO;

	hwlog_info("ic_%u congfig_ibat_sns_res=%d\n",
		di->ic_role, di->sense_r_config);
	return 0;
}

static int sc8562_threshold_reg_init(struct sc8562_device_info *di, u8 mode)
{
	int ret, vbus_ovp, vwpc_ovp, vusb_ovp, ibat_ocp, ibus_ocp;

	if (mode == SC8562_CHG_FBYPASS_MODE) {
		vbus_ovp = SC8562_VBUS_OVP_FBPSC_INIT;
		vusb_ovp = SC8562_VUSB_VWPC_OVP_DEF;
		vwpc_ovp = SC8562_VUSB_VWPC_OVP_F21SC_INIT;
		ibat_ocp = SC8562_IBAT_OCP_FBPSC_INIT;
		ibus_ocp = SC8562_IBUS_OCP_FBPSC_INIT;
	} else if (mode == SC8562_CHG_F21SC_MODE || mode == SC8562_CHG_R12SC_MODE) {
		vbus_ovp = SC8562_VBUS_OVP_F21SC_INIT;
		vusb_ovp = SC8562_VUSB_VWPC_OVP_F21SC_INIT;
		vwpc_ovp = SC8562_VUSB_VWPC_OVP_F21SC_INIT;
		ibat_ocp = di->sc_ibat_ocp;
		ibus_ocp = di->sc_ibus_ocp;
	} else if (mode == SC8562_CHG_F41SC_MODE || mode == SC8562_CHG_R14SC_MODE) {
		vbus_ovp = SC8562_VBUS_OVP_F41SC_INIT;
		vusb_ovp = SC8562_VUSB_VWPC_OVP_F41SC_INIT;
		vwpc_ovp = SC8562_VUSB_VWPC_OVP_F41SC_INIT;
		ibat_ocp = di->sc4_ibat_ocp;
		ibus_ocp = di->sc4_ibus_ocp;
	} else {
		vbus_ovp = SC8562_VBUS_OVP_F21SC_INIT;
		vusb_ovp = SC8562_VUSB_VWPC_OVP_F21SC_INIT;
		vwpc_ovp = SC8562_VUSB_VWPC_OVP_F21SC_INIT;
		ibat_ocp = di->sc_ibat_ocp;
		ibus_ocp = di->sc_ibus_ocp;
	}

	ret = sc8562_config_vusb_ovp_th_mv(di, vusb_ovp);
	ret += sc8562_config_vwpc_ovp_th_mv(di, vwpc_ovp);
	ret += sc8562_config_vout_ovp_th_mv(di, SC8562_VOUT_OVP_INIT);
	ret += sc8562_config_vbus_ovp_th_mv(di, vbus_ovp, mode);
	ret += sc8562_config_ibus_ocp_th_ma(di, ibus_ocp);
	ret += sc8562_config_vbat_ovp_th_mv(di, SC8562_VBAT_OVP_INIT);
	ret += sc8562_config_ibat_ocp_th_ma(di, ibat_ocp);
	ret += sc8562_config_pmid2out_uvp_th_mv(di, mode);
	ret += sc8562_write_mask(di, SC8562_CTRL4_REG, SC8562_CTRL4_MODE_MASK,
		SC8562_CTRL4_MODE_SHIFT, mode);
	if (ret)
		hwlog_err("ic_%u protect threshold init failed\n", di->ic_role);

	di->charge_mode = mode;
	return ret;
}

static int sc8562_lvc_charge_enable(int enable, void *dev_data)
{
	int ret;
	u8 ctrl1_reg = 0;
	u8 ctrl4_reg = 0;
	u8 chg_en;
	struct sc8562_device_info *di = dev_data;

	if (!di)
		return -EPERM;

	chg_en = enable ? SC8562_CTRL1_CP_ENABLE : SC8562_CTRL1_CP_DISABLE;
	ret = sc8562_write_mask(di, SC8562_CTRL1_REG, SC8562_CTRL1_CP_EN_MASK,
		SC8562_CTRL1_CP_EN_SHIFT, SC8562_CTRL1_CP_DISABLE);
	ret += sc8562_write_mask(di, SC8562_CTRL4_REG, SC8562_CTRL4_MODE_MASK,
		SC8562_CTRL4_MODE_SHIFT, SC8562_CHG_FBYPASS_MODE);
	ret += sc8562_write_mask(di, SC8562_CTRL1_REG, SC8562_CTRL1_CP_EN_MASK,
		SC8562_CTRL1_CP_EN_SHIFT, chg_en);
	ret += sc8562_read_byte(di, SC8562_CTRL1_REG, &ctrl1_reg);
	ret += sc8562_read_byte(di, SC8562_CTRL4_REG, &ctrl4_reg);
	if (ret)
		return -EIO;

	hwlog_info("ic_role=%u, lvc_charge_enable [%x]=0x%x, [%x]=0x%x\n",
		di->ic_role, SC8562_CTRL1_REG, ctrl1_reg,
		SC8562_CTRL4_REG, ctrl4_reg);
	return 0;
}

static int sc8562_sc_charge_enable(int enable, void *dev_data)
{
	int ret;
	u8 ctrl1_reg = 0;
	u8 ctrl4_reg = 0;
	u8 chg_en;
	struct sc8562_device_info *di = dev_data;

	if (!di)
		return -EPERM;

	chg_en = enable ? SC8562_CTRL1_CP_ENABLE : SC8562_CTRL1_CP_DISABLE;
	ret = sc8562_write_mask(di, SC8562_CTRL1_REG, SC8562_CTRL1_CP_EN_MASK,
		SC8562_CTRL1_CP_EN_SHIFT, SC8562_CTRL1_CP_DISABLE);
	ret += sc8562_write_mask(di, SC8562_CTRL4_REG, SC8562_CTRL4_MODE_MASK,
		SC8562_CTRL4_MODE_SHIFT, SC8562_CHG_F21SC_MODE);
	ret += sc8562_write_mask(di, SC8562_CTRL1_REG, SC8562_CTRL1_CP_EN_MASK,
		SC8562_CTRL1_CP_EN_SHIFT, chg_en);
	ret += sc8562_read_byte(di, SC8562_CTRL1_REG, &ctrl1_reg);
	ret += sc8562_read_byte(di, SC8562_CTRL4_REG, &ctrl4_reg);
	if (ret)
		return -EIO;

	hwlog_info("ic_role=%u, sc_charge_enable [%x]=0x%x, [%x]=0x%x\n",
		di->ic_role, SC8562_CTRL1_REG, ctrl1_reg,
		SC8562_CTRL4_REG, ctrl4_reg);
	return 0;
}

static int sc8562_sc4_charge_enable(int enable, void *dev_data)
{
	int ret;
	u8 ctrl1_reg = 0;
	u8 ctrl4_reg = 0;
	u8 chg_en;
	struct sc8562_device_info *di = dev_data;

	if (!di)
		return -EPERM;

	chg_en = enable ? SC8562_CTRL1_CP_ENABLE : SC8562_CTRL1_CP_DISABLE;
	ret = sc8562_write_mask(di, SC8562_CTRL1_REG, SC8562_CTRL1_CP_EN_MASK,
		SC8562_CTRL1_CP_EN_SHIFT, SC8562_CTRL1_CP_DISABLE);
	ret += sc8562_write_mask(di, SC8562_CTRL4_REG, SC8562_CTRL4_MODE_MASK,
		SC8562_CTRL4_MODE_SHIFT, SC8562_CHG_F41SC_MODE);
	ret += sc8562_write_mask(di, SC8562_CTRL1_REG, SC8562_CTRL1_CP_EN_MASK,
		SC8562_CTRL1_CP_EN_SHIFT, chg_en);
	ret += sc8562_read_byte(di, SC8562_CTRL1_REG, &ctrl1_reg);
	ret += sc8562_read_byte(di, SC8562_CTRL4_REG, &ctrl4_reg);
	if (ret)
		return -EIO;

	hwlog_info("ic_role=%u, sc4_charge_enable [%x]=0x%x, [%x]=0x%x\n",
		di->ic_role, SC8562_CTRL1_REG, ctrl1_reg,
		SC8562_CTRL4_REG, ctrl4_reg);
	return 0;
}

static int sc8562_reverse_protect_enable(bool enable, int mode, struct sc8562_device_info *di)
{
	int ret;

	if (mode != SC8562_CHG_R14SC_MODE)
		return 0;

	if (!enable) {
		ret = sc8562_write_mask(di, SC8565_CTRL6_REG, SC8565_IBUS_RCP_DIS_MASK,
			SC8565_IBUS_RCP_DIS_SHIFT, SC8565_IBUS_RCP_ENABLE);
		ret += sc8562_write_mask(di, SC8562_PMID2OUT_UVP_REG, SC8562_PMID2OUT_UVP_DIS_MASK,
			SC8562_PMID2OUT_UVP_DIS_SHIFT, SC8565_PMID2OUT_UVP_DISABLE);
		if (di->device_id == SWITCHCAP_SC8565)
			ret += sc8562_write_mask(di, SC8565_CONV_OCP_REG, SC8565_CONV_OCP_MASK,
				SC8565_CONV_OCP_SHIFT, SC8565_CONV_OCP_DISABLE);
	} else {
		ret = sc8562_write_mask(di, SC8565_CTRL6_REG, SC8565_IBUS_RCP_DIS_MASK,
			SC8565_IBUS_RCP_DIS_SHIFT, SC8565_IBUS_RCP_DISABLE);
		ret += sc8562_write_mask(di, SC8562_PMID2OUT_UVP_REG, SC8562_PMID2OUT_UVP_DIS_MASK,
			SC8562_PMID2OUT_UVP_DIS_SHIFT, SC8565_PMID2OUT_UVP_ENABLE);
		if (di->device_id == SWITCHCAP_SC8565)
			ret += sc8562_write_mask(di, SC8565_CONV_OCP_REG, SC8565_CONV_OCP_MASK,
				SC8565_CONV_OCP_SHIFT, SC8565_CONV_OCP_ENABLE);
	}

	return ret;
}

static int sc8562_reverse_mode_enable(bool enable, int mode, struct sc8562_device_info *di)
{
	int ret;
	u8 ctrl1_reg = 0;
	u8 ctrl4_reg = 0;
	u8 reverse_en;

	reverse_en = enable ? SC8562_CTRL1_CP_ENABLE : SC8562_CTRL1_CP_DISABLE;
	ret = sc8562_write_mask(di, SC8562_CTRL1_REG, SC8562_CTRL1_CP_EN_MASK,
		SC8562_CTRL1_CP_EN_SHIFT, SC8562_CTRL1_CP_DISABLE);
	if (reverse_en) {
		ret += sc8562_write_mask(di, SC8562_CTRL4_REG, SC8562_CTRL4_MODE_MASK,
			SC8562_CTRL4_MODE_SHIFT, mode);
		ret += sc8562_write_multi_mask(di, SC8562_CTRL1_REG, SC8562_CTRL1_ACDRV_MANUAL_EN_MASK |
			SC8562_CTRL1_OVPGATE_EN_MASK | SC8562_CTRL1_WPCGATE_EN_MASK,
			SC8562_CTRL1_ACDRV_MANUAL_EN_MASK | SC8562_CTRL1_WPCGATE_EN_MASK);
		ret += sc8562_reverse_protect_enable(false, mode, di);
		power_msleep(DT_MSLEEP_100MS, 0, NULL);
	} else {
		ret += sc8562_write_mask(di, SC8562_CTRL4_REG, SC8562_CTRL4_MODE_MASK,
			SC8562_CTRL4_MODE_SHIFT, SC8562_CHG_F21SC_MODE);
		ret += sc8562_reverse_protect_enable(true, mode, di);
	}
	ret += sc8562_write_mask(di, SC8562_CTRL1_REG, SC8562_CTRL1_QB_EN_MASK,
		SC8562_CTRL1_QB_EN_SHIFT, reverse_en);
	ret += sc8562_write_mask(di, SC8562_CTRL1_REG, SC8562_CTRL1_CP_EN_MASK,
		SC8562_CTRL1_CP_EN_SHIFT, reverse_en);

	power_msleep(DT_MSLEEP_200MS, 0, NULL);
	ret += sc8562_read_byte(di, SC8562_CTRL1_REG, &ctrl1_reg);
	ret += sc8562_read_byte(di, SC8562_CTRL4_REG, &ctrl4_reg);
	if (ret)
		return -EIO;

	hwlog_info("%s ic_%u reg[0x%x]=0x%x, reg[0x%x]=0x%x\n", __func__,
		di->ic_role, SC8562_CTRL1_REG, ctrl1_reg, SC8562_CTRL4_REG, ctrl4_reg);
	return 0;
}

static int sc8562_wired_reverse_mode_enable(bool enable, struct sc8562_device_info *di)
{
	int i, ret;
	u8 reverse_en;
	u8 charge_mode;
	int retry_times = 15; /* retry 15*20=300ms */
	int vbus = 0;

	reverse_en = enable ? SC8562_CTRL1_CP_ENABLE : SC8562_CTRL1_CP_DISABLE;
	charge_mode = enable ? SC8562_CHG_R12SC_MODE : SC8562_CHG_F21SC_MODE;
	ret = sc8562_write_mask(di, SC8562_CTRL1_REG, SC8562_CTRL1_CP_EN_MASK,
		SC8562_CTRL1_CP_EN_SHIFT, SC8562_CTRL1_CP_DISABLE);
	ret += sc8562_write_mask(di, SC8562_CTRL4_REG, SC8562_CTRL4_MODE_MASK,
		SC8562_CTRL4_MODE_SHIFT, charge_mode);
	ret += sc8562_write_mask(di, SC8562_CTRL1_REG, SC8562_CTRL1_QB_EN_MASK,
		SC8562_CTRL1_QB_EN_SHIFT, reverse_en);
	ret += sc8562_write_mask(di, SC8562_CTRL1_REG, SC8562_CTRL1_CP_EN_MASK,
		SC8562_CTRL1_CP_EN_SHIFT, reverse_en);
	if (enable) {
		for (i = 0; i < retry_times; i++) {
			if (!sc8562_is_device_close(di)) {
				sc8562_get_vbus_mv(&vbus, di);
				hwlog_info("%s rc open succ, i=%d, vbus=%d\n", __func__, i, vbus);
				break;
			}
			power_msleep(DT_MSLEEP_20MS, 0, NULL);
		}
		if (i >= retry_times) {
			sc8562_wired_reverse_mode_enable(false, di);
			hwlog_info("%s rc open failed\n", __func__);
			return -EPERM;
		}
		ret += sc8562_write_multi_mask(di, SC8562_CTRL1_REG, SC8562_CTRL1_ACDRV_MANUAL_EN_MASK |
			SC8562_CTRL1_OVPGATE_EN_MASK | SC8562_CTRL1_WPCGATE_EN_MASK,
			SC8562_CTRL1_ACDRV_MANUAL_EN_MASK | SC8562_CTRL1_OVPGATE_EN_MASK);
	} else {
		ret += sc8562_write_multi_mask(di, SC8562_CTRL1_REG,
			SC8562_CTRL1_ACDRV_MANUAL_EN_MASK | SC8562_CTRL1_OVPGATE_EN_MASK,
			SC8562_CTRL1_ACDRV_MANUAL_EN_MASK);
	}

	hwlog_info("%s enable=%d ret=%d\n", __func__, enable, ret);
	return ret;
}

static int sc8562_reg_reset(struct sc8562_device_info *di)
{
	int ret;
	u8 ctrl4_reg = 0;

	ret = sc8562_write_mask(di, SC8562_CTRL4_REG,
		SC8562_CTRL4_REG_RST_MASK, SC8562_CTRL4_REG_RST_SHIFT,
		SC8562_REG_RST_ENABLE);
	power_usleep(DT_USLEEP_1MS);
	ret += sc8562_write_multi_mask(di, SC8562_CTRL1_REG,
		SC8562_CTRL1_ACDRV_MANUAL_EN_MASK | SC8562_CTRL1_WPCGATE_EN_MASK |
		SC8562_CTRL1_OVPGATE_EN_MASK,
		SC8562_CTRL1_ACDRV_MANUAL_EN_MASK | SC8562_CTRL1_OVPGATE_EN_MASK |
		SC8562_CTRL1_WPCGATE_EN_MASK);
	ret += sc8562_config_vusb_ovp_th_mv(di, SC8562_VUSB_VWPC_OVP_F21SC_INIT);
	ret += sc8562_config_vwpc_ovp_th_mv(di, SC8562_VUSB_VWPC_OVP_F21SC_INIT);
	if (ret)
		return -EIO;

	ret = sc8562_read_byte(di, SC8562_CTRL4_REG, &ctrl4_reg);
	if (ret)
		return -EIO;

	di->charge_mode = SC8562_CHG_F21SC_MODE;
	hwlog_info("ic_%u reg_reset [%x]=0x%x\n",
		di->ic_role, SC8562_CTRL4_REG, ctrl4_reg);
	return 0;
}

static int sc8562_chip_init(void *dev_data)
{
	return 0;
}

static int sc8562_reg_init(struct sc8562_device_info *di)
{
	int ret;

	ret = sc8562_config_watchdog_ms(SC8562_WD_TMR_TIMING_DIS, di);
	ret += sc8562_config_ibat_sns_res(di);
	ret += sc8562_write_byte(di, SC8562_INT_MASK_REG,
		SC8562_INT_MASK_REG_INIT);
	ret += sc8562_write_mask(di, SC8562_ADC_CTRL_REG,
		SC8562_ADC_CTRL_ADC_EN_MASK, SC8562_ADC_CTRL_ADC_EN_SHIFT, SC8562_ADC_ENABLE);
	ret += sc8562_write_mask(di, SC8562_ADC_CTRL_REG,
		SC8562_ADC_CTRL_ADC_RATE_MASK, SC8562_ADC_CTRL_ADC_RATE_SHIFT, SC8562_ADC_DISABLE);
	ret += sc8562_write_mask(di, SC8562_DPDM_CTRL2_REG,
		SC8562_DP_BUFF_EN_MASK, SC8562_DP_BUFF_EN_SHIFT, SC8562_DP_BUFF_EN_ENABLE);
	ret += sc8562_write_byte(di, SC8562_SCP_FLAG_MASK_REG,
		SC8562_SCP_FLAG_MASK_REG_INIT);
	ret += sc8562_write_mask(di, SC8562_IBUS_UCP_REG,
		SC8562_IBUS_UCP_FALL_DG_SET_MASK, SC8562_IBUS_UCP_FALL_DG_SET_SHIFT,
		SC8562_IBUS_UCP_FALL_DG_DEFAULT);
	if (di->gpio_enable > 0)
		ret += gpio_direction_output(di->gpio_enable, SC8562_GPIO_ENABLE);
	ret += sc8562_write_mask(di, SC8562_FUN_DIS_REG,
		SC8562_TSBAT_FLT_DIS_MASK, SC8562_TSBAT_FLT_DIS_SHIFT, SC8562_TSBAT_FLT_DIS_DISABLE);
	ret += sc8562_write_mask(di, SC8562_CTRL3_REG,
		SC8562_CTRL3_SS_TIMEOUT_MASK, SC8562_CTRL3_SS_TIMEOUT_SHIFT, SC8562_SS_TIMEOUT_DISABLE);
	if (di->device_id == SWITCHCAP_SC8565) {
		ret += sc8562_write_mask(di, SC8565_CTRL6_REG,
			SC8565_VUSB_REMOVE_MASK_MASK,
			SC8565_VUSB_REMOVE_MASK_SHIFT,
			SC8565_VUSB_REMOVE_MASK_ENABLE);
		ret += sc8562_write_byte(di, SC8565_CTRL_INITIAL_REG, 0x01);
		ret += sc8562_write_mask(di, SC8562_VUSB_OVP_REG,
			SC8562_OVPGATE_ON_DG_SET_MASK, SC8562_OVPGATE_ON_DG_SET_SHIFT, !!di->ovpgate_on_dg_set);
	}
	if (ret)
		hwlog_err("ic_%u reg_init failed %d\n", di->ic_role, ret);

	return ret;
}

static int sc8562_enable_adc(int enable, void *dev_data)
{
	int ret;
	u8 reg = 0;
	u8 value = enable ? 0x1 : 0x0;
	struct sc8562_device_info *di = dev_data;

	if (!di)
		return -ENODEV;

	ret = sc8562_write_mask(di, SC8562_ADC_CTRL_REG, SC8562_ADC_CTRL_ADC_EN_MASK,
		SC8562_ADC_CTRL_ADC_EN_SHIFT, value);
	if (ret)
		return -EPERM;

	ret = sc8562_read_byte(di, SC8562_ADC_CTRL_REG, &reg);
	if (ret)
		return -EPERM;

	hwlog_info("ic_%u adc_enable [%x]=0x%x\n",
		di->ic_role, SC8562_ADC_CTRL_REG, reg);
	return 0;
}

static int sc8562_lvc_charge_init(void *dev_data)
{
	int ret;
	struct sc8562_device_info *di = dev_data;

	if (!di)
		return -EPERM;

	ret = sc8562_reg_init(di);
	ret += sc8562_config_switching_frequency(SC8562_SW_FREQ_MIN, di);
	if (ret)
		return -EIO;

	di->init_finish_flag = SC8562_INIT_FINISH;

	return 0;
}

static int sc8562_sc_charge_init(void *dev_data)
{
	int ret;
	struct sc8562_device_info *di = dev_data;

	if (!di)
		return -EPERM;

	ret = sc8562_reg_init(di);
	ret += sc8562_config_switching_frequency(SC8562_SW_FREQ_MIN, di);
	if (ret)
		return -EIO;

	di->init_finish_flag = SC8562_INIT_FINISH;

	return 0;
}

static int sc8562_sc4_charge_init(void *dev_data)
{
	int ret;
	struct sc8562_device_info *di = dev_data;

	if (!di)
		return -EPERM;

	ret = sc8562_reg_init(di);
	ret += sc8562_config_switching_frequency(SC8562_SW_FREQ_600KHZ, di);
	if (ret)
		return -EIO;

	di->init_finish_flag = SC8562_INIT_FINISH;

	return 0;
}

static int sc8562_reg_and_threshold_init(void *dev_data)
{
	int ret;
	struct sc8562_device_info *di = dev_data;

	if (!di)
		return -EPERM;

	ret = sc8562_reg_init(di);
	ret += sc8562_threshold_reg_init(di, SC8562_CHG_F21SC_MODE);

	return ret;
}

static int sc8562_reg_reset_and_init(void *dev_data)
{
	int ret;
	struct sc8562_device_info *di = dev_data;

	if (!di)
		return -EPERM;

	ret = sc8562_reg_reset(di);
	ret += sc8562_reg_and_threshold_init(di);

	return ret;
}

static int sc8562_charge_exit(void *dev_data)
{
	int ret;
	struct sc8562_device_info *di = dev_data;

	if (!di)
		return -EPERM;

	ret = sc8562_sc_charge_enable(SC8562_SWITCHCAP_DISABLE, di);
	di->fcp_support = false;
	di->init_finish_flag = false;
	di->int_notify_enable_flag = false;

	return ret;
}

static int sc8562_batinfo_exit(void *dev_data)
{
	return 0;
}

static int sc8562_batinfo_init(void *dev_data)
{
	struct sc8562_device_info *di = dev_data;

	if (!di)
		return -EPERM;

	return sc8562_chip_init(di);
}

static int sc8562_lvc_set_threshold(int enable, void *dev_data)
{
	struct sc8562_device_info *di = dev_data;
	u8 mode = enable ? SC8562_CHG_FBYPASS_MODE : SC8562_CHG_F21SC_MODE;

	if (!di)
		return -EPERM;

	if (sc8562_threshold_reg_init(di, mode))
		return -EIO;

	return 0;
}

static int sc8562_sc_set_threshold(int enable, void *dev_data)
{
	struct sc8562_device_info *di = dev_data;
	u8 mode = SC8562_CHG_F21SC_MODE;

	if (!di)
		return -EPERM;

	if (sc8562_threshold_reg_init(di, mode))
		return -EIO;

	return 0;
}

static int sc8562_sc4_set_threshold(int enable, void *dev_data)
{
	struct sc8562_device_info *di = dev_data;
	u8 mode = enable ? SC8562_CHG_F41SC_MODE : SC8562_CHG_F21SC_MODE;

	if (!di)
		return -EPERM;

	if (sc8562_threshold_reg_init(di, mode))
		return -EIO;

	return 0;
}

static void sc8562_reverse_ocp_disable(struct sc8562_device_info *di, bool disable)
{
	sc8562_write_mask(di, SC8565_CTRL6_REG, SC8565_IBUS_RCP_DIS_MASK, SC8565_IBUS_RCP_DIS_SHIFT, disable);
}

static int sc8562_reverse12_set_threshold(bool enable, struct sc8562_device_info *di)
{
	u8 mode = enable ? SC8562_CHG_R12SC_MODE : SC8562_CHG_F21SC_MODE;

	return sc8562_threshold_reg_init(di, mode);
}

static int sc8562_reverse14_set_threshold(bool enable, struct sc8562_device_info *di)
{
	u8 mode = enable ? SC8562_CHG_R14SC_MODE : SC8562_CHG_F41SC_MODE;

	sc8562_reverse_ocp_disable(di, enable);
	return sc8562_threshold_reg_init(di, mode);
}

static int sc8562_get_register_head(char *buffer, int size, void *dev_data)
{
	struct sc8562_device_info *di = dev_data;

	if (!buffer || !di)
		return -EPERM;

	snprintf(buffer, size,
		"dev       mode   Ibus   Vbus   Ibat   Vusb   Vout   Vbat   Temp   ");

	return 0;
}

static int sc8562_value_dump(char *buffer, int size, void *dev_data)
{
	u8 val = 0;
	int ret;
	char buff[SC8562_BUF_LEN] = { 0 };
	struct sc8562_device_info *di = dev_data;
	struct sc8562_dump_value dv = { 0 };
	char device_name[SC8562_BUF_LEN] = { 0 };

	if (!buffer || !di)
		return -EPERM;

	dv.vbat = sc8562_get_vbat_mv(dev_data);
	(void)sc8562_get_ibus_ma(&dv.ibus, dev_data);
	(void)sc8562_get_vbus_mv(&dv.vbus, dev_data);
	(void)sc8562_get_ibat_ma(&dv.ibat, dev_data);
	(void)sc8562_get_vusb_mv(&dv.vusb, dev_data);
	(void)sc8562_get_vout_mv(&dv.vout, dev_data);
	(void)sc8562_get_device_temp(&dv.temp, dev_data);
	(void)sc8562_read_byte(di, SC8562_CTRL4_REG, &val);

	ret = snprintf_s(device_name, SC8562_BUF_LEN, SC8562_BUF_LEN - 1, "%s_%d",
		dc_get_device_name_without_mode(di->device_id), di->ic_role);
	if (ret < 0)
		return -EPERM;
	snprintf(buff, sizeof(buff), "%-10s", device_name);
	strncat(buffer, buff, strlen(buff));

	if (sc8562_is_device_close(dev_data))
		snprintf(buff, sizeof(buff), "%s", "OFF    ");
	else if (((val & SC8562_CTRL4_MODE_MASK) >> SC8562_CTRL4_MODE_SHIFT) ==
		SC8562_CHG_FBYPASS_MODE)
		snprintf(buff, sizeof(buff), "%s", "LVC    ");
	else if (((val & SC8562_CTRL4_MODE_MASK) >> SC8562_CTRL4_MODE_SHIFT) ==
		SC8562_CHG_F21SC_MODE)
		snprintf(buff, sizeof(buff), "%s", "SC     ");
	else if (((val & SC8562_CTRL4_MODE_MASK) >> SC8562_CTRL4_MODE_SHIFT) ==
		SC8562_CHG_F41SC_MODE)
		snprintf(buff, sizeof(buff), "%s", "SC4    ");
	else
		snprintf(buff, sizeof(buff), "%s", "BUCK   ");

	strncat(buffer, buff, strlen(buff));
	snprintf(buff, sizeof(buff), "%-7d%-7d%-7d%-7d%-7d%-7d%-7d",
		dv.ibus, dv.vbus, dv.ibat, dv.vusb, dv.vout, dv.vbat, dv.temp);
	strncat(buffer, buff, strlen(buff));

	return 0;
}

static int sc8562_cp_post_probe(void *dev_data)
{
	return 0;
}

static int sc8562_cp_device_check(void *dev_data)
{
	return 0;
}

static int sc8562_enable_wired_reverse_charge(bool enable, void *dev_data)
{
	int ret;
	struct sc8562_device_info *di = dev_data;

	if (!di)
		return -EPERM;

	ret = sc8562_reverse12_set_threshold(enable, di);
	if (ret)
		return ret;

	return sc8562_wired_reverse_mode_enable(enable, di);
}

static int sc8562_reverse_chip_init(void *dev_data, int mode, bool enable)
{
	struct sc8562_device_info *di = dev_data;
	int ret;
	int retry = 5; /* retry 5 times */
	int vbat = 0;

	if (!di || ((mode != SC8562_CHG_R12SC_MODE) && (mode != SC8562_CHG_R14SC_MODE)))
		return -EPERM;

	power_supply_get_int_prop(POWER_PLATFORM_BAT_PSY_NAME,
		POWER_SUPPLY_PROP_VOLTAGE_NOW, &vbat,
		POWER_SUPPLY_DEFAULT_VOLTAGE_NOW, POWER_UV_PER_MV);
	hwlog_info("vbat=%d\n", vbat);
	if (mode == SC8562_CHG_R14SC_MODE) {
		ret = sc8562_config_switching_frequency(SC8562_SW_FREQ_600KHZ, di);
		ret += sc8562_reverse14_set_threshold(enable, di);
	} else {
		ret = sc8562_reverse12_set_threshold(enable, di);
	}
	if (ret)
		return ret;

	if (enable) {
		do {
			sc8562_reverse_mode_enable(true, mode, di);
			if (!sc8562_is_device_close(dev_data)) {
				ret = sc8562_reverse_protect_enable(true, mode, di);
				if (ret)
					return ret;
				hwlog_info("set rvs cp open succ\n");
				return 0;
			}
		} while (retry-- > 0);
		hwlog_err("set rvs cp open failed\n");
		return -EIO;
	}
	ret = sc8562_reverse_mode_enable(false, mode, di);
	return ret;
}

static int sc8562_reverse_cp_chip_init(void *dev_data, bool enable)
{
	return sc8562_reverse_chip_init(dev_data, SC8562_CHG_R12SC_MODE, enable);
}

static int sc8562_reverse_cp4_chip_init(void *dev_data, bool enable)
{
	return sc8562_reverse_chip_init(dev_data, SC8562_CHG_R14SC_MODE, enable);
}

static struct dc_ic_ops sc8562_lvc_ops = {
	.dev_name = "sc8562",
	.ic_init = sc8562_lvc_charge_init,
	.ic_exit = sc8562_charge_exit,
	.ic_enable = sc8562_lvc_charge_enable,
	.ic_discharge = sc8562_discharge,
	.is_ic_close = sc8562_is_device_close,
	.get_ic_id = sc8562_get_device_id,
	.config_ic_watchdog = sc8562_config_watchdog_ms,
	.kick_ic_watchdog = sc8562_kick_watchdog_ms,
	.ic_reg_reset_and_init = sc8562_reg_and_threshold_init,
	.set_ic_thld = sc8562_lvc_set_threshold,
};

static struct dc_ic_ops sc8562_sc_ops = {
	.dev_name = "sc8562",
	.ic_init = sc8562_sc_charge_init,
	.ic_exit = sc8562_charge_exit,
	.ic_enable = sc8562_sc_charge_enable,
	.ic_discharge = sc8562_discharge,
	.is_ic_close = sc8562_is_device_close,
	.get_ic_id = sc8562_get_device_id,
	.config_ic_watchdog = sc8562_config_watchdog_ms,
	.kick_ic_watchdog = sc8562_kick_watchdog_ms,
	.ic_reg_reset_and_init = sc8562_reg_and_threshold_init,
	.set_ic_thld = sc8562_sc_set_threshold,
};

static struct dc_ic_ops sc8562_sc4_ops = {
	.dev_name = "sc8562",
	.ic_init = sc8562_sc4_charge_init,
	.ic_exit = sc8562_charge_exit,
	.ic_enable = sc8562_sc4_charge_enable,
	.ic_discharge = sc8562_discharge,
	.is_ic_close = sc8562_is_device_close,
	.get_ic_id = sc8562_get_device_id,
	.config_ic_watchdog = sc8562_config_watchdog_ms,
	.kick_ic_watchdog = sc8562_kick_watchdog_ms,
	.ic_reg_reset_and_init = sc8562_reg_and_threshold_init,
	.set_ic_thld = sc8562_sc4_set_threshold,
};

static struct dc_batinfo_ops sc8562_batinfo_ops = {
	.init = sc8562_batinfo_init,
	.exit = sc8562_batinfo_exit,
	.get_bat_btb_voltage = sc8562_get_vbat_mv,
	.get_bat_package_voltage = sc8562_get_vbat_mv,
	.get_vbus_voltage = sc8562_get_vbus_mv,
	.get_bat_current = sc8562_get_ibat_ma,
	.get_ic_ibus = sc8562_get_ibus,
	.get_ic_temp = sc8562_get_device_temp,
	.get_ic_vout = sc8562_get_vout_mv,
	.get_ic_vusb = sc8562_get_vusb_mv,
};

static struct charge_pump_ops sc8562_cp_ops = {
	.cp_type = CP_TYPE_MAIN,
	.chip_name = "sc8562",
	.dev_check = sc8562_cp_device_check,
	.post_probe = sc8562_cp_post_probe,
	.rvs_cp_chip_init = sc8562_reverse_cp_chip_init,
	.rvs_cp4_chip_init = sc8562_reverse_cp4_chip_init,
};

static struct rchg_pd_ic_ops sc8562_wired_rc_ops = {
	.dev_name = "sc8562",
	.enable_reverse_charge = sc8562_enable_wired_reverse_charge,
};

static struct power_log_ops sc8562_log_ops = {
	.dev_name = "sc8562",
	.dump_log_head = sc8562_get_register_head,
	.dump_log_content = sc8562_value_dump,
};

static void sc8562_init_ops_dev_data(struct sc8562_device_info *di)
{
	memcpy(&di->lvc_ops, &sc8562_lvc_ops, sizeof(struct dc_ic_ops));
	di->lvc_ops.dev_data = (void *)di;
	memcpy(&di->sc_ops, &sc8562_sc_ops, sizeof(struct dc_ic_ops));
	di->sc_ops.dev_data = (void *)di;
	memcpy(&di->sc4_ops, &sc8562_sc4_ops, sizeof(struct dc_ic_ops));
	di->sc4_ops.dev_data = (void *)di;
	memcpy(&di->batinfo_ops, &sc8562_batinfo_ops, sizeof(struct dc_batinfo_ops));
	di->batinfo_ops.dev_data = (void *)di;
	memcpy(&di->log_ops, &sc8562_log_ops, sizeof(struct power_log_ops));
	di->log_ops.dev_data = (void *)di;

	if (!di->ic_role) {
		snprintf(di->name, CHIP_DEV_NAME_LEN, "sc8562");
	} else {
		snprintf(di->name, CHIP_DEV_NAME_LEN, "sc8562_%u", di->ic_role);
		di->lvc_ops.dev_name = di->name;
		di->sc_ops.dev_name = di->name;
		di->sc4_ops.dev_name = di->name;
		di->log_ops.dev_name = di->name;
	}
}

static void sc8562_ops_register(struct sc8562_device_info *di)
{
	int ret;

	sc8562_init_ops_dev_data(di);

	ret = dc_ic_ops_register(LVC_MODE, di->ic_role, &di->lvc_ops);
	ret += dc_ic_ops_register(SC_MODE, di->ic_role, &di->sc_ops);
	ret += dc_ic_ops_register(SC4_MODE, di->ic_role, &di->sc4_ops);
	ret += dc_batinfo_ops_register(di->ic_role, &di->batinfo_ops, di->device_id);
	if (ret)
		hwlog_err("ic_%u sysinfo ops register failed\n", di->ic_role);

	ret = sc8562_protocol_ops_register(di);
	if (ret)
		hwlog_err("ic_%u scp or fcp ops register failed\n", di->ic_role);

	sc8565_power_sw_register(di);

	power_log_ops_register(&di->log_ops);
	if (di->rvs_cp_support) {
		sc8562_cp_ops.dev_data = (void *)di;
		(void)charge_pump_ops_register(&sc8562_cp_ops);
	}

	if (di->wired_rvs_support) {
		sc8562_wired_rc_ops.dev_data = (void *)di;
		(void)rchg_pd_ops_register(&sc8562_wired_rc_ops);
	}
}

static void sc8562_fault_event_notify(unsigned long event, void *data)
{
	power_event_anc_notify(POWER_ANT_DC_FAULT, event, data);
}

static void sc8562_interrupt_handle(struct sc8562_device_info *di,
	struct nty_data *data, u8 *flag)
{
	int val = 0;

	if (flag[SC8562_IRQ_VUSB_OVP] & SC8562_VUSB_OVP_FLAG_MASK) {
		hwlog_info("%s ic_%u USB OVP happened\n", __func__, di->ic_role);
		sc8562_fault_event_notify(POWER_NE_DC_FAULT_AC_OVP, data);
	} else if (flag[SC8562_IRQ_VBAT_OVP] & SC8562_VBAT_OVP_FLAG_MASK) {
		val = sc8562_get_vbat_mv(di);
		hwlog_info("%s ic_%u BAT OVP happened, vbat=%d\n", __func__, di->ic_role, val);
		if (val >= SC8562_VBAT_OVP_INIT)
			sc8562_fault_event_notify(POWER_NE_DC_FAULT_VBAT_OVP, data);
	} else if (flag[SC8562_IRQ_IBAT_OCP] & SC8562_IBAT_OCP_FLAG_MASK) {
		sc8562_get_ibat_ma(&val, di);
		hwlog_info("%s ic_%u BAT OCP happened, ibat=%d\n", __func__, di->ic_role, val);
		if (((val >= SC8562_IBAT_OCP_F41SC_INIT) && (di->charge_mode == SC8562_CHG_F41SC_MODE)) ||
			((val >= SC8562_IBAT_OCP_F21SC_INIT) && (di->charge_mode == SC8562_CHG_F21SC_MODE)) ||
			((val >= SC8562_IBAT_OCP_FBPSC_INIT) && (di->charge_mode == SC8562_CHG_FBYPASS_MODE)))
			sc8562_fault_event_notify(POWER_NE_DC_FAULT_IBAT_OCP, data);
	} else if (flag[SC8562_IRQ_FLT_FLAG] & SC8562_VBUS_OVP_FLAG_MASK) {
		sc8562_get_vbus_mv(&val, di);
		hwlog_info("%s ic_%u BUS OVP happened, vbus=%d\n", __func__, di->ic_role, val);
		if (((val >= SC8562_VBUS_OVP_F41SC_INIT) && (di->charge_mode == SC8562_CHG_F41SC_MODE)) ||
			((val >= SC8562_VBUS_OVP_F21SC_INIT) && (di->charge_mode == SC8562_CHG_F21SC_MODE)) ||
			((val >= SC8562_VBUS_OVP_FBPSC_INIT) && (di->charge_mode == SC8562_CHG_FBYPASS_MODE)))
			sc8562_fault_event_notify(POWER_NE_DC_FAULT_VBUS_OVP, data);
	} else if (flag[SC8562_IRQ_IBUS_OCP] & SC8562_IBUS_OCP_FLAG_MASK) {
		sc8562_get_ibus_ma(&val, di);
		hwlog_info("%s ic_%u BUS OCP happened, ibus=%d\n", __func__, di->ic_role, val);
		if (((val >= SC8562_IBUS_OCP_F41SC_INIT) && (di->charge_mode == SC8562_CHG_F41SC_MODE)) ||
			((val >= SC8562_IBUS_OCP_F21SC_INIT) && (di->charge_mode == SC8562_CHG_F21SC_MODE)) ||
			((val >= SC8562_IBUS_OCP_FBPSC_INIT) && (di->charge_mode == SC8562_CHG_FBYPASS_MODE)))
			sc8562_fault_event_notify(POWER_NE_DC_FAULT_IBUS_OCP, data);
	} else if (flag[SC8562_IRQ_FLT_FLAG] & SC8562_VOUT_OVP_FLAG_MASK) {
		hwlog_info("%s ic_%u VOUT OVP happened\n", __func__, di->ic_role);
	}

	if (flag[SC8562_IRQ_SCP_FLAG] & SC8562_TRANS_DONE_FLAG_MASK)
		di->scp_trans_done = true;
}

static void sc8562_interrupt_work(struct work_struct *work)
{
	u8 flag[SC8562_IRQ_END] = { 0 };
	struct sc8562_device_info *di = NULL;
	struct nty_data *data = NULL;

	if (!work)
		return;

	di = container_of(work, struct sc8562_device_info, irq_work);
	if (!di || !di->client) {
		hwlog_err("di is null\n");
		return;
	}

	(void)sc8562_read_byte(di, SC8562_VBAT_OVP_REG, &flag[SC8562_IRQ_VBAT_OVP]);
	(void)sc8562_read_byte(di, SC8562_FLT_FLAG_REG, &flag[SC8562_IRQ_FLT_FLAG]);
	(void)sc8562_read_byte(di, SC8562_VUSB_OVP_REG, &flag[SC8562_IRQ_VUSB_OVP]);
	(void)sc8562_read_byte(di, SC8562_VWPC_OVP_REG, &flag[SC8562_IRQ_VWPC_OVP]);
	(void)sc8562_read_byte(di, SC8562_IBUS_OCP_REG, &flag[SC8562_IRQ_IBUS_OCP]);
	(void)sc8562_read_byte(di, SC8562_IBAT_OCP_REG, &flag[SC8562_IRQ_IBAT_OCP]);
	(void)sc8562_read_byte(di, SC8562_SCP_FLAG_MASK_REG, &flag[SC8562_IRQ_SCP_FLAG]);
	(void)sc8562_read_byte(di, SC8562_INT_FLAG_REG, &flag[SC8562_IRQ_INT_FLAG]);

	data = &(di->nty_data);
	data->addr = di->client->addr;

	if (di->int_notify_enable_flag) {
		sc8562_interrupt_handle(di, data, flag);
		sc8562_dump_register(di);
	}

	hwlog_info("ic_%u FLAG_VBAT_OVP[0x%x]=0x%x FLAG_FLT_FLAG[0x%x]=0x%x FLAG_VUSB_OVP[0x%x]=0x%x " \
		"FLAG_VWPC_OVP[0x%x]=0x%x FLAG_IBUS_OCP[0x%x]=0x%x FLAG_IBAT_OCP[0x%x]=0x%x " \
		"FLAG_SCP_FLAG[0x%x]=0x%x FLAG_INT_FLAG[0x%x]=0x%x\n", di->ic_role,
		SC8562_VBAT_OVP_REG, flag[SC8562_IRQ_VBAT_OVP],
		SC8562_FLT_FLAG_REG, flag[SC8562_IRQ_FLT_FLAG],
		SC8562_VUSB_OVP_REG, flag[SC8562_IRQ_VUSB_OVP],
		SC8562_VWPC_OVP_REG, flag[SC8562_IRQ_VWPC_OVP],
		SC8562_IBUS_OCP_REG, flag[SC8562_IRQ_IBUS_OCP],
		SC8562_IBAT_OCP_REG, flag[SC8562_IRQ_IBAT_OCP],
		SC8562_SCP_FLAG_MASK_REG, flag[SC8562_IRQ_SCP_FLAG],
		SC8562_INT_FLAG_REG, flag[SC8562_IRQ_INT_FLAG]);

	enable_irq(di->irq_int);
}

static irqreturn_t sc8562_interrupt(int irq, void *_di)
{
	struct sc8562_device_info *di = _di;

	if (!di)
		return IRQ_HANDLED;

	if (di->init_finish_flag)
		di->int_notify_enable_flag = true;

	hwlog_debug("ic_%u int happened\n", di->ic_role);
	disable_irq_nosync(di->irq_int);
	queue_work(di->int_wq, &di->irq_work);

	return IRQ_HANDLED;
}

static int sc8562_irq_init(struct sc8562_device_info *di,
	struct device_node *np)
{
	int ret;

	di->int_wq = create_singlethread_workqueue("sc8562_int_irq");
	INIT_WORK(&di->irq_work, sc8562_interrupt_work);
	ret = power_gpio_config_interrupt(np,
		"gpio_int", "sc8562_gpio_int", &di->gpio_int, &di->irq_int);
	if (ret)
		return ret;

	ret = request_irq(di->irq_int, sc8562_interrupt,
		IRQF_TRIGGER_FALLING, "sc8562_int_irq", di);
	if (ret) {
		hwlog_err("gpio irq request failed\n");
		di->irq_int = -1;
		gpio_free(di->gpio_int);
		return ret;
	}

	enable_irq_wake(di->irq_int);

	return 0;
}

static int sc8562_gpio_init(struct sc8562_device_info *di,
	struct device_node *np)
{
	/* sc8565 has not gpio_reset and gpio_enable */
	if (of_find_property(np, "gpio_reset", NULL) &&
		power_gpio_config_output(np, "gpio_reset", "sc8562_gpio_reset",
		&di->gpio_reset, SC8562_SWITCHCAP_DISABLE))
		return -EINVAL;

	if (of_find_property(np, "gpio_enable", NULL) &&
		power_gpio_config_output(np, "gpio_enable", "sc8562_gpio_enable",
		&di->gpio_enable, SC8562_GPIO_ENABLE)) {
		if (di->gpio_reset > 0)
			gpio_free(di->gpio_reset);
		return -EPERM;
	}

	/* To avoid entering the low-power mode, pull up LPM pin first. */
	if (of_find_property(np, "gpio_lpm", NULL) &&
		power_gpio_config_output(np, "gpio_lpm", "sc8562_gpio_lpm",
		&di->gpio_lpm, SC8562_GPIO_ENABLE)) {
		if (di->gpio_reset > 0)
			gpio_free(di->gpio_reset);
		if (di->gpio_enable > 0)
			gpio_free(di->gpio_enable);
		return -EPERM;
	}

	/* ic need, ensure that the I2C communication is normal */
	(void)power_msleep(di->lpm_exit_time, 0, NULL);

	return 0;
}

static void sc8562_parse_dts(struct device_node *np,
	struct sc8562_device_info *di)
{
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"switching_frequency", &di->switching_frequency,
		SC8562_SW_FREQ_750KHZ);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"sc_ibus_ocp", &di->sc_ibus_ocp, SC8562_IBUS_OCP_F21SC_INIT);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"sc4_ibus_ocp", &di->sc4_ibus_ocp, SC8562_IBUS_OCP_F41SC_INIT);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"sc_ibat_ocp", &di->sc_ibat_ocp, SC8562_IBAT_OCP_F21SC_INIT);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"sc4_ibat_ocp", &di->sc4_ibat_ocp, SC8562_IBAT_OCP_F41SC_INIT);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"watchdog_time", &di->watchdog_time, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "scp_support",
		(u32 *)&(di->dts_scp_support), 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "fcp_support",
		(u32 *)&(di->dts_fcp_support), 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "ic_role",
		(u32 *)&(di->ic_role), IC_ONE);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"sense_r_config", &di->sense_r_config, SENSE_R_1_MOHM);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"sense_r_actual", &di->sense_r_actual, SENSE_R_1_MOHM);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"lpm_exit_time", &di->lpm_exit_time, DT_MSLEEP_50MS);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"rvs_cp_support", &di->rvs_cp_support, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"wired_rvs_support", &di->wired_rvs_support, 0);
	/* VBUS_OVP register[03h] Bit7, 0-40ms(default), 1-128ms */
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"ovpgate_on_dg_set", &di->ovpgate_on_dg_set, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"reboot_notifier_en", &di->reboot_notifier_en, 0);
	sc8565_parse_sw_attr(np, di);
}

static void sc8562_init_lock_mutex(struct sc8562_device_info *di)
{
	mutex_init(&di->scp_detect_lock);
	mutex_init(&di->accp_adapter_reg_lock);
}

static void sc8562_destroy_lock_mutex(struct sc8562_device_info *di)
{
	mutex_destroy(&di->scp_detect_lock);
	mutex_destroy(&di->accp_adapter_reg_lock);
}

static void sc8562_ic_reset_in_shutdown(struct sc8562_device_info *di)
{
	if (di->device_id == SWITCHCAP_SC8562)
		sc8562_scp_adapter_default_vset(di);
	sc8562_reg_reset(di);
	sc8562_write_mask(di, SC8562_CTRL1_REG, SC8562_CTRL1_ACDRV_MANUAL_EN_MASK,
		SC8562_CTRL1_ACDRV_MANUAL_EN_SHIFT, 0);
}

static int sc8562_reboot_notifier_call(struct notifier_block *nb, unsigned long event, void *data)
{
	struct sc8562_device_info *di = container_of(nb, struct sc8562_device_info, reboot_nb);

	if (!di || !di->reboot_notifier_en)
		return NOTIFY_OK;

	sc8562_ic_reset_in_shutdown(di);
	di->is_reboot = true;
	return NOTIFY_OK;
}

static int sc8562_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int ret;
	struct sc8562_device_info *di = NULL;
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
	di->i2c_is_working = true;

	sc8562_parse_dts(np, di);

	ret = sc8562_gpio_init(di, np);
	if (ret)
		goto sc8562_fail_0;

	ret = sc8562_get_device_id(di);
	if (ret == SC8562_DEVICE_ID_GET_FAIL)
		goto sc8562_fail_2;

	sc8562_init_lock_mutex(di);

	(void)power_pinctrl_config(di->dev, "pinctrl-names", 1); /* 1:pinctrl-names length */

	ret = sc8562_reg_reset_and_init(di);
	if (ret)
		goto sc8562_fail_1;

	ret = sc8562_irq_init(di, np);
	if (ret)
		goto sc8562_fail_1;

	di->reboot_nb.notifier_call = sc8562_reboot_notifier_call;
	register_reboot_notifier(&di->reboot_nb);

	sc8562_ops_register(di);
	i2c_set_clientdata(client, di);

	return 0;

sc8562_fail_1:
	sc8562_destroy_lock_mutex(di);
sc8562_fail_2:
	if (di->gpio_lpm > 0)
		gpio_free(di->gpio_lpm);
	if (di->gpio_reset > 0)
		gpio_free(di->gpio_reset);
	if (di->gpio_enable > 0)
		gpio_free(di->gpio_enable);
sc8562_fail_0:
	di->chip_already_init = 0;
	devm_kfree(&client->dev, di);

	return ret;
}

static int sc8562_remove(struct i2c_client *client)
{
	struct sc8562_device_info *di = i2c_get_clientdata(client);

	if (!di)
		return -EPERM;

	sc8562_reg_reset(di);
	unregister_reboot_notifier(&di->reboot_nb);

	if (di->irq_int)
		free_irq(di->irq_int, di);

	if (di->gpio_int)
		gpio_free(di->gpio_int);

	if (di->gpio_enable)
		gpio_free(di->gpio_enable);
	sc8562_destroy_lock_mutex(di);

	return 0;
}

static void sc8562_shutdown(struct i2c_client *client)
{
	struct sc8562_device_info *di = i2c_get_clientdata(client);

	if (!di || di->reboot_notifier_en)
		return;

	sc8562_ic_reset_in_shutdown(di);
}

#ifdef CONFIG_PM
static int sc8562_i2c_suspend(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct sc8562_device_info *di = NULL;

	if (!client)
		return 0;

	di = i2c_get_clientdata(client);
	if (di)
		sc8562_enable_adc(0, (void *)di);

	di->i2c_is_working = false;
	hwlog_info("ic_%u %s\n", di->ic_role, __func__);
	return 0;
}

static int sc8562_i2c_resume(struct device *dev)
{
	return 0;
}

static void sc8562_i2c_complete(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct sc8562_device_info *di = NULL;

	if (!client)
		return;

	di = i2c_get_clientdata(client);
	if (!di)
		return;

	di->i2c_is_working = true;
	sc8562_enable_adc(1, (void *)di);
	hwlog_info("ic_%u %s\n", di->ic_role, __func__);
}

static const struct dev_pm_ops sc8562_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(sc8562_i2c_suspend, sc8562_i2c_resume)
	.complete = sc8562_i2c_complete,
};
#define SC8562_PM_OPS (&sc8562_pm_ops)
#else
#define SC8562_PM_OPS (NULL)
#endif /* CONFIG_PM */

MODULE_DEVICE_TABLE(i2c, sc8562);
static const struct of_device_id sc8562_of_match[] = {
	{
		.compatible = "sc8562",
		.data = NULL,
	},
	{},
};

static const struct i2c_device_id sc8562_i2c_id[] = {
	{ "sc8562", 0 },
	{}
};

static struct i2c_driver sc8562_driver = {
	.probe = sc8562_probe,
	.remove = sc8562_remove,
	.shutdown = sc8562_shutdown,
	.id_table = sc8562_i2c_id,
	.driver = {
		.owner = THIS_MODULE,
		.name = "sc8562",
		.of_match_table = of_match_ptr(sc8562_of_match),
		.pm = SC8562_PM_OPS,
	},
};

static int __init sc8562_init(void)
{
	return i2c_add_driver(&sc8562_driver);
}

static void __exit sc8562_exit(void)
{
	i2c_del_driver(&sc8562_driver);
}

module_init(sc8562_init);
module_exit(sc8562_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("sc8562 module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
