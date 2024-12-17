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
#include <linux/delay.h>
#include <dpu/soc_dpu_define.h>
#include <linux/interrupt.h>
#include <linux/atomic.h>
#include <dpu/dpu_itfsw.h>

#include "dkmd_acquire_fence.h"
#include "dkmd_release_fence.h"
#if defined(CONFIG_WLT_DISPLAY)
#include "dpu_wlt.h"
#endif
#include "cmdlist_interface.h"
#include "dpu_comp_mgr.h"
#include "dpu_comp_online.h"
#include "dpu_comp_vactive.h"
#include "dpu_comp_maintain.h"
#include "dpu_cmdlist.h"
#include "dpu_isr.h"
#include "dpu_comp_abnormal_handle.h"
#include "dpu_comp_frame_isr.h"
#include "dvfs.h"
#include "panel/panel_drv.h"
#include "dpu_comp_secure.h"
#include "dpu_comp_bl.h"
#include "dpu_comp_init.h"
#include "effect/hiace/dpu_effect_hiace.h"
#include "dpu_comp_mipi_dsi_bit_clk_upt.h"
#include "mipi_dsi_bit_clk_upt_helper.h"
#include "gfxdev_mgr.h"
#include "dpu_comp_online_utils.h"
#include "dkmd_fence_utils.h"
#include "dpu_ppc_status_control.h"

static void dpu_comp_clear_fastboot_config(struct dpu_composer *dpu_comp)
{
	char __iomem *dpp_base = dpu_comp->comp_mgr->dpu_base + DPU_DPP0_OFFSET;
	char __iomem *dsc_base = dpu_comp->comp_mgr->dpu_base + DPU_DSC0_OFFSET;
	char __iomem *itfch_base = dpu_comp->comp_mgr->dpu_base + DPU_ITF_CH0_OFFSET;
	char __iomem *dpp1_base = dpu_comp->comp_mgr->dpu_base + DPU_DPP1_OFFSET;
	char __iomem *itfch1_base = dpu_comp->comp_mgr->dpu_base + DPU_ITF_CH1_OFFSET;

	if (dpu_comp->conn_info->base.pipe_sw_itfch_idx == PIPE_SW_PRE_ITFCH0) {
		/* clear reg_ctrl_debug */
		set_reg(DPU_DPP_REG_CTRL_DEBUG_ADDR(dpp_base), 0, 1, 10);
		set_reg(DPU_DSC_REG_CTRL_DEBUG_ADDR(dsc_base), 0, 1, 10);
		set_reg(DPU_ITF_CH_REG_CTRL_DEBUG_ADDR(itfch_base), 0, 1, 10);
	} else if (dpu_comp->conn_info->base.pipe_sw_itfch_idx == PIPE_SW_PRE_ITFCH1) {
		/* clear reg_ctrl_debug */
		set_reg(DPU_DPP_REG_CTRL_DEBUG_ADDR(dpp1_base), 0, 1, 10);
		set_reg(DPU_ITF_CH_REG_CTRL_DEBUG_ADDR(itfch1_base), 0, 1, 10);
	}

	dpu_pr_info("%d booting complete, change fastboot config", dpu_comp->comp.index);
}

static bool need_direct_vote(struct dpu_composer *dpu_comp)
{
	int32_t i;
	uint32_t power_on_comp_cnt = 0;
	bool is_multi_panel_display;
	bool is_lem_buildin;

	for (i = DEVICE_COMP_PRIMARY_ID; i <= DEVICE_COMP_BUILTIN_ID; i++) {
		if (dpu_comp->comp_mgr->power_status.refcount.value[i] != 0)
			power_on_comp_cnt++;
	}

	// power_status is not locked, power_status may be changed in later process and cause unknown problem.
	is_multi_panel_display = (power_on_comp_cnt > 1);
	dpu_pr_debug("multi panel display=%d", is_multi_panel_display);

	is_lem_buildin = (dpu_comp->conn_info->base.pipe_sw_itfch_idx == PIPE_SW_PRE_ITFCH1) &&
		(dpu_comp->comp.index == DEVICE_COMP_BUILTIN_ID);

	return is_multi_panel_display || is_lem_buildin || is_mipi_video_panel(&dpu_comp->conn_info->base);
}

