// SPDX-License-Identifier: GPL-2.0
/*
 * battery_charge_balance_parallel.c
 *
 * huawei parallel battery charge balance driver
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

#include <securec.h>
#include <chipset_common/hwpower/battery/battery_charge_balance.h>
#include <chipset_common/hwpower/common_module/power_dsm.h>
#include <chipset_common/hwpower/common_module/power_log.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/common_module/power_supply_interface.h>

#define HWLOG_TAG battery_charge_balance_parallel
HWLOG_REGIST();

#define BAL_DETECT_CYCLE        5
#define BAL_RATIO_ERR_COUNT     3
#define BAL_DSM_BUF_SIZE        256
#define BAL_DETECT_CUR_TH       5600
#define BAL_UNBLANCE_MAX        1
#define BAL_UNBLANCE_MIN        0

static void bat_chg_bal_parallel_requst_current(int cur, int other_cur,
	struct bat_cccv_node cccv, struct bat_cccv_node other_cccv,
	int *own, int *other)
{
	int delta;

	if (cur <= 0) {
		*own = cccv.cur;
		*other = other_cccv.cur;
	} else {
		*own = cccv.cur;
		delta = cccv.cur - cur;
		if (other_cur <= 0) {
			*other = 0;
		} else {
			*other = other_cur + delta * other_cur / cur;
			if (*other < 0)
				*other = 0;
		}
	}
}

static void bat_chg_bal_parallel_current_bias_detect_dmd(
	struct bat_chg_balance_info *info, struct bat_chg_bal_device *di)
{
	int ret;
	int cur_ratio = info[BAT_MAIN].cur * 1000 / info[BAT_AUX].cur; /* multiplied by 1000 to calc ratio */
	char tmp_buf[BAL_DSM_BUF_SIZE] = { 0 };
	int info_total_cur = info[BAT_MAIN].cur + info[BAT_AUX].cur;

	if (info_total_cur < BAL_DETECT_CUR_TH) {
		di->detect_cycle = 0;
		di->ratio_err_cnt = 0;
		return;
	}

	if ((cur_ratio < di->unbalance_th[BAL_UNBLANCE_MIN]) ||
		(cur_ratio > di->unbalance_th[BAL_UNBLANCE_MAX])) {
		di->ratio_err_cnt++;
		if (di->ratio_err_cnt == BAL_RATIO_ERR_COUNT) {
			ret = snprintf_s(tmp_buf, BAL_DSM_BUF_SIZE, BAL_DSM_BUF_SIZE - 1,
				"%s: main-%d, %d, %d, aux-%d, %d, %d cur_ratio: %d ratio_range:%d %d\n",
				"cur_ratio out range, cur vol temp info",
				info[BAT_MAIN].cur, info[BAT_MAIN].vol, info[BAT_MAIN].temp,
				info[BAT_AUX].cur, info[BAT_AUX].vol, info[BAT_AUX].temp, cur_ratio,
				di->unbalance_th[BAL_UNBLANCE_MIN], di->unbalance_th[BAL_UNBLANCE_MAX]);
			if (ret >= 0)
				power_dsm_report_dmd(POWER_DSM_BATTERY_DETECT,
					POWER_DSM_DUAL_BATTERY_CURRENT_BIAS_DETECT, tmp_buf);
		}
	}

	di->detect_cycle++;
	if (di->detect_cycle >= BAL_DETECT_CYCLE) {
		di->detect_cycle = 0;
		di->ratio_err_cnt = 0;
	}
}

static int bat_chg_bal_parallel_check_bat_exist(struct bat_chg_bal_device *di,
	struct bat_cccv_node *cccv_node, int size,
	struct bat_chg_balance_cur *result)
{
	int is_exist[BAT_BALANCE_COUNT] = { 0 };

	if (size > BAT_BALANCE_COUNT)
		return -EINVAL;

