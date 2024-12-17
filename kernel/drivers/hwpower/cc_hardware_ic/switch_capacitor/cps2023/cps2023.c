/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 */
// SPDX-License-Identifier: GPL-2.0
/*
 * cps2023.c
 *
 * cps2023 driver
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

#include "cps2023.h"
#include "cps2023_i2c.h"
#include "cps2023_scp.h"
#include "cps2023_ufcs.h"
#include "cps2023_ovp_switch.h"
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
#include <chipset_common/hwpower/wireless_charge/wireless_power_supply.h>

#define HWLOG_TAG cps2023_chg
HWLOG_REGIST();

#define CPS2023_REG_DUMP_MAX_NUM  0x14
#define CPS2023_REG_DUMP_BUF_LEN  4096
#define CPS2023_NOT_USED           0
#define CPS2023_USED               1
#define CPS2023_INIT_FINISH        1
#define CPS2023_BUF_LEN            80
#define CPS2023_CHG_MAX_MODE       6
#define CPS2023_VBUS_OVP_TYPE      0
#define CPS2023_VUSB_OVP_TYPE      1
#define CPS2023_VWPC_OVP_TYPE      2
#define CPS2023_IBAT_OCP_TYPE      3
#define CPS2023_IBUS_OCP_TYPE      4
#define CPS2023_OVP_OCP_TYPE_NUM   5
#define CPS2023_COMP_MAX_NUM       3

enum cps2023_info {
	CPS2023_INFO_IC_NAME = 0,
	CPS2023_INFO_MAX_IBAT,
	CPS2023_INFO_IBUS_OCP,
	CPS2023_INFO_TOTAL,
};

struct cps2023_dump_value {
	int vbat;
	int ibus;
	int vbus;
	int ibat;
	int vusb;
	int vout;
	int temp;
};

static void cps2023_dump_register(struct cps2023_device_info *di)
{
	int i, ret;
	int len = 0;
	char buff[CPS2023_REG_DUMP_BUF_LEN] = { 0 };
	u8 value[CPS2023_REG_DUMP_MAX_NUM] = { 0 };

	cps2023_read_block(di, CPS2023_VBAT_OVP_REG, value, CPS2023_REG_DUMP_MAX_NUM);
	for (i = 0; i < CPS2023_REG_DUMP_MAX_NUM; i++) {
		ret = snprintf_s(buff + len, sizeof(buff) - len, sizeof(buff) - len - 1,
			"reg[0x%x]=0x%x ", CPS2023_VBAT_OVP_REG + i, value[i]);
		if (ret < 0)
			return;
		len += ret;
	}
	hwlog_info("%s ic_%u %s\n", __func__, di->ic_role, buff);
}

static int cps2023_discharge(int enable, void *dev_data)
{
	return 0;
}

static int cps2023_is_device_close(void *dev_data)
{
	u8 val = 0;
	struct cps2023_device_info *di = dev_data;

	if (!di)
		return -ENODEV;

	if (cps2023_read_byte(di, CPS2023_CONVERTER_STATE_REG, &val))
		return -EIO;

	if (val & CPS2023_CP_SWITCHING_STAT_MASK)
		return 0;

	return 1; /* 1:ic is closed */
}

static int cps2023_get_device_id(void *dev_data)
{
	u8 part_info = 0;
	int ret;
	struct cps2023_device_info *di = dev_data;
	int i;

	if (!di)
		return CPS2023_DEVICE_ID_GET_FAIL;

	if (di->get_id_time == CPS2023_USED)
		return di->device_id;

	for (i = 0; i < CPS2023_DEVICE_ID_GET_RETRY; i++) {
		di->get_id_time = CPS2023_USED;
		ret = cps2023_read_byte(di, CPS2023_DEVICE_ID_REG, &part_info);
		if (ret) {
			di->get_id_time = CPS2023_NOT_USED;
			hwlog_err("ic_%u get_device_id read failed\n", di->ic_role);
		} else {
			break;
		}
	}

	if (di->get_id_time == CPS2023_NOT_USED)
		return CPS2023_DEVICE_ID_GET_FAIL;

	if (part_info == CPS2023_DEVICE_ID_CPS2023)
		di->device_id = SWITCHCAP_CPS2023;
	else
		di->device_id = CPS2023_DEVICE_ID_GET_FAIL;

	hwlog_info("ic_%u get_device_id [%x]=0x%x, device_id: 0x%x\n",
		di->ic_role, CPS2023_DEVICE_ID_REG, part_info, di->device_id);

	return di->device_id;
}

static int cps2023_get_vbat_mv(void *dev_data)
{
	s16 data = 0;
	int vbat;
	struct cps2023_device_info *di = dev_data;

	if (!di)
		return -ENODEV;

	if (cps2023_read_word(di, CPS2023_VBAT_ADC1_REG, &data))
		return -EIO;

	/* VBAT ADC LBS: 1.25mV */
	vbat = (int)data * 125 / 100;

	hwlog_info("ic_%u VBAT_ADC=0x%x, vbat=%d\n", di->ic_role, data, vbat);

	return vbat;
}

static int cps2023_get_ibat_ma(int *ibat, void *dev_data)
{
	s16 data = 0;
	int ibat_ori;
	struct cps2023_device_info *di = dev_data;

	if (!ibat || !di)
		return -ENODEV;

	if (cps2023_read_word(di, CPS2023_IBAT_ADC1_REG, &data))
		return -EIO;

	/* IBAT ADC LBS: 3.75mA */
	ibat_ori = (int)data * 375 / 100;
	*ibat = ibat_ori * di->sense_r_config;
	*ibat /= di->sense_r_actual;

	hwlog_info("ic_%u IBAT_ADC=0x%x ibat_ori=%d ibat=%d\n",
		di->ic_role, data, ibat_ori, *ibat);

	return 0;
}

static int cps2023_get_ibus_ma(int *ibus, void *dev_data)
{
	s16 data = 0;
	struct cps2023_device_info *di = dev_data;

	if (!di || !ibus)
		return -ENODEV;

	if (cps2023_read_word(di, CPS2023_IBUS_ADC1_REG, &data))
		return -EIO;

	/* IBUS ADC LBS: 1.875mA */
	*ibus = (int)data * 1875 / 1000;

	hwlog_info("ic_%u IBUS_ADC=0x%x, ibus=%d\n", di->ic_role, data, *ibus);

	return 0;
}

int cps2023_get_vbus_mv(int *vbus, void *dev_data)
{
	s16 data = 0;
	struct cps2023_device_info *di = dev_data;

	if (!di || !vbus)
		return -ENODEV;

	if (cps2023_read_word(di, CPS2023_VBUS_ADC1_REG, &data))
		return -EIO;

	/* VBUS ADC LBS: 6.25mV */
	*vbus = (int)data * 6250 / 1000;

	hwlog_info("ic_%u VBUS_ADC=0x%x, vbus=%d\n", di->ic_role, data, *vbus);

	return 0;
}

int cps2023_get_vbus_uvp_status(void *dev_data)
{
	u8 reg = 0;
	int ret;
	struct cps2023_device_info *di = dev_data;

	if (!di)
		return -EPERM;

	ret = cps2023_read_byte(di, CPS2023_INT_STAT_REG, &reg);
	if (ret)
		return -EIO;
	hwlog_info("ic_%u [%x]=0x%x\n", di->ic_role, CPS2023_INT_STAT_REG, reg);

	return (reg & CPS2023_VBUS_PRESENT_STAT_MASK) ? 0 : CPS2023_VBUS_PRESENT_STAT_UVP_MODE;
}

static int cps2023_get_vusb_mv(int *vusb, void *dev_data)
{
	s16 data = 0;
	struct cps2023_device_info *di = dev_data;

	if (!vusb || !di)
		return -ENODEV;

	if (cps2023_read_word(di, CPS2023_VUSB_ADC1_REG, &data))
		return -EIO;

	/* VUSB_ADC LSB: 6.25mV */
	*vusb = (int)data * 625 / 100;

	hwlog_info("ic_%u VUSB_ADC=0x%x, vusb=%d\n", di->ic_role, data, *vusb);

	return 0;
}

static int cps2023_get_device_temp(int *temp, void *dev_data)
{
	s16 data = 0;
	struct cps2023_device_info *di = dev_data;

	if (!temp || !di)
		return -ENODEV;

	if (cps2023_read_word(di, CPS2023_TDIE_ADC1_REG, &data))
		return -EIO;

	/* TDIE_ADC LSB: 0.5C - 40 */
	*temp = (int)data * 5 / 10 - 40;

	hwlog_info("ic_%u TDIE_ADC=0x%x temp=%d\n", di->ic_role, data, *temp);

	return 0;
}

static int cps2023_get_vout_mv(int *vout, void *dev_data)
{
	s16 data = 0;
	struct cps2023_device_info *di = dev_data;

	if (!vout || !di)
		return -ENODEV;

	if (cps2023_read_word(di, CPS2023_VOUT_ADC1_REG, &data))
		return -EIO;

	/* VOUT_ADC LSB: 1.25mV */
	*vout = (int)data * 125 / 100;

	hwlog_info("ic_%u VOUT_ADC=0x%x, vout=%d\n", di->ic_role, data, *vout);

	return 0;
}

static int cps2023_config_watchdog_ms(int time, void *dev_data)
{
	u8 val;
	u8 reg = 0;
	int ret;
	struct cps2023_device_info *di = dev_data;

	if (!di)
		return -ENODEV;

	if (di->watchdog_time && time > 0)
		time = (time > di->watchdog_time) ? time : di->watchdog_time;

	if (time >= CPS2023_WD_TMR_TIMING_30000MS)
		val = CPS2023_WD_TMR_30000MS;
	else if (time >= CPS2023_WD_TMR_TIMING_5000MS)
		val = CPS2023_WD_TMR_5000MS;
	else if (time >= CPS2023_WD_TMR_TIMING_1000MS)
		val = CPS2023_WD_TMR_1000MS;
	else if (time >= CPS2023_WD_TMR_TIMING_500MS)
		val = CPS2023_WD_TMR_500MS;
	else if (time >= CPS2023_WD_TMR_TIMING_200MS)
		val = CPS2023_WD_TMR_200MS;
	else
		val = CPS2023_WD_TMR_TIMING_DIS;

	ret = cps2023_write_mask(di, CPS2023_CTRL3_REG, CPS2023_CTRL3_WD_TIMEOUT_MASK,
		CPS2023_CTRL3_WD_TIMEOUT_SHIFT, val);
	ret += cps2023_read_byte(di, CPS2023_CTRL3_REG, &reg);
	if (ret)
		return -EIO;

	hwlog_info("ic_%u config_watchdog_ms [%x]=0x%x\n",
		di->ic_role, CPS2023_CTRL3_REG, reg);
	return 0;
}

