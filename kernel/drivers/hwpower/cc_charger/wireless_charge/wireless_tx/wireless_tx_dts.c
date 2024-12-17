// SPDX-License-Identifier: GPL-2.0
/*
 * wireless_tx_dts.c
 *
 * parse dts for wireless reverse charging
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

#include <linux/slab.h>
#include <chipset_common/hwpower/wireless_charge/wireless_tx_dts.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/wireless_charge/wireless_tx_pwr_ctrl.h>
#include <chipset_common/hwpower/wireless_charge/wireless_tx_ic_intf.h>

#define HWLOG_TAG wireless_tx_dts
HWLOG_REGIST();

/* for basic config */
#define WLTX_HI_PWR_SOC_LTH             90
#define WLTX_HI_PWR_TH_CFG_LEN          4
#define WLTX_HI_PWR_VTH                 5000 /* mV */
#define WLTX_HI_PWR2_VTH                9000 /* mV */
#define WLTX_HI_PWR_ITH                 4000 /* mA */
#define WLTX_PWR_LOW_TH_DFLT            0

static struct wltx_dts *g_wltx_dts;

struct wltx_dts *wltx_get_dts(void)
{
	return g_wltx_dts;
}

static int wltx_parse_basic_cfg(const struct device_node *np, struct wltx_dts *dts)
{
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"pwr_type", &dts->pwr_type, WL_TX_PWR_VBUS_OTG);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"vbus_change_type", &dts->vbus_change_type,
		WLTX_VBUS_CHANGED_BY_WIRED_CHSW);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"tx_high_pwr_soc", &dts->high_pwr_soc, WLTX_HI_PWR_SOC_LTH);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"ss_low_th", &dts->ss_low_th, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"pwr_low_th", &dts->pwr_low_th, WLTX_PWR_LOW_TH_DFLT);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"vbus_5vbst_exclusive", &dts->vbus_5vbst_exclusive, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"bst5v_need_hiz_en", &dts->bst5v_need_hiz_en, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"support_id_prevchk", &dts->support_id_prevchk, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"product_id", (u32 *)&dts->product_id, 0);

	return 0;
}

static int wltx_parse_high_pwr_th(const struct device_node *np, struct wltx_dts *dts)
{
	int size, len;

	size = sizeof(*dts->high_pwr_th_cfg);
	dts->high_pwr_th_cfg = kzalloc(size, GFP_KERNEL);
	if (!dts->high_pwr_th_cfg)
		return -ENOMEM;

	len = of_property_count_u32_elems(np, "wltx_hi_pwr_th");
	if (len != WLTX_HI_PWR_TH_CFG_LEN)
		goto parse_high_pwr_th_err;

	if (power_dts_read_u32_array(power_dts_tag(HWLOG_TAG), np,
		"wltx_hi_pwr_th", (u32 *)dts->high_pwr_th_cfg, len))
		goto parse_high_pwr_th_err;

	goto print_para;

parse_high_pwr_th_err:
	dts->high_pwr_th_cfg->hi_pwr_vth = WLTX_HI_PWR_VTH;
	dts->high_pwr_th_cfg->hi_pwr_ith = WLTX_HI_PWR_ITH;
	dts->high_pwr_th_cfg->hi_pwr2_vth = WLTX_HI_PWR2_VTH;
	dts->high_pwr_th_cfg->hi_pwr2_ith = WLTX_HI_PWR_ITH;


print_para:
	hwlog_info("[parse_high_pwr_th] hi_pwr_vth:%d hi_pwr_ith:%d hi_pwr2_vth:%d hi_pwr2_ith:%d\n",
		dts->high_pwr_th_cfg->hi_pwr_vth, dts->high_pwr_th_cfg->hi_pwr_ith,
		dts->high_pwr_th_cfg->hi_pwr2_vth, dts->high_pwr_th_cfg->hi_pwr2_ith);
	return 0;
}

