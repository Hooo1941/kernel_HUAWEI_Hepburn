
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

#include <linux/delay.h>
#include "dpu_conn_mgr.h"
#include "dkmd_rect.h"
#include "mipi_dsi_partial_update.h"
#include "dkmd_lcd_interface.h"
#include "panel_mgr.h"

#define V_FRONT_PORCH_MAX 1023

static bool is_dirty_region_empty(const struct dkmd_rect *disp_rect)
{
	return (disp_rect->x == 0 && disp_rect->y == 0 && disp_rect->w == 0 && disp_rect->h == 0);
}

static bool is_dirty_region_valid(const struct dkmd_rect *dirty_rect,
	const struct dkmd_object_info *pinfo, struct dpu_connector *connector)
{
	uint32_t dsc_slice_height = connector->dsc.dsc_info.slice_height;

	if (pinfo->dsc_en == 0) /* dsc is disable */
		return true;

	/* dsc dirty_rect width should be panel width */
	if ((dirty_rect->x != 0) || (dirty_rect->w != pinfo->xres)) {
		dpu_pr_err("dirty_rect x or w is not panel_width for dsc!");
		return false;
	}

	/* dsc dirty_rect height should be align with dsc slice_height */
	if (dsc_slice_height == 0) {
		dpu_pr_err("slice height is 0 for dsc!");
		return false;
	}

	if ((dirty_rect->y % dsc_slice_height != 0) || (dirty_rect->h % dsc_slice_height != 0)) {
		dpu_pr_err("dirty_rect y or h is not aligned with dsc_slice_height for dsc!");
		return false;
	}

	return true;
}

static void get_dpu_dirty_rect(const struct dkmd_object_info *pinfo, const struct dkmd_rect *disp_rect,
	struct dkmd_rect *dirty_rect)
{
	if (!is_dirty_region_empty(disp_rect)) {
		*dirty_rect = *disp_rect;
		return;
	}

	dirty_rect->x = 0;
	dirty_rect->y = 0;
	dirty_rect->w = pinfo->xres;
	dirty_rect->h = pinfo->yres;
}

static bool is_same_dirty_rect(const struct dkmd_rect *dirty_rect, const struct dkmd_rect *new_dirty_rect)
{
	return ((dirty_rect->x == new_dirty_rect->x) && (dirty_rect->y == new_dirty_rect->y) &&
		(dirty_rect->w == new_dirty_rect->w) && (dirty_rect->h == new_dirty_rect->h));
}

static void mipi_dsi_partial_update_set_reg(const struct dpu_connector *connector)
{
	struct dkmd_rect dirty_rect = connector->dirty_rect;
	struct dkmd_connector_info *pinfo = connector->conn_info;
	struct dpu_connector *primay_connector = get_primary_connector(pinfo);
	uint32_t ldi_vrt_ctrl0;
	uint32_t edpi_cmd_size;

	if (pinfo->base.dsc_en != 0)
		dirty_rect.w = pinfo->base.dsc_out_width;

	if (is_dual_mipi_panel(&connector->conn_info->base))
		dirty_rect.w /= 2;

	ldi_vrt_ctrl0 = (connector->mipi.vfp + connector->conn_info->base.yres - dirty_rect.h);
	edpi_cmd_size = dirty_rect.w;

	set_reg(primay_connector->connector_base +
		DPU_DSI_LDI_DPI0_HRZ_CTRL2_ADDR(DSI_ADDR_TO_OFFSET), dpu_width(dirty_rect.w), 13, 0);
	set_reg(primay_connector->connector_base +
		DPU_DSI_LDI_VRT_CTRL2_ADDR(DSI_ADDR_TO_OFFSET), dpu_height(dirty_rect.h), 13, 0);
	set_reg(primay_connector->connector_base +
		DPU_DSI_VIDEO_VFP_NUM_ADDR(DSI_ADDR_TO_OFFSET), ldi_vrt_ctrl0, 20, 0);
	set_reg(primay_connector->connector_base +
		DPU_DSI_VIDEO_VACT_NUM_ADDR(DSI_ADDR_TO_OFFSET), dirty_rect.h, 24, 0);

	if (connector->mipi.color_mode != DSI_DSC24_COMPRESSED_DATA)
		set_reg(primay_connector->connector_base + DPU_DSI_EDPI_CMD_SIZE_ADDR(DSI_ADDR_TO_OFFSET), edpi_cmd_size, 16, 0);

	if (is_dual_mipi_panel(&connector->conn_info->base) && primay_connector->bind_connector) {
		set_reg(primay_connector->bind_connector->connector_base +
			DPU_DSI_LDI_DPI0_HRZ_CTRL2_ADDR(DSI_ADDR_TO_OFFSET), dpu_width(dirty_rect.w), 13, 0);
		set_reg(primay_connector->bind_connector->connector_base +
			DPU_DSI_LDI_VRT_CTRL2_ADDR(DSI_ADDR_TO_OFFSET), dpu_height(dirty_rect.h), 13, 0);
		set_reg(primay_connector->bind_connector->connector_base +
			DPU_DSI_VIDEO_VFP_NUM_ADDR(DSI_ADDR_TO_OFFSET), ldi_vrt_ctrl0, 20, 0);
		set_reg(primay_connector->bind_connector->connector_base +
			DPU_DSI_VIDEO_VACT_NUM_ADDR(DSI_ADDR_TO_OFFSET), dirty_rect.h, 24, 0);

		if (connector->mipi.color_mode != DSI_DSC24_COMPRESSED_DATA)
			set_reg(primay_connector->bind_connector->connector_base +
				DPU_DSI_EDPI_CMD_SIZE_ADDR(DSI_ADDR_TO_OFFSET), edpi_cmd_size, 16, 0);
	}

	dpu_pr_info("dirty_rect_origin[%d %d %u %u], dirty_rect_reg[%d %d %u %u], vfp=%u, ldi_vrt_ctrl0=%u",
		connector->dirty_rect.x, connector->dirty_rect.y, connector->dirty_rect.w, connector->dirty_rect.h,
		dirty_rect.x, dirty_rect.y, dirty_rect.w, dirty_rect.h, connector->mipi.vfp, ldi_vrt_ctrl0);
}

