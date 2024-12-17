// SPDX-License-Identifier: GPL-2.0
/*
 * sy6513.c
 *
 * sy6513 driver
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

#include "sy6513.h"
#include "sy6513_i2c.h"
#include "sy6513_ovp_switch.h"
#include "sy6513_scp.h"
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
#include <huawei_platform/hwpower/common_module/power_platform_macro.h>

#define HWLOG_TAG sy6513
HWLOG_REGIST();

#define SY6513_REG_DUMP_MAX_NUM 0x1E
#define SY6513_REG_DUMP_BUF_LEN 4096

/* Switching Frequency 400kHz 500kHz 600kHz 750kHz 850kHz 1000kHz 300kHz 350kHz */
static int sy6513_switching_frequency_list[SY6513_SWITCHING_FREQUENCY_LEVEL_NUM] = {400, 500, 600, 750, 850, 1000, 300, 350};

static void sy6513_dump_register(struct sy6513_device_info *di)
{
	int ret;
	int i;
	int len = 0;
	char buff[SY6513_REG_DUMP_BUF_LEN] = { 0 };
	u8 value[SY6513_REG_DUMP_MAX_NUM] = { 0 };

	sy6513_read_block(di, SY6513_CHARGE_MODE_CONTROL1_REG, value, SY6513_REG_DUMP_MAX_NUM);
	for (i = 0; i < SY6513_REG_DUMP_MAX_NUM; i++) {
		ret = snprintf_s(buff + len, sizeof(buff) - len, sizeof(buff) - len - 1,
			"reg[0x%x]=0x%x ", SY6513_CHARGE_MODE_CONTROL1_REG + i, value[i]);
			if (ret < 0)
				return;
			len += ret;
	}
	hwlog_info("%s ic_%u %s\n", __func__, di->ic_role, buff);
}

static int sy6513_is_device_close(void *dev_data)
{
	u8 val = 0;
	struct sy6513_device_info *di = dev_data;

	if (!di)
		return -ENODEV;

	if (sy6513_read_byte(di, SY6513_CHARGE_MODE_CONTROL2_REG, &val))
		return -EINVAL;

	if (val & SY6513_CHARGE_ENABLE_MASK)
		return 0;

	return 1;
}

static bool sy6513_is_adc_disabled(struct sy6513_device_info *di)
{
	u8 reg = 0;
	int ret;

	ret = sy6513_read_byte(di, SY6513_ADC_CTRL_REG, &reg);
	if (ret || !(reg & SY6513_ADC_ENABLE_MASK))
		return true;

	return false;
}

static bool sy6513_is_ic_disabled(struct sy6513_device_info *di)
{
	u8 reg = 0;
	int ret;

	ret = sy6513_read_byte(di, SY6513_CHARGE_MODE_CONTROL2_REG, &reg);
	if (ret || !((reg & SY6513_CHARGE_ENABLE_MASK) && (reg & SY6513_CP_SWITCHING_STAT_MASK)))
		return true;

	return false;
}

static bool sy6513_judge_ic_disabled(struct sy6513_device_info *di)
{
	return (di->charge_mode != SY6513_CHG_FBYPASS_MODE) && sy6513_is_ic_disabled(di);
}

static int sy6513_get_device_id(void *dev_data)
{
	u8 chip_id = 0;
	struct sy6513_device_info *di = dev_data;

	if (!di)
		return SY6513_DEVICE_ID_GET_FAIL;

	if (di->get_id_time == SY6513_USED)
		return di->device_id;

	di->get_id_time = SY6513_USED;

	if (sy6513_read_byte(di, SY6513_DEVICE_ID_REG, &chip_id)) {
		di->get_id_time = SY6513_NOT_USED;
		return SY6513_DEVICE_ID_GET_FAIL;
	}

	if (chip_id == SY6513_DEVICE_ID) {
		di->device_id = SWITCHCAP_SY6513;
	} else {
		di->device_id = SY6513_DEVICE_ID_GET_FAIL;
	}

	hwlog_info("device_id: 0x%x\n", di->device_id);

	return di->device_id;
}

static int sy6513_get_vbat_mv(void *dev_data)
{
	s16 data = 0;
	int vbat;
	struct sy6513_device_info *di = dev_data;

	if (!di)
		return -ENODEV;

	if (sy6513_is_adc_disabled(di))
		return 0;

	if (sy6513_read_word(di, SY6513_VBAT1_ADC1_REG, &data))
		return -EINVAL;

	/* VBAT ADC LBS: 0.3125mV */
	vbat = (int)data * 3125 / 10000;

	hwlog_info("ic_%u VBAT_ADC=0x%x, vbat=%d\n", di->ic_role, data, vbat);

	return vbat;
}

static int sy6513_get_ibat_ma(int *ibat, void *dev_data)
{
	u16 data = 0;
	int ibat_ori;
	struct sy6513_device_info *di = dev_data;

	if (!ibat || !di)
		return -ENODEV;

	if (sy6513_is_adc_disabled(di)) {
		*ibat = 0;
		return 0;
	}

	if (sy6513_read_word(di, SY6513_IBAT_ADC1_REG, &data))
		return -EINVAL;

	/* IBAT ADC LBS: 1.25mA */
	ibat_ori = (int)data * 125 / 100;
	if ((data >> IBAT_IBUS_SYMBOL_BIT) & 0x1)
		ibat_ori = -ibat_ori;

	*ibat = ibat_ori * di->sense_r_config;
	*ibat /= di->sense_r_actual;

	hwlog_info("ic_%u IBAT_ADC=0x%x ibat_ori=%d ibat=%d\n", di->ic_role, data, ibat_ori, *ibat);

	return 0;
}

static int sy6513_get_ibus_ma(int *ibus, void *dev_data)
{
	u16 data = 0;
	int ret;
	struct sy6513_device_info *di = dev_data;

	if (!di || !ibus)
		return -ENODEV;

	if (sy6513_judge_ic_disabled(di) || sy6513_is_adc_disabled(di)) {
		*ibus = 0;
		return 0;
	}
	ret = sy6513_read_word(di, SY6513_IBUS_ADC1_REG, &data);
	if (ret)
		return ret;

	/* Bit7 is sign bit; ibus ADC LBS: 1.25mA */
	*ibus = (int)data * 125 / 100;
	if ((data >> IBAT_IBUS_SYMBOL_BIT) & 0x1)
		*ibus = -*ibus;

	hwlog_info("ic_%u IBUS_ADC=0x%x, ibus=%d\n", di->ic_role, data, *ibus);

	return 0;
}

int sy6513_get_vbus_mv(int *vbus, void *dev_data)
{
	s16 data = 0;
	struct sy6513_device_info *di = dev_data;

	if (!di || !vbus)
		return -ENODEV;

	if (sy6513_is_adc_disabled(di)) {
		*vbus = 0;
		return 0;
	}

	if (sy6513_read_word(di, SY6513_VBUS_ADC1_REG, &data))
		return -EINVAL;

	/* VBUS ADC LBS: 1.5mV */
	*vbus = (int)data * 15 / 10;

	hwlog_info("ic_%u VBUS_ADC=0x%x, vbus=%d\n", di->ic_role, data, *vbus);

	return 0;
}

static int sy6513_get_vusb_mv(int *vusb, void *dev_data)
{
	s16 data = 0;
	struct sy6513_device_info *di = dev_data;

	if (!vusb || !di)
		return -ENODEV;

	if (sy6513_is_adc_disabled(di)) {
		*vusb = 0;
		return 0;
	}

	if (sy6513_read_word(di, SY6513_VUSB_ADC1_REG, &data))
		return -EINVAL;

	/* VUSB_ADC LSB:  1.5mV */
	*vusb = (int)data * 15 / 10;

	hwlog_info("ic_%u VUSB_ADC=0x%x, vusb=%d\n", di->ic_role, data, *vusb);

	return 0;
}

static int sy6513_get_device_temp(int *temp, void *dev_data)
{
	u8 data = 0;
	struct sy6513_device_info *di = dev_data;

	if (!temp || !di)
		return -ENODEV;

	if (sy6513_is_adc_disabled(di)) {
		*temp = 0;
		return 0;
	}

	if (sy6513_read_byte(di, SY6513_TDIE_REG, &data))
		return -EINVAL;

	/* TDIE_ADC = REG[7:0] - 40℃ */
	*temp = (int)data - 40;

	hwlog_info("ic_%u TDIE_ADC=0x%x temp=%d\n", di->ic_role, data, *temp);

	return 0;
}

