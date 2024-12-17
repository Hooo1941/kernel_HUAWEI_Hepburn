// SPDX-License-Identifier: GPL-2.0
/*
 * wireless_ic_fod.c
 *
 * ic fod for wireless charging
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
#include <chipset_common/hwpower/hardware_ic/wireless_ic_fod.h>
#include <chipset_common/hwpower/hardware_ic/wireless_ic_debug.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_fod.h>
#include <chipset_common/hwpower/wireless_charge/wireless_trx_ic_intf.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_common.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/common_module/power_dts.h>

#define HWLOG_TAG wireless_ic_fod
HWLOG_REGIST();

#define WLTRX_FOD_TMP_STR_LEN      16
#define WLRX_FOD_TMP_BUF_SIZE      40
#define WLRX_FOD_PLOSS_LEN_MAX     32

static struct wlrx_fod_ploss *g_rx_fod_ploss[WLTRX_IC_MAX];
static struct wltx_fod_ploss *g_tx_fod_ploss[WLTRX_IC_MAX];

static struct wlrx_fod_ploss *wlrx_get_dts_fod_ploss(unsigned int ic_type)
{
	if (!wltrx_ic_is_type_valid(ic_type))
		return NULL;

	return g_rx_fod_ploss[ic_type];
}

u8 *wlrx_get_fod_ploss_th(unsigned int ic_type, int vfc_cfg, unsigned int tx_type, u16 fod_len)
{
	int i;
	u8 *rx_fod = NULL;
	enum wlrx_scene scn_id;
	struct wlrx_fod_ploss *rx_ploss = wlrx_get_dts_fod_ploss(ic_type);

	if (!rx_ploss)
		return NULL;

	scn_id = wlrx_get_scene();
	if ((scn_id < 0) || (scn_id >= WLRX_SCN_END))
		return NULL;

	for (i = 0; i < rx_ploss->rx_fod_cfg_row; i++) {
		if (rx_ploss->ploss_cond[i].vfc != vfc_cfg)
			continue;
		if ((rx_ploss->ploss_cond[i].scn >= 0) && (rx_ploss->ploss_cond[i].scn != scn_id))
			continue;
		if ((rx_ploss->ploss_cond[i].tx_type >= 0) &&
			(rx_ploss->ploss_cond[i].tx_type != tx_type))
			continue;
		break;
	}
	hwlog_info("[wlrx_get_fod_ploss_th] id=%d scn_id=%d tx_type=%u vfc_cfg=%d\n",
		i, scn_id, tx_type, vfc_cfg);
	if (i >= rx_ploss->rx_fod_cfg_row) {
		hwlog_err("wlrx_get_fod_ploss_th: ploss_th mismatch\n");
		return NULL;
	}

	rx_fod = rx_ploss->ploss_th + i * fod_len;
	power_print_u8_array("rx_ploss_th", rx_fod, 1, fod_len); /* row=1 */
	return rx_fod;
}

static int wlrx_debug_set_fod_ploss(struct wlrx_fod_ploss *di, int vfc, u8 *ploss, int ploss_len)
{
	int i, j;
	u8 *rx_fod = NULL;

	for (i = 0; i < di->rx_fod_cfg_row; i++) {
		if (di->ploss_cond[i].vfc != vfc)
			continue;
		di->ploss_cond[i].scn = -1;
		di->ploss_cond[i].tx_type = -1;
		break;
	}

	if (i >= di->rx_fod_cfg_row) {
		hwlog_err("set_fod_ploss: ploss_th mismatch\n");
		return -EINVAL;
	}

	rx_fod = di->ploss_th + i * ploss_len;
	for (j = 0; j < ploss_len; j++)
		rx_fod[j] = ploss[j];

	hwlog_info("[set_fod_ploss] id=%d vfc=%d\n", i, vfc);
	power_print_u8_array("rx_ploss_th", rx_fod, 1, ploss_len); /* row=1 */
	return 0;
}