static int cps2023_kick_watchdog_ms(void *dev_data)
{
	struct cps2023_device_info *di = dev_data;
	u8 reg = 0;

	if (!di)
		return -ENODEV;

	if (cps2023_read_byte(di, CPS2023_CTRL3_REG, &reg))
		return -EIO;

	hwlog_info("ic_%u kick_watchdog_ms [%x]=0x%x\n",
		di->ic_role, CPS2023_CTRL3_REG, reg);
	return 0;
}

static int cps2023_config_vbat_ovp_th_mv(struct cps2023_device_info *di, int ovp_th)
{
	u8 vbat;
	int ret;

	if (ovp_th < CPS2023_VBAT_OVP_MIN)
		ovp_th = CPS2023_VBAT_OVP_MIN;

	if (ovp_th > CPS2023_VBAT_OVP_MAX)
		ovp_th = CPS2023_VBAT_OVP_MAX;

	vbat = (u8)((ovp_th - CPS2023_VBAT_OVP_MIN) / CPS2023_VBAT_OVP_STEP);
	ret = cps2023_write_mask(di, CPS2023_VBAT_OVP_REG, CPS2023_VBAT_OVP_TH_MASK,
		CPS2023_VBAT_OVP_TH_SHIFT, vbat);
	ret += cps2023_read_byte(di, CPS2023_VBAT_OVP_REG, &vbat);
	if (ret)
		return -EIO;

	hwlog_info("ic_%u config_vbat_ovp_threshold_mv [%x]=0x%x\n",
		di->ic_role, CPS2023_VBAT_OVP_REG, vbat);

	return 0;
}

static int cps2023_config_ibat_ocp_th_ma(struct cps2023_device_info *di, int ocp_th)
{
	u8 value;
	int ret;

	if (ocp_th < CPS2023_IBAT_OCP_MIN)
		ocp_th = CPS2023_IBAT_OCP_MIN;

	if (ocp_th > CPS2023_IBAT_OCP_MAX)
		ocp_th = CPS2023_IBAT_OCP_MAX;

	value = (u8)((ocp_th - CPS2023_IBAT_OCP_MIN) / CPS2023_IBAT_OCP_STEP);
	ret = cps2023_write_mask(di, CPS2023_IBAT_OCP_REG, CPS2023_IBAT_OCP_TH_MASK,
		CPS2023_IBAT_OCP_TH_SHIFT, value);
	ret += cps2023_read_byte(di, CPS2023_IBAT_OCP_REG, &value);
	if (ret)
		return -EIO;

	hwlog_info("ic_%u config_ibat_ocp_threshold_ma [%x]=0x%x\n",
		di->ic_role, CPS2023_IBAT_OCP_REG, value);
	return 0;
}

int cps2023_config_vusb_ovp_th_mv(struct cps2023_device_info *di, int ovp_th)
{
	u8 value;
	int ret;

	if (ovp_th < CPS2023_VUSB_VWPC_OVP_MIN)
		value = CPS2023_VUSB_VWPC_OVP_DEF_VAL;
	else if (ovp_th >= CPS2023_VUSB_VWPC_OVP_MAX)
		value = CPS2023_VUSB_VWPC_OVP_MAX_VAL;
	else
		value = (u8)((ovp_th - CPS2023_VUSB_VWPC_OVP_MIN) / CPS2023_VUSB_VWPC_OVP_STEP);

	ret = cps2023_write_mask(di, CPS2023_VUSB_OVP_REG, CPS2023_VUSB_OVP_TH_MASK,
		CPS2023_VUSB_OVP_TH_SHIFT, value);
	ret += cps2023_read_byte(di, CPS2023_VUSB_OVP_REG, &value);
	if (ret)
		return -EIO;

	hwlog_info("ic_%u config_vusb_ovp_threshold_mv [%x]=0x%x\n",
		di->ic_role, CPS2023_VUSB_OVP_REG, value);
	return 0;
}

static int cps2023_config_vwpc_ovp_th_mv(struct cps2023_device_info *di, int ovp_th)
{
	u8 value;
	int ret;

	if (ovp_th < CPS2023_VUSB_VWPC_OVP_MIN)
		value = CPS2023_VUSB_VWPC_OVP_DEF_VAL;
	else if (ovp_th >= CPS2023_VUSB_VWPC_OVP_MAX)
		value = CPS2023_VUSB_VWPC_OVP_MAX_VAL;
	else
		value = (u8)((ovp_th - CPS2023_VUSB_VWPC_OVP_MIN) / CPS2023_VUSB_VWPC_OVP_STEP);

	ret = cps2023_write_mask(di, CPS2023_VWPC_OVP_REG, CPS2023_VWPC_OVP_TH_MASK,
		CPS2023_VWPC_OVP_TH_SHIFT, value);
	ret += cps2023_read_byte(di, CPS2023_VWPC_OVP_REG, &value);
	if (ret)
		return -EIO;

	hwlog_info("ic_%u config_vwpc_ovp_threshold_mv [%x]=0x%x\n",
		di->ic_role, CPS2023_VWPC_OVP_REG, value);
	return 0;
}

int cps2023_config_vout_ovp_th_mv(struct cps2023_device_info *di, int ovp_th)
{
	u8 value;
	int ret;

	if (ovp_th < CPS2023_VOUT_OVP_MIN)
		ovp_th = CPS2023_VOUT_OVP_MIN;

	if (ovp_th > CPS2023_VOUT_OVP_MAX)
		ovp_th = CPS2023_VOUT_OVP_MAX;

	value = (u8)((ovp_th - CPS2023_VOUT_OVP_MIN) / CPS2023_VOUT_OVP_STEP);
	ret = cps2023_write_mask(di, CPS2023_VOUT_VBUS_OVP_REG, CPS2023_VOUT_OVP_TH_MASK,
		CPS2023_VOUT_OVP_TH_SHIFT, value);
	ret += cps2023_read_byte(di, CPS2023_VOUT_VBUS_OVP_REG, &value);
	if (ret)
		return -EIO;

	hwlog_info("ic_%u config_vout_ovp_threshold_mv [%x]=0x%x\n",
		di->ic_role, CPS2023_VOUT_VBUS_OVP_REG, value);
	return 0;
}

int cps2023_config_vbus_ovp_th_mv(struct cps2023_device_info *di, int ovp_th, int mode)
{
	u8 value;
	int ret;

	switch (mode) {
	case CPS2023_CHG_FBYPASS_MODE:
		if (ovp_th < CPS2023_VBUS_OVP_FBPSC_MIN)
			ovp_th = CPS2023_VBUS_OVP_FBPSC_MIN;

		if (ovp_th > CPS2023_VBUS_OVP_FBPSC_MAX)
			ovp_th = CPS2023_VBUS_OVP_FBPSC_MAX;

		value = (u8)((ovp_th - CPS2023_VBUS_OVP_FBPSC_MIN) / CPS2023_VBUS_OVP_FBPSC_STEP);
		break;
	case CPS2023_CHG_F21SC_MODE:
	case CPS2023_CHG_R12SC_MODE:
		if (ovp_th < CPS2023_VBUS_OVP_F21SC_MIN)
			ovp_th = CPS2023_VBUS_OVP_F21SC_MIN;

		if (ovp_th > CPS2023_VBUS_OVP_F21SC_MAX)
			ovp_th = CPS2023_VBUS_OVP_F21SC_MAX;

		value = (u8)((ovp_th - CPS2023_VBUS_OVP_F21SC_MIN) / CPS2023_VBUS_OVP_F21SC_STEP);
		break;
	case CPS2023_CHG_F41SC_MODE:
	case CPS2023_CHG_R14SC_MODE:
		if (ovp_th < CPS2023_VBUS_OVP_F41SC_MIN)
			ovp_th = CPS2023_VBUS_OVP_F41SC_MIN;

		if (ovp_th > CPS2023_VBUS_OVP_F41SC_MAX)
			ovp_th = CPS2023_VBUS_OVP_F41SC_MAX;

		value = (u8)((ovp_th - CPS2023_VBUS_OVP_F41SC_MIN) / CPS2023_VBUS_OVP_F41SC_STEP);
		break;
	default:
		if (ovp_th < CPS2023_VBUS_OVP_F21SC_MIN)
			ovp_th = CPS2023_VBUS_OVP_F21SC_MIN;

		if (ovp_th > CPS2023_VBUS_OVP_F21SC_MAX)
			ovp_th = CPS2023_VBUS_OVP_F21SC_MAX;

		value = (u8)((ovp_th - CPS2023_VBUS_OVP_F21SC_MIN) / CPS2023_VBUS_OVP_F21SC_STEP);
		break;
	}

	ret = cps2023_write_mask(di, CPS2023_VOUT_VBUS_OVP_REG, CPS2023_VBUS_OVP_TH_MASK,
		CPS2023_VBUS_OVP_TH_SHIFT, value);
	ret += cps2023_read_byte(di, CPS2023_VOUT_VBUS_OVP_REG, &value);
	if (ret)
		return -EIO;

	hwlog_info("ic_%u config_vbus_ovp_threshole_mv [%x]=0x%x\n",
		di->ic_role, CPS2023_VOUT_VBUS_OVP_REG, value);
	return 0;
}

static int cps2023_config_pmid2out_uvp_th_mv(struct cps2023_device_info *di, int mode)
{
	u8 value = CPS2023_PMID2OUT_UVP_DEFAULT;

	if ((mode == CPS2023_CHG_R12SC_MODE)|| (mode == CPS2023_CHG_R14SC_MODE))
		value = CPS2023_PMID2OUT_UVP_R12;

	return cps2023_write_mask(di, CPS2023_PMID2OUT_UVP_REG, CPS2023_PMID2OUT_UVP_TH_MASK,
		CPS2023_PMID2OUT_UVP_TH_SHIFT, value);
}

