// SPDX-License-Identifier: GPL-2.0
/*
 * wireless_rx_pmode.c
 *
 * power mode for wireless charging
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

#include <chipset_common/hwpower/wireless_charge/wireless_rx_pmode.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_module.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_acc.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_pctrl.h>
#include <huawei_platform/hwpower/wireless_charge/wireless_rx_platform.h>
#include <chipset_common/hwpower/wireless_charge/wireless_dc_check.h>
#include <chipset_common/hwpower/battery/battery_temp.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/common_module/power_algorithm.h>
#include <chipset_common/hwpower/wireless_charge/wireless_dc_common.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_ic_intf.h>

#define HWLOG_TAG wireless_rx_pmode
HWLOG_REGIST();

/* for vmode_para */
#define WLRX_VMODE_CFG_ROW             5
#define WLRX_VMODE_CFG_COL             2

struct wlrx_pmode_dts {
	int pcfg_level;
	int vmode_cfg_level;
	struct wlrx_pmode *pcfg;
	struct wlrx_vmode *vmode_cfg;
};

struct wlrx_pmode_dev {
	int curr_pid;
	int curr_vid;
	struct wlrx_pmode_dts *dts;
};

static struct wlrx_pmode_dev *g_rx_pmode_di[WLTRX_DRV_MAX];

static unsigned int wlrx_pmode_get_ic_type(unsigned int drv_type)
{
	return wltrx_get_dflt_ic_type(drv_type);
}

static struct wlrx_pmode_dev *wlrx_pmode_get_di(unsigned int drv_type)
{
	if (!wltrx_is_drv_type_valid(drv_type)) {
		hwlog_err("get_di: drv_type=%u err\n", drv_type);
		return NULL;
	}

	return g_rx_pmode_di[drv_type];
}

int wlrx_pmode_select_vid(unsigned int drv_type, int vtx)
{
	int id = 0;
	struct wlrx_pmode_dev *di = wlrx_pmode_get_di(drv_type);

	if (!di || !di->dts) {
		hwlog_err("select_vid: di or dts is null\n");
		return id;
	}

	for (id = 0; id < di->dts->vmode_cfg_level; id++) {
		if (vtx == di->dts->vmode_cfg[id].vtx)
			break;
	}

	if (id >= di->dts->vmode_cfg_level) {
		id = 0;
		hwlog_err("select_vid: match vmode_index failed\n");
	}

	return id;
}

void wlrx_pmode_set_vid(unsigned int drv_type, int vtx)
{
	struct wlrx_pmode_dev *di = wlrx_pmode_get_di(drv_type);

	if (!di)
		return;

	di->curr_vid = wlrx_pmode_select_vid(drv_type, vtx);
	hwlog_info("[set_vid] id=%d\n", di->curr_vid);
}

int wlrx_pmode_get_vid(unsigned int drv_type)
{
	struct wlrx_pmode_dev *di = wlrx_pmode_get_di(drv_type);

	if (!di)
		return 0;

	return di->curr_vid;
}

int wlrx_pmode_get_vtx_by_id(unsigned int drv_type, int id)
{
	struct wlrx_pmode_dev *di = wlrx_pmode_get_di(drv_type);

	if (!di || !di->dts) {
		hwlog_err("get_vtx_by_id: di or dts is null\n");
		return 0;
	}

	if (id < 0 || id >= di->dts->vmode_cfg_level)
		id = 0;

	return di->dts->vmode_cfg[id].vtx;
}

int wlrx_pmode_get_curr_pid(unsigned int drv_type)
{
	struct wlrx_pmode_dev *di = wlrx_pmode_get_di(drv_type);

	if (!di)
		return 0;

	return di->curr_pid;
}

void wlrx_pmode_set_curr_pid(unsigned int drv_type, int pid)
{
	struct wlrx_pmode_dev *di = wlrx_pmode_get_di(drv_type);

	if (!di || !di->dts || (pid < 0) || (pid >= di->dts->pcfg_level))
		return;

	di->curr_pid = pid;
	wlrx_ic_set_cur_pmode_name(wlrx_pmode_get_ic_type(drv_type),
		di->dts->pcfg[pid].name);
	hwlog_info("[set_curr_pid] pid=%d\n", pid);
}

