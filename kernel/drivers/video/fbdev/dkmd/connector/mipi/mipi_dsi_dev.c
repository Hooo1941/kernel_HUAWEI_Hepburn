
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
#include <soc_pctrl_interface.h>
#include "dpu_conn_mgr.h"
#include "dsc/dsc_config.h"
#include "spr/spr_config.h"
#include "mipi_dsi_partial_update.h"
#include "mipi_config_utils.h"
#include "dpu_config_utils.h"
#include "mipi_dsi_bit_clk_upt_helper.h"
#include "dksm_dmd.h"
#include "panel_mgr.h"

extern uint32_t get_lcd_always_on(void);

/*******************************************************************************
 * MIPI DPHY GPIO for FPGA
 */
#define GPIO_MIPI_DPHY_PG_SEL_A_NAME "pg_sel_a"
#define GPIO_MIPI_DPHY_PG_SEL_B_NAME "pg_sel_b"
#define GPIO_MIPI_DPHY_TX_RX_A_NAME "tx_rx_a"
#define GPIO_MIPI_DPHY_TX_RX_B_NAME "tx_rx_b"

#define GPIO_PG_SEL_A (56)
#define GPIO_TX_RX_A (58)
#define GPIO_PG_SEL_B (37)
#define GPIO_TX_RX_B (39)

static uint32_t gpio_pg_sel_a = GPIO_PG_SEL_A;
static uint32_t gpio_tx_rx_a = GPIO_TX_RX_A;
static uint32_t gpio_pg_sel_b = GPIO_PG_SEL_B;
static uint32_t gpio_tx_rx_b = GPIO_TX_RX_B;

static struct gpio_desc mipi_dphy_gpio_request_cmds[] = {
	{ DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0, GPIO_MIPI_DPHY_PG_SEL_A_NAME, &gpio_pg_sel_a, 0 },
	{ DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0, GPIO_MIPI_DPHY_PG_SEL_B_NAME, &gpio_pg_sel_b, 0 },
	{ DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0, GPIO_MIPI_DPHY_TX_RX_A_NAME, &gpio_tx_rx_a, 0 },
	{ DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0, GPIO_MIPI_DPHY_TX_RX_B_NAME, &gpio_tx_rx_b, 0 },
};

static struct gpio_desc mipi_dphy_gpio_free_cmds[] = {
	{ DTYPE_GPIO_FREE, WAIT_TYPE_MS, 0, GPIO_MIPI_DPHY_PG_SEL_A_NAME, &gpio_pg_sel_a, 0 },
	{ DTYPE_GPIO_FREE, WAIT_TYPE_MS, 0, GPIO_MIPI_DPHY_PG_SEL_B_NAME, &gpio_pg_sel_b, 0 },
	{ DTYPE_GPIO_FREE, WAIT_TYPE_MS, 0, GPIO_MIPI_DPHY_TX_RX_A_NAME, &gpio_tx_rx_a, 0 },
	{ DTYPE_GPIO_FREE, WAIT_TYPE_MS, 0, GPIO_MIPI_DPHY_TX_RX_B_NAME, &gpio_tx_rx_b, 0 },
};

static struct gpio_desc mipi_dphy_gpio_normal_cmds[] = {
	{ DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 0, GPIO_MIPI_DPHY_PG_SEL_A_NAME, &gpio_pg_sel_a, 0 },
	{ DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 0, GPIO_MIPI_DPHY_TX_RX_A_NAME, &gpio_tx_rx_a, 1 },
	{ DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 0, GPIO_MIPI_DPHY_PG_SEL_B_NAME, &gpio_pg_sel_b, 0 },
	{ DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 0, GPIO_MIPI_DPHY_TX_RX_B_NAME, &gpio_tx_rx_b, 1 },
};

static struct gpio_desc mipi_dphy_gpio_lowpower_cmds[] = {
	{ DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 0, GPIO_MIPI_DPHY_PG_SEL_A_NAME, &gpio_pg_sel_a, 0 },
	{ DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 0, GPIO_MIPI_DPHY_TX_RX_A_NAME, &gpio_tx_rx_a, 0 },
	{ DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 0, GPIO_MIPI_DPHY_PG_SEL_B_NAME, &gpio_pg_sel_b, 0 },
	{ DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 0, GPIO_MIPI_DPHY_TX_RX_B_NAME, &gpio_tx_rx_b, 0 },
};

static void mipi_dsi_dphy_fastboot_fpga(struct dkmd_connector_info *pinfo)
{
	if (pinfo->base.fpga_flag)
		(void)peri_gpio_cmds_tx(mipi_dphy_gpio_request_cmds, ARRAY_SIZE(mipi_dphy_gpio_request_cmds));
}

static void mipi_dsi_dphy_on_fpga(struct dkmd_connector_info *pinfo)
{
	if (pinfo->base.fpga_flag) {
		(void)peri_gpio_cmds_tx(mipi_dphy_gpio_request_cmds, ARRAY_SIZE(mipi_dphy_gpio_request_cmds));
		(void)peri_gpio_cmds_tx(mipi_dphy_gpio_normal_cmds, ARRAY_SIZE(mipi_dphy_gpio_normal_cmds));
	}
}

