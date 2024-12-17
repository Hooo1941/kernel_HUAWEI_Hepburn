// SPDX-License-Identifier: GPL-2.0
/*
 * direct_charge_vote.c
 *
 * vote interface of direct charge module
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

#include <chipset_common/hwpower/direct_charge/direct_charge_vote.h>
#include <chipset_common/hwpower/common_module/power_printk.h>

#define HWLOG_TAG direct_charge_vote
HWLOG_REGIST();

int dc_vote_get_effective_result_with_lock(int obj)
{
	return chg_vote_get_effective_result_with_lock(VOTE_OBJ_TYPE_DC, obj);
}

int dc_vote_set(int obj, int client, bool en, int val)
{
	return chg_vote_set(VOTE_OBJ_TYPE_DC, obj, client, en, val);
}

void dc_vote_clear(int obj)
{
	chg_vote_clear(VOTE_OBJ_TYPE_DC, obj);
}