bool wlrx_pmode_is_pid_valid(unsigned int drv_type, int pid)
{
	struct wlrx_pmode_dev *di = wlrx_pmode_get_di(drv_type);

	if (!di || !di->dts || (pid < 0) || (pid >= di->dts->pcfg_level))
		return false;

	return true;
}

int wlrx_pmode_get_exp_pid(unsigned int drv_type, int pid)
{
	struct wlrx_pmode_dev *di = wlrx_pmode_get_di(drv_type);

	if (!di || !di->dts || !di->dts->pcfg ||
		(pid < 0) || (pid >= di->dts->pcfg_level))
		return 0;

	return di->dts->pcfg[pid].expect_mode;
}

struct wlrx_pmode *wlrx_pmode_get_pcfg_by_pid(unsigned int drv_type, int pid)
{
	struct wlrx_pmode_dev *di = wlrx_pmode_get_di(drv_type);

	if (!di || !di->dts || !di->dts->pcfg ||
		(pid < 0) || (pid >= di->dts->pcfg_level))
		return NULL;

	return &di->dts->pcfg[pid];
}

struct wlrx_pmode *wlrx_pmode_get_curr_pcfg(unsigned int drv_type)
{
	struct wlrx_pmode_dev *di = wlrx_pmode_get_di(drv_type);

	if (!di)
		return NULL;

	return wlrx_pmode_get_pcfg_by_pid(drv_type, di->curr_pid);
}

int wlrx_pmode_get_pcfg_level(unsigned int drv_type)
{
	struct wlrx_pmode_dev *di = wlrx_pmode_get_di(drv_type);

	if (!di || !di->dts)
		return 0;

	return di->dts->pcfg_level;
}

int wlrx_pmode_get_pid_by_name(unsigned int drv_type, const char *mode_name)
{
	int pid;
	struct wlrx_pmode_dev *di = wlrx_pmode_get_di(drv_type);

	if (!mode_name || !di || !di->dts || !di->dts->pcfg)
		return 0;

	for (pid = 0; pid < di->dts->pcfg_level; pid++) {
		if (power_strsame(mode_name, di->dts->pcfg[pid].name))
			return pid;
	}

	return 0;
}

bool wlrx_pmode_is_dc_mode_by_name(const char *mode_name)
{
	unsigned int dc_mode = wldc_get_mode_by_name(mode_name);

	return (dc_mode >= WLDC_MODE_BEGIN) && (dc_mode < WLDC_MODE_END);
}

bool wlrx_pmode_is_dc_mode_by_pid(unsigned int drv_type, int pid)
{
	struct wlrx_pmode_dev *di = wlrx_pmode_get_di(drv_type);

	if (!di || !di->dts || !di->dts->pcfg ||
		(pid < 0) || (pid >= di->dts->pcfg_level))
		return false;

	return wlrx_pmode_is_dc_mode_by_name(di->dts->pcfg[pid].name);
}

bool wlrx_pmode_in_dc_mode(unsigned int drv_type)
{
	struct wlrx_pmode_dev *di = wlrx_pmode_get_di(drv_type);

	if (!di)
		return false;

	return wlrx_pmode_is_dc_mode_by_pid(drv_type, di->curr_pid);
}

static bool wlrx_pmode_quick_judge(unsigned int drv_type, struct wlrx_pmode *pcfg)
{
	struct wlprot_acc_cap *acc_cap = wlrx_acc_get_cap(drv_type);
	struct wlrx_pctrl *product_pcfg = wlrx_pctrl_get_product_cfg(drv_type);

	if (!acc_cap || !product_pcfg)
		return false;

	if ((acc_cap->vmax < pcfg->vtx_min) || (product_pcfg->vtx < pcfg->vtx) ||
		(product_pcfg->vrx < pcfg->vrx) || (product_pcfg->irx < pcfg->irx))
		return false;
	if (acc_cap->vmax * acc_cap->imax < pcfg->vtx_min * pcfg->itx_min)
		return false;

	if (strstr(pcfg->name, WLRX_RVS_MODE_POSTFIX) && !wlrx_is_rvs_tx(drv_type))
		return false;

	if (strstr(pcfg->name, WLRX_12V_MODE_POSTFIX) && !wlrx_is_low_vpa_car_tx(drv_type))
		return false;

	if (strstr(pcfg->name, WLRX_15V_MODE_POSTFIX) && wlrx_is_low_vpa_car_tx(drv_type))
		return false;

	return true;
}

