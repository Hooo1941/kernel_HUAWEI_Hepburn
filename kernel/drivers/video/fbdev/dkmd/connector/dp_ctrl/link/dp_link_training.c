/*
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

#include <linux/uaccess.h>
#include "dp_link_training.h"
#include "controller/dp_core_interface.h"
#include "dp_link_layer_interface.h"
#include "dp_irq.h"
#include "dp_ctrl.h"
#include "dp_aux.h"
#include "hidptx_dp_core.h"
#include "dp_dsc_algorithm.h"
#include "drm_dp_helper_additions.h"
#include "hidptx_dp_avgen.h"
#include "dp_ctrl_config.h"
#include "dp_ctrl_dev.h"

static int dptx_link_read_status(struct dp_ctrl *dptx)
{
	int retval = 0;

	dpu_check_and_return(!dptx, -EINVAL, err, "[DP] dptx is NULL!\n");

	retval = dptx_read_bytes_from_dpcd(dptx, DP_LANE0_1_STATUS, dptx->link.status, sizeof(dptx->link.status));
	if (retval) {
		dpu_pr_err("[DP] failed to dptx_read_bytes_from_dpcd DP_DPCD_REV.\n");
		return retval;
	}

	dpu_pr_info("[DP] [0x202] dptx_read_dpcd 0x202: [%x][%x][%x][%x][%x][%x]\n", dptx->link.status[0],
		dptx->link.status[1], dptx->link.status[2], dptx->link.status[3], dptx->link.status[4], dptx->link.status[5]);

	return 0;
}

static int dptx_link_check_cr_done(struct dp_ctrl *dptx, bool *out_done)
{
	int retval;
	uint8_t byte = 0;
	uint32_t reg;

	dpu_check_and_return(!dptx, -EINVAL, err, "[DP] dptx is NULL!\n");
	dpu_check_and_return((out_done == NULL), -EINVAL, err, "[DP] out_done is NULL!\n");

	*out_done = false;

	retval = dptx_read_dpcd(dptx, DP_TRAINING_AUX_RD_INTERVAL, &byte);
	if (retval)
		return retval;

	reg = min_t(uint32_t, (byte & 0x7f), 4);
	reg *= 4000;
	if (reg == 0)
		reg = 400;

	/* DPTX spec acquire:
	 * TRAINING_AUX_RD_INTERVAL
	 * 00h = 400us.
	 * 01h = 4ms all.
	 * 02h = 8ms all.
	 * 03h = 12ms all.
	 * 04h = 16ms all.
	 */
	udelay(reg);

	mdelay(4); /* Fix me: Reducing rate problem */

	retval = dptx_link_read_status(dptx);
	if (retval)
		return retval;

	*out_done = drm_dp_clock_recovery_ok(dptx->link.status, dptx->link.lanes);

	dpu_pr_info("[DP] [0x00e] CR_DONE = %d, byte:%lx\n", *out_done, byte);

	return 0;
}

static int dptx_link_check_ch_eq_done(struct dp_ctrl *dptx, bool *out_cr_done, bool *out_ch_eq_done)
{
	int retval;
	bool done = false;

	dpu_check_and_return(!dptx, -EINVAL, err, "[DP] dptx is NULL!\n");
	dpu_check_and_return(!out_cr_done, -EINVAL, err, "[DP] out_cr_done is NULL!\n");
	dpu_check_and_return(!out_ch_eq_done, -EINVAL, err, "[DP] out_ch_eq_done is NULL!\n");

	retval = dptx_link_check_cr_done(dptx, &done);
	if (retval)
		return retval;

	*out_cr_done = false;
	*out_ch_eq_done = false;

	if (!done)
		return 0;

	*out_cr_done = true;
	*out_ch_eq_done = drm_dp_channel_eq_ok(dptx->link.status, dptx->link.lanes);

	dpu_pr_info("[DP] CH_EQ_DONE = %d\n", *out_ch_eq_done);

	return 0;
}

int dptx_link_rate_index(struct dp_ctrl *dp_ctrl, uint8_t rate)
{
	int i;

	for (i = 0; i < (int32_t)dp_ctrl->rx_rate_count; i++) {
		if (dp_ctrl->rx_rate_caps[i] == rate)
			return i;
	}
	return -1;
}

static int dptx_link_training_lanes_set(struct dp_ctrl *dptx, uint8_t patten, bool patten_valide)
{
	int retval;
	uint32_t i;
	uint8_t bytes[5] = { patten, 0xff, 0xff, 0xff, 0xff };
	uint8_t byte = 0;

	dpu_check_and_return(!dptx, -EINVAL, err, "[DP] dptx is NULL!\n");
	dpu_check_and_return((dptx->link.lanes > 4), -EINVAL, err, "[DP] lanes is out of scope!\n");

	for (i = 0; i < dptx->link.lanes; i++) {
		byte |= ((dptx->link.vswing_level[i] <<
			  DP_TRAIN_VOLTAGE_SWING_SHIFT) &
			 DP_TRAIN_VOLTAGE_SWING_MASK);

		if (dptx->link.vswing_level[i] == 3)
			byte |= DP_TRAIN_MAX_SWING_REACHED;

		byte |= ((dptx->link.preemp_level[i] <<
			  DP_TRAIN_PRE_EMPHASIS_SHIFT) &
			 DP_TRAIN_PRE_EMPHASIS_MASK);

		if (dptx->link.preemp_level[i] == 3)
			byte |= DP_TRAIN_MAX_PRE_EMPHASIS_REACHED;

		bytes[i + 1] = byte;
	}

	if (patten_valide) {
		retval = dptx_write_bytes_to_dpcd(dptx, DP_TRAINING_PATTERN_SET, bytes, dptx->link.lanes + 1);
	} else {
		retval = dptx_write_bytes_to_dpcd(dptx, DP_TRAINING_LANE0_SET, bytes + 1, dptx->link.lanes);
	}
	if (retval)
		return retval;

	return 0;
}