static int cps2023_config_ibus_ocp_th_ma(struct cps2023_device_info *di, int ocp_th)
{
	u8 value;
	int ret;

	if (ocp_th < CPS2023_IBUS_OCP_MIN)
		ocp_th = CPS2023_IBUS_OCP_MIN;

	if (ocp_th > CPS2023_IBUS_OCP_MAX)
		ocp_th = CPS2023_IBUS_OCP_MAX;

	value = (u8)((ocp_th - CPS2023_IBUS_OCP_MIN) / CPS2023_IBUS_OCP_STEP);
	ret = cps2023_write_mask(di, CPS2023_IBUS_OCP_REG,
		CPS2023_IBUS_OCP_TH_MASK, CPS2023_IBUS_OCP_TH_SHIFT, value);
	ret += cps2023_read_byte(di, CPS2023_IBUS_OCP_REG, &value);
	if (ret)
		return -EIO;

	hwlog_info("ic_%u config_ibus_ocp_threshold_ma [%x]=0x%x\n",
		di->ic_role, CPS2023_IBUS_OCP_REG, value);
	return 0;
}

static int cps2023_config_switching_frequency(int data, struct cps2023_device_info *di)
{
	u8 freq;
	int freq_shift, ret;

	if (data < CPS2023_SW_FREQ_MIN)
		data = CPS2023_SW_FREQ_MIN;

	if (data > CPS2023_SW_FREQ_MAX)
		data = CPS2023_SW_FREQ_MAX;

	freq = (u8)((data - CPS2023_SW_FREQ_MIN) / CPS2023_SW_FREQ_STEP);
	freq_shift = CPS2023_SW_FREQ_SHIFT_NORMAL;
	ret = cps2023_write_mask(di, CPS2023_CTRL2_REG, CPS2023_CTRL2_FSW_SET_MASK,
		CPS2023_CTRL2_FSW_SET_SHIFT, freq);
	ret += cps2023_write_mask(di, CPS2023_CTRL2_REG, CPS2023_CTRL2_FREQ_DITHER_MASK,
			CPS2023_CTRL2_FREQ_DITHER_SHIFT, freq_shift);
	if (ret)
		return -EIO;

	hwlog_info("ic_%u config_switching_frequency [%x]=0x%x\n",
		di->ic_role, CPS2023_CTRL2_REG, freq);
	hwlog_info("ic_%u config_adjustable_switching_frequency [%x]=0x%x\n",
		di->ic_role, CPS2023_CTRL2_REG, freq_shift);

	return 0;
}

static int cps2023_config_ibat_sns_res(struct cps2023_device_info *di)
{
	u8 res_value;
	int ret;

	if (di->sense_r_config == SENSE_R_1_MOHM)
		res_value = CPS2023_IBAT_SNS_RES_1MOHM;
	else
		res_value = CPS2023_IBAT_SNS_RES_2MOHM;

	ret = cps2023_write_mask(di, CPS2023_CTRL4_REG, CPS2023_CTRL4_SET_IBAT_SNS_RES_MASK,
		CPS2023_CTRL4_SET_IBAT_SNS_RES_SHIFT, res_value);
	if (ret)
		return -EIO;

	hwlog_info("ic_%u congfig_ibat_sns_res=%d\n",
		di->ic_role, di->sense_r_config);
	return 0;
}

static int cps2023_threshold_reg_init(struct cps2023_device_info *di, u8 mode)
{
	int ret;
	/* 0: vbus; 1:vusb; 2:vwpc; 3:ibat_ocp; 4: ibus_ocp */
	int cps2023_ovpocp_value[CPS2023_CHG_MAX_MODE][CPS2023_OVP_OCP_TYPE_NUM] =  {
		{CPS2023_VBUS_OVP_F41SC_INIT, CPS2023_VUSB_VWPC_OVP_F41SC_INIT, CPS2023_VUSB_VWPC_OVP_F41SC_INIT,
			di->cps2023_sc4_para.ibat_ocp, di->cps2023_sc4_para.ibus_ocp},
		{CPS2023_VBUS_OVP_F21SC_INIT, CPS2023_VUSB_VWPC_OVP_F21SC_INIT, CPS2023_VUSB_VWPC_OVP_F21SC_INIT,
			di->cps2023_sc_para.ibat_ocp, di->cps2023_sc_para.ibus_ocp},
		{CPS2023_VBUS_OVP_FBPSC_INIT, CPS2023_VUSB_VWPC_OVP_DEF, CPS2023_VUSB_VWPC_OVP_F21SC_INIT,
			di->cps2023_lvc_para.ibat_ocp, di->cps2023_lvc_para.ibus_ocp},
		{0, 0, 0, 0, 0},
		{CPS2023_VBUS_OVP_F41SC_INIT, CPS2023_VUSB_VWPC_OVP_F41SC_INIT, CPS2023_VUSB_VWPC_OVP_F41SC_INIT,
			di->cps2023_sc4_para.ibat_ocp, di->cps2023_sc4_para.ibus_ocp},
		{CPS2023_VBUS_OVP_F21SC_INIT, CPS2023_VUSB_VWPC_OVP_F21SC_INIT, CPS2023_VUSB_VWPC_OVP_F21SC_INIT,
			di->cps2023_sc_para.ibat_ocp, di->cps2023_sc_para.ibus_ocp},
	};

	ret = cps2023_config_vusb_ovp_th_mv(di, cps2023_ovpocp_value[mode][CPS2023_VUSB_OVP_TYPE]);
	ret += cps2023_config_vwpc_ovp_th_mv(di, cps2023_ovpocp_value[mode][CPS2023_VWPC_OVP_TYPE]);
	ret += cps2023_config_vout_ovp_th_mv(di, di->vout_ovp_mv);
	ret += cps2023_config_vbus_ovp_th_mv(di, cps2023_ovpocp_value[mode][CPS2023_VBUS_OVP_TYPE], mode);
	ret += cps2023_config_ibus_ocp_th_ma(di, cps2023_ovpocp_value[mode][CPS2023_IBUS_OCP_TYPE]);
	ret += cps2023_config_vbat_ovp_th_mv(di, CPS2023_VBAT_OVP_INIT);
	ret += cps2023_config_ibat_ocp_th_ma(di, cps2023_ovpocp_value[mode][CPS2023_IBAT_OCP_TYPE]);
	ret += cps2023_config_pmid2out_uvp_th_mv(di, mode);
	ret += cps2023_write_mask(di, CPS2023_CTRL4_REG, CPS2023_CTRL4_MODE_MASK,
		CPS2023_CTRL4_MODE_SHIFT, mode);
	if (ret)
		hwlog_err("ic_%u protect threshold init failed\n", di->ic_role);

	di->charge_mode = mode;
	return ret;
}

static int cps2023_lvc_charge_enable(int enable, void *dev_data)
{
	int ret;
	u8 ctrl1_reg = 0;
	u8 ctrl4_reg = 0;
	u8 chg_en;
	struct cps2023_device_info *di = dev_data;

	if (!di)
		return -ENODEV;

	chg_en = enable ? CPS2023_CTRL1_CP_ENABLE : CPS2023_CTRL1_CP_DISABLE;
	ret = cps2023_write_mask(di, CPS2023_CTRL1_REG, CPS2023_CTRL1_CP_EN_MASK,
		CPS2023_CTRL1_CP_EN_SHIFT, CPS2023_CTRL1_CP_DISABLE);
	ret += cps2023_write_mask(di, CPS2023_CTRL4_REG, CPS2023_CTRL4_MODE_MASK,
		CPS2023_CTRL4_MODE_SHIFT, CPS2023_CHG_FBYPASS_MODE);
	ret += cps2023_write_mask(di, CPS2023_CTRL1_REG, CPS2023_CTRL1_CP_EN_MASK,
		CPS2023_CTRL1_CP_EN_SHIFT, chg_en);
	ret += cps2023_read_byte(di, CPS2023_CTRL1_REG, &ctrl1_reg);
	ret += cps2023_read_byte(di, CPS2023_CTRL4_REG, &ctrl4_reg);
	if (ret)
		return -EIO;

	hwlog_info("ic_role=%u, lvc_charge_enable [%x]=0x%x, [%x]=0x%x\n",
		di->ic_role, CPS2023_CTRL1_REG, ctrl1_reg,
		CPS2023_CTRL4_REG, ctrl4_reg);
	return 0;
}

static int cps2023_sc_charge_enable(int enable, void *dev_data)
{
	int ret;
	u8 ctrl1_reg = 0;
	u8 ctrl4_reg = 0;
	u8 chg_en;
	struct cps2023_device_info *di = dev_data;

	if (!di)
		return -ENODEV;

	chg_en = enable ? CPS2023_CTRL1_CP_ENABLE : CPS2023_CTRL1_CP_DISABLE;
	ret = cps2023_write_mask(di, CPS2023_CTRL1_REG, CPS2023_CTRL1_CP_EN_MASK,
		CPS2023_CTRL1_CP_EN_SHIFT, CPS2023_CTRL1_CP_DISABLE);
	ret += cps2023_write_mask(di, CPS2023_CTRL4_REG, CPS2023_CTRL4_MODE_MASK,
		CPS2023_CTRL4_MODE_SHIFT, CPS2023_CHG_F21SC_MODE);
	ret += cps2023_write_mask(di, CPS2023_CTRL1_REG, CPS2023_CTRL1_CP_EN_MASK,
		CPS2023_CTRL1_CP_EN_SHIFT, chg_en);
	ret += cps2023_read_byte(di, CPS2023_CTRL1_REG, &ctrl1_reg);
	ret += cps2023_read_byte(di, CPS2023_CTRL4_REG, &ctrl4_reg);
	if (ret)
		return -EIO;

	hwlog_info("ic_role=%u, sc_charge_enable [%x]=0x%x, [%x]=0x%x\n",
		di->ic_role, CPS2023_CTRL1_REG, ctrl1_reg,
		CPS2023_CTRL4_REG, ctrl4_reg);
	return 0;
}

static int cps2023_sc4_charge_enable(int enable, void *dev_data)
{
	int ret;
	u8 ctrl1_reg = 0;
	u8 ctrl4_reg = 0;
	u8 chg_en;
	struct cps2023_device_info *di = dev_data;

	if (!di)
		return -ENODEV;

	chg_en = enable ? CPS2023_CTRL1_CP_ENABLE : CPS2023_CTRL1_CP_DISABLE;
	ret = cps2023_write_mask(di, CPS2023_CTRL1_REG, CPS2023_CTRL1_CP_EN_MASK,
		CPS2023_CTRL1_CP_EN_SHIFT, CPS2023_CTRL1_CP_DISABLE);
	ret += cps2023_write_mask(di, CPS2023_CTRL4_REG, CPS2023_CTRL4_MODE_MASK,
		CPS2023_CTRL4_MODE_SHIFT, CPS2023_CHG_F41SC_MODE);
	ret += cps2023_write_mask(di, CPS2023_CTRL1_REG, CPS2023_CTRL1_CP_EN_MASK,
		CPS2023_CTRL1_CP_EN_SHIFT, chg_en);
	ret += cps2023_read_byte(di, CPS2023_CTRL1_REG, &ctrl1_reg);
	ret += cps2023_read_byte(di, CPS2023_CTRL4_REG, &ctrl4_reg);
	if (ret)
		return -EIO;

	hwlog_info("ic_role=%u, sc4_charge_enable [%x]=0x%x, [%x]=0x%x\n",
		di->ic_role, CPS2023_CTRL1_REG, ctrl1_reg,
		CPS2023_CTRL4_REG, ctrl4_reg);
	return 0;
}