static int32_t mipi_dsi_clk_enable(struct dpu_connector *connector)
{
	uint32_t i = 0;
	int32_t ret = 0;

	for (i = 0; i < CLK_GATE_MAX_IDX; i++) {
		if (!IS_ERR_OR_NULL(connector->connector_clk[i])) {
			ret = clk_prepare_enable(connector->connector_clk[i]);
			if (ret)
				dpu_pr_err("clk_id:%u enable failed, error=%d!\n", i, ret);
		}
	}

	/* dsi mem shut down */
	if (connector->pctrl_base)
		set_reg(SOC_PCTRL_PERI_CTRL102_ADDR(connector->pctrl_base), 0xc0000, 32, 0);

	if (connector->bind_connector) {
		dpu_pr_info("set bind_connector clk enable!\n");
		ret = mipi_dsi_clk_enable(connector->bind_connector);
	}

	if (connector->conn_info->sw_post_chn_idx[PRIMARY_CONNECT_CHN_IDX] != 0 &&
		connector->conn_info->sw_post_chn_idx[PRIMARY_CONNECT_CHN_IDX] < 4) {
		dpu_pr_info("set %d clk enable!\n", connector->conn_info->sw_post_chn_idx[PRIMARY_CONNECT_CHN_IDX]);
		set_reg(DPU_GLB_DISP_MODULE_CLK_SEL_ADDR(connector->dpu_base + DPU_GLB0_OFFSET),
			0x1, 1, 4 - (uint8_t)connector->conn_info->sw_post_chn_idx[PRIMARY_CONNECT_CHN_IDX]);
		set_reg(DPU_GLB_DISP_MODULE_CLK_EN_ADDR(connector->dpu_base + DPU_GLB0_OFFSET),
			0x1, 1, 4 - (uint8_t)connector->conn_info->sw_post_chn_idx[PRIMARY_CONNECT_CHN_IDX]);
	}

	return ret;
}

static void mipi_dsi_on_sub1(struct dpu_connector *connector)
{
	char __iomem *mipi_dsi_base = connector->connector_base;

	/* mipi init */
	mipi_init(connector);

	/* dsi memory init */
	mipi_dsi_mem_init(mipi_dsi_base);

	/* switch to cmd mode */
	set_reg(DPU_DSI_MODE_CTRL_ADDR(mipi_dsi_base), 0x1, 1, 0);
	/* cmd mode: low power mode */
	set_reg(DPU_DSI_CMD_MODE_CTRL_ADDR(mipi_dsi_base), 0x7f, 7, 8);
	set_reg(DPU_DSI_CMD_MODE_CTRL_ADDR(mipi_dsi_base), 0xf, 4, 16);
	set_reg(DPU_DSI_CMD_MODE_CTRL_ADDR(mipi_dsi_base), 0x1, 1, 24);
	/* disable generate High Speed clock */
	set_reg(DPU_DSI_LP_CLK_CTRL_ADDR(mipi_dsi_base), 0x0, 1, 0);

	if (connector->bind_connector) {
		dpu_pr_info("set bind_connector init and on sub1!\n");
		mipi_dsi_on_sub1(connector->bind_connector);
	}
}

static void mipi_dsi_on_sub2(struct dpu_connector *connector)
{
	char __iomem *mipi_dsi_base = connector->connector_base;
	struct dkmd_connector_info *pinfo = connector->conn_info;
	struct dfr_info *dfr_info = NULL;

	/* switch to video mode */
	if (is_mipi_video_panel(&pinfo->base))
		set_reg(DPU_DSI_MODE_CTRL_ADDR(mipi_dsi_base), 0x0, 1, 0);

	/* set reload mode for both longv and longh exist */
	dfr_info = dkmd_get_dfr_info(pinfo);
	if (dfr_info && dfr_info->dfr_mode == DFR_MODE_LONG_VH)
		set_reg(DPU_DSI_SHADOW_REG_CTRL_ADDR(mipi_dsi_base), 0x101, 32, 0);

	/* cmd mode: high speed mode */
	if (is_mipi_cmd_panel(&pinfo->base)) {
		set_reg(DPU_DSI_CMD_MODE_CTRL_ADDR(mipi_dsi_base), 0x0, 7, 8);
		set_reg(DPU_DSI_CMD_MODE_CTRL_ADDR(mipi_dsi_base), 0x0, 4, 16);
		set_reg(DPU_DSI_CMD_MODE_CTRL_ADDR(mipi_dsi_base), 0x0, 1, 24);
	}

	/* enable EOTP TX */
	if (connector->mipi.phy_mode == DPHY_MODE)
		set_reg(DPU_DSI_PERIP_CHAR_CTRL_ADDR(mipi_dsi_base), 0x1, 1, 0);

	/* enable generate High Speed clock, non continue */
	if (connector->mipi.non_continue_en)
		set_reg(DPU_DSI_LP_CLK_CTRL_ADDR(mipi_dsi_base), 0x3, 2, 0);
	else
		set_reg(DPU_DSI_LP_CLK_CTRL_ADDR(mipi_dsi_base), 0x1, 2, 0);

	if (connector->bind_connector) {
		dpu_pr_info("set bind_connector on sub2!\n");
		mipi_dsi_on_sub2(connector->bind_connector);
	}

	if (pinfo->base.hs_pkt_discontin_support)
		mipi_dsi_enable_hs_pkt_discontin_ctrl(mipi_dsi_base);
}

