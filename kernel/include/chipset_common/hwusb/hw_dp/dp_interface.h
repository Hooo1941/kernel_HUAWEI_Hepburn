/*
 * dp_interface.h
 *
 * dp interface header file
 *
 * Copyright (c) 2021-2022 Huawei Technologies Co., Ltd.
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

#ifndef __DP_INTERFACE_H__
#define __DP_INTERFACE_H__

#include <linux/kernel.h>
#include <linux/string.h>
#include <huawei_platform/log/imonitor.h>

enum dp_imonitor_param {
	DP_PARAM_TIME,
	DP_PARAM_TIME_START = DP_PARAM_TIME,
	DP_PARAM_TIME_LINK_SUCC,
	DP_PARAM_TIME_STOP,
	DP_PARAM_TIME_MAX,

	DP_PARAM_BASIC,
	DP_PARAM_WIDTH = DP_PARAM_BASIC,
	DP_PARAM_HIGH,
	DP_PARAM_MAX_WIDTH,
	DP_PARAM_MAX_HIGH,
	DP_PARAM_PIXEL_CLOCK,
	DP_PARAM_FPS,
	DP_PARAM_MAX_RATE,
	DP_PARAM_MAX_LANES,
	DP_PARAM_LINK_RATE,     /* current rate */
	DP_PARAM_LINK_LANES,    /* current lanes */
	DP_PARAM_TU,
	DP_PARAM_SOURCE_MODE,   /* same or diff source mode */
	DP_PARAM_USER_MODE,     /* vesa_id */
	DP_PARAM_USER_FORMAT,
	DP_PARAM_BASIC_AUDIO,   /* whether or not to support audio(only video) */
	DP_PARAM_BASIC_MAX,

	DP_PARAM_EXTEND,
	DP_PARAM_EDID = DP_PARAM_EXTEND,
	DP_PARAM_EDID_BLOCK0 = DP_PARAM_EDID,
	DP_PARAM_EDID_BLOCK1,
	DP_PARAM_EDID_BLOCK2,
	DP_PARAM_EDID_BLOCK3,
	DP_PARAM_EDID_MAX,
	DP_PARAM_DPCD_RX_CAPS,
	DP_PARAM_EXTEND_MAX,

	DP_PARAM_HDCP,
	DP_PARAM_HDCP_VERSION = DP_PARAM_HDCP,
	DP_PARAM_HDCP_KEY_S,    /* hdcp key authentication success */
	DP_PARAM_HDCP_KEY_F,    /* hdcp key authentication failed */
	DP_PARAM_HDCP_MAX,

	DP_PARAM_DIAG,
	DP_PARAM_LINK_RETRAINING = DP_PARAM_DIAG,
	DP_PARAM_SAFE_MODE,     /* whether or not to be safe_mode(force display) */
	DP_PARAM_READ_EDID_FAILED,
	DP_PARAM_LINK_TRAINING_FAILED,
	DP_PARAM_LINK_RETRAINING_FAILED,
	DP_PARAM_HOTPLUG_RETVAL, /* hotplug success or failed */
	DP_PARAM_IRQ_VECTOR,
	DP_PARAM_SOURCE_SWITCH,
	DP_PARAM_DSS_PWRON_FAILED,
	DP_PARAM_DIAG_MAX,

	DP_PARAM_NUM,
	DP_PARAM_TIME_NUM = DP_PARAM_TIME_MAX - DP_PARAM_TIME,
	DP_PARAM_EDID_NUM = DP_PARAM_EDID_MAX - DP_PARAM_EDID,
};

typedef int (*switch_source_handler)(void);

#if IS_ENABLED(CONFIG_HUAWEI_DP_SOURCE)
/* Interface provided by hw, include data monitor, event report, and etc. */
void huawei_dp_imonitor_set_param(enum dp_imonitor_param param,
	void *data);
void huawei_dp_imonitor_set_param_aux_rw(bool rw, bool i2c,
	uint32_t addr, uint32_t len, int retval);
void huawei_dp_imonitor_set_param_timing(uint16_t h_active,
	uint16_t v_active, uint32_t pixel_clock);