static int cps2023_reverse_protect_enable(bool enable, int mode, struct cps2023_device_info *di)
{
	int ret;

	if (mode != CPS2023_CHG_R14SC_MODE)
		return 0;

	if (!enable) {
		ret = cps2023_write_mask(di, CPS2023_PMID2OUT_UVP_REG, CPS2023_PMID2OUT_UVP_DIS_MASK,
			CPS2023_PMID2OUT_UVP_DIS_SHIFT, CPS2023_PMID2OUT_UVP_DISABLE);
		ret += cps2023_write_mask(di, CPS2023_IBUS_OCP_REG, CPS2023_IBUS_OCP_DIS_MASK,
			CPS2023_IBUS_OCP_DIS_SHIFT, CPS2023_IBUS_OCP_DISABLE);
	} else {
		ret = cps2023_write_mask(di, CPS2023_PMID2OUT_UVP_REG, CPS2023_PMID2OUT_UVP_DIS_MASK,
			CPS2023_PMID2OUT_UVP_DIS_SHIFT, CPS2023_PMID2OUT_UVP_ENABLE);
		ret += cps2023_write_mask(di, CPS2023_IBUS_OCP_REG, CPS2023_IBUS_OCP_DIS_MASK,
			CPS2023_IBUS_OCP_DIS_SHIFT, CPS2023_IBUS_OCP_ENABLE);
	}

	return ret;
}

static int cps2023_reverse_mode_reg_init(bool enable,  int mode, struct cps2023_device_info *di)
{
	int ret;
	int retry = 5;
	u8 reverse_en;

	/* open rx ext pwr */
	wlps_control(0, WLPS_RX_EXT_PWR, true);
	reverse_en = enable ? CPS2023_CTRL1_CP_ENABLE : CPS2023_CTRL1_CP_DISABLE;
	ret = cps2023_write_mask(di, CPS2023_CTRL1_REG, CPS2023_CTRL1_CP_EN_MASK,
		CPS2023_CTRL1_CP_EN_SHIFT, CPS2023_CTRL1_CP_DISABLE);
	if (reverse_en) {
		ret += cps2023_write_mask(di, CPS2023_CTRL4_REG, CPS2023_CTRL4_MODE_MASK,
			CPS2023_CTRL4_MODE_SHIFT, mode);
		ret += cps2023_write_multi_mask(di, CPS2023_CTRL1_REG, CPS2023_CTRL1_ACDRV_MANUAL_EN_MASK |
			CPS2023_CTRL1_OVPGATE_EN_MASK | CPS2023_CTRL1_WPCGATE_EN_MASK, 0x20); /* ACDRV_MANUAL_EN */
		ret += cps2023_reverse_protect_enable(false, mode, di);
		power_msleep(DT_MSLEEP_10MS, 0, NULL);

		ret += cps2023_write_mask(di, CPS2023_CTRL1_REG, CPS2023_CTRL1_CP_EN_MASK,
			CPS2023_CTRL1_CP_EN_SHIFT, reverse_en);
		ret += cps2023_write_mask(di, CPS2023_CTRL1_REG, CPS2023_CTRL1_QB_EN_MASK,
			CPS2023_CTRL1_QB_EN_SHIFT, reverse_en);
		/* open tx sw with cp & qb en 150ms delay */
		power_msleep(DT_MSLEEP_150MS, 0, NULL);
		wlps_control(0, WLPS_TX_SW, true);
		/* close tx sw with 50ms delay */
		power_msleep(DT_MSLEEP_50MS, 0, NULL);
		wlps_control(0, WLPS_TX_SW, false);
		cps2023_write_multi_mask(di, CPS2023_CTRL1_REG, CPS2023_CTRL1_ACDRV_MANUAL_EN_MASK |
			CPS2023_CTRL1_OVPGATE_EN_MASK | CPS2023_CTRL1_WPCGATE_EN_MASK, 0x30); /* ACDRV_MANUAL_EN & WPCGATE_EN */
		do {
			ret += cps2023_write_mask(di, CPS2023_CTRL1_REG, CPS2023_CTRL1_CP_EN_MASK,
				CPS2023_CTRL1_CP_EN_SHIFT, reverse_en);
			ret += cps2023_write_mask(di, CPS2023_CTRL1_REG, CPS2023_CTRL1_QB_EN_MASK,
				CPS2023_CTRL1_QB_EN_SHIFT, reverse_en);
			ret += cps2023_write_multi_mask(di, CPS2023_CTRL1_REG, CPS2023_CTRL1_ACDRV_MANUAL_EN_MASK |
				CPS2023_CTRL1_OVPGATE_EN_MASK | CPS2023_CTRL1_WPCGATE_EN_MASK, 0x30); /* ACDRV_MANUAL_EN & WPCGATE_EN */
			power_msleep(DT_MSLEEP_50MS, 0, NULL);
		} while (retry-- > 0);
	} else {
		ret += cps2023_write_mask(di, CPS2023_CTRL4_REG, CPS2023_CTRL4_MODE_MASK,
			CPS2023_CTRL4_MODE_SHIFT, CPS2023_CHG_F21SC_MODE);
		ret += cps2023_reverse_protect_enable(true, mode, di);
		ret += cps2023_write_mask(di, CPS2023_CTRL1_REG, CPS2023_CTRL1_CP_EN_MASK,
			CPS2023_CTRL1_CP_EN_SHIFT, reverse_en);
		ret += cps2023_write_mask(di, CPS2023_CTRL1_REG, CPS2023_CTRL1_QB_EN_MASK,
			CPS2023_CTRL1_QB_EN_SHIFT, reverse_en);
	}

	return ret;
}

static int cps2023_wired_reverse_mode_enable(bool enable, struct cps2023_device_info *di)
{
	int i, ret;
	u8 reverse_en;
	u8 charge_mode;
	int retry_times = 15; /* retry 15*20=300ms */
	int vbus = 0;

	reverse_en = enable ? CPS2023_CTRL1_CP_ENABLE : CPS2023_CTRL1_CP_DISABLE;
	charge_mode = enable ? CPS2023_CHG_R12SC_MODE : CPS2023_CHG_F21SC_MODE;
	ret = cps2023_write_mask(di, CPS2023_CTRL1_REG, CPS2023_CTRL1_CP_EN_MASK,
		CPS2023_CTRL1_CP_EN_SHIFT, CPS2023_CTRL1_CP_DISABLE);
	ret += cps2023_write_mask(di, CPS2023_CTRL4_REG, CPS2023_CTRL4_MODE_MASK,
		CPS2023_CTRL4_MODE_SHIFT, charge_mode);
	ret += cps2023_write_mask(di, CPS2023_CTRL1_REG, CPS2023_CTRL1_QB_EN_MASK,
		CPS2023_CTRL1_QB_EN_SHIFT, reverse_en);
	ret += cps2023_write_mask(di, CPS2023_CTRL1_REG, CPS2023_CTRL1_CP_EN_MASK,
		CPS2023_CTRL1_CP_EN_SHIFT, reverse_en);

	if (enable) {
		for (i = 0; i < retry_times; i++) {
			if (!cps2023_is_device_close(di)) {
				cps2023_get_vbus_mv(&vbus, di);
				hwlog_info("%s rc open succ, i=%d, vbus=%d\n", __func__, i, vbus);
				break;
			}
			power_msleep(DT_MSLEEP_20MS, 0, NULL);
		}
		if (i >= retry_times) {
			cps2023_wired_reverse_mode_enable(false, di);
			hwlog_info("%s rc open failed\n", __func__);
			return -EPERM;
		}
		ret += cps2023_write_multi_mask(di, CPS2023_CTRL1_REG, CPS2023_CTRL1_ACDRV_MANUAL_EN_MASK |
			CPS2023_CTRL1_OVPGATE_EN_MASK | CPS2023_CTRL1_WPCGATE_EN_MASK,
			CPS2023_CTRL1_ACDRV_MANUAL_EN_MASK | CPS2023_CTRL1_OVPGATE_EN_MASK); /* usb ovp enable */
	} else {
		ret += cps2023_write_multi_mask(di, CPS2023_CTRL1_REG,
			CPS2023_CTRL1_ACDRV_MANUAL_EN_MASK | CPS2023_CTRL1_OVPGATE_EN_MASK,
			CPS2023_CTRL1_ACDRV_MANUAL_EN_MASK); /* usb ovp disable */
	}

	hwlog_info("%s enable=%d ret=%d\n", __func__, enable, ret);
	return ret;
}

static int cps2023_reverse_mode_enable(bool enable,  int mode, struct cps2023_device_info *di)
{
	int ret;
	u8 ctrl1_reg = 0;
	u8 ctrl4_reg = 0;

	ret = cps2023_reverse_mode_reg_init(enable, mode, di);
	power_msleep(DT_MSLEEP_200MS, 0, NULL);
	ret += cps2023_read_byte(di, CPS2023_CTRL1_REG, &ctrl1_reg);
	ret += cps2023_read_byte(di, CPS2023_CTRL4_REG, &ctrl4_reg);
	if (ret)
		return -EIO;

	hwlog_info("%s ic_%u reg[0x%x]=0x%x, reg[0x%x]=0x%x\n", __func__,
		di->ic_role, CPS2023_CTRL1_REG, ctrl1_reg, CPS2023_CTRL4_REG, ctrl4_reg);
	return 0;
}