static void mipi_dsi_partial_update_set_display_region(struct dpu_connector *connector)
{
	struct dkmd_rect dirty_rect = connector->dirty_rect;

	if (is_dual_mipi_panel(&connector->conn_info->base)) {
		dirty_rect.x /= 2;
		dirty_rect.w /= 2;
	}

	pipeline_next_ops_handle(connector->conn_info->conn_device, connector->conn_info,
		LCD_SET_DISPLAY_REGION, &dirty_rect);
	dpu_pr_debug("set display region[%d %d %u %u]", dirty_rect.x, dirty_rect.y, dirty_rect.w, dirty_rect.h);
}

/*
 * partial update x not change, only y dynamic changed,
 * dsc compress width, height not change
 * so dirty region[0, y, panel_xres, h], dirty region after dsc[0, y, dsc_out_width, h]
 * for single mipi(single ddic) dirty region set to panel[0, y, panel_xres, h]
 * for dual mipi(dual ddic) dirty region set to panel[0, y, panel_xres/2, h]
 */
int32_t mipi_dsi_partial_update(struct dpu_connector *connector, const void *value)
{
	struct dkmd_connector_info *pinfo = connector->conn_info;
	struct dkmd_rect dirty_rect = {0};

	if (pinfo->dirty_region_updt_support == 0)
		return 0;

	get_dpu_dirty_rect(&pinfo->base, (const struct dkmd_rect *)value, &dirty_rect);
	dpu_pr_debug("dirty_rect[%d %d %u %u] pre dirty_rect[%d %d %u %u]",
		dirty_rect.x, dirty_rect.y, dirty_rect.w, dirty_rect.h,
		connector->dirty_rect.x, connector->dirty_rect.y, connector->dirty_rect.w, connector->dirty_rect.h);

	if (is_same_dirty_rect(&connector->dirty_rect, &dirty_rect))
		return 0;

	if (!is_dirty_region_valid(&dirty_rect, &pinfo->base, connector)) {
		dpu_pr_err("dirty_rect[%d %d %u %u] is invalid!", dirty_rect.x, dirty_rect.y, dirty_rect.w, dirty_rect.h);
		return 0;
	}

	connector->dirty_rect = dirty_rect;

	mipi_dsi_partial_update_set_reg(connector);

	mipi_dsi_partial_update_set_display_region(connector);
	return 0;
}

int32_t mipi_dsi_reset_partial_update(struct dpu_connector *connector, const void *value)
{
	struct dkmd_connector_info *pinfo = connector->conn_info;
	void_unused(value);

	connector->dirty_rect.x = 0;
	connector->dirty_rect.y = 0;
	connector->dirty_rect.w = pinfo->base.xres;
	connector->dirty_rect.h = pinfo->base.yres;

	dpu_pr_info("reset dirty_rect[%d %d %u %u]",
		connector->dirty_rect.x, connector->dirty_rect.y, connector->dirty_rect.w, connector->dirty_rect.h);

	return 0;
}