static int sy6513_get_vout_mv(int *vout, void *dev_data)
{
	s16 data = 0;
	struct sy6513_device_info *di = dev_data;

	if (!vout || !di)
		return -ENODEV;

	if (sy6513_is_adc_disabled(di)) {
		*vout = 0;
		return 0;
	}

	if (sy6513_read_word(di, SY6513_VOUT_ADC1_REG, &data))
		return -EINVAL;

	/* VOUT_ADC LSB:  0.3125mV */
	*vout = (int)data * 3125 / 10000;

	hwlog_info("ic_%u VOUT_ADC=0x%x, vout=%d\n", di->ic_role, data, *vout);

	return 0;
}

static int sy6513_disable_watchdog(struct sy6513_device_info *di)
{
	u8 reg;
	int ret;

	ret = sy6513_write_mask(di, SY6513_CHARGE_MODE_CONTROL1_REG, SY6513_WATCHDOG_ENABLE_MASK,
		SY6513_WATCHDOG_ENABLE_SHIFT, SY6513_WATCHDOG_DISABLE);
	ret += sy6513_read_byte(di, SY6513_CHARGE_MODE_CONTROL1_REG, &reg);
	if (ret)
		return -EINVAL;

	hwlog_info("ic_%u config_watchdog_ms [%x]=0x%x\n", di->ic_role, SY6513_CHARGE_MODE_CONTROL1_REG, reg);
	return 0;
}

static int sy6513_config_watchdog_ms(int time, void *dev_data)
{
	u8 val;
	u8 reg;
	int ret;
	struct sy6513_device_info *di = dev_data;

	if (!di)
		return -ENODEV;

	if (di->watchdog_time && time > 0)
		time = (time > di->watchdog_time) ? time : di->watchdog_time;

	if (time < SY6513_WATCHDOG_TIMER_500MS)
		return sy6513_disable_watchdog(di);

	if (time >= SY6513_WATCHDOG_TIMER_5000MS) {
		val = SY6513_WATCHDOG_TIMER_5S;
	} else if (time >= SY6513_WATCHDOG_TIMER_2000MS) {
		val = SY6513_WATCHDOG_TIMER_2S;
	} else if (time >= SY6513_WATCHDOG_TIMER_1000MS) {
		val = SY6513_WATCHDOG_TIMER_1S;
	} else {
		val = SY6513_WATCHDOG_TIMER_0P5S;
	}

	ret = sy6513_write_mask(di, SY6513_CHARGE_MODE_CONTROL1_REG, SY6513_WATCHDOG_ENABLE_MASK,
		SY6513_WATCHDOG_ENABLE_SHIFT, SY6513_WATCHDOG_ENABLE);
	ret += sy6513_write_mask(di, SY6513_CHARGE_MODE_CONTROL1_REG, SY6513_WATCHDOG_TIMER_SET_MASK,
		SY6513_WATCHDOG_TIMER_SET_SHIFT, val);
	ret += sy6513_read_byte(di, SY6513_CHARGE_MODE_CONTROL1_REG, &reg);
	if (ret)
		return -EINVAL;

	hwlog_info("ic_%u config_watchdog_ms [%x]=0x%x\n", di->ic_role, SY6513_CHARGE_MODE_CONTROL1_REG, reg);
	return 0;
}

static int sy6513_en_cfg(struct sy6513_device_info *di, int enable)
{
	int ret;
	u8 chg_en_reg = 0;

	ret = gpio_direction_output(di->gpio_enable, enable);

	ret += sy6513_write_mask(di, SY6513_CHARGE_MODE_CONTROL2_REG, SY6513_CHARGE_ENABLE_MASK,
		SY6513_CHARGE_ENABLE_SHIFT, enable);
	ret += sy6513_read_byte(di, SY6513_CHARGE_MODE_CONTROL2_REG, &chg_en_reg);
	if (ret)
		return ret;

	hwlog_info("ic_role=%d, charge_enable enable=%d,[0x%x]=0x%x\n", di->ic_role, enable,
		SY6513_CHARGE_MODE_CONTROL2_REG, chg_en_reg);

	return ret;
}

static int sy6513_config_ibat_sns_res(struct sy6513_device_info *di)
{
	u8 res_config;
	int ret;

	if (di->sense_r_config == SENSE_R_2_MOHM) {
		res_config = SY6513_RSENSE_2MOHM;
	} else if (di->sense_r_config == SENSE_R_1_MOHM) {
		res_config = SY6513_RSENSE_1MOHM;
	} else {
		res_config = SY6513_RSENSE_0P5MOHM;
	}

	ret = sy6513_write_mask(di, SY6513_IBAT_PROTECTION_REG, SY6513_PLACEMENT_RSENSE_MASK,
		SY6513_PLACEMENT_RSENSE_SHIFT, 0);
	ret += sy6513_write_mask(di, SY6513_IBAT_PROTECTION_REG, SY6513_RSENSE_MASK,
		SY6513_RSENSE_SHIFT, res_config);

	return ret;
}

static int sy6513_opt_regs_init(struct sy6513_device_info *di)
{
	u8 vdrop_min;
	u8 vdrop_ovp_max;
	int ret;

	ret = sy6513_write_mask(di, SY6513_USB_OVP_ENABLE_REG, SY6513_VUSB_OVP_RNG_MASK,
		SY6513_VUSB_OVP_RNG_SHIFT, SY6513_VUSB_OVP_23V);
	vdrop_min = (u8)((SY6513_VDROP_MIN_MAX - SY6513_VDROP_MIN_MIN) / SY6513_VDROP_MIN_STEP);
	ret += sy6513_write_mask(di, SY6513_SC_VDROP_OVP_REG, SY6513_VDROP_MIN_MASK,
		SY6513_VDROP_MIN_SHIFT, vdrop_min);
	vdrop_ovp_max = (u8)((SY6513_VDROP_OVP_MAX - SY6513_VDROP_OVP_MIN) / SY6513_VDROP_OVP_STEP);
	ret += sy6513_write_mask(di, SY6513_SC_VDROP_OVP_REG, SY6513_VDROP_OVP_MASK,
		SY6513_VDROP_OVP_SHIFT, vdrop_ovp_max);
	ret += sy6513_write_mask(di, SY6513_PROTECT_CONTROL2_REG, SY6513_IBUS_RCP_PEAK_ENABLE_MASK,
		SY6513_IBUS_RCP_PEAK_ENABLE_SHIFT, SY6513_IBUS_RCP_PEAK_DISABLE);
	ret += sy6513_write_mask(di, SY6513_PROTECT_CONTROL2_REG, SY6513_IBUS_UCP_ENABLE_MASK,
		SY6513_IBUS_UCP_ENABLE_SHIFT, SY6513_IBUS_UCP_DISABLE);
	ret += sy6513_write_mask(di, SY6513_WPC_OVP_ENABLE_REG, SY6513_VWPC_OVP_RNG_MASK,
		SY6513_VWPC_OVP_RNG_SHIFT, SY6513_VWPC_OVP_23V);
	ret += sy6513_write_mask(di, SY6513_SC_CONTROL_REG, SY6513_SWITCHING_FREQUENCY_MASK,
		SY6513_SWITCHING_FREQUENCY_SHIFT, SY6513_SWITCHING_FREQUENCY_500KHZ);
	ret += sy6513_config_ibat_sns_res(di);

	return ret;
}

static int sy6513_adc_channel_cfg(struct sy6513_device_info *di)
{
	int ret;

	ret = sy6513_write_mask(di, SY6513_ADC_CTRL_REG, SY6513_ADC_ENABLE_MASK,
		SY6513_ADC_ENABLE_SHIFT, SY6513_ADC_ENABLE);
	ret += sy6513_write_mask(di, SY6513_ADC_CTRL1_REG, SY6513_AVERAGE_TIMES_FOR_SAMPLING_OF_ADC_MASK,
		SY6513_AVERAGE_TIMES_FOR_SAMPLING_OF_ADC_SHIFT, SY6513_AVERAGE_TIMES_FOR_SAMPLING_OF_ADC_8);

	return ret;
}