static int cps2023_reg_reset(struct cps2023_device_info *di)
{
	int ret;
	u8 ctrl4_reg = 0;

	ret = cps2023_write_mask(di, CPS2023_CTRL4_REG,
		CPS2023_CTRL4_REG_RST_MASK, CPS2023_CTRL4_REG_RST_SHIFT,
		CPS2023_REG_RST_ENABLE);
	power_usleep(DT_USLEEP_1MS);
	ret += cps2023_write_multi_mask(di, CPS2023_CTRL1_REG,
		CPS2023_CTRL1_ACDRV_MANUAL_EN_MASK | CPS2023_CTRL1_OVPGATE_EN_MASK,
		CPS2023_CTRL1_ACDRV_MANUAL_EN_MASK | CPS2023_CTRL1_OVPGATE_EN_MASK);
	ret += cps2023_config_vusb_ovp_th_mv(di, CPS2023_VUSB_VWPC_OVP_F21SC_INIT);
	ret += cps2023_config_vwpc_ovp_th_mv(di, CPS2023_VUSB_VWPC_OVP_F21SC_INIT);
	if (ret)
		return -EIO;

	ret = cps2023_read_byte(di, CPS2023_CTRL4_REG, &ctrl4_reg);
	if (ret)
		return -EIO;

	di->charge_mode = CPS2023_CHG_F21SC_MODE;
	hwlog_info("ic_%u reg_reset [%x]=0x%x\n",
		di->ic_role, CPS2023_CTRL4_REG, ctrl4_reg);
	return 0;
}

static int cps2023_chip_init(void *dev_data)
{
	return 0;
}

static int cps2023_reg_init(struct cps2023_device_info *di)
{
	int ret;

	ret = cps2023_config_watchdog_ms(CPS2023_WD_TMR_TIMING_DIS, di);
	ret += cps2023_config_ibat_sns_res(di);
	ret += cps2023_write_byte(di, CPS2023_INT_MASK_REG,
		CPS2023_INT_MASK_REG_INIT);
	ret += cps2023_write_mask(di, CPS2023_ADC_CTRL_REG,
		CPS2023_ADC_CTRL_ADC_EN_MASK, CPS2023_ADC_CTRL_ADC_EN_SHIFT, CPS2023_ADC_ENABLE);
	ret += cps2023_write_mask(di, CPS2023_ADC_CTRL_REG,
		CPS2023_ADC_CTRL_ADC_RATE_MASK, CPS2023_ADC_CTRL_ADC_RATE_SHIFT, CPS2023_ADC_DISABLE);
	ret += cps2023_write_mask(di, CPS2023_IBUS_UCP_REG,
		CPS2023_IBUS_UCP_FALL_DG_SET_MASK, CPS2023_IBUS_UCP_FALL_DG_SET_SHIFT,
		CPS2023_IBUS_UCP_FALL_DG_DEFAULT);
	if (di->gpio_enable > 0)
		ret += gpio_direction_output(di->gpio_enable, CPS2023_GPIO_ENABLE);
	ret += cps2023_write_mask(di, CPS2023_CTRL3_REG,
		CPS2023_CTRL3_SS_TIMEOUT_MASK, CPS2023_CTRL3_SS_TIMEOUT_SHIFT, CPS2023_SS_TIMEOUT_DISABLE);
	if (ret)
		hwlog_err("ic_%u reg_init failed %d\n", di->ic_role, ret);

	return ret;
}

static int cps2023_enable_adc(int enable, void *dev_data)
{
	int ret;
	u8 reg = 0;
	u8 value = enable ? 0x1 : 0x0;
	struct cps2023_device_info *di = dev_data;

	if (!di)
		return -ENODEV;

	ret = cps2023_write_mask(di, CPS2023_ADC_CTRL_REG, CPS2023_ADC_CTRL_ADC_EN_MASK,
		CPS2023_ADC_CTRL_ADC_EN_SHIFT, value);
	if (ret)
		return -EIO;

	ret = cps2023_read_byte(di, CPS2023_ADC_CTRL_REG, &reg);
	if (ret)
		return -EIO;

	hwlog_info("ic_%u adc_enable [%x]=0x%x\n",
		di->ic_role, CPS2023_ADC_CTRL_REG, reg);
	return 0;
}

static int cps2023_lvc_charge_init(void *dev_data)
{
	int ret;
	struct cps2023_device_info *di = dev_data;

	if (!di)
		return -ENODEV;

	ret = cps2023_reg_init(di);
	ret += cps2023_config_switching_frequency(CPS2023_SW_FREQ_MIN, di);
	if (ret)
		return -EIO;

	di->init_finish_flag = CPS2023_INIT_FINISH;

	return 0;
}

static int cps2023_sc_charge_init(void *dev_data)
{
	int ret;
	struct cps2023_device_info *di = dev_data;

	if (!di)
		return -ENODEV;

	ret = cps2023_reg_init(di);
	ret += cps2023_config_switching_frequency(CPS2023_SW_FREQ_MIN, di);
	if (ret)
		return -EIO;

	di->init_finish_flag = CPS2023_INIT_FINISH;

	return 0;
}

static int cps2023_sc4_charge_init(void *dev_data)
{
	int ret;
	struct cps2023_device_info *di = dev_data;

	if (!di)
		return -ENODEV;

	ret = cps2023_reg_init(di);
	ret += cps2023_config_switching_frequency(CPS2023_SW_FREQ_600KHZ, di);
	if (ret)
		return -EIO;

	di->init_finish_flag = CPS2023_INIT_FINISH;

	return 0;
}

static int cps2023_reg_and_threshold_init(void *dev_data)
{
	int ret;
	struct cps2023_device_info *di = dev_data;

	if (!di)
		return -ENODEV;

	ret = cps2023_reg_init(di);
	ret += cps2023_threshold_reg_init(di, CPS2023_CHG_F21SC_MODE);

	return ret;
}

static int cps2023_reg_reset_and_init(void *dev_data)
{
	int ret;
	struct cps2023_device_info *di = dev_data;

	if (!di)
		return -ENODEV;

	ret = cps2023_reg_reset(di);
	ret += cps2023_reg_and_threshold_init(di);

	return ret;
}

static int cps2023_charge_exit(void *dev_data)
{
	int ret;
	struct cps2023_device_info *di = dev_data;

	if (!di)
		return -ENODEV;

	ret = cps2023_sc_charge_enable(CPS2023_SWITCHCAP_DISABLE, di);
	di->fcp_support = false;
	di->init_finish_flag = false;
	di->int_notify_enable_flag = false;

	return ret;
}

static int cps2023_batinfo_exit(void *dev_data)
{
	return 0;
}

static int cps2023_batinfo_init(void *dev_data)
{
	struct cps2023_device_info *di = dev_data;

	if (!di)
		return -ENODEV;

	return cps2023_chip_init(di);
}

static int cps2023_lvc_set_threshold(int enable, void *dev_data)
{
	struct cps2023_device_info *di = dev_data;
	u8 mode = enable ? CPS2023_CHG_FBYPASS_MODE : CPS2023_CHG_F21SC_MODE;

	if (!di)
		return -ENODEV;

	if (cps2023_threshold_reg_init(di, mode))
		return -EIO;

	return 0;
}

static int cps2023_sc_set_threshold(int enable, void *dev_data)
{
	struct cps2023_device_info *di = dev_data;
	u8 mode = CPS2023_CHG_F21SC_MODE;

	if (!di)
		return -ENODEV;

	if (cps2023_threshold_reg_init(di, mode))
		return -EIO;

	return 0;
}

static int cps2023_sc4_set_threshold(int enable, void *dev_data)
{
	struct cps2023_device_info *di = dev_data;
	u8 mode = enable ? CPS2023_CHG_F41SC_MODE : CPS2023_CHG_F21SC_MODE;

	if (!di)
		return -ENODEV;

	if (cps2023_threshold_reg_init(di, mode))
		return -EIO;

	return 0;
}

static int cps2023_reverse12_set_threshold(bool enable, struct cps2023_device_info *di)
{
	u8 mode = enable ? CPS2023_CHG_R12SC_MODE : CPS2023_CHG_F21SC_MODE;

	return cps2023_threshold_reg_init(di, mode);
}

static int cps2023_reverse14_set_threshold(bool enable, struct cps2023_device_info *di)
{
	u8 mode = enable ? CPS2023_CHG_R14SC_MODE : CPS2023_CHG_F41SC_MODE;

	return cps2023_threshold_reg_init(di, mode);
}

static int cps2023_get_register_head(char *buffer, int size, void *dev_data)
{
	struct cps2023_device_info *di = dev_data;

	if (!buffer || !di)
		return -ENODEV;

	snprintf(buffer, size,
		"dev       mode   Ibus   Vbus   Ibat   Vusb   Vout   Vbat   Temp   ");

	return 0;
}

static int cps2023_value_dump(char *buffer, int size, void *dev_data)
{
	u8 val = 0;
	int ret;
	char buff[CPS2023_BUF_LEN] = { 0 };
	struct cps2023_device_info *di = dev_data;
	struct cps2023_dump_value dv = { 0 };
	char device_name[CPS2023_BUF_LEN] = { 0 };

	if (!buffer || !di)
		return -ENODEV;

	dv.vbat = cps2023_get_vbat_mv(dev_data);
	(void)cps2023_get_ibus_ma(&dv.ibus, dev_data);
	(void)cps2023_get_vbus_mv(&dv.vbus, dev_data);
	(void)cps2023_get_ibat_ma(&dv.ibat, dev_data);
	(void)cps2023_get_vusb_mv(&dv.vusb, dev_data);
	(void)cps2023_get_vout_mv(&dv.vout, dev_data);
	(void)cps2023_get_device_temp(&dv.temp, dev_data);
	(void)cps2023_read_byte(di, CPS2023_CTRL4_REG, &val);

	ret = snprintf_s(device_name, CPS2023_BUF_LEN, CPS2023_BUF_LEN - 1, "%s_%d",
		dc_get_device_name_without_mode(di->device_id), di->ic_role);
	if (ret < 0)
		return -EPERM;
	snprintf(buff, sizeof(buff), "%-10s", device_name);
	strncat(buffer, buff, strlen(buff));

	if (cps2023_is_device_close(dev_data))
		snprintf(buff, sizeof(buff), "%s", "OFF    ");
	else if (((val & CPS2023_CTRL4_MODE_MASK) >> CPS2023_CTRL4_MODE_SHIFT) ==
		CPS2023_CHG_FBYPASS_MODE)
		snprintf(buff, sizeof(buff), "%s", "LVC    ");
	else if (((val & CPS2023_CTRL4_MODE_MASK) >> CPS2023_CTRL4_MODE_SHIFT) ==
		CPS2023_CHG_F21SC_MODE)
		snprintf(buff, sizeof(buff), "%s", "SC     ");
	else if (((val & CPS2023_CTRL4_MODE_MASK) >> CPS2023_CTRL4_MODE_SHIFT) ==
		CPS2023_CHG_F41SC_MODE)
		snprintf(buff, sizeof(buff), "%s", "SC4    ");
	else
		snprintf(buff, sizeof(buff), "%s", "BUCK   ");

	strncat(buffer, buff, strlen(buff));
	snprintf(buff, sizeof(buff), "%-7d%-7d%-7d%-7d%-7d%-7d%-7d",
		dv.ibus, dv.vbus, dv.ibat, dv.vusb, dv.vout, dv.vbat, dv.temp);
	strncat(buffer, buff, strlen(buff));

	return 0;
}

