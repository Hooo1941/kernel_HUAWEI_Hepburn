/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#include "psr_config_base.h"
#include "dp_core_interface.h"
#include "dp_aux.h"
#include "hidptx/hidptx_reg.h"
#include "hidptx_dp_core.h"
#include "dp_link_training.h"

void dptx_psr_intr_en(struct dp_ctrl *dptx)
{
	uint32_t reg;

	dpu_check_and_no_retval(!dptx, err, "[DP] dptx is NULL!");

	reg = dptx_readl(dptx, DPTX_INTR_ACPU_TIMING_GEN_ENABLE);
	reg |= DPTX_IRQ_ALL_PSR_INTR | DPTX_ADAPTIVE_SYNC_INTR | DPTX_PSR_ENTRY_INTR | DPTX_PSR_EXIT_INTR;
	dptx_writel(dptx, DPTX_INTR_ACPU_TIMING_GEN_ENABLE, reg);

	if (dptx->psr_params.psr_version == DP_PSR_IS_SUPPORTED)
		return;

	reg = dptx_readl(dptx, DPTX_INTR_ENABLE);
	reg |= (DPTX_SU_UPDATE | DPTX_PSR2_VACTIVE_START | DPTX_CFG_ALPM_ENTRY | DPTX_CFG_ALPM_EXIT);
	dptx_writel(dptx, DPTX_INTR_ENABLE, reg);
	dpu_pr_info("[DP] dptx_psr_intr_en set PSR intr\n");
}

int dptx_get_psr_cap(struct dp_ctrl *dptx)
{
	uint16_t psr2_su_xcoordinate;
	uint8_t byte;
	int retval;

	dpu_check_and_return((dptx == NULL), -EINVAL, err, "[DP] dptx is NULL!");

	retval = dptx_read_dpcd(dptx, DP_PSR_SUPPORT, &byte);
	if (retval) {
		dpu_pr_err("[DP] Failed to get psr version info %d", retval);
		return retval;
	}
	dptx->psr_params.psr_version = byte;
	if (byte < DP_PSR_IS_SUPPORTED || byte > DP_PSR2_WITH_Y_COORD_IS_SUPPORTED) {
		dpu_pr_info("[DP] Panel don't support PSR or version mismatch: %d", byte);
		dptx->psr_params.psr_version = INVALID_PSR_CAP;
		return 0;
	}

	retval = dptx_read_dpcd(dptx, DP_PSR_CAPS, &byte);
	if (retval) {
		dpu_pr_err("[DP] Failed to get psr cap info %d", retval);
		return retval;
	}

	dptx->psr_params.need_training = true;
	dptx->psr_params.ml_close_require = true;
	dptx->psr_params.setup_time = MAX_PSR_SETUP_TIME - ((byte & 0xF) >> DP_PSR_SETUP_TIME_SHIFT) * PSR_SETUP_TIME_STEP;

	if (byte & DP_PSR2_SU_Y_COORDINATE_REQUIRED)
		dptx->psr_params.ycoordinate_require = true;
	else
		dptx->psr_params.ycoordinate_require = false;

	if (byte & DP_PSR2_SU_GRANULARITY_REQUIRED)
		dptx->psr_params.su_require = true;
	else
		dptx->psr_params.su_require = false;

	if (dptx->psr_params.su_require) {
		psr2_su_xcoordinate = 0;
		retval = dptx_read_dpcd(dptx, DP_PSR2_SU_X_GRANULARITY, &byte);
		if (retval) {
			dpu_pr_err("[DP] Failed to get psr x granularity info %d", retval);
			return retval;
		}
		psr2_su_xcoordinate = byte;
		retval = dptx_read_dpcd(dptx, DP_PSR2_SU_X_GRANULARITY + 1, &byte);
		if (retval) {
			dpu_pr_err("[DP] Failed to get psr x granularity info %d", retval);
			return retval;
		}
		psr2_su_xcoordinate |= (byte << 8);
		dptx->psr_params.x_granularity = psr2_su_xcoordinate;

		retval = dptx_read_dpcd(dptx, DP_PSR2_SU_Y_GRANULARITY, &byte);
		if (retval) {
			dpu_pr_err("[DP] Failed to get psr y granularity info %d", retval);
			return retval;
		}
		if (byte)
			dptx->psr_params.y_granularity = byte;
		else
			dptx->psr_params.y_granularity = MIN_PSR_HEIGHT_GRANULARITY;
	}

	retval = dptx_read_dpcd(dptx, DP_PSR_EN_CFG, &byte);
	if (retval) {
		dpu_pr_err("[DP] Failed to read dpcd %d", retval);
		return retval;
	}
	byte |= DP_PSR_ENABLE;
	if (!dptx->psr_params.ml_close_require)
		byte |= DP_PSR_MAIN_LINK_ACTIVE;
	retval = dptx_write_dpcd(dptx, DP_PSR_EN_CFG, byte);
	if (retval) {
		dpu_pr_err("[DP] Failed to write dpcd %d", retval);
		return retval;
	}

	return 0;
}