static int sy6513_adc_enable(int enable, void *dev_data)
{
	int ret;
	struct sy6513_device_info *di = dev_data;

	if (!di)
		return -ENODEV;

	ret = sy6513_write_mask(di, SY6513_ADC_CTRL_REG, SY6513_ADC_ENABLE_MASK,
		SY6513_ADC_ENABLE_SHIFT, enable);
	hwlog_info("adc enable=%d ret=%d\n", enable, ret);

	return ret;
}

static int sy6513_charger_irq_clear(struct sy6513_device_info *di)
{
	int ret;
	int i;
	u8 irq_flag = 0;
	u8 val;

	for (i = 0; i < SY6513_IRQ_FLAG_REG_NUM;) {
		val = 0;
		ret = sy6513_read_byte(di, SY6513_FLT_FLAG1_REG + i, &irq_flag);
		ret += sy6513_read_byte(di, SY6513_FLT_FLAG1_REG + 1 + i, &val);
		if (val) {
			hwlog_info("%s, irq_flag 0x%x = 0x%x, irq_flag_mask 0x%x = 0x%x\n", __func__,
				SY6513_FLT_FLAG1_REG + i, irq_flag, SY6513_FLT_FLAG1_REG + 1 + i, val);
			ret += sy6513_write_byte(di, SY6513_FLT_FLAG1_REG + 1 + i, 0x0);
		}
		/* step = 2 */
		i += 2;
	}

	return ret;
}

static int sy6513_reg_init(struct sy6513_device_info *di)
{
	int ret;

	ret = sy6513_disable_watchdog(di);
	ret += sy6513_en_cfg(di, SY6513_CHARGE_DISABLE);
	ret += sy6513_opt_regs_init(di);
	ret += sy6513_adc_channel_cfg(di);
	ret += sy6513_adc_enable(SY6513_ADC_ENABLE, di);
	ret += sy6513_charger_irq_clear(di);
	ret += sy6513_write_mask(di, SY6513_BC12_CONTROL_REG, SY6513_CHGDET_ON_TIME_MASK,
		SY6513_CHGDET_ON_TIME_SHIFT, false);
	if (ret)
		hwlog_err("reg_init fail %d\n", ret);

	return ret;
}

static int sy6513_config_switching_frequency(int data, struct sy6513_device_info *di)
{
	u8 freq = 0;
	int ret;
	int i;

	if (data < SY6513_SWITCHING_FREQUENCY_MIN)
		data = SY6513_SWITCHING_FREQUENCY_MIN;

	if (data > SY6513_SWITCHING_FREQUENCY_MAX)
		data = SY6513_SWITCHING_FREQUENCY_MAX;

	for (i = 0; i < SY6513_SWITCHING_FREQUENCY_LEVEL_NUM; i++) {
		if (data >= sy6513_switching_frequency_list[i]) {
			freq = (u8)i;
			break;
		}
	}

	ret = sy6513_write_mask(di, SY6513_SC_CONTROL_REG, SY6513_SWITCHING_FREQUENCY_MASK,
		SY6513_SWITCHING_FREQUENCY_SHIFT, freq);
	if (ret)
		return -EINVAL;

	hwlog_info("sy6513 config_switching_frequency [%x]=0x%x\n", SY6513_SC_CONTROL_REG, freq);

	return 0;
}

static int sy6513_charge_init(void *dev_data)
{
	int ret;
	struct sy6513_device_info *di = dev_data;

	if (!di)
		return -ENODEV;

	ret = sy6513_reg_init(di);
	ret += sy6513_config_switching_frequency(SY6513_SWITCHING_FREQUENCY_500KHZ, di);
	if (ret)
		return -EINVAL;

	hwlog_info("ic_role=%d device_id=%d init\n", di->ic_role, di->device_id);

	di->init_finish_flag = SY6513_INIT_FINISH;

	return 0;
}

static int sy6513_usb_wpc_ovp_enable(void *dev_data, u8 mode)
{
	int ret;
	u8 usb_ovp_val = 0;
	u8 wpc_ovp_val = 0;
	u8 usb_ovp_mode = SY6513_USB_OVP_MODE_FORWARD;
	u8 wpc_ovp_mode  = SY6513_WPC_OVP_MODE_FORWARD;
	struct sy6513_device_info *di = dev_data;

	if (!di)
		return -ENODEV;

	usb_ovp_val = usb_ovp_vals[mode];
	wpc_ovp_val = wpc_ovp_vals[mode];

	if (mode >= SY6513_CHG_RBYPASS_MODE && mode <= SY6513_CHG_R14SC_MODE) {
		usb_ovp_mode = SY6513_USB_OVP_MODE_REVERSE;
		wpc_ovp_mode = SY6513_WPC_OVP_MODE_REVERSE;
	}

	ret = sy6513_write_mask(di, SY6513_CHARGE_MODE_CONTROL2_REG, SY6513_USB_OVP_MODE_MASK,
		SY6513_USB_OVP_MODE_SHIFT, usb_ovp_mode);
	ret += sy6513_write_mask(di, SY6513_CHARGE_MODE_CONTROL2_REG, SY6513_WPC_OVP_MODE_MASK,
		SY6513_WPC_OVP_MODE_SHIFT, wpc_ovp_mode);
	ret += sy6513_write_mask(di, SY6513_USB_OVP_ENABLE_REG, SY6513_VUSB_OVP_RNG_MASK,
		SY6513_VUSB_OVP_RNG_SHIFT, usb_ovp_val);
	ret += sy6513_write_mask(di, SY6513_WPC_OVP_ENABLE_REG, SY6513_VWPC_OVP_RNG_MASK,
		SY6513_VWPC_OVP_RNG_SHIFT, wpc_ovp_val);
	if (ret)
		return ret;

	return 0;
}

static int sy6513_charge_mode_enable(void *dev_data, u8 mode)
{
	int ret;
	u8 mode_reg;
	u8 chg_en_reg;

	struct sy6513_device_info *di = dev_data;

	if (!di)
		return -ENODEV;

	ret = sy6513_usb_wpc_ovp_enable(di, mode);
	ret += sy6513_write_mask(di, SY6513_CHARGE_MODE_CONTROL1_REG, SY6513_CHG_MODE_MASK,
		SY6513_CHG_MODE_SHIFT, mode);
	ret += sy6513_read_byte(di, SY6513_CHARGE_MODE_CONTROL1_REG, &mode_reg);
	ret += sy6513_read_byte(di, SY6513_CHARGE_MODE_CONTROL2_REG, &chg_en_reg);
	if (ret)
		return ret;

	di->charge_mode = mode;
	hwlog_info("ic_role=%d, charge_enable [0x%x]=0x%x,[0x%x]=0x%x\n", di->ic_role, SY6513_CHARGE_MODE_CONTROL1_REG,
		mode_reg, SY6513_CHARGE_MODE_CONTROL2_REG, chg_en_reg);

	return 0;
}

static int sy6513_lvc_charge_enable(int enable, void *dev_data)
{
	int ret;
	struct sy6513_device_info *di = dev_data;

	ret = sy6513_charge_mode_enable(di, SY6513_CHG_FBYPASS_MODE);
	ret += sy6513_en_cfg(di, enable);
	if (ret)
		return ret;

	hwlog_info("mode = %d, ic_role=%d, enable=%d ", SY6513_CHG_FBYPASS_MODE, di->ic_role, enable);

	return 0;
}

static int sy6513_sc_charge_enable(int enable, void *dev_data)
{
	int ret;
	struct sy6513_device_info *di = dev_data;

	ret = sy6513_charge_mode_enable(di, SY6513_CHG_F21SC_MODE);
	ret += sy6513_en_cfg(di, enable);
	if (ret)
		return ret;

	hwlog_info("mode = %d, ic_role=%d, enable=%d ", SY6513_CHG_F21SC_MODE, di->ic_role, enable);

	return 0;
}

