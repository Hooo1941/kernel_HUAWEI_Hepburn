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
#include <dpu/dpu_wlt.h>
#include "dpu_wlt.h"
#include "dkmd_wlt.h"
#include "cmdlist_interface.h"
#include "cmdlist_node.h"
#include "dkmd_object.h"

static struct wlt_cmdlist_info g_wlt_cmdlist_info;
static bool wlt_cmdlist_is_inited = false;

void dpu_wlt_set_slice0_addr(char __iomem *dpu_base)
{
	struct dpu_dacc_wlt wlt_info;
	char __iomem *dmc_base = NULL;
	uint32_t *cmd_item_base = NULL;
	struct cmd_item cmd_item0;
	struct cmd_item cmd_item1;

	if (wlt_cmdlist_is_inited == false)
		return;

	dmc_base = dpu_base + DACC_OFFSET + DMC_OFFSET;
	set_reg(SOC_DACC_WLT_SLICE0_DDR_ADDR0_ADDR(dmc_base),
		cmdlist_get_phy_addr(g_wlt_cmdlist_info.scene_id, g_wlt_cmdlist_info.slice0_cmdlist_id), 32, 0);
	set_reg(SOC_DACC_WLT_SLICE1_DDR_ADDR0_ADDR(dmc_base),
		cmdlist_get_phy_addr(g_wlt_cmdlist_info.scene_id, g_wlt_cmdlist_info.slice1_cmdlist_id), 32, 0);
	set_reg(SOC_DACC_WLT_SLICE2_DDR_ADDR0_ADDR(dmc_base),
		cmdlist_get_phy_addr(g_wlt_cmdlist_info.scene_id, g_wlt_cmdlist_info.slice2_cmdlist_id), 32, 0);
	set_reg(SOC_DACC_WLT_SLICE3_DDR_ADDR0_ADDR(dmc_base),
		cmdlist_get_phy_addr(g_wlt_cmdlist_info.scene_id, g_wlt_cmdlist_info.slice3_cmdlist_id), 32, 0);


	cmd_item_base = (uint32_t *)cmdlist_get_payload_addr(g_wlt_cmdlist_info.scene_id,
		g_wlt_cmdlist_info.slice0_cmdlist_id);

	if (unlikely(!cmd_item_base)) {
		dpu_pr_err("invalid cmd_item_base!");
		return;
	}

	cmd_item0 = *(struct cmd_item *)(cmd_item_base);
	cmd_item1 = *((struct cmd_item *)cmd_item_base + 1);

	wlt_info.slice0_addr0_h.value = cmd_item0.cmd_item.cmd_mode_0.data0;
	wlt_info.slice0_addr0_l.value = cmd_item0.cmd_item.cmd_mode_0.data1;
	wlt_info.slice0_addr1_l.value = cmd_item0.cmd_item.cmd_mode_0.data2;
	wlt_info.slice0_addr2_l.value = cmd_item1.cmd_item.cmd_mode_0.data0;
	wlt_info.slice0_addr3_l.value = cmd_item1.cmd_item.cmd_mode_0.data1;

	set_reg(SOC_DACC_WLT_SLICE0_ADDR0_H_ADDR(dmc_base), wlt_info.slice0_addr0_h.value, 32, 0);
	set_reg(SOC_DACC_WLT_SLICE0_ADDR0_L_ADDR(dmc_base), wlt_info.slice0_addr0_l.value, 32, 0);
	set_reg(SOC_DACC_WLT_SLICE0_ADDR1_L_ADDR(dmc_base), wlt_info.slice0_addr1_l.value, 32, 0);
	set_reg(SOC_DACC_WLT_SLICE0_ADDR2_L_ADDR(dmc_base), wlt_info.slice0_addr2_l.value, 32, 0);
	set_reg(SOC_DACC_WLT_SLICE0_ADDR3_L_ADDR(dmc_base), wlt_info.slice0_addr3_l.value, 32, 0);
}

void dkmd_wlt_set_cmdlist(struct wlt_cmdlist_info *cmdlist_info)
{
	g_wlt_cmdlist_info.scene_id = cmdlist_info->scene_id;
	g_wlt_cmdlist_info.slice0_cmdlist_id = cmdlist_info->slice0_cmdlist_id;
	g_wlt_cmdlist_info.slice1_cmdlist_id = cmdlist_info->slice1_cmdlist_id;
	g_wlt_cmdlist_info.slice2_cmdlist_id = cmdlist_info->slice2_cmdlist_id;
	g_wlt_cmdlist_info.slice3_cmdlist_id = cmdlist_info->slice3_cmdlist_id;

	wlt_cmdlist_is_inited = true;
}