static void disable_intra_dvfs_scene(struct dpu_composer *dpu_comp, struct intra_frame_dvfs_info *dvfs_info)
{
	if ((dpu_comp->secure_ctrl.secure_event == TUI_SEC_ENABLE) ||
		is_mipi_video_panel(&dpu_comp->conn_info->base) ||
		is_multi_device_active(dpu_comp->comp_mgr))
		dvfs_info->is_supported_intra_dvfs = false;
}

static void composer_online_preprocess(struct comp_online_present *present, struct disp_frame *frame)
{
	int32_t i;
	uint64_t wait_fence_tv;
	struct disp_layer *layer = NULL;
	struct dpu_composer *dpu_comp = NULL;
	struct dkmd_connector_info *pinfo = NULL;
	struct dpu_comp_frame *using_frame = &present->frames[present->incoming_idx];

	dpu_comp = present->dpu_comp;
	pinfo = dpu_comp->conn_info;
	dpu_trace_ts_begin(&wait_fence_tv);

	frame->dvfs_info.frame_rate = present->frame_rate;
	frame->dvfs_info.sw_dvfs_frm_rate = pinfo->base.sw_dvfs_frm_rate;

	/* For scenes we can not enable intra dvfs. use param is_supported_intra_dvfs to ban those scenes */
	disable_intra_dvfs_scene(dpu_comp, &frame->dvfs_info);
	dpu_dvfs_intra_frame_vote(present->dpu_comp->comp.index, &frame->dvfs_info,
		need_direct_vote(present->dpu_comp));

	/* lock dma buf and wait layer acquired fence */
	using_frame->in_frame = *frame;
	for (i = 0; i < (int32_t)frame->layer_count; ++i) {
		layer = &frame->layer[i];
		dkmd_buf_sync_lock_dma_buf(&present->timeline, layer->share_fd, frame->present_fence_pt);
		dkmd_acquire_fence_wait_fd(layer->acquired_fence, ACQUIRE_FENCE_TIMEOUT_MSEC);
		/* fd need be close by HAL */
		layer->acquired_fence = -1;
	}

	/**
	 * @brief increase timeline step value, normally inc step is 1
	 * vsync isr will increase the step with pt_value
	 */
	dkmd_timeline_inc_step(&present->timeline);

#if defined(CONFIG_WLT_DISPLAY)
	/* set slice0 addr for wlt */
	dpu_wlt_set_slice0_addr(dpu_comp->comp_mgr->dpu_base);
#endif

	/* check cmdlist_id and lock */
	dpu_cmdlist_sync_lock(&present->timeline, frame);

	/**
	 * @brief After three frames, free logo buffer,
	 * g_dpu_complete_start need set true after booting complete
	 */
	if ((frame->frame_index > 3) && !dpu_comp->dpu_boot_complete) {
		dpu_comp->dpu_boot_complete = true;
		dpu_comp_clear_fastboot_config(dpu_comp);

		if (!g_dpu_complete_start && composer_dpu_free_logo_buffer(dpu_comp)) {
			g_dpu_complete_start = true; /* executed only once */
			dpu_pr_info("booting complete, change dpu compelte flag");
		}
	}
	dpu_trace_ts_end(&wait_fence_tv, "online compose wait gpu fence");
}

static bool composer_present_need_hold(struct dpu_composer *dpu_comp)
{
	if ((g_debug_present_hold != 0) && ((dpu_comp->comp.comp_err_status & ABNORMAL_UNDERFLOW) != 0)) {
		dpu_pr_warn("composer present hold start");
		mdelay(DPU_COMPOSER_HOLD_TIME);
		dpu_pr_warn("composer present hold end");
		return true;
	}
	return false;
}

