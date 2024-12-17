// SPDX-License-Identifier: GPL-2.0
/*
 * ffc_base.c
 *
 * ffc base driver
 *
 * Copyright (c) 2022-2022 Huawei Technologies Co., Ltd.
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
#include <chipset_common/hwpower/charger/charger_common_interface.h>
#include <chipset_common/hwpower/common_module/power_common_macro.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/common_module/power_supply.h>
#include <chipset_common/hwpower/hardware_monitor/ffc_base.h>
#include <huawei_platform/hwpower/common_module/power_platform_macro.h>
#include "../../cc_battery/battery_model/battery_model.h"

#define HWLOG_TAG ffc_base
HWLOG_REGIST();

#define FFC_VTERM_DELAY_MAX_CNT 2

static struct ffc_buck_term_para *ffc_select_buck_term_para(struct ffc_ctrl_dev *di)
{
	int i;
	const char *brand = POWER_SUPPLY_DEFAULT_BRAND;
	int bat_type;

	if (!di->ffc_term_para_group || di->term_para_select_ok)
		return di->buck_term_para;

	(void)power_supply_get_str_property_value(POWER_PLATFORM_BAT_PSY_NAME,
		POWER_SUPPLY_PROP_BRAND, &brand);
	bat_type = bat_model_get_bat_cathode_type();
	hwlog_info("bat brand=%s, bat_type=%d\n", brand, bat_type);
	switch (bat_type) {
	case BAT_MODEL_BAT_CATHODE_TYPE_GRAPHITE:
		bat_type = BATTERY_C;
		break;
	case BAT_MODEL_BAT_CATHODE_TYPE_SILICON:
		bat_type = BATTERY_SI;
		break;
	default:
		return di->buck_term_para;
	}

	for (i = 0; i < di->group_size; i++) {
		if (!di->ffc_term_para_group[i].bat_info.parse_ok)
			continue;

		if (!strstr(brand, di->ffc_term_para_group[i].bat_info.bat_sn))
			continue;

		if (bat_type != di->ffc_term_para_group[i].bat_info.bat_type)
			continue;

		memcpy(di->buck_term_para, di->ffc_term_para_group[i].term_para_group, sizeof(di->buck_term_para));
		di->term_para_select_ok = true;
		di->bat_type = bat_type;
		break;
	}

	return di->buck_term_para;
}

int ffc_get_buck_vterm_with_temp(struct ffc_ctrl_dev *di)
{
	int tbat = 0;
	int i;
	struct ffc_buck_term_para *para = NULL;

	if (!di || !di->buck_term_para_flag)
		return 0;

	para = ffc_select_buck_term_para(di);
	bat_temp_get_temperature(BAT_TEMP_MIXED, &tbat);
	hwlog_info("tbat=%d\n", tbat);
	for (i = 0; i < FFC_MAX_CHARGE_TERM; i++) {
		if ((tbat < para[i].temp_low) || (tbat > para[i].temp_high))
			continue;
		hwlog_info("%s set vterm increase %d\n", __func__, para[i].vterm_gain);
		return para[i].vterm_gain;
	}
	return 0;
}

int ffc_get_buck_vterm(struct ffc_ctrl_dev *di)
{
	int tbat = 0;
	int i;
	int ichg_avg = charge_get_battery_current_avg();
	struct ffc_buck_term_para *para = NULL;
	unsigned int vterm_dec = 0;

	if (!di || !di->buck_term_para_flag)
		return 0;

	charge_get_vterm_dec(&vterm_dec);
	para = ffc_select_buck_term_para(di);
	bat_temp_get_temperature(BAT_TEMP_MIXED, &tbat);
	hwlog_info("ichg_avg=%d, tbat=%d\n", ichg_avg, tbat);
	for (i = 0; i < FFC_MAX_CHARGE_TERM; i++) {
		if ((tbat < para[i].temp_low) || (tbat > para[i].temp_high))
			continue;
		if ((ichg_avg > para[i].ichg_thre) || (vterm_dec && (di->bat_type == BATTERY_SI))) {
			hwlog_info("buck set vterm increase %d\n", para[i].vterm_gain);
			return para[i].vterm_gain;
		}
	}
	return 0;
}

int ffc_get_buck_ichg_th(struct ffc_ctrl_dev *di)
{
	int tbat = 0;
	int i;
	struct ffc_buck_term_para *para = NULL;

	if (!di || !di->buck_term_para_flag)
		return 0;

	para = ffc_select_buck_term_para(di);
	bat_temp_get_temperature(BAT_TEMP_MIXED, &tbat);
	for (i = 0; i < FFC_MAX_CHARGE_TERM; i++) {
		if ((tbat < para[i].temp_low) || (tbat > para[i].temp_high))
			continue;
		hwlog_info("buck set ichg_thre %d\n", para[i].ichg_thre);
		return para[i].ichg_thre;
	}
	return 0;
}

int ffc_get_buck_iterm(struct ffc_ctrl_dev *di)
{
	int tbat = 0;
	int i;
	struct ffc_buck_term_para *para = NULL;

	if (!di || !di->buck_term_para_flag)
		return 0;

	para = ffc_select_buck_term_para(di);
	bat_temp_get_temperature(BAT_TEMP_MIXED, &tbat);
	for (i = 0; i < FFC_MAX_CHARGE_TERM; i++) {
		if ((tbat < para[i].temp_low) || (tbat > para[i].temp_high))
			continue;
		hwlog_info("buck set iterm %d\n", para[i].iterm);
		return para[i].iterm;
	}
	return 0;
}

static int ffc_parse_buck_term_para(struct device_node *np, const char *prop, struct ffc_buck_term_para *para, int size)
{
	int len;
	int row;
	int col;
	int idata[FFC_MAX_CHARGE_TERM * FFC_BUCK_TERM_TOTAL] = { 0 };

	len = power_dts_read_string_array(power_dts_tag(HWLOG_TAG), np,
		prop, idata, FFC_MAX_CHARGE_TERM, FFC_BUCK_TERM_TOTAL);
	if ((len < 0) || (len / FFC_BUCK_TERM_TOTAL > size))
		return -EINVAL;

	for (row = 0; row < len / FFC_BUCK_TERM_TOTAL; row++) {
		col = row * FFC_BUCK_TERM_TOTAL + FFC_BUCK_TEMP_LOW;
		para[row].temp_low = idata[col];
		col = row * FFC_BUCK_TERM_TOTAL + FFC_BUCK_TEMP_HIGH;
		para[row].temp_high = idata[col];
		col = row * FFC_BUCK_TERM_TOTAL + FFC_BUCK_VTERM_GAIN;
		para[row].vterm_gain = idata[col];
		col = row * FFC_BUCK_TERM_TOTAL + FFC_BUCK_ICHG_THRE;
		para[row].ichg_thre = idata[col];
		col = row * FFC_BUCK_TERM_TOTAL + FFC_BUCK_ITERM;
		para[row].iterm = idata[col];
	}

	for (row = 0; row < len / FFC_BUCK_TERM_TOTAL; row++)
		hwlog_info("buck_term_para[%d]: %d %d %d %d %d\n", row, para[row].temp_low, para[row].temp_high,
			para[row].vterm_gain, para[row].ichg_thre, para[row].iterm);

	return 0;
}

static void ffc_parse_buck_term_para_dts(struct device_node *np, struct ffc_ctrl_dev *di)
{
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "delay_times",
		&di->delay_max_times, FFC_VTERM_DELAY_MAX_CNT);
	di->fcp_support_ffc = of_property_read_bool(np, "fcp_support_ffc");
	if (!ffc_parse_buck_term_para(np, "buck_term_para", di->buck_term_para, FFC_MAX_CHARGE_TERM))
		di->buck_term_para_flag = true;
}

static int ffc_parse_buck_term_para_with_index(struct device_node *np, struct ffc_ctrl_dev *di, int index)
{
	const char *term_para = di->ffc_term_para_group[index].bat_info.buck_term_para_index;

	if (ffc_parse_buck_term_para(np, term_para, di->ffc_term_para_group[index].term_para_group,
		FFC_MAX_CHARGE_TERM))
		return -EINVAL;

	di->ffc_term_para_group[index].bat_info.parse_ok = 1;
	return 0;
}

static int ffc_parse_bat_para(struct device_node *np, struct ffc_ctrl_dev *di)
{
	int i, row, col, array_len, idata;
	const char *tmp_string = NULL;

	array_len = power_dts_read_count_strings(power_dts_tag(HWLOG_TAG), np, "ffc_bat_para",
		FFC_MAX_CHARGE_TERM, FFC_BAT_INFO_TOTAL);
	if (array_len < 0)
		return -EINVAL;

	di->group_size = array_len / FFC_BAT_INFO_TOTAL;
	di->ffc_term_para_group = kzalloc(sizeof(struct ffc_term_para_group) * array_len, GFP_KERNEL);
	if (!di->ffc_term_para_group)
		return -ENOMEM;

	for (i = 0; i < array_len; i++) {
		if (power_dts_read_string_index(power_dts_tag(HWLOG_TAG), np, "ffc_bat_para", i, &tmp_string))
			return -EINVAL;

		row = i / FFC_BAT_INFO_TOTAL;
		col = i % FFC_BAT_INFO_TOTAL;

		switch (col) {
		case FFC_BAT_SN:
			strncpy(di->ffc_term_para_group[row].bat_info.bat_sn, tmp_string, FFC_PARA_LEN_MAX - 1);
			break;
		case FFC_BAT_TYPE:
			if (kstrtoint(tmp_string, POWER_BASE_DEC, &idata))
				return -EINVAL;
			di->ffc_term_para_group[row].bat_info.bat_type = idata;
			break;
		case FFC_TERM_PARA_INDEX:
			strncpy(di->ffc_term_para_group[row].bat_info.buck_term_para_index, tmp_string,
				FFC_PARA_LEN_MAX - 1);
			break;
		default:
			break;
		}
	}

	for (i = 0; i < di->group_size; i++)
		hwlog_info("ffc_bat_para[%d]=%s %d %s\n", i, di->ffc_term_para_group[i].bat_info.bat_sn,
			di->ffc_term_para_group[i].bat_info.bat_type,
			di->ffc_term_para_group[i].bat_info.buck_term_para_index);

	return 0;
}

static void ffc_parse_buck_term_para_group(struct device_node *np, struct ffc_ctrl_dev *di)
{
	int i;

	if (ffc_parse_bat_para(np, di))
		return;

	for (i = 0; i < di->group_size; i++) {
		if (ffc_parse_buck_term_para_with_index(np, di, i))
			return;
	}
}

void ffc_ctrl_parse_dts(struct device_node *np, struct ffc_ctrl_dev *di)
{
	ffc_parse_buck_term_para_dts(np, di);
	ffc_parse_buck_term_para_group(np, di);
}