static int32_t mipi_dsi_on(struct dkmd_connector_info *pinfo)
{
	struct dpu_connector *connector = NULL;

	if (!pinfo) {
		dpu_pr_err("pinfo is NULL!\n");
		return -EINVAL;
	}

	connector = get_primary_connector(pinfo);
	if (!connector) {
		dpu_pr_err("primary connector is NULL!\n");
		return -EINVAL;
	}

	connector->enable_ser_vp_sync = true;

	mipi_dsi_dphy_on_fpga(pinfo);

	/* pipeline next connector hardware power on */
	pipeline_next_on(pinfo->base.peri_device, pinfo);

	(void)mipi_dsi_clk_enable(connector);

	mipi_dsi_on_sub1(connector);

	/* set ppc_config_id to lcdkit before step2 */
	(void)pipeline_next_ops_handle(pinfo->base.peri_device, pinfo, SET_PPC_CONFIG_ID, &pinfo->ppc_config_id_record);

	/* mipi lp video/command mode */
	pipeline_next_on(pinfo->base.peri_device, pinfo);

	mipi_dsi_on_sub2(connector);

	/* mipi hs video/command mode */
	pipeline_next_on(pinfo->base.peri_device, pinfo);

	/* set ppc mipi para */
	mipi_dsi_panel_partial_ctrl_set_reg(connector, &pinfo->ppc_config_id_record);

	(void)pipeline_next_ops_handle(pinfo->base.peri_device, pinfo, CHECK_LCD_STATUS, NULL);

	dpu_pr_info("primary dsi-%d -\n", connector->connector_id);

	return 0;
}

static void mipi_dsi_dphy_off_fpga(struct dkmd_connector_info *pinfo)
{
	if (pinfo->base.fpga_flag) {
		(void)peri_gpio_cmds_tx(mipi_dphy_gpio_lowpower_cmds, ARRAY_SIZE(mipi_dphy_gpio_lowpower_cmds));
		(void)peri_gpio_cmds_tx(mipi_dphy_gpio_free_cmds, ARRAY_SIZE(mipi_dphy_gpio_free_cmds));
	}
}

static void mipi_dsi_clk_disable(struct dpu_connector *connector)
{
	int32_t i = 0;

	for (i = 0; i < CLK_GATE_MAX_IDX; i++) {
		if (!IS_ERR_OR_NULL(connector->connector_clk[i]))
			clk_disable_unprepare(connector->connector_clk[i]);
	}

	if (connector->bind_connector) {
		dpu_pr_info("set bind_connector clk disable!\n");
		mipi_dsi_clk_disable(connector->bind_connector);
	}
}

static void mipi_dsi_off_sub(struct dpu_connector *connector)
{
	char __iomem *mipi_dsi_base = connector->connector_base;

	/* switch to cmd mode */
	set_reg(DPU_DSI_MODE_CTRL_ADDR(mipi_dsi_base), 0x1, 1, 0);
	/* cmd mode: low power mode */
	set_reg(DPU_DSI_CMD_MODE_CTRL_ADDR(mipi_dsi_base), 0x7f, 7, 8);
	set_reg(DPU_DSI_CMD_MODE_CTRL_ADDR(mipi_dsi_base), 0xf, 4, 16);
	set_reg(DPU_DSI_CMD_MODE_CTRL_ADDR(mipi_dsi_base), 0x1, 1, 24);

	/* disable generate High Speed clock */
	set_reg(DPU_DSI_LP_CLK_CTRL_ADDR(mipi_dsi_base), 0x0, 1, 0);
	udelay(10);  /* 10us */

	/* shutdown d_phy */
	set_reg(DPU_DSI_CDPHY_RST_CTRL_ADDR(mipi_dsi_base), 0x0, 3, 0);

	mipi_dsi_reset_deinit(connector);

	if (connector->bind_connector) {
		dpu_pr_info("set bind_connector off!\n");
		mipi_dsi_off_sub(connector->bind_connector);
	}
}

int32_t mipi_wait_ldi_vstate_idle(struct dpu_connector *connector, const void *value)
{
	uint32_t delay_count = 0;
	uint32_t delay_count_max = 16; /* Read register status, maximum waiting time is 16ms */
	void_unused(value);

	/*
	 * Read register status, maximum waiting time is 16ms
	 * 0x7FF--The lower 11 bits of the register
	 * 0x1--Register value
	 */
	while (((uint32_t)inp32(DPU_DSI_VSTATE_ADDR(connector->connector_base)) & 0x7FF) != 0x1) {
		if (++delay_count > delay_count_max) {
			dpu_pr_warn("wait ldi vstate idle timeout\n");
			break;
		}
		usleep_range(1000, 1100); /* need sleep 1ms. */
	}

	if (delay_count > 0)
		dpu_pr_warn("wait ldi vstate idle done, delay_count = %u\n", delay_count);

	return 0;
}

