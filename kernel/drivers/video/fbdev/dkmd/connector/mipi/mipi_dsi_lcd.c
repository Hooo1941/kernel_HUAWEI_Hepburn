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
#include <linux/spinlock.h>
#include <linux/module.h>
#include <dpu/soc_dpu_define.h>

#include "peri/dkmd_peri.h"
#include "dkmd_mipi_panel_info.h"
#include "dkmd_lcd_interface.h"
#include "dpu_conn_mgr.h"
#include "dkmd_comp.h"

static bool check_mipi_status(struct dpu_connector *connector)
{
	struct composer *comp = NULL;

	if (!connector->conn_info || !connector->conn_info->base.comp_obj_info) {
		dpu_pr_err("pinfo or comp_obj_info is nullptr!");
		return false;
	}
	comp = container_of(connector->conn_info->base.comp_obj_info, struct composer, base);
	if (!comp) {
		dpu_pr_err("comp is nullptr!");
		return false;
	}

	return true;
}

static struct dpu_connector *get_real_connector(uint32_t raw_dsi_index, uint32_t panel_type)
{
	uint32_t real_dsi_idx;

	if (raw_dsi_index != CONNECTOR_ID_DSI0)
		real_dsi_idx = raw_dsi_index;
	else
		/* bit12: SECONDARY_PANEL_CMD_TYPE from lcdkit */
		real_dsi_idx = (((panel_type & PANEL_EXTERNAL) != 0) ? CONNECTOR_ID_DSI0_BUILTIN : CONNECTOR_ID_DSI0);

	return get_connector_by_id(real_dsi_idx);
}

static void mipi_dsi_tx_lp_mode_cfg(char __iomem *dsi_base)
{
	/*
	 * gen short cmd read switch low-power,
	 * include 0-parameter,1-parameter,2-parameter
	 */
	set_reg(DPU_DSI_CMD_MODE_CTRL_ADDR(dsi_base), 0x7, 3, 8);
	/* gen long cmd write switch low-power */
	set_reg(DPU_DSI_CMD_MODE_CTRL_ADDR(dsi_base), 0x1, 1, 14);
	/*
	 * dcs short cmd write switch high-speed,
	 * include 0-parameter,1-parameter
	 */
	set_reg(DPU_DSI_CMD_MODE_CTRL_ADDR(dsi_base), 0x3, 2, 16);
}

static void mipi_dsi_rx_lp_mode_cfg(char __iomem *dsi_base)
{
	/*
	 * gen short cmd read switch low-power,
	 * include 0-parameter,1-parameter,2-parameter
	 */
	set_reg(DPU_DSI_CMD_MODE_CTRL_ADDR(dsi_base), 0x7, 3, 11);
	/* dcs short cmd read switch low-power */
	set_reg(DPU_DSI_CMD_MODE_CTRL_ADDR(dsi_base), 0x1, 1, 18);
	/* read packet size cmd switch low-power */
	set_reg(DPU_DSI_CMD_MODE_CTRL_ADDR(dsi_base), 0x1, 1, 24);
}

static void mipi_dsi_tx_hs_mode_cfg(char __iomem *dsi_base)
{
	/*
	 * gen short cmd read switch low-power,
	 * include 0-parameter,1-parameter,2-parameter
	 */
	set_reg(DPU_DSI_CMD_MODE_CTRL_ADDR(dsi_base), 0x0, 3, 8);
	/* gen long cmd write switch high-speed */
	set_reg(DPU_DSI_CMD_MODE_CTRL_ADDR(dsi_base), 0x0, 1, 14);
	/*
	 * dcs short cmd write switch high-speed,
	 * include 0-parameter,1-parameter
	 */
	set_reg(DPU_DSI_CMD_MODE_CTRL_ADDR(dsi_base), 0x0, 2, 16);
}

