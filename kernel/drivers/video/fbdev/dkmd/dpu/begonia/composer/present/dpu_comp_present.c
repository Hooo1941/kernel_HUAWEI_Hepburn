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

#include "dkmd_log.h"

#include <linux/delay.h>
#include <dpu/soc_dpu_define.h>
#include "dkmd_release_fence.h"
#include "dkmd_acquire_fence.h"
#include "dpu_conn_mgr.h"
#include "dpu_comp_mgr.h"
#include "scene/dpu_comp_scene.h"
#include "dpu_comp_vactive.h"
#include "dpu_cmdlist.h"
#include "cmdlist_interface.h"
#include "dpu_comp_offline.h"
#include "dpu_comp_online.h"
#include "dpu_comp_sysfs.h"
#include "dpu_config_utils.h"
#include "dvfs.h"
#include "dpu_comp_esd.h"
#include "dpu_comp_ppc.h"
#include "dpu_ppc_status_control.h"
#include "dpu_comp_low_temp_handler.h"

void composer_present_timeline_resync(struct dpu_composer *dpu_comp)
{
	struct comp_online_present *present = NULL;
	struct dkmd_timeline *timeline = NULL;
	struct dkmd_isr *isr_ctrl = NULL;

	if (unlikely(!dpu_comp)) {
		dpu_pr_err("dpu_comp is null");
		return;
	}

	if (unlikely(is_offline_panel(&dpu_comp->comp.base)))
		return;

	present = (struct comp_online_present *)dpu_comp->present_data;
	if (unlikely(!present)) {
		dpu_pr_err("present is null");
		return;
	}

	timeline = &present->timeline;
	isr_ctrl = (struct dkmd_isr *)timeline->isr;
	if (unlikely(!isr_ctrl)) {
		dpu_pr_err("isr_ctrl is null");
		return;
	}

	dkmd_timeline_resync_pt(timeline, 2);

	dkmd_timeline_set_reset_flag(timeline, true);
	dkmd_isr_notify_listener(isr_ctrl, timeline->listening_isr_bit);
	dkmd_timeline_set_reset_flag(timeline, false);

	present->vactive_start_flag = 1;
	present->frame_start_flag = 0;
}

int32_t composer_manager_present(struct composer *comp, void *in_frame)
{
	int ret = 0;
	struct disp_frame *frame = NULL;
	struct dpu_composer *dpu_comp = NULL;

	if (!in_frame) {
		dpu_pr_err("in_frame is nullptr");
		return -1;
	}

	frame = (struct disp_frame *)in_frame;
	dpu_comp = to_dpu_composer(comp);
	if (!dpu_comp) {
		dpu_pr_err("dpu_comp is nullptr");
		return -1;
	}

	dpu_tui_update_ready_status(dpu_comp, frame->tui_ready_status);

	ret = dpu_comp->overlay(dpu_comp, frame);
	if (ret != 0)
		dpu_pr_err("scene_id=%d free cmdlist_id=%d buffer", frame->scene_id, frame->cmdlist_id);

	return ret;
}

void composer_present_dfr_setup(struct dpu_composer *dpu_comp)
{
	struct dkmd_connector_info *pinfo = NULL;
	struct comp_online_present *present = NULL;
	if (!dpu_comp) {
		dpu_pr_err("dpu_comp is nullptr");
		return;
	}

	pinfo = dpu_comp->conn_info;

	if (is_offline_panel(&pinfo->base))
		return;

	present = (struct comp_online_present *)dpu_comp->present_data;

	if (present->dfr_ctrl.setup_data)
		present->dfr_ctrl.setup_data(&present->dfr_ctrl);
}

static void composer_present_ppc_setup(struct dpu_composer *dpu_comp)
{
	struct dkmd_connector_info *pinfo = NULL;
	struct comp_online_present *present = NULL;
	if (!dpu_comp) {
		dpu_pr_err("dpu_comp is nullptr");
		return;
	}

	pinfo = dpu_comp->conn_info;

	if (is_offline_panel(&pinfo->base))
		return;

	present = (struct comp_online_present *)dpu_comp->present_data;

	if (is_ppc_support(&pinfo->base) && present->ppc_ctrl.setup_data)
		present->ppc_ctrl.setup_data(&present->ppc_ctrl);
}

