/*
 * huawei_charger_common.h
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

#ifndef _HUAWEI_CHARGER_COMMON_H_
#define _HUAWEI_CHARGER_COMMON_H_

#include <linux/types.h>
#include <chipset_common/hwpower/charger/charger_common_interface.h>

struct charge_core_data;

bool charge_check_charger_ts(void);
bool charge_check_charger_otg_state(void);
int charge_set_charge_state(int state);
int get_charge_current_max(void);
struct charge_core_data *charge_core_get_params(void);

#endif /* _HUAWEI_CHARGER_COMMON_H_ */