static int cps2023_cp_post_probe(void *dev_data)
{
	return 0;
}

static int cps2023_cp_device_check(void *dev_data)
{
	return 0;
}
static int cps2023_reverse_chip_init(void *dev_data, int mode, bool enable)
{
	struct cps2023_device_info *di = dev_data;
	int ret;
	int retry = 5; /* retry 5 times */
	int vbat = 0;

	if (!di || ((mode != CPS2023_CHG_R12SC_MODE) && (mode != CPS2023_CHG_R14SC_MODE)))
		return -EPERM;

	power_supply_get_int_prop(POWER_PLATFORM_BAT_PSY_NAME,
		POWER_SUPPLY_PROP_VOLTAGE_NOW, &vbat,
		POWER_SUPPLY_DEFAULT_VOLTAGE_NOW, POWER_UV_PER_MV);
	hwlog_info("vbat=%d\n", vbat);
	if (mode == CPS2023_CHG_R14SC_MODE) {
		ret = cps2023_config_switching_frequency(CPS2023_SW_FREQ_600KHZ, di);
		ret += cps2023_reverse14_set_threshold(enable, di);
	} else {
		ret = cps2023_reverse12_set_threshold(enable, di);
	}
	if (ret)
		return ret;

	if (enable) {
		do {
			cps2023_reverse_mode_enable(true, mode, di);
			if (!cps2023_is_device_close(dev_data)) {
				hwlog_info("set rvs cp open succ\n");
				return 0;
			}
		} while (retry-- > 0);
		hwlog_err("set rvs cp open failed\n");
		return -EPERM;
	}
	ret = cps2023_reverse_mode_enable(false, mode, di);
	return ret;
}

static int cps2023_enable_wired_reverse_charge(bool enable, void *dev_data)
{
	struct cps2023_device_info *di = dev_data;
	int ret;

	if (!di)
		return -EPERM;

	ret = cps2023_reverse12_set_threshold(enable, di);
	if (ret)
		return ret;

	return cps2023_wired_reverse_mode_enable(enable, di);
}

static int cps2023_reverse_cp_chip_init(void *dev_data, bool enable)
{
	return cps2023_reverse_chip_init(dev_data, CPS2023_CHG_R12SC_MODE, enable);
}

static int cps2023_reverse_cp4_chip_init(void *dev_data, bool enable)
{
	return cps2023_reverse_chip_init(dev_data, CPS2023_CHG_R14SC_MODE, enable);
}

static struct dc_ic_ops cps2023_lvc_ops = {
	.dev_name = "cps2023",
	.ic_init = cps2023_lvc_charge_init,
	.ic_exit = cps2023_charge_exit,
	.ic_enable = cps2023_lvc_charge_enable,
	.ic_discharge = cps2023_discharge,
	.is_ic_close = cps2023_is_device_close,
	.get_ic_id = cps2023_get_device_id,
	.config_ic_watchdog = cps2023_config_watchdog_ms,
	.kick_ic_watchdog = cps2023_kick_watchdog_ms,
	.ic_reg_reset_and_init = cps2023_reg_and_threshold_init,
	.set_ic_thld = cps2023_lvc_set_threshold,
};

static struct dc_ic_ops cps2023_sc_ops = {
	.dev_name = "cps2023",
	.ic_init = cps2023_sc_charge_init,
	.ic_exit = cps2023_charge_exit,
	.ic_enable = cps2023_sc_charge_enable,
	.ic_discharge = cps2023_discharge,
	.is_ic_close = cps2023_is_device_close,
	.get_ic_id = cps2023_get_device_id,
	.config_ic_watchdog = cps2023_config_watchdog_ms,
	.kick_ic_watchdog = cps2023_kick_watchdog_ms,
	.ic_reg_reset_and_init = cps2023_reg_and_threshold_init,
	.set_ic_thld = cps2023_sc_set_threshold,
};

static struct dc_ic_ops cps2023_sc4_ops = {
	.dev_name = "cps2023",
	.ic_init = cps2023_sc4_charge_init,
	.ic_exit = cps2023_charge_exit,
	.ic_enable = cps2023_sc4_charge_enable,
	.ic_discharge = cps2023_discharge,
	.is_ic_close = cps2023_is_device_close,
	.get_ic_id = cps2023_get_device_id,
	.config_ic_watchdog = cps2023_config_watchdog_ms,
	.kick_ic_watchdog = cps2023_kick_watchdog_ms,
	.ic_reg_reset_and_init = cps2023_reg_and_threshold_init,
	.set_ic_thld = cps2023_sc4_set_threshold,
};

static struct dc_batinfo_ops cps2023_batinfo_ops = {
	.init = cps2023_batinfo_init,
	.exit = cps2023_batinfo_exit,
	.get_bat_btb_voltage = cps2023_get_vbat_mv,
	.get_bat_package_voltage = cps2023_get_vbat_mv,
	.get_vbus_voltage = cps2023_get_vbus_mv,
	.get_bat_current = cps2023_get_ibat_ma,
	.get_ic_ibus = cps2023_get_ibus_ma,
	.get_ic_temp = cps2023_get_device_temp,
	.get_ic_vout = cps2023_get_vout_mv,
	.get_ic_vusb = cps2023_get_vusb_mv,
};

static struct charge_pump_ops cps2023_cp_ops = {
	.cp_type = CP_TYPE_MAIN,
	.chip_name = "cps2023",
	.dev_check = cps2023_cp_device_check,
	.post_probe = cps2023_cp_post_probe,
	.rvs_cp_chip_init = cps2023_reverse_cp_chip_init,
	.rvs_cp4_chip_init = cps2023_reverse_cp4_chip_init,
};

static struct rchg_pd_ic_ops cps2023_wired_rc_ops = {
	.dev_name = "cps2023",
	.enable_reverse_charge = cps2023_enable_wired_reverse_charge,
};

static struct power_log_ops cps2023_log_ops = {
	.dev_name = "cps2023",
	.dump_log_head = cps2023_get_register_head,
	.dump_log_content = cps2023_value_dump,
};

static void cps2023_init_ops_dev_data(struct cps2023_device_info *di)
{
	memcpy(&di->lvc_ops, &cps2023_lvc_ops, sizeof(struct dc_ic_ops));
	di->lvc_ops.dev_data = (void *)di;
	memcpy(&di->sc_ops, &cps2023_sc_ops, sizeof(struct dc_ic_ops));
	di->sc_ops.dev_data = (void *)di;
	memcpy(&di->sc4_ops, &cps2023_sc4_ops, sizeof(struct dc_ic_ops));
	di->sc4_ops.dev_data = (void *)di;
	memcpy(&di->batinfo_ops, &cps2023_batinfo_ops, sizeof(struct dc_batinfo_ops));
	di->batinfo_ops.dev_data = (void *)di;
	memcpy(&di->log_ops, &cps2023_log_ops, sizeof(struct power_log_ops));
	di->log_ops.dev_data = (void *)di;

	if (!di->ic_role) {
		snprintf(di->name, CHIP_DEV_NAME_LEN, "cps2023");
	} else {
		snprintf(di->name, CHIP_DEV_NAME_LEN, "cps2023_%u", di->ic_role);
		di->lvc_ops.dev_name = di->name;
		di->sc_ops.dev_name = di->name;
		di->sc4_ops.dev_name = di->name;
		di->log_ops.dev_name = di->name;
	}
}

static void cps2023_ops_register(struct cps2023_device_info *di)
{
	int ret;

	cps2023_init_ops_dev_data(di);

	ret = dc_ic_ops_register(LVC_MODE, di->ic_role, &di->lvc_ops);
	ret += dc_ic_ops_register(SC_MODE, di->ic_role, &di->sc_ops);
	ret += dc_ic_ops_register(SC4_MODE, di->ic_role, &di->sc4_ops);
	ret += dc_batinfo_ops_register(di->ic_role, &di->batinfo_ops, di->device_id);
	if (ret)
		hwlog_err("ic_%u sysinfo ops register failed\n", di->ic_role);

	ret = cps2023_protocol_ops_register(di);
	if (ret)
		hwlog_err("ic_%u scp or fcp ops register failed\n", di->ic_role);

	cps2023_power_sw_register(di);

	power_log_ops_register(&di->log_ops);
	if (di->rvs_cp_support) {
		cps2023_cp_ops.dev_data = (void *)di;
		(void)charge_pump_ops_register(&cps2023_cp_ops);
	}

	if (di->wired_rvs_support) {
		cps2023_wired_rc_ops.dev_data = (void *)di;
		(void)rchg_pd_ops_register(&cps2023_wired_rc_ops);
	}
}

static void cps2023_fault_event_notify(unsigned long event, void *data)
{
	power_event_anc_notify(POWER_ANT_DC_FAULT, event, data);
}

static void cps2023_vusb_ovp_interrupt_handle(struct cps2023_device_info *di, struct nty_data *data)
{
	hwlog_info("%s ic_%u USB OVP happened\n", __func__, di->ic_role);
	cps2023_fault_event_notify(POWER_NE_DC_FAULT_AC_OVP, data);
}

static void cps2023_vbat_ovp_interrupt_handle(struct cps2023_device_info *di, struct nty_data *data)
{
	int val = 0;

	val = cps2023_get_vbat_mv(di);
	hwlog_info("%s ic_%u BAT OVP happened, vbat=%d\n", __func__, di->ic_role, val);
	if (val >= CPS2023_VBAT_OVP_INIT)
		cps2023_fault_event_notify(POWER_NE_DC_FAULT_VBAT_OVP, data);
}

static void cps2023_ibat_ocp_interrupt_handle(struct cps2023_device_info *di, struct nty_data *data)
{
	int val = 0;

	cps2023_get_ibat_ma(&val, di);
	hwlog_info("%s ic_%u BAT OCP happened, ibat=%d\n", __func__, di->ic_role, val);
	if (((val >= CPS2023_IBAT_OCP_F41SC_INIT) && (di->charge_mode == CPS2023_CHG_F41SC_MODE)) ||
		((val >= CPS2023_IBAT_OCP_F21SC_INIT) && (di->charge_mode == CPS2023_CHG_F21SC_MODE)) ||
		((val >= CPS2023_IBAT_OCP_FBPSC_INIT) && (di->charge_mode == CPS2023_CHG_FBYPASS_MODE)))
		cps2023_fault_event_notify(POWER_NE_DC_FAULT_IBAT_OCP, data);
}