static uint32_t mipi_get_lcd_always_on(void)
{
#if defined(CONFIG_DKMD_DPU_AOD)
	return get_lcd_always_on();
#else
	return 0;
#endif
}

static int32_t mipi_dsi_power_off(struct dkmd_connector_info *pinfo)
{
	struct dpu_connector *connector = NULL;

	if (!pinfo) {
		dpu_pr_err("pinfo is NULL!\n");
		return -EINVAL;
	}

	connector = get_primary_connector(pinfo);
	if (!connector) {
		dpu_pr_err("primary connector is NULL!\n");
		return -EINVAL;
	}
	dpu_pr_info("dsi-%d, +\n", connector->connector_id);
	/* add MIPI LP mode here if necessary MIPI LP mode end */

	/* disable mipi ldi */
	pinfo->disable_ldi(pinfo);

	/*
	 * Read register status, maximum waiting time is 16ms
	 * 0x7FF--The lower 11 bits of the register
	 * 0x1--Register value
	 */
	if (is_mipi_video_panel(&pinfo->base) || (mipi_get_lcd_always_on() == 1))
		mipi_wait_ldi_vstate_idle(connector, NULL);

	/* Here need to enter ulps when panel off bypass ddic power down */
	mipi_dsi_off_sub(connector);
	mipi_dsi_clk_disable(connector);
	mipi_dsi_dphy_off_fpga(pinfo);
	connector->is_vactive_end_recieved = VACTIVE_END_RECEIVED;
	connector->enable_ser_vp_sync = false;
	return 0;
}

static int32_t mipi_dsi_off(struct dkmd_connector_info *pinfo)
{
	int32_t ret;

	if (unlikely(!pinfo)) {
		dpu_pr_err("pinfo is null");
		return -EINVAL;
	}

	pipeline_next_off(pinfo->base.peri_device, pinfo);
	pipeline_next_off(pinfo->base.peri_device, pinfo);
	ret = mipi_dsi_power_off(pinfo);
	if (ret != 0) {
		dpu_pr_err("mipi power off err\n");
		return -EINVAL;
	}

	pipeline_next_off(pinfo->base.peri_device, pinfo);
	return 0;
}

static int32_t mipi_dsi_fake_power_off(struct dpu_connector *connector, const void *value)
{
	int32_t ret;
	struct dkmd_connector_info *pinfo = connector->conn_info;
	void_unused(value);

	(void)pipeline_next_ops_handle(pinfo->base.peri_device, pinfo, FAKE_POWER_OFF, NULL);
	mipi_wait_ldi_vstate_idle(connector, NULL);
	ret = mipi_dsi_power_off(pinfo);
	if (ret != 0) {
		dpu_pr_err("mipi power off err\n");
		return -EINVAL;
	}

	return 0;
}

static int32_t mipi_dsi_set_fastboot(struct dpu_connector *connector, const void *value)
{
	uint32_t fastboot_enable = *(uint32_t *)value;
	char __iomem *mipi_dsi_base = connector->connector_base;
	struct dkmd_connector_info *pinfo = connector->conn_info;

	dpu_pr_info("fastboot_enable: %d ++++++", fastboot_enable);

	if (!pinfo)
		return -EINVAL;

	mipi_dsi_dphy_fastboot_fpga(pinfo);
	mipi_dsi_clk_enable(connector);

	/* disable mipi ldi */
	if (is_mipi_cmd_panel(&pinfo->base))
		pinfo->disable_ldi(pinfo);

	/* need enable this in the scenario of fastboot continuous display */
	if (pinfo->base.hs_pkt_discontin_support) {
		mipi_dsi_enable_hs_pkt_discontin_ctrl(mipi_dsi_base);

		if (connector->bind_connector != NULL) {
			mipi_dsi_base = connector->bind_connector->connector_base;
			mipi_dsi_enable_hs_pkt_discontin_ctrl(mipi_dsi_base);
		}
	}

	(void)pipeline_next_ops_handle(pinfo->base.peri_device, pinfo, SET_FASTBOOT, connector->connector_base);

	return 0;
}

static uint32_t get_stopstate_mask_value(uint8_t lane_nums)
{
	uint32_t stopstate_msk;

	if (lane_nums == DSI_4_LANES)
		stopstate_msk = BIT(0);
	else if (lane_nums == DSI_3_LANES)
		stopstate_msk = BIT(0) | BIT(4);
	else if (lane_nums == DSI_2_LANES)
		stopstate_msk = BIT(0) | BIT(3) | BIT(4);
	else
		stopstate_msk = BIT(0) | BIT(2) | BIT(3) | BIT(4);

	return stopstate_msk;
}

