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
#include "mipi_dsi_bit_clk_upt_helper.h"
#include <linux/atomic.h>
#include <securec.h>
#include <dpu/soc_dpu_define.h>
#include "peri/dkmd_peri.h"
#include "dkmd_log.h"
#include "mipi_cdphy_utils.h"
#include "dpu_connector.h"

#define PLL_FBK_DIV_MAX_VALUE 0xFF
#define MIPI_LDI_VSTATE (0x01FC)

#define SHIFT_4BIT 4
#define SHIFT_8BIT 8
#define SHIFT_16BIT 16

#define disp_reduce(x) ((x) > 0 ? ((x)-1) : (x))
#define MIPI_LDI_DPI0_HRZ_CTRL3 (0x01F4)

#define PHY_LOCK_STANDARD_STATUS 0x00000001

#define PLL_PRE_DIV_ADDR 0x00010048
#define PLL_POS_DIV_ADDR 0x00010049
#define PLL_FBK_DIV_ADDR 0x0001004A
#define PLL_UPT_CTRL_ADDR 0x0001004B

#define DEFAULT_VFP_TIME 80
#define MIPI_CLK_UPDT_TIMEOUT 30

static bool mipi_dsi_check_ldi_vstate(const char __iomem* mipi_dsi_base, uint64_t lane_byte_clk)
{
	bool is_ready = false;
	uint32_t count = 0;
	uint32_t ldi_vstate;

	/* read ldi vstate reg value and mask bit0:15 */
	ldi_vstate = inp32(mipi_dsi_base + MIPI_LDI_VSTATE) & 0xFFFF; /*lint -e529*/
	while ((count < DEFAULT_VFP_TIME)) {
		udelay(1); /* 1us delay each time  */
		ldi_vstate = inp32(mipi_dsi_base + MIPI_LDI_VSTATE) & 0xFFFF; /*lint -e529*/
		count++;
		if (ldi_vstate == LDI_VSTATE_VFP)
			break;
	}

	if ((ldi_vstate == LDI_VSTATE_VFP) || (ldi_vstate == LDI_VSTATE_IDLE) || (ldi_vstate == LDI_VSTATE_V_WAIT_GPU))
		is_ready = true;

	if (!is_ready)
		dpu_pr_debug("ldi_vstate = 0x%x\n", ldi_vstate);

	return is_ready;
}

static void mipi_pll_cfg_for_clk_upt(char __iomem* mipi_dsi_base, struct mipi_dsi_phy_ctrl* phy_ctrl)
{
	/* PLL configuration III */
	mipi_config_phy_test_code(
		mipi_dsi_base, PLL_POS_DIV_ADDR, (phy_ctrl->rg_pll_posdiv << SHIFT_4BIT) | phy_ctrl->rg_pll_prediv);
	/* PLL configuration IV */
	mipi_config_phy_test_code(mipi_dsi_base, PLL_FBK_DIV_ADDR, (phy_ctrl->rg_pll_fbkdiv & PLL_FBK_DIV_MAX_VALUE));
}

static void mipi_dsi_set_cdphy_bit_clk_upt_cmd(
	struct dpu_connector* connector, char __iomem* mipi_dsi_base, struct mipi_dsi_phy_ctrl* phy_ctrl)
{
	bool is_ready = false;

	/* PLL configuration and update control */
	mipi_cdphy_init_config(connector, phy_ctrl, mipi_dsi_base);

	mipi_dsi_phy_timing_config(phy_ctrl, mipi_dsi_base);

	/* escape clock dividor */
	set_reg(DPU_DSI_CLK_DIV_CTRL_ADDR(mipi_dsi_base), phy_ctrl->clk_division, 8, 0);
	set_reg(DPU_DSI_CLK_DIV_CTRL_ADDR(mipi_dsi_base), phy_ctrl->clk_division, 8, 8);

	is_ready = mipi_phy_status_check(mipi_dsi_base, PHY_LOCK_STANDARD_STATUS);
	if (!is_ready)
		dpu_pr_debug("connector %u phylock is not ready!\n", connector->connector_id);
}

static void mipi_dsi_set_cdphy_bit_clk_upt_video(
	struct dpu_connector* connector, char __iomem* mipi_dsi_base, struct mipi_dsi_phy_ctrl* phy_ctrl)
{
	uint32_t fpga_flag = 0;
	bool is_ready = false;
	struct mipi_dsi_timing timing;
	struct mipi_panel_info* mipi = NULL;
	struct dkmd_connector_info* pinfo = NULL;

	mipi = &connector->mipi;
	fpga_flag = connector->conn_info->base.fpga_flag;
	pinfo = connector->conn_info;
	connector->dsi_phy_ctrl = *phy_ctrl;

