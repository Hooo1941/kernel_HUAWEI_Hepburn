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
#include <dpu/dpu_dm.h>
#include <soc_media1_crg_interface.h>
#include <linux/interrupt.h>
#include "dkmd_listener.h"
#include "dpu_comp_mgr.h"
#include "dpu_comp_vactive.h"
#include "dpu_comp_abnormal_handle.h"
#include "dpu_comp_smmu.h"
#include "dpu_opr_config.h"
#include "dpu_dacc.h"
#include "dpu_config_utils.h"
#include "cmdlist_interface.h"
#include "dksm_debug.h"
#include "dpu_cmdlist.h"
#include "dpu_config_utils.h"

// range example:[DM_SCENE_INFO_OFFSET, DM_UVUP_OFFSET)
struct dpu_dsm_dm_dump dsm_dm_dump_range[] = {
	{DM_SCENE_INFO_OFFSET, DM_UVUP_OFFSET},
	{DM_SROT0_OFFSET, DM_HEMCD_OFFSET},
	{DM_SCL0_OFFSET, DM_DDIC_OFFSET},
	{DM_DSC_OFFSET, DM_WCH_OFFSET},
};

uint32_t dpu_comp_get_dsm_dm_dump_cnt(void)
{
	return ARRAY_SIZE(dsm_dm_dump_range);
}

void dpu_comp_abnormal_dump_lbuf(char __iomem *dpu_base)
{
	char __iomem *module_base = dpu_base + DPU_LBUF_OFFSET;

	dpu_pr_warn("LBUF:\n"
		"\tLBUF_DCPT_STATE=%#x\t\tLBUF_DCPT_DMA_STATE=%#x\t\tLBUF_CTL_STATE=%#08x\t\tLBUF_RF_STATE=%#x\n"
		"\tLBUF_PRE_USE_STATE=%#x\t\tLBUF_PPST_USE_STATE=%#x\t\tLBUF_PRE_CHECK_STATE=%#x\t\tLBUF_PPST_CHECK_STATE=%#x\n"
		"\tLBUF_SCN_STATE=%#08x\t\tLBUF_SCN_DMA_STATE=%#x\t\tLBUF_RUN_STATE=%#08x\t\tLBUF_RUN_MST_STATE=%#x\n"
		"\tLBUF_RUN_CTL_STATE=%#x\t\tLBUF_RUN_WR_STATE=%#x\t\tLBUF_RUN_RD_STATE=%#x\t\tLBUF_RUN_RD2_STATE=%#x\n"
		"\tLBUF_PART0_RMST_PASS=%#x\tLBUF_PART0_WMST_PASS=%#x\tLBUF_PART0_IDX_DATA=%#x\tLBUF_PART0_RUN_DAT=%#x\n"
		"\tLBUF_PART1_RMST_PASS=%#x\tLBUF_PART1_WMST_PASS=%#x\tLBUF_PART1_IDX_DATA=%#x\tLBUF_PART1_RUN_DATA=%#x\n"
		"\tLBUF_PART2_RMST_PASS=%#x\tLBUF_PART2_WMST_PASS=%#x\tLBUF_PART2_IDX_DATA=%#x\tLBUF_PART2_RUN_DATA=%#x\n",
		inp32(DPU_LBUF_DCPT_STATE_ADDR(module_base)),
		inp32(DPU_LBUF_DCPT_DMA_STATE_ADDR(module_base)),
		inp32(DPU_LBUF_CTL_STATE_ADDR(module_base)),
		inp32(DPU_LBUF_RF_STATE_ADDR(module_base)),
		inp32(DPU_LBUF_PRE_USE_STATE_ADDR(module_base)),
		inp32(DPU_LBUF_PPST_USE_STATE_ADDR(module_base)),
		inp32(DPU_LBUF_PRE_CHECK_STATE_ADDR(module_base)),
		inp32(DPU_LBUF_PPST_CHECK_STATE_ADDR(module_base)),
		inp32(DPU_LBUF_SCN_STATE_ADDR(module_base)),
		inp32(DPU_LBUF_SCN_DMA_STATE_ADDR(module_base)),
		inp32(DPU_LBUF_RUN_STATE_ADDR(module_base)),
		inp32(DPU_LBUF_RUN_MST_STATE_ADDR(module_base)),
		inp32(DPU_LBUF_RUN_CTL_STATE_ADDR(module_base)),
		inp32(DPU_LBUF_RUN_WR_STATE_ADDR(module_base)),
		inp32(DPU_LBUF_RUN_RD_STATE_ADDR(module_base)),
		inp32(DPU_LBUF_RUN_RD2_STATE_ADDR(module_base)),
		inp32(DPU_LBUF_PART0_RMST_PASS_ADDR(module_base)),
		inp32(DPU_LBUF_PART0_WMST_PASS_ADDR(module_base)),
		inp32(DPU_LBUF_PART0_IDX_DATA_ADDR(module_base)),
		inp32(DPU_LBUF_PART0_RUN_DAT_ADDR(module_base)),
		inp32(DPU_LBUF_PART1_RMST_PASS_ADDR(module_base)),
		inp32(DPU_LBUF_PART1_WMST_PASS_ADDR(module_base)),
		inp32(DPU_LBUF_PART1_IDX_DATA_ADDR(module_base)),
		inp32(DPU_LBUF_PART1_RUN_DATA_ADDR(module_base)),
		inp32(DPU_LBUF_PART2_RMST_PASS_ADDR(module_base)),
		inp32(DPU_LBUF_PART2_WMST_PASS_ADDR(module_base)),
		inp32(DPU_LBUF_PART2_IDX_DATA_ADDR(module_base)),
		inp32(DPU_LBUF_PART2_RUN_DATA_ADDR(module_base)));
}