static int sy6513_sc4_charge_enable(int enable, void *dev_data)
{
	int ret;
	struct sy6513_device_info *di = dev_data;

	ret = sy6513_charge_mode_enable(di, SY6513_CHG_F41SC_MODE);
	ret += sy6513_en_cfg(di, enable);
	if (ret)
		return ret;

	hwlog_info("mode = %d, ic_role=%d, enable=%d ", SY6513_CHG_F41SC_MODE, di->ic_role, enable);

	return 0;
}

static int sy6513_sc4_charge_exit(void *dev_data)
{
	int ret;
	struct sy6513_device_info *di = dev_data;

	if (!di)
		return -ENODEV;

	ret = sy6513_sc4_charge_enable(SY6513_CHARGE_DISABLE, di);
	di->fcp_support = false;
	di->init_finish_flag = 0;
	di->int_notify_enable_flag = 0;

	return ret;
}

static int sy6513_sc_charge_exit(void *dev_data)
{
	int ret;
	struct sy6513_device_info *di = dev_data;

	if (!di)
		return -ENODEV;

	ret = sy6513_sc_charge_enable(SY6513_CHARGE_DISABLE, di);
	di->fcp_support = false;
	di->init_finish_flag = 0;
	di->int_notify_enable_flag = 0;

	return ret;
}

static int sy6513_lvc_charge_exit(void *dev_data)
{
	int ret;
	struct sy6513_device_info *di = dev_data;

	if (!di)
		return -ENODEV;

	ret = sy6513_lvc_charge_enable(SY6513_CHARGE_DISABLE, di);
	di->fcp_support = false;
	di->init_finish_flag = 0;
	di->int_notify_enable_flag = 0;

	return ret;
}

static int sy6513_discharge(int enable, void *dev_data)
{
	return 0;
}

static int sy6513_kick_watchdog_ms(void *dev_data)
{
	return 0;
}

static int sy6513_config_vbat_ovp_threshold(struct sy6513_device_info *di, int ovp_th)
{
	u8 vbat;
	int ret;

	if (ovp_th < SY6513_FWD_VBAT1_OVP_MIN)
		ovp_th = SY6513_FWD_VBAT1_OVP_MIN;

	if (ovp_th > SY6513_FWD_VBAT1_OVP_MAX)
		ovp_th = SY6513_FWD_VBAT1_OVP_MAX;

	vbat = (u8)((ovp_th - SY6513_FWD_VBAT1_OVP_MIN) / SY6513_FWD_VBAT1_OVP_STEP);

	ret = sy6513_write_mask(di, SY6513_VBAT_PROTECTION_REG, SY6513_FWD_VBAT1_OVP_MASK,
		SY6513_FWD_VBAT1_OVP_SHIFT, vbat);
	ret += sy6513_read_byte(di, SY6513_VBAT_PROTECTION_REG, &vbat);
	if (ret)
		return ret;

	hwlog_info("config_vbat_ovp_threshold_mv [%x]=0x%x\n", SY6513_VBAT_PROTECTION_REG, vbat);

	return 0;
}

static int sy6513_config_vout_ovp_threshold(struct sy6513_device_info *di, int ovp_th)
{
	u8 vout;
	int ret;

	if (ovp_th < SY6513_VOUT_OVP_MIN)
		ovp_th = SY6513_VOUT_OVP_MIN;

	if (ovp_th > SY6513_VOUT_OVP_MAX)
		ovp_th = SY6513_VOUT_OVP_MAX;

	vout = (u8)((ovp_th - SY6513_VOUT_OVP_MIN) / SY6513_VOUT_OVP_STEP);
	ret = sy6513_write_mask(di, SY6513_VBUS_VOUT_OVP_REG, SY6513_VOUT_OVP_MASK,
		SY6513_VOUT_OVP_SHIFT, vout);
	vout = 0;
	ret += sy6513_read_byte(di, SY6513_VBUS_VOUT_OVP_REG, &vout);
	if (ret)
		return ret;

	hwlog_info("config_vout_ovp_threshold_mv [0x%x]=0x%x\n", SY6513_VBUS_VOUT_OVP_REG, vout);

	return 0;
}

int sy6513_config_vusb_ovp_th_mv(struct sy6513_device_info *di, int ovp_th)
{
	u8 value;
	int ret;

	ret = sy6513_write_mask(di, SY6513_USB_OVP_ENABLE_REG, SY6513_VUSB_OVP_RNG_MASK,
		SY6513_VUSB_OVP_RNG_SHIFT, ovp_th);
	ret += sy6513_read_byte(di, SY6513_USB_OVP_ENABLE_REG, &value);
	if (ret)
		return -EINVAL;

	hwlog_info("ic_%u config_vusb_ovp_threshold_mv [%x]=0x%x\n", di->ic_role, SY6513_USB_OVP_ENABLE_REG, value);

	return 0;
}

int sy6513_config_vbus_ovp_th_mv(struct sy6513_device_info *di, int ovp_th, int mode)
{
	u8 value;
	int ret;

	if (mode != SY6513_CHG_FBYPASS_MODE && mode != SY6513_CHG_F21SC_MODE && mode != SY6513_CHG_F41SC_MODE)
		return 0;

	if (mode == SY6513_CHG_FBYPASS_MODE) {
		ret = sy6513_write_mask(di, SY6513_VBUS_VOUT_OVP_REG, SY6513_BPS_VBUS_OVP_MASK,
			SY6513_BPS_VBUS_OVP_SHIFT, SY6513_VBUS_OVP_FBYPASS_5600MV);
	} else if (mode == SY6513_CHG_F21SC_MODE) {
		ret = sy6513_write_mask(di, SY6513_VBUS_VOUT_OVP_REG, SY6513_21SC_VBUS_OVP_MASK,
			SY6513_21SC_VBUS_OVP_SHIFT, SY6513_VBUS_OVP_F21SC_12000MV);
	} else {
		ret = sy6513_write_mask(di, SY6513_VBUS_VOUT_OVP_REG, SY6513_41SC_VBUS_OVP_MASK,
			SY6513_41SC_VBUS_OVP_SHIFT, SY6513_VBUS_OVP_F41SC_21000MV);
	}

	ret += sy6513_read_byte(di, SY6513_VBUS_VOUT_OVP_REG, &value);
	if (ret)
		return -EINVAL;

	hwlog_info("ic_%u config_vbus_ovp_threshole_mv [%x]=0x%x\n", di->ic_role, SY6513_VBUS_VOUT_OVP_REG, value);

	return 0;
}

static int sy6513_reg_reset(struct sy6513_device_info *di)
{
	int ret;
	u8 value;

	ret = sy6513_write_mask(di, SY6513_CHARGE_MODE_CONTROL1_REG, SY6513_ADC_ENABLE_MASK,
		SY6513_ADC_ENABLE_SHIFT, SY6513_ADC_ENABLE);
	ret += sy6513_write_multi_mask(di, SY6513_PULL_DOWN_CONFIG_REG,
		SY6513_OVP_G_EN_REVERSE_MODE_MASK, 0x1);
	ret += sy6513_read_byte(di, SY6513_PULL_DOWN_CONFIG_REG, &value);
	hwlog_info("ic_%u ovpgate_en_reg [%x]=0x%x\n", di->ic_role, SY6513_PULL_DOWN_CONFIG_REG, value);

	return gpio_direction_output(di->gpio_reset, 0);
}