static void cps2023_vbus_ovp_interrupt_handle(struct cps2023_device_info *di, struct nty_data *data)
{
	int vbus_val = 0;

	cps2023_get_vbus_mv(&vbus_val, di);
	hwlog_info("%s ic_%u BUS OVP happened, vbus=%d\n", __func__, di->ic_role, vbus_val);
	if (((vbus_val >= CPS2023_VBUS_OVP_F41SC_INIT) && (di->charge_mode == CPS2023_CHG_F41SC_MODE)) ||
		((vbus_val >= CPS2023_VBUS_OVP_F21SC_INIT) && (di->charge_mode == CPS2023_CHG_F21SC_MODE)) ||
		((vbus_val >= CPS2023_VBUS_OVP_FBPSC_INIT) && (di->charge_mode == CPS2023_CHG_FBYPASS_MODE)))
		cps2023_fault_event_notify(POWER_NE_DC_FAULT_VBUS_OVP, data);
}

static void cps2023_ibus_ocp_interrupt_handle(struct cps2023_device_info *di, struct nty_data *data)
{
	int ibus_val = 0;

	cps2023_get_ibus_ma(&ibus_val, di);
	hwlog_info("%s ic_%u BUS OCP happened, ibus=%d\n", __func__, di->ic_role, ibus_val);
	if (((ibus_val >= CPS2023_IBUS_OCP_F41SC_INIT) && (di->charge_mode == CPS2023_CHG_F41SC_MODE)) ||
		((ibus_val >= CPS2023_IBUS_OCP_F21SC_INIT) && (di->charge_mode == CPS2023_CHG_F21SC_MODE)) ||
		((ibus_val >= CPS2023_IBUS_OCP_FBPSC_INIT) && (di->charge_mode == CPS2023_CHG_FBYPASS_MODE)))
		cps2023_fault_event_notify(POWER_NE_DC_FAULT_IBUS_OCP, data);
}

static void cps2023_kick_watchdog_work(struct work_struct *work)
{
	struct cps2023_device_info *di = NULL;

	di = container_of(work, struct cps2023_device_info, kick_wtd_work.work);
	if (!di) {
		hwlog_err("di is null\n");
		return;
	}

	cps2023_kick_watchdog_ms(di);
	schedule_delayed_work(&di->kick_wtd_work, msecs_to_jiffies(di->kick_ic_wtd_timer));
}

static void cps2023_interrupt_handle(struct cps2023_device_info *di,
	struct nty_data *data, u8 *flag)
{
	if (flag[CPS2023_IRQ_VUSB_OVP] & CPS2023_VUSB_OVP_FLAG_MASK) {
		cps2023_vusb_ovp_interrupt_handle(di, data);
	} else if (flag[CPS2023_IRQ_VBAT_OVP] & CPS2023_VBAT_OVP_FLAG_MASK) {
		cps2023_vbat_ovp_interrupt_handle(di, data);
	} else if (flag[CPS2023_IRQ_IBAT_OCP] & CPS2023_IBAT_OCP_FLAG_MASK) {
		cps2023_ibat_ocp_interrupt_handle(di, data);
	} else if (flag[CPS2023_IRQ_FLT_FLAG] & CPS2023_VBUS_OVP_FLAG_MASK) {
		cps2023_vbus_ovp_interrupt_handle(di, data);
	} else if (flag[CPS2023_IRQ_IBUS_OCP] & CPS2023_IBUS_OCP_FLAG_MASK) {
		cps2023_ibus_ocp_interrupt_handle(di, data);
	} else if (flag[CPS2023_IRQ_FLT_FLAG] & CPS2023_VOUT_OVP_FLAG_MASK) {
		hwlog_info("%s ic_%u VOUT OVP happened\n", __func__, di->ic_role);
	}
}

static void cps2023_interrupt_work(struct work_struct *work)
{
	u8 flag[CPS2023_IRQ_END] = { 0 };
	struct cps2023_device_info *di = NULL;
	struct nty_data *data = NULL;

	if (!work)
		return;

	di = container_of(work, struct cps2023_device_info, irq_work);
	if (!di || !di->client) {
		hwlog_err("di is null\n");
		return;
	}

	(void)cps2023_read_byte(di, CPS2023_VBAT_OVP_REG, &flag[CPS2023_IRQ_VBAT_OVP]);
	(void)cps2023_read_byte(di, CPS2023_FLT_FLAG_REG, &flag[CPS2023_IRQ_FLT_FLAG]);
	(void)cps2023_read_byte(di, CPS2023_VUSB_OVP_REG, &flag[CPS2023_IRQ_VUSB_OVP]);
	(void)cps2023_read_byte(di, CPS2023_IBUS_OCP_REG, &flag[CPS2023_IRQ_IBUS_OCP]);
	(void)cps2023_read_byte(di, CPS2023_IBAT_OCP_REG, &flag[CPS2023_IRQ_IBAT_OCP]);
	(void)cps2023_read_byte(di, CPS2023_INT_FLAG_REG, &flag[CPS2023_IRQ_INT_FLAG]);

	data = &(di->nty_data);
	data->addr = di->client->addr;

	if (di->int_notify_enable_flag) {
		cps2023_interrupt_handle(di, data, flag);
		cps2023_dump_register(di);
	}

	hwlog_info("ic_%u FLAG_VBAT_OVP[0x%x]=0x%x FLAG_FLT_FLAG[0x%x]=0x%x FLAG_VUSB_OVP[0x%x]=0x%x " \
		"FLAG_IBUS_OCP[0x%x]=0x%x FLAG_IBAT_OCP[0x%x]=0x%x FLAG_INT_FLAG[0x%x]=0x%x\n", \
		di->ic_role,
		CPS2023_VBAT_OVP_REG, flag[CPS2023_IRQ_VBAT_OVP],
		CPS2023_FLT_FLAG_REG, flag[CPS2023_IRQ_FLT_FLAG],
		CPS2023_VUSB_OVP_REG, flag[CPS2023_IRQ_VUSB_OVP],
		CPS2023_IBUS_OCP_REG, flag[CPS2023_IRQ_IBUS_OCP],
		CPS2023_IBAT_OCP_REG, flag[CPS2023_IRQ_IBAT_OCP],
		CPS2023_INT_FLAG_REG, flag[CPS2023_IRQ_INT_FLAG]);

	enable_irq(di->irq_int);
}

static irqreturn_t cps2023_interrupt(int irq, void *_di)
{
	struct cps2023_device_info *di = _di;

	if (!di)
		return IRQ_HANDLED;

	if (di->init_finish_flag)
		di->int_notify_enable_flag = true;

	hwlog_debug("ic_%u int happened\n", di->ic_role);
	disable_irq_nosync(di->irq_int);
	queue_work(di->int_wq, &di->irq_work);

	return IRQ_HANDLED;
}

static int cps2023_irq_init(struct cps2023_device_info *di,
	struct device_node *np)
{
	int ret;

	di->int_wq = create_singlethread_workqueue("cps2023_int_irq");
	INIT_WORK(&di->irq_work, cps2023_interrupt_work);

	ret = power_gpio_config_interrupt(np,
		"gpio_int", "cps2023_gpio_int", &di->gpio_int, &di->irq_int);
	if (ret)
		return ret;

	ret = request_irq(di->irq_int, cps2023_interrupt,
		IRQF_TRIGGER_FALLING, "cps2023_int_irq", di);
	if (ret) {
		hwlog_err("gpio irq request failed\n");
		di->irq_int = -1;
		gpio_free(di->gpio_int);
		return ret;
	}

	enable_irq_wake(di->irq_int);

	return 0;
}

static int cps2023_gpio_init(struct cps2023_device_info *di,
	struct device_node *np)
{
	if (of_find_property(np, "gpio_reset", NULL) &&
		power_gpio_config_output(np, "gpio_reset", "cps2023_gpio_reset",
		&di->gpio_reset, CPS2023_SWITCHCAP_DISABLE))
		return -EPERM;

	if (of_find_property(np, "gpio_enable", NULL) &&
		power_gpio_config_output(np, "gpio_enable", "cps2023_gpio_enable",
		&di->gpio_enable, CPS2023_GPIO_ENABLE)) {
		if (di->gpio_reset > 0)
			gpio_free(di->gpio_reset);
		return -EPERM;
	}

	/* To avoid entering the low-power mode, pull up LPM pin first. */
	if (of_find_property(np, "gpio_lpm", NULL) &&
		power_gpio_config_output(np, "gpio_lpm", "cps2023_gpio_lpm",
		&di->gpio_lpm, CPS2023_GPIO_ENABLE)) {
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

static void cps2023_parse_dts(struct device_node *np,
	struct cps2023_device_info *di)
{
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"switching_frequency", &di->switching_frequency,
		CPS2023_SW_FREQ_750KHZ);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"watchdog_time", &di->watchdog_time, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "scp_support",
		(u32 *)&(di->dts_scp_support), 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "fcp_support",
		(u32 *)&(di->dts_fcp_support), 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "ufcs_support",
		(u32 *)&(di->dts_ufcs_support), 0);
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
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"vout_ovp_mv", &di->vout_ovp_mv, CPS2023_VOUT_OVP_INIT);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"reboot_notifier_en", &di->reboot_notifier_en, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"kick_ic_wtd_timer", &di->kick_ic_wtd_timer, CPS2023_WD_TMR_TIMING_DIS);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"switch_auto_manual_mode_en", &di->switch_auto_manual_mode_en, 0);
	cps2023_parse_sw_attr(np, di);
}

static int cps2023_parse_mode_para_dts(struct device_node *np,
	struct cps2023_mode_para *data, const char *name, int id)
{
	int array_len, col, row, idata;
	int index = -1; /* -1 : illegal value */
	const char *device_name = dc_get_device_name_without_mode(id);
	const char *tmp_string = NULL;

	array_len = power_dts_read_count_strings(power_dts_tag(HWLOG_TAG), np,
		name, CPS2023_COMP_MAX_NUM, CPS2023_INFO_TOTAL);
	if (array_len < 0)
		return -EPERM;

