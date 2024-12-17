/* SPDX-License-Identifier: GPL-2.0 */
/*
 * wireless_rx_pctrl.h
 *
 * power control for wireless charging
 *
 * Copyright (c) 2020-2021 Huawei Technologies Co., Ltd.
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

#ifndef _WIRELESS_RX_PCTRL_H_
#define _WIRELESS_RX_PCTRL_H_

struct wlrx_pctrl {
	int vtx;
	int vrx;
	int irx;
};

#ifdef CONFIG_WIRELESS_CHARGER
struct wlrx_pctrl *wlrx_pctrl_get_product_cfg(unsigned int drv_type);
void wlrx_pctrl_update_para(unsigned int drv_type, struct wlrx_pctrl *pctrl);
int wlrx_pctrl_get_product_pmax(unsigned int drv_type);
int wlrx_pctrl_get_rx_ilimit(unsigned int drv_type);
void wlrx_pctrl_handle_cm_ctrl(unsigned int drv_type, u8 type, u8 val);
void wlrx_pctrl_count_dcdiscon(unsigned int drv_type, bool count_flag);
u8 wlrx_pctrl_get_cm_ctrl_src(unsigned int drv_type);
int wlrx_pctrl_get_fix_fop(unsigned int drv_type);
#else
static inline struct wlrx_pctrl *wlrx_pctrl_get_product_cfg(unsigned int drv_type)
{
	return NULL;
}

static inline void wlrx_pctrl_update_para(unsigned int drv_type, struct wlrx_pctrl *pctrl)
{
}

static inline int wlrx_pctrl_get_product_pmax(unsigned int drv_type)
{
	return 0;
}

static inline int wlrx_pctrl_get_rx_ilimit(unsigned int drv_type)
{
	return 0;
}

static inline void wlrx_pctrl_handle_cm_ctrl(unsigned int drv_type, u8 type, u8 val)
{
}

static inline void wlrx_pctrl_count_dcdiscon(unsigned int drv_type, bool count_flag)
{
}

static inline u8 wlrx_pctrl_get_cm_ctrl_src(unsigned int drv_type)
{
	return 0;
}

static inline int wlrx_pctrl_get_fix_fop(unsigned int drv_type)
{
	return -EINVAL;
}

#endif /* CONFIG_WIRELESS_CHARGER */

#endif /* _WIRELESS_RX_PCTRL_H_ */
