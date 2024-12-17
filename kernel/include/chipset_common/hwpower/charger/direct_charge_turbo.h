/* SPDX-License-Identifier: GPL-2.0 */
/*
 * direct_charge_turbo.h
 *
 * direct charge turbo module
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

#ifndef _DIRECT_CHARGE_TURBO_H_
#define _DIRECT_CHARGE_TURBO_H_

#include <chipset_common/hwpower/direct_charge/direct_charger.h>

#ifdef CONFIG_DIRECT_CHARGER
int dc_turbo_get_time_para(int mode, struct direct_charge_time_para **para, int *size, int adap_mode);
int dc_turbo_get_charge_time(int mode, int *time, int adap_mode);
void dc_turbo_send_max_power(int value);
#else
static inline int dc_turbo_get_time_para(int mode, struct direct_charge_time_para **para, int *size, int adap_mode)
{
	return -1;
}

static inline int dc_turbo_get_charge_time(int mode, int *time, int adap_mode)
{
	return -1;
}

static inline void dc_turbo_send_max_power(int value)
{
}
#endif /* CONFIG_DIRECT_CHARGER */

#endif /* _DIRECT_CHARGE_TURBO_H_ */
