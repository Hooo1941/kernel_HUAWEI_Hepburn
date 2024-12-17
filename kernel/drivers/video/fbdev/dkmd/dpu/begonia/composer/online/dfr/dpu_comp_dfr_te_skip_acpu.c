/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <linux/ktime.h>
#include <linux/types.h>
#include <linux/wait.h>
#include <linux/spinlock.h>
#include <linux/notifier.h>
#include <linux/kthread.h>
#include "dpu_comp_dfr_te_skip_acpu.h"
#include <dpu/soc_dpu_define.h>
#include "dpu_comp_mgr.h"
#include "dkmd_lcd_interface.h"
#include "dkmd_peri.h"
#include "dpu_conn_mgr.h"
#include "dpu_comp_maintain.h"

struct dfr_te_skip_acpu_ctrl {
	uint32_t te_cnt;
	bool inited;
	bool frame_update;
	bool first_frame;
	bool valid_vsync;
	bool routine_enabled;
	struct semaphore dfr_acpu_sem;
};

static struct dfr_te_skip_acpu_ctrl g_te_skip_acpu_ctrl = { 0 };

static uint32_t get_skip_te_num(struct dpu_comp_dfr_ctrl *dfr_ctrl)
{
	if ((dfr_ctrl->cur_te_rate >= dfr_ctrl->cur_frm_rate) && (dfr_ctrl->cur_frm_rate != 0))
		return dfr_ctrl->cur_te_rate / dfr_ctrl->cur_frm_rate;

	dpu_pr_warn("invalid skip te number, return default value 1");
	return 1;
}

static void dfr_te_skip_acpu_enable_ldi(struct dpu_comp_dfr_ctrl *dfr_ctrl)
{
	if (dfr_ctrl->dpu_comp->conn_info->enable_ldi)
		dfr_ctrl->dpu_comp->conn_info->enable_ldi(dfr_ctrl->dpu_comp->conn_info);
	else
		dpu_pr_err("enable ldi failed");
}

static void report_vsync_by_te_isr(struct dpu_comp_dfr_ctrl *dfr_ctrl)
{
	struct dfr_te_skip_acpu_ctrl *priv = (struct dfr_te_skip_acpu_ctrl *)dfr_ctrl->priv_data;
	struct dkmd_isr *isr_ctrl = &dfr_ctrl->dpu_comp->isr_ctrl;

	dpu_pr_debug("+");
	/* report the vsync according to the frame rate(skip te num),
	 * special case: if frame rate equal to te rate(skip_te_num= 1), vsync need be reportted in each te isr.
	 */
	if ((get_skip_te_num(dfr_ctrl) == 1) || (priv->te_cnt % get_skip_te_num(dfr_ctrl) == 1)) {
		dkmd_isr_notify_listener(isr_ctrl, DSI_INT_VSYNC_COUNT_BY_TE);

		/* if current te is used for a new frame present, then refresh statistic is requried */
		if (priv->valid_vsync) {
			priv->valid_vsync = false;
			dkmd_isr_notify_listener(isr_ctrl, NOTIFY_REFRESH);
			dpu_pr_debug("valid vsync timestamp = %llu us", ktime_to_us(ktime_get()));
		} else {
			dpu_pr_debug("count vsync timestamp = %llu us", ktime_to_us(ktime_get()));
		}
	}
}

static void set_sfu_by_te_isr(struct dpu_comp_dfr_ctrl *dfr_ctrl)
{
	struct dfr_te_skip_acpu_ctrl *priv = (struct dfr_te_skip_acpu_ctrl *)dfr_ctrl->priv_data;

	dpu_pr_debug("+");
	if (priv->first_frame) {
		dpu_pr_info("quick present case, do not set sfu in te isr");
		return;
	}

	/* when a new frame is updated and te cnt meets the requirements, then set sfu */
	if ((priv->frame_update) && (priv->te_cnt % get_skip_te_num(dfr_ctrl) == 0)) {
		dpu_pr_debug("te isr set sfu");
		dfr_te_skip_acpu_enable_ldi(dfr_ctrl);
		priv->valid_vsync = true;
		priv->frame_update = false;
		priv->te_cnt = 0;
	}
}

