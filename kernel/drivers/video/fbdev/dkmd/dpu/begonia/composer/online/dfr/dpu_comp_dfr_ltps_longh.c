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

#include <linux/kthread.h>
#include <linux/spinlock.h>
#include "dpu_comp_dfr_ltps_longh.h"
#include "dpu_comp_mgr.h"
#include "panel_mgr.h"
#include "dpu_conn_mgr.h"
#include "mipi_dsi_dev.h"
#ifdef CONFIG_POWER_DUBAI
#include <huawei_platform/log/hwlog_kernel.h>
#endif

struct dfr_ltps_longh_ctrl {
	bool inited;
	bool te_isr_routine_enabled;
	bool vactive_isr_routine_enabled;
	bool commit_skip_flag;
	spinlock_t commit_flag_spin_lock;
	uint32_t pre_frm_rate;
	uint32_t pfm_mode_status;
	uint32_t vactive_status;
	spinlock_t status_spin_lock;
	uint32_t te_need_skip_num;
	spinlock_t num_spin_lock;
	spinlock_t cur_frm_rate_spin_lock;
	struct dpu_comp_dfr_ctrl *dfr_ctrl;
	struct kthread_work te_mode_swicth_work;
};

enum PFM_MODE_STATUS {
	PRE_FPS_60,
	/* Switching mode before switching frames 60hz to high fps */
	PRE_SWICTH_PFM,
	/* Switching frames before switching mode high fps to 60hz */
	PRE_SWICTH_FPS,
};

enum VACTIVE_STATUS{
	VACTIVE_END,
	VACTIVE_START,
};

enum SKIP_FRM_NUM{
	SKIP_ONE_FRM = 1,
	SKIP_TWO_FRM,
};

static struct dfr_ltps_longh_ctrl g_ltps_longh_ctrl = { 0 };

/* panel pfm（Pulse frequency modulation）mode switch Only in 60hz */
static bool dfr_ltps_updt_pfm_mode(struct dpu_comp_dfr_ctrl *dfr_ctrl,
	struct dpu_panel_ops *pops, struct dfr_ltps_longh_ctrl *priv)
{
	/* pre frame high fps, cur frame 60hz need update pfm mode */
	if (dfr_ctrl->pre_frm_rate == dfr_ctrl->cur_frm_rate &&
		priv->pfm_mode_status == PRE_SWICTH_FPS) {
		dpu_pr_debug("pre frame high fps -> 60hz, cur frame is_need_set_pfm");
		pops->update_fps_pfm_mode(dfr_ctrl->cur_frm_rate);
		priv->pfm_mode_status = PRE_FPS_60;
		return true;
	}
	/* next frame high fps, cur frame 60hz need update pfm mode */
	if ((dfr_ctrl->pre_frm_rate != dfr_ctrl->cur_frm_rate) &&
		(dfr_ctrl->pre_frm_rate == 60) && priv->pfm_mode_status == PRE_FPS_60) {
		dpu_pr_debug("frame rate change 60hz -> %u, cur frame is_need_set_pfm",
			dfr_ctrl->cur_frm_rate);
		pops->update_fps_pfm_mode(dfr_ctrl->cur_frm_rate);
		priv->pfm_mode_status = PRE_SWICTH_PFM;
		return true;
	}
	/* cur frame high fps, set flag for next frame 60hz update pfm mode */
	if ((dfr_ctrl->pre_frm_rate != dfr_ctrl->cur_frm_rate) &&
		(dfr_ctrl->cur_frm_rate == 60)) {
		priv->pfm_mode_status = PRE_SWICTH_FPS;
		dpu_pr_debug("frame rate change %u -> 60hz, next frame is_need_set_pfm",
			dfr_ctrl->pre_frm_rate);
	}

	return false;
}

static inline void set_priv_vactive_status(struct dfr_ltps_longh_ctrl *priv,
	uint32_t status)
{
	unsigned long flags = 0;

	spin_lock_irqsave(&priv->status_spin_lock, flags);
	priv->vactive_status = status;
	spin_unlock_irqrestore(&priv->status_spin_lock, flags);
}

