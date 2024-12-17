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
#include "mipi_config_utils.h"
#include "dkmd_log.h"
#include "dkmd_object.h"
#include <dpu/soc_dpu_define.h>
#include "dpu_connector.h"

void mipi_dsi_reset_init(struct dpu_connector *connector)
{
	dpu_check_and_no_retval(!connector, err, "connector is NULL!");

	set_reg(DPU_DSI_GLB_DSI_RESET_EN_ADDR(connector->dpu_base + DPU_MIPI_DSI_GLB_OFFSET),
		1, 1, (uint8_t)get_connector_phy_id(connector->connector_id));
}

void mipi_dsi_reset_deinit(struct dpu_connector *connector)
{
	void_unused(connector);
}

void mipi_dsi_mem_init(char __iomem *mipi_dsi_base)
{
	/* dsi_mem_ctrl bit[28:27] TRA = 0x01 */
	set_reg(DPU_DSI_MEM_CTRL_ADDR(mipi_dsi_base), 0x1, 2, 27);
}

/* TE_CTRL_3 is used by dacc */
void mipi_dsi_te_ctrl3_init(char __iomem *mipi_dsi_base, uint32_t te_pin_val, uint64_t lane_byte_clk)
{
	set_reg(DPU_DSI_TE_CTRL_3_ADDR(mipi_dsi_base), (0x1 << 17) | (te_pin_val << 6) | 0x1, 18, 0);

	set_reg(DPU_DSI_TE_HS_NUM_3_ADDR(mipi_dsi_base), 0x0, 32, 0);
	set_reg(DPU_DSI_TE_HS_WD_3_ADDR(mipi_dsi_base), 0x24024, 32, 0);

	set_reg(DPU_DSI_TE_VS_WD_3_ADDR(mipi_dsi_base), (uint32_t)((0x3FC << 12) | (2 * lane_byte_clk / 1000000)), 32, 0);
}

bool mipi_dsi_is_bypass_by_pg(uint32_t connector_id)
{
	void_unused(connector_id);
	return false;
}

void mipi_dsi_enable_hs_pkt_discontin_ctrl(char __iomem *mipi_dsi_base)
{
	dpu_check_and_no_retval(!mipi_dsi_base, err, "mipi_dsi_base is NULL!");

	dpu_pr_info("+");
	set_reg(DPU_DSI_CMD_HS_PKT_DISCONTIN_CTRL_ADDR(mipi_dsi_base), 0x1, 1, 0);
}
