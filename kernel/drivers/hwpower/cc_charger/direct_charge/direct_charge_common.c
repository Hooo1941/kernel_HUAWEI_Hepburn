/*
 * direct_charge_common.c
 *
 * direct charge common driver
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

#include <chipset_common/hwpower/direct_charge/direct_charge_common.h>
#include <chipset_common/hwpower/common_module/power_common_macro.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_ic.h>

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif

#define HWLOG_TAG direct_charge_common
HWLOG_REGIST();

struct dc_common_dev {
	bool need_recheck_flag;
	bool first_check_completed;
};

struct mode_info {
	int idx;
	const char *name;
	int ratio;
};

static struct mode_info g_dc_map_tbl[] = {
	[LVC_MODE] = { DC_MODE_LVC, "lvc_mode", 1 },
	[SC_MODE] = { DC_MODE_SC, "sc_mode", 2 },
	[SC4_MODE] = { DC_MODE_SC4, "sc4_mode", 4 },
};

static bool dc_comm_mode_is_valid(int mode)
{
	switch (mode) {
	case LVC_MODE:
	case SC_MODE:
	case SC4_MODE:
		return true;
	default:
		break;
	}

	hwlog_err("invalid mode\n");
	return false;
}

int dc_comm_get_mode_idx(int mode)
{
	if (!dc_comm_mode_is_valid(mode))
		return -EINVAL;

	return g_dc_map_tbl[mode].idx;
}

const char *dc_comm_get_mode_name(int mode)
{
	if (!dc_comm_mode_is_valid(mode))
		return "invalid mode";

	return g_dc_map_tbl[mode].name;
}

int dc_comm_get_mode_ratio(int mode)
{
	if (!dc_comm_mode_is_valid(mode))
		return -EINVAL;

	return g_dc_map_tbl[mode].ratio;
}

void dc_comm_set_mode_ratio(int mode, int ratio)
{
	if (!dc_comm_mode_is_valid(mode)) {
		hwlog_err("invalid mode\n");
		return;
	}

	g_dc_map_tbl[mode].ratio = ratio;
}

static struct dc_common_dev g_dc_comm_di;
static struct dc_common_dev *dc_comm_get_di(void)
{
	return &g_dc_comm_di;
}

void dc_comm_set_need_recheck_flag(bool flag)
{
	struct dc_common_dev *di = dc_comm_get_di();

	if (!di)
		return;

	di->need_recheck_flag = flag;
	hwlog_info("set need_recheck_flag=%d\n", flag);
}

bool dc_comm_get_need_recheck_flag(void)
{
	struct dc_common_dev *di = dc_comm_get_di();

	if (!di)
		return false;

	return di->need_recheck_flag;
}

void dc_comm_set_first_check_completed(bool flag)
{
	struct dc_common_dev *di = dc_comm_get_di();

	if (!di)
		return;

	di->first_check_completed = flag;
	hwlog_info("set first_check_completed=%d\n", flag);
}

bool dc_comm_get_first_check_completed(void)
{
	struct dc_common_dev *di = dc_comm_get_di();

	if (!di)
		return false;

	return di->first_check_completed;
}