static void wltx_parse_dflt_ilim(struct wltx_dts *dts)
{
	int i;
	int len;
	struct device_node *np = wltx_ic_get_dev_node(WLTRX_IC_MAIN);

	if (!np)
		return;

	len = power_dts_read_string_array(power_dts_tag(HWLOG_TAG), np,
		"tx_dflt_ilim", (int *)dts->dflt_ilim_cfg,
		WLTX_DFLT_ILIM_ROW, WLTX_DFLT_ILIM_COL);
	if (len <= 0) {
		dts->dflt_ilim_cfg_level = 0;
		return;
	}

	dts->dflt_ilim_cfg_level = len / WLTX_DFLT_ILIM_COL;
	for (i = 0; i < dts->dflt_ilim_cfg_level; i++)
		hwlog_info("[dflt_ilim_cfg][%d] pwr_src:%d vin_min:%d vin_max:%d ilim:%d imax:%d\n",
			i, dts->dflt_ilim_cfg[i].pwr_src, dts->dflt_ilim_cfg[i].vin_min,
			dts->dflt_ilim_cfg[i].vin_max, dts->dflt_ilim_cfg[i].ilim,
			dts->dflt_ilim_cfg[i].imax);
}

static void wltx_parse_whitelist_ilim(struct wltx_dts *dts)
{
	int i;

	if (power_dts_read_dynamic_intstr_array(power_dts_tag(HWLOG_TAG),
		wltx_ic_get_dev_node(WLTRX_IC_MAIN),
		"tx_whitelist_ilim", (int **)&dts->whitelist_ilim,
		(int)(sizeof(*(dts->whitelist_ilim)) / sizeof(int)),
		&dts->whitelist_ilim_level))
		return;

	for (i = 0; i < dts->whitelist_ilim_level; i++)
		hwlog_info("[whitelist_ilim][%d] pwr_src:%d rx_product_id:0x%x "
			"fop_min=%d fop_max=%d ilim:%d imax:%d\n", i,
			dts->whitelist_ilim[i].pwr_src, (u8)dts->whitelist_ilim[i].rx_id,
			dts->whitelist_ilim[i].fop_min, dts->whitelist_ilim[i].fop_max,
			dts->whitelist_ilim[i].ilim, dts->whitelist_ilim[i].imax);
}

static void wltx_parse_prevfod1_product_list(struct wltx_dts *dts)
{
	int i, len;
	struct device_node *np = wltx_ic_get_dev_node(WLTRX_IC_MAIN);

	if (!np)
		return;

	len = of_property_count_u8_elems(np, "prevfod1_product_list");
	if (len <= 0)
		return;

	dts->prevfod1_product_list = kzalloc(len * sizeof(*(dts->prevfod1_product_list)), GFP_KERNEL);
	if (!dts->prevfod1_product_list)
		return;

	if (power_dts_read_u8_array(power_dts_tag(HWLOG_TAG), np,
		"prevfod1_product_list", dts->prevfod1_product_list, len)) {
		kfree(dts->prevfod1_product_list);
		return;
	}

	dts->prevfod1_product_list_len = len;
	power_print_u8_array("prevfod1_product_list", dts->prevfod1_product_list, 1, dts->prevfod1_product_list_len);
}

void wltx_kfree_dts(void)
{
	if (!g_wltx_dts)
		return;

	if (g_wltx_dts->prevfod1_product_list)
		kfree(g_wltx_dts->prevfod1_product_list);
	if (g_wltx_dts->whitelist_ilim)
		kfree(g_wltx_dts->whitelist_ilim);
	kfree(g_wltx_dts);
	g_wltx_dts = NULL;
}

int wltx_parse_dts(const struct device_node *np)
{
	int ret;
	struct wltx_dts *dts = NULL;

	if (!np)
		return -EINVAL;

	dts = kzalloc(sizeof(*dts), GFP_KERNEL);
	if (!dts)
		return -ENOMEM;

	g_wltx_dts = dts;
	ret = wltx_parse_basic_cfg(np, dts);
	if (ret)
		goto exit;
	ret = wltx_parse_high_pwr_th(np, dts);
	if (ret)
		goto exit;
	wltx_parse_dflt_ilim(dts);
	wltx_parse_whitelist_ilim(dts);
	wltx_parse_prevfod1_product_list(dts);

	return 0;

exit:
	wltx_kfree_dts();
	return -EINVAL;
}