static void dptx_psr_sdp_config(struct dp_ctrl *dptx)
{
	uint32_t reg;

	reg = dptx_readl(dptx, DPTX_GCTL0);
	reg |= DPTX_CFG_SDP_ENABLE;
	dptx_writel(dptx, DPTX_GCTL0, reg);

	reg = dptx_readl(dptx, DPTX_PSR_CTRL);
	reg &= ~DPTX_CFG_PSR_SDP_HB2_MASK;
	reg &= ~DPTX_CFG_PSR_SDP_HB3_MASK;

	if (dptx->psr_params.psr_version == DP_PSR_IS_SUPPORTED) {
		reg |= (DPTX_PSR_SDP_HB2_VALUE << DPTX_CFG_PSR_SDP_HB2_SHIFT);
		reg |= (DPTX_PSR_SDP_HB3_VALUE << DPTX_CFG_PSR_SDP_HB3_SHIFT);
	} else {
		if (dptx->psr_params.ycoordinate_require) {
			reg |= (DPTX_PSR2_YVAL_SDP_HB3_VALUE << DPTX_CFG_PSR_SDP_HB3_SHIFT);
			reg |= (DPTX_PSR2_YVAL_SDP_HB2_VALUE << DPTX_CFG_PSR_SDP_HB2_SHIFT);
			reg |= DPTX_CFG_SU_Y_COO_VALID;
		} else {
			reg |= (DPTX_PSR2_SDP_HB3_VALUE << DPTX_CFG_PSR_SDP_HB3_SHIFT);
			reg |= (DPTX_PSR2_SDP_HB2_VALUE << DPTX_CFG_PSR_SDP_HB2_SHIFT);
			reg &= ~DPTX_CFG_SU_Y_COO_VALID;
		}
	}
	dptx_writel(dptx, DPTX_PSR_CTRL, reg);
}

int dptx_psr_ml_config(struct dp_ctrl *dptx, int status)
{
	uint32_t reg;
	int retval;
	uint8_t lanes = dptx->link.lanes;

	/* entry electrical idle */
	reg = dptx_readl(dptx, DPTX_PHYIF_CTRL0);
	reg &= ~DPTX_LANE_DATA_EN_MASK;
	dptx_writel(dptx, DPTX_PHYIF_CTRL0, reg);

	reg = dptx_readl(dptx, DPTX_PHYIF_CTRL3);
	switch (lanes) {
	case 4:
		reg &= ~(DPTX_LANE3_PHY_STATUS_MASK | DPTX_LANE2_PHY_STATUS_MASK);
		reg |= (((uint32_t)status << DPTX_LANE3_PHY_STATUS_SHIFT) |
			((uint32_t)status << DPTX_LANE2_PHY_STATUS_SHIFT));
	case 2:
		reg &= ~DPTX_LANE1_PHY_STATUS_MASK;
		reg |= (uint32_t)status << DPTX_LANE1_PHY_STATUS_SHIFT;
	case 1:
		reg &= ~DPTX_LANE0_PHY_STATUS_MASK;
		reg |= (uint32_t)status;
		break;
	default:
		dpu_pr_err("[DP] Invalid number of lanes %d", lanes);
		return -EINVAL;
	}
	dptx_writel(dptx, DPTX_PHYIF_CTRL3, reg);

	retval = dptx_phy_wait_busy(dptx, dptx->link.lanes);
	if (retval != 0) {
		dpu_pr_err("[DP] Timed out waiting for PHY BUSY");
		return retval;
	}

	return 0;
}

