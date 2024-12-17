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
#include <dpu/soc_dpu_define.h>
#include <dkmd_dpu.h>
#include "cmdlist_interface.h"
#include "dpu_cmdlist.h"
#include "dpu_comp_smmu.h"

struct dpu_cmdlist_frame_info g_curr_cmdlist_frm_info;
struct dpu_cmdlist_frame_info g_last_cmdlist_frm_info;

static void dpu_cmdlist_init_ch(char __iomem *dpu_base)
{
	uint32_t i;

	if (unlikely(!dpu_base)) {
		dpu_pr_warn("dpu_base is NULL");
		return;
	}

	set_reg(DPU_CMD_TWO_SAME_FRAME_BYPASS_ADDR(dpu_base + DPU_CMDLIST_OFFSET), 0x1, 1, 0);
	for (i = 0; i < (uint32_t)DPU_SCENE_MAX; ++i) {
		outp32(DPU_CMD_CMDLIST_CH_INTE_ADDR(dpu_base + DPU_CMDLIST_OFFSET, i), 0xE4);
		set_reg(DPU_CMD_CMDLIST_CH_CTRL_ADDR(dpu_base + DPU_CMDLIST_OFFSET, i), 1, 1, 0);
	}
}

void dpu_cmdlist_present_commit(char __iomem *dpu_base, uint32_t scene_id, uint32_t cmdlist_id)
{
	dma_addr_t cmdlist_buf_addr;
	uint64_t cur_ts;

	if (unlikely(!dpu_base)) {
		dpu_pr_err("dpu_base is null");
		return;
	}

	if (scene_id >= DPU_SCENE_MAX) {
		dpu_pr_err("scene_id = %u is out of range", scene_id);
		return;
	}

	dsb(sy);

	if (g_debug_fence_timeline)
		dpu_pr_info("scene_id=%u cmdlist_id=%u", scene_id, cmdlist_id);

	cmdlist_buf_addr = dkmd_cmdlist_get_dma_addr(scene_id, cmdlist_id);
	if (cmdlist_buf_addr != 0) {
		cur_ts = (uint64_t)ktime_to_us(ktime_get());
		outp32(DPU_DM_CMDLIST_ADDR0_ADDR(dpu_base + g_dm_tlb_info[scene_id].dm_data_addr), cmdlist_buf_addr);
		dpu_pr_debug("cur_ts=%llu scene_id=%u cmdlist_id=%u vstate=0x%x",
			cur_ts, scene_id, cmdlist_id, inp32(DPU_DSI_VSTATE_ADDR(dpu_base)));
	}
}

void dpu_cmdlist_init_commit(char __iomem *dpu_base, dma_addr_t cmdlist_buf_addr)
{
	uint32_t times = 0;
	DPU_CMD_CMDLIST_CH_INTS_UNION intr_status;

	if (unlikely(!dpu_base)) {
		dpu_pr_err("dpu_base is null");
		return;
	}

	if (unlikely(cmdlist_buf_addr == 0)) {
		dpu_pr_err("cmdlist_buf_addr is null");
		return;
	}

	dsb(sy);

	dpu_cmdlist_init_ch(dpu_base);

	set_reg(DPU_CMD_CMDLIST_CH_STAAD_ADDR(dpu_base + DPU_CMDLIST_OFFSET, DPU_SCENE_INITAIL), cmdlist_buf_addr, 32, 0);

	writel(~BIT(DPU_SCENE_INITAIL), DPU_CMD_CMDLIST_ADDR_MASK_DIS_ADDR(dpu_base + DPU_CMDLIST_OFFSET));

	/* start */
	set_reg(DPU_CMD_CMDLIST_START_ADDR(dpu_base + DPU_CMDLIST_OFFSET), BIT(DPU_SCENE_INITAIL), 8, 0);

	while (true) {
		intr_status.value = inp32(DPU_CMD_CMDLIST_CH_INTS_ADDR((dpu_base + DPU_CMDLIST_OFFSET), DPU_SCENE_INITAIL));

		if (intr_status.reg.ch_taskdone_ints) {
			dpu_pr_debug("wait init finish inte for %u us", (times * 50));
			break;
		} else if (times >= 100){
			dpu_pr_err("wait init finish timeout");
			break;
		} else {
			udelay(50);
			times++;
		}
	}
}

