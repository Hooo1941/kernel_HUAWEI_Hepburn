// SPDX-License-Identifier: GPL-2.0
/*
 * battery_charge_balance_serial.c
 *
 * huawei serial battery charge balance driver
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

#include <chipset_common/hwpower/battery/battery_charge_balance.h>
#include <chipset_common/hwpower/common_module/power_algorithm.h>
#include <chipset_common/hwpower/common_module/power_log.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_rechg.h>

#define HWLOG_TAG battery_charge_balance_serial
HWLOG_REGIST();

int bat_chg_bal_serial_get_cur_info(struct bat_chg_bal_device *di,
	struct bat_chg_balance_info *info, u32 info_len,
	struct bat_cccv_node *cccv_node, u32 node_len,
	struct bat_chg_balance_cur *result)
{
	if (!di || !info || !cccv_node || !result) {
		hwlog_err("%s invalid paras\n", __func__);
		return -EINVAL;
	}

	if ((info_len != BAT_BALANCE_COUNT) || (node_len != BAT_BALANCE_COUNT)) {
		hwlog_err("%s invalid lens\n", __func__);
		return -EINVAL;
	}

	result->cccv[BAT_MAIN] = cccv_node[BAT_MAIN];
	result->cccv[BAT_AUX] = cccv_node[BAT_AUX];
	result->total_cur = power_min_positive(cccv_node[BAT_MAIN].cur, cccv_node[BAT_AUX].cur) * info_len;
	result->total_cur += dc_rechg_get_ibat();

	di->bal_cur = result->total_cur;
	di->req_cur[BAT_MAIN] = cccv_node[BAT_MAIN].cur;
	di->req_cur[BAT_AUX] = cccv_node[BAT_AUX].cur;
	hwlog_info("%s info main %d %d %d, aux %d %d %d," \
		"cccv main %d %d, aux %d %d, total_cur %d\n", __func__,
		info[BAT_MAIN].temp, info[BAT_MAIN].vol, info[BAT_MAIN].cur,
		info[BAT_AUX].temp, info[BAT_AUX].vol, info[BAT_AUX].cur,
		cccv_node[BAT_MAIN].vol, cccv_node[BAT_MAIN].cur,
		cccv_node[BAT_AUX].vol, cccv_node[BAT_AUX].cur,
		result->total_cur);

	return 0;
}