static int wlrx_debug_get_fod_ploss(struct wlrx_fod_ploss *di, char *buf, int len)
{
	int i, j;
	int ret = 0;

	ret += snprintf_s(buf + ret, len - ret, len - ret - 1, "id scn tx_type vfc\n");
	for (i = 0; i < di->rx_fod_cfg_row; i++)
		ret += snprintf_s(buf + ret, len - ret, len - ret - 1, "%d %d %d %d\n",
			di->ploss_cond[i].id, di->ploss_cond[i].scn,
			di->ploss_cond[i].tx_type, di->ploss_cond[i].vfc);

	for (i = 0; i < di->rx_fod_cfg_row; i++) {
		for (j = 0; j < di->ploss_len; j++)
			ret += snprintf_s(buf + ret, len - ret, len - ret - 1,
				"0x%x ", di->ploss_th[i * di->ploss_len + j]);
		ret += snprintf_s(buf + ret, len - ret, len - ret - 1, "\n");
	}

	return ret;
}

static int wlrx_debug_ploss_handle_input(const char *buf,
	u8 *ploss, int size, int ploss_len, int *vfc)
{
	int i = 0;
	int j;
	int temp[WLRX_FOD_TMP_BUF_SIZE] = { 0 };
	char *token = NULL;
	char *next_token = NULL;
	char *p = (char *)buf;

	hwlog_info("[ploss_handle_input] input %s\n", buf);
	token = strtok_s(p, " ", &next_token);
	while (token != NULL) {
		if (kstrtol(token, POWER_BASE_HEX, (long *)(&(temp[i]))))
			return -EINVAL;
		if (i == 0 && temp[i] != WLTRX_UNLOCK_VAL)
			return -EINVAL;
		i++;
		if (i >= WLRX_FOD_TMP_BUF_SIZE)
			return -EINVAL;
		token = strtok_s(NULL, " ", &next_token);
	}

	hwlog_info("[ploss_handle_input] after parse:\n");
	for (j = 0; j < i; j++)
		hwlog_info("0x%x ", temp[j]);

	hwlog_info("\n");
	*vfc = temp[1]; /* 1: vfc is second input */
	 /* i - 2: input ploss length */
	if ((i - 2 != ploss_len) || (ploss_len > size)) {
		hwlog_err("ploss_handle_input: input ploss len invalid %d", i - 2);
		return -EINVAL;
	}

	for (j = 0; j < ploss_len; j++)
		ploss[j] = (u8)temp[j + 2]; /* 2: remove the first two input */

	return 0;
}

static ssize_t wlrx_debug_fod_ploss_store(void *dev_data, const char *buf, size_t size)
{
	int vfc = 0;
	int ret;
	int ploss_len;
	u8 ploss[WLRX_FOD_PLOSS_LEN_MAX] = { 0 };
	struct wlrx_fod_ploss *di = NULL;

	di = (struct wlrx_fod_ploss *)dev_data;
	if (!di) {
		hwlog_err("ploss_store: di null\n");
		return -EINVAL;
	}

	ploss_len = di->ploss_len;
	ret = wlrx_debug_ploss_handle_input(buf, ploss,
		WLRX_FOD_PLOSS_LEN_MAX, ploss_len, &vfc);
	if (ret)
		return -EINVAL;

	ret = wlrx_debug_set_fod_ploss(di, vfc, ploss, ploss_len);
	if (ret)
		hwlog_err("ploss_store: set fail\n");

	return (ssize_t)size;
}

static ssize_t wlrx_debug_fod_ploss_show(void *dev_data, char *buf, size_t size)
{
	int ret;
	struct wlrx_fod_ploss *di = NULL;

	di = (struct wlrx_fod_ploss *)dev_data;
	if (!di) {
		hwlog_err("fod_ploss_show: di null\n");
		return -EINVAL;
	}

	ret = wlrx_debug_get_fod_ploss(di, buf, size);
	return ret;
}