static void mipi_dsi_rx_hs_mode_cfg(char __iomem *dsi_base)
{
	/*
	 * gen short cmd read switch high-speed,
	 * include 0-parameter,1-parameter,2-parameter
	 */
	set_reg(DPU_DSI_CMD_MODE_CTRL_ADDR(dsi_base), 0x0, 3, 11);
	/* dcs short cmd read switch high-speed */
	set_reg(DPU_DSI_CMD_MODE_CTRL_ADDR(dsi_base), 0x0, 1, 18);
	/* read packet size cmd switch high-speed */
	set_reg(DPU_DSI_CMD_MODE_CTRL_ADDR(dsi_base), 0x0, 1, 24);
}

void mipi_dsi_set_lp_mode(uint32_t dsi_idx, uint32_t panel_type)
{
	struct dpu_connector *connector = NULL;

	connector = get_real_connector(dsi_idx, panel_type);
	if (!connector) {
		dpu_pr_err("connector_id=%u is not available!\n", dsi_idx);
		return;
	}
	if (!check_mipi_status(connector))
		return;
	composer_active_vsync(connector->conn_info, true);
	mipi_dsi_tx_lp_mode_cfg(connector->connector_base);
	mipi_dsi_rx_lp_mode_cfg(connector->connector_base);
	composer_active_vsync(connector->conn_info, false);
}

void mipi_dsi_set_hs_mode(uint32_t dsi_idx, uint32_t panel_type)
{
	struct dpu_connector *connector = NULL;

	connector = get_real_connector(dsi_idx, panel_type);
	if (!connector) {
		dpu_pr_err("connector_id=%u is not available!\n", dsi_idx);
		return;
	}
	if (!check_mipi_status(connector))
		return;
	composer_active_vsync(connector->conn_info, true);
	mipi_dsi_tx_hs_mode_cfg(connector->connector_base);
	mipi_dsi_rx_hs_mode_cfg(connector->connector_base);
	composer_active_vsync(connector->conn_info, false);
}

int32_t mipi_dsi_cmds_tx_for_usr(uint32_t dsi_idx, struct dsi_cmd_desc *cmds, int32_t cnt, uint32_t panel_type)
{
	int32_t ret = 0;
	struct dpu_connector *connector = NULL;

	if (!cmds) {
		dpu_pr_err("NULL cmds\n");
		return -1;
	}

	if (cnt <= 0 || cnt > TX_CMD_MAX) {
		dpu_pr_err("invalid cnt\n");
		return -1;
	}

	connector = get_real_connector(dsi_idx, panel_type);
	if (!connector) {
		dpu_pr_err("connector_id=%u is not available!\n", dsi_idx);
		return -1;
	}
	if (!check_mipi_status(connector))
		return -1;
	composer_active_vsync(connector->conn_info, true);
	if (mipi_dsi_fifo_is_full(connector->connector_base)) {
		dpu_pr_err("connector_id=%u fifo is full", dsi_idx);
		composer_active_vsync(connector->conn_info, false);
		return -1;
	}
	ret = mipi_dsi_cmds_tx(cmds, cnt, connector->connector_base);
	composer_active_vsync(connector->conn_info, false);

	return ret;
}

#define WAIT_MAX_TIME_FOR_ISR_SAFE 100
static int32_t check_cmd_wait_time_for_isr_safe(struct dsi_cmd_desc *cmds)
{
	if (!cmds) {
		dpu_pr_err("NULL cmds\n");
		return -1;
	}

	if (cmds->waittype == WAIT_TYPE_MS || cmds->wait > WAIT_MAX_TIME_FOR_ISR_SAFE)
		return -1;
	return 0;
}


/* avoid too long wait time or mutex lock to cause sleep */
int32_t mipi_dsi_cmds_tx_for_usr_isr_safe(uint32_t dsi_idx, struct dsi_cmd_desc *cmds, int32_t cnt, uint32_t panel_type)
{
	struct dpu_connector *connector = NULL;

	if (!cmds) {
		dpu_pr_err("NULL cmds\n");
		return -1;
	}

	if (cnt <= 0 || cnt > TX_CMD_MAX) {
		dpu_pr_err("invalid cnt\n");
		return -1;
	}

	connector = get_real_connector(dsi_idx, panel_type);
	if (!connector) {
		dpu_pr_err("connector_id=%u is not available!\n", dsi_idx);
		return -1;
	}
	if (!check_mipi_status(connector))
		return -1;

	if (check_cmd_wait_time_for_isr_safe(cmds) != 0) {
		dpu_pr_err("connector_id=%u wait too long for isr trans\n", dsi_idx);
		return -1;
	}

	return mipi_dsi_cmds_tx(cmds, cnt, connector->connector_base);
}