static void mipi_dsi_set_reg(struct dkmd_connector_info *pinfo, uint32_t offset, uint32_t val, uint8_t bw, uint8_t bs)
{
	struct dpu_connector *primay_connector = get_primary_connector(pinfo);

	set_reg(primay_connector->connector_base + offset, val, bw, bs);
	if (primay_connector->bind_connector)
		set_reg(primay_connector->bind_connector->connector_base + offset, val, bw, bs);
}

static uint8_t get_mipi_dsi_lane_num(struct dkmd_connector_info *pinfo)
{
	struct dpu_connector *primay_connector = get_primary_connector(pinfo);
	return primay_connector->mipi.lane_nums;
}

static int32_t mipi_dsi_set_clear(struct dpu_connector *connector, const void *value)
{
	bool valid_status = false;
	uint32_t stopstate_msk = 0;
	struct dkmd_connector_info *pinfo = connector->conn_info;

	if (is_mipi_cmd_panel(&pinfo->base) && (g_ldi_data_gate_en != 0))
		mipi_dsi_set_reg(pinfo, DPU_DSI_FRM_VALID_DBG_ADDR(DSI_ADDR_TO_OFFSET), 1, 1, 29);

	/* 5. delay vfp+1, config stopstate_enable and stopstate_mask */
	stopstate_msk = get_stopstate_mask_value(get_mipi_dsi_lane_num(pinfo));
	mipi_dsi_set_reg(pinfo, DPU_DSI_DPHYTX_CTRL_ADDR(DSI_ADDR_TO_OFFSET), 1, 1, 0);
	mipi_dsi_set_reg(pinfo, DPU_DSI_DPHYTX_CTRL_ADDR(DSI_ADDR_TO_OFFSET), stopstate_msk, 5, 3);

	/* 6. wait and check dsi dphytx_trstop_flag */
	valid_status = check_addr_status_is_valid(
		DPU_DSI_DPHYTX_TRSTOP_FLAG_ADDR(connector->connector_base), 0b01, 5, 10); /* timeout: 50us */
	if (connector->bind_connector != NULL)
		valid_status = valid_status && check_addr_status_is_valid(
			DPU_DSI_DPHYTX_TRSTOP_FLAG_ADDR(connector->bind_connector->connector_base), 0b01, 5, 10); /* timeout: 50us */

	if (!valid_status)
		dpu_pr_warn("check dphytx trstop flag failed!");

	/* 7. config dsi core shutdownz */
	mipi_dsi_set_reg(pinfo, DPU_DSI_POR_CTRL_ADDR(DSI_ADDR_TO_OFFSET), 0x0, 1, 0);
	/* timing constraint */
	udelay(5);
	mipi_dsi_set_reg(pinfo, DPU_DSI_POR_CTRL_ADDR(DSI_ADDR_TO_OFFSET), 0x1, 1, 0);

	/* 8. clear dsi core shutdownz */
	mipi_dsi_set_reg(pinfo, DPU_DSI_DPHYTX_CTRL_ADDR(DSI_ADDR_TO_OFFSET), 0, 1, 0);
	mipi_dsi_set_reg(pinfo, DPU_DSI_DPHYTX_CTRL_ADDR(DSI_ADDR_TO_OFFSET), 0, 5, 3);

	/* clear GLB DSS_RS_CLEAR */
	outp32(DPU_DSI_CPU_ITF_INTS_ADDR(connector->connector_base), ~0);

	if (is_mipi_video_panel(&pinfo->base))
		pinfo->enable_ldi(pinfo);

	/* 9. wait vactive intr and enable frm_valid */
	if (is_mipi_cmd_panel(&pinfo->base) && (g_ldi_data_gate_en != 0))
		mipi_dsi_set_reg(pinfo, DPU_DSI_FRM_VALID_DBG_ADDR(DSI_ADDR_TO_OFFSET), 0, 1, 29);

	return 0;
}

static int32_t mipi_dsi_isr_enable(struct dpu_connector *connector, const void *value)
{
	uint32_t mask = ~0;
	struct dkmd_isr *isr_ctrl = (struct dkmd_isr *)value;

	/* 1. interrupt mask */
	outp32(DPU_DSI_CPU_ITF_INT_MSK_ADDR(connector->connector_base), mask);

	/* 2. enable irq */
	isr_ctrl->handle_func(isr_ctrl, DKMD_ISR_ENABLE);

	/* 3. interrupt clear */
	outp32(DPU_DSI_CPU_ITF_INTS_ADDR(connector->connector_base), mask);

	/* 4. interrupt umask */
	outp32(DPU_DSI_CPU_ITF_INT_MSK_ADDR(connector->connector_base), isr_ctrl->unmask);

	if (is_mipi_cmd_panel(&connector->conn_info->base) && (g_ldi_data_gate_en == 0))
		set_reg(DPU_DSI_FRM_VALID_DBG_ADDR(connector->connector_base), 1, 1, 29);

	return 0;
}