static const struct sy6513_thre_configs g_thre_cfg_tbl[] = {
	{SY6513_CHG_FBYPASS_MODE, SY6513_VBUS_VOUT_OVP_REG, SY6513_BPS_VBUS_OVP_MASK, SY6513_BPS_VBUS_OVP_SHIFT, SY6513_VBUS_OVP_FBYPASS_5600MV},
	{SY6513_CHG_FBYPASS_MODE, SY6513_IBUS_PROTECTION_REG, SY6513_21SC_IBUS_OCP_MASK, SY6513_21SC_IBUS_OCP_SHIFT, SY6513_21SC_IBUS_OCP_6000MA},
	{SY6513_CHG_FBYPASS_MODE, SY6513_IBAT_PROTECTION_REG, SY6513_FWD_IBAT_OCP_MASK, SY6513_FWD_IBAT_OCP_SHIFT, SY6513_FWD_IBAT_OCP_8500MA},
	{SY6513_CHG_F21SC_MODE, SY6513_VBUS_VOUT_OVP_REG, SY6513_21SC_VBUS_OVP_MASK, SY6513_21SC_VBUS_OVP_SHIFT, SY6513_VBUS_OVP_F21SC_12000MV},
	{SY6513_CHG_F21SC_MODE, SY6513_IBUS_PROTECTION_REG, SY6513_21SC_IBUS_OCP_MASK, SY6513_21SC_IBUS_OCP_SHIFT, SY6513_21SC_IBUS_OCP_6000MA},
	{SY6513_CHG_F21SC_MODE, SY6513_IBAT_PROTECTION_REG, SY6513_FWD_IBAT_OCP_MASK, SY6513_FWD_IBAT_OCP_SHIFT, SY6513_FWD_IBAT_OCP_8500MA},
	{SY6513_CHG_F41SC_MODE, SY6513_VBUS_VOUT_OVP_REG, SY6513_41SC_VBUS_OVP_MASK, SY6513_41SC_VBUS_OVP_SHIFT, SY6513_VBUS_OVP_F41SC_21000MV},
	{SY6513_CHG_F41SC_MODE, SY6513_IBUS_PROTECTION_REG, SY6513_41SC_IBUS_OCP_MASK, SY6513_41SC_IBUS_OCP_SHIFT, SY6513_41SC_IBUS_OCP_3750MA},
	{SY6513_CHG_F41SC_MODE, SY6513_IBAT_PROTECTION_REG, SY6513_FWD_IBAT_OCP_MASK, SY6513_FWD_IBAT_OCP_SHIFT, SY6513_FWD_IBAT_OCP_8500MA},
	{SY6513_CHG_RBYPASS_MODE, SY6513_SC_VDROP_OVP_REG, SY6513_RBPS_IBUS_OCP_MASK, SY6513_RBPS_IBUS_OCP_SHIFT, SY6513_RBPS_IBUS_OCP_1A},
	{SY6513_CHG_R12SC_MODE, SY6513_IBUS_PROTECTION_REG, SY6513_12SC_IBUS_OCP_MASK, SY6513_12SC_IBUS_OCP_SHIFT, SY6513_12SC_IBUS_OCP_3000MA},
	{SY6513_CHG_R14SC_MODE, SY6513_IBUS_PROTECTION_REG, SY6513_14SC_IBUS_OCP_MASK, SY6513_14SC_IBUS_OCP_SHIFT, SY6513_14SC_IBUS_OCP_1500MA}
};

static int sy6513_threshold_reg_init(struct sy6513_device_info *di, u8 mode)
{
	int ret;
	int i;
	u8 vbus;
	u8 ibus;
	u8 ibat;
	u8 ribus;

	ret = sy6513_config_vout_ovp_threshold(di, SY6513_VOUT_OVP_INIT);
	ret += sy6513_config_vbat_ovp_threshold(di, SY6513_FWD_VBAT1_OVP_INIT);

	int num = ARRAY_SIZE(g_thre_cfg_tbl);
	for (i = 0; i < num; i++) {
		if (mode == g_thre_cfg_tbl[i].mode)
			ret += sy6513_write_mask(di, g_thre_cfg_tbl[i].reg, g_thre_cfg_tbl[i].mask,
				g_thre_cfg_tbl[i].shift, g_thre_cfg_tbl[i].value);
	}

	ret += sy6513_charge_mode_enable(di, mode);
	if (ret)
		hwlog_err("ic_%u protect threshold init failed\n", di->ic_role);

	ret += sy6513_read_byte(di, SY6513_VBUS_VOUT_OVP_REG, &vbus);
	ret += sy6513_read_byte(di, SY6513_IBUS_PROTECTION_REG, &ibus);
	ret += sy6513_read_byte(di, SY6513_IBAT_PROTECTION_REG, &ibat);
	ret += sy6513_read_byte(di, SY6513_SC_VDROP_OVP_REG, &ribus);
	if (ret)
		return -EINVAL;

	hwlog_info("ic_%u vbus_ovp_threshole_mv [0x%x]=0x%x, ibus_ovp_threshole [0x%x]=0x%x\n",
		di->ic_role, SY6513_VBUS_VOUT_OVP_REG, vbus, SY6513_IBUS_PROTECTION_REG, ibus);
	hwlog_info("ic_%u ibat_ovp_threshole_mv [0x%x]=0x%x, ribus_ovp_threshole [0x%x]=0x%x\n",
		di->ic_role, SY6513_IBAT_PROTECTION_REG, ibat, SY6513_SC_VDROP_OVP_REG, ribus);

	di->charge_mode = mode;
	return ret;
}

static int sy6513_reg_and_threshold_init(void *dev_data)
{
	int ret;
	struct sy6513_device_info *di = dev_data;

	if (!di)
		return -ENODEV;

	ret = sy6513_reg_init(di);
	ret += sy6513_threshold_reg_init(di, SY6513_CHG_F21SC_MODE);

	return ret;
}

static int sy6513_reg_reset_and_init(void *dev_data)
{
	int ret;
	struct sy6513_device_info *di = dev_data;

	if (!di)
		return -ENODEV;

	ret = sy6513_reg_reset(di);
	ret += sy6513_reg_and_threshold_init(di);

	return ret;
}

static int sy6513_batinfo_init(void *dev_data)
{
	return 0;
}

static int sy6513_batinfo_exit(void *dev_data)
{
	return 0;
}

static int sy6513_lvc_set_threshold(int enable, void *dev_data)
{
	struct sy6513_device_info *di = dev_data;
	u8 mode = enable ? SY6513_CHG_FBYPASS_MODE : SY6513_CHG_F21SC_MODE;

	if (!di)
		return -EPERM;

	if (sy6513_threshold_reg_init(di, mode))
		return -EINVAL;

	return 0;
}

static int sy6513_sc_set_threshold(int enable, void *dev_data)
{
	struct sy6513_device_info *di = dev_data;
	u8 mode = SY6513_CHG_F21SC_MODE;

	if (!di)
		return -ENODEV;

	if (sy6513_threshold_reg_init(di, mode))
		return -EINVAL;

	return 0;
}

static int sy6513_sc4_set_threshold(int enable, void *dev_data)
{
	struct sy6513_device_info *di = dev_data;
	u8 mode = enable ? SY6513_CHG_F41SC_MODE : SY6513_CHG_F21SC_MODE;

	if (!di)
		return -ENODEV;

	if (sy6513_threshold_reg_init(di, mode))
		return -EINVAL;

	return 0;
}

static int sy6513_reverse12_mode_enable(bool enable, struct sy6513_device_info *di)
{
	int ret;
	u8 pull_down_config_reg = 0;
	u8 reverse_en;

	reverse_en = enable ? SY6513_CP_SWITCHING_STAT_ENABLE : SY6513_CP_SWITCHING_STAT_DISABLE;

	if (reverse_en) {
		u8 reverse_mask = SY6513_OVP_G_EN_REVERSE_MODE_MASK | SY6513_WPC_G_EN_REVERSE_MODE_MASK;
		ret = sy6513_charge_mode_enable(di, SY6513_CHG_R12SC_MODE);
		ret += sy6513_write_multi_mask(di, SY6513_PULL_DOWN_CONFIG_REG, reverse_mask, 0x0);
	} else {
		ret = sy6513_charge_mode_enable(di, SY6513_CHG_R12SC_MODE);
	}

	power_msleep(DT_MSLEEP_200MS, 0, NULL);
	ret += sy6513_read_byte(di, SY6513_PULL_DOWN_CONFIG_REG, &pull_down_config_reg);
	if (ret)
		return -EINVAL;

	hwlog_info("%s ic_%u reg[0x%x]=0x%x, reg[0x%x]=0x%x\n", __func__,
		di->ic_role, SY6513_PULL_DOWN_CONFIG_REG, pull_down_config_reg);
	return 0;
}

static int sy6513_reverse12_set_threshold(bool enable, struct sy6513_device_info *di)
{
	u8 mode = enable ? SY6513_CHG_R12SC_MODE : SY6513_CHG_F21SC_MODE;

	return sy6513_threshold_reg_init(di, mode);
}