static bool wlrx_pmode_normal_judge(unsigned int drv_type, struct wlrx_pmode *pcfg)
{
	if ((pcfg->auth >= 0) && !wlrx_acc_auth_succ(drv_type))
		return false;

	return true;
}

static bool wlrx_pmode_final_judge(unsigned int drv_type, struct wlrx_pmode *pcfg)
{
	int tbatt = 0;
	struct wlprot_acc_cap *acc_cap = wlrx_acc_get_cap(drv_type);

	if (!acc_cap)
		return false;
	/* if acc not support 12V, 12V pmode will judge fail */
	if (!acc_cap->support_12v && (pcfg->vtx == 12000))
		return false;

	bat_temp_get_temperature(BAT_TEMP_MIXED, &tbatt);
	if ((pcfg->tbatt >= 0) && (tbatt >= pcfg->tbatt))
		return false;

	return true;
}

bool wlrx_pmode_judge(unsigned int drv_type, int pid, unsigned int judge_type)
{
	struct wlrx_pmode *pcfg = NULL;

	if (!wlrx_pmode_is_pid_valid(drv_type, pid))
		return false;

	pcfg = wlrx_pmode_get_pcfg_by_pid(drv_type, pid);
	if (!pcfg) {
		hwlog_err("judge: input err\n");
		return false;
	}

	switch (judge_type) {
	case WLDC_PMODE_FINAL_JUDGE:
	case WLRX_PMODE_FINAL_JUDGE:
		if (!wlc_pmode_final_judge(drv_type, pid, pcfg))
			return false;
		if (!wlrx_pmode_final_judge(drv_type, pcfg))
			return false;
		/* fall-through */
	case WLRX_PMODE_NORMAL_JUDGE:
		if (!wlrx_pmode_normal_judge(drv_type, pcfg))
			return false;
		/* fall-through */
	case WLRX_PMODE_QUICK_JUDGE:
		if (!wlrx_pmode_quick_judge(drv_type, pcfg))
			return false;
		break;
	default:
		hwlog_err("judge: type=%u err\n", judge_type);
		return false;
	}

	if ((judge_type == WLRX_PMODE_FINAL_JUDGE) &&
		wlrx_pmode_is_dc_mode_by_name(pcfg->name)) {
		if (!wlrx_pmode_dc_judge_crit(pcfg->name, pid))
			return false;
	}

	return true;
}

static int wlrx_pmode_cfg_str2int(const char *str, int *pcfg, int i)
{
	if (kstrtoint(str, 0, &pcfg[(i - 1) % WLRX_PMODE_CFG_COL]))
		return -EINVAL;

	return 0;
}

static int wlrx_pmode_parse_vmode_cfg(const struct device_node *np,
	struct wlrx_pmode_dts *dts)
{
	int i, len, size;

	len = power_dts_read_u32_count(power_dts_tag(HWLOG_TAG), np,
		"volt_mode", WLRX_VMODE_CFG_ROW, WLRX_VMODE_CFG_COL);
	if (len <= 0)
		return -EINVAL;

	size = sizeof(*dts->vmode_cfg) * (len / WLRX_VMODE_CFG_COL);
	dts->vmode_cfg = kzalloc(size, GFP_KERNEL);
	if (!dts->vmode_cfg)
		return -ENOMEM;

	if (power_dts_read_u32_array(power_dts_tag(HWLOG_TAG), np,
		"volt_mode", (u32 *)dts->vmode_cfg, len))
		return -EINVAL;

