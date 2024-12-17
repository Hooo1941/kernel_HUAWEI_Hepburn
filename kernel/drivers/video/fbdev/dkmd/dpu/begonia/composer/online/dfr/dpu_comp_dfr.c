/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
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
#include "dpu_comp_dfr.h"
#include "dpu_comp_dfr_constant.h"
#include "dpu_comp_dfr_te_skip.h"
#include "dpu_comp_dfr_te_skip_acpu.h"
#include "dpu_comp_dfr_ltps.h"
#include "dpu_comp_dfr_ltps_longh.h"
#include "dpu_comp_mgr.h"
#include "dkmd_dfr_info.h"
#include "dpu_conn_mgr.h"
#ifdef CONFIG_POWER_DUBAI
#include <huawei_platform/log/hwlog_kernel.h>
#endif

#ifndef array_len
#define array_len(array) (sizeof(array) / sizeof((array)[0]))
#endif

#define FPS_60HZ 60
#define FPS_120HZ 120
#define FPS_360HZ 360

void dpu_comp_dfr_ctrl_setup(struct dpu_composer *dpu_comp, struct comp_online_present *present)
{
	struct dpu_comp_dfr_ctrl* dfr_ctrl = &present->dfr_ctrl;
	struct dfr_info *dfr_info = dkmd_get_dfr_info(dpu_comp->conn_info);

	dpu_pr_info("comp %d, +",dpu_comp->comp.index);
	if (!dfr_info) {
		dpu_pr_info("dfr info is null, use permanent mode default");
		dfr_ctrl->mode = DFR_MODE_CONSTANT;
	} else {
		dfr_ctrl->mode = dfr_info->dfr_mode;
	}

	dfr_ctrl->dpu_comp = dpu_comp;
	dfr_ctrl->cur_safe_frm_rate = 1;
	present->frame_rate = FPS_60HZ;

	dpu_pr_info("dfr mode %d\n", dfr_ctrl->mode);
	switch (dfr_ctrl->mode) {
	case DFR_MODE_CONSTANT:
		dfr_ctrl->setup_data = dfr_constant_setup_data;
		dfr_ctrl->switch_frm_rate = dfr_constant_switch_frm_rate;
		dfr_ctrl->commit = dfr_constant_commit;
		dfr_ctrl->send_dcs_cmds_with_frm = dfr_constant_send_dcs_cmds;
		dfr_ctrl->send_dcs_cmds_with_refresh = dfr_constant_send_dcs_cmds_with_refresh;
		break;
	case DFR_MODE_TE_SKIP_BY_MCU:
		present->frame_rate = FPS_120HZ;
		dfr_ctrl->setup_data = dfr_te_skip_setup_priv_data;
		dfr_ctrl->release_data = dfr_te_skip_release_priv_data;
		dfr_ctrl->switch_frm_rate = dfr_te_skip_switch_frm_rate;
		dfr_ctrl->commit = dfr_te_skip_commit;
		dfr_ctrl->send_dcs_cmds_with_frm = dfr_te_skip_send_dcs_cmds;
		dfr_ctrl->send_dcs_cmds_with_refresh = dfr_te_skip_send_dcs_cmds_by_riscv;
		dfr_ctrl->set_safe_frm_rate = dfr_te_skip_set_safe_frm_rate;
		dfr_ctrl->send_refresh = dfr_te_skip_self_refresh;
		dfr_ctrl->enable_dimming = dfr_te_skip_enable_dimming;
		dfr_ctrl->disable_dimming = dfr_te_skip_disable_dimming;
		dfr_ctrl->enable_safe_frm_rate = dfr_te_skip_enable_safe_frm_rate;
		dfr_ctrl->disable_safe_frm_rate = dfr_te_skip_disable_safe_frm_rate;
		break;
	case DFR_MODE_TE_SKIP_BY_ACPU:
		present->frame_rate = FPS_60HZ;
		dfr_ctrl->setup_data = dfr_te_skip_acpu_setup_priv_data;
		dfr_ctrl->release_data = dfr_te_skip_acpu_release_priv_data;
		dfr_ctrl->switch_frm_rate = dfr_te_skip_acpu_switch_frm_rate;
		dfr_ctrl->commit = dfr_te_skip_acpu_commit;
		dfr_ctrl->set_safe_frm_rate = dfr_te_skip_acpu_set_safe_frm_rate;
		dfr_ctrl->exit_idle_status = dfr_te_skip_acpu_exit_idle_status;
		break;
	case DFR_MODE_LONGH_TE_SKIP_BY_MCU:
		present->frame_rate = FPS_120HZ;
		dfr_longh_te_skip_func_register(dfr_ctrl);
		break;
	case DFR_MODE_LONG_V:
	case DFR_MODE_LONG_VH:
		present->frame_rate = dpu_comp->conn_info->base.fps;
		dfr_ctrl->cur_frm_rate = dpu_comp->conn_info->base.fps;
		dfr_ctrl->pre_frm_rate = dpu_comp->conn_info->base.fps;
		dfr_ctrl->setup_data = dfr_ltps_setup_data;
		dfr_ctrl->switch_frm_rate = dfr_ltps_switch_frm_rate;
		dfr_ctrl->commit = dfr_ltps_commit;
		dfr_ctrl->send_dcs_cmds_with_frm = dfr_ltps_send_dcs_cmds;
		dfr_ctrl->update_frm_rate_isr_handler = dfr_ltps_update_frm_rate_isr_handler;

#ifdef CONFIG_POWER_DUBAI
		/* mipi fps rate control init */
		HWDUBAI_LOGE("DUBAI_TAG_EPS_LCD_FREQ", "sourcerate=%d targetrate=%d",
			dfr_ctrl->pre_frm_rate, dfr_ctrl->cur_frm_rate);
#endif

		break;
	case DFR_MODE_LONG_H:
		present->frame_rate = dpu_comp->conn_info->base.fps;
		dfr_ctrl->cur_frm_rate = dpu_comp->conn_info->base.fps;
		dfr_ctrl->pre_frm_rate = dpu_comp->conn_info->base.fps;
		dfr_ctrl->switch_frm_rate = dfr_ltps_longh_switch_frm_rate;
		dfr_ctrl->setup_data = dfr_ltps_longh_setup_priv_data;
		dfr_ctrl->release_data = dfr_ltps_longh_release_priv_data;
		dfr_ctrl->commit = dfr_ltps_longh_commit;
		dfr_ctrl->send_dcs_cmds_with_frm = dfr_ltps_send_dcs_cmds;
		dfr_ctrl->update_frm_rate_isr_handler = dfr_ltps_longh_update_frm_rate_isr_handler;
		break;
	default:
		dpu_pr_warn("unsupported dfr mode %d, force to use permanent mode", dfr_ctrl->mode);
		dfr_ctrl->setup_data = dfr_constant_setup_data;
		dfr_ctrl->switch_frm_rate = dfr_constant_switch_frm_rate;
		dfr_ctrl->commit = dfr_constant_commit;
		dfr_ctrl->send_dcs_cmds_with_frm = dfr_constant_send_dcs_cmds;
		dfr_ctrl->send_dcs_cmds_with_refresh = dfr_constant_send_dcs_cmds_with_refresh;
		break;
	}
	dpu_pr_info("comp %d, -", dpu_comp->comp.index);
}