static inline void set_priv_commit_skip_flag(struct dfr_ltps_longh_ctrl *priv,
	bool value)
{
	unsigned long flags = 0;

	spin_lock_irqsave(&priv->commit_flag_spin_lock, flags);
	priv->commit_skip_flag = value;
	spin_unlock_irqrestore(&priv->commit_flag_spin_lock, flags);
}

static inline void set_priv_te_need_skip_num(struct dfr_ltps_longh_ctrl *priv,
	int num)
{
	unsigned long flags = 0;

	spin_lock_irqsave(&priv->num_spin_lock, flags);
	priv->te_need_skip_num = (uint32_t)num;
	spin_unlock_irqrestore(&priv->num_spin_lock, flags);
}

static inline void reduce_priv_te_need_skip_num(struct dfr_ltps_longh_ctrl *priv)
{
	unsigned long flags = 0;

	spin_lock_irqsave(&priv->num_spin_lock, flags);
	priv->te_need_skip_num--;
	spin_unlock_irqrestore(&priv->num_spin_lock, flags);
}

static inline void set_dfr_cur_frm_rate(struct dpu_comp_dfr_ctrl *dfr_ctrl,
	struct dfr_ltps_longh_ctrl *priv, uint32_t tar_frm_rate)
{
	unsigned long flags = 0;

	spin_lock_irqsave(&priv->cur_frm_rate_spin_lock, flags);
	dfr_ctrl->cur_frm_rate = tar_frm_rate;
	spin_unlock_irqrestore(&priv->cur_frm_rate_spin_lock, flags);
}

int32_t dfr_ltps_longh_switch_frm_rate(struct dpu_comp_dfr_ctrl *dfr_ctrl,
	uint32_t frame_rate)
{
	struct dfr_ltps_longh_ctrl *priv = NULL;

	dpu_check_and_return(!dfr_ctrl, -1, err, "dfr_ctrl is null");
	priv = (struct dfr_ltps_longh_ctrl *)dfr_ctrl->priv_data;
	dpu_check_and_return(!priv, -1, err, "priv is null");

	priv->pre_frm_rate = dfr_ctrl->cur_frm_rate;
	set_dfr_cur_frm_rate(dfr_ctrl, priv, frame_rate);
	return 0;
}

static int32_t dfr_ltps_set_skip_frm_num(struct dpu_comp_dfr_ctrl *dfr_ctrl,
	struct dpu_panel_info *dpinfo, struct dfr_ltps_longh_ctrl *priv)
{
	/* longH high to low fps change need skip one frm */
	if (dfr_ctrl->pre_frm_rate > dfr_ctrl->cur_frm_rate) {
		dpu_pr_info("vactive isr %u -> %u, te mode switch, te need skip",
			dfr_ctrl->pre_frm_rate, dfr_ctrl->cur_frm_rate);
		set_priv_te_need_skip_num(priv, SKIP_ONE_FRM);

		/* panel te mode change need skip two frm */
		if (dpinfo->panel_spcl_cfg.lcd_updt_fps_te_support)
			set_priv_te_need_skip_num(priv, SKIP_TWO_FRM);

		dpu_pr_info("priv->te_need_skip_num = %u", priv->te_need_skip_num);
	}
	return 0;
}