static void wlrx_fod_ploss_debug_register(struct wlrx_fod_ploss *di, unsigned int ic_type)
{
	char dir_name[WLTRX_DEBUG_DIR_LEN] = {0};

	if (wlic_dbg_rx_get_dirname(ic_type, dir_name, WLTRX_DEBUG_DIR_LEN) <= 0)
		return;

	power_dbg_ops_register(dir_name, "ploss", di, wlrx_debug_fod_ploss_show,
		wlrx_debug_fod_ploss_store);
}

static int wlrx_parse_fod_ploss(unsigned int ic_type, const struct device_node *np,
	struct wlrx_fod_ploss *di, u16 fod_len)
{
	int i;
	char buffer[WLTRX_FOD_TMP_STR_LEN] = { 0 };

	di->ploss_th = kcalloc(di->rx_fod_cfg_row, fod_len, GFP_KERNEL);
	if (!di->ploss_th)
		return -ENOMEM;

	for (i = 0; i < di->rx_fod_cfg_row; i++) {
		snprintf(buffer, WLTRX_FOD_TMP_STR_LEN - 1, "rx_ploss_th%d", i);
		if (power_dts_read_u8_array(power_dts_tag(HWLOG_TAG), np,
			buffer, di->ploss_th + i * fod_len, fod_len))
			goto parse_plossth_fail;
		memset(buffer, 0, WLTRX_FOD_TMP_STR_LEN);
	}

	power_print_u8_array("rx_ploss_th", di->ploss_th, di->rx_fod_cfg_row, fod_len);
	g_rx_fod_ploss[ic_type] = di;
	di->ploss_len = fod_len;
	return 0;

parse_plossth_fail:
	kfree(di->ploss_th);
	return -EINVAL;
}

