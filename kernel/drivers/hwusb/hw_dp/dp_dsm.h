/*
 * dp_dsm.h
 *
 * dsm for dp module
 *
 * Copyright (c) 2021-2022 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#ifndef __DP_DSM_H__
#define __DP_DSM_H__

#include <linux/types.h>
#include <chipset_common/hwusb/hw_dp/dp_interface.h>

#if IS_ENABLED(CONFIG_HUAWEI_DP_QCOM)
#define DP_DSM_DPCD_RX_CAPS_SIZE 16
#elif defined CONFIG_HUAWEI_DP_HISI
#define DP_DSM_DPCD_RX_CAPS_SIZE 256
#else
#define DP_DSM_DPCD_RX_CAPS_SIZE 0
#endif

#define DP_DSM_EDID_BLOCK_SIZE   128
#define DP_DSM_VS_PE_NUM         4
#define DP_DSM_VS_PE_LEN         2
#define DP_DSM_VS_PE_MASK        0xF

enum dp_imonitor_type {
	DP_IMONITOR_TYPE,
	/* link success */
	DP_IMONITOR_BASIC_INFO = DP_IMONITOR_TYPE,
	DP_IMONITOR_TIME_INFO,
	DP_IMONITOR_EXTEND_INFO,
	/* link failed */
	DP_IMONITOR_HPD,
	DP_IMONITOR_LINK_TRAINING,
	DP_IMONITOR_HOTPLUG,
	DP_IMONITOR_HDCP,

	DP_IMONITOR_TYPE_NUM,
};

enum dp_dmd_type {
	DP_DMD_TYPE,
	DP_DMD_LINK_SUCCESS = DP_DMD_TYPE,
	DP_DMD_LINK_FAILED,

	DP_DMD_TYPE_NUM,
};

/* dp link state: report event to PC suite */
enum dp_link_state {
	DP_LINK_STATE_CABLE_IN,         /* typec cable in */
	DP_LINK_STATE_CABLE_OUT,        /* typec cable out */
	DP_LINK_STATE_MULTI_HPD,        /* report hpd repeatedly from PD module */
	DP_LINK_STATE_AUX_FAILED,       /* read/write regs failed by aux channel */
	DP_LINK_STATE_SAFE_MODE,        /* safe mode, 640*480 */
	DP_LINK_STATE_EDID_FAILED,      /* read edid failed, 1024*768 */
	DP_LINK_STATE_LINK_FAILED,      /* link training failed */
	DP_LINK_STATE_LINK_RETRAINING,  /* link retraining by short irq */
	DP_LINK_STATE_HDCP_FAILED,      /* hdcp auth failed in DRM files */

	/* for MMIE test */
	DP_LINK_STATE_HPD_NOT_EXISTED,  /* hpd not existed */
	DP_LINK_STATE_LINK_REDUCE_RATE, /* reduce lanes or rate in link training */
	DP_LINK_STATE_INVALID_COMBINATIONS, /* combinations not support 4k@60fps */

	DP_LINK_STATE_MAX,
};

#define DP_HPD_REPEATED_THRESHOLD  6    /* 3 --> 6, reduce hpd report num */
#define DP_SAFE_MODE_DISPLAY_WIDTH 640
#define DP_SAFE_MODE_DISPLAY_HIGH  480
#define DP_LINK_EVENT_BUF_MAX      64

void dp_link_state_event(enum dp_link_state state, bool dptx_vr_flag);

#if IS_ENABLED(CONFIG_HUAWEI_DP_DSM)
int  dp_dsm_init(void);
void dp_dsm_exit(void);
void dp_imonitor_set_pd_event(uint8_t irq_type, uint8_t cur_mode,
	uint8_t mode_type, uint8_t dev_type, uint8_t typec_orien);
void dp_imonitor_set_param(enum dp_imonitor_param param, void *data);
void dp_imonitor_set_param_aux_rw(bool rw, bool i2c, uint32_t addr,
	uint32_t len, int retval);
void dp_imonitor_set_param_timing(uint16_t h_active, uint16_t v_active,
	uint32_t pixel_clock, uint8_t vesa_id);
void dp_imonitor_set_param_err_count(uint16_t lane0_err,
	uint16_t lane1_err, uint16_t lane2_err, uint16_t lane3_err);

/*
 * 1. record vs and pe for imonitor
 * 2. reset vs and pe force when to debug(eng build)
 */
void dp_imonitor_set_param_vs_pe(int index, uint8_t *vs, uint8_t *pe);

/*
 * 1. user version: this is empty function.
 * 2. not user version: support reset resolution by user_mode and user_format.
 */
void dp_imonitor_set_param_resolution(uint8_t *user_mode, uint8_t *user_format);

/*
 * NOTE: DON'T CALL THE FUNCTION(dp_imonitor_report) IN IRQ HANDLER,
 * otherwise it might be panic.
 * REASON: can't use vmalloc in irq handler,
 * but use vmalloc in imonitor_create_eventobj func.
 * for info report by imonitor
 */
void dp_imonitor_report(enum dp_imonitor_type type, void *data);
/* for info report by dmd */
void dp_dmd_report(enum dp_dmd_type type, const char *fmt, ...);
void dp_set_hotplug_state(enum dp_link_state state);
void dp_set_dptx_vr_status(bool dptx_vr);
void dp_set_monitor_info(char *monitor, int len, uint8_t *edid);
#else
static inline int  dp_dsm_init(void)
{
	return 0;
}

static inline void dp_dsm_exit(void)
{
}

static inline void dp_imonitor_set_pd_event(uint8_t irq_type,
	uint8_t cur_mode, uint8_t mode_type, uint8_t dev_type, uint8_t typec_orien)
{
}
static inline void dp_imonitor_set_param(enum dp_imonitor_param param,
	void *data)
{
}
static inline void dp_imonitor_set_param_aux_rw(bool rw, bool i2c,
	uint32_t addr, uint32_t len, int retval)
{
}
static inline void dp_imonitor_set_param_timing(uint16_t h_active,
	uint16_t v_active, uint32_t pixel_clock, uint8_t vesa_id)
{
}
static inline void dp_imonitor_set_param_err_count(uint16_t lane0_err,
	uint16_t lane1_err, uint16_t lane2_err, uint16_t lane3_err)
{
}
static inline void dp_imonitor_set_param_vs_pe(int index,
	uint8_t *vs, uint8_t *pe)
{
}
static inline void dp_imonitor_set_param_resolution(
	uint8_t *user_mode, uint8_t *user_format)
{
}
static inline void dp_imonitor_report(enum dp_imonitor_type type, void *data)
{
}
static inline void dp_dmd_report(enum dp_dmd_type type, const char *fmt, ...)
{
}
static inline void dp_set_hotplug_state(enum dp_link_state state)
{
}
static inline void dp_set_dptx_vr_status(bool dptx_vr)
{
}
static inline void dp_set_monitor_info(char *monitor, int len,
	uint8_t *edid)
{
}
#endif /* CONFIG_HUAWEI_DP_DSM */

int dp_get_pd_event_result(char *buf, int size);
int dp_get_hotplug_result(char *buf, int size);
int dp_get_timing_info_result(char *buf, int size);
int dp_get_vs_pe_result(char *buf, int size);

#endif /* __DP_DSM_H__ */