static int32_t composer_online_overlay(struct dpu_composer *dpu_comp, struct disp_frame *frame)
{
	struct composer_scene *scene = NULL;
	uint32_t scene_id = (uint32_t)frame->scene_id;
	struct comp_online_present *present = (struct comp_online_present *)dpu_comp->present_data;
	struct disp_frame *displaying_frame = &present->frames[present->displaying_idx].in_frame;
	struct disp_frame *displayed_frame = &present->frames[present->displayed_idx].in_frame;
	struct dpu_connector* connector = get_primary_connector(dpu_comp->conn_info);

	if (unlikely(scene_id > DPU_SCENE_ONLINE_3)) {
		dpu_pr_err("invalid scene_id=%u", scene_id);
		return -1;
	}

	if (unlikely(is_ppc_support(&dpu_comp->conn_info->base) &&
		frame->ppc_config_id != dpu_comp->conn_info->ppc_config_id_record)) {
		dpu_pr_warn("frame's ppc_config_id=%u mismatch conn_info's ppc_config_id=%u",
			frame->ppc_config_id, dpu_comp->conn_info->ppc_config_id_record);
		return -1;
	}

	scene = dpu_comp->comp_mgr->scene[scene_id];
	if (unlikely(!scene)) {
		dpu_pr_err("unsupport scene_id=%u", scene_id);
		return -1;
	}

	if (unlikely(scene->present == NULL)) {
		dpu_pr_err("scene present func ptr is null");
		return -1;
	}

	if (composer_present_need_hold(dpu_comp)) {
		dpu_pr_warn("skip composer online overlay");
		return 0;
	}
	dpu_comp_active_vsync(dpu_comp);
	composer_online_update_frame_rate(present, frame->active_frame_rate);

	dpu_pr_debug("scene id %u, frame_index=%u: frame_rate=%u vactive_start_flag=%u enter", scene_id, frame->frame_index,
		present->frame_rate, present->vactive_start_flag);

#ifdef CONFIG_DKMD_DEBUG_ENABLE
	present->buffers++;
	trace_buffer_num(present->buffers);
#endif

	/* wait vactive isr */
	if (dpu_comp_vactive_wait_event(present) != 0) {
		dpu_pr_err("scene_id=%u wait vactive timeout! please check config and resync timeline!", scene_id);
		(void)pipeline_next_ops_handle(dpu_comp->conn_info->conn_device,
			dpu_comp->conn_info, DUMP_EXCEPTION_INFO, NULL);
		dpu_comp_deactive_vsync(dpu_comp);
		return -1;
	}

	process_ppc_event(dpu_comp, PPC_EVENT_PRESENT);

	if (atomic_read(&connector->mipi.dsi_bit_clk_upt_flag) == 1) {
		if (wait_for_mipi_resource_available(connector) != 0)
			dpu_pr_info("try to change mipi clk in next frame");
	}

	// set display region before display
	pipeline_next_ops_handle(dpu_comp->conn_info->conn_device,
		dpu_comp->conn_info, MIPI_DSI_PARTIAL_UPDATE, &frame->disp_rect);

	composer_online_preprocess(present, frame);

	/* commit to hardware, but sometimes hardware is handling exceptions */
	scene->frame_index = frame->frame_index;
	dpu_comp_scene_switch(dpu_comp->conn_info, scene);  // maybe dp need this config

	if ((dpu_comp->alsc) && (dpu_comp->alsc->comp_alsc_set_reg))
		dpu_comp->alsc->comp_alsc_set_reg(dpu_comp, &(frame->disp_rect), frame->alsc_en);

	if (dpu_comp->hiace_ctrl)
		dpu_comp->hiace_ctrl->lut_apply(dpu_comp);

	scene->present(scene, frame->cmdlist_id);
	/* cpu config drm layer */
	dpu_online_drm_layer_config(displayed_frame, displaying_frame, frame);
	dpu_comp->isr_ctrl.unmask &= ~DSI_INT_UNDER_FLOW;

	dpu_comp_dfr_ctrl_process(dpu_comp, present, frame);
	dpu_comp_deactive_vsync(dpu_comp);

	if (!is_dp_panel(&dpu_comp->conn_info->base) && !is_hdmi_panel(&dpu_comp->conn_info->base))
		dpu_backlight_update(&dpu_comp->bl_ctrl, 0);

	dpu_online_utiles_post_process(present);

	dpu_pr_debug("vactive_start_flag=%d exit", present->vactive_start_flag);
	return 0;
}