static int sy6513_get_register_head(char *buffer, int size, void *dev_data)
{
	struct sy6513_device_info *di = dev_data;

	if (!buffer || !di)
		return -ENODEV;

	snprintf(buffer, size, "dev       mode   Ibus   Vbus   Ibat   Vusb   Vout   Vbat   Temp   ");

	return 0;
}

static int sy6513_value_dump(char *buffer, int size, void *dev_data)
{
	u8 val = 0;
	char buff[SY6513_BUF_LEN] = { 0 };
	struct sy6513_device_info *di = dev_data;
	struct sy6513_dump_value dv = { 0 };

	if (!buffer || !di)
		return -ENODEV;

	dv.vbat = sy6513_get_vbat_mv(dev_data);
	(void)sy6513_get_ibus_ma(&dv.ibus, dev_data);
	(void)sy6513_get_vbus_mv(&dv.vbus, dev_data);
	(void)sy6513_get_ibat_ma(&dv.ibat, dev_data);
	(void)sy6513_get_vusb_mv(&dv.vusb, dev_data);
	(void)sy6513_get_vout_mv(&dv.vout, dev_data);
	(void)sy6513_get_device_temp(&dv.temp, dev_data);
	(void)sy6513_read_byte(di, SY6513_CHARGE_MODE_CONTROL1_REG, &val);

	snprintf(buff, sizeof(buff), "%s ", di->name);
	strncat(buffer, buff, strlen(buff));

	if (sy6513_is_device_close(dev_data)) {
		snprintf(buff, sizeof(buff), "%s", "   OFF   ");
	} else if (((val & SY6513_CHG_MODE_MASK) >> SY6513_CHG_MODE_SHIFT) == SY6513_CHG_FBYPASS_MODE) {
		snprintf(buff, sizeof(buff), "%s", "   LVC   ");
	} else if (((val & SY6513_CHG_MODE_MASK) >> SY6513_CHG_MODE_SHIFT) == SY6513_CHG_F21SC_MODE) {
		snprintf(buff, sizeof(buff), "%s", "   SC   ");
	} else if (((val & SY6513_CHG_MODE_MASK) >> SY6513_CHG_MODE_SHIFT) == SY6513_CHG_F41SC_MODE) {
		snprintf(buff, sizeof(buff), "%s", "   SC4   ");
	}

	strncat(buffer, buff, strlen(buff));
	snprintf(buff, sizeof(buff), "%-7d%-7d%-7d%-7d%-7d%-7d%-7d",
		dv.ibus, dv.vbus, dv.ibat, dv.vusb, dv.vout, dv.vbat, dv.temp);
	strncat(buffer, buff, strlen(buff));

	return 0;
}

static int sy6513_cp_post_probe(void *dev_data)
{
	return 0;
}

static int sy6513_cp_device_check(void *dev_data)
{
	return 0;
}

static int sy6513_reverse_cp_chip_init(void *dev_data, bool enable)
{
	struct sy6513_device_info *di = dev_data;
	int ret;
	int retry = 5; /* retry 5 times */
	int vbat = 0;

	if (!di)
		return -ENODEV;

	power_supply_get_int_prop(POWER_PLATFORM_BAT_PSY_NAME, POWER_SUPPLY_PROP_VOLTAGE_NOW,
		&vbat, POWER_SUPPLY_DEFAULT_VOLTAGE_NOW, POWER_UV_PER_MV);
	hwlog_info("%s vbat=%d, enable = %d\n", __func__, vbat, enable);
	ret = sy6513_reverse12_set_threshold(enable, di);
	if (ret)
		return ret;

	if (enable) {
		do {
			sy6513_reverse12_mode_enable(true, di);
			if (!sy6513_is_device_close(dev_data)) {
				hwlog_info("set rvs cp open succ\n");
				return 0;
			}
		} while (retry-- > 0);
		hwlog_err("set rvs cp open failed\n");
		return -EINVAL;
	}

	ret = sy6513_reverse12_mode_enable(false, di);

	return ret;
}

static struct dc_ic_ops sy6513_lvc_ops = {
	.dev_name = "sy6513",
	.ic_init = sy6513_charge_init,
	.ic_exit = sy6513_lvc_charge_exit,
	.ic_enable = sy6513_lvc_charge_enable,
	.ic_discharge = sy6513_discharge,
	.is_ic_close = sy6513_is_device_close,
	.get_ic_id = sy6513_get_device_id,
	.config_ic_watchdog = sy6513_config_watchdog_ms,
	.kick_ic_watchdog = sy6513_kick_watchdog_ms,
	.ic_reg_reset_and_init = sy6513_reg_and_threshold_init,
	.set_ic_thld = sy6513_lvc_set_threshold,
};

static struct dc_ic_ops sy6513_sc_ops = {
	.dev_name = "sy6513",
	.ic_init = sy6513_charge_init,
	.ic_exit = sy6513_sc_charge_exit,
	.ic_enable = sy6513_sc_charge_enable,
	.ic_discharge = sy6513_discharge,
	.is_ic_close = sy6513_is_device_close,
	.get_ic_id = sy6513_get_device_id,
	.config_ic_watchdog = sy6513_config_watchdog_ms,
	.kick_ic_watchdog = sy6513_kick_watchdog_ms,
	.ic_reg_reset_and_init = sy6513_reg_and_threshold_init,
	.set_ic_thld = sy6513_sc_set_threshold,
};

static struct dc_ic_ops sy6513_sc4_ops = {
	.dev_name = "sy6513",
	.ic_init = sy6513_charge_init,
	.ic_exit = sy6513_sc4_charge_exit,
	.ic_enable = sy6513_sc4_charge_enable,
	.ic_discharge = sy6513_discharge,
	.is_ic_close = sy6513_is_device_close,
	.get_ic_id = sy6513_get_device_id,
	.config_ic_watchdog = sy6513_config_watchdog_ms,
	.kick_ic_watchdog = sy6513_kick_watchdog_ms,
	.ic_reg_reset_and_init = sy6513_reg_and_threshold_init,
	.set_ic_thld = sy6513_sc4_set_threshold,
};

static struct dc_batinfo_ops sy6513_batinfo_ops = {
	.init = sy6513_batinfo_init,
	.exit = sy6513_batinfo_exit,
	.get_bat_btb_voltage = sy6513_get_vbat_mv,
	.get_bat_package_voltage = sy6513_get_vbat_mv,
	.get_vbus_voltage = sy6513_get_vbus_mv,
	.get_bat_current = sy6513_get_ibat_ma,
	.get_ic_ibus = sy6513_get_ibus_ma,
	.get_ic_temp = sy6513_get_device_temp,
	.get_ic_vout = sy6513_get_vout_mv,
	.get_ic_vusb = sy6513_get_vusb_mv,
};

static struct charge_pump_ops sy6513_cp_ops = {
	.cp_type = CP_TYPE_MAIN,
	.chip_name = "sy6513",
	.dev_check = sy6513_cp_device_check,
	.post_probe = sy6513_cp_post_probe,
	.rvs_cp_chip_init = sy6513_reverse_cp_chip_init,
};

static struct power_log_ops sy6513_log_ops = {
	.dev_name = "sy6513",
	.dump_log_head = sy6513_get_register_head,
	.dump_log_content = sy6513_value_dump,
};

static void sy6513_init_ops_dev_data(struct sy6513_device_info *di)
{
	memcpy(&di->lvc_ops, &sy6513_lvc_ops, sizeof(struct dc_ic_ops));
	di->lvc_ops.dev_data = (void *)di;
	memcpy(&di->sc_ops, &sy6513_sc_ops, sizeof(struct dc_ic_ops));
	di->sc_ops.dev_data = (void *)di;
	memcpy(&di->sc4_ops, &sy6513_sc4_ops, sizeof(struct dc_ic_ops));
	di->sc4_ops.dev_data = (void *)di;
	memcpy(&di->batinfo_ops, &sy6513_batinfo_ops, sizeof(struct dc_batinfo_ops));
	di->batinfo_ops.dev_data = (void *)di;
	memcpy(&di->log_ops, &sy6513_log_ops, sizeof(struct power_log_ops));
	di->log_ops.dev_data = (void *)di;

	if (!di->ic_role) {
		snprintf(di->name, CHIP_DEV_NAME_LEN, "sy6513");
	} else {
		snprintf(di->name, CHIP_DEV_NAME_LEN, "sy6513_%u", di->ic_role);
		di->lvc_ops.dev_name = di->name;
		di->sc_ops.dev_name = di->name;
		di->sc4_ops.dev_name = di->name;
		di->log_ops.dev_name = di->name;
	}
}

