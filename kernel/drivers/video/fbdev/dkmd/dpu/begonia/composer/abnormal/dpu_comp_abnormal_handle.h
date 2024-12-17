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

#ifndef COMP_ABNORMAL_HANDLE_H
#define COMP_ABNORMAL_HANDLE_H

#include <linux/types.h>
#include "dkmd_isr.h"

#define INPUT_LAYER_OFFSET          0x80
#define INPUT_LAYER_CNT             12
#define DM_LAYER_HEIGHT_OFFSET      0x1A0
#define DM_SCENE_INFO_OFFSET      0x50
#define DM_SROT0_OFFSET      0xA0
#define DM_SCL0_OFFSET      0xF0
#define DM_DPP_OFFSET      0x140
#define DM_DSC_OFFSET      0x170
#define DM_ITF_OFFSET      0x190

#define DM_UVUP_OFFSET      0x90
#define DM_HEMCD_OFFSET      0xE0
#define DM_SCL4_OFFSET      0x130
#define DM_DDIC_OFFSET      0x150
#define DM_WCH_OFFSET      0x180
#define MAX_DUMP_LAYER_CNT 8
#define VALID_DUMP_LAYER_INFO 0x40
struct dpu_composer;

enum {
	ABNORMAL_UNDERFLOW = BIT(0),
	ABNORMAL_WAIT_VACTIVE_TIMEOUT = BIT(1),
	ABNORMAL_CHECK_LDI_FAILED = BIT(2),
};

struct dpu_dsm_dm_dump {
	uint32_t start_addr;
	uint32_t end_addr;
};

extern struct dpu_dsm_dm_dump dsm_dm_dump_range[];
void dpu_comp_abnormal_handle_init(struct dkmd_isr *isr, struct dpu_composer *dpu_comp, uint32_t listening_bit);
void dpu_comp_abnormal_handle_deinit(struct dkmd_isr *isr, uint32_t listening_bit);
void dpu_comp_abnormal_debug_dump(struct dpu_composer *dpu_comp, uint32_t scene_id);
void dpu_comp_abnormal_dump_lbuf(char __iomem *dpu_base);
void dpu_comp_abnormal_dump_dacc(char __iomem *dpu_base, uint32_t scene_id);
void dpu_comp_abnormal_dump_reg_dm(char __iomem *dpu_base, uint32_t scene_id);
uint32_t dpu_comp_get_dsm_dm_dump_cnt(void);
void dpu_comp_abnormal_dvfs(uint32_t comp_index);
#endif