int dptx_link_adjust_drive_settings(struct dp_ctrl *dptx, int *out_changed)
{
	int retval;
	uint32_t lanes;
	uint32_t i;
	uint8_t byte = 0;
	uint8_t adj[4] = { 0 };
	int changed = false;

	dpu_check_and_return(!dptx, -EINVAL, err, "[DP] dptx is NULL!\n");

	lanes = dptx->link.lanes;
	dpu_pr_info("[DP] Lane: %d\n", lanes);

	switch (lanes) {
	/* This case (value 4) is not terminated by a 'break' statement */
	case 4:
		retval = dptx_read_dpcd(dptx, DP_ADJUST_REQUEST_LANE2_3, &byte);
		if (retval)
			return retval;

		adj[2] = byte & 0x0f;
		adj[3] = (byte & 0xf0) >> 4;
	case 2:
	case 1:
		retval = dptx_read_dpcd(dptx, DP_ADJUST_REQUEST_LANE0_1, &byte);
		if (retval)
			return retval;

		adj[0] = byte & 0x0f;
		adj[1] = (byte & 0xf0) >> 4;
		break;
	default:
		dpu_pr_err("[DP] Invalid number of lanes %d\n", lanes);
		return -EINVAL;
	}

	/* Save the drive settings */
	for (i = 0; i < lanes; i++) {
		uint8_t vs = adj[i] & 0x3;
		uint8_t pe = (adj[i] & 0xc) >> 2;

		if (dptx->link.vswing_level[i] != vs)
			changed = true;

		dptx->link.vswing_level[i] = vs;
		dptx->link.preemp_level[i] = pe;
		dpu_pr_info("[DP] [0x206/0x207]lanes[%d] vs:%d, pe:%d\n", i, vs, pe);
	}

	if (dptx->dptx_link_set_preemp_vswing)
		dptx->dptx_link_set_preemp_vswing(dptx);

	retval = dptx_link_training_lanes_set(dptx, 0, false);
	if (retval)
		return retval;

	if (out_changed != NULL)
		*out_changed = changed;

	return 0;
}

static int dptx_link_training_init(struct dp_ctrl *dptx, uint8_t rate, uint8_t lanes)
{
	uint8_t sink_max_rate;
	uint8_t sink_max_lanes;

	dpu_check_and_return(!dptx, -EINVAL, err, "[DP] dptx is NULL!\n");
	dpu_check_and_return(dptx->rx_rate_count <= 0, -EINVAL, err, "[DP] Invalid rx_rate_count!\n");
	dpu_pr_info("[DP] linktraining init lanes=%d, rate=%d\n", lanes, rate);

	if (rate > DPTX_PHYIF_CTRL_EDP_RATE_R432) {
		dpu_pr_err("[DP] Invalid rate %d\n", rate);
		rate = DPTX_PHYIF_CTRL_RATE_RBR;
	}

	if ((lanes == 0) || (lanes == 3) || (lanes > 4)) {
		dpu_pr_err("[DP] Invalid lanes %d\n", lanes);
		lanes = 1;
	}

	/* Initialize link parameters */
	memset(dptx->link.preemp_level, 0, sizeof(uint8_t) * 4);
	memset(dptx->link.vswing_level, 0, sizeof(uint8_t) * 4);
	memset(dptx->link.status, 0, DP_LINK_STATUS_SIZE);

	sink_max_lanes = drm_dp_max_lane_count(dptx->rx_caps);
	if (sink_max_lanes > 0 && lanes > sink_max_lanes) {
		lanes = sink_max_lanes;
		dptx->max_lanes = sink_max_lanes;
	}

	sink_max_rate = dptx->rx_caps[DP_MAX_LINK_RATE];
	sink_max_rate = dptx_bw_to_phy_rate(sink_max_rate);
	// dptx rate cannt bigger than sink rate
	if (dptx_link_rate_index(dptx, sink_max_rate) >= 0 &&
		dptx_link_rate_index(dptx, rate) > dptx_link_rate_index(dptx, sink_max_rate)) {
		dpu_pr_info("[DP] Reduce source rate%d to rate%d\n", rate, sink_max_rate);
		rate = sink_max_rate;
		dptx->max_rate = sink_max_rate;
	}

	if (dptx_link_rate_index(dptx, (uint8_t)dptx->rx_rate_caps[0]) < 0) {
		dpu_pr_warn("[DP] ERROR OCCUR, bad rate, set to default RBR");
		dptx->rx_rate_caps[0] = DPTX_PHYIF_CTRL_RATE_RBR;
		dptx->rx_rate_index = 0;
	}
	if (dptx_link_rate_index(dptx, rate) < 0) {
		rate = (uint8_t)dptx->rx_rate_caps[0];
		dptx->rx_rate_index = 0;
	}
	dptx->rx_rate_index = (uint32_t)dptx_link_rate_index(dptx, rate);

	dpu_pr_info("[DP] Sink Device Capability: lanes=%d, rate=%d, current: lanes=%d, rate=%d\n",
		sink_max_lanes, sink_max_rate, lanes, rate);
	dp_imonitor_set_param(DP_PARAM_MAX_RATE,   &sink_max_rate);
	dp_imonitor_set_param(DP_PARAM_MAX_LANES,  &sink_max_lanes);
	dp_imonitor_set_param(DP_PARAM_LINK_RATE,  &rate);
	dp_imonitor_set_param(DP_PARAM_LINK_LANES, &lanes);

	dptx->link.lanes = lanes;
	dptx->link.rate = rate;
	dptx->link.trained = false;

	return 0;
}