static int32_t composer_online_create_fence(struct dpu_composer *dpu_comp, struct disp_present_fence *fence)
{
	struct comp_online_present *present = (struct comp_online_present *)dpu_comp->present_data;

	fence->fence_fd = dkmd_release_fence_create(&present->timeline, &fence->fence_pt);
	return 0;
}

static int32_t composer_online_release_fence(struct dpu_composer *dpu_comp, int32_t fd)
{
	struct comp_online_present *present = (struct comp_online_present *)dpu_comp->present_data;
	struct dkmd_timeline *timeline = NULL;
	struct dkmd_isr *isr_ctrl = NULL;
	int32_t ret;

	if (unlikely(present == NULL)) {
		dpu_pr_err("present is null");
		return -1;
	}

	if (unlikely(fd < 0)) {
		dpu_pr_debug("fd < 0");
		return 0;
	}

	if (g_debug_fence_timeline)
		dpu_pr_info("signal present fence %d", fd);

	timeline = &present->timeline;
	dkmd_timeline_dec_next_value(timeline);
	ret = dkmd_fence_signal_fence(fd);

	isr_ctrl = (struct dkmd_isr *)timeline->isr;
	if (unlikely(!isr_ctrl)) {
		dpu_pr_err("isr_ctrl is null");
		return -1;
	}
	dkmd_timeline_set_reset_flag(timeline, true);
	dkmd_isr_notify_listener(isr_ctrl, timeline->listening_isr_bit);
	dkmd_timeline_set_reset_flag(timeline, false);
	return ret;
}

static void composer_online_vsync_init(struct dpu_composer *dpu_comp, struct comp_online_present *present)
{
	uint32_t te_bit;
	uint32_t vsync_bit;
	struct dkmd_isr *isr_ctrl = NULL;
	struct dkmd_connector_info *pinfo = dpu_comp->conn_info;
	struct dpu_comp_dfr_ctrl* dfr_ctrl = &present->dfr_ctrl;

	dpu_vsync_init(&present->vsync_ctrl, &dpu_comp->attrs);
	present->vsync_ctrl.dpu_comp = dpu_comp;

	switch (dfr_ctrl->mode) {
	case DFR_MODE_CONSTANT:
	case DFR_MODE_LONG_V:
	case DFR_MODE_LONG_H:
	case DFR_MODE_LONG_VH:
		dpu_pr_info("lcd_te_idx:%d,", pinfo->base.lcd_te_idx);
		te_bit = pinfo->base.lcd_te_idx == 0 ? DSI_INT_LCD_TE0 : DSI_INT_LCD_TE1;
		vsync_bit = (is_mipi_cmd_panel(&pinfo->base) && (!is_force_update(&pinfo->base))) ? te_bit : DSI_INT_VSYNC;
		isr_ctrl = &dpu_comp->isr_ctrl;
		break;
	case DFR_MODE_TE_SKIP_BY_MCU:
	case DFR_MODE_LONGH_TE_SKIP_BY_MCU:
		vsync_bit = NOTIFY_BOTH_VSYNC_TIMELINE;
		isr_ctrl = &dpu_comp->comp_mgr->mdp_isr_ctrl;
		break;
	case DFR_MODE_TE_SKIP_BY_ACPU:
		vsync_bit = DSI_INT_VSYNC_COUNT_BY_TE;
		isr_ctrl = &dpu_comp->isr_ctrl;
		break;
	default:
		dpu_pr_err("invalid dfr mode %d", dfr_ctrl->mode);
		return;
	}

	present->vsync_ctrl.isr = isr_ctrl;
	present->vsync_ctrl.listening_isr_bit = vsync_bit;
	dkmd_isr_register_listener(isr_ctrl, present->vsync_ctrl.notifier,
		present->vsync_ctrl.listening_isr_bit, &present->vsync_ctrl);
}