int32_t dfr_ltps_longh_update_frm_rate_isr_handler(struct dpu_comp_dfr_ctrl *dfr_ctrl)
{
	struct dpu_panel_ops *pops = NULL;
	struct dpu_connector *connector = NULL;
	struct dkmd_connector_info *pinfo = NULL;
	struct dpu_panel_info *dpinfo = NULL;
	struct dfr_ltps_longh_ctrl *priv = NULL;

	dpu_check_and_return(!dfr_ctrl, -1, err, "dfr ctrl is null");
	dpu_check_and_return(!dfr_ctrl->dpu_comp, -1, err, "dpu_comp is null");
	pinfo = dfr_ctrl->dpu_comp->conn_info;
	dpu_check_and_return(!pinfo, -1, err, "pinfo is null");

	pops = get_panel_ops(pinfo->base.id);
	dpu_check_and_return(!pops, -1, err, "panel ops is null");

	dpinfo = pops->get_panel_info();
	dpu_check_and_return(!dpinfo, -1, err, "dpinfo is null");

	dpu_check_and_return(!dfr_ctrl->priv_data, -1, err,
		"dfr_ctrl->priv_data is null");
	priv = (struct dfr_ltps_longh_ctrl *)dfr_ctrl->priv_data;
	/* vactive end */
	set_priv_vactive_status(priv, VACTIVE_END);

	if (dpinfo->panel_spcl_cfg.lcd_updt_fps_pfm_support &&
		pops->update_fps_pfm_mode) {
		if (dfr_ltps_updt_pfm_mode(dfr_ctrl, pops, priv))
			return 0;
	}

	if (dfr_ctrl->pre_frm_rate == dfr_ctrl->cur_frm_rate) {
		dpu_pr_debug("frame rate do not change");
		return 0;
	}

	dfr_ltps_set_skip_frm_num(dfr_ctrl, dpinfo, priv);

	dpu_pr_info("frame rate change from %u to %u", dfr_ctrl->pre_frm_rate,
		dfr_ctrl->cur_frm_rate);
#ifdef CONFIG_POWER_DUBAI
	/* report when lcd fresh rate change */
	HWDUBAI_LOGE("DUBAI_TAG_EPS_LCD_FREQ", "sourcerate=%u targetrate=%u",
		dfr_ctrl->pre_frm_rate, dfr_ctrl->cur_frm_rate);
#endif

	connector = get_primary_connector(pinfo);
	dpu_check_and_return(!connector, -1, err, "connector is null");

	mipi_dsi_dfr_update(connector, dfr_ctrl->cur_frm_rate, dfr_ctrl->mode);

	if (pops->update_lcd_fps)
		pops->update_lcd_fps(dfr_ctrl->cur_frm_rate);

	if (dpinfo->panel_spcl_cfg.lcd_updt_fps_te_support &&
		pops->update_fps_te_mode)
		kthread_queue_work(&dfr_ctrl->dpu_comp->handle_worker,
			&priv->te_mode_swicth_work);

	dfr_ctrl->pre_frm_rate = dfr_ctrl->cur_frm_rate;
	return 0;
}

static bool is_support_updt_fps_te_mode(struct dpu_comp_dfr_ctrl *dfr_ctrl)
{
	struct dpu_panel_ops *pops = NULL;
	struct dkmd_connector_info *pinfo = NULL;
	struct dpu_panel_info *dpinfo = NULL;

	dpu_check_and_return(!dfr_ctrl->dpu_comp, false, err, "dpu_comp is null");
	pinfo = dfr_ctrl->dpu_comp->conn_info;
	dpu_check_and_return(!pinfo, false, err, "pinfo is null");

	pops = get_panel_ops(pinfo->base.id);
	dpu_check_and_return(!pops, false, err, "panel ops is null");

	dpinfo = pops->get_panel_info();
	dpu_check_and_return(!dpinfo, false, err, "dpinfo is null");

	if (dpinfo->panel_spcl_cfg.lcd_updt_fps_te_support &&
		pops->update_fps_te_mode)
		return true;

	return false;
}