void composer_present_dfr_release(struct dpu_composer *dpu_comp)
{
	struct dkmd_connector_info *pinfo = dpu_comp->conn_info;
	struct comp_online_present *present = NULL;

	if (is_offline_panel(&pinfo->base))
		return;

	present = (struct comp_online_present *)dpu_comp->present_data;

	if (present->dfr_ctrl.release_data)
		present->dfr_ctrl.release_data(&present->dfr_ctrl);
}

void composer_present_ppc_release(struct dpu_composer *dpu_comp)
{
	struct dkmd_connector_info *pinfo = dpu_comp->conn_info;
	struct comp_online_present *present = NULL;

	if (is_offline_panel(&pinfo->base))
		return;

	present = (struct comp_online_present *)dpu_comp->present_data;

	if (is_ppc_support(&pinfo->base) && present->ppc_ctrl.release_data)
		present->ppc_ctrl.release_data(&present->ppc_ctrl);
}

void composer_present_power_on(struct dpu_composer *dpu_comp)
{
	struct dkmd_connector_info *pinfo = dpu_comp->conn_info;
	struct comp_online_present *present = NULL;

	if (is_offline_panel(&pinfo->base)) {
		composer_offline_setup(dpu_comp, (struct comp_offline_present *)dpu_comp->present_data);
		return;
	}

	present = (struct comp_online_present *)dpu_comp->present_data;
	present->vactive_start_flag = 1;
	present->frame_start_flag = 0;
	present->buffers = 0;

	dkmd_isr_request(&dpu_comp->isr_ctrl);
	dpu_vsync_enable_routine(&present->vsync_ctrl);
	dkmd_timeline_enable_routine(&present->timeline);
	comp_mntn_enable_routine(&present->comp_maintain);

	composer_present_dfr_setup(dpu_comp);

	composer_present_ppc_setup(dpu_comp);

	/* NOTICE: stub max perf config when dptx connect */
	if (is_dp_panel(&pinfo->base)) {
		dpu_comp_active_vsync(dpu_comp);
		dpu_dvfs_direct_vote(dpu_comp->comp.index, DPU_PERF_LEVEL_MAX, true);
	}
}

void composer_present_power_off(struct dpu_composer *dpu_comp, bool is_dpu_poweroff)
{
	struct dkmd_connector_info *pinfo = dpu_comp->conn_info;
	struct comp_online_present *present = NULL;

	if (is_offline_panel(&pinfo->base)) {
		composer_offline_release(dpu_comp, (struct comp_offline_present *)dpu_comp->present_data);
		return;
	}

	present = (struct comp_online_present *)dpu_comp->present_data;

	dpu_comp->isr_ctrl.handle_func(&dpu_comp->isr_ctrl, DKMD_ISR_RELEASE);
	dpu_vsync_disable_routine(&present->vsync_ctrl);
	dkmd_timeline_disable_routine(&present->timeline);
	comp_mntn_disable_routine(&present->comp_maintain);

	composer_present_dfr_release(dpu_comp);

	composer_present_ppc_release(dpu_comp);

	if (is_dp_panel(&pinfo->base)) {
		/* NOTICE: if only dp was disconnected, cancel max perf config and recovery dvfs vote.
		 * One good solution is to each composer device to record their vote as well,
		 * and then cancel vote when do not need.
		 */
		if (!is_dpu_poweroff)
			dpu_dvfs_reset_comp_vote(dpu_comp->comp.index);

		dpu_comp_deactive_vsync(dpu_comp);
	}
}

void composer_present_data_setup(struct dpu_composer *dpu_comp, bool inited)
{
	struct dkmd_connector_info *pinfo = dpu_comp->conn_info;
	struct comp_online_present *present = NULL;

	(void)inited;
	if (is_offline_panel(&pinfo->base))
		return;

	present = (struct comp_online_present *)dpu_comp->present_data;
	pipeline_next_ops_handle(pinfo->conn_device, pinfo, SETUP_ISR, (void *)&dpu_comp->isr_ctrl);
	dkmd_isr_setup(&dpu_comp->isr_ctrl);
	list_add_tail(&dpu_comp->isr_ctrl.list_node, &dpu_comp->comp_mgr->isr_list);
	dpu_comp_add_attrs(&dpu_comp->attrs);

	dpu_comp_dfr_ctrl_setup(dpu_comp, present);
	composer_online_setup(dpu_comp, present);

	dpu_comp_ppc_ctrl_setup(dpu_comp, present);
	dpu_tui_register(dpu_comp);
	dpu_comp_esd_register(dpu_comp, present);
	dpu_low_temp_register(dpu_comp, present);
}

