/* SPDX-License-Identifier: GPL-2.0 */
/*
 * wireless_tx_dts.h
 *
 * parse dts head file for wireless reverse charging
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

#ifndef _WIRELESS_TX_DTS_H_
#define _WIRELESS_TX_DTS_H_

#define WLTX_DFLT_ILIM_COL          5
#define WLTX_DFLT_ILIM_ROW          9

struct wltx_high_pwr_th {
	int hi_pwr_vth;
	int hi_pwr_ith;
	int hi_pwr2_vth;
	int hi_pwr2_ith;
};

struct wltx_dflt_ilim {
	int pwr_src;
	int vin_min;
	int vin_max;
	int ilim;
	int imax;
};

struct wltx_whitelist_ilim {
	int pwr_src;
	int rx_id;
	int fop_min;
	int fop_max;
	int ilim;
	int imax;
};

/*
 * vbus_change_type:WLTX_VBUS_CHANGED_BY_WIRED_CHSW abandoned
 * since 2021 because of some complex bugs
 */
struct wltx_dts {
	int pwr_type;
	int high_pwr_soc;
	int vbus_change_type;
	int ss_low_th;
	int pwr_low_th;
	int vbus_5vbst_exclusive;
	int bst5v_need_hiz_en;
	int support_id_prevchk;
	u8 product_id;
	struct wltx_high_pwr_th *high_pwr_th_cfg;
	int dflt_ilim_cfg_level;
	struct wltx_dflt_ilim dflt_ilim_cfg[WLTX_DFLT_ILIM_ROW];
	int whitelist_ilim_level;
	struct wltx_whitelist_ilim *whitelist_ilim;
	int prevfod1_product_list_len;
	u8 *prevfod1_product_list;
};

struct device_node;

#ifdef CONFIG_WIRELESS_CHARGER
struct wltx_dts *wltx_get_dts(void);
int wltx_parse_dts(const struct device_node *np);
void wltx_kfree_dts(void);
#else
static inline struct wltx_dts *wltx_get_dts(void)
{
	return NULL;
}

static inline int wltx_parse_dts(const struct device_node *np)
{
	return -EINVAL;
}

static inline void wltx_kfree_dts(void)
{
}
#endif /* CONFIG_WIRELESS_CHARGER */

#endif /* _WIRELESS_TX_DTS_H_ */