static void dptx_psr_exit_manage(struct dp_ctrl *dptx)
{
	uint32_t reg;
	uint8_t byte;
	int retval;
	uint8_t lanes = dptx->link.lanes;;

	reg = dptx_readl(dptx, DPTX_PHYIF_CTRL0);
	reg &= ~DPTX_PATTERN_SEL_MASK;
	reg |= (DPTX_TPS1_PATTERN << DPTX_PATTERN_SEL_SHIFT);
	dptx_writel(dptx, DPTX_PHYIF_CTRL0, reg);

	/* exit electrical idle */
	reg = dptx_readl(dptx, DPTX_PHYIF_CTRL0);
	reg &= ~DPTX_LANE_DATA_EN_MASK;
	reg |= cfg_phy_lanes(lanes) << DPTX_LANE_DATA_EN_SHIFT;
	dptx_writel(dptx, DPTX_PHYIF_CTRL0, reg);

	retval = dptx_read_dpcd(dptx, DP_SET_POWER, &byte);
	if (retval) {
		dpu_pr_err("[DP] Read DPCD error");
		return;
	}
	byte &= ~GENMASK(2, 0);
	byte |= DP_SET_POWER_D0;
	retval = dptx_write_dpcd(dptx, DP_SET_POWER, byte);
	if (retval) {
		dpu_pr_err("[DP] Write DPCD error");
		return;
	}
}

static uint32_t dptx_time_per_line_calc(struct dp_ctrl *dptx)
{
	uint32_t index = 0;
	uint32_t count = 2;
	uint32_t vtotal = dptx->vparams.mdtd.v_blanking + dptx->vparams.mdtd.v_active;
	dpu_pr_info("[DP] dptx_time_per_line_calc v_blanking:%d, v_active:%d",
		dptx->vparams.mdtd.v_blanking, dptx->vparams.mdtd.v_active);

	while (count < vtotal) {
		count *= 2;
		index++;
	}

	return index;
}

static void dptx_psr2_aux_frame_sync_disable(struct dp_ctrl *dptx)
{
	uint32_t reg;
	uint32_t div;

	div = dptx_time_per_line_calc(dptx);
	reg = dptx_readl(dptx, DPTX_GTC_CONTROL2);
	reg &= ~DPTX_CFG_GTC_LINE_LEN_DIV_MASK;
	reg |= (div & 0xF);
	dptx_writel(dptx, DPTX_GTC_CONTROL2, reg);

	reg = dptx_readl(dptx, DPTX_GTC_CONTROL1);
	reg &= ~(DPTX_CFG_GTC_ENABLE | DPTX_CFG_AUX_FRAME_SYNC_ENABLE);
	reg |= (DPTX_CFG_NO_AUX_FRAME_SYNC_ENABLE | DPTX_CFG_GTC_LINE_CNT_MODE_SEL);
	dptx_writel(dptx, DPTX_GTC_CONTROL1, reg);
}

static int dptx_psr_set_capture_indication(struct dp_ctrl *dptx)
{
	int retval;
	uint8_t byte;
	uint32_t reg;
	uint32_t htotal_time;
	uint32_t vblank_time;
	uint32_t hblank_time;

	if (unlikely(dptx->vparams.mdtd.pixel_clock == 0)) {
		dpu_pr_err("[DP] Illegal pixel_clock value");
		return -EINVAL;
	}
	htotal_time = (uint32_t)(1000 * (dptx->vparams.mdtd.h_active + dptx->vparams.mdtd.h_blanking) /
		dptx->vparams.mdtd.pixel_clock);
	vblank_time = dptx->vparams.mdtd.v_blanking * htotal_time;
	retval = dptx_read_dpcd(dptx, DP_PSR_EN_CFG, &byte);
	if (retval) {
		dpu_pr_err("[DP] Read DPCD error");
		return retval;
	}
	reg = dptx_readl(dptx, DPTX_PSR_CTRL);
	if (vblank_time > dptx->psr_params.setup_time) {
		byte &= ~DP_PSR_FRAME_CAPTURE;
		reg &= ~DPTX_CFG_FRAME_CAPTURE_IND;
	} else {
		byte |= DP_PSR_FRAME_CAPTURE;
		reg |= DPTX_CFG_FRAME_CAPTURE_IND;
	}

	if (dptx->psr_params.psr_version >= DP_PSR2_IS_SUPPORTED) {
		hblank_time = (uint32_t)(1000000 * dptx->vparams.mdtd.h_blanking / dptx->vparams.mdtd.pixel_clock);
		if (hblank_time > MIN_PSR2_SCAN_LINE_CAPTURE_TIME)
			byte &= ~DP_PSR_SELECTIVE_UPDATE;
		else
			byte |= DP_PSR_SELECTIVE_UPDATE;
	}
	retval = dptx_write_dpcd(dptx, DP_PSR_EN_CFG, byte);
	if (retval) {
		dpu_pr_err("[DP] write DPCD error");
		return retval;
	}
	dptx_writel(dptx, DPTX_PSR_CTRL, reg);

	return 0;
}