static struct dpu_comp_dfr_ctrl * dkmd_dfr_get_dfr_ctrl_ptr(struct dkmd_connector_info *pinfo)
{
	struct composer *comp = NULL;
	struct dpu_composer *dpu_comp = NULL;
	struct comp_online_present *present = NULL;

	if (!pinfo || !pinfo->base.comp_obj_info) {
		dpu_pr_err("pinfo or comp_obj_info is nullptr!");
		return NULL;
	}

	comp = container_of(pinfo->base.comp_obj_info, struct composer, base);
	if (!comp) {
		dpu_pr_err("comp is nullptr!");
		return NULL;
	}

	dpu_comp = to_dpu_composer(comp);
	if (!dpu_comp) {
		dpu_pr_err("dpu_comp is nullptr!");
		return NULL;
	}
	present = (struct comp_online_present *)dpu_comp->present_data;
	if (!present) {
		dpu_pr_err("present is nullptr!");
		return NULL;
	}

	return &present->dfr_ctrl;
}

static bool check_connector_status(struct dpu_connector *connector)
{
	struct composer *comp = NULL;

	if (!connector->conn_info || !connector->conn_info->base.comp_obj_info) {
		dpu_pr_err("pinfo or comp_obj_info is nullptr!");
		return false;
	}
	comp = container_of(connector->conn_info->base.comp_obj_info, struct composer, base);
	if (!comp) {
		dpu_pr_err("comp is nullptr!");
		return false;
	}

	if (!comp->power_on) {
		dpu_pr_err("is not power_on!");
		return false;
	}

	if (comp->is_fake_power_off) {
		dpu_pr_warn("connector:%u is fake off\n", connector->connector_id);
		return false;
	}

	return true;
}