static int dptx_link_training_pattern_set(struct dp_ctrl *dptx, uint8_t pattern)
{
	int retval;
	dpu_check_and_return(!dptx, -EINVAL, err, "[DP] dptx is NULL!\n");

	retval = dptx_link_training_lanes_set(dptx, pattern, true);
	if (retval) {
		dpu_pr_err("[DP] 102/103 dptx_link_training_lanes_set fail after 102");
		return retval;
	}
	return 0;
}

static int dptx_link_wait_cr_and_adjust(struct dp_ctrl *dptx, bool ch_eq)
{
	int i;
	int retval;
	int changed = 0;
	int param_repeat_times = 0;
	bool done = false;

	dpu_check_and_return(!dptx, -EINVAL, err, "[DP] dptx is NULL!\n");

	retval = dptx_link_check_cr_done(dptx, &done);
	if (retval)
		return retval;

	if (done)
		return 0;

	/* Try 10 times at most according to dp1.4 */
	for (i = 0; i < 10; i++) {
		retval = dptx_link_adjust_drive_settings(dptx, &changed);
		if (retval)
			return retval;

		/* Reset repeat count if we changed settings */
		if (changed)
			param_repeat_times = 0;

		retval = dptx_link_check_cr_done(dptx, &done);
		if (retval)
			return retval;

		if (done)
			return 0;

		param_repeat_times++;
		if (param_repeat_times >= 5) /* Try each adjustment setting 5 times */
			return -EPROTO;

		/* check for all lanes */
		/* Failed and reached the maximum voltage swing */
		if (dptx->link.vswing_level[0] == 3)
			return -EPROTO;
	}

	return -EPROTO;
}

static int dptx_link_cr(struct dp_ctrl *dptx)
{
	int retval;

	dpu_check_and_return(!dptx, -EINVAL, err, "[DP] dptx is NULL!\n");

	dpu_pr_info("[DP] link training excute cr verify\n");

	if (dptx->dptx_phy_set_pattern)
		dptx->dptx_phy_set_pattern(dptx, DPTX_PHYIF_CTRL_TPS_1);

	// start send dp wave
	dptx_phy_set_lanes(dptx, dptx->link.lanes);

	retval = dptx_link_training_pattern_set(dptx, DP_TRAINING_PATTERN_1 | DP_LINK_SCRAMBLING_DISABLE);
	if (retval)
		return retval;

	return dptx_link_wait_cr_and_adjust(dptx, false);
}

static int dptx_link_ch_eq_get_pattern(struct dp_ctrl *dptx, uint32_t *pattern, uint8_t *dp_pattern)
{
	dpu_check_and_return(!dptx, -EINVAL, err, "[DP] dptx is NULL!\n");
	dpu_check_and_return(!pattern, -EINVAL, err, "[DP] pattern is NULL!\n");
	dpu_check_and_return(!dp_pattern, -EINVAL, err, "[DP] dp_pattern is NULL!\n");

	dpu_pr_info("[DP] dptx->max_rate: %d, dptx->link.rate: %d\n", dptx->max_rate, dptx->link.rate);

	switch (dptx->max_rate) {
	case DPTX_PHYIF_CTRL_EDP_RATE_R432:
	case DPTX_PHYIF_CTRL_RATE_HBR3:
		if (drm_dp_tps4_supported(dptx->rx_caps)) {
			*pattern = DPTX_PHYIF_CTRL_TPS_4;
			*dp_pattern = DP_TRAINING_PATTERN_4;
			break;
		}
	/* Fall through */
	case DPTX_PHYIF_CTRL_EDP_RATE_R324:
	case DPTX_PHYIF_CTRL_RATE_HBR2:
		if (drm_dp_tps3_supported(dptx->rx_caps)) {
			*pattern = DPTX_PHYIF_CTRL_TPS_3;
			*dp_pattern = DP_TRAINING_PATTERN_3;
			break;
		}
	/* Fall through */
	case DPTX_PHYIF_CTRL_EDP_RATE_R243:
	case DPTX_PHYIF_CTRL_EDP_RATE_R216:
	case DPTX_PHYIF_CTRL_RATE_RBR:
	case DPTX_PHYIF_CTRL_RATE_HBR:
		*pattern = DPTX_PHYIF_CTRL_TPS_2;
		*dp_pattern = DP_TRAINING_PATTERN_2;
		break;
	default:
		dpu_pr_err("[DP] Invalid rate %d\n", dptx->link.rate);
		return -EINVAL;
	}

	return 0;
}

static int dptx_link_ch_eq(struct dp_ctrl *dptx)
{
	int retval;
	bool cr_done = false;
	bool ch_eq_done = false;
	uint32_t pattern;
	uint32_t i;
	uint8_t dp_pattern;

	dpu_check_and_return(!dptx, -EINVAL, err, "[DP] dptx is NULL!\n");

	dpu_pr_info("[DP] link training excute eq verify\n");

	retval = dptx_link_ch_eq_get_pattern(dptx, &pattern, &dp_pattern);
	if (retval)
		return retval;

	if (dptx->dptx_phy_set_pattern)
		dptx->dptx_phy_set_pattern(dptx, pattern);

	/* this needs to be different for other versions of
	 * DPRX
	 */
	if (dp_pattern != DP_TRAINING_PATTERN_4) {
		retval = dptx_link_training_pattern_set(dptx, dp_pattern | DP_LINK_SCRAMBLING_DISABLE);
	} else {
		retval = dptx_link_training_pattern_set(dptx, dp_pattern);
		dpu_pr_info("[DP] Enabling scrambling for TPS4\n");
	}
	if (retval)
		return retval;

	for (i = 0; i < 6; i++) {
		retval = dptx_link_check_ch_eq_done(dptx, &cr_done, &ch_eq_done);
		if (retval)
			return retval;

		dptx->cr_fail = false;

		if (!cr_done) {
			dptx->cr_fail = true;
			return -EPROTO;
		}

		if (ch_eq_done)
			return 0;

		retval = dptx_link_adjust_drive_settings(dptx, NULL);
		if (retval)
			return retval;
	}
	dpu_pr_warn("[DP] link check ch eq failed");

	return -EPROTO;
}

