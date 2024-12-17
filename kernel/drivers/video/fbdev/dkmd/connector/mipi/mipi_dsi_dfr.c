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

#include <linux/module.h>
#include <dpu/soc_dpu_define.h>
#include "dkmd_log.h"
#include "dpu_connector.h"
#include "mipi_dsi_dev.h"

static void mipi_dsi_dfr_set_porch_reg(struct mipi_panel_info *mipi,
	char __iomem *mipi_dsi_base, int mode)
{
	switch (mode) {
	case DFR_MODE_LONG_V:
		set_reg(DPU_DSI_VIDEO_VFP_NUM_ADDR(mipi_dsi_base), mipi->vfp, 20, 0);
		break;
	case DFR_MODE_LONG_H:
		set_reg(DPU_DSI_VIDEO_HLINE_NUM_ADDR(mipi_dsi_base), mipi->hline_time, 15, 0);
		break;
	case DFR_MODE_LONG_VH:
		set_reg(DPU_DSI_VIDEO_VFP_NUM_ADDR(mipi_dsi_base), mipi->vfp, 20, 0);
		set_reg(DPU_DSI_VIDEO_HLINE_NUM_ADDR(mipi_dsi_base), mipi->hline_time, 15, 0);
		set_reg(DPU_DSI_SHADOW_REG_CTRL_ADDR(mipi_dsi_base), 0x1, 1, 8);
		break;
	default:
		dpu_pr_warn("unspported dfr mode = %d", mode);
		break;
	}
}

static void mipi_dsi_dfr_porch_update(struct dpu_connector *connector,
	int target_framerate, int mode)
{
	uint32_t frm_rate_index = 0;
	struct mipi_panel_info *mipi = &connector->mipi;
	char __iomem *mipi_dsi_base = connector->connector_base;
	uint32_t mipi_frm_rate_mode_num;

	mipi_frm_rate_mode_num = mipi->mipi_frm_rate_mode_num < MIPI_FRM_RATE_NUM_MAX ?
		mipi->mipi_frm_rate_mode_num : MIPI_FRM_RATE_NUM_MAX;

	for (frm_rate_index = 0; frm_rate_index < mipi_frm_rate_mode_num; frm_rate_index++) {
		if (mipi->mipi_timing_modes[frm_rate_index].frame_rate == (uint32_t)target_framerate) {
			mipi->hsa = mipi->mipi_timing_modes[frm_rate_index].mipi_timing.hsa;
			mipi->hbp = mipi->mipi_timing_modes[frm_rate_index].mipi_timing.hbp;
			mipi->dpi_hsize = mipi->mipi_timing_modes[frm_rate_index].mipi_timing.dpi_hsize;
			mipi->hline_time = mipi->mipi_timing_modes[frm_rate_index].mipi_timing.hline_time;
			mipi->vsa = mipi->mipi_timing_modes[frm_rate_index].mipi_timing.vsa;
			mipi->vbp = mipi->mipi_timing_modes[frm_rate_index].mipi_timing.vbp;
			mipi->vfp = mipi->mipi_timing_modes[frm_rate_index].mipi_timing.vfp;

			mipi_dsi_dfr_set_porch_reg(mipi, mipi_dsi_base, mode);

			dpu_pr_info("target framerate = %d, paramters:dpi_hsize = %u,hsa = %u, hbp = %u, hline_time = %u,"
				"vsa=%u, vbp=%u, vfp=%u, vactive_line=%u\n",
				target_framerate, mipi->dpi_hsize, mipi->hsa,
				mipi->hbp, mipi->hline_time,
				mipi->vsa, mipi->vbp,
				mipi->vfp, mipi->vactive_line);
			return;
		}
	}

	dpu_pr_info("unspported target frame rate %d\n");
}

void mipi_dsi_dfr_update(struct dpu_connector *connector, int target_frmrate, int mode)
{
	dpu_check_and_no_retval(!connector, err, "connector is null ptr\n");

	mipi_dsi_dfr_porch_update(connector, target_frmrate, mode);
	if (is_dual_mipi_panel(&connector->conn_info->base) && connector->bind_connector)
		mipi_dsi_dfr_porch_update(connector->bind_connector, target_frmrate, mode);
}