static int32_t mipi_dsi_isr_disable(struct dpu_connector *connector, const void *value)
{
	uint32_t mask = ~0;
	struct dkmd_isr *isr_ctrl = (struct dkmd_isr *)value;

	/* 1. interrupt mask */
	outp32(DPU_DSI_CPU_ITF_INT_MSK_ADDR(connector->connector_base), mask);

	/* 2. disable irq */
	isr_ctrl->handle_func(isr_ctrl, DKMD_ISR_DISABLE);

	return 0;
}

static irqreturn_t dpu_dsi_isr(int32_t irq, void *ptr)
{
	int32_t diff = 0;
	uint32_t isr_s2, mask, vsync_bit;
	uint32_t dsi_te_id;
	struct dpu_connector *connector = NULL;
	struct dkmd_isr *isr_ctrl = (struct dkmd_isr *)ptr;
	static ktime_t vactive_start_timestamp[CONNECTOR_ID_MAX] = { 0 };
	static ktime_t vactive_end_timestamp[CONNECTOR_ID_MAX] = { 0 };

	dpu_check_and_return(!isr_ctrl, IRQ_NONE, err, "isr_ctrl is null!");

	connector = (struct dpu_connector *)isr_ctrl->parent;
	dpu_check_and_return(!connector, IRQ_NONE, err, "connector is null!");

	isr_s2 = inp32(DPU_DSI_CPU_ITF_INTS_ADDR(connector->connector_base));
	outp32(DPU_DSI_CPU_ITF_INTS_ADDR(connector->connector_base), isr_s2);
	isr_s2 &= ~((uint32_t)inp32(DPU_DSI_CPU_ITF_INT_MSK_ADDR(connector->connector_base)));

	dsi_te_id = connector->conn_info->base.lcd_te_idx == 0 ? DSI_INT_LCD_TE0 : DSI_INT_LCD_TE1;
	vsync_bit = (is_mipi_cmd_panel(&connector->conn_info->base) &&
		(!is_force_update(&connector->conn_info->base))) ? dsi_te_id : DSI_INT_VSYNC;
	if ((isr_s2 & vsync_bit) == vsync_bit) {
		dpu_pr_debug("get te itr notify, isr_s2: %#x, connector_id is %d", isr_s2, connector->connector_id);
		if (connector->is_vactive_end_recieved == VACTIVE_END_WAIT) {
			dpu_pr_warn("NOTICE: do not receive vactive end itr of last frame, conn_id = %u", connector->connector_id);
			connector->is_vactive_end_recieved = VACTIVE_END_MISS_REPORTED;
			if (g_debug_dmd_report_vact_end_miss == 1)
				dksm_dmd_report_vactive_end_miss(connector->connector_id);
		}
		dkmd_isr_notify_listener(isr_ctrl, vsync_bit);
	}

	if ((isr_s2 & DSI_INT_VACT0_START) == DSI_INT_VACT0_START) {
		dpu_print_timestamp("vactive_start");
		vactive_start_timestamp[connector->connector_id] = ktime_get();
		dpu_pr_debug("get vactive0 start itr notify, isr_s2: %#x, conn_id = %u",
			isr_s2, connector->connector_id);
		connector->is_vactive_end_recieved = VACTIVE_END_WAIT;
		dkmd_isr_notify_listener(isr_ctrl, DSI_INT_VACT0_START);
		outp32(DPU_DSI_CPU_ITF_INT_MSK_ADDR(connector->connector_base), isr_ctrl->unmask);
	}

	if ((isr_s2 & DSI_INT_VACT0_END) == DSI_INT_VACT0_END) {
		dpu_pr_debug("get vactive0 end itr notify, isr_s2: %#x, conn_id = %u",
			isr_s2, connector->connector_id);
		dpu_print_timestamp("vactive_end");
		vactive_end_timestamp[connector->connector_id] = ktime_get();

		diff = (int32_t)ktime_us_delta(vactive_end_timestamp[connector->connector_id],
			vactive_start_timestamp[connector->connector_id]);
		if (diff < 1000 && diff > -1000)
			dpu_pr_warn("pls check vactive_start and vactive_end: %d us, conn_id = %u", diff, connector->connector_id);

		connector->is_vactive_end_recieved = VACTIVE_END_RECEIVED;
		dkmd_isr_notify_listener(isr_ctrl, DSI_INT_VACT0_END);
	}

	if ((isr_s2 & DSI_INT_FRM_END) == DSI_INT_FRM_END) {
		dpu_pr_debug("get frame end itr notify, isr_s2: %#x, conn_id = %u",
			isr_s2, connector->connector_id);
		dkmd_isr_notify_listener(isr_ctrl, DSI_INT_FRM_END);
	}

	if ((isr_s2 & DSI_INT_FRM_START) == DSI_INT_FRM_START) {
		dpu_print_timestamp("frm_start");
		dpu_pr_debug("get frame start itr notify, isr_s2: %#x, conn_id = %u", isr_s2, connector->connector_id);
		dkmd_isr_notify_listener(isr_ctrl, DSI_INT_FRM_START);
	}

	if ((isr_s2 & DSI_INT_UNDER_FLOW) == DSI_INT_UNDER_FLOW || g_debug_underflow_fail == 1) {
		mask = (uint32_t)inp32(DPU_DSI_CPU_ITF_INT_MSK_ADDR(connector->connector_base));
		mask |= DSI_INT_UNDER_FLOW;
		outp32(DPU_DSI_CPU_ITF_INT_MSK_ADDR(connector->connector_base), mask);

		dpu_pr_warn("get underflow itr notify, isr_s2: %#x, conn_id = %u", isr_s2, connector->connector_id);
		dpu_print_timestamp("underflow");
		connector->is_vactive_end_recieved = VACTIVE_END_RECEIVED;
		dkmd_isr_notify_listener(isr_ctrl, DSI_INT_UNDER_FLOW);
	}

	return IRQ_HANDLED;
}