void composer_present_data_release(struct dpu_composer *dpu_comp, bool is_poweroff)
{
	struct comp_online_present *present = NULL;

	(void)is_poweroff;
	if (is_offline_panel(&dpu_comp->conn_info->base))
		return;

	present = (struct comp_online_present *)dpu_comp->present_data;
	composer_online_release(dpu_comp, present);
	list_del(&dpu_comp->isr_ctrl.list_node);
	dpu_tui_unregister(dpu_comp);
	dpu_comp_esd_unregister(dpu_comp);
	dpu_low_temp_unregister(dpu_comp, present);
}

int32_t composer_present_dfr_send_dcs_by_riscv(struct dpu_composer *dpu_comp, uint32_t bl_level)
{
	struct comp_online_present *present = NULL;

	if (is_offline_panel(&dpu_comp->conn_info->base)) {
		dpu_pr_info("offline scene, return!\n");
		return -1;
	}
	present = (struct comp_online_present *)dpu_comp->present_data;

	if (present->dfr_ctrl.send_dcs_cmds_with_refresh(&present->dfr_ctrl, bl_level) == -1) {
		dpu_pr_err("send dcs cmds with refresh error!\n");
		return -1;
	}
	return 0;
}

int32_t composer_present_set_safe_frm_rate(struct dpu_composer *dpu_comp, uint32_t safe_frm_rate)
{
	struct comp_online_present *present = NULL;

	if (is_offline_panel(&dpu_comp->conn_info->base) || !dpu_comp->present_data) {
		dpu_pr_info("offline scene or present_data is nullptr, return!\n");
		return -1;
	}

	present = (struct comp_online_present *)dpu_comp->present_data;
	if (!present->dfr_ctrl.set_safe_frm_rate) {
		dpu_pr_warn("Need not to set safe frm rate!");
		return -1;
	}
	dpu_pr_info("safe_frm_rate = %u!", safe_frm_rate);
	return present->dfr_ctrl.set_safe_frm_rate(&present->dfr_ctrl, safe_frm_rate);
}

int32_t composer_present_set_active_rect(struct dpu_composer *dpu_comp, uint32_t ppc_config_id)
{
	struct comp_online_present *present = NULL;
	struct dkmd_connector_info *pinfo = dpu_comp->conn_info;
	struct dpu_connector *connector = get_primary_connector(pinfo);
	bool is_need_switch = false;

	if (dpu_comp_preprocess_set_active_rect(dpu_comp, ppc_config_id, &is_need_switch) != 0)
		return -1;

	if (!is_need_switch)
		return 0;

	present = (struct comp_online_present *)dpu_comp->present_data;
	if (unlikely(!present)) {
		dpu_pr_err("present is null");
		return -1;
	}
	dpu_comp_active_vsync(dpu_comp);
	dpu_pr_info("+");

	process_ppc_event(dpu_comp, PPC_EVENT_WAIT_SWITCH);
	if (is_ppc_status_switchable(&present->ppc_ctrl)) {
		dpu_pr_info("ppc status switchable, can set active rect");
	} else if (wait_event_switch_ppc_status(present, connector) != 0) {
		process_ppc_event(dpu_comp, PPC_EVENT_WAIT_VSYNC_TIMEOUT);
		dpu_comp_deactive_vsync(dpu_comp);
		return 0;
	}

	if (present->ppc_ctrl.set_active_rect(&present->ppc_ctrl, ppc_config_id) != 0) {
		dpu_pr_warn("set active rect failed!");
		process_ppc_event(dpu_comp, PPC_EVENT_DONE);
		dpu_comp_deactive_vsync(dpu_comp);
		return -1;
	}

	pipeline_next_ops_handle(pinfo->conn_device, pinfo, MIPI_DSI_PPC_SET_REG, &ppc_config_id);

	process_ppc_event(dpu_comp, PPC_EVENT_DONE);

	dpu_comp_deactive_vsync(dpu_comp);
	return 0;
}
