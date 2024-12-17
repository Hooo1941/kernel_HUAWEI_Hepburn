// SPDX-License-Identifier: GPL-2.0
/*
 * direct_charge_vote.h
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

#ifndef _DIRECT_CHARGE_VOTE_H_
#define _DIRECT_CHARGE_VOTE_H_

#include <chipset_common/hwpower/charger/charge_common_vote.h>

int dc_vote_get_effective_result_with_lock(int obj);
int dc_vote_set(int obj, int client, bool en, int val);
void dc_vote_clear(int obj);

#endif /* _DIRECT_CHARGE_VOTE_H_ */