static int32_t mipi_dsi_isr_setup(struct dpu_connector *connector, const void *value)
{
	uint32_t vsync_bit = 0;
	uint32_t frm_start_bit = 0;
	uint32_t dsi_te_id;
	struct dkmd_isr *isr_ctrl = (struct dkmd_isr *)value;
	struct dfr_info *dfr_info = NULL;

	if (unlikely((!connector) || (!value))) {
		dpu_pr_err("connector or value is null\n");
		return -1;
	}

	dfr_info = dkmd_get_dfr_info(connector->conn_info);
	(void)snprintf(isr_ctrl->irq_name, sizeof(isr_ctrl->irq_name), "irq_dsi_%u", connector->connector_id);
	isr_ctrl->irq_no = connector->connector_irq;
	isr_ctrl->isr_fnc = dpu_dsi_isr;
	isr_ctrl->parent = connector;

	dsi_te_id = connector->conn_info->base.lcd_te_idx == 0 ? DSI_INT_LCD_TE0 : DSI_INT_LCD_TE1;
	vsync_bit = (is_mipi_cmd_panel(&connector->conn_info->base) &&
		(!is_force_update(&connector->conn_info->base))) ? dsi_te_id : DSI_INT_VSYNC;

	if (dpu_config_enable_pmctrl_dvfs())
		frm_start_bit = DSI_INT_FRM_START;

	/* mcu control frame rate, acpu don't need receive te interrupt */
	if (dfr_info && (dfr_info->dfr_mode == DFR_MODE_TE_SKIP_BY_MCU ||
		dfr_info->dfr_mode == DFR_MODE_LONGH_TE_SKIP_BY_MCU))
		isr_ctrl->unmask = ~(DSI_INT_VACT0_START | DSI_INT_VACT0_END | DSI_INT_FRM_END |
			DSI_INT_UNDER_FLOW | frm_start_bit);
	else
		isr_ctrl->unmask = ~(vsync_bit | DSI_INT_VACT0_START | DSI_INT_VACT0_END |
			DSI_INT_FRM_END | DSI_INT_UNDER_FLOW);

	if (is_ppc_support(&connector->conn_info->base))
		isr_ctrl->unmask &= ~DSI_INT_FRM_START;

	return 0;
}

static int32_t mipi_dsc_init(struct dpu_connector *connector, const void *value)
{
	(void)value;
	dsc_init(&connector->dsc, connector->dsc_base);
	return 0;
}

static int32_t mipi_spr_init(struct dpu_connector *connector, const void *value)
{
	(void)value;
	spr_init(&connector->spr, connector->dpp_base, connector->dsc_base);
	return 0;
}

static int32_t mipi_dsi_ulps_handle(struct dpu_connector *connector, const void *value)
{
	mipi_dsi_ulps_cfg(connector, *(bool *)value);
	return 0;
}

static int32_t mipi_dcs_cmd_tx_for_platform(struct dpu_connector *connector, const void *value)
{
	char __iomem *dpu_base = connector->connector_base;
	struct dsi_cmds *ddic_cmds = NULL;
	int32_t idle_cycle;
	if (!dpu_base || !value) {
		dpu_pr_err("dpu_base or value is null, error\n");
		return -1;
	}

	ddic_cmds = (struct dsi_cmds *)value;
	if (connector->bind_connector)
		return mipi_dual_dsi_cmds_tx(ddic_cmds->cmds, (int32_t)ddic_cmds->cmd_num,
			connector->bind_connector->connector_base,
			ddic_cmds->cmds, (int32_t)ddic_cmds->cmd_num, dpu_base, EN_DSI_TX_NORMAL_MODE, true);
	idle_cycle = mipi_dsi_fifo_idle_cycle(dpu_base);
	if (idle_cycle >= 0) {
		if (idle_cycle != 0)
			dpu_pr_info("single mipi fifo idle cycle %d\n", idle_cycle);
		return mipi_dsi_cmds_tx(ddic_cmds->cmds, (int32_t)ddic_cmds->cmd_num, dpu_base);
	}
	return 0;
}