static int dptx_link_reduce_rate(struct dp_ctrl *dptx)
{
	uint32_t rate = 0;

	dpu_check_and_return(!dptx, -EINVAL, err, "[DP] dptx is NULL!\n");
	dpu_check_and_return(dptx->rx_rate_index >= DPTX_RATE_CAP_SIZE || dptx->rx_rate_count < 1,
		-EINVAL, err, "[DP] rx_rate_index is err!\n");

	rate = dptx->link.rate;

	// check valide of parameter of rate.
	if (dptx->rx_rate_caps[dptx->rx_rate_index] != rate) {
		dpu_pr_warn("[DP] Reducing rate %u not equal %u\n", rate, dptx->rx_rate_caps[dptx->rx_rate_index]);
		dptx->rx_rate_index = dptx->rx_rate_count - 1;
		rate = dptx->rx_rate_caps[dptx->rx_rate_index];
	}

	if (dptx->rx_rate_index <= 0) {
		dpu_pr_info("[DP] reduce rate to min %d, (min rate:%d)\n", rate,
			dptx->connector->conn_info->min_dptx_rate);
		return -EPROTO;
	}

	if (dptx->rx_rate_index <= (uint32_t)dptx_link_rate_index(dptx, (uint8_t)dptx->connector->conn_info->min_dptx_rate)) {
		dpu_pr_info("[DP] Already Reducing rate to %d\n", dptx->connector->conn_info->min_dptx_rate);
		return -EPROTO;
	}

	rate = dptx->rx_rate_caps[dptx->rx_rate_index - 1];
	dptx->rx_rate_index = dptx->rx_rate_index - 1;

	dpu_pr_info("[DP] start Reducing rate from %u to %u\n", dptx->link.rate, rate);
	dptx->link.rate = rate;

	return 0;
}

static int dptx_link_reduce_lanes(struct dp_ctrl *dptx)
{
	uint32_t lanes;

	dpu_check_and_return(!dptx, -EINVAL, err, "[DP] dptx is NULL!\n");

	if (dptx->link.lanes <= dptx->connector->conn_info->min_dptx_lane) {
		dpu_pr_info("[DP] Already Reducing lane to min %d\n", dptx->connector->conn_info->min_dptx_lane);
		return -EPROTO;
	}

	switch (dptx->link.lanes) {
	case 4:
		lanes = 2;
		break;
	case 2:
		lanes = 1;
		break;
	case 1:
	default:
		dpu_pr_info("[DP] dptx_link_reduce_lanes lane is 1, cannt reduce!\n");
		return -EPROTO;
	}
	dpu_pr_info("[DP] Reducing lanes from %u to %u\n", dptx->link.lanes, lanes);
	dptx->link.lanes = lanes;

	return 0;
}

/*
 * When cr fail, try to redue lane num or lane rate
 */
static void dptx_link_cr_adjust_link_params(struct dp_ctrl *dptx, int *retval, bool *try_again)
{
	uint8_t byte_status0;
	uint8_t byte_status1;
	bool lane_status;

	dpu_check_and_no_retval(!dptx, err, "[DP] dptx is NULL!\n");
	dpu_check_and_no_retval(!retval, err, "[DP] retval is NULL!\n");
	dpu_check_and_no_retval(!try_again, err, "[DP] try_again is NULL!\n");

	if (*retval != -EPROTO) {
		*try_again = false;
		return;
	}

	/* for factory test */
	if (dp_factory_mode_is_enable()) {
		dp_factory_link_cr_or_ch_eq_fail(true);
		dpu_pr_err("[DP] not support reduce rate by CR verify in factory mode!\n");
		*retval = -ECONNREFUSED;
		*try_again = false;
		return;
	}
	dpu_pr_info("[DP] Reduce rate by CR verify\n");
	if (dptx_link_reduce_rate(dptx)) {
		byte_status0 = dptx->link.status[DP_LANE0_1_STATUS - DP_LANE0_1_STATUS];
		byte_status1 = dptx->link.status[DP_LANE0_1_STATUS - DP_LANE0_1_STATUS];
		dpu_pr_info("[DP] byte_status0:%x, byte_status1:%x\n", byte_status0, byte_status1);
		lane_status = (byte_status0 & 0x01) && (byte_status0 & 0x10);
		/*
			* If CR_DONE bits for some lanes
			* are set, we should reduce lanes to
			* those lanes.
			*/
		if ((dptx->link.lanes == 4 && !lane_status) || (dptx->link.lanes == 2 && !(byte_status0 & 0x01))) {
			dpu_pr_info("[DP] none lanes need reduce\n");
			*retval = -EPROTO;
			*try_again = false;
			return;
		}
		if (dptx_link_reduce_lanes(dptx)) {
			*retval = -EPROTO;
			*try_again = false;
			return;
		}
		dptx->link.rate  = dptx->max_rate;
	}
	dptx_link_training_init(dptx, dptx->link.rate, dptx->link.lanes);
	*try_again = true;
}

/*
 * When ch eq fail, try to redue lane num or lane rate
 */
