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

#include <linux/dma-buf.h>
#include <linux/delay.h>
#include <linux/syscalls.h>
#include <securec.h>
#include "dpu_comp_mgr.h"
#include "dpu_comp_offline.h"
#include "dkmd_acquire_fence.h"
#include "dpu_comp_smmu.h"
#include "dpu_dacc.h"
#include "cmdlist_interface.h"
#include "dpu_comp_abnormal_handle.h"
#include "dvfs.h"
#include "dkmd_listener.h"
#include "dpu_comp_secure.h"
#include "config/dpu_offline_config_utils.h"
#include "dpu_debug_dump.h"

static void composer_offline_postprocess(struct comp_offline_present *present, bool timeout)
{
	uint32_t i;
	struct dkmd_dma_buf *buffer = NULL;
	struct disp_frame *frame = &present->frame.in_frame;
	struct composer *comp = &present->dpu_comp->comp;

	dpu_dvfs_reset_comp_vote(comp->index);
	dpu_comp_smmu_offline_tlb_flush((uint32_t)frame->scene_id, frame->frame_index);
	dkmd_cmdlist_release_locked((uint32_t)frame->scene_id, frame->cmdlist_id);
	if (frame->layer_count > DISP_LAYER_MAX_COUNT) {
		dpu_pr_err("layer_count size out of range!");
		return;
	}
	for (i = 0; i < frame->layer_count; ++i) {
		buffer = &(present->frame.layer_dma_buf[i]);
		if ((buffer->share_fd > 0) && !IS_ERR_OR_NULL(buffer->buf_handle)) {
			dpu_pr_debug("release %s", buffer->name);
			dma_buf_put(buffer->buf_handle);
			buffer->share_fd = -1;
			buffer->buf_handle = NULL;
		}
	}
	present->wb_done = 0;
	present->wb_has_presented = 0;

	if (unlikely(timeout)) {
		up(&comp->blank_sem);
		composer_manager_power_down(present->dpu_comp);
		composer_manager_reset_hardware(present->dpu_comp);
		composer_manager_power_up(present->dpu_comp);
		down(&comp->blank_sem);
		present->dpu_comp->hdr_info.block_hdr_mean = 0;
	}

	comp->off(comp, COMPOSER_OFF_MODE_BLANK);
}

static void composer_offline_check_wb_finish(struct comp_offline_present *present)
{
	int32_t ret = 0;
	uint64_t frame_end_tv;
	uint32_t timeout_interval = ASIC_EVENT_TIMEOUT_MS;
	char __iomem *dpu_base = NULL;
	int times = 0;
	/* add for config drm layer */
	struct disp_frame *frame = &present->frame.in_frame;
	struct dpu_composer *dpu_comp = present->dpu_comp;

	dpu_trace_ts_begin(&frame_end_tv);
	dpu_base = dpu_comp->comp_mgr->dpu_base;

	if (unlikely(dpu_comp->comp.index >= DEVICE_COMP_MAX_COUNT)) {
		dpu_pr_err("dpu_comp->comp.index=%u", dpu_comp->comp.index);
		return;
	}

	while (true) {
		ret = (int32_t)wait_event_interruptible_timeout(present->wb_wq,
			(present->wb_done == 1), /* wb type status is true */
			(long)msecs_to_jiffies(timeout_interval));
		if ((ret == -ERESTARTSYS) && (times++ < 50))
			mdelay(10);
		else
			break;
	}

	dpu_offline_drm_layer_clear(frame);

	if (ret <= 0) {
		dpu_pr_warn("offline compose timeout!!");
		if (g_debug_underflow_dump_enable) {
			dpu_comp_abnormal_dump_reg_dm(dpu_comp->comp_mgr->dpu_base, (uint32_t)present->frame.in_frame.scene_id);
			dpu_parse_layer_info(dpu_comp->comp_mgr->dpu_base, (uint32_t)present->frame.in_frame.scene_id);
		}

		if (g_debug_dpu_clear_enable) {
			dpu_pr_info("power_status=0x%llx comp.index=%d power=%u",
				dpu_comp->comp_mgr->power_status.status, dpu_comp->comp.index,
				dpu_comp->comp_mgr->power_status.refcount.value[dpu_comp->comp.index]);
			if (dpu_comp->comp_mgr->power_status.status == 0) {
				dpu_pr_info("already power off, do not need handle underflow clear!");
				return;
			}

			ret = (int32_t)dpu_dacc_handle_clear(dpu_base, (uint32_t)present->frame.in_frame.scene_id);
			if (ret != 0) {
				dpu_pr_warn("offline clear failed, reset hardware");
				composer_offline_postprocess(present, true);
				dpu_trace_ts_end(&frame_end_tv, "offline compose failed");
				return;
			}
		}
	} else {
		present->offline_succ = 1;
	}

	composer_offline_postprocess(present, false);
	dpu_trace_ts_end(&frame_end_tv, "offline compose present");
}

