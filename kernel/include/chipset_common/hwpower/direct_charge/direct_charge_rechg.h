// SPDX-License-Identifier: GPL-2.0
/*
 * direct_recharge.h
 *
 * direct recharge module
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

#ifndef _DIRECT_RECHARGE_H_
#define _DIRECT_RECHARGE_H_

#define DC_RECHARGE_ENABLE               1
#define DC_RECHARGE_DISABLE              0

#ifdef CONFIG_DIRECT_CHARGER
bool dc_rechg_is_support(void);
bool dc_rechg_is_en(void);
bool dc_rechg_is_enforce_en(void);
void dc_rechg_enforce_en(void);
int dc_rechg_get_ibat(void);
void dc_rechg_set_enable(int enable);
void dc_rechg_regulation(int lim_ibus, int target_ibat);
bool dc_rechg_is_ibus_support(int ibus);
bool dc_rechg_is_ibat_th_support(int ibat_th);
#else
static inline bool dc_rechg_is_support(void)
{
	return false;
}

static inline bool dc_rechg_is_en(void)
{
	return false;
}

static inline bool dc_rechg_is_enforce_en(void)
{
	return false;
}

static inline void dc_rechg_enforce_en(void)
{
}

static int void dc_rechg_get_ibat(void)
{
	return 0;
}

static inline void dc_rechg_set_enable(int enable)
{
}

static inline void dc_rechg_regulation(int lim_ibus, int target_ibat)
{
}

static inline bool dc_rechg_is_ibus_support(int ibus)
{
	return false;
}

static inline bool dc_rechg_is_ibat_th_support(int ibat_th)
{
	return false;
}

#endif /* CONFIG_DIRECT_CHARGER */

#endif /* _DIRECT_RECHARGE_H_ */
