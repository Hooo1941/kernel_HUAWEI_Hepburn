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
#include <linux/types.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <securec.h>

#include "hdmitx_vrr.h"

#include "dkmd_log.h"
#include "hdmitx_common.h"

#define HDMI_VRR_RATE_CONVERT_K 1000
#define HDMI_VRR_RATE_CONVERT_M 1000000
#define HDMI_CHECK_TIME_PERIOD 2000

#define REG_VRR_CTRL 0x80
#define HDMI_TIMING_GEN_VRR_EN_MASK    BIT(31)
#define HDMI_V_FRONT_MVRR_MASK         GENMASK(16, 1)
#define HDMI_TIMING_GEN_VRR_START_MASK BIT(0)

#define HDMITX_VRR_ERRO_VALUE 994

#define ceil(a, b) (((a) / (b)) + (((a) % (b)) > 0 ? 1 : 0))

static u32 cnt_fresh = 0;
static u32 vrr_total_interval = 16;
u32 g_enable_vrr = 0;

void hdmitx_vrr_enable(u32 enable_vrr)
{
	g_enable_vrr = enable_vrr;
	cnt_fresh = 0;
	dpu_pr_info("enable vrr = %u", enable_vrr);
	return;
}

static s32 hdmitx_get_vrr_mode_by_edid(struct hdmitx_ctrl *hdmitx)
{
	struct hdmi_detail *detail = &hdmitx->select_mode.detail;
	struct hdmitx_vrr_mode *vrr_mode = &hdmitx->hdmitx_vrr.vrr_mode;

	vrr_mode->pixel_clock = detail->pixel_clock;
	vrr_mode->h_total = detail->h_total;
	vrr_mode->v_total = detail->v_total;
	vrr_mode->field_rate_max = detail->field_rate;

	return 0;
}

static s32 hdmitx_get_vrr_rate(struct hdmitx_vrr_info *vrr_info, struct vrr_property *vrr_edid)
{
	if (vrr_info == NULL) {
		dpu_pr_err("vrr mode is NULL");
		return -1;
	}

	if (vrr_edid == NULL) {
		dpu_pr_err("vrr is NULL");
		return -1;
	}

	vrr_info->vrr_mode.field_rate_min = vrr_edid->vrr_min;

	if (vrr_edid->vrr_max != 0)
		vrr_info->vrr_mode.field_rate_max =
			vrr_edid->vrr_max > vrr_info->vrr_mode.field_rate_max ? vrr_info->vrr_mode.field_rate_max : vrr_edid->vrr_max;

	dpu_pr_info("vrr_min %u, vrr_max %u, field_rate_min %u, field_rate_max %u",
		vrr_edid->vrr_min,
		vrr_edid->vrr_max,
		vrr_info->vrr_mode.field_rate_min,
		vrr_info->vrr_mode.field_rate_max);
	return 0;
}

static void hdmitx_vrr_get_m_vrr(struct hdmitx_ctrl *hdmitx)
{
	u64 m_vrr_dividend;
	struct hdmitx_vrr_mode *vrr_mode = &hdmitx->hdmitx_vrr.vrr_mode;
	struct vrr_property *vrr_edid = &hdmitx->hdmitx_connector->vrr;
	u32 vic = hdmitx->select_mode.vic;
	s32 ret;

	if (vrr_mode == NULL || vrr_edid == NULL) {
		dpu_pr_err("vrr mode is NULL");
		return;
	}

	ret = hdmitx_get_vrr_mode_by_vic(vrr_mode, vic);
	if (ret != 0) {
		ret = hdmitx_get_vrr_mode_by_edid(hdmitx);
		if (ret != 0) {
			dpu_pr_err("can not get vrr mode");
			return;
		}
	}

	ret = hdmitx_get_vrr_rate(&hdmitx->hdmitx_vrr, vrr_edid);
	if (ret != 0) {
		dpu_pr_err("config vrr error");
		return;
	}

	m_vrr_dividend = vrr_mode->h_total * vrr_mode->field_rate_min * HDMITX_VRR_ERRO_VALUE;
	hdmitx->hdmitx_vrr.m_vrr =
		(u32)ceil((vrr_mode->pixel_clock * HDMI_VRR_RATE_CONVERT_M), m_vrr_dividend) - vrr_mode->v_total;

	dpu_pr_info("pixel_clock %u, h_total %u, v_total %u",
		vrr_mode->pixel_clock,
		vrr_mode->h_total,
		vrr_mode->v_total);
	dpu_pr_info("get mvrr = %u", hdmitx->hdmitx_vrr.m_vrr);

	return;
}