static void dptx_link_ch_eq_adjust_link_params(struct dp_ctrl *dptx, int *retval, bool *try_again)
{
	int res;
	dpu_check_and_no_retval(!dptx, err, "[DP] dptx is NULL!\n");
	dpu_check_and_no_retval(!retval, err, "[DP] retval is NULL!\n");
	dpu_check_and_no_retval(!try_again, err, "[DP] try_again is NULL!\n");

	if (*retval != -EPROTO) {
		*try_again = false;
		return;
	}

	/* for factory test */
	if (dp_factory_mode_is_enable()) {
		dp_factory_link_cr_or_ch_eq_fail(false);
		dpu_pr_err("[DP] not support reduce rate by EQ verify in factory mode!\n");
		*retval = -ECONNREFUSED;
		*try_again = false;
		return;
	}

	if (!dptx->cr_fail) {
		if (dptx->link.lanes <= dptx->connector->conn_info->min_dptx_lane) {
			if (dptx_link_reduce_rate(dptx)) {
				*retval = -EPROTO;
				*try_again = false;
				return;
			}
			dptx->link.lanes = dptx->max_lanes;
		} else {
			(void)dptx_link_reduce_lanes(dptx);
		}
	} else {
		dpu_pr_info("[DP] Reduce rate by EQ verify\n");
		if (dptx_link_reduce_rate(dptx)) {
			if (dptx_link_reduce_lanes(dptx)) {
				*retval = -EPROTO;
				*try_again = false;
				return;
			}
			dptx->link.rate  = dptx->max_rate;
		}
	}
	// send 0 when finish once LinkTraining.
	res = dptx_link_training_pattern_set(dptx, DP_TRAINING_PATTERN_DISABLE);
	if (res)
		dpu_pr_info("[DP] set 102 failed, res: %d\n", res);

	dpu_pr_info("[DP] ch_eq rate=%d, lanes=%d", dptx->link.rate, dptx->link.lanes);
	dptx_link_training_init(dptx, dptx->link.rate, dptx->link.lanes);
	*try_again = true;
}

struct dptx_link_handle {
	int (* dptx_link)(struct dp_ctrl *dptx);
	void (* dptx_adjust_link_params)(struct dp_ctrl *dptx, int *retval, bool *try_again);
};

int dptx_link_training(struct dp_ctrl *dptx, uint8_t rate, uint8_t lanes)
{
	uint8_t byte, i;
	int retval;
	bool try_again = false;
	struct dptx_link_handle link_handle[] = {
		{ dptx_link_cr, dptx_link_cr_adjust_link_params },
		{ dptx_link_ch_eq, dptx_link_ch_eq_adjust_link_params }
	};

	dpu_check_and_return(!dptx, -EINVAL, err, "[DP] dptx is NULL!\n");

	if (dptx->dptx_triger_media_transfer)
		dptx->dptx_triger_media_transfer(dptx, false);

	retval = dptx_link_training_init(dptx, rate, lanes);
	if (retval)
		goto fail;

again:
	dpu_pr_info("[DP] Link training start!\n");
	if (dptx->dptx_link_set_lane_status)
		retval = dptx->dptx_link_set_lane_status(dptx);
	if (retval)
		goto fail;

	for (i = 0; i < (uint8_t)ARRAY_SIZE(link_handle); i++) {
		if ((retval = link_handle[i].dptx_link(dptx)) != 0) {
			try_again = false;
			link_handle[i].dptx_adjust_link_params(dptx, &retval, &try_again);
			if (try_again)
				goto again;
			else
				goto fail;
		}
	}
	if (dptx->dptx_phy_set_pattern)
		dptx->dptx_phy_set_pattern(dptx, DPTX_PHYIF_CTRL_TPS_NONE);

	retval = dptx_link_training_pattern_set(dptx, DP_TRAINING_PATTERN_DISABLE);
	if (retval)
		goto fail;

	/* Branch device detection */
	(void)dptx_read_dpcd(dptx, DP_SINK_COUNT, &byte);
	dpu_pr_info("[DP] DP_SINK_COUNT : %x", byte);
	(void)dptx_read_dpcd(dptx, DP_SINK_COUNT_ESI, &byte);
	dpu_pr_info("[DP] DP_SINK_COUNT_ESI : %x", byte);

	if (dptx->dptx_link_set_lane_after_training)
		dptx->dptx_link_set_lane_after_training(dptx);

	if (dptx->dptx_triger_media_transfer)
		dptx->dptx_triger_media_transfer(dptx, true);

	dptx->detect_times = 0;

	return 0;

fail:
	if (dptx->dptx_sink_device_connected && dptx->dptx_sink_device_connected(dptx)) {
		if (dptx->dptx_phy_set_pattern)
			dptx->dptx_phy_set_pattern(dptx, DPTX_PHYIF_CTRL_TPS_NONE);

		retval = dptx_link_training_pattern_set(dptx, DP_TRAINING_PATTERN_DISABLE);
		if (retval)
			return retval;
		dpu_pr_warn("[DP] Link training failed %d", retval);
	} else {
		dpu_pr_err("[DP]Link training failed as sink is disconnected %d", retval);
	}

	return -EINVAL;
}

int dptx_link_check_status(struct dp_ctrl *dptx)
{
	int retval;
	int ret;
	uint8_t byte;
	uint8_t bytes[2];

	dpu_check_and_return(!dptx, -EINVAL, err, "[DP] dptx is NULL!\n");

	retval = dptx_read_bytes_from_dpcd(dptx, DP_SINK_COUNT, bytes, 2);
	if (retval)
		return retval;

	retval = dptx_link_read_status(dptx);
	if (retval)
		return retval;

	byte = dptx->link.status[DP_LANE_ALIGN_STATUS_UPDATED - DP_LANE0_1_STATUS];
	if ((byte & DP_LINK_STATUS_UPDATED) == 0)
		return 0;

	dpu_pr_info("[DP] dptx_link_check_status rate=%d, lanes=%d", dptx->max_rate, dptx->max_lanes);

	/* Check if need to retrain link */
	if (dptx->link.trained &&
		(!drm_dp_channel_eq_ok(dptx->link.status, dptx->link.lanes) ||
		!drm_dp_clock_recovery_ok(dptx->link.status, dptx->link.lanes))) {
		dpu_pr_info("[DP] Retraining link\n");
		dp_imonitor_set_param(DP_PARAM_LINK_RETRAINING, NULL);

		if (dptx->fec && dptx->dptx_fec_enable)
			dptx->dptx_fec_enable(dptx, false);

		retval = dptx_link_training(dptx, dptx->max_rate, dptx->max_lanes);

		if (dptx->fec && dptx->dptx_fec_enable) {
			ret = dptx->dptx_fec_enable(dptx, true);
			if (ret)
				dpu_pr_err("[DP] fec enable failed!\n");
		}
		if (retval < 0) {
			dp_imonitor_set_param(DP_PARAM_LINK_TRAINING_FAILED, &retval);
			return retval;
		}
		if (!(dptx->current_link_rate == dptx->link.rate && dptx->current_link_lanes == dptx->link.lanes)) {
			dpu_pr_warn("[DP] pre rate: %d, pre lanes: %d, cur rate: %d, cur lanes: %d",
				dptx->current_link_rate, dptx->current_link_lanes, dptx->link.rate, dptx->link.lanes);
			dpu_pr_warn("[DP] pre rate or lanes is not equal to current rate or lanes, do unplug and plug!");
			if (dptx->handle_hotunplug)
				dptx->handle_hotunplug(dptx);

			mdelay(100);

			if (dptx->dp_dis_reset)
				dptx->dp_dis_reset(dptx, true);

			if (dptx->dptx_core_on)
				dptx->dptx_core_on(dptx);

			if (dptx->handle_hotplug)
				ret = dptx->handle_hotplug(dptx);
		}
	}

	return 0;
}