static void sy6513_ops_register(struct sy6513_device_info *di)
{
	int ret;

	sy6513_init_ops_dev_data(di);

	ret = dc_ic_ops_register(LVC_MODE, di->ic_role, &di->lvc_ops);
	ret += dc_ic_ops_register(SC_MODE, di->ic_role, &di->sc_ops);
	ret += dc_ic_ops_register(SC4_MODE, di->ic_role, &di->sc4_ops);
	ret += dc_batinfo_ops_register(di->ic_role, &di->batinfo_ops, di->device_id);
	if (ret)
		hwlog_err("ic_%u sysinfo ops register failed\n", di->ic_role);

	ret = sy6513_protocol_ops_register(di);
	if (ret)
		hwlog_err("ic_%u scp or fcp ops register failed\n", di->ic_role);

	sy6513_power_sw_register(di);

	power_log_ops_register(&di->log_ops);
	if (di->rvs_cp_support) {
		sy6513_cp_ops.dev_data = (void *)di;
		(void)charge_pump_ops_register(&sy6513_cp_ops);
	}
}

static bool sy6513_vbus_value_check(struct sy6513_device_info *di, int val)
{
	bool ret = false;

	switch (di->charge_mode) {
	case SY6513_CHG_FBYPASS_MODE:
		if (val >= SY6513_BPS_VBUS_OVP_MIN)
			ret = true;
		break;
	case SY6513_CHG_F21SC_MODE:
		if (val >= SY6513_21SC_VBUS_OVP_MIN * SY6513_21SC_VBUS_OVP_STEP)
			ret = true;
		break;
	case SY6513_CHG_F41SC_MODE:
		if (val >= SY6513_41SC_VBUS_OVP_MIN * SY6513_21SC_VBUS_OVP_STEP)
			ret = true;
		break;
	default:
		hwlog_err("mode check fail, mode=%u\n", di->charge_mode);
		break;
	}

	return ret;
}

static bool sy6513_ibus_value_check(struct sy6513_device_info *di, int val)
{
	bool ret = false;

	/* step: 2mA */
	val *= 2;

	switch (di->charge_mode) {
	case SY6513_CHG_FBYPASS_MODE:
	case SY6513_CHG_F21SC_MODE:
		if (val >= SY6513_21SC_IBUS_OCP_6000MA_VALUE)
			ret = true;
		break;
	case SY6513_CHG_F41SC_MODE:
		if (val >= SY6513_41SC_IBUS_OCP_3750MA_VALUE)
			ret = true;
		break;
	default:
		hwlog_err("mode check fail, mode=%u\n", di->charge_mode);
		break;
	}

	return ret;
}

static void sy6513_fault_event_notify(unsigned long event, void *data)
{
	power_event_anc_notify(POWER_ANT_DC_FAULT, event, data);
}

static void sy6513_fault_handle(struct sy6513_device_info *di, struct nty_data *data, u8 *flag, int flag_len)
{
	int val = 0;
	u8 flag1 = flag[SY6513_IRQ_FLT_FLAG1];
	u8 flag2 = flag[SY6513_IRQ_FLT_FLAG2];
	u8 flag3 = flag[SY6513_IRQ_FLT_FLAG3];
	u8 ibus_judge_mask = SY6513_41SC_IBUS_OCP_FLAG_MASK | SY6513_21SC_OR_FBPS_IBUS_OCP_FLAG_MASK |
		SY6513_14SC_IBUS_OCP_FLAG_MASK | SY6513_12SC_OR_RBPS_IBUS_OCP_FLAG_MASK;

	if (flag3 & SY6513_VBAT1_OVP_FLAG_MASK) {
		val = sy6513_get_vbat_mv(di);
		hwlog_info("%s ic_%d VBAT1 OVP happened, vbat=%d\n", __func__, di->ic_role, val);
		if (val >= SY6513_FWD_VBAT1_OVP_INIT)
			sy6513_fault_event_notify(POWER_NE_DC_FAULT_VBAT_OVP, data);
	} else if (flag1 & SY6513_VUSB_OVP_FLAG_MASK) {
		sy6513_get_vusb_mv(&val, di);
		hwlog_info("%s ic_%d VUSB OVP happened, vusb=%d\n", __func__, di->ic_role, val);
		sy6513_fault_event_notify(POWER_NE_DC_FAULT_AC_OVP, data);
	} else if (flag1 & SY6513_IBAT_OCP_FLAG_MASK) {
		sy6513_get_ibat_ma(&val, di);
		hwlog_info("%s ic_%d IBAT OCP happened, ibat=%d\n", __func__, di->ic_role, val);
		if (val >= SY6513_FWD_IBAT_OCP_INIT)
			sy6513_fault_event_notify(POWER_NE_DC_FAULT_IBAT_OCP, data);
	} else if (flag1 & SY6513_VBUS_OVP_FLAG_MASK) {
		sy6513_get_vbus_mv(&val, di);
		hwlog_info("%s ic_%d VBUS OVP happened, vbus=%d\n", __func__, di->ic_role, val);
		if (sy6513_vbus_value_check(di, val))
			sy6513_fault_event_notify(POWER_NE_DC_FAULT_VBUS_OVP, data);
	} else if (flag2 & ibus_judge_mask) {
		sy6513_get_ibus_ma(&val, di);
		hwlog_info("%s ic_%d IBUS OCP happened, ibus=%d\n", __func__, di->ic_role, val);
		if (sy6513_ibus_value_check(di, val))
			sy6513_fault_event_notify(POWER_NE_DC_FAULT_IBUS_OCP, data);
	} else if (flag3 & SY6513_SC_VOUT_OVP_FLAG_MASK) {
		sy6513_get_vout_mv(&val, di);
		hwlog_info("%s ic_%d VOUT_OVP happened, vout=%d\n", __func__, di->ic_role, val);
	}
}

static void sy6513_interrupt_work(struct work_struct *work)
{
	u8 flag[SY6513_IRQ_END] = {0};
	struct sy6513_device_info *di = NULL;
	struct nty_data *data = NULL;

	if (!work)
		return;

	di = container_of(work, struct sy6513_device_info, irq_work);
	if (!di || !di->client) {
		hwlog_err("di is null\n");
		return;
	}

	(void)sy6513_read_byte(di, SY6513_FLT_FLAG1_REG, &flag[SY6513_IRQ_FLT_FLAG1]);
	(void)sy6513_read_byte(di, SY6513_FLT_FLAG2_REG, &flag[SY6513_IRQ_FLT_FLAG2]);
	(void)sy6513_read_byte(di, SY6513_FLT_FLAG3_REG, &flag[SY6513_IRQ_FLT_FLAG3]);
	(void)sy6513_read_byte(di, SY6513_FLT_FLAG4_REG, &flag[SY6513_IRQ_FLT_FLAG4]);
	(void)sy6513_read_byte(di, SY6513_FLT_FLAG5_REG, &flag[SY6513_IRQ_FLT_FLAG5]);
	(void)sy6513_read_byte(di, SY6513_FLT_FLAG6_REG, &flag[SY6513_IRQ_FLT_FLAG6]);

	data = &(di->nty_data);
	data->addr = di->client->addr;

	if (di->int_notify_enable_flag) {
		sy6513_fault_handle(di, data, flag, SY6513_IRQ_END);
		sy6513_dump_register(di);
	}

	if (sy6513_charger_irq_clear(di))
		hwlog_err("irq clear fail\n");

	hwlog_info("ic_%d FLAG1 [0x%x]=0x%x FLAG2 [0x%x]=0x%x FLAG3 [0x%x]=0x%x " \
		"FLAG4 [0x%x]=0x%x FLAG5 [0x%x]=0x%x FLAG6 [0x%x]=0x%x\n", di->ic_role,
		SY6513_FLT_FLAG1_REG, flag[SY6513_IRQ_FLT_FLAG1],
		SY6513_FLT_FLAG2_REG, flag[SY6513_IRQ_FLT_FLAG2],
		SY6513_FLT_FLAG3_REG, flag[SY6513_IRQ_FLT_FLAG3],
		SY6513_FLT_FLAG4_REG, flag[SY6513_IRQ_FLT_FLAG4],
		SY6513_FLT_FLAG5_REG, flag[SY6513_IRQ_FLT_FLAG5],
		SY6513_FLT_FLAG6_REG, flag[SY6513_IRQ_FLT_FLAG6]);

	enable_irq(di->irq_int);
}