static void dfr_te_skip_acpu_te_isr_handle_work(struct kthread_work *work)
{
	struct dpu_comp_dfr_ctrl *dfr_ctrl = NULL;
	struct dfr_te_skip_acpu_ctrl *priv = NULL;

	dpu_pr_debug("+");
	dpu_check_and_no_retval(!work, err, "work is NULL!!!");

	dfr_ctrl = container_of(work, struct dpu_comp_dfr_ctrl, dfr_work);
	dpu_check_and_no_retval(!dfr_ctrl, err, "dfr_ctrl is null");

	priv = (struct dfr_te_skip_acpu_ctrl *)dfr_ctrl->priv_data;
	dpu_check_and_no_retval(!priv, err, "priv is null");

	down(&priv->dfr_acpu_sem);
	priv->te_cnt++;
	dpu_pr_debug("te cnt: %u", priv->te_cnt);

	/* report vsync to SF when required */
	report_vsync_by_te_isr(dfr_ctrl);

	/* set SFU when required */
	set_sfu_by_te_isr(dfr_ctrl);
	up(&priv->dfr_acpu_sem);
}

static int32_t dfr_te_skip_acpu_te_isr_notify(struct notifier_block *self, unsigned long action, void *data)
{
	struct dkmd_listener_data *listener_data = (struct dkmd_listener_data *)data;
	struct dpu_comp_dfr_ctrl *dfr_ctrl = (struct dpu_comp_dfr_ctrl *)(listener_data->data);
	struct dfr_te_skip_acpu_ctrl *priv = NULL;

	dpu_check_and_return(!dfr_ctrl, -1, err, "dfr_ctrl is null");

	priv = (struct dfr_te_skip_acpu_ctrl *)dfr_ctrl->priv_data;
	dpu_check_and_return(!priv, -1, err, "priv is null");

	if (!priv->routine_enabled)
		return 0;

	dpu_pr_debug("+");
	kthread_queue_work(&dfr_ctrl->dpu_comp->handle_worker, &dfr_ctrl->dfr_work);

	return 0;
}

static struct notifier_block g_te_isr_notifier = {
	.notifier_call = dfr_te_skip_acpu_te_isr_notify,
};

static uint32_t dfr_te_skip_acpu_get_te_rate(struct dpu_comp_dfr_ctrl *dfr_ctrl)
{
	int idx;
	struct dfr_info *dinfo = dkmd_get_dfr_info(dfr_ctrl->dpu_comp->conn_info);

	dpu_check_and_return(!dinfo, 0, err, "dfr info is null");
	dpu_check_and_return(dinfo->oled_info.ltpo_info.te_freq_num == 0, 0, err, "invalid te freq");

	if (dfr_ctrl->cur_frm_rate == 0)
		return dinfo->oled_info.ltpo_info.te_freqs[0];

	for (idx = 0; idx < TE_FREQ_NUM_MAX; idx++) {
		if (dinfo->oled_info.ltpo_info.te_freqs[idx] % dfr_ctrl->cur_frm_rate == 0)
			return dinfo->oled_info.ltpo_info.te_freqs[idx];
	}

	dpu_pr_err("frm rate is %uhz, no valid te freq to devide", dfr_ctrl->cur_frm_rate);
	return 0;
}

static void dfr_te_skip_acpu_register_te_isr(struct dpu_comp_dfr_ctrl *dfr_ctrl)
{
	uint32_t dsi_te_id;
	struct dkmd_isr *isr_ctrl = &dfr_ctrl->dpu_comp->isr_ctrl;
	struct dkmd_connector_info *pinfo = dfr_ctrl->dpu_comp->conn_info;

	dpu_pr_debug("+");
	kthread_init_work(&dfr_ctrl->dfr_work, dfr_te_skip_acpu_te_isr_handle_work);

	dsi_te_id = pinfo->base.lcd_te_idx == 0 ? DSI_INT_LCD_TE0 : DSI_INT_LCD_TE1;
	dkmd_isr_register_listener(isr_ctrl, &g_te_isr_notifier, dsi_te_id, dfr_ctrl);
}