static struct dpu_connector *get_real_connector(uint32_t raw_dsi_index, uint32_t panel_type)
{
	uint32_t real_dsi_idx;

	if (raw_dsi_index != CONNECTOR_ID_DSI0)
		real_dsi_idx = raw_dsi_index;
	else
		/* bit12: SECONDARY_PANEL_CMD_TYPE from lcdkit */
		real_dsi_idx = (((panel_type & PANEL_EXTERNAL) != 0) ? CONNECTOR_ID_DSI0_BUILTIN : CONNECTOR_ID_DSI0);

	return get_connector_by_id(real_dsi_idx);
}

int32_t dkmd_exit_current_vsync_idle_period(uint32_t conn_id, uint32_t panel_type)
{
	struct dpu_connector *connector0 = NULL;

	connector0 = get_real_connector(conn_id, panel_type);
	if ((!connector0) || (!connector0->connector_base)) {
		dpu_pr_err("connector_id0=%u is not available!\n", conn_id);
		return -1;
	}

	if (!check_connector_status(connector0))
		return -1;

	composer_active_vsync(connector0->conn_info, true);
	composer_active_vsync(connector0->conn_info, false);
	return 0;
}

int32_t dkmd_dfr_send_refresh(uint32_t conn_id, uint32_t panel_type)
{
	struct dpu_connector *connector0 = NULL;
	struct dpu_comp_dfr_ctrl *dfr_ctrl = NULL;

	connector0 = get_real_connector(conn_id, panel_type);
	if ((!connector0) || (!connector0->connector_base)) {
		dpu_pr_err("connector_id0=%u is not available!\n", conn_id);
		return -1;
	}

	if (!check_connector_status(connector0))
		return -1;

	composer_active_vsync(connector0->conn_info, true);
	dfr_ctrl = dkmd_dfr_get_dfr_ctrl_ptr(connector0->conn_info);
	if (dfr_ctrl && dfr_ctrl->send_refresh)
		dfr_ctrl->send_refresh(dfr_ctrl);
	composer_active_vsync(connector0->conn_info, false);

	return 0;
}

int32_t dkmd_dfr_send_safe_frm_rate(uint32_t conn_id, uint32_t panel_type, uint32_t safe_frm_rate)
{
	struct dpu_connector *connector0 = NULL;
	struct dpu_comp_dfr_ctrl *dfr_ctrl = NULL;

	connector0 = get_real_connector(conn_id, panel_type);
	if ((!connector0) || (!connector0->connector_base)) {
		dpu_pr_err("connector_id0=%u is not available!\n", conn_id);
		return -1;
	}

	if (!check_connector_status(connector0))
		return -1;

	composer_active_vsync(connector0->conn_info, true);
	dfr_ctrl = dkmd_dfr_get_dfr_ctrl_ptr(connector0->conn_info);
	if (dfr_ctrl && dfr_ctrl->send_safe_frm_rate)
		dfr_ctrl->send_safe_frm_rate(dfr_ctrl, safe_frm_rate);
	composer_active_vsync(connector0->conn_info, false);

	return 0;
}