int32_t mipi_dsi_panel_partial_ctrl_set_reg(struct dpu_connector *connector, const void *value)
{
	char __iomem * first_dsi_addr = NULL;
	char __iomem * second_dsi_addr = NULL;
	struct dkmd_connector_info *pinfo = connector->conn_info;
	struct dpu_panel_ops *entry_pops = NULL;
	struct dpu_panel_info *panel_info = NULL;
	uint32_t *ppc_config_id = NULL;
	uint32_t ldi_vrt_ctrl0;
	struct dkmd_rect new_rect;

	dpu_check_and_return(!pinfo, -EINVAL, err, "pinfo is null\n");

	if (!is_ppc_support(&pinfo->base))
		return 0;

	dpu_check_and_return(!value, -EINVAL, err, "value is null\n");
	ppc_config_id = (uint32_t *)value;
	dpu_pr_info("ppc_config_id %u", *ppc_config_id);

	dpu_check_and_return(*ppc_config_id > PPC_CONFIG_ID_CNT, -EINVAL, err, "invalid config id\n");

	entry_pops = get_panel_ops(connector->conn_info->base.id);
	dpu_check_and_return(!entry_pops, -EINVAL, err, "entry pops is null\n");

	panel_info = entry_pops->get_panel_info();
	dpu_check_and_return(!panel_info, -EINVAL, err, "panel info is null\n");

	first_dsi_addr = connector->dpu_base + g_connector_offset[connector->connector_id];
	if (connector->bind_connector)
		second_dsi_addr = connector->dpu_base + g_connector_offset[connector->bind_connector->connector_id];

	new_rect.x = (int32_t)panel_info->ppc_config_id_rect_info[*ppc_config_id].rect.left;
	new_rect.y = (int32_t)panel_info->ppc_config_id_rect_info[*ppc_config_id].rect.top;
	new_rect.w = panel_info->ppc_config_id_rect_info[*ppc_config_id].rect.right -
					panel_info->ppc_config_id_rect_info[*ppc_config_id].rect.left;
	new_rect.h = panel_info->ppc_config_id_rect_info[*ppc_config_id].rect.bottom -
					panel_info->ppc_config_id_rect_info[*ppc_config_id].rect.top;
	if (pinfo->base.dsc_en != 0) {
		new_rect.w = pinfo->base.dsc_out_width;
		dpu_pr_info("dsc_out_width %u", pinfo->base.dsc_out_width);
	}

	if (is_dual_mipi_panel(&connector->conn_info->base))
		new_rect.w /= 2;

	ldi_vrt_ctrl0 = (connector->mipi.vfp + connector->conn_info->base.yres - new_rect.h);
	dpu_pr_info("mipi.vfp %u base.yres %u ppc_config_id %u", connector->mipi.vfp,
															connector->conn_info->base.yres, *ppc_config_id);

	set_reg(DPU_DSI_LDI_DPI0_HRZ_CTRL2_ADDR(first_dsi_addr), dpu_width(new_rect.w), 13, 0);
	set_reg(DPU_DSI_LDI_VRT_CTRL2_ADDR(first_dsi_addr), dpu_height(new_rect.h), 13, 0);
	set_reg(DPU_DSI_VIDEO_VFP_NUM_ADDR(first_dsi_addr), ldi_vrt_ctrl0, 20, 0);
	set_reg(DPU_DSI_VIDEO_VACT_NUM_ADDR(first_dsi_addr), new_rect.h, 24, 0);
	if (connector->mipi.color_mode != DSI_DSC24_COMPRESSED_DATA)
		set_reg(DPU_DSI_EDPI_CMD_SIZE_ADDR(first_dsi_addr), new_rect.w, 16, 0);

	if (connector->bind_connector) {
		set_reg(DPU_DSI_LDI_DPI0_HRZ_CTRL2_ADDR(second_dsi_addr), dpu_width(new_rect.w), 13, 0);
		set_reg(DPU_DSI_LDI_VRT_CTRL2_ADDR(second_dsi_addr), dpu_height(new_rect.h), 13, 0);
		set_reg(DPU_DSI_VIDEO_VFP_NUM_ADDR(second_dsi_addr), ldi_vrt_ctrl0, 20, 0);
		set_reg(DPU_DSI_VIDEO_VACT_NUM_ADDR(second_dsi_addr), new_rect.h, 24, 0);
		if (connector->mipi.color_mode != DSI_DSC24_COMPRESSED_DATA)
			set_reg(DPU_DSI_EDPI_CMD_SIZE_ADDR(second_dsi_addr), new_rect.w, 16, 0);
	}

	pinfo->ppc_config_id_active = *ppc_config_id;

	dpu_pr_info("ppc_config_id:%d, new_rect_reg[%d %d %u %u], vfp=%u, ldi_vrt_ctrl0=%u",
		*ppc_config_id, new_rect.x, new_rect.y, new_rect.w, new_rect.h, connector->mipi.vfp, ldi_vrt_ctrl0);

	return 0;
}