static void dptx_psr2_enable(struct dp_ctrl *dptx)
{
	int retval;
	uint8_t byte;
	uint32_t reg;

	/* sink psr2 open */
	retval = dptx_read_dpcd(dptx, DP_PSR_EN_CFG, &byte);
	if (retval) {
		dpu_pr_err("[DP] Read DPCD error");
		return;
	}
	byte |= (DP_PSR_ENABLE_PSR2 | DP_PSR_IRQ_HPD_WITH_CRC_ERRORS);
	retval = dptx_write_dpcd(dptx, DP_PSR_EN_CFG, byte);
	if (retval) {
		dpu_pr_err("[DP] write DPCD error");
		return;
	}

	/* source psr2 open */
	reg = dptx_readl(dptx, DPTX_PSR_CTRL);
	reg |= DPTX_CFG_PSR2_ENABLE;
	dptx_writel(dptx, DPTX_PSR_CTRL, reg);
}

static int dptx_psr2_line_num_cfg(struct dp_ctrl *dptx)
{
	uint32_t val;
	uint32_t line_num;
	uint32_t htotal_time;

	if (unlikely(dptx->vparams.mdtd.pixel_clock == 0)) {
		dpu_pr_err("[DP] Illegal pixel_clock value");
		return -EINVAL;
	}
	val = dptx_readl(dptx, DPTX_ALPM_ENABLE);
	htotal_time = (uint32_t)(1000 * (dptx->vparams.mdtd.h_active + dptx->vparams.mdtd.h_blanking) /
		dptx->vparams.mdtd.pixel_clock);
	dpu_pr_info("[DP] dptx_psr2_line_num_cfg h_active:%d, h_blanking:%d, pixel_clock:%d",
		dptx->vparams.mdtd.h_active, dptx->vparams.mdtd.h_blanking, dptx->vparams.mdtd.pixel_clock);
	if (htotal_time == 0) {
		dpu_pr_err("[DP] Illegal htotal time value");
		return -EINVAL;
	}
	line_num = (FW_3A_EXIT_TIME + htotal_time - 1) / htotal_time + 1;
	val &= ~DPTX_CFG_ALPM_EXIT_LINE_NUM_MASK;
	val |= (line_num << DPTX_CFG_ALPM_EXIT_LINE_NUM_SHIFT);
	/* should equal to (frm_start_dly - rx2tx_dly) */
	line_num = FRM_START_LINE_NUM_VAL;
	val &= ~DPTX_CFG_FRM_START_LINE_NUM_MASK;
	val |= (line_num << DPTX_CFG_FRM_START_LINE_NUM_SHIFT);
	val &= ~DPTX_CFG_ALPM_PHY_PWD_DELAY;
	/* need longer time for sending PPS per frame */
	val |= (ALPM_PHY_PWD_DELAY_TIME << DPTX_CFG_ALPM_PHY_PWD_DELAY_SHIFT);
	dptx_writel(dptx, DPTX_ALPM_ENABLE, val);

	return 0;
}