static void composer_online_vsync_deinit(struct dpu_composer *dpu_comp, struct comp_online_present *present)
{
	struct dkmd_isr *isr_ctrl = NULL;
	struct dpu_comp_dfr_ctrl* dfr_ctrl = &present->dfr_ctrl;

	dpu_pr_info("+");

	switch (dfr_ctrl->mode) {
	case DFR_MODE_CONSTANT:
	case DFR_MODE_LONG_V:
	case DFR_MODE_LONG_H:
	case DFR_MODE_LONG_VH:
		isr_ctrl = &dpu_comp->isr_ctrl;
		break;
	case DFR_MODE_TE_SKIP_BY_MCU:
	case DFR_MODE_LONGH_TE_SKIP_BY_MCU:
		isr_ctrl = &dpu_comp->comp_mgr->mdp_isr_ctrl;
		dpu_pr_info("skip by mcu, mdp isr");
		break;
	case DFR_MODE_TE_SKIP_BY_ACPU:
		isr_ctrl = &dpu_comp->isr_ctrl;
		dpu_pr_info("skip by acpu, dsi isr");
		break;
	default:
		dpu_pr_err("invalid dfr mode %d", dfr_ctrl->mode);
		return;
	}

	dkmd_isr_unregister_listener(isr_ctrl, present->vsync_ctrl.notifier, present->vsync_ctrl.listening_isr_bit);
	present->vsync_ctrl.listening_isr_bit = 0;

	dpu_pr_info("-");
}

static void composer_online_timeline_init(struct dpu_composer *dpu_comp, struct comp_online_present *present)
{
	uint32_t te_bit;
	uint32_t timeline_bit;
	char tmp_name[256] = {0};
	struct dkmd_isr *isr_ctrl = NULL;
	struct dkmd_connector_info *pinfo = dpu_comp->conn_info;
	struct dpu_comp_dfr_ctrl* dfr_ctrl = &present->dfr_ctrl;

	(void)snprintf(tmp_name, sizeof(tmp_name), "online_composer_%s", dpu_comp->comp.base.name);
	dkmd_timeline_init(&present->timeline, tmp_name, dpu_comp);
	present->timeline.present_handle_worker = &dpu_comp->handle_worker;

	switch (dfr_ctrl->mode) {
	case DFR_MODE_CONSTANT:
	case DFR_MODE_LONG_V:
	case DFR_MODE_LONG_H:
	case DFR_MODE_LONG_VH:
		te_bit = pinfo->base.lcd_te_idx == 0 ? DSI_INT_LCD_TE0 : DSI_INT_LCD_TE1;
		timeline_bit = (is_mipi_cmd_panel(&pinfo->base) && (!is_force_update(&pinfo->base))) ? te_bit : DSI_INT_VSYNC;
		isr_ctrl = &dpu_comp->isr_ctrl;
		break;
	case DFR_MODE_TE_SKIP_BY_MCU:
	case DFR_MODE_LONGH_TE_SKIP_BY_MCU:
		timeline_bit = NOTIFY_BOTH_VSYNC_TIMELINE;
		isr_ctrl = &dpu_comp->comp_mgr->mdp_isr_ctrl;
		break;
	case DFR_MODE_TE_SKIP_BY_ACPU:
		timeline_bit = DSI_INT_VSYNC_COUNT_BY_TE;
		isr_ctrl = &dpu_comp->isr_ctrl;
		break;
	default:
		dpu_pr_err("invalid dfr mode %d", dfr_ctrl->mode);
		return;
	}

	present->timeline.isr = isr_ctrl;
	present->timeline.listening_isr_bit = timeline_bit;
	dkmd_isr_register_listener(isr_ctrl, present->timeline.notifier,
		present->timeline.listening_isr_bit, &present->timeline);
}

