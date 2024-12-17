/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
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

#include <dpu/soc_dpu_define.h>
#include <dkmd_cmdlist.h>
#include "dpu_comp_mgr.h"
#include <dpu/dpu_ld.h>
#include "dpu_ld_init.h"
#include "dpu_comp_init.h"
#include "dpu_cmdlist.h"
#include "dkmd_object.h"
#include "dpu_config_utils.h"
#include "cmdlist_interface.h"

static void dpu_lut_write_coefs(uint32_t *cmd_item_base, uint32_t *lut_tap, uint32_t tap_size)
{
	uint32_t i;

	dpu_check_and_no_retval(!cmd_item_base, err, "item base is null!");
	for (i = 0; i < (tap_size / sizeof(uint32_t)); i++)
		*(cmd_item_base + i) = lut_tap[i];
}

static int32_t dpu_lut_cmdlist_config(struct dpu_composer *dpu_comp,
	struct lut_table *lut_tlb)
{
	/* 1. alloc cmdlist node */
	uint32_t cmdlist_id = cmdlist_create_user_client(DPU_SCENE_INITAIL, DATA_TRANSPORT_TYPE,
		lut_tlb->offset, lut_tlb->lut_size);

	/* 2. config cmdlist node */
	uint32_t *payload_addr = (uint32_t *)cmdlist_get_payload_addr(DPU_SCENE_INITAIL, cmdlist_id);
	if (unlikely(!payload_addr)) {
		dpu_pr_err("payload_addr is null");
		return -1;
	}
	dpu_lut_write_coefs(payload_addr, lut_tlb->lut_value, lut_tlb->lut_size);

	/* 3. add comlist node to init list header tail */
	cmdlist_flush_client(DPU_SCENE_INITAIL, cmdlist_id);
	cmdlist_append_client(DPU_SCENE_INITAIL, dpu_comp->init_scene_cmdlist, cmdlist_id);
	return 0;
}

static int32_t dpu_lut_config(struct dpu_composer *dpu_comp, struct lut_table table[], uint32_t len)
{
	uint32_t i;

	for (i = 0; i < len; i++)
		if (dpu_lut_cmdlist_config(dpu_comp, &table[i]))
			return -1;
	return 0;
}

int32_t dpu_ld_init(struct dpu_composer *dpu_comp)
{
	set_reg(DPU_DBCU_MIF_CTRL_WCH3_ADDR(dpu_comp->comp_mgr->dpu_base + DPU_DBCU_OFFSET), 0, 1, 0);
	if (dpu_lut_config(dpu_comp, g_ld_lut_table, ARRAY_SIZE(g_ld_lut_table)))
		return -1;
	return 0;
}