static bool is_need_skip_commit(struct dpu_comp_dfr_ctrl *dfr_ctrl,
	struct dfr_ltps_longh_ctrl *priv)
{
	/*
	*  case 1:fps change first
	*  need skip commit wait te skip num complete
	*/
	if (priv->commit_skip_flag || priv->te_need_skip_num) {
		dpu_pr_info("skip commit wait te skip complete");
		set_priv_commit_skip_flag(priv, true);
		return true;
	}
	/*
	*  case 2:commit first
	*  high fps switch to low fps rate during display vactive
	*  need skip this commit
	*/
	if ((priv->pre_frm_rate > dfr_ctrl->cur_frm_rate ||
		dfr_ctrl->pre_frm_rate > dfr_ctrl->cur_frm_rate) &&
		priv->vactive_status) {
		dpu_pr_info("skip commit,fps high to low in vactive");
		set_priv_commit_skip_flag(priv, true);
		return true;
	}

	return false;
}

int32_t dfr_ltps_longh_commit(struct dpu_comp_dfr_ctrl *dfr_ctrl)
{
	struct dfr_ltps_longh_ctrl *priv = NULL;

	dpu_check_and_return(!dfr_ctrl, -1, err, "dfr ctrl is null");
	priv = (struct dfr_ltps_longh_ctrl *)dfr_ctrl->priv_data;
	dpu_check_and_return(!priv, -1, err, "priv is null");
	dpu_check_and_return(!dfr_ctrl->dpu_comp, -1, err, "dpu_comp is null");
	dpu_check_and_return(!dfr_ctrl->dpu_comp->conn_info, -1, err, "conn_info is null");

	if (is_need_skip_commit(dfr_ctrl, priv))
		return 0;

	if (dfr_ctrl->dpu_comp->conn_info->enable_ldi)
		dfr_ctrl->dpu_comp->conn_info->enable_ldi(dfr_ctrl->dpu_comp->conn_info);

	return 0;
}

static void te_mode_switch_handle_work(struct kthread_work *work)
{
	struct dpu_panel_ops *pops = NULL;
	struct dpu_comp_dfr_ctrl *dfr_ctrl = NULL;
	struct dkmd_connector_info *pinfo = NULL;
	struct dfr_ltps_longh_ctrl *priv = NULL;

	dpu_check_and_no_retval(!work, err, "work is NULL");

	priv = container_of(work, struct dfr_ltps_longh_ctrl, te_mode_swicth_work);
	dpu_check_and_no_retval(!priv, err, "priv is null");

	dfr_ctrl = priv->dfr_ctrl;
	dpu_check_and_no_retval(!dfr_ctrl, err, "dfr_ctrl is null pointer");
	dpu_check_and_no_retval(!dfr_ctrl->dpu_comp, err, "dpu_comp is null pointer");

	pinfo = dfr_ctrl->dpu_comp->conn_info;
	dpu_check_and_no_retval(!pinfo, err, "pinfo is null");

	pops = get_panel_ops(pinfo->base.id);
	dpu_check_and_no_retval(!pops, err, "panel ops is null");

	dpu_pr_info("te_handle_work done");
	if (pops->update_fps_te_mode)
		pops->update_fps_te_mode(dfr_ctrl->cur_frm_rate);
}

static void dfr_te_skip_longh_enable_ldi(struct dpu_comp_dfr_ctrl *dfr_ctrl,
	struct dfr_ltps_longh_ctrl *priv)
{
	dpu_check_and_no_retval(!dfr_ctrl->dpu_comp, err, "dpu_comp is null");
	dpu_check_and_no_retval(!dfr_ctrl->dpu_comp->conn_info, err, "conn_info is null");

	if (dfr_ctrl->dpu_comp->conn_info->enable_ldi)
		dfr_ctrl->dpu_comp->conn_info->enable_ldi(dfr_ctrl->dpu_comp->conn_info);

	set_priv_commit_skip_flag(priv, false);
}

