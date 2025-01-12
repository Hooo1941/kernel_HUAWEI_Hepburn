/* SPDX-License-Identifier: GPL-2.0 */
/*
 * wireless_ic_iout.h
 *
 * ic iout for wireless charging
 *
 * Copyright (c) 2021-2021 Huawei Technologies Co., Ltd.
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

#ifndef _WIRELESS_IC_IOUT_H_
#define _WIRELESS_IC_IOUT_H_

#include <linux/bitops.h>
#include <linux/slab.h>

#define WLIC_DEFAULT_RX_IMAX       1350
#define WLIC_DEFAULT_RX_IMAX_IC    5000

enum wlic_iout_factor {
	WLICI_FACTOR_BEGIN = 0,
	WLICI_FACTOR_TURBO = WLICI_FACTOR_BEGIN,
	WLICI_FACTOR_END,
};

struct device_node;

#ifdef CONFIG_WIRELESS_CHARGER
void wlic_iout_start_sample(unsigned int ic_type);
void wlic_iout_stop_sample(unsigned int ic_type);
void wlic_iout_select_para(unsigned int ic_type, unsigned int factor, bool flag);
void wlic_get_rx_imax(unsigned int ic_type, int *rx_imax);
void wlic_get_rx_iavg(unsigned int ic_type, int *rx_iavg);
void wlic_iout_init(unsigned int ic_type, const struct device_node *np, int *imax_ic);
void wlic_iout_deinit(unsigned int ic_type);
void wlic_iout_set_cur_pmode_name(unsigned int ic_type, const char *cur_pmode_name);
#else
static inline void wlic_iout_start_sample(unsigned int ic_type)
{
}

static inline void wlic_iout_stop_sample(unsigned int ic_type)
{
}

static inline void wlic_iout_select_para(unsigned int ic_type, unsigned int factor, bool flag)
{
}

static inline void wlic_get_rx_imax(unsigned int ic_type, int *rx_imax)
{
}

static inline void wlic_get_rx_iavg(unsigned int ic_type, int *rx_iavg)
{
}

static inline void wlic_iout_init(unsigned int ic_type, const struct device_node *np, int *imax_ic)
{
}

static inline void wlic_iout_deinit(unsigned int ic_type)
{
}

static inline void wlic_iout_set_cur_pmode_name(unsigned int ic_type, const char *cur_pmode_name)
{
}
#endif /* CONFIG_WIRELESS_CHARGER */

#endif /* _WIRELESS_IC_IOUT_H_ */