	for (row = 0; row < array_len / CPS2023_INFO_TOTAL; row++) {
		col = row * CPS2023_INFO_TOTAL + CPS2023_INFO_IC_NAME;
		if (power_dts_read_string_index(power_dts_tag(HWLOG_TAG),
			np, name, col, &tmp_string))
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
		col = index * CPS2023_INFO_TOTAL + CPS2023_INFO_MAX_IBAT;
		power_dts_read_string_index(power_dts_tag(HWLOG_TAG),
			np, name, col, &tmp_string);
		kstrtoint(tmp_string, POWER_BASE_DEC, &idata);
		data->ibat_ocp = idata;
		col = index * CPS2023_INFO_TOTAL + CPS2023_INFO_IBUS_OCP;
		power_dts_read_string_index(power_dts_tag(HWLOG_TAG),
			np, name, col, &tmp_string);
		kstrtoint(tmp_string, POWER_BASE_DEC, &idata);
		data->ibus_ocp = idata;
	}

	return 0;
}

static void cps2023_parse_diff_para_dts(struct device_node *np,
	struct cps2023_device_info *di)
{
	if (cps2023_parse_mode_para_dts(np, &di->cps2023_lvc_para,
		"lvc_para", di->device_id)) {
		di->cps2023_lvc_para.ibat_ocp = CPS2023_IBAT_OCP_FBPSC_INIT;
		di->cps2023_lvc_para.ibus_ocp = CPS2023_IBUS_OCP_FBPSC_INIT;
	}
	if (cps2023_parse_mode_para_dts(np, &di->cps2023_sc_para,
		"sc_para", di->device_id)) {
		di->cps2023_sc_para.ibat_ocp = CPS2023_IBAT_OCP_F21SC_INIT;
		di->cps2023_sc_para.ibus_ocp = CPS2023_IBUS_OCP_F21SC_INIT;
	}
	if (cps2023_parse_mode_para_dts(np, &di->cps2023_sc4_para,
		"sc4_para", di->device_id)) {
		di->cps2023_sc4_para.ibat_ocp = CPS2023_IBAT_OCP_F41SC_INIT;
		di->cps2023_sc4_para.ibus_ocp = CPS2023_IBUS_OCP_F41SC_INIT;
	}
}
static void cps2023_init_lock_mutex(struct cps2023_device_info *di)
{
	mutex_init(&di->scp_detect_lock);
	mutex_init(&di->accp_adapter_reg_lock);
}

static void cps2023_destroy_lock_mutex(struct cps2023_device_info *di)
{
	mutex_destroy(&di->scp_detect_lock);
	mutex_destroy(&di->accp_adapter_reg_lock);
}

static void cps2023_ic_reset_in_shutdown(struct cps2023_device_info *di)
{
	cps2023_reg_reset(di);
	cps2023_write_mask(di, CPS2023_CTRL1_REG, CPS2023_CTRL1_ACDRV_MANUAL_EN_MASK,
		CPS2023_CTRL1_ACDRV_MANUAL_EN_SHIFT, 0);

	if (di->kick_ic_wtd_timer)
		cancel_delayed_work(&di->kick_wtd_work);
}

static int cps2023_reboot_notifier_call(struct notifier_block *nb, unsigned long event, void *data)
{
	struct cps2023_device_info *di = container_of(nb, struct cps2023_device_info, reboot_nb);

	if (!di || !di->reboot_notifier_en)
		return NOTIFY_OK;

	cps2023_ic_reset_in_shutdown(di);
	di->is_reboot = true;
	return NOTIFY_OK;
}

static int cps2023_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int ret;
	struct cps2023_device_info *di = NULL;
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

	cps2023_parse_dts(np, di);
	cps2023_parse_diff_para_dts(np, di);

	ret = cps2023_gpio_init(di, np);
	if (ret)
		goto cps2023_fail_0;

	ret = cps2023_get_device_id(di);
	if (ret == CPS2023_DEVICE_ID_GET_FAIL)
		goto cps2023_fail_2;

	cps2023_init_lock_mutex(di);

	(void)power_pinctrl_config(di->dev, "pinctrl-names", 1); /* 1:pinctrl-names length */

	ret = cps2023_reg_reset_and_init(di);
	if (ret)
		goto cps2023_fail_1;

	ret = cps2023_irq_init(di, np);
	if (ret)
		goto cps2023_fail_1;

	di->reboot_nb.notifier_call = cps2023_reboot_notifier_call;
	register_reboot_notifier(&di->reboot_nb);

	cps2023_ops_register(di);
	i2c_set_clientdata(client, di);

	if (di->kick_ic_wtd_timer) {
		INIT_DELAYED_WORK(&di->kick_wtd_work, cps2023_kick_watchdog_work);
		cps2023_config_watchdog_ms(CPS2023_WD_TMR_TIMING_5000MS, di);
		schedule_delayed_work(&di->kick_wtd_work, msecs_to_jiffies(0));
	}

	return 0;

cps2023_fail_1:
	cps2023_destroy_lock_mutex(di);
cps2023_fail_2:
	if (di->gpio_lpm > 0)
		gpio_free(di->gpio_lpm);
	if (di->gpio_reset > 0)
		gpio_free(di->gpio_reset);
	if (di->gpio_enable > 0)
		gpio_free(di->gpio_enable);
cps2023_fail_0:
	di->chip_already_init = 0;
	devm_kfree(&client->dev, di);

	return ret;
}

static int cps2023_remove(struct i2c_client *client)
{
	struct cps2023_device_info *di = i2c_get_clientdata(client);

	if (!di)
		return -ENODEV;

	cps2023_reg_reset(di);
	unregister_reboot_notifier(&di->reboot_nb);

	if (di->irq_int)
		free_irq(di->irq_int, di);

	if (di->gpio_int)
		gpio_free(di->gpio_int);

	if (di->gpio_enable)
		gpio_free(di->gpio_enable);
	cps2023_destroy_lock_mutex(di);

	return 0;
}

static void cps2023_shutdown(struct i2c_client *client)
{
	struct cps2023_device_info *di = i2c_get_clientdata(client);

	if (!di || di->reboot_notifier_en)
		return;

	cps2023_ic_reset_in_shutdown(di);
}

#ifdef CONFIG_PM
static void set_auto_mode(struct cps2023_device_info *di)
{
	u8 array[2] = {0};
	int ret;
	int vbus_present;
	int mode;

	ret = cps2023_read_byte(di, CPS2023_INT_STAT_REG, &array[0]);
	ret += cps2023_read_byte(di, CPS2023_CTRL1_REG, &array[1]);
	if (ret)
		return;

	vbus_present = (array[0] & (CPS2023_VBUS_PRESENT_STAT_MASK | CPS2023_VWPC_INSERT_STAT_MASK |
		CPS2023_VUSB_INSERT_STAT_MASK)) >> CPS2023_VUSB_INSERT_STAT_SHIFT;
	mode = (array[1] & CPS2023_CTRL1_ACDRV_MANUAL_EN_MASK) >> CPS2023_CTRL1_ACDRV_MANUAL_EN_SHIFT;
	hwlog_info("%s reg[0x10] = 0x%x vbus_present = %d reg[0x0B] = 0x%x mode = %d\n",
		__func__, array[0], vbus_present, array[1], mode);

	if (!vbus_present && mode) {
		ret = cps2023_write_mask(di, CPS2023_CTRL1_REG, CPS2023_CTRL1_ACDRV_MANUAL_EN_MASK,
			CPS2023_CTRL1_ACDRV_MANUAL_EN_SHIFT, 0);
		if (ret)
			return;
		hwlog_info("write Auto mode succ\n");
	}
}

static int cps2023_i2c_suspend(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct cps2023_device_info *di = NULL;

	if (!client)
		return 0;

	di = i2c_get_clientdata(client);
	if (di)
		cps2023_enable_adc(0, (void *)di);

	di->i2c_is_working = false;
	if (di->kick_ic_wtd_timer) {
		cps2023_config_watchdog_ms(CPS2023_WD_TMR_TIMING_DIS, di);
		cancel_delayed_work(&di->kick_wtd_work);
	}

	hwlog_info("ic_%u %s\n", di->ic_role, __func__);

	if (di->switch_auto_manual_mode_en)
		set_auto_mode(di);
	return 0;
}

static int cps2023_i2c_resume(struct device *dev)
{
	return 0;
}

static void cps2023_i2c_complete(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct cps2023_device_info *di = NULL;
	int ret;

	if (!client)
		return;

	di = i2c_get_clientdata(client);
	if (!di)
		return;
	di->i2c_is_working = true;
	cps2023_enable_adc(1, (void *)di);
	if (di->kick_ic_wtd_timer) {
		cps2023_config_watchdog_ms(CPS2023_WD_TMR_TIMING_5000MS, di);
		schedule_delayed_work(&di->kick_wtd_work, msecs_to_jiffies(0));
	}

	hwlog_info("ic_%u %s\n", di->ic_role, __func__);

	if (!di->switch_auto_manual_mode_en)
		return;
	ret = cps2023_write_mask(di, CPS2023_CTRL1_REG, CPS2023_CTRL1_ACDRV_MANUAL_EN_MASK,
		CPS2023_CTRL1_ACDRV_MANUAL_EN_SHIFT, 1);
	if (ret)
		return;
	hwlog_info("write Manual mode succ\n");
}

static const struct dev_pm_ops cps2023_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(cps2023_i2c_suspend, cps2023_i2c_resume)
	.complete = cps2023_i2c_complete,
};
#define CPS2023_PM_OPS (&cps2023_pm_ops)
#else
#define CPS2023_PM_OPS (NULL)
#endif /* CONFIG_PM */

MODULE_DEVICE_TABLE(i2c, cps2023);
static const struct of_device_id cps2023_of_match[] = {
	{
		.compatible = "sc8562",
		.data = NULL,
	},
	{},
};

static const struct i2c_device_id cps2023_i2c_id[] = {
	{ "sc8562", 0 },
	{}
};

static struct i2c_driver cps2023_driver = {
	.probe = cps2023_probe,
	.remove = cps2023_remove,
	.shutdown = cps2023_shutdown,
	.id_table = cps2023_i2c_id,
	.driver = {
		.owner = THIS_MODULE,
		.name = "cps2023",
		.of_match_table = of_match_ptr(cps2023_of_match),
		.pm = CPS2023_PM_OPS,
	},
};

static int __init cps2023_init(void)
{
	return i2c_add_driver(&cps2023_driver);
}

static void __exit cps2023_exit(void)
{
	i2c_del_driver(&cps2023_driver);
}

module_init(cps2023_init);
module_exit(cps2023_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("cps2023 module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