static int32_t dfr_te_skip_longh_te_isr_notify(struct notifier_block *self,
	unsigned long action, void *data)
{
	struct dkmd_listener_data *listener_data = NULL;
	struct dpu_comp_dfr_ctrl *dfr_ctrl = NULL;
	struct dfr_ltps_longh_ctrl *priv = NULL;

	dpu_check_and_return(!data, -1, err, "data is null pointer");
	listener_data = (struct dkmd_listener_data *)data;
	dfr_ctrl = (struct dpu_comp_dfr_ctrl *)(listener_data->data);
	dpu_check_and_return(!dfr_ctrl, -1, err, "dfr_ctrl is null");
	priv = (struct dfr_ltps_longh_ctrl *)dfr_ctrl->priv_data;
	dpu_check_and_return(!priv, -1, err, "priv is null");

	if (!priv->te_isr_routine_enabled)
		return 0;

	if (!priv->te_need_skip_num) {
		if (priv->commit_skip_flag) {
			dpu_pr_info("frame rate switch commit skip success, set sfu in te isr");
			dfr_te_skip_longh_enable_ldi(dfr_ctrl, priv);
		}
		dpu_pr_debug("te is not skip, do not set sfu in te isr");
		return 0;
	}

	/* te is skipped, then set sfu */
	reduce_priv_te_need_skip_num(priv);
	dpu_pr_info("te need skip, priv->te_need_skip_num = %u",
		priv->te_need_skip_num);
	if ((priv->te_need_skip_num == 0) && priv->commit_skip_flag)
		dfr_te_skip_longh_enable_ldi(dfr_ctrl, priv);

	return 0;
}

static struct notifier_block g_te_isr_notifier = {
	.notifier_call = dfr_te_skip_longh_te_isr_notify,
};

static void dfr_te_skip_register_te_isr(struct dpu_comp_dfr_ctrl *dfr_ctrl)
{
	uint32_t dsi_te_id;
	struct dkmd_isr *isr_ctrl = NULL;
	struct dkmd_connector_info *pinfo = NULL;

	dpu_check_and_no_retval(!dfr_ctrl->dpu_comp, err, "dpu_comp is null");
	pinfo = dfr_ctrl->dpu_comp->conn_info;
	dpu_check_and_no_retval(!pinfo, err, "pinfo is null");

	isr_ctrl = &dfr_ctrl->dpu_comp->isr_ctrl;
	dsi_te_id = pinfo->base.lcd_te_idx == 0 ? DSI_INT_LCD_TE0 : DSI_INT_LCD_TE1;
	dkmd_isr_register_listener(isr_ctrl, &g_te_isr_notifier, dsi_te_id, dfr_ctrl);
}

static void dfr_te_skip_unregister_te_isr(struct dpu_comp_dfr_ctrl *dfr_ctrl)
{
	uint32_t dsi_te_id;
	struct dkmd_isr *isr_ctrl = NULL;
	struct dkmd_connector_info *pinfo = NULL;

	dpu_check_and_no_retval(!dfr_ctrl->dpu_comp, err, "dpu_comp is null");
	pinfo = dfr_ctrl->dpu_comp->conn_info;
	dpu_check_and_no_retval(!pinfo, err, "pinfo is null");

	isr_ctrl = &dfr_ctrl->dpu_comp->isr_ctrl;
	dsi_te_id = pinfo->base.lcd_te_idx == 0 ? DSI_INT_LCD_TE0 : DSI_INT_LCD_TE1;
	dkmd_isr_unregister_listener(isr_ctrl, &g_te_isr_notifier, dsi_te_id);
}

static int32_t dfr_longh_vactive_start_isr_notify(struct notifier_block *self,
	unsigned long action, void *data)
{
	struct dkmd_listener_data *listener_data = NULL;
	struct dpu_comp_dfr_ctrl *dfr_ctrl = NULL;
	struct dfr_ltps_longh_ctrl *priv = NULL;

	dpu_check_and_return(!data, -1, err, "data is null pointer");
	listener_data = (struct dkmd_listener_data *)data;
	dfr_ctrl = (struct dpu_comp_dfr_ctrl *)(listener_data->data);
	dpu_check_and_return(!dfr_ctrl, -1, err, "dfr_ctrl is null");
	priv = (struct dfr_ltps_longh_ctrl *)dfr_ctrl->priv_data;
	dpu_check_and_return(!priv, -1, err, "priv is null");

	if (!priv->vactive_isr_routine_enabled)
		return 0;

	/* vactive start */
	set_priv_vactive_status(priv, VACTIVE_START);
	return 0;
}