static void composer_online_timeline_deinit(struct dpu_composer *dpu_comp, struct comp_online_present *present)
{
	struct dkmd_isr *isr_ctrl = NULL;
	struct dpu_comp_dfr_ctrl* dfr_ctrl = &present->dfr_ctrl;

	dpu_pr_info("+");

	switch (dfr_ctrl->mode) {
	case DFR_MODE_CONSTANT:
	case DFR_MODE_LONG_V:
	case DFR_MODE_LONG_H:
	case DFR_MODE_LONG_VH:
		isr_ctrl = &dpu_comp->isr_ctrl;
		break;
	case DFR_MODE_TE_SKIP_BY_MCU:
	case DFR_MODE_LONGH_TE_SKIP_BY_MCU:
		isr_ctrl = &dpu_comp->comp_mgr->mdp_isr_ctrl;
		dpu_pr_info("skip by mcu, mdp isr");
		break;
	case DFR_MODE_TE_SKIP_BY_ACPU:
		isr_ctrl = &dpu_comp->isr_ctrl;
		dpu_pr_info("skip by acpu, dsi isr");
		break;
	default:
		dpu_pr_err("invalid dfr mode %d", dfr_ctrl->mode);
		return;
	}

	if (present->timeline.listening_isr_bit != 0) {
		dkmd_isr_unregister_listener(isr_ctrl, present->timeline.notifier, present->timeline.listening_isr_bit);
		present->timeline.listening_isr_bit = 0;
	}

	dpu_pr_info("-");
}

static void composer_online_alsc_init(struct dpu_composer *dpu_comp)
{
	struct composer *comp = &dpu_comp->comp;
	struct dkmd_connector_info *pinfo = dpu_comp->conn_info;

	if (!comp->register_alsc) {
		dpu_pr_info("not need support alsc");
		return;
	}

	if (is_builtin_panel(&pinfo->base)) {
		comp->register_alsc(comp, DEVICE_COMP_BUILTIN_ID, DPU_DPP0_OFFSET);
		return;
	}

	comp->register_alsc(comp, DEVICE_COMP_PRIMARY_ID, DPU_DPP0_OFFSET);
}

static void composer_online_alsc_deinit(struct dpu_composer *dpu_comp)
{
	struct composer *comp = &dpu_comp->comp;
	struct dkmd_connector_info *pinfo = dpu_comp->conn_info;

	if (!comp->unregister_alsc) {
		dpu_pr_info("not need support alsc");
		return;
	}

	if (is_dp_panel(&pinfo->base) ||
		is_hdmi_panel(&pinfo->base))
		return;

	comp->unregister_alsc(comp);
}

void composer_online_setup(struct dpu_composer *dpu_comp, struct comp_online_present *present)
{
	struct dkmd_isr *isr_ctrl = &dpu_comp->isr_ctrl;
	struct dkmd_connector_info *pinfo = dpu_comp->conn_info;
	struct dpu_comp_dfr_ctrl* dfr_ctrl = &present->dfr_ctrl;

	composer_online_vsync_init(dpu_comp, present);
	composer_online_timeline_init(dpu_comp, present);
	dpu_comp_vactive_init(isr_ctrl, dpu_comp, DSI_INT_VACT0_START);
	dpu_comp_vactive_end_init(isr_ctrl, dpu_comp, DSI_INT_VACT0_END);
	dpu_comp_abnormal_handle_init(isr_ctrl, dpu_comp, DSI_INT_UNDER_FLOW);
	dpu_comp_frame_start_isr_init(isr_ctrl, dpu_comp, DSI_INT_FRM_START);
	dpu_comp_frame_end_isr_init(isr_ctrl, dpu_comp, DSI_INT_FRM_END);

	/* register refresh isr to screen(LTPO) for fps count */
	if (dfr_ctrl->mode == DFR_MODE_TE_SKIP_BY_MCU || dfr_ctrl->mode == DFR_MODE_LONGH_TE_SKIP_BY_MCU) {
		present->comp_maintain.dpu_comp = dpu_comp;
		comp_mntn_refresh_stat_init(&dpu_comp->comp_mgr->mdp_isr_ctrl, &present->comp_maintain, NOTIFY_REFRESH);
	} else if (dfr_ctrl->mode == DFR_MODE_TE_SKIP_BY_ACPU) {
		present->comp_maintain.dpu_comp = dpu_comp;
		comp_mntn_refresh_stat_init(isr_ctrl, &present->comp_maintain, NOTIFY_REFRESH);
	}

	if (!is_dp_panel(&pinfo->base) && !is_hdmi_panel(&pinfo->base)) {
		if (pinfo->base.pipe_sw_itfch_idx == PIPE_SW_PRE_ITFCH0) {
			dpu_comp_alsc_handle_init(isr_ctrl, dpu_comp, DSI_INT_VACT0_START); // alsc only for primary
			dpu_comp_alsc_handle_init(isr_ctrl, dpu_comp, DSI_INT_VACT0_END);
			dpu_comp_hdr_handle_init(isr_ctrl, dpu_comp, DSI_INT_VACT0_END);
			dpu_comp_hiace_handle_init(&dpu_comp->comp_mgr->sdp_isr_ctrl, dpu_comp, DPU_DPP0_HIACE_NS_INT);
			composer_online_alsc_init(dpu_comp);
			dpu_effect_hiace_init(dpu_comp, DPU_DPP0_OFFSET);
		}
		// register sysfs bl file
		dpu_backlight_init(&dpu_comp->bl_ctrl, &dpu_comp->attrs, dpu_comp);
		dpu_comp_mipi_dsi_bit_clk_upt_init(dpu_comp);
	}

	present->dpu_comp = dpu_comp;
	dpu_comp->overlay = composer_online_overlay;
	dpu_comp->create_fence = composer_online_create_fence;
	dpu_comp->release_fence = composer_online_release_fence;
	present->buffers = 0;
}