int32_t mipi_dual_dsi_cmds_tx_for_usr_isr_safe(uint32_t dsi_idx0, struct dsi_cmd_desc *cmds0,
	int32_t cnt0, uint32_t dsi_idx1, struct dsi_cmd_desc *cmds1, int32_t cnt1, uint8_t tx_mode, uint32_t panel_type)
{
	struct dpu_connector *connector0 = NULL;
	struct dpu_connector *connector1 = NULL;

	if (!cmds0 || !cmds1) {
		dpu_pr_err("invalid param\n");
		return -1;
	}

	if (cnt0 <= 0 || cnt0 > TX_CMD_MAX || cnt1 <= 0 || cnt1 > TX_CMD_MAX) {
		dpu_pr_err("invalid cnt\n");
		return -1;
	}

	connector0 = get_real_connector(dsi_idx0, panel_type);
	if ((!connector0) || (!connector0->connector_base)) {
		dpu_pr_err("connector_id0=%u is not available!\n", dsi_idx0);
		return -1;
	}
	connector1 = get_connector_by_id(dsi_idx1);
	if ((!connector1) || (!connector1->connector_base)) {
		dpu_pr_err("connector_id1=%u is not available!\n", dsi_idx1);
		return -1;
	}
	if (!check_mipi_status(connector0))
		return -1;

	if (check_cmd_wait_time_for_isr_safe(cmds0) != 0 ||
	    check_cmd_wait_time_for_isr_safe(cmds1) != 0) {
		dpu_pr_err("wait too long for isr trans\n");
		return -1;
	}

	return mipi_dual_dsi_cmds_tx(cmds0, cnt0,
		connector0->connector_base, cmds1, cnt1,
		connector1->connector_base, EN_DSI_TX_NORMAL_MODE, false);
}

static int32_t mipi_dsi_set_cmds_rx_common(uint8_t *out,
	int out_len, struct dsi_cmd_desc *cm, bool little_endian_support,
	char __iomem *dsi_base)
{
	int ret;
	int j;
	int cnt = 0;
	uint8_t tmp[BUF_MAX] = {0};
	uint32_t tmp_value[READ_MAX] = {0};

	if (unlikely(cm->dlen > BUF_MAX)) {
		dpu_pr_err("dlen = %u is invalid.", cm->dlen);
		return -1;
	}

	if (mipi_dsi_fifo_is_full(dsi_base)) {
		dpu_pr_err("mipi fifo is full, error\n");
		return -EINVAL;
	}
	if (mipi_dsi_cmd_is_write(cm)) {
		(void)mipi_dsi_cmds_tx(cm, 1, dsi_base);
	} else {
		ret = mipi_dsi_lread_reg(tmp_value, READ_MAX, cm, cm->dlen, dsi_base);
		if (ret) {
			dpu_pr_err("mipi read error\n");
			return ret;
		}
		ret = mipi_dsi_get_read_value(cm, tmp, tmp_value, (uint32_t)out_len, little_endian_support);
		if (ret) {
			dpu_pr_err("get read value error\n");
			return ret;
		}
	}

	if (!mipi_dsi_cmd_is_write(cm)) {
		for (j = 0; ((uint32_t)j < cm->dlen) && (cnt <= out_len); j++)
			out[cnt++] = tmp[j];
	}
	return 0;
}