int dptx_link_retraining_common(struct dp_ctrl *dptx, uint8_t rate, uint8_t lanes)
{
	int i, retval;

	dpu_check_and_return(!dptx, -EINVAL, err, "[DP] dptx is NULL!\n");

	for (i = 0; i < LINK_REPEAT; i++) {
		retval = dptx_link_training(dptx, rate, lanes);
		if (retval == 0) {
			dpu_pr_info("[DP] init portid:%d, init-rate:%d, init-lanes:%d, link success, continue!!!",
				dptx->port_id, rate, lanes);
			break;
		}
		dpu_pr_info("[DP] link retraining, try again!!!\n");
		mdelay(100);
	}

	if (i >= LINK_REPEAT) {
		dpu_pr_err("[DP] Link retrain failed!!!\n");
		return -EINVAL;
	}
	dpu_pr_info("[DP] Common retraining link rate=%d lanes=%d\n", rate, lanes);

	return retval;
}

/*
 * dptx_link_retrain() - Retrain link
 * @dptx: The dptx struct
 * @rate: Link rate - Possible options: 0 - RBR, 1 - HBR, 2 - HBR2, 3 - HBR3
 * @lanes: Link lanes count - Possible options 1,2 or 4
 *
 * Returns 0 on success otherwise negative errno.
 */
int dptx_link_retraining(struct dp_ctrl *dptx, uint8_t rate, uint8_t lanes)
{
	int i, retval;
	uint32_t edid_info_size = 0;
	bool bsafe_mode = false;
	retval = 0;

	dpu_check_and_return(!dptx, -EINVAL, err, "[DP] dptx is NULL!\n");

	if (dptx->dptx_sink_device_connected && !(dptx->dptx_sink_device_connected(dptx))) {
		dpu_pr_err("[DP] Not connected\n");
		return  -ENODEV;
	}

	if (lanes != 1 && lanes != 2 && lanes != 4) {
		dpu_pr_err("[DP] Invalid number of lanes %d\n",
			lanes);
		return  -EINVAL;
	}

	if (rate > DPTX_PHYIF_CTRL_EDP_RATE_R432) {
		dpu_pr_err("[DP] Invalid number of lane rate %d\n",
			rate);
		return  -EINVAL;
	}

	/* get edid */
	retval = dptx_link_get_device_edid(dptx, &bsafe_mode, &edid_info_size);
	if (retval) {
		dpu_pr_err("[DP] read edid failed!");
		return retval;
	}

	dpu_pr_info("[DP] dptx_link_retraining rate=%d, lanes=%d", rate, lanes);
	for (i = 0; i < LINK_REPEAT; i++) {
		retval = dptx_link_training(dptx, rate, lanes);
		if (!retval) {
			dpu_pr_info("[DP] init portid:%d, init-rate:%d, init-lanes:%d, link success, continue!!!",
				dptx->port_id, rate, lanes);
			break;
		}
		dpu_pr_info("[DP] link retraining, try again!!!\n");
		mdelay(100);
	}

	if (i >= LINK_REPEAT) {
		dpu_pr_err("[DP] Link retrain failed!!!\n");
		return -EINVAL;
	}

	/* choose timing */
	retval = dptx_link_choose_timing(dptx, bsafe_mode, (int)edid_info_size);
	if (retval != 0) {
		dpu_pr_err("[DP] Link choose timing failed!");
		return retval;
	}

	if (dptx->dptx_video_ts_change)
		dptx->dptx_video_ts_change(dptx, 0);

	dpu_pr_info("[DP] Retraining link rate=%d lanes=%d\n",
		rate, lanes);

	return retval;
}

static int dptx_power_handler(struct dp_ctrl *dptx, bool ublank)
{
	dpu_check_and_return(!dptx, -EINVAL, err, "[DP] dptx is NULL!");

	if (!dptx->video_transfer_enable) {
		dpu_pr_info("[DP] disconnected, SR never excute!");
		return 0;
	}

	if (ublank) {
		if (dptx->dp_dis_reset)
			dptx->dp_dis_reset(dptx, true);

		if (dptx->dptx_core_on)
			dptx->dptx_core_on(dptx);

		dpu_pr_info("[DP] Retraining when blank on!");
		if (dptx->dptx_hpd_handler && !dptx->is_pluging_out)
			dptx->dptx_hpd_handler(dptx, true, dptx->max_lanes);
		if (dptx->dptx_audio_config)
			dptx->dptx_audio_config(dptx);
	} else {
		dptx_write_dpcd(dptx, DP_SET_POWER, DP_SET_POWER_D3);
		if (dptx->dptx_disable_default_video_stream)
			dptx->dptx_disable_default_video_stream(dptx, 0);
		if (dptx->dptx_triger_media_transfer)
			dptx->dptx_triger_media_transfer(dptx, false);
		mdelay(100);
		if (dptx->dptx_core_off)
			dptx->dptx_core_off(dptx);

		if (dptx->dp_dis_reset)
			dptx->dp_dis_reset(dptx, false);

		dpu_pr_info("[DP] Disable stream when blank off\n");
	}

	return 0;
}