void dfr_te_skip_acpu_setup_priv_data(struct dpu_comp_dfr_ctrl *dfr_ctrl)
{
	struct dfr_te_skip_acpu_ctrl *priv = NULL;

	dpu_pr_debug("+");
	dpu_check_and_no_retval(!dfr_ctrl, err, "dfr_ctrl is null");

	priv = &g_te_skip_acpu_ctrl;
	priv->te_cnt = 0;
	priv->frame_update = false;
	priv->first_frame = true;
	priv->valid_vsync = false;
	sema_init(&priv->dfr_acpu_sem, 1);

	dfr_ctrl->priv_data = priv;

	if (!priv->inited) {
		priv->inited = true;
		dfr_te_skip_acpu_register_te_isr(dfr_ctrl);
	}

	priv->routine_enabled = true;
}

void dfr_te_skip_acpu_release_priv_data(struct dpu_comp_dfr_ctrl *dfr_ctrl)
{
	struct dfr_te_skip_acpu_ctrl *priv = NULL;

	dpu_pr_debug("+");
	dpu_check_and_no_retval(!dfr_ctrl, err, "dfr_ctrl is null");

	priv = (struct dfr_te_skip_acpu_ctrl *)dfr_ctrl->priv_data;
	dpu_check_and_no_retval(!priv, err, "priv is null");

	priv->routine_enabled = false;
}

void dfr_te_skip_acpu_exit_idle_status(struct dpu_comp_dfr_ctrl *dfr_ctrl)
{
	struct dfr_te_skip_acpu_ctrl *priv = NULL;

	dpu_pr_debug("+");
	dpu_check_and_no_retval(!dfr_ctrl, err, "dfr_ctrl is null");

	priv = (struct dfr_te_skip_acpu_ctrl *)dfr_ctrl->priv_data;
	dpu_check_and_no_retval(!priv, err, "priv is null");

	priv->te_cnt = 0;
	priv->frame_update = false;
	priv->first_frame = true;
	priv->valid_vsync = false;
}

int32_t dfr_te_skip_acpu_switch_frm_rate(struct dpu_comp_dfr_ctrl *dfr_ctrl, uint32_t frame_rate)
{
	dpu_pr_debug("+");
	dpu_check_and_return(!dfr_ctrl, -1, err, "dfr_ctrl is null");

	if (g_debug_dpu_frm_rate != 0)
		frame_rate = g_debug_dpu_frm_rate;

	dfr_ctrl->pre_frm_rate = dfr_ctrl->cur_frm_rate;
	dfr_ctrl->cur_frm_rate = frame_rate;

	dfr_ctrl->pre_te_rate = dfr_ctrl->cur_te_rate;
	dfr_ctrl->cur_te_rate = dfr_te_skip_acpu_get_te_rate(dfr_ctrl);

	if (dfr_ctrl->pre_frm_rate != dfr_ctrl->cur_frm_rate)
		dpu_pr_info("switch frame rate from %uhz to %uhz, switch te rate from %u to %u",
			dfr_ctrl->pre_frm_rate, dfr_ctrl->cur_frm_rate, dfr_ctrl->pre_te_rate, dfr_ctrl->cur_te_rate);

	dpu_pr_debug("-");
	return 0;
}

static int32_t get_safe_frm_rate_idx(struct dpu_comp_dfr_ctrl *dfr_ctrl, uint32_t safe_frm_rate)
{
	struct dfr_info *dinfo = NULL;
	int32_t frm_index;

	dinfo = dkmd_get_dfr_info(dfr_ctrl->dpu_comp->conn_info);
	dpu_check_and_return(!dinfo, -1, err, "dinfo is null");

	dpu_pr_info("cur safe frm rate is %u", safe_frm_rate);
	if (dinfo->oled_info.ltpo_info.safe_frm_rates_num > SAFE_FRM_RATE_MAX_NUM) {
		dpu_pr_warn("safe frm rates num exceed max num");
		dinfo->oled_info.ltpo_info.safe_frm_rates_num = SAFE_FRM_RATE_MAX_NUM;
	}

	frm_index = (int32_t)dinfo->oled_info.ltpo_info.safe_frm_rates_num -1;
	for (; frm_index >= 0; frm_index--) {
		if (safe_frm_rate == dinfo->oled_info.ltpo_info.safe_frm_rates[frm_index].safe_frm_rate)
			break;
	}

	dpu_pr_info("cur safe_frm_rate_idx is %u", frm_index);
	return frm_index;
}

