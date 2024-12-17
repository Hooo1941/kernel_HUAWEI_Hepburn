// SPDX-License-Identifier: GPL-2.0
/*
 * buck_charge_jeita.c
 *
 * buck charge jeita driver
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

#include <chipset_common/hwpower/buck_charge/buck_charge.h>
#include <chipset_common/hwpower/buck_charge/buck_charge_jeita.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/common_module/power_printk.h>

#define HWLOG_TAG buck_charge_jeita
HWLOG_REGIST();

#define INVALID_BATTERY_TEMP (-40)
#define DEFAULT_BATTERY_TEMP 25

int buck_charge_jeita_parse_jeita_table(struct device_node *np, void *p)
{
	int i, row, col, len;
	int idata[BC_JEITA_PARA_LEVEL * BCJ_INFO_TOTAL] = { 0 };
	struct buck_charge_dev *di = (struct buck_charge_dev *)p;

	len = power_dts_read_string_array(power_dts_tag(HWLOG_TAG), np,
		"jeita_table", idata, BC_JEITA_PARA_LEVEL, BCJ_INFO_TOTAL);
	if (len < 0)
		return -EINVAL;

	for (row = 0; row < len / BCJ_INFO_TOTAL; row++) {
		col = row * BCJ_INFO_TOTAL + BCJ_TEMP_MIN;
		di->jeita_table[row].temp_min = idata[col];
		col = row * BCJ_INFO_TOTAL + BCJ_TEMP_MAX;
		di->jeita_table[row].temp_max = idata[col];
		col = row * BCJ_INFO_TOTAL + BCJ_IIN_LIMIT;
		di->jeita_table[row].iin_limit = idata[col];
		col = row * BCJ_INFO_TOTAL + BCJ_ICHG_LIMIT;
		di->jeita_table[row].ichg_limit = idata[col];
		col = row * BCJ_INFO_TOTAL + BCJ_VTERM;
		di->jeita_table[row].vterm = idata[col];
		col = row * BCJ_INFO_TOTAL + BCJ_TEMP_BACK;
		di->jeita_table[row].temp_back = idata[col];
	}

	for (i = 0; i < BC_JEITA_PARA_LEVEL; i++)
		hwlog_info("buck charge temp_para[%d] %d %d %d %d %d %d\n",
			i, di->jeita_table[i].temp_min, di->jeita_table[i].temp_max, di->jeita_table[i].iin_limit,
			di->jeita_table[i].ichg_limit, di->jeita_table[i].vterm, di->jeita_table[i].temp_back);
	return 0;
}

void buck_charge_jeita_tbatt_handler(int temp,
	struct bc_jeita_para jeita_table[], struct bc_jeita_result *data)
{
	int i;
	static int last_i;
	static int last_iin;
	static int last_ichg;
	static int last_vterm;
	static int last_temp;
	static int flag_running_first = 1;

	if (!data) {
		hwlog_err("data is null\n");
		return;
	}

	if (flag_running_first && (temp <= INVALID_BATTERY_TEMP))
		temp = DEFAULT_BATTERY_TEMP;

	for (i = 0; i < BC_JEITA_PARA_LEVEL; i++) {
		if ((temp >= jeita_table[i].temp_min) && (temp < jeita_table[i].temp_max)) {
			if ((last_temp - temp <= 0) || (jeita_table[i].temp_max - temp > jeita_table[i].temp_back) ||
				(abs(last_i - i) > 1) || (flag_running_first == 1)) {
				data->iin = jeita_table[i].iin_limit;
				data->ichg = jeita_table[i].ichg_limit;
				data->vterm = jeita_table[i].vterm;
			} else {
				data->iin = last_iin;
				data->ichg = last_ichg;
				data->vterm = last_vterm;
			}
			break;
		}
	}
	last_i = i;
	flag_running_first = 0;
	last_temp = temp;
	last_iin = data->iin;
	last_ichg = data->ichg;
	last_vterm = data->vterm;

	hwlog_info("%s: i = %d, temp = %d, data->iin = %d, data->ichg = %d, data->vterm = %d\n",
		__func__, i, temp, data->iin, data->ichg, data->vterm);
}
