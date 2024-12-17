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

#include <platform_include/see/bl31_smc.h>
#include <platform_include/display/linux/dpu_drmdriver.h>
#include "dkmd_log.h"
#include "dpu_comp_secure.h"

static uint32_t get_drm_layer_id_from_frame(struct disp_frame * frame)
{
	uint32_t i;
	struct disp_layer *layer = NULL;
	uint32_t layers_id = 0;

	for (i = 0; i < frame->layer_count; ++i) {
		layer = &frame->layer[i];
		if (layer->is_protected_mode != true) // pre_pre_frame this layer not secure
			continue;
		layers_id = layers_id | BIT(i);
	}

	// TUI use CLIENT Compose, tui occupancy layer is BIT(1), drm layer is BIT(0).
	if ((layers_id != 0) && (frame->tui_ready_status != 0))
		layers_id |= BIT(1);

	return layers_id;
}

static void dpu_drm_layer_mmu_proc_config(uint32_t layer_id, uint32_t scene_id)
{
	configure_dss_service_security(FID_BL31_DISPLAY_SDMA_MMU_SEC_CONFIG, layer_id, scene_id, 0);
}

static void dpu_drm_layer_mmu_proc_clear(uint32_t layer_id, uint32_t scene_id)
{
	configure_dss_service_security(FID_BL31_DISPLAY_SDMA_MMU_SEC_DECONFIG, layer_id, scene_id, 0);
}

static void dpu_drm_layer_dacc_prop_config(uint32_t layer_id, uint32_t scene_id)
{
	configure_dss_service_security(FID_BL31_DISPLAY_DACC_LAYER_PROP_SEC_CONFIG, layer_id, scene_id, 0);
}

void dpu_online_drm_layer_config(struct disp_frame *prev_prev_frame,
		struct disp_frame *prev_frame, struct disp_frame *curr_frame)
{
	uint32_t curr_layers_id = 0;
	uint32_t prev_layers_id = 0;
	uint32_t prev_prev_layers_id = 0;
	uint32_t mmu_proc_clear_layer_id = 0;
	if (prev_prev_frame == NULL || prev_frame == NULL || curr_frame == NULL) {
		dpu_pr_err("notify prev_prev_frame or prev_frame or curr_frame is NULL!\n");
		return;
	}
	curr_layers_id = get_drm_layer_id_from_frame(curr_frame);
	prev_layers_id = get_drm_layer_id_from_frame(prev_frame);
	prev_prev_layers_id = get_drm_layer_id_from_frame(prev_prev_frame);
	/* if curr layer is drm or pre layer is drm or pre_pre layer is normal, not clear the register;
	 * mmu_proc_clear_layer_id != 0, need clear;mmu_proc_clear_layer_id = 0, no need clear;
	 * because clt protect_en is rw register, before is reload;
	 */
	mmu_proc_clear_layer_id = ~(curr_layers_id | prev_layers_id | (~prev_prev_layers_id));
	dpu_pr_debug("notify curr_layers_id=%d, prev_layers_id=%d, prev_prev_layers_id=%d\n",
		curr_layers_id, prev_layers_id, prev_prev_layers_id);
	dpu_pr_debug("notify mmu_proc_clear_layer_id=%u\n", mmu_proc_clear_layer_id);
	if (curr_layers_id != 0) { // curr frame has drm layer
		dpu_drm_layer_mmu_proc_config(curr_layers_id, (uint32_t)curr_frame->scene_id);
		dpu_drm_layer_dacc_prop_config(curr_layers_id, (uint32_t)curr_frame->scene_id);
	}

	if (mmu_proc_clear_layer_id != 0) // clear prev_prev_frame drm layer protect_en config
		dpu_drm_layer_mmu_proc_clear(mmu_proc_clear_layer_id, (uint32_t)prev_prev_frame->scene_id);

	return;
}

static void dpu_drm_layer_wch_mmu_proc_config(uint32_t wch_id, uint32_t scene_id)
{
	configure_dss_service_security(FID_BL31_DISPLAY_WCH_MMU_SEC_CONFIG, wch_id, scene_id, 0);
}

static void dpu_drm_layer_wch_mmu_proc_clear(uint32_t wch_id, uint32_t scene_id)
{
	configure_dss_service_security(FID_BL31_DISPLAY_WCH_MMU_SEC_DECONFIG, wch_id, scene_id, 0);
}

void dpu_offline_drm_layer_config(struct disp_frame *curr_frame)
{
	uint32_t curr_layers_id = 0;
	if (curr_frame == NULL) {
		dpu_pr_err("notify curr_frame is NULL!\n");
		return;
	}
	curr_layers_id = get_drm_layer_id_from_frame(curr_frame);
	dpu_pr_debug("notify offline_drm_layer_config curr_layers_id=%d\n", curr_layers_id);
	if (curr_layers_id != 0) { // curr frame has drm layer
		dpu_drm_layer_mmu_proc_config(curr_layers_id, (uint32_t)curr_frame->scene_id);
		dpu_drm_layer_dacc_prop_config(curr_layers_id, (uint32_t)curr_frame->scene_id);
	}
	if (curr_frame->protected_wch_id != -1) // curr frame has drm wb layer, need config protect wch
		dpu_drm_layer_wch_mmu_proc_config((uint32_t)curr_frame->protected_wch_id, (uint32_t)curr_frame->scene_id);
	return;
}

void dpu_offline_drm_layer_clear(struct disp_frame *curr_frame)
{
	uint32_t curr_layers_id = 0;
	if (curr_frame == NULL) {
		dpu_pr_err("notify curr_frame is NULL!\n");
		return;
	}
	curr_layers_id = get_drm_layer_id_from_frame(curr_frame);
	dpu_pr_debug("notify offline_drm_layer_clear curr_layers_id=%d\n", curr_layers_id);
	if (curr_layers_id != 0) // curr frame has drm layer
		dpu_drm_layer_mmu_proc_clear(curr_layers_id, (uint32_t)curr_frame->scene_id);

	if (curr_frame->protected_wch_id != -1) // curr frame has drm wb layer, need clear protect wch
		dpu_drm_layer_wch_mmu_proc_clear((uint32_t)curr_frame->protected_wch_id, (uint32_t)curr_frame->scene_id);
	return;
}