static int32_t update_safe_frm_rate_to_ddic(struct dpu_comp_dfr_ctrl *dfr_ctrl)
{
	int32_t frm_index;
	struct dfr_info *dinfo = NULL;
	struct dsi_cmds *safe_frm_rate_cmd = NULL;
	struct dkmd_connector_info *pinfo = dfr_ctrl->dpu_comp->conn_info;
	struct comp_online_present *present = (struct comp_online_present *)dfr_ctrl->dpu_comp->present_data;

	dinfo = dkmd_get_dfr_info(dfr_ctrl->dpu_comp->conn_info);
	dpu_check_and_return(!dinfo, -1, err, "dinfo is null");

	frm_index = get_safe_frm_rate_idx(dfr_ctrl, dfr_ctrl->cur_safe_frm_rate);
	/* check safe frm_idx */
	if (frm_index == -1) {
		dpu_pr_warn("No need to set safe frm rate, invalid value!");
		return 0;
	}
	dpu_pr_info("safe frm rate update, frm rate is %u, frm_index is %u", dfr_ctrl->cur_safe_frm_rate, frm_index);

	safe_frm_rate_cmd = &dinfo->oled_info.ltpo_info.safe_frm_rates[frm_index].dsi_cmds;
	pipeline_next_ops_handle(pinfo->conn_device, pinfo, DCS_CMD_TX_FOR_PLATFORM, safe_frm_rate_cmd);

	/* for the frame rate statistics in maintain module */
	if (dfr_ctrl->cur_safe_frm_rate > 0) {
		present->comp_maintain.self_refresh_period_us = PERIOD_US_1HZ / dfr_ctrl->cur_safe_frm_rate;
	} else {
		dpu_pr_warn("meet invalide safe frame rate(%u) when set it to maintain!", dfr_ctrl->cur_safe_frm_rate);
	}

	return 0;
}

static void set_sfu_by_commit_normal(struct dpu_comp_dfr_ctrl *dfr_ctrl)
{
	struct dfr_te_skip_acpu_ctrl *priv = (struct dfr_te_skip_acpu_ctrl *)dfr_ctrl->priv_data;

	dpu_pr_debug("+");
	/* when a new frame is updated and te cnt meets the requirements, then set sfu */
	if ((priv->te_cnt != 0) && (priv->te_cnt % get_skip_te_num(dfr_ctrl) == 0)) {
		dfr_te_skip_acpu_enable_ldi(dfr_ctrl);
		priv->valid_vsync = true;
		priv->te_cnt = 0;
		priv->frame_update = false;
		dpu_pr_debug("normal commit set sfu");
	} else {
		priv->frame_update = true;
	}
}

static void set_sfu_by_commit_quick_present(struct dpu_comp_dfr_ctrl *dfr_ctrl)
{
	struct dfr_te_skip_acpu_ctrl *priv = (struct dfr_te_skip_acpu_ctrl *)dfr_ctrl->priv_data;

	dpu_pr_debug("+");
	dfr_te_skip_acpu_enable_ldi(dfr_ctrl);
	priv->valid_vsync = true;
	priv->te_cnt = 0;
	priv->frame_update = false;
	dpu_pr_info("quick present commit set sfu");
}