int32_t dkmd_dfr_enable_dimming(uint32_t conn_id, uint32_t panel_type)
{
	struct dpu_connector *connector0 = NULL;
	struct dpu_comp_dfr_ctrl *dfr_ctrl = NULL;

	connector0 = get_real_connector(conn_id, panel_type);
	if ((!connector0) || (!connector0->connector_base)) {
		dpu_pr_err("connector_id0=%u is not available!\n", conn_id);
		return -1;
	}

	if (!check_connector_status(connector0))
		return -1;

	composer_active_vsync(connector0->conn_info, true);
	dfr_ctrl = dkmd_dfr_get_dfr_ctrl_ptr(connector0->conn_info);
	if (dfr_ctrl && dfr_ctrl->enable_dimming)
		dfr_ctrl->enable_dimming(dfr_ctrl);
	composer_active_vsync(connector0->conn_info, false);

	return 0;
}

int32_t dkmd_dfr_disable_dimming(uint32_t conn_id, uint32_t panel_type)
{
	struct dpu_connector *connector0 = NULL;
	struct dpu_comp_dfr_ctrl *dfr_ctrl = NULL;

	connector0 = get_real_connector(conn_id, panel_type);
	if ((!connector0) || (!connector0->connector_base)) {
		dpu_pr_err("connector_id0=%u is not available!\n", conn_id);
		return -1;
	}

	if (!check_connector_status(connector0))
		return -1;

	composer_active_vsync(connector0->conn_info, true);
	dfr_ctrl = dkmd_dfr_get_dfr_ctrl_ptr(connector0->conn_info);
	if (dfr_ctrl && dfr_ctrl->disable_dimming)
		dfr_ctrl->disable_dimming(dfr_ctrl);
	composer_active_vsync(connector0->conn_info, false);

	return 0;
}

int32_t dkmd_dfr_enable_safe_frm_rate(uint32_t conn_id, uint32_t panel_type)
{
	struct dpu_connector *connector0 = NULL;
	struct dpu_comp_dfr_ctrl *dfr_ctrl = NULL;

	connector0 = get_real_connector(conn_id, panel_type);
	if ((!connector0) || (!connector0->connector_base)) {
		dpu_pr_err("connector_id0=%u is not available!\n", conn_id);
		return -1;
	}
	if (!check_connector_status(connector0))
		return -1;

	composer_active_vsync(connector0->conn_info, true);
	dfr_ctrl = dkmd_dfr_get_dfr_ctrl_ptr(connector0->conn_info);
	if (dfr_ctrl && dfr_ctrl->enable_safe_frm_rate)
		dfr_ctrl->enable_safe_frm_rate(dfr_ctrl);
	composer_active_vsync(connector0->conn_info, false);
	return 0;
}

int32_t dkmd_dfr_disable_safe_frm_rate(uint32_t conn_id, uint32_t panel_type, int32_t disable_type)
{
	struct dpu_connector *connector0 = NULL;
	struct dpu_comp_dfr_ctrl *dfr_ctrl = NULL;

	connector0 = get_real_connector(conn_id, panel_type);
	if ((!connector0) || (!connector0->connector_base)) {
		dpu_pr_err("connector_id0=%u is not available!\n", conn_id);
		return -1;
	}
	if (!check_connector_status(connector0))
		return -1;

	composer_active_vsync(connector0->conn_info, true);
	dfr_ctrl = dkmd_dfr_get_dfr_ctrl_ptr(connector0->conn_info);
	if (dfr_ctrl && dfr_ctrl->disable_safe_frm_rate)
		dfr_ctrl->disable_safe_frm_rate(dfr_ctrl, disable_type);
	composer_active_vsync(connector0->conn_info, false);
	return 0;
}

void dpu_comp_dfr_ctrl_process(struct dpu_composer *dpu_comp, struct comp_online_present *present,
	struct disp_frame *frame)
{
	if (present->dfr_ctrl.switch_frm_rate) {
		present->dfr_ctrl.switch_frm_rate(&present->dfr_ctrl, present->frame_rate);
		present->comp_maintain.cur_te_rate = present->dfr_ctrl.cur_te_rate;
	}

	if (present->dfr_ctrl.send_dcs_cmds_with_frm) {
		if (present->dfr_ctrl.send_dcs_cmds_with_frm(&present->dfr_ctrl, &frame->effect_params) != 0)
			dpu_pr_warn("send dcs cmds with frame invalid");
	}

	if (present->dfr_ctrl.commit)
		present->dfr_ctrl.commit(&present->dfr_ctrl);
}