static void composer_offline_preprocess(struct comp_offline_present *present)
{
	uint32_t i;
	struct disp_layer *layer = NULL;
	struct disp_frame *frame = &present->frame.in_frame;
	int ret;

	dpu_dvfs_inter_frame_vote(present->dpu_comp->comp.index, &frame->dvfs_info);

	/* wait layer acquired fence and lock dma buf */
	if (frame->layer_count > DISP_LAYER_MAX_COUNT) {
		dpu_pr_err("layer_count is out of range\n");
		return;
	}
	for (i = 0; i < frame->layer_count; ++i) {
		layer = &frame->layer[i];
		if (layer->acquired_fence > 0) {
			dkmd_acquire_fence_wait_fd(layer->acquired_fence, ACQUIRE_FENCE_TIMEOUT_MSEC);
			layer->acquired_fence = -1;
		}
		if (layer->share_fd > 0) {
			present->frame.layer_dma_buf[i].share_fd = layer->share_fd;
			present->frame.layer_dma_buf[i].buf_handle = dma_buf_get(layer->share_fd);
			ret = snprintf_s(present->frame.layer_dma_buf[i].name, DKMD_SYNC_NAME_SIZE, DKMD_SYNC_NAME_SIZE - 1, \
								"buf_share_fd_%d", layer->share_fd);
			if (ret < 0)
				dpu_pr_err("format string failed, truncation occurs");
		}
	}
}

static int32_t composer_offline_overlay(struct dpu_composer *dpu_comp, struct disp_frame *frame)
{
	int32_t ret = 0;
	uint32_t scene_id = (uint32_t)frame->scene_id;
	struct composer_scene *scene = NULL;
	struct comp_offline_present *present = (struct comp_offline_present *)dpu_comp->present_data;

	if (unlikely(scene_id > DPU_SCENE_OFFLINE_2 || scene_id < DPU_SCENE_ONLINE_3)) {
		dpu_pr_err("invalid scene_id=%u", scene_id);
		return -1;
	}

	scene = dpu_comp->comp_mgr->scene[scene_id];
	if (unlikely(!scene)) {
		dpu_pr_err("unsupport scene_id=%u", scene_id);
		return -1;
	}
	frame->out_present_fence = -1;
	present->frame.in_frame = *frame;
	present->wb_has_presented = 1;
	composer_offline_preprocess(present);

	/* commit to hardware, but sometimes hardware is handling exceptions,
	 */
	if (likely(scene->present != NULL)) {
		/* cpu config drm layer */
		dpu_offline_drm_layer_config(frame);
		scene->present(scene, frame->cmdlist_id);
	}

	composer_offline_check_wb_finish(present);
	if (present->offline_succ)
		present->offline_succ = 0;
	else
		ret = -1; // offline fail

	return ret;
}

static int32_t dpu_offline_isr_notify(struct notifier_block *self, unsigned long action, void *data)
{
	struct dkmd_listener_data *listener_data = (struct dkmd_listener_data *)data;
	struct dpu_composer *dpu_comp = (struct dpu_composer *)(listener_data->data);
	struct comp_offline_present *present = (struct comp_offline_present *)dpu_comp->present_data;
	char __iomem *hdr_base = NULL;

	hdr_base  = dpu_comp->comp_mgr->dpu_base + DPU_HDR_OFFSET;
	dpu_comp->hdr_info.block_hdr_mean += inp32(DPU_HDR_MEAN_STT_ADDR(hdr_base));

	if ((action & WCH_FRM_END_INTS) == WCH_FRM_END_INTS) {
		if (present->wb_has_presented == 1) {
			present->wb_done = 1;
			wake_up_interruptible_all(&present->wb_wq);
		}
		composer_set_hdr_mean(&dpu_comp->hdr_info, dpu_comp->hdr_info.block_hdr_mean);
	}

	if (offline_is_support_wch3_writeback() && (action & WCH3_FRM_END_INTS) == WCH3_FRM_END_INTS) {
		if (present->wb_has_presented == 1) {
			present->wb_done = 1;
			wake_up_interruptible_all(&present->wb_wq);
		}
		composer_set_hdr_mean(&dpu_comp->hdr_info, dpu_comp->hdr_info.block_hdr_mean);
	}

	return 0;
}

static struct notifier_block offline_isr_notifier = {
	.notifier_call = dpu_offline_isr_notify,
};

void composer_offline_setup(struct dpu_composer *dpu_comp, struct comp_offline_present *present)
{
	present->dpu_comp = dpu_comp;

	sema_init(&(present->wb_sem), 1);
	init_waitqueue_head(&(present->wb_wq));

	pipeline_next_ops_handle(dpu_comp->conn_info->conn_device,
		dpu_comp->conn_info, SETUP_ISR, (void *)&dpu_comp->isr_ctrl);
	dkmd_isr_setup(&dpu_comp->isr_ctrl);
	dkmd_isr_request(&dpu_comp->isr_ctrl);
	list_add_tail(&dpu_comp->isr_ctrl.list_node, &dpu_comp->comp_mgr->isr_list);
	dkmd_isr_register_listener(&dpu_comp->isr_ctrl, &offline_isr_notifier, WCH_FRM_END_INTS, dpu_comp);
	dkmd_isr_register_listener(&dpu_comp->isr_ctrl, &offline_isr_notifier, WCH_BLK_END_INTS, dpu_comp);
	if (offline_is_support_wch3_writeback())
		dkmd_isr_register_listener(&dpu_comp->isr_ctrl, &offline_isr_notifier, WCH3_FRM_END_INTS, dpu_comp);

	dpu_comp_active_vsync(dpu_comp);

	dpu_comp->overlay = composer_offline_overlay;
	dpu_comp->create_fence = NULL;
	dpu_comp->release_fence = NULL;
}

void composer_offline_release(struct dpu_composer *dpu_comp, struct comp_offline_present *present)
{
	dpu_comp->isr_ctrl.handle_func(&dpu_comp->isr_ctrl, DKMD_ISR_RELEASE);
	list_del(&dpu_comp->isr_ctrl.list_node);
	dpu_comp_deactive_vsync(dpu_comp);
}