	power_supply_get_int_property_value("battery_gauge",
		POWER_SUPPLY_PROP_PRESENT, &is_exist[BAT_MAIN]);
	power_supply_get_int_property_value("battery_gauge_aux",
		POWER_SUPPLY_PROP_PRESENT, &is_exist[BAT_AUX]);
	if (!is_exist[BAT_MAIN] && !is_exist[BAT_AUX]) {
		hwlog_err("battery main and aux not exist, do not balance\n");
		result->total_cur =
			cccv_node[BAT_MAIN].cur + cccv_node[BAT_AUX].cur;
		di->bal_cur = result->total_cur;
		return -EPERM;
	} else if (!is_exist[BAT_MAIN]) {
		hwlog_err("battery main not exist, do not balance\n");
		result->total_cur = cccv_node[BAT_AUX].cur;
		di->bal_cur = result->total_cur;
		return -EPERM;
	} else if (!is_exist[BAT_AUX] && !di->batt_balance_by_single_gauge) {
		hwlog_err("battery aux not exist, do not balance\n");
		result->total_cur = cccv_node[BAT_MAIN].cur;
		di->bal_cur = result->total_cur;
		return -EPERM;
	}

	return 0;
}

int bat_chg_bal_parallel_get_cur_info(struct bat_chg_bal_device *di,
	struct bat_chg_balance_info *info, u32 info_len,
	struct bat_cccv_node *cccv_node, u32 node_len,
	struct bat_chg_balance_cur *result)
{
	int own_cur[BAT_BALANCE_COUNT];
	int other_cur[BAT_BALANCE_COUNT];
	int request_cur[BAT_BALANCE_COUNT];

	if (!di || !info || !cccv_node || !result) {
		hwlog_err("%s invalid paras\n", __func__);
		return -EINVAL;
	}

	if ((info_len != BAT_BALANCE_COUNT) || (node_len != BAT_BALANCE_COUNT)) {
		hwlog_err("%s invalid lens\n", __func__);
		return -EINVAL;
	}

	bat_chg_bal_parallel_current_bias_detect_dmd(info, di);
	result->cccv[BAT_MAIN] = cccv_node[BAT_MAIN];
	result->cccv[BAT_AUX] = cccv_node[BAT_AUX];
	hwlog_info("%s info main %d %d %d, aux %d %d %d, cccv main %d %d, aux %d %d\n",
		__func__, info[BAT_MAIN].temp, info[BAT_MAIN].vol,
		info[BAT_MAIN].cur, info[BAT_AUX].temp, info[BAT_AUX].vol,
		info[BAT_AUX].cur, cccv_node[BAT_MAIN].vol,
		cccv_node[BAT_MAIN].cur, cccv_node[BAT_AUX].vol,
		cccv_node[BAT_AUX].cur);

	if (bat_chg_bal_parallel_check_bat_exist(di, cccv_node, BAT_BALANCE_COUNT, result))
		return 0;

	bat_chg_bal_parallel_requst_current(info[BAT_MAIN].cur, info[BAT_AUX].cur,
		cccv_node[BAT_MAIN], cccv_node[BAT_AUX],
		&own_cur[BAT_MAIN], &other_cur[BAT_MAIN]);
	bat_chg_bal_parallel_requst_current(info[BAT_AUX].cur, info[BAT_MAIN].cur,
		cccv_node[BAT_AUX], cccv_node[BAT_MAIN],
		&own_cur[BAT_AUX], &other_cur[BAT_AUX]);

	request_cur[BAT_MAIN] = own_cur[BAT_MAIN] <= other_cur[BAT_AUX] ?
		own_cur[BAT_MAIN] : other_cur[BAT_AUX];
	request_cur[BAT_AUX] = own_cur[BAT_AUX] <= other_cur[BAT_MAIN] ?
		own_cur[BAT_AUX] : other_cur[BAT_MAIN];
	result->total_cur = request_cur[BAT_MAIN] + request_cur[BAT_AUX];

	di->bal_cur = result->total_cur;
	di->req_cur[BAT_MAIN] = request_cur[BAT_MAIN];
	di->req_cur[BAT_AUX] = request_cur[BAT_AUX];
	hwlog_info("%s own_cur %d %d, other_cur %d %d, total_cur %d request_cur %d %d\n",
		__func__, own_cur[BAT_MAIN], own_cur[BAT_AUX],
		other_cur[BAT_MAIN], other_cur[BAT_AUX], result->total_cur,
		request_cur[BAT_MAIN], request_cur[BAT_AUX]);

	return 0;
}