	/* PLL configuration */
	mipi_pll_cfg_for_clk_upt(mipi_dsi_base, phy_ctrl);

	/* PLL update control */
	mipi_config_phy_test_code(mipi_dsi_base, PLL_UPT_CTRL_ADDR, 0x1);

	if (mipi->phy_mode == CPHY_MODE)
		mipi_config_cphy_spec1v0_parameter(phy_ctrl, mipi, mipi_dsi_base, (int32_t)fpga_flag);
	else
		mipi_config_dphy_spec1v2_parameter(phy_ctrl, mipi, mipi_dsi_base);

	is_ready = mipi_phy_status_check(mipi_dsi_base, PHY_LOCK_STANDARD_STATUS);
	if (!is_ready)
		dpu_pr_debug("fb%d, phylock is not ready!\n", connector->connector_id);

	/* phy_stop_wait_time */
	set_reg(DPU_DSI_CDPHY_LANE_NUM_ADDR(mipi_dsi_base), phy_ctrl->phy_stop_wait_time, 8, 8);

	/*
	 * 4. Define the DPI Horizontal timing configuration:
	 *
	 * Hsa_time = HSA*(PCLK period/Clk Lane Byte Period);
	 * Hbp_time = HBP*(PCLK period/Clk Lane Byte Period);
	 * Hline_time = (HSA+HBP+HACT+HFP)*(PCLK period/Clk Lane Byte Period);
	 */
	memset_s(&timing, sizeof(timing), 0, sizeof(timing));
	get_mipi_dsi_timing_config_para(mipi, phy_ctrl, &timing);

	if (timing.hline_time < (timing.hsa + timing.hbp + timing.dpi_hsize))
		dpu_pr_debug("wrong hfp\n");

	set_reg(DPU_DSI_VIDEO_HSA_NUM_ADDR(mipi_dsi_base), timing.hsa, 12, 0);
	set_reg(DPU_DSI_VIDEO_HBP_NUM_ADDR(mipi_dsi_base), timing.hbp, 12, 0);
	set_reg(mipi_dsi_base + MIPI_LDI_DPI0_HRZ_CTRL3, disp_reduce(timing.dpi_hsize), 13, 0);
	set_reg(DPU_DSI_VIDEO_HLINE_NUM_ADDR(mipi_dsi_base), timing.hline_time, 15, 0);

	/* Configure core's phy parameters */
	set_reg(DPU_DSI_CLKLANE_TRANS_TIME_ADDR(mipi_dsi_base), phy_ctrl->clk_lane_lp2hs_time, 10, 0);
	set_reg(DPU_DSI_CLKLANE_TRANS_TIME_ADDR(mipi_dsi_base), phy_ctrl->clk_lane_hs2lp_time, 10, 16);

	outp32(DPU_DSI_DATALANE_TRNAS_TIME_ADDR(mipi_dsi_base),
		(phy_ctrl->data_lane_lp2hs_time + (phy_ctrl->data_lane_hs2lp_time << 16)));

	dpu_pr_debug("connector_id %d End", connector->connector_id);
}

static void mipi_set_cdphy_bit_clk(struct dpu_connector* connector, struct mipi_dsi_phy_ctrl* phy_ctrl,
	struct dkmd_connector_info* pinfo, uint8_t esd_enable)
{
	struct dpu_connector* bind_connector = NULL;
	char __iomem* mipi_dsi_base = connector->connector_base;

	if (is_mipi_cmd_panel(&pinfo->base)) {
		mipi_dsi_set_cdphy_bit_clk_upt_cmd(connector, mipi_dsi_base, phy_ctrl);
		if (is_dual_mipi_panel(&pinfo->base)) {
			bind_connector = connector->bind_connector;
			if (bind_connector)
				mipi_dsi_set_cdphy_bit_clk_upt_cmd(connector, bind_connector->connector_base, phy_ctrl);
		}
	} else {
		set_reg(DPU_DSI_LP_CLK_CTRL_ADDR(mipi_dsi_base), 0x0, DSI_CLK_BW, DSI_CLK_BS);
		mipi_dsi_set_cdphy_bit_clk_upt_video(connector, mipi_dsi_base, phy_ctrl);
		set_reg(DPU_DSI_LP_CLK_CTRL_ADDR(mipi_dsi_base), 0x1, DSI_CLK_BW, DSI_CLK_BS);
		if (is_dual_mipi_panel(&pinfo->base)) {
			bind_connector = connector->bind_connector;
			if (bind_connector) {
				set_reg(DPU_DSI_LP_CLK_CTRL_ADDR(bind_connector->connector_base), 0x0, DSI_CLK_BW, DSI_CLK_BS);
				mipi_dsi_set_cdphy_bit_clk_upt_video(connector, bind_connector->connector_base, phy_ctrl);
				set_reg(DPU_DSI_LP_CLK_CTRL_ADDR(bind_connector->connector_base), 0x1, DSI_CLK_BW, DSI_CLK_BS);
			}
		}
		pinfo->esd_enable = esd_enable;
		pinfo->enable_ldi(pinfo);
	}
}

