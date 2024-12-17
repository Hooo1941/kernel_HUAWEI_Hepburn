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

#ifndef DPU_PANEL_PARTIAL_CTL_H
#define DPU_PANEL_PARTIAL_CTL_H

#include "dkmd_lcd_interface.h"
#include "dpu_comp_mgr.h"

#define DCS_CMD_MAX_CNT_FOR_CMDLIST 15
#define DCS_CMDLIST_PACKET_NUM 7

enum ppc_first_part_config_id_idx {
	PPC_CONFIG_ID_F_MODE_1ST_PART_IDX = 0,
	PPC_CONFIG_ID_F_MODE_2ND_PART_IDX,
	PPC_CONFIG_ID_F_MODE_3RD_PART_IDX,
	PPC_CONFIG_ID_F_MODE_4TH_PART_IDX,
	PPC_CONFIG_ID_F_MODE_5TH_PART_IDX,
	PPC_CONFIG_ID_F_MODE_6TH_PART_IDX,
	PPC_CONFIG_ID_F_MODE_7TH_PART_IDX,
	PPC_CONFIG_ID_M_MODE_1ST_PART_IDX,
	PPC_CONFIG_ID_M_MODE_2ND_PART_IDX,
	PPC_CONFIG_ID_M_MODE_3RD_PART_IDX,
	PPC_CONFIG_ID_M_MODE_4TH_PART_IDX,
	PPC_CONFIG_ID_M_MODE_5TH_PART_IDX,
	PPC_CONFIG_ID_M_MODE_6TH_PART_IDX,
	PPC_CONFIG_ID_M_MODE_7TH_PART_IDX,
	PPC_CONFIG_ID_G_MODE_1ST_PART_IDX,
	PPC_CONFIG_ID_G_MODE_2ND_PART_IDX,
	PPC_CONFIG_ID_G_MODE_3RD_PART_IDX,
	PPC_CONFIG_ID_G_MODE_4TH_PART_IDX,
	PPC_CONFIG_ID_G_MODE_5TH_PART_IDX,
	PPC_CONFIG_ID_G_MODE_6TH_PART_IDX,
	PPC_CONFIG_ID_G_MODE_7TH_PART_IDX,
	PPC_CONFIG_ID_FIRST_PART_MAX_IDX = 21,
	PPC_CONFIG_ID_F_MODE_SECOND_PART_IDX = 21,
	PPC_CONFIG_ID_M_MODE_SECOND_PART_IDX = 22,
	PPC_CONFIG_ID_G_MODE_SECOND_PART_IDX = 23,
	PPC_CONFIG_ID_SECOND_PART_MAX_IDX = 24,
	PPC_CONFIG_ID_TOTAL_CMDLIST_CNT = 24,
};

enum ppc_dsi_idx {
	PPC_1ST_DSI_IDX = 0,
	PPC_2ND_DSI_IDX = 1
};

struct panel_partial_ctrl {
	uint32_t assist_scene_id;
	uint32_t cmdlist_scene_id;
	uint32_t header_cmdlist_ids[PPC_CONFIG_ID_TOTAL_CMDLIST_CNT][PPC_CONFIG_ID_DSI_CNT];
	uint32_t reg_cmdlist_ids[PPC_CONFIG_ID_TOTAL_CMDLIST_CNT][PPC_CONFIG_ID_DSI_CNT];
	uint32_t first_part_cmdlist_cnt[PPC_CONFIG_ID_CNT];
	dma_addr_t cmdlist_phy_addr[PPC_CONFIG_ID_TOTAL_CMDLIST_CNT][PPC_CONFIG_ID_DSI_CNT];
	uint32_t ppc_enable_panel_estv_support;
	uint32_t ppc_enable_panel_estv_support_factory;
	uint32_t ppc_panel_estv_wait_te_cnt;

	struct notifier_block *notifier;
};

struct dpu_comp_panel_partial_ctrl {
	void* priv_data;
	struct dpu_composer *dpu_comp;
};

void dpu_ppc_release_priv_data(struct dpu_comp_ppc_ctrl *ppc_ctrl);
int32_t dpu_ppc_setup_priv_data(struct dpu_comp_ppc_ctrl *ppc_ctrl);
int32_t dpu_ppc_set_active_rect(struct dpu_comp_ppc_ctrl *ppc_ctrl, uint32_t ppc_config_id);
#endif