int32_t mipi_dsi_cmds_rx_for_usr(uint32_t dsi_idx, uint8_t *out,
	int out_len, struct dsi_cmd_desc *cm, bool little_endian_support, uint32_t panel_type)
{
	int32_t ret = 0;
	struct dpu_connector *connector = get_real_connector(dsi_idx, panel_type);

	if (!out || !cm) {
		dpu_pr_err("invalid param\n");
		return -1;
	}

	if (out_len <= 0 ||  out_len > BUF_MAX) {
		dpu_pr_err("invalid out_len\n");
		return -1;
	}

	if (!connector) {
		dpu_pr_err("connector_id=%u is not available!\n", dsi_idx);
		return -1;
	}
	if (!check_mipi_status(connector))
		return -1;
	composer_active_vsync(connector->conn_info, true);
	ret = mipi_dsi_set_cmds_rx_common(out, out_len, cm,
		little_endian_support, connector->connector_base);
	composer_active_vsync(connector->conn_info, false);

	return ret;
}

int32_t mipi_dual_dsi_cmds_tx_for_usr(uint32_t dsi_idx0, struct dsi_cmd_desc *cmds0,
	int32_t cnt0, uint32_t dsi_idx1, struct dsi_cmd_desc *cmds1, int32_t cnt1, uint8_t tx_mode, uint32_t panel_type)
{
	int32_t ret = 0;
	struct dpu_connector *connector0 = NULL;
	struct dpu_connector *connector1 = NULL;

	if (!cmds0 || !cmds1) {
		dpu_pr_err("invalid param\n");
		return -1;
	}

	if (cnt0 <= 0 || cnt0 > TX_CMD_MAX || cnt1 <= 0 || cnt1 > TX_CMD_MAX) {
		dpu_pr_err("invalid cnt\n");
		return -1;
	}

	connector0 = get_real_connector(dsi_idx0, panel_type);
	if ((!connector0) || (!connector0->connector_base)) {
		dpu_pr_err("connector_id0=%u is not available!\n", dsi_idx0);
		return -1;
	}
	connector1 = get_real_connector(dsi_idx1, panel_type);
	if ((!connector1) || (!connector1->connector_base)) {
		dpu_pr_err("connector_id1=%u is not available!\n", dsi_idx1);
		return -1;
	}
	if (!check_mipi_status(connector0))
		return -1;
	composer_active_vsync(connector0->conn_info, true);
	ret = mipi_dual_dsi_cmds_tx(cmds0, cnt0,
		connector0->connector_base, cmds1, cnt1,
		connector1->connector_base, EN_DSI_TX_NORMAL_MODE, true);
	composer_active_vsync(connector0->conn_info, false);

	return ret;
}

int32_t mipi_dual_dsi_cmds_rx_for_usr(uint32_t dsi_idx0, uint8_t *dsi_out0, uint32_t dsi_idx1,
	uint8_t *dsi_out1, int out_len, struct dsi_cmd_desc *cm, bool little_endian_support, uint32_t panel_type)
{
	int32_t ret = 0;
	struct dpu_connector *connector0 = NULL;
	struct dpu_connector *connector1 = NULL;

	if (!dsi_out0 || !dsi_out1 || !cm) {
		dpu_pr_err("invalid param\n");
		return -1;
	}

	if (out_len <= 0 ||  out_len > BUF_MAX) {
		dpu_pr_err("invalid out_len\n");
		return -1;
	}

	connector0 = get_real_connector(dsi_idx0, panel_type);
	if (!connector0) {
		dpu_pr_err("connector_id0=%u is not available!\n", dsi_idx0);
		return -1;
	}

	connector1 = get_real_connector(dsi_idx1, panel_type);
	if (!connector1) {
		dpu_pr_err("connector_id1=%u is not available!\n", dsi_idx1);
		return -1;
	}
	if (!check_mipi_status(connector0))
		return -1;
	composer_active_vsync(connector0->conn_info, true);
	ret = mipi_dual_dsi_cmds_rx(connector0->connector_base, dsi_out0,
		connector1->connector_base, dsi_out1, out_len, cm, little_endian_support);
	composer_active_vsync(connector0->conn_info, false);

	return ret;
}