static enum hrtimer_restart test_adpative_sync_handler(struct hrtimer *timer)
{
	struct hdmitx_ctrl *hdmitx = NULL;
	u32 min_interval;
	u32 max_interval;

	if (timer == NULL) {
		dpu_pr_info("timer is NULL");
		return HRTIMER_NORESTART;
	}

	hdmitx = container_of(timer, struct hdmitx_ctrl, hdmitx_hrtimer);
	if (hdmitx == NULL) {
		dpu_pr_info("hdmitx is NULL");
		return HRTIMER_NORESTART;
	}
	if (unlikely((hdmitx->hdmitx_vrr.vrr_mode.field_rate_max == 0) ||
		(hdmitx->hdmitx_vrr.vrr_mode.field_rate_min == 0))) {
		dpu_pr_err("field_rate_max or field_rate_min is zero");
		return HRTIMER_NORESTART;
	}
	min_interval = HDMI_VRR_RATE_CONVERT_K / hdmitx->hdmitx_vrr.vrr_mode.field_rate_max;
	max_interval = HDMI_VRR_RATE_CONVERT_K / hdmitx->hdmitx_vrr.vrr_mode.field_rate_min;

	cnt_fresh++;
	if (cnt_fresh % 500 == 0) {
		vrr_total_interval++;
		if (vrr_total_interval >= max_interval)
			vrr_total_interval = min_interval;
		dpu_pr_info("[HDMI] Vsync interval = %u ms", vrr_total_interval);
	}

	hdmi_write_bits(hdmitx->sysctrl_base, REG_VRR_CTRL, HDMI_TIMING_GEN_VRR_START_MASK, 0x1);

	hrtimer_start(&hdmitx->hdmitx_hrtimer, ktime_set(0, vrr_total_interval * HDMI_VRR_RATE_CONVERT_M), HRTIMER_MODE_REL);
	return HRTIMER_NORESTART;
}

bool hdmitx_vrr_is_enable(struct hdmitx_ctrl *hdmitx)
{
	bool vrr_enable;

	if (hdmitx == NULL)
		return false;

	vrr_enable = (hdmitx->hdmitx_vrr.vrr_en == true) && (hdmitx->hdmitx_connector->vrr.vrr_min != 0);

	return vrr_enable;
}

void hdmitx_vrr_start(struct hdmitx_ctrl *hdmitx)
{
	if (hdmitx == NULL) {
		dpu_pr_info("hdmitx is NULL");
		return;
	}
	dpu_pr_info("+");

	hdmitx_vrr_get_m_vrr(hdmitx);
	hdmi_write_bits(hdmitx->sysctrl_base, REG_VRR_CTRL,
		HDMI_V_FRONT_MVRR_MASK, hdmitx->hdmitx_vrr.m_vrr);
	hdmi_write_bits(hdmitx->sysctrl_base, REG_VRR_CTRL,
		HDMI_TIMING_GEN_VRR_EN_MASK, 0x1);

	hrtimer_init(&hdmitx->hdmitx_hrtimer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	hdmitx->hdmitx_hrtimer.function = test_adpative_sync_handler;
	/* 2s delay */
	hrtimer_start(&hdmitx->hdmitx_hrtimer,
		ktime_set(HDMI_CHECK_TIME_PERIOD / HDMI_VRR_RATE_CONVERT_K,
			(HDMI_CHECK_TIME_PERIOD % HDMI_VRR_RATE_CONVERT_K) * HDMI_VRR_RATE_CONVERT_M),
		HRTIMER_MODE_REL);
	dpu_pr_info("hdmitx vrr is start");
	return;
}

void hdmitx_vrr_stop()
{
	struct hdmitx_ctrl *hdmitx = g_hdmitx;

	dpu_pr_info("+");
	if (hdmitx == NULL) {
		dpu_pr_info("hdmitx is NULL");
		return;
	}
	hrtimer_cancel(&hdmitx->hdmitx_hrtimer);
	hdmi_writel(hdmitx->sysctrl_base, REG_VRR_CTRL, 0x0);
}

#ifdef CONFIG_DKMD_DEBUG_ENABLE
EXPORT_SYMBOL_GPL(hdmitx_vrr_enable);
EXPORT_SYMBOL_GPL(hdmitx_vrr_start);
EXPORT_SYMBOL_GPL(hdmitx_vrr_stop);
#endif