int wlrx_parse_fod_cfg(unsigned int ic_type, const struct device_node *np, u16 fod_len)
{
	int i, len;
	struct wlrx_fod_ploss *di = NULL;

	if (!np || !wltrx_ic_is_type_valid(ic_type) || g_rx_fod_ploss[ic_type]) {
		hwlog_err("wlrx_parse_fod_cfg: ic_type=%u err or repeated\n", ic_type);
		return -EINVAL;
	}

	di = kzalloc(sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	len = power_dts_read_count_strings(power_dts_tag(HWLOG_TAG), np,
		"rx_fod_cond", WLTRX_FOD_CFG_ROW, WLRX_FOD_COND_END);
	if (len <= 0)
		goto parse_fod_fail;

	di->rx_fod_cfg_row = len / WLRX_FOD_COND_END;
	di->ploss_cond = kcalloc(di->rx_fod_cfg_row, sizeof(*(di->ploss_cond)), GFP_KERNEL);
	if (!di->ploss_cond)
		goto parse_fod_fail;

	len = power_dts_read_string_array(power_dts_tag(HWLOG_TAG), np,
		"rx_fod_cond", (int *)di->ploss_cond, di->rx_fod_cfg_row, WLRX_FOD_COND_END);
	if (len <= 0)
		goto parse_ploss_cond_fail;

	for (i = 0; i < di->rx_fod_cfg_row; i++)
		hwlog_info("rx_ploss_cond[%d] id=%d scn=%d tx_type=%d vfc=%d\n", i,
			di->ploss_cond[i].id, di->ploss_cond[i].scn,
			di->ploss_cond[i].tx_type, di->ploss_cond[i].vfc);

	if (wlrx_parse_fod_ploss(ic_type, np, di, fod_len))
		goto parse_ploss_cond_fail;

	wlrx_fod_ploss_debug_register(di, ic_type);
	return 0;

parse_ploss_cond_fail:
	kfree(di->ploss_cond);
parse_fod_fail:
	kfree(di);
	hwlog_err("wlrx_parse_fod_cfg: failed\n");
	return -EINVAL;
}

static struct wltx_fod_ploss *wltx_get_dts_fod_ploss(unsigned int ic_type)
{
	if (!wltrx_ic_is_type_valid(ic_type))
		return NULL;

	return g_tx_fod_ploss[ic_type];
}

u8 *wltx_get_fod_ploss_th(unsigned int ic_type, int rx_type, u16 fod_len)
{
	int i;
	u8 *tx_fod = NULL;
	struct wltx_fod_ploss *tx_ploss = wltx_get_dts_fod_ploss(ic_type);

	if (!tx_ploss)
		return NULL;

	for (i = 0; i < tx_ploss->tx_fod_cfg_row; i++) {
		if ((tx_ploss->ploss_cond[i].rx_type > 0) &&
			(tx_ploss->ploss_cond[i].rx_type != rx_type))
			continue;
		break;
	}

	hwlog_info("[wltx_get_fod_ploss_th] id=%d rx_type=%u\n", i, rx_type);
	if (i >= tx_ploss->tx_fod_cfg_row) {
		hwlog_err("wltx_get_fod_ploss_th: ploss_th mismatch\n");
		return NULL;
	}

	tx_fod = tx_ploss->ploss_th + i * fod_len;
	power_print_u8_array("tx_ploss_th", tx_fod, 1, fod_len); /* row=1 */
	return tx_fod;
}

static int wltx_parse_fod_ploss(unsigned int ic_type, const struct device_node *np,
	struct wltx_fod_ploss *di, u16 fod_len)
{
	int i;
	char buffer[WLTRX_FOD_TMP_STR_LEN] = { 0 };

	di->ploss_th = kcalloc(di->tx_fod_cfg_row, fod_len, GFP_KERNEL);
	if (!di->ploss_th)
		return -ENOMEM;

	for (i = 0; i < di->tx_fod_cfg_row; i++) {
		snprintf(buffer, WLTRX_FOD_TMP_STR_LEN - 1, "tx_ploss_th%d", i);
		if (power_dts_read_u8_array(power_dts_tag(HWLOG_TAG), np,
			buffer, di->ploss_th + i * fod_len, fod_len))
			goto parse_plossth_fail;
		memset(buffer, 0, WLTRX_FOD_TMP_STR_LEN);
	}

	power_print_u8_array("tx_ploss_th", di->ploss_th, di->tx_fod_cfg_row, fod_len);
	g_tx_fod_ploss[ic_type] = di;
	return 0;

parse_plossth_fail:
	kfree(di->ploss_th);
	return -EINVAL;
}

int wltx_parse_fod_cfg(unsigned int ic_type, const struct device_node *np, u16 fod_len)
{
	int i, len;
	struct wltx_fod_ploss *di = NULL;

	if (!np || !wltrx_ic_is_type_valid(ic_type) || g_tx_fod_ploss[ic_type]) {
		hwlog_err("wltx_parse_fod_cfg: ic_type=%u err or repeated\n", ic_type);
		return -EINVAL;
	}

	di = kzalloc(sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	len = power_dts_read_count_strings(power_dts_tag(HWLOG_TAG), np,
		"tx_fod_cond", WLTRX_FOD_CFG_ROW, WLTX_FOD_COND_END);
	if (len <= 0)
		goto parse_fod_fail;

	di->tx_fod_cfg_row = len / WLTX_FOD_COND_END;
	di->ploss_cond = kcalloc(di->tx_fod_cfg_row, sizeof(*(di->ploss_cond)), GFP_KERNEL);
	if (!di->ploss_cond)
		goto parse_fod_fail;

	len = power_dts_read_string_array(power_dts_tag(HWLOG_TAG), np,
		"tx_fod_cond", (int *)di->ploss_cond, di->tx_fod_cfg_row, WLTX_FOD_COND_END);
	if (len <= 0)
		goto parse_ploss_cond_fail;

	for (i = 0; i < di->tx_fod_cfg_row; i++)
		hwlog_info("tx_ploss_cond[%d] id=%d rx_type=%d\n", i,
			di->ploss_cond[i].id, di->ploss_cond[i].rx_type);

	if (!wltx_parse_fod_ploss(ic_type, np, di, fod_len))
		return 0;

parse_ploss_cond_fail:
	kfree(di->ploss_cond);
parse_fod_fail:
	kfree(di);
	hwlog_err("wltx_parse_fod_cfg: failed\n");
	return -EINVAL;
}