static irqreturn_t sy6513_interrupt(int irq, void *_di)
{
	struct sy6513_device_info *di = _di;

	if (!di)
		return IRQ_HANDLED;

	if (di->chip_already_init == 0)
		hwlog_err("chip not init\n");

	if (di->init_finish_flag == SY6513_INIT_FINISH)
		di->int_notify_enable_flag = SY6513_ENABLE_INT_NOTIFY;

	hwlog_info("int happened\n");
	disable_irq_nosync(di->irq_int);
	schedule_work(&di->irq_work);

	return IRQ_HANDLED;
}

static int sy6513_irq_init(struct sy6513_device_info *di, struct device_node *np)
{
	int ret;

	INIT_WORK(&di->irq_work, sy6513_interrupt_work);
	ret = power_gpio_config_interrupt(np, "gpio_int", "sy6513_gpio_int", &di->gpio_int, &di->irq_int);
	if (ret)
		return ret;

	ret = request_irq(di->irq_int, sy6513_interrupt, IRQF_TRIGGER_FALLING, "sy6513_int_irq", di);
	if (ret) {
		hwlog_err("gpio irq request fail\n");
		di->irq_int = -1;
		gpio_free(di->gpio_int);
		return ret;
	}

	enable_irq_wake(di->irq_int);

	return 0;
}

static int sy6513_gpio_init(struct sy6513_device_info *di, struct device_node *np)
{
	if (of_find_property(np, "gpio_reset", NULL) &&
		power_gpio_config_output(np, "gpio_reset", "sy6513_gpio_reset", &di->gpio_reset, SY6513_SWITCHCAP_ENABLE))
		return -EPERM;

	if (of_find_property(np, "gpio_enable", NULL) &&
		power_gpio_config_output(np, "gpio_enable", "sy6513_gpio_enable", &di->gpio_enable, SY6513_GPIO_DISABLE)) {
		if (di->gpio_reset > 0)
			gpio_free(di->gpio_reset);
		return -EPERM;
	}

	(void)power_gpio_config_output(np, "gpio_lpm", "sy6513_gpio_lpm", &di->gpio_lpm, SY6513_GPIO_ENABLE);

	return 0;
}

static void sy6513_parse_dts(struct device_node *np, struct sy6513_device_info *di)
{
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "switching_frequency",
		&di->switching_frequency, SY6513_SWITCHING_FREQUENCY_600KHZ);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "sc4_ibus_ocp",
		&di->sc4_ibus_ocp, SY6513_41SC_IBUS_OCP_3750MA);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "sc4_ibat_ocp",
		&di->sc4_ibat_ocp, SY6513_FWD_IBAT_OCP_8500MA);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "watchdog_time",
		&di->watchdog_time, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "scp_support",
		(u32 *)&(di->dts_scp_support), 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "fcp_support",
		(u32 *)&(di->dts_fcp_support), 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "ic_role",
		(u32 *)&(di->ic_role), IC_ONE);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "sense_r_config",
		&di->sense_r_config, SENSE_R_1_MOHM);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "sense_r_actual",
		&di->sense_r_actual, SENSE_R_1_MOHM);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "rvs_cp_support",
		&di->rvs_cp_support, 0);
	sy6513_parse_sw_attr(np, di);
}

static void sy6513_init_lock_mutex(struct sy6513_device_info *di)
{
	mutex_init(&di->scp_detect_lock);
	mutex_init(&di->accp_adapter_reg_lock);
}

static void sy6513_destroy_lock_mutex(struct sy6513_device_info *di)
{
	mutex_destroy(&di->scp_detect_lock);
	mutex_destroy(&di->accp_adapter_reg_lock);
}

static int sy6513_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int ret;
	struct sy6513_device_info *di = NULL;
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

	sy6513_parse_dts(np, di);

	ret = sy6513_gpio_init(di, np);
	if (ret)
		goto sy6513_fail_0;

	ret = sy6513_get_device_id(di);
	if (ret == SY6513_DEVICE_ID_GET_FAIL)
		goto sy6513_fail_1;

	sy6513_init_lock_mutex(di);

	(void)power_pinctrl_config(di->dev, "pinctrl-names", 1); /* 1:pinctrl-names length */
	ret = sy6513_reg_reset_and_init(di);
	if (ret)
		goto sy6513_fail_2;

	ret = sy6513_irq_init(di, np);
	if (ret)
		goto sy6513_fail_2;

	sy6513_ops_register(di);
	i2c_set_clientdata(client, di);

	return 0;

sy6513_fail_0:
	di->chip_already_init = 0;
	devm_kfree(&client->dev, di);
sy6513_fail_1:
	if (di->gpio_lpm > 0)
		gpio_free(di->gpio_lpm);
	if (di->gpio_reset > 0)
		gpio_free(di->gpio_reset);
	if (di->gpio_enable > 0)
		gpio_free(di->gpio_enable);
sy6513_fail_2:
	sy6513_destroy_lock_mutex(di);

	return ret;
}

static int sy6513_remove(struct i2c_client *client)
{
	struct sy6513_device_info *di = i2c_get_clientdata(client);

	if (!di)
		return -ENODEV;

	if (di->irq_int)
		free_irq(di->irq_int, di);

	if (di->gpio_int)
		gpio_free(di->gpio_int);

	if (di->gpio_enable)
		gpio_free(di->gpio_enable);

	if (di->gpio_reset)
		gpio_free(di->gpio_reset);

	return 0;
}

static void sy6513_shutdown(struct i2c_client *client)
{
	struct sy6513_device_info *di = i2c_get_clientdata(client);

	if (!di)
		return;

	sy6513_reg_reset(di);
}

#ifdef CONFIG_PM
static int sy6513_i2c_suspend(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct sy6513_device_info *di = NULL;

	if (!client)
		return 0;

	di = i2c_get_clientdata(client);
	if (di)
		sy6513_adc_enable(SY6513_ADC_DISABLE, di);

	return 0;
}

static int sy6513_i2c_resume(struct device *dev)
{
	return 0;
}

static void sy6513_i2c_complete(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct sy6513_device_info *di = NULL;

	if (!client)
		return;

	di = i2c_get_clientdata(client);
	if (di)
		sy6513_adc_enable(SY6513_ADC_ENABLE, di);

	return;
}

static const struct dev_pm_ops g_sy6513_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(sy6513_i2c_suspend, sy6513_i2c_resume)
	.complete = sy6513_i2c_complete,
};
#define SY6513_PM_OPS (&g_sy6513_pm_ops)
#else
#define SY6513_PM_OPS (NULL)
#endif /* CONFIG_PM */

MODULE_DEVICE_TABLE(i2c, sy6513);
static const struct of_device_id sy6513_of_match[] = {
	{
		.compatible = "sy6513",
		.data = NULL,
	},
	{},
};

static const struct i2c_device_id sy6513_i2c_id[] = {{"sy6513", 0}, {}};

static struct i2c_driver sy6513_driver = {
	.probe = sy6513_probe,
	.remove = sy6513_remove,
	.shutdown = sy6513_shutdown,
	.id_table = sy6513_i2c_id,
	.driver =
		{
			.owner = THIS_MODULE,
			.name = "sy6513",
			.of_match_table = of_match_ptr(sy6513_of_match),
			.pm = SY6513_PM_OPS,
		},
};

static int __init sy6513_init(void)
{
	return i2c_add_driver(&sy6513_driver);
}

static void __exit sy6513_exit(void)
{
	i2c_del_driver(&sy6513_driver);
}

module_init(sy6513_init);
module_exit(sy6513_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("sy6513 module driver");