static int dptx_enable_alpm(struct dp_ctrl *dptx)
{
	int retval;
	uint8_t byte;
	uint32_t reg;

	/* enable sink ALPM */
	retval = dptx_read_dpcd(dptx, DP_RECEIVER_ALPM_CONFIG, &byte);
	if (retval) {
		dpu_pr_err("[DP] Read DPCD error");
		return retval;
	}
	byte |= (DP_ALPM_ENABLE | DP_ALPM_LOCK_ERROR_IRQ_HPD_ENABLE);
	retval = dptx_write_dpcd(dptx, DP_RECEIVER_ALPM_CONFIG, byte);
	if (retval) {
		dpu_pr_err("[DP] write DPCD error");
		return retval;
	}

	reg = dptx_readl(dptx, DPTX_ALPM_ENABLE);
	reg &= ~DPTX_ALPM_PHY_POWER_MODE_MASK;
	reg |= (ALPM_P2 << DPTX_ALPM_PHY_POWER_MODE_SHIFT);
	reg |= DPTX_CFG_ALPM_3A_ENABLE;
	reg &= ~DPTX_CFG_ALPM_2A_ENABLE;

	if ((dptx->connector->conn_info->base.fpga_flag != 0) || !dptx->psr_params.ml_close_require) {
		reg |= DPTX_CFG_ALPM_PHY_STATUS_TIMEOUT_ENABLE;
		dptx_writel(dptx, DTPX_ALPM_TIMEOUT, (EXIT_ALPM_TIME |
					(ENTRY_ALPM_TIME << DPTX_CFG_PHY_POWER_OFF_TIMEOUT_LINK_NUM_SHIFT)));
		reg &= ~DPTX_ALPM_PHY_POWER_MODE_MASK;
		reg &= ~DPTX_CFG_ALPM_3A_ENABLE;
	} else {
		reg &= ~DPTX_CFG_ALPM_PHY_STATUS_TIMEOUT_ENABLE;
	}
	dptx_writel(dptx, DPTX_ALPM_ENABLE, reg);

	retval = dptx_psr2_line_num_cfg(dptx);
	if (retval) {
		dpu_pr_err("[DP] Failed to init ALPM function %d", retval);
		return retval;
	}

	return 0;
}

static int dptx_psr2_mloff_config(struct dp_ctrl *dptx)
{
	uint32_t reg;
	uint32_t hblank_time;

	if (unlikely(dptx->vparams.mdtd.pixel_clock == 0)) {
		dpu_pr_err("[DP] Illegal pixel_clock value");
		return -EINVAL;
	}
	reg = dptx_readl(dptx, DPTX_PSR_CTRL);
	hblank_time = (uint32_t)(1000000 * dptx->vparams.mdtd.h_blanking / dptx->vparams.mdtd.pixel_clock);
	if (hblank_time > MIN_PSR2_SCAN_LINE_CAPTURE_TIME)
		reg &= ~DPTX_CFG_SU_REGION_SCAN_LINE_CAPTURE_IND;
	else
		reg |= DPTX_CFG_SU_REGION_SCAN_LINE_CAPTURE_IND;
	reg |= DPTX_CFG_PSR2_TIMING_GEN_OFF;
	if (dptx->psr_params.ml_close_require)
		reg |= DPTX_CFG_PSR_ACTIVE_ML_OFF;
	else
		reg &= ~DPTX_CFG_PSR_ACTIVE_ML_OFF;
	dptx_writel(dptx, DPTX_PSR_CTRL, reg);

	return 0;
}

int dptx_psr_initial(struct dp_ctrl *dptx)
{
	uint32_t reg;
	int retval;

	dpu_check_and_return((dptx == NULL), -EINVAL, err, "[DP] dptx is NULL!");

	dpu_pr_info("[DP] [DPTX] initial PSR");

	reg = dptx_readl(dptx, DPTX_PSR_CTRL);
	reg |= DPTX_CFG_PSR_ENABLE;
	if (dptx->psr_params.ml_close_require)
		reg |= DPTX_CFG_PSR_ACTIVE_ML_OFF;
	else
		reg &= ~DPTX_CFG_PSR_ACTIVE_ML_OFF;
	dptx_writel(dptx, DPTX_PSR_CTRL, reg);

	dptx_psr_sdp_config(dptx);
	retval = dptx_psr_set_capture_indication(dptx);
	if (retval) {
		dpu_pr_err("[DP] Failed to init psr function %d", retval);
		return retval;
	}

	return 0;
}