	dts->vmode_cfg_level = len / WLRX_VMODE_CFG_COL;
	for (i = 0; i < dts->vmode_cfg_level; i++)
		hwlog_info("vmode[%d], id: %u vtx: %-5d\n",
			i, dts->vmode_cfg[i].id, dts->vmode_cfg[i].vtx);

	return 0;
}

static int wlrx_pmode_parse_pcfg(const struct device_node *np, struct wlrx_pmode_dts *dts)
{
	int i, len, size;
	const char *tmp_str = NULL;
	struct wlrx_pmode *pcfg = NULL;

	len = power_dts_read_count_strings(power_dts_tag(HWLOG_TAG), np,
		"rx_mode_para", WLRX_PMODE_CFG_ROW, WLRX_PMODE_CFG_COL);
	if (len <= 0)
		return -EINVAL;

	size = sizeof(*dts->pcfg) * (len / WLRX_PMODE_CFG_COL);
	dts->pcfg = kzalloc(size, GFP_KERNEL);
	if (!dts->pcfg)
		return -ENOMEM;

	dts->pcfg_level = len / WLRX_PMODE_CFG_COL;
	for (i = 0; i < len; i++) {
		if (power_dts_read_string_index(power_dts_tag(HWLOG_TAG), np,
			"rx_mode_para", i, &tmp_str))
			return -EINVAL;
		if ((i % WLRX_PMODE_CFG_COL) == 0) { /* 0: pmode name */
			dts->pcfg[i / WLRX_PMODE_CFG_COL].name = tmp_str;
			continue;
		}
		if (wlrx_pmode_cfg_str2int(tmp_str,
			(int *)&dts->pcfg[i / WLRX_PMODE_CFG_COL].vtx_min, i))
			return -EINVAL;
	}
	for (i = 0; i < dts->pcfg_level; i++) {
		pcfg = &dts->pcfg[i];
		hwlog_info("pmode[%d] name:%-4s vtx_min:%-5d itx_min:%-4d\t"
			"vtx:%-5d vrx:%-5d irx:%-4d vrect_lth:%-5d tbatt:%-3d\t"
			"cable:%-2d auth:%-2d icon:%u timeout:%-4d expect_mode:%-2d\n",
			i, pcfg->name, pcfg->vtx_min, pcfg->itx_min,
			pcfg->vtx, pcfg->vrx, pcfg->irx, pcfg->vrect_lth,
			pcfg->tbatt, pcfg->cable, pcfg->auth, pcfg->icon,
			pcfg->timeout, pcfg->expect_mode);
	}
	return 0;
}

static int wlrx_pmode_parse_dts(const struct device_node *np, struct wlrx_pmode_dts **dts)
{
	int ret;

	*dts = kzalloc(sizeof(**dts), GFP_KERNEL);
	if (!*dts)
		return -ENOMEM;

	ret = wlrx_pmode_parse_pcfg(np, *dts);
	if (ret)
		return ret;

	ret = wlrx_pmode_parse_vmode_cfg(np, *dts);
	if (ret)
		return ret;

	return 0;
}

static void wlrx_pmode_kfree_dev(struct wlrx_pmode_dev *di)
{
	if (!di)
		return;

	if (di->dts) {
		kfree(di->dts->vmode_cfg);
		kfree(di->dts->pcfg);
		kfree(di->dts);
	}

	kfree(di);
}

int wlrx_pmode_init(unsigned int drv_type, struct device *dev)
{
	int ret;
	struct wlrx_pmode_dev *di = NULL;

	if (!dev || !wltrx_is_drv_type_valid(drv_type))
		return -EINVAL;

	di = kzalloc(sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	ret = wlrx_pmode_parse_dts(dev->of_node, &di->dts);
	if (ret)
		goto exit;

	g_rx_pmode_di[drv_type] = di;
	return 0;

exit:
	wlrx_pmode_kfree_dev(di);
	return ret;
}

void wlrx_pmode_deinit(unsigned int drv_type)
{
	if (!wltrx_is_drv_type_valid(drv_type))
		return;

	wlrx_pmode_kfree_dev(g_rx_pmode_di[drv_type]);
	g_rx_pmode_di[drv_type] = NULL;
}
