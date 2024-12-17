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
#include "smmu/dpu_comp_smmu.h"
#include "config/dpu_comp_config_utils.h"
#include "cmdlist_interface.h"

void dpu_comp_smmu_ch_set_reg(uint32_t reg_cmdlist_id, uint32_t scene_id, uint32_t frame_index)
{
	uint32_t i = 0;
	soc_dss_mmu_id_attr target;
	struct swid_info *info = NULL;

	if (dpu_is_smmu_bypass()) {
		dpu_pr_debug("dpu_is_smmu_bypass");
		return;
	}

	for (i = 0; i < ARRAY_SIZE(g_sdma_swid_tlb_info); i++) {
		if (scene_id == g_sdma_swid_tlb_info[i].scene_id ) {
			info = &g_sdma_swid_tlb_info[i];
			break;
		}
	}

	if (info == NULL) {
		dpu_pr_err("invalid scene_id=%u", scene_id);
		return;
	}

	target.value = 0;
	target.reg.ch_sid = dpu_get_ch_sid();
	target.reg.ch_ssidv = 1;
	target.reg.ch_ssid = dpu_comp_get_ssid(frame_index) + scene_id * DPU_SSID_MAX_NUM;
	for (i = info->swid_reg_offset_start; i <= info->swid_reg_offset_end; i++) {
		dkmd_set_reg(scene_id, reg_cmdlist_id, DPU_DBCU_MMU_ID_ATTR_NS_0_ADDR(DPU_DBCU0_OFFSET) + 0x4 * i, target.value);
		dpu_pr_debug("offset[%#x] scene_id=%u config mmu_id=0x%x", 0x4 * i, scene_id, target.value);
	}

	dkmd_set_reg(scene_id, reg_cmdlist_id, DPU_DBCU_AIF_CMD_RELOAD_ADDR(DPU_DBCU_OFFSET), 1);
}