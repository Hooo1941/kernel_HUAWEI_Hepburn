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

#include <linux/io.h>
#include "dpu/soc_dpu_define.h"
#include "dkmd_object.h"

void dpu_comp_abnormal_dump_dacc(char __iomem *dpu_base, uint32_t scene_id)
{
	char __iomem *module_base = dpu_base + DACC_OFFSET + DMC_OFFSET;

	dpu_pr_warn("DACC:\n\n"
		"CORE_STATUS_REG=%#x\t\tDACC_CORE_MON_PC_REG=%#x\t\tDBG_DEMURA_RLST_CNT=%#x\t\tDBG_SCL_RLST_CNT_01=%#x\n"
		"DBG_SCL_RLST_CNT_23=%#x\t\tDBG_UVUP_RLST_YCNT=%#x\t\tDBG_CST_ALL=%#x\t\tDBG_FSM_MISC=%#x\n"
		"DBG_BUSY_REQ_MISC=%#x\t\tDBG_RESERVED_0=%#x\t\tDBG_RESERVED_1=%#x\n\n",
		inp32(SOC_DACC_CORE_STATUS_REG_ADDR(module_base)), inp32(SOC_DACC_CORE_MON_PC_REG_ADDR(module_base)),
		inp32(SOC_DACC_DBG_DEMURA_RLST_CNT_ADDR(module_base, scene_id)),
		inp32(SOC_DACC_DBG_SCL_RLST_CNT_01_ADDR(module_base, scene_id)),
		inp32(SOC_DACC_DBG_SCL_RLST_CNT_23_ADDR(module_base, scene_id)),
		inp32(SOC_DACC_DBG_UVUP_RLST_YCNT_ADDR(module_base, scene_id)),
		inp32(SOC_DACC_DBG_CST_ALL_ADDR(module_base, scene_id)),
		inp32(SOC_DACC_DBG_FSM_MISC_ADDR(module_base, scene_id)),
		inp32(SOC_DACC_DBG_BUSY_REQ_MISC_ADDR(module_base, scene_id)),
		inp32(SOC_DACC_DBG_RESERVED_0_ADDR(module_base, scene_id)),
		inp32(SOC_DACC_DBG_RESERVED_1_ADDR(module_base, scene_id)));
}