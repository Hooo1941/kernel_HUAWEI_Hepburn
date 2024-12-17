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

#include "dpu_comp_dfr_ltps.h"
#include "dpu_comp_mgr.h"
#include "panel_mgr.h"
#include "dpu_conn_mgr.h"
#include "mipi_dsi_dev.h"
#ifdef CONFIG_POWER_DUBAI
#include <huawei_platform/log/hwlog_kernel.h>
#endif

struct dfr_ltps_ctrl {
	bool inited;
	spinlock_t switch_frm_lock;
};

static struct dfr_ltps_ctrl g_dfr_ltps_ctrl;

void dfr_ltps_setup_data(struct dpu_comp_dfr_ctrl *dfr_ctrl)
{
	struct dfr_ltps_ctrl *priv = NULL;

	dpu_check_and_no_retval(!dfr_ctrl, err, "dfr_ctrl is null");

	priv = &g_dfr_ltps_ctrl;
	if (!priv->inited) {
		priv->inited = true;
		spin_lock_init(&priv->switch_frm_lock);
	}

	dfr_ctrl->priv_data = priv;
}

int32_t dfr_ltps_switch_frm_rate(struct dpu_comp_dfr_ctrl *dfr_ctrl, uint32_t frame_rate)
{
	unsigned long flags = 0;
	struct dfr_ltps_ctrl *priv = NULL;

	dpu_check_and_return(!dfr_ctrl, -1, err, "dfr ctrl is null\n");

	priv = (struct dfr_ltps_ctrl *)dfr_ctrl->priv_data;

	spin_lock_irqsave(&priv->switch_frm_lock, flags);
	dfr_ctrl->cur_frm_rate = frame_rate;
	spin_unlock_irqrestore(&priv->switch_frm_lock, flags);

	return 0;
}

int32_t dfr_ltps_update_frm_rate_isr_handler(struct dpu_comp_dfr_ctrl *dfr_ctrl)
{
	unsigned long flags = 0;
	struct dpu_panel_ops *pops = NULL;
	struct dpu_connector *connector = NULL;
	struct dkmd_connector_info *pinfo = NULL;
	struct dfr_ltps_ctrl *priv = NULL;

	dpu_check_and_return(!dfr_ctrl, -1, err, "dfr ctrl is null\n");
	pinfo = dfr_ctrl->dpu_comp->conn_info;
	dpu_check_and_return(!pinfo, -1, err, "pinfo is null\n");

	priv = (struct dfr_ltps_ctrl *)dfr_ctrl->priv_data;
	dpu_check_and_return(!priv, -1, err, "priv is null\n");
	spin_lock_irqsave(&priv->switch_frm_lock, flags);

	if (dfr_ctrl->pre_frm_rate == dfr_ctrl->cur_frm_rate) {
		dpu_pr_debug("frame rate do not change\n");
		spin_unlock_irqrestore(&priv->switch_frm_lock, flags);
		return 0;
	}

	dpu_pr_info("frame rate change from %d to %d\n", dfr_ctrl->pre_frm_rate, dfr_ctrl->cur_frm_rate);
#ifdef CONFIG_POWER_DUBAI
	/* report when lcd fresh rate change */
	HWDUBAI_LOGE("DUBAI_TAG_EPS_LCD_FREQ", "sourcerate=%u targetrate=%u",
		dfr_ctrl->pre_frm_rate, dfr_ctrl->cur_frm_rate);
#endif

	connector = get_primary_connector(pinfo);
	mipi_dsi_dfr_update(connector, (int32_t)dfr_ctrl->cur_frm_rate, dfr_ctrl->mode);

	pops = get_panel_ops(pinfo->base.id);
	if (!pops) {
		dpu_pr_err("panel ops is null");
		spin_unlock_irqrestore(&priv->switch_frm_lock, flags);
		return -1;
	}

	if (pops->update_lcd_fps)
		pops->update_lcd_fps(dfr_ctrl->cur_frm_rate);

	dfr_ctrl->pre_frm_rate = dfr_ctrl->cur_frm_rate;
	spin_unlock_irqrestore(&priv->switch_frm_lock, flags);

	return 0;
}


int32_t dfr_ltps_commit(struct dpu_comp_dfr_ctrl *dfr_ctrl)
{
	dpu_check_and_return(!dfr_ctrl, -1, err, "dfr ctrl is null\n");
	if (dfr_ctrl->dpu_comp->conn_info->enable_ldi)
		dfr_ctrl->dpu_comp->conn_info->enable_ldi(dfr_ctrl->dpu_comp->conn_info);
	return 0;
}

int32_t dfr_ltps_send_dcs_cmds(struct dpu_comp_dfr_ctrl *dfr_ctrl, struct disp_effect_params *effect_params)
{
	struct dkmd_connector_info *pinfo = NULL;
	struct dpu_bl_ctrl *bl_ctrl = NULL;

	dpu_check_and_return(!dfr_ctrl, -1, err, "dfr_ctrl is null");
	dpu_check_and_return(!dfr_ctrl->dpu_comp, -1, err, "dpu_composer is null");
	dpu_check_and_return(effect_params->effect_num > EFFECT_PARAMS_MAX_NUM, -1, err, "effect num exceeds max num");
	if ((effect_params->delay != 0 || effect_params->effect_num == 0) && (g_debug_dpu_send_dcs_cmds ==0)) {
		dpu_pr_debug("Nothing to send !\n");
		return 0;
	}

	bl_ctrl = &dfr_ctrl->dpu_comp->bl_ctrl;
	pinfo = dfr_ctrl->dpu_comp->conn_info;
	dpu_check_and_return(!pinfo, -1, err, "pinfo is null");

	dpu_pr_info("set bl_level level with frame+\n");
	dpu_backlight_update_level(bl_ctrl, effect_params);
	pipeline_next_ops_handle(pinfo->conn_device, pinfo, SET_BACKLIGHT, &bl_ctrl->bl_level);
	dpu_pr_info("set bl_level level with frame-, bl_level is %u\n", bl_ctrl->bl_level);

	return 0;
}