int dptx_psr_entry(struct dp_ctrl *dptx)
{
	uint32_t reg;

	dpu_check_and_return((dptx == NULL), -EINVAL, err, "[DP] dptx is NULL!");

	dpu_pr_info("[DP] PSR start to enter, port_id is %d", dptx->port_id);
	reg = dptx_readl(dptx, DPTX_PSR_CTRL);
	reg |= DPTX_CFG_PSR_ENTRY;
	dptx_writel(dptx, DPTX_PSR_CTRL, reg);
	dpu_pr_info("[DP] PSR enter done, port_id is %d", dptx->port_id);

	return 0;
}

int dptx_change_psr_status(struct dp_ctrl *dptx, uint32_t psr_status)
{
	uint32_t reg;
	int retval;
	bool ml_status = true;
	uint32_t status;

	dpu_check_and_return((dptx == NULL), -EINVAL, err, "[DP] dptx is NULL!");

	if (psr_status == PSR_UPDATE) {
		status = DPTX_CFG_PSR_UPDATE_ENABLE;
		dpu_pr_info("[DP] PSR status change to update, port_id is %d", dptx->port_id);
	} else if (psr_status == PSR_EXIT) {
		status = DPTX_CFG_PSR_EXIT;
		dpu_pr_info("[DP] PSR status change to exit, port_id is %d", dptx->port_id);
	} else {
		dpu_pr_err("[DP] Illegal psr status");
		return -EINVAL;
	}

	reg = dptx_readl(dptx, DPTX_PHYIF_CTRL3);
	if ((reg & DPTX_LANE0_PHY_STATUS_MASK) == LANE_POWER_MODE_P3)
		ml_status = false;

	dpu_pr_info("[DP] ml_status : %s", ml_status ? "true" : "false");
	if (!ml_status) {
		retval = dptx_psr_ml_config(dptx, LANE_POWER_MODE_P0);
		if (retval != 0) {
			dpu_pr_err("[DP] Config mainlink status P0 fail");
			return retval;
		}
		dptx_psr_exit_manage(dptx);
	}

	reg = dptx_readl(dptx, DPTX_PSR_CTRL);
	reg |= status;
	dptx_writel(dptx, DPTX_PSR_CTRL, reg);
	dpu_pr_info("[DP] PSR status changed, port_id is %d", dptx->port_id);

	if (!ml_status) {
		if (dptx->psr_params.need_training) {
			dptx_fast_link(dptx, true);
		} else {
			reg = dptx_readl(dptx, DPTX_PHYIF_CTRL0);
			reg &= ~DPTX_PATTERN_SEL_MASK;
			dptx_writel(dptx, DPTX_PHYIF_CTRL0, reg);
		}
	}

	return 0;
}

int dptx_psr2_initial(struct dp_ctrl *dptx)
{
	uint8_t byte;
	int retval;

	dpu_check_and_return((dptx == NULL), -EINVAL, err, "[DP] dptx is NULL!");

	dpu_pr_info("[DP] [DPTX] initial PSR2");

	dptx_psr_sdp_config(dptx);
	dptx_psr2_aux_frame_sync_disable(dptx);
	retval = dptx_psr_set_capture_indication(dptx);
	if (retval) {
		dpu_pr_err("[DP] Failed to init psr2 function %d", retval);
		return retval;
	}
	dptx_psr2_enable(dptx);

	retval = dptx_read_dpcd(dptx, DP_RECEIVER_ALPM_CAP, &byte);
	if (retval) {
		dpu_pr_err("[DP] Read DPCD error");
		return -EINVAL;
	}
	if (byte & DP_ALPM_CAP) {
		retval = dptx_enable_alpm(dptx);
		if (retval) {
			dpu_pr_err("[DP] Failed to enable psr2 ALPM %d", retval);
			return -EINVAL;
		}
	}

	retval = dptx_psr2_mloff_config(dptx);
	if (retval) {
		dpu_pr_err("[DP] Failed to config psr2 lane status %d", retval);
		return -EINVAL;
	}

	return 0;
}