static void dptx_hdr_calculate_infoframe_data(struct dp_ctrl *dptx)
{
	dpu_check_and_no_retval(!dptx, err, "[DP] dptx is NULL!\n");

	dptx->hdr_infoframe.data[HDR_INFOFRAME_EOTF_BYTE_NUM] =
		(uint8_t)dptx->hdr_metadata.electro_optical_transfer_function;
	dptx->hdr_infoframe.data[HDR_INFOFRAME_METADATA_ID_BYTE_NUM] =
		(uint8_t)dptx->hdr_metadata.static_metadata_descriptor_id;
	dptx->hdr_infoframe.data[HDR_INFOFRAME_DISP_PRI_X_0_LSB] = (uint8_t)(dptx->hdr_metadata.red_primary_x & LSB_MASK);
	dptx->hdr_infoframe.data[HDR_INFOFRAME_DISP_PRI_X_0_MSB] =
		(uint8_t)((dptx->hdr_metadata.red_primary_x & MSB_MASK) >> SHIFT_8BIT);
	dptx->hdr_infoframe.data[HDR_INFOFRAME_DISP_PRI_Y_0_LSB] = (uint8_t)(dptx->hdr_metadata.red_primary_y & LSB_MASK);
	dptx->hdr_infoframe.data[HDR_INFOFRAME_DISP_PRI_Y_0_MSB] =
		(uint8_t)((dptx->hdr_metadata.red_primary_y & MSB_MASK) >> SHIFT_8BIT);
	dptx->hdr_infoframe.data[HDR_INFOFRAME_DISP_PRI_X_1_LSB] = (uint8_t)(dptx->hdr_metadata.green_primary_x & LSB_MASK);
	dptx->hdr_infoframe.data[HDR_INFOFRAME_DISP_PRI_X_1_MSB] =
		(uint8_t)((dptx->hdr_metadata.green_primary_x & MSB_MASK) >> SHIFT_8BIT);
	dptx->hdr_infoframe.data[HDR_INFOFRAME_DISP_PRI_Y_1_LSB] = (uint8_t)(dptx->hdr_metadata.green_primary_y & LSB_MASK);
	dptx->hdr_infoframe.data[HDR_INFOFRAME_DISP_PRI_Y_1_MSB] =
		(uint8_t)((dptx->hdr_metadata.green_primary_y & MSB_MASK) >> SHIFT_8BIT);
	dptx->hdr_infoframe.data[HDR_INFOFRAME_DISP_PRI_X_2_LSB] = (uint8_t)(dptx->hdr_metadata.blue_primary_x & LSB_MASK);
	dptx->hdr_infoframe.data[HDR_INFOFRAME_DISP_PRI_X_2_MSB] =
		(uint8_t)((dptx->hdr_metadata.blue_primary_x & MSB_MASK) >> SHIFT_8BIT);
	dptx->hdr_infoframe.data[HDR_INFOFRAME_DISP_PRI_Y_2_LSB] = (uint8_t)(dptx->hdr_metadata.blue_primary_y & LSB_MASK);
	dptx->hdr_infoframe.data[HDR_INFOFRAME_DISP_PRI_Y_2_MSB] =
		(uint8_t)((dptx->hdr_metadata.blue_primary_y & MSB_MASK) >> SHIFT_8BIT);
	dptx->hdr_infoframe.data[HDR_INFOFRAME_WHITE_POINT_X_LSB] = (uint8_t)(dptx->hdr_metadata.white_point_x & LSB_MASK);
	dptx->hdr_infoframe.data[HDR_INFOFRAME_WHITE_POINT_X_MSB] =
		(uint8_t)((dptx->hdr_metadata.white_point_x & MSB_MASK) >> SHIFT_8BIT);
	dptx->hdr_infoframe.data[HDR_INFOFRAME_WHITE_POINT_Y_LSB] = (uint8_t)(dptx->hdr_metadata.white_point_y & LSB_MASK);
	dptx->hdr_infoframe.data[HDR_INFOFRAME_WHITE_POINT_Y_MSB] =
		(uint8_t)((dptx->hdr_metadata.white_point_y & MSB_MASK) >> SHIFT_8BIT);
	dptx->hdr_infoframe.data[HDR_INFOFRAME_MAX_LUMI_LSB] =
		(uint8_t)(dptx->hdr_metadata.max_display_mastering_luminance & LSB_MASK);
	dptx->hdr_infoframe.data[HDR_INFOFRAME_MAX_LUMI_MSB] =
		(uint8_t)((dptx->hdr_metadata.max_display_mastering_luminance & MSB_MASK) >> SHIFT_8BIT);
	dptx->hdr_infoframe.data[HDR_INFOFRAME_MIN_LUMI_LSB] =
		(uint8_t)(dptx->hdr_metadata.min_display_mastering_luminance & LSB_MASK);
	dptx->hdr_infoframe.data[HDR_INFOFRAME_MIN_LUMI_MSB] =
		(uint8_t)((dptx->hdr_metadata.min_display_mastering_luminance & MSB_MASK) >> SHIFT_8BIT);
	dptx->hdr_infoframe.data[HDR_INFOFRAME_MAX_LIGHT_LEVEL_LSB] =
		(uint8_t)(dptx->hdr_metadata.max_content_light_level & LSB_MASK);
	dptx->hdr_infoframe.data[HDR_INFOFRAME_MAX_LIGHT_LEVEL_MSB] =
		(uint8_t)((dptx->hdr_metadata.max_content_light_level & MSB_MASK) >> SHIFT_8BIT);
	dptx->hdr_infoframe.data[HDR_INFOFRAME_MAX_AVERAGE_LEVEL_LSB] =
		(uint8_t)(dptx->hdr_metadata.max_frame_average_light_level & LSB_MASK);
	dptx->hdr_infoframe.data[HDR_INFOFRAME_MAX_AVERAGE_LEVEL_MSB] =
		(uint8_t)((dptx->hdr_metadata.max_frame_average_light_level & MSB_MASK) >> SHIFT_8BIT);
}

