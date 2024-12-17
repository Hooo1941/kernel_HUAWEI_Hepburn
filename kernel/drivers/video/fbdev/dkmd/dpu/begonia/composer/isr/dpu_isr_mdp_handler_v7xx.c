/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
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
#include <linux/of_irq.h>
#include <linux/kernel.h>
#include "dkmd_listener.h"
#include "dpu_comp_mgr.h"
#include "dpu_isr.h"

void dpu_mdp_dacc_dfr_isr_handler(char __iomem *dpu_base, struct dkmd_isr *isr_ctrl)
{
	/* dfr isr type, use scene 6 0x1c8 register */
	uint32_t isr_type = inp32(DPU_DM_LAYER_RSV0_ADDR(
		dpu_base + g_dm_tlb_info[DPU_SCENE_OFFLINE_2].dm_data_addr, 0));
	switch (isr_type) {
	case DPU_DACC_SECONDARY_ISR_VALID_VSYNC:
		dpu_pr_debug("dacc valid vsync, cur_time = %llu ms", ktime_to_ms(ktime_get()));
		dkmd_isr_notify_listener(isr_ctrl, NOTIFY_BOTH_VSYNC_TIMELINE);

		/* fps count only for LTPO screen */
		dkmd_isr_notify_listener(isr_ctrl, NOTIFY_REFRESH);
		break;
	case DPU_DACC_SECONDARY_ISR_COUNT_VSYNC:
		dpu_pr_debug("dacc count vsync, cur_time = %llu ms", ktime_to_ms(ktime_get()));
		dkmd_isr_notify_listener(isr_ctrl, NOTIFY_BOTH_VSYNC_TIMELINE);
		break;
	case DPU_DACC_SECONDARY_ISR_SELF_REFRESH:
		dpu_pr_info("refresh for frm rate dimming, cur_time = %llu ms", ktime_to_ms(ktime_get()));
		/* fps count only for LTPO screen */
		dkmd_isr_notify_listener(isr_ctrl, NOTIFY_REFRESH);
		break;
	case DPU_DACC_SECONDARY_ISR_DIMMING_DONE:
		dpu_pr_info("frm rate dimming done");
		dkmd_isr_notify_listener(isr_ctrl, NOTIFY_DIMMING_DONE);
		break;
	case DACC_SECONDARY_ISR_VSYNC_AND_SELF_REFRESH:
		dpu_pr_info("count and refresh cur_time = %llu ms", ktime_to_ms(ktime_get()));
		dkmd_isr_notify_listener(isr_ctrl, NOTIFY_BOTH_VSYNC_TIMELINE);

		/* fps count only for LTPO screen */
		dkmd_isr_notify_listener(isr_ctrl, NOTIFY_REFRESH);
		break;
	case DACC_SECONDARY_ISR_COUNT_VSYNC_AND_DIMMING_DONE:
		dpu_pr_info("frm rate dimming done");
		dkmd_isr_notify_listener(isr_ctrl, NOTIFY_DIMMING_DONE);
		dkmd_isr_notify_listener(isr_ctrl, NOTIFY_BOTH_VSYNC_TIMELINE);
		break;
	case DACC_SECONDARY_ISR_TE:
		dpu_pr_info("dfr and ppc share same isr please ignore, otherwise acpu secondary isr respond slow");
		break;
	default:
		dpu_pr_warn("invalid dfr isr type %u", isr_type);
		break;
	}
}

void dpu_mdp_dacc_ppc_isr_handler(char __iomem *dpu_base, struct dkmd_isr *isr_ctrl)
{
	uint32_t isr_type = inp32(DPU_DM_LAYER_DMA_SEL_ADDR(
		dpu_base + g_dm_tlb_info[DPU_SCENE_OFFLINE_2].dm_data_addr, 1));

	if ((isr_type & DACC_SECONDARY_ISR_PPC_DONE) == DACC_SECONDARY_ISR_PPC_DONE) {
		dpu_pr_info("ppc done");
		dkmd_isr_notify_listener(isr_ctrl, NOTIFY_PPC_DONE);
	}
}