void huawei_dp_imonitor_set_param_vs_pe(int index, uint8_t *vs,
	uint8_t *pe);
bool huawei_dp_factory_mode_is_enable(void);
void huawei_dp_factory_link_cr_or_ch_eq_fail(bool is_cr);
bool huawei_dp_factory_is_4k_60fps(uint8_t rate, uint8_t lanes,
	uint16_t h_active, uint16_t v_active, uint8_t fps);
bool huawei_dp_is_vrefresh_valid(uint32_t vrefresh);
void huawei_dp_register_switch_handler(switch_source_handler handler);

#ifdef CONFIG_VR_DISPLAY
void huawei_dp_set_dptx_vr_status(bool dptx_vr);
#endif /* CONFIG_VR_DISPLAY */

int huawei_dp_get_current_dp_source_mode(void);
void huawei_dp_imonitor_set_pd_event(uint8_t irq_type,
	uint8_t cur_mode, uint8_t mode_type, uint8_t dev_type, uint8_t typec_orien);
void huawei_dp_save_edid(const uint8_t *edid_buf, uint32_t buf_len);
#else
static inline void huawei_dp_imonitor_set_param(
	enum dp_imonitor_param param, void *data)
{
}

static inline void huawei_dp_imonitor_set_param_aux_rw(bool rw,
	bool i2c, uint32_t addr, uint32_t len, int retval)
{
}

static inline void huawei_dp_imonitor_set_param_timing(
	uint16_t h_active, uint16_t v_active, uint32_t pixel_clock)
{
}

static inline void huawei_dp_imonitor_set_param_vs_pe(int index,
	uint8_t *vs, uint8_t *pe)
{
}

static inline bool huawei_dp_factory_mode_is_enable(void)
{
	return false;
}

static inline void huawei_dp_factory_link_cr_or_ch_eq_fail(
	bool is_cr)
{
}

static inline bool huawei_dp_factory_is_4k_60fps(uint8_t rate, uint8_t lanes,
	uint16_t h_active, uint16_t v_active, uint8_t fps)
{
	return true;
}

static inline bool huawei_dp_is_vrefresh_valid(uint32_t vrefresh)
{
	return true;
}

static inline void huawei_dp_register_switch_handler(
	switch_source_handler handler)
{
}

#ifdef CONFIG_VR_DISPLAY
static inline void huawei_dp_set_dptx_vr_status(bool dptx_vr) {}
#endif /* CONFIG_VR_DISPLAY */

static inline int huawei_dp_get_current_dp_source_mode(void)
{
	return SAME_SOURCE;
}

static inline void huawei_dp_imonitor_set_pd_event(uint8_t irq_type,
	uint8_t cur_mode, uint8_t mode_type, uint8_t dev_type, uint8_t typec_orien)
{
}

static inline void huawei_dp_save_edid(const uint8_t *edid_buf,
	uint32_t buf_len)
{
}
#endif /* HUAWEI_DP_SOURCE */

#if IS_ENABLED(CONFIG_HUAWEI_DP_LAHAINA)
#include <dp/dp_catalog.h>
#include <dp/dp_link.h>
#include <dp/dp_panel.h>

void qcom_dp_set_basic_info(struct dp_panel *panel,
	struct dp_link *link, struct dp_catalog *catalog);
void qcom_dp_set_sink_caps(struct dp_panel *dp_panel);
#endif /* HUAWEI_DP_LAHAINA */

#if IS_ENABLED(CONFIG_HUAWEI_DP_LAHAINA) || IS_ENABLED(CONFIG_HUAWEI_DP_TARO)
void qcom_dp_set_pd_event(uint8_t cur_hpd_high,
	uint8_t connected, uint8_t dp_data, uint8_t orientation);
#else
static inline void qcom_dp_set_pd_event(uint8_t cur_hpd_high,
	uint8_t connected, uint8_t dp_data, uint8_t orientation)
{
}
#endif /* HUAWEI_DP_LAHAINA || HUAWEI_DP_TARO */

#endif /* DP_INTERFACE_H */
