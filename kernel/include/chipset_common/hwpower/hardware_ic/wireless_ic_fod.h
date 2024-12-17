/* SPDX-License-Identifier: GPL-2.0 */
/*
 * wireless_ic_fod.h
 *
 * head file for wireless hardware ic fod
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

#ifndef _WIRELESS_IC_FOD_H_
#define _WIRELESS_IC_FOD_H_

#include <linux/errno.h>
#include <linux/types.h>

#define WLTRX_FOD_CFG_ROW          30
#define WLTX_FOD_STATUS_FMT_OLD    0
#define WLTX_FOD_STATUS_FMT_NEW    1

enum wlrx_fod_cond {
	WLRX_FOD_COND_BEGIN = 0,
	WLRX_FOD_COND_ID = WLRX_FOD_COND_BEGIN,
	WLRX_FOD_COND_RX_SCN,
	WLRX_FOD_COND_TX_TYPE,
	WLRX_FOD_COND_VFC,
	WLRX_FOD_COND_END,
};

struct wlrx_fod_cfg {
	int id;
	int scn;
	int tx_type;
	int vfc; /* unit: mV */
};

struct wlrx_fod_ploss {
	int rx_fod_cfg_row;
	struct wlrx_fod_cfg *ploss_cond;
	u8 *ploss_th;
	int ploss_len;
};

enum wltx_fod_cond {
	WLTX_FOD_COND_BEGIN = 0,
	WLTX_FOD_COND_ID = WLTX_FOD_COND_BEGIN,
	WLTX_FOD_COND_RX_TYPE,
	WLTX_FOD_COND_RSVD,
	WLTX_FOD_COND_END,
};

struct wltx_ic_fod_cfg {
	int id;
	int rx_type;
	int rsvd;
};

struct wltx_fod_ploss {
	int tx_fod_cfg_row;
	struct wltx_ic_fod_cfg *ploss_cond;
	u8 *ploss_th;
};

#ifdef CONFIG_WIRELESS_CHARGER
int wlrx_parse_fod_cfg(unsigned int ic_type, const struct device_node *np, u16 fod_len);
u8 *wlrx_get_fod_ploss_th(unsigned int ic_type, int vfc_cfg, unsigned int tx_type, u16 fod_len);
int wltx_parse_fod_cfg(unsigned int ic_type, const struct device_node *np, u16 fod_len);
u8 *wltx_get_fod_ploss_th(unsigned int ic_type, int rx_type, u16 fod_len);
#else
static inline int wlrx_parse_fod_cfg(unsigned int ic_type, const struct device_node *np,
	u16 fod_len)
{
	return -ENOTSUPP;
}

static inline u8 *wlrx_get_fod_ploss_th(unsigned int ic_type, int vfc_cfg, unsigned int tx_type,
	u16 fod_len)
{
	return NULL;
}

static inline int wltx_parse_fod_cfg(unsigned int ic_type, const struct device_node *np, u16 fod_len)
{
	return -ENOTSUPP;
}

static inline u8 *wltx_get_fod_ploss_th(unsigned int ic_type, int rx_type, u16 fod_len)
{
	return NULL;
}
#endif /* CONFIG_WIRELESS_CHARGER */

#endif /* _WIRELESS_IC_FOD_H_ */
