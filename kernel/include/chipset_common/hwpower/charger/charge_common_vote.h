// SPDX-License-Identifier: GPL-2.0
/*
 * charge_common_vote.h
 *
 * charge vote common driver
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

#ifndef _CHARGE_COMMON_VOTE_H_
#define _CHARGE_COMMON_VOTE_H_

#include <linux/io.h>

enum vote_object_type {
	VOTE_OBJ_TYPE_BEGIN,
	VOTE_OBJ_TYPE_BUCK = VOTE_OBJ_TYPE_BEGIN,
	VOTE_OBJ_TYPE_DC,
	VOTE_OBJ_TYPE_END,
};

enum vote_object {
	VOTE_OBJ_BEGIN,
	VOTE_OBJ_FCC = VOTE_OBJ_BEGIN,
	VOTE_OBJ_USB_ICL,
	VOTE_OBJ_VTERM,
	VOTE_OBJ_ITERM,
	VOTE_OBJ_DIS_CHG,
	VOTE_OBJ_IBUS,
	VOTE_OBJ_CABLE_CURR,
	VOTE_OBJ_ADPT_CURR,
	VOTE_OBJ_END,
};

enum vote_client {
	VOTE_CLIENT_BEGIN,
	VOTE_CLIENT_THERMAL = VOTE_CLIENT_BEGIN,
	VOTE_CLIENT_USER,
	VOTE_CLIENT_JEITA,
	VOTE_CLIENT_BASP,
	VOTE_CLIENT_WARM_WR,
	VOTE_CLIENT_RT,
	VOTE_CLIENT_FCP,
	VOTE_CLIENT_WLS,
	VOTE_CLIENT_FFC,
	VOTE_CLIENT_SMT_BATT,
	VOTE_CLIENT_IBUS_DETECT,
	VOTE_CLIENT_POWER_IF,
	VOTE_CLIENT_RSMC,
	VOTE_CLIENT_DC,
	/* direct charge */
	VOTE_CLIENT_CABLE,
	VOTE_CLIENT_ADAPTER,
	VOTE_CLIENT_TIME,
	VOTE_CLIENT_CABLE_TYPE,
	VOTE_CLIENT_CABLE_1ST_RES,
	VOTE_CLIENT_CABLE_2ND_RES,
	VOTE_CLIENT_ADPT_IWATT,
	VOTE_CLIENT_ADPT_ANTIFAKE,
	VOTE_CLIENT_ADPT_TIME,
	VOTE_CLIENT_ADPT_MAX_CURR,
	VOTE_CLIENT_END,
};

int chg_vote_set(unsigned int obj_type, unsigned int obj, unsigned int client, bool en, int val);
void chg_vote_clear(unsigned int obj_type, unsigned int obj);
int chg_vote_get_effective_result_with_lock(unsigned int obj_type, unsigned int obj);
int chg_vote_get_effective_result_without_lock(unsigned int obj_type, unsigned int obj);
bool chg_vote_is_client_enabled_locked(unsigned int obj_type, unsigned int obj, unsigned int client, bool lock_flag);

#endif /* _CHARGE_COMMON_VOTE_H_ */