static struct notifier_block g_vactive_start_isr_notifier = {
	.notifier_call = dfr_longh_vactive_start_isr_notify,
};

static void dfr_longh_register_vactive_start_isr(struct dpu_comp_dfr_ctrl *dfr_ctrl)
{
	struct dkmd_isr *isr_ctrl = NULL;

	dpu_check_and_no_retval(!dfr_ctrl->dpu_comp, err, "dpu_comp is null");

	isr_ctrl = &dfr_ctrl->dpu_comp->isr_ctrl;
	dkmd_isr_register_listener(isr_ctrl, &g_vactive_start_isr_notifier,
		DSI_INT_VACT0_START, dfr_ctrl);
}

static void dfr_longh_unregister_vactive_start_isr(struct dpu_comp_dfr_ctrl *dfr_ctrl)
{
	struct dkmd_isr *isr_ctrl = NULL;

	dpu_check_and_no_retval(!dfr_ctrl->dpu_comp, err, "dpu_comp is null");

	isr_ctrl = &dfr_ctrl->dpu_comp->isr_ctrl;
	dkmd_isr_unregister_listener(isr_ctrl, &g_vactive_start_isr_notifier,
		DSI_INT_VACT0_START);
}

void dfr_ltps_longh_setup_priv_data(struct dpu_comp_dfr_ctrl *dfr_ctrl)
{
	struct dfr_ltps_longh_ctrl *priv = NULL;

	dpu_check_and_no_retval(!dfr_ctrl, err, "dfr_ctrl is null");

	dfr_ctrl->priv_data = (void*)&g_ltps_longh_ctrl;
	priv = (struct dfr_ltps_longh_ctrl *)dfr_ctrl->priv_data;
	priv->pre_frm_rate = dfr_ctrl->pre_frm_rate;
	priv->vactive_status = 0;
	priv->pfm_mode_status = PRE_FPS_60;
	spin_lock_init(&(priv->status_spin_lock));
	priv->te_need_skip_num = 0;
	spin_lock_init(&(priv->num_spin_lock));
	spin_lock_init(&(priv->cur_frm_rate_spin_lock));
	priv->commit_skip_flag = false;
	spin_lock_init(&(priv->commit_flag_spin_lock));
	priv->dfr_ctrl = dfr_ctrl;

	if (!priv->inited) {
		priv->inited = true;
		dfr_te_skip_register_te_isr(dfr_ctrl);
		dfr_longh_register_vactive_start_isr(dfr_ctrl);
		if (is_support_updt_fps_te_mode(dfr_ctrl))
			kthread_init_work(&priv->te_mode_swicth_work,
				te_mode_switch_handle_work);
	}

	priv->te_isr_routine_enabled = true;
	priv->vactive_isr_routine_enabled = true;
}

void dfr_ltps_longh_release_priv_data(struct dpu_comp_dfr_ctrl *dfr_ctrl)
{
	struct dfr_ltps_longh_ctrl *priv = NULL;

	dpu_check_and_no_retval(!dfr_ctrl, err, "dfr_ctrl is null");
	priv = (struct dfr_ltps_longh_ctrl *)dfr_ctrl->priv_data;
	dpu_check_and_no_retval(!priv, err, "priv is null");

	if (dfr_ctrl->pre_frm_rate != dfr_ctrl->cur_frm_rate) {
		dpu_pr_info("pre_frm_rate %u is not equal cur_frm_rate %u",
			dfr_ctrl->pre_frm_rate, dfr_ctrl->cur_frm_rate);
		set_dfr_cur_frm_rate(dfr_ctrl, priv, dfr_ctrl->pre_frm_rate);
	}

	priv->te_isr_routine_enabled = false;
	priv->vactive_isr_routine_enabled = false;
}
