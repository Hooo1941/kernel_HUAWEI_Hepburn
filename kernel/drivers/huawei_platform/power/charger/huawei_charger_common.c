/*
 * huawei_charger_common.c
 *
 * common interface for huawei charger driver
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
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

#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/power/huawei_charger.h>
#include <huawei_platform/power/huawei_charger_common.h>
#include <huawei_platform/hwpower/common_module/power_platform.h>

#define HWLOG_TAG huawei_charger_common
HWLOG_REGIST();

static struct charge_extra_ops *g_extra_ops;

int charge_extra_ops_register(struct charge_extra_ops *ops)
{
	int ret = 0;

	if (ops) {
		g_extra_ops = ops;
		hwlog_info("charge extra ops register ok\n");
	} else {
		hwlog_err("charge extra ops register fail\n");
		ret = -EPERM;
	}

	return ret;
}

bool charge_check_charger_ts(void)
{
	if (!g_extra_ops || !g_extra_ops->check_ts) {
		hwlog_err("g_extra_ops or check_ts is null\n");
		return false;
	}

	return g_extra_ops->check_ts();
}

bool charge_check_charger_otg_state(void)
{
	if (!g_extra_ops || !g_extra_ops->check_otg_state) {
		hwlog_err("g_extra_ops or check_otg_state is null\n");
		return false;
	}

	return g_extra_ops->check_otg_state();
}

int charge_set_charge_state(int state)
{
	if (!g_extra_ops || !g_extra_ops->set_state) {
		hwlog_err("g_extra_ops or set_state is null\n");
		return -1;
	}

	return g_extra_ops->set_state(state);
}

int get_charge_current_max(void)
{
	if (!g_extra_ops || !g_extra_ops->get_charge_current) {
		hwlog_err("g_extra_ops or get_charge_current is null\n");
		return 0;
	}

	return g_extra_ops->get_charge_current();
}