int32_t dfr_te_skip_acpu_commit(struct dpu_comp_dfr_ctrl *dfr_ctrl)
{
	struct dfr_te_skip_acpu_ctrl *priv = NULL;

	dpu_check_and_return(!dfr_ctrl, -1, err, "dfr_ctrl is null");

	priv = (struct dfr_te_skip_acpu_ctrl *)dfr_ctrl->priv_data;
	dpu_check_and_return(!priv, -1, err, "priv is null");

	dpu_pr_debug("+");
	dpu_pr_debug("te cnt: %u", priv->te_cnt);

	/* when idle, need resend safe frm rate by commit */
	if (priv->first_frame)
		update_safe_frm_rate_to_ddic(dfr_ctrl);

	down(&priv->dfr_acpu_sem);
	/* set sfu for quick present case */
	if (priv->first_frame) {
		set_sfu_by_commit_quick_present(dfr_ctrl);
		priv->first_frame = false;
		up(&priv->dfr_acpu_sem);
		return 0;
	}

	/* set sfu when required */
	set_sfu_by_commit_normal(dfr_ctrl);
	up(&priv->dfr_acpu_sem);

	dpu_pr_debug("-");
	return 0;
}

#define SAFE_FRM_RATE_MAX 0xffff
#define SAFE_FRM_RATE_DEFAULT 60
static bool check_safe_frm_rate(struct dpu_comp_dfr_ctrl *dfr_ctrl, uint32_t safe_frm_rate)
{
	uint32_t parse_frm_rate = safe_frm_rate;
	int32_t frm_index = 0;

	dpu_pr_debug("+");
	if (safe_frm_rate == SAFE_FRM_RATE_MAX) {
		dpu_pr_warn("safe_frm_rate is max, need set default safe frm rate!");
		parse_frm_rate = SAFE_FRM_RATE_DEFAULT;
	}

	if (dfr_ctrl->cur_safe_frm_rate == parse_frm_rate || parse_frm_rate == 0) {
		dpu_pr_debug("No need to send safe frm rate");
		if (g_debug_dpu_safe_frm_rate == 0)
			return false;
		parse_frm_rate = g_debug_dpu_safe_frm_rate;
	}

	frm_index = get_safe_frm_rate_idx(dfr_ctrl, parse_frm_rate);
	if (frm_index == -1) {
		dpu_pr_warn("not support safe frm rate!");
		return false;
	}
	dfr_ctrl->cur_safe_frm_rate = parse_frm_rate;

	dpu_pr_debug("-");
	return true;
}

int32_t dfr_te_skip_acpu_set_safe_frm_rate(struct dpu_comp_dfr_ctrl *dfr_ctrl, uint32_t safe_frm_rate)
{
	struct composer_manager *comp_mgr = NULL;
	struct dfr_te_skip_acpu_ctrl *priv = NULL;

	dpu_check_and_return(!dfr_ctrl, -1, err, "dfr_ctrl is null");
	dpu_check_and_return(!dfr_ctrl->dpu_comp->comp_mgr, -1, err, "comp mgr is null");

	dpu_pr_debug("+");
	comp_mgr = dfr_ctrl->dpu_comp->comp_mgr;
	priv = (struct dfr_te_skip_acpu_ctrl *)dfr_ctrl->priv_data;
	dpu_check_and_return(!priv, -1, err, "priv is null");

	/* check safe frame rate */
	if(!check_safe_frm_rate(dfr_ctrl, safe_frm_rate)) {
		dpu_pr_warn("No need to set safe frm rate, invalid value!");
		return 0;
	}
	dpu_pr_info("safe frm rate update, frm rate is %u", dfr_ctrl->cur_safe_frm_rate);

	down(&comp_mgr->power_sem);
	if (!composer_check_power_status(dfr_ctrl->dpu_comp)) {
		dpu_pr_warn("composer %u, panel power off!", dfr_ctrl->dpu_comp->comp.index);
		up(&comp_mgr->power_sem);
		return 0;
	}
	up(&comp_mgr->power_sem);

	dpu_comp_active_vsync(dfr_ctrl->dpu_comp);
	update_safe_frm_rate_to_ddic(dfr_ctrl);
	dpu_comp_deactive_vsync(dfr_ctrl->dpu_comp);

	dpu_pr_debug("-");
	return 0;
}