int mipi_dsi_bit_clk_upt(struct dpu_connector* connector, const void* value)
{
	struct mipi_dsi_phy_ctrl phy_ctrl = {0};
	struct dkmd_connector_info* conn_info = NULL;
	bool is_ready = false;
	uint8_t esd_enable;
	uint64_t lane_byte_clk;
	uint32_t dsi_bit_clk_upt;

	dpu_check_and_return(!connector, 0, err, "connector is NULL!\n");
	conn_info = connector->conn_info;
	dsi_bit_clk_upt = connector->mipi.dsi_bit_clk_upt;
	dpu_check_and_return(
		connector->connector_id != CONNECTOR_ID_DSI0 &&
		connector->connector_id != CONNECTOR_ID_DSI0_BUILTIN &&
		connector->connector_id != CONNECTOR_ID_DSI2,
		0, err, "connector %d, not support\n", connector->connector_id);

	atomic_set(&connector->mipi.dsi_bit_clk_upt_flag, 1);
	esd_enable = conn_info->esd_enable;
	if (dsi_bit_clk_upt == connector->mipi.dsi_bit_clk) {
		atomic_set(&connector->mipi.dsi_bit_clk_upt_flag, 0);
		dpu_pr_debug("mipi_dsi_bit_clk_upt not need change");
		return 0;
	}

	dpu_pr_debug("Mipi clk need change from %d M switch to %d M\n", connector->mipi.dsi_bit_clk, dsi_bit_clk_upt);
	if (connector->mipi.phy_mode == CPHY_MODE)
		get_dsi_cphy_ctrl(connector, &phy_ctrl);
	else
		get_dsi_dphy_ctrl(connector, &phy_ctrl);

	lane_byte_clk = connector->dsi_phy_ctrl.lane_byte_clk;
	if (connector->mipi.phy_mode == CPHY_MODE)
		lane_byte_clk = connector->dsi_phy_ctrl.lane_word_clk;

	if (is_mipi_video_panel(&conn_info->base) && conn_info->disable_ldi) {
		conn_info->esd_enable = 0;
		conn_info->disable_ldi(conn_info);
	}

	is_ready = mipi_dsi_check_ldi_vstate(connector->connector_base, lane_byte_clk);
	if (!is_ready) {
		if (is_mipi_video_panel(&conn_info->base) && conn_info->enable_ldi) {
			conn_info->esd_enable = esd_enable;
			conn_info->enable_ldi(conn_info);
		}
		dpu_pr_debug("PERI_STAT0 or ldi vstate is not ready\n");
		atomic_set(&connector->mipi.dsi_bit_clk_upt_flag, 0);
		return 0;
	}

	mipi_set_cdphy_bit_clk(connector, &phy_ctrl, conn_info, esd_enable);
	dpu_pr_info("Mipi clk success changed from %d M switch to %d M\n", connector->mipi.dsi_bit_clk, dsi_bit_clk_upt);

	connector->dsi_phy_ctrl = phy_ctrl;
	connector->mipi.dsi_bit_clk = dsi_bit_clk_upt;
	atomic_set(&connector->mipi.dsi_bit_clk_upt_flag, 0);
	return 0;
}

void mipi_dsi_bit_clk_upt_isr_handler(struct dpu_connector* connector)
{
	pipeline_next_ops_handle(connector->conn_info->conn_device, connector->conn_info, MIPI_DSI_BIT_CLK_UPT, NULL);
}


int wait_for_mipi_resource_available(struct dpu_connector* connector)
{
	int wait_count = 0;
	int flag = 0;
	int flag_tmp = 0;

	dpu_check_and_return(!connector, -1, err, "connector is NULL!\n");
	flag = atomic_read(&connector->mipi.dsi_bit_clk_upt_flag);
	if (flag == 1) {
		flag_tmp = flag;
		while ((flag_tmp == 1) && (wait_count < MIPI_CLK_UPDT_TIMEOUT)) {
			wait_count++;
			usleep_range(1000, 1100); /* sleep range from 1000us to 1100us */
			flag_tmp = atomic_read(&connector->mipi.dsi_bit_clk_upt_flag);
		}
		dpu_pr_debug("wait flag 0x%x, cost time %d ms", flag, wait_count);
	}
	if (wait_count >= MIPI_CLK_UPDT_TIMEOUT)
		return -1;
	return 0;
}