void composer_online_release(struct dpu_composer *dpu_comp, struct comp_online_present *present)
{
	struct dkmd_isr *isr_ctrl = &dpu_comp->isr_ctrl;
	struct dpu_comp_dfr_ctrl* dfr_ctrl = &present->dfr_ctrl;

	composer_online_timeline_deinit(dpu_comp, present);
	composer_online_vsync_deinit(dpu_comp, present);
	dpu_comp_vactive_deinit(isr_ctrl, DSI_INT_VACT0_START);
	dpu_comp_vactive_end_deinit(isr_ctrl, DSI_INT_VACT0_END);
	dpu_comp_abnormal_handle_deinit(isr_ctrl, DSI_INT_UNDER_FLOW);
	dpu_comp_frame_start_isr_deinit(isr_ctrl, DSI_INT_FRM_START);
	dpu_comp_frame_end_isr_deinit(isr_ctrl, DSI_INT_FRM_END);

	/* unregister refresh isr to screen(LTPO) for fps count */
	if (dfr_ctrl->mode == DFR_MODE_TE_SKIP_BY_MCU || dfr_ctrl->mode == DFR_MODE_LONGH_TE_SKIP_BY_MCU)
		comp_mntn_refresh_stat_deinit(&dpu_comp->comp_mgr->mdp_isr_ctrl, NOTIFY_REFRESH);
	else if (dfr_ctrl->mode == DFR_MODE_TE_SKIP_BY_ACPU)
		comp_mntn_refresh_stat_deinit(isr_ctrl, NOTIFY_REFRESH);

	if (!is_dp_panel(&dpu_comp->conn_info->base) && !is_hdmi_panel(&dpu_comp->conn_info->base)) {
		if (dpu_comp->conn_info->base.pipe_sw_itfch_idx == PIPE_SW_PRE_ITFCH0) {
			dpu_comp_alsc_handle_deinit(isr_ctrl, DSI_INT_VACT0_START);
			dpu_comp_alsc_handle_deinit(isr_ctrl, DSI_INT_VACT0_END);
			dpu_comp_hdr_handle_deinit(isr_ctrl, DSI_INT_VACT0_END);
			dpu_comp_hiace_handle_deinit(&dpu_comp->comp_mgr->sdp_isr_ctrl, DPU_DPP0_HIACE_NS_INT);
			composer_online_alsc_deinit(dpu_comp);
			dpu_effect_hiace_deinit(dpu_comp);
		}
	}
}