static const char *dpu_cmdlist_sync_get_name(struct dkmd_timeline_listener *listener)
{
	return "cmdlist_sync";
}

static bool dpu_cmdlist_sync_is_signaled(struct dkmd_timeline_listener *listener, uint64_t tl_val)
{
	bool ret = tl_val > listener->pt_value;

	if (g_debug_fence_timeline)
		dpu_pr_info("signal=%d, tl_val=%llu, listener_value=%llu", ret, tl_val, listener->pt_value);

	return ret;
}

static int32_t dpu_cmdlist_sync_handle_signal(struct dkmd_timeline_listener *listener)
{
	struct dpu_cmdlist_frame_info *frame_info = NULL;

	dpu_check_and_return(!listener->listener_data, -1, err, "listener_data is null!");
	frame_info = (struct dpu_cmdlist_frame_info *)listener->listener_data;

	if (g_debug_fence_timeline)
		dpu_pr_info("frame_info scene_id=%d, cmdlist_id=%d, frame_index=%d",
			frame_info->scene_id, frame_info->cmdlist_id, frame_info->frame_index);

	return 0;
}

static void dpu_cmdlist_sync_release(struct dkmd_timeline_listener *listener)
{
	struct dpu_cmdlist_frame_info *frame_info = NULL;

	dpu_check_and_no_retval(!listener->listener_data, err, "listener_data is null!");
	frame_info = (struct dpu_cmdlist_frame_info *)listener->listener_data;

	if (g_debug_fence_timeline)
		dpu_pr_info("frame_info scene_id=%d, cmdlist_id=%d, frame_index=%d",
			frame_info->scene_id, frame_info->cmdlist_id, frame_info->frame_index);

	dpu_comp_smmu_tlb_flush(frame_info->scene_id, frame_info->frame_index);

	dkmd_cmdlist_release_locked(frame_info->scene_id, frame_info->cmdlist_id);

	kfree(listener->listener_data);
	listener->listener_data = NULL;
}

static struct dkmd_timeline_listener_ops g_cmdlist_buf_listener_ops = {
	.get_listener_name = dpu_cmdlist_sync_get_name,
	.is_signaled = dpu_cmdlist_sync_is_signaled,
	.handle_signal = dpu_cmdlist_sync_handle_signal,
	.release = dpu_cmdlist_sync_release,
};

int32_t dpu_cmdlist_sync_lock(struct dkmd_timeline *timeline, struct disp_frame *frame)
{
	struct dkmd_timeline_listener *listener = NULL;
	struct dpu_cmdlist_frame_info *frame_info = NULL;

	dpu_assert(!timeline);
	dpu_assert(!frame);

	frame_info = (struct dpu_cmdlist_frame_info *)kmalloc(sizeof(*frame_info), GFP_KERNEL);
	dpu_check_and_return(!frame_info, -1, err, "frame_info alloc failed!");
	frame_info->scene_id = (uint32_t)(frame->scene_id);
	frame_info->cmdlist_id = frame->cmdlist_id;
	frame_info->frame_index = frame->frame_index;

	g_last_cmdlist_frm_info = g_curr_cmdlist_frm_info;
	g_curr_cmdlist_frm_info = *frame_info;

	/* video mode need wait two frame */
	listener = dkmd_timeline_alloc_listener(&g_cmdlist_buf_listener_ops, frame_info, frame->present_fence_pt + 1);
	if (!listener) {
		dpu_pr_err("alloc cmdlist buf listener fail cmdlist_id: %u", frame->cmdlist_id);
		kfree(frame_info);
		frame_info = NULL;
		return -1;
	}

	if (g_debug_fence_timeline)
		dpu_pr_info("frame_info scene_id=%d, cmdlist_id=%d, frame_index=%d pt_value=%llu",
			frame_info->scene_id, frame_info->cmdlist_id, frame_info->frame_index, frame->present_fence_pt + 1);

	dkmd_timeline_add_listener(timeline, listener);

	return 0;
}