int dptx_hdr_infoframe_sdp_send(struct dp_ctrl *dptx, const void __user *argp)
{
	int i;

	dpu_check_and_return(!dptx, -EINVAL, err, "[DP] dptx is NULL!\n");
	dpu_check_and_return((argp == NULL), -EINVAL, err, "[DP] argp is NULL!\n");

	dptx->hdr_infoframe.type_code = INFOFRAME_PACKET_TYPE_HDR;
	dptx->hdr_infoframe.version_number = HDR_INFOFRAME_VERSION;
	dptx->hdr_infoframe.length = HDR_INFOFRAME_LENGTH;

	memset(dptx->hdr_infoframe.data, 0x00, HDR_INFOFRAME_LENGTH);

	if (copy_from_user(&(dptx->hdr_metadata), (void __user *)argp, sizeof(dptx->hdr_metadata))) {
		dpu_pr_err("[DP] [%s]: copy arg failed!\n", __func__);
		return -EFAULT;
	}

	dptx_hdr_calculate_infoframe_data(dptx);

	for (i = 0; i < HDR_INFOFRAME_LENGTH; i++)
		dpu_pr_info("[DP] hdr_infoframe->data[%d] = 0x%02x", i, dptx->hdr_infoframe.data[i]);

	mutex_lock(&dptx->dptx_mutex);

	if (dptx->dptx_hdr_infoframe_set_reg)
		dptx->dptx_hdr_infoframe_set_reg(dptx, 1);

	mutex_unlock(&dptx->dptx_mutex);

	return 0;
}

static void fast_link_training(struct dp_ctrl *dptx)
{
	uint32_t pattern;

	dpu_pr_info("[DP] +\n");

	dptx_phy_set_pattern(dptx, DPTX_PHYIF_CTRL_TPS_1);
	udelay(500);

	switch (dptx->max_rate) {
	case DPTX_PHYIF_CTRL_EDP_RATE_R432:
	case DPTX_PHYIF_CTRL_RATE_HBR3:
		if (drm_dp_tps4_supported(dptx->rx_caps)) {
			pattern = DPTX_PHYIF_CTRL_TPS_4;
			break;
		}
		/* Fall through */
	case DPTX_PHYIF_CTRL_EDP_RATE_R324:
	case DPTX_PHYIF_CTRL_RATE_HBR2:
		if (drm_dp_tps3_supported(dptx->rx_caps)) {
			pattern = DPTX_PHYIF_CTRL_TPS_3;
			break;
		}
		/* Fall through */
	case DPTX_PHYIF_CTRL_EDP_RATE_R243:
	case DPTX_PHYIF_CTRL_EDP_RATE_R216:
	case DPTX_PHYIF_CTRL_RATE_RBR:
	case DPTX_PHYIF_CTRL_RATE_HBR:
		pattern = DPTX_PHYIF_CTRL_TPS_2;
		break;
	default:
		dpu_pr_err("[DP] Invalid rate %d\n", dptx->link.rate);
		return;
	}

	dptx_phy_set_pattern(dptx, pattern);
	udelay(500);
	dptx_phy_set_pattern(dptx, DPTX_PHYIF_CTRL_TPS_NONE);
	dpu_pr_info("[DP] -\n");
}

void dptx_fast_link(struct dp_ctrl *dptx, bool open)
{
	int retval;
	dpu_check_and_no_retval((dptx == NULL), err, "[DP] NULL Pointer\n");

	dpu_pr_info("[DP] start dptx_fast_link open: %s.\n", open ? "true" : "false");
	mutex_lock(&dptx->dptx_mutex);
	if (!open) {
		retval = dptx_write_dpcd(dptx, DP_SET_POWER, DP_SET_POWER_D3);
		if (retval) {
			dpu_pr_err("[DP] dptx_write_dpcd return value: %d.\n", retval);
			mutex_unlock(&dptx->dptx_mutex);
			return;
		}
		if (dptx->dptx_disable_default_video_stream)
			dptx->dptx_disable_default_video_stream(dptx, 0);
	} else {
		retval = dptx_write_dpcd(dptx, DP_SET_POWER, DP_SET_POWER_D0);
		if (retval) {
			dpu_pr_err("[DP] dptx_write_dpcd return value: %d.\n", retval);
			mutex_unlock(&dptx->dptx_mutex);
			return;
		}
		mdelay(10);
		fast_link_training(dptx);
		if (dptx->dptx_enable_default_video_stream)
			dptx->dptx_enable_default_video_stream(dptx, 0);
	}
	mutex_unlock(&dptx->dptx_mutex);
}

void dptx_link_layer_init(struct dp_ctrl *dptx)
{
	dpu_check_and_no_retval(!dptx, err, "[DP] dptx is NULL!\n");

	dptx_link_params_reset(dptx);

	dptx->handle_hotplug = handle_hotplug;
	dptx->handle_hotunplug = handle_hotunplug;
	dptx->dptx_power_handler = dptx_power_handler;
	dptx->dptx_dsc_check_rx_cap = dptx_dsc_check_rx_cap;
	dptx->dptx_dsc_para_init = dptx_dsc_para_init;

	dptx_ctrl_layer_init(dptx);
}