static struct connector_ops_handle_data dsi_ops_table[] = {
	{ SET_FASTBOOT, mipi_dsi_set_fastboot },
	{ DO_CLEAR, mipi_dsi_set_clear },
	{ SETUP_ISR, mipi_dsi_isr_setup },
	{ ENABLE_ISR, mipi_dsi_isr_enable },
	{ DISABLE_ISR, mipi_dsi_isr_disable },
	{ INIT_SPR, mipi_spr_init },
	{ INIT_DSC, mipi_dsc_init },
	{ HANDLE_MIPI_ULPS, mipi_dsi_ulps_handle },
	{ MIPI_DSI_PARTIAL_UPDATE, mipi_dsi_partial_update },
	{ MIPI_DSI_PPC_SET_REG, mipi_dsi_panel_partial_ctrl_set_reg },
	{ RESET_PARTIAL_UPDATE, mipi_dsi_reset_partial_update},
	{ FAKE_POWER_OFF, mipi_dsi_fake_power_off },
	{ DCS_CMD_TX_FOR_PLATFORM, mipi_dcs_cmd_tx_for_platform},
	{ MIPI_DSI_BIT_CLK_UPT, mipi_dsi_bit_clk_upt },
	{ WAIT_LDI_VSTATE_IDLE, mipi_wait_ldi_vstate_idle },
};

static int32_t mipi_dsi_ops_handle(struct dkmd_connector_info *pinfo, uint32_t ops_cmd_id, void *value)
{
	struct dpu_connector *connector = NULL;

	dpu_pr_debug("ops_cmd_id = %d!", ops_cmd_id);
	dpu_check_and_return(!pinfo, -EINVAL, err, "pinfo is NULL!");
	connector = get_primary_connector(pinfo);
	dpu_check_and_return(!connector, -EINVAL, err, "connector is NULL!");

	if (dkdm_connector_hanlde_func(dsi_ops_table, ARRAY_SIZE(dsi_ops_table), ops_cmd_id, connector, value) != 0)
		return pipeline_next_ops_handle(pinfo->base.peri_device, pinfo, ops_cmd_id, value);

	return 0;
}

static void enable_dsi_ldi(struct dkmd_connector_info *pinfo)
{
	struct dpu_connector *connector = get_primary_connector(pinfo);

	/* issued instructions: single_frm_update */
	set_reg(DPU_DSI_LDI_FRM_MSK_UP_ADDR(connector->connector_base), 0x1, 1, 0);
	if (connector->bind_connector)
		set_reg(DPU_DSI_LDI_FRM_MSK_UP_ADDR(connector->bind_connector->connector_base), 0x1, 1, 0);

	if (connector->bind_connector)
		set_reg(DPU_DSI_LDI_CTRL_ADDR(connector->connector_base), 0x1, 1, 5);
	else
		set_reg(DPU_DSI_LDI_CTRL_ADDR(connector->connector_base), 0x1, 1, 0);
}

static void disable_dsi_ldi(struct dkmd_connector_info *pinfo)
{
	struct dpu_connector *connector = get_primary_connector(pinfo);
	if (connector->bind_connector)
		set_reg(DPU_DSI_LDI_CTRL_ADDR(connector->connector_base), 0x0, 1, 5);
	else
		set_reg(DPU_DSI_LDI_CTRL_ADDR(connector->connector_base), 0x0, 1, 0);

	if (is_force_update(&pinfo->base)) {
		/* force update need ldi enable */
		set_reg(DPU_DSI_LDI_CTRL_ADDR(connector->connector_base), 0x1, 1, 0);
		dpu_pr_info("[vsync] enable next frame delay!!");
	}
}

static int32_t check_dsi_ldi_status(struct dkmd_connector_info *pinfo)
{
	uint32_t vstate0, vstate1;
	struct dpu_connector *connector = get_primary_connector(pinfo);
	if (!connector->bind_connector)
		return 0;

	vstate0 = inp32(DPU_DSI_VSTATE_ADDR(connector->connector_base));
	vstate1 = inp32(DPU_DSI_VSTATE_ADDR(connector->bind_connector->connector_base));
	if ((vstate0 == LDI_VSTATE_VACTIVE0 && vstate1 == LDI_VSTATE_V_WAIT_TE0) ||
		(vstate0 == LDI_VSTATE_V_WAIT_TE0 && vstate1 == LDI_VSTATE_VACTIVE0)) {
		dpu_pr_err("dual mipi, vstate mismatch[0x%x, 0x%x]", vstate0, vstate1);
		return -1;
	}

	return 0;
}

void mipi_dsi_default_setup(struct dpu_connector *connector)
{
	if (mipi_dsi_is_bypass_by_pg(connector->connector_id)) {
		dpu_pr_warn("mipi dsi bypass by pg config");
		return;
	}
	mipi_transfer_lock_init();

	connector->on_func = mipi_dsi_on;
	connector->off_func = mipi_dsi_off;
	connector->ops_handle_func = mipi_dsi_ops_handle;

	connector->enable_ser_vp_sync = true;
	connector->conn_info->enable_ldi = enable_dsi_ldi;
	connector->conn_info->disable_ldi = disable_dsi_ldi;
	connector->conn_info->check_ldi_status = check_dsi_ldi_status;
}
