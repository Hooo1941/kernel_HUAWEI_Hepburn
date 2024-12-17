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

#ifndef DKMD_DPU_H
#define DKMD_DPU_H

#include <linux/types.h>
#include "dkmd_user_panel_info.h"
#include "dkmd_hiace_info.h"
#include "dkmd_ffd.h"

#define DISP_IOCTL_MAGIC 'D'

#define DISP_CREATE_FENCE _IOWR(DISP_IOCTL_MAGIC, DISP_IOCTL_CREATE_FENCE, struct disp_present_fence)
#define DISP_RELEASE_FENCE _IOW(DISP_IOCTL_MAGIC, DISP_IOCTL_RELEASE_FENCE, int32_t)
#define DISP_PRESENT _IOW(DISP_IOCTL_MAGIC, DISP_IOCTL_PRESENT, struct disp_frame)
#define DISP_IOBLANK _IOW(DISP_IOCTL_MAGIC, DISP_IOCTL_IOBLANK, int)
#define DISP_GET_HDR_MEAN _IOR(DISP_IOCTL_MAGIC, DISP_IOCTL_GET_HDR_MEAN, uint32_t)
#define DISP_SAFE_FRM_RATE _IOW(DISP_IOCTL_MAGIC, DISP_IOCTL_SAFE_FRM_RATE, int)
#define DISP_SET_ACTIVE_RECT _IOW(DISP_IOCTL_MAGIC, DISP_IOCTL_SET_ACTIVE_RECT, uint32_t)
/**
 * @brief this cmd was used to get product config items,
 *        those itmes may were defined at dts or panel,
 *        each online scene must have product config items.
 */
#define DISP_GET_PRODUCT_CONFIG _IOR(DISP_IOCTL_MAGIC, DISP_IOCTL_GET_PRODUCT_CONFIG, struct product_config)

#define DISP_GET_ALSC_INFO _IOR(DISP_IOCTL_MAGIC, DISP_IOCTL_GET_ALSC_INFO, struct alsc_info)
#define DISP_GET_FUSA_INFO _IOR(DISP_IOCTL_MAGIC, DISP_IOCTL_GET_FUSA_INFO, struct dkmd_ffd_cfg)

// for hiace
#define DISP_SET_HIACE_HIST_CONFIG \
	_IOW(DISP_IOCTL_MAGIC, DISP_IOCTL_SET_HIACE_HIST_CONFIG, struct dkmd_hiace_hist_config)
#define DISP_GET_HIACE_HIST _IOR(DISP_IOCTL_MAGIC, DISP_IOCTL_GET_HIACE_HIST, struct dkmd_hiace_hist_data)
#define DISP_SET_HIACE_LUT _IOW(DISP_IOCTL_MAGIC, DISP_IOCTL_SET_HIACE_LUT, struct dkmd_hiace_lut_data)
#define DISP_WAKE_UP_HIACE_HIST _IOW(DISP_IOCTL_MAGIC, DISP_IOCTL_WAKE_UP_HIACE_HIST, int)

#define DISP_HDCP_IN _IOW(DISP_IOCTL_MAGIC, DISP_IOCTL_HDCP_IN, int)
#define DISP_HDCP_OUT _IOW(DISP_IOCTL_MAGIC, DISP_IOCTL_HDCP_OUT, int)

enum {
	DISP_IOCTL_CREATE_FENCE = 0x10,
	DISP_IOCTL_PRESENT,
	DISP_IOCTL_IOBLANK,
	DISP_IOCTL_GET_PRODUCT_CONFIG,
	DISP_IOCTL_GET_HDR_MEAN,
	DISP_IOCTL_GET_ALSC_INFO,
	DISP_IOCTL_SET_HIACE_HIST_CONFIG,
	DISP_IOCTL_GET_HIACE_HIST,
	DISP_IOCTL_SET_HIACE_LUT,
	DISP_IOCTL_WAKE_UP_HIACE_HIST,
	DISP_IOCTL_SAFE_FRM_RATE,
	DISP_IOCTL_RELEASE_FENCE,
	DISP_IOCTL_HDCP_IN,
	DISP_IOCTL_HDCP_OUT,
	DISP_IOCTL_GET_FUSA_INFO,
	DISP_IOCTL_SET_ACTIVE_RECT
};

enum {
	ASYNC_MODE_NONE, // for synchronous mode
	ASYNC_MODE_WITH_OUT_FENCE,
	ASYNC_MODE_NO_OUT_FENCE,
	ASYNC_MODE_INVALID,
};

// should be consistent with linux/fb.h
enum {
	/* screen: unblanked, hsync: on,  vsync: on */
	DISP_BLANK_UNBLANK,
	/* screen: blanked,   hsync: on,  vsync: on, Notice: this is used by DISP_BLANK_FAKE_OFF, do not reuse this */
	DISP_BLANK_NORMAL,
	/* screen: blanked,   hsync: on,  vsync: off */
	DISP_BLANK_VSYNC_SUSPEND,
	/* screen: blanked,   hsync: off, vsync: on */
	DISP_BLANK_HSYNC_SUSPEND,
	/* screen: blanked,   hsync: off, vsync: off */
	DISP_BLANK_POWERDOWN,
	/* screen: no change,   hsync: off, vsync: off */
	DISP_BLANK_FAKE_OFF = DISP_BLANK_NORMAL,
};

enum {
	DISP_FOLD_TYPE_UNKNOWN = 0,
	DISP_FOLD_TYPE_FOLD,
	DISP_FOLD_TYPE_FLIP,
};

enum {
	DISP_EFFECT_WITH_BL = 0,
	DISP_EFFECT_TE2_FIXED, /* fixed 120HZ */
	DISP_EFFECT_TE2_FOLLOWED, /* followed refresh frequency */
	DISP_EFFECT_WITH_OTHER,
	DISP_EFFECT_WITH_INVALID,
};

enum dfr_mode {
	DFR_MODE_CONSTANT,
	DFR_MODE_LONG_V,
	DFR_MODE_LONG_H,
	DFR_MODE_TE_SKIP_BY_MCU,
	DFR_MODE_TE_SKIP_BY_ACPU,
	DFR_MODE_LONG_VH, // both longv and longh exist
	DFR_MODE_LONGH_TE_SKIP_BY_MCU,
	DFR_MODE_INVALID
};

// should be consistent with linux/fb.h
#define DISP_GFX_GET_FIX_INFO 0x4602
#define DISP_GFX_GET_VAR_INFO 0x4600
#define DISP_GFX_PUT_VAR_INFO 0x4601
#define DISP_GFX_PAN_DISPLAY 0x4606
#define DISP_GFX_IOBLANK 0x4611
struct disp_layer {
	int32_t share_fd;
	int32_t acquired_fence;
	uint32_t format;
	uint32_t stride;
	bool is_protected_mode;
};

struct vote_freq_info {
	uint64_t current_total_freq;
	uint64_t sdma_freq;
};

struct intra_frame_dvfs_info {
	uint32_t perf_level;
	int32_t scene_id;
	struct vote_freq_info vote_freq_info;
	uint32_t frame_rate;
	uint32_t sw_dvfs_frm_rate;
	bool is_supported_intra_dvfs;
};

#define EFFECT_VALUES_MAX_LEN 5
struct disp_effect_item {
	uint32_t effect_type;
	uint32_t effect_len;
	int32_t effect_values[EFFECT_VALUES_MAX_LEN];
};

#define EFFECT_PARAMS_MAX_NUM 3
struct disp_effect_params {
	uint32_t effect_num;
	uint32_t delay;
	struct disp_effect_item params[EFFECT_PARAMS_MAX_NUM];
};

struct disp_present_fence {
	int32_t fence_fd;
	uint64_t fence_pt;
};

#define DISP_LAYER_MAX_COUNT 32

struct disp_frame {
	int32_t scene_id;
	uint32_t cmdlist_id;
	uint32_t layer_count;
	int32_t protected_wch_id;  // add for drm layers config
	struct disp_layer layer[DISP_LAYER_MAX_COUNT];

	/* for offline compose output buffer release signal */
	int32_t out_present_fence;
	uint32_t frame_index;
	uint32_t active_frame_rate;  // for ltpo dynamic frame rate
	struct disp_effect_params effect_params;
	struct dkmd_rect disp_rect; // display rect on screen
	struct intra_frame_dvfs_info dvfs_info;
	uint32_t async_mode;
	uint32_t tui_ready_status;
	uint32_t alsc_en;
	uint64_t present_fence_pt;
	uint32_t ppc_config_id; /* panel partial control, active display area id */
};

#define SCENE_SPLIT_MAX 4
enum split_mode {
	SCENE_SPLIT_MODE_NONE = 0,
	SCENE_SPLIT_MODE_V,
	SCENE_SPLIT_MODE_H,
};

// dimension
enum dim_id {
	DIMENSION_NORMAL = 0,
	DIMENSION_MEDIA,
	DIMENSION_LOW,
	DIMENSION_RESERVED,
	DIMENSION_ID_MAX = 4
};

enum fps_id {
	FPS_NORMAL = 0,
	FPS_LEVEL1,
	FPS_LEVEL2,
	FPS_LEVEL3,
	FPS_LEVEL4,
	FPS_LEVEL5,
	FPS_LEVEL6,
	FPS_LEVEL7,
	FPS_LEVEL_MAX,
};

struct rog_dim {
	int32_t width;
	int32_t height;
};

enum color_mode {
	COLOR_MODE_NATIVE = 0,
	COLOR_MODE_STANDARD_BT601_625 = 1,
	COLOR_MODE_STANDARD_BT601_625_UNADJUSTED = 2,
	COLOR_MODE_STANDARD_BT601_525 = 3,
	COLOR_MODE_STANDARD_BT601_525_UNADJUSTED = 4,
	COLOR_MODE_STANDARD_BT709 = 5,
	COLOR_MODE_DCI_P3 = 6,
	COLOR_MODE_SRGB = 7,
	COLOR_MODE_ADOBE_RGB = 8,
	COLOR_MODE_DISPLAY_P3 = 9,
	COLOR_MODE_BT2020 = 10,
	COLOR_MODE_BT2100_PQ = 11,
	COLOR_MODE_BT2100_HLG = 12,
};

#define COLOR_MODE_COUNT_MAX 5

union feature_bits {
	uint64_t value;
	struct {
		uint64_t enable_hdr10 : 1;
		uint64_t enable_dsc : 1;
		uint64_t enable_spr : 1;
		uint64_t enable_lbuf_reserve : 1;
		uint64_t enable_online_async : 1;
		uint64_t reserved : 59;
	} bits;
};

enum driver_framework {
	DRV_FB = 0,
	DRV_CHR,
	DRV_DRM,
	DRV_VIRTUAL_FB
};

// used for display's driver and device feature
union driver_bits {
	uint64_t value;
	struct {
		uint64_t drv_framework : 3;
		uint64_t is_pluggable  : 1;
		uint64_t reserved      : 60;
	} bits;
};

union fold_info_bits {
	uint32_t value;
	struct {
		uint32_t fold_type : 3;
		uint32_t reserved : 29;
	} bits;
};

struct alsc_info {
	uint32_t alsc_en;
	uint32_t alsc_addr;
	uint32_t alsc_size;
	uint32_t bl_update;
};

#define ONLINE_SCENE_MAX 4
#define DEFAULT_OPR_MASK 0xFFFFFFFFFFFFFFFF

struct support_scene_info {
	uint32_t scene_count;
	uint32_t scene_id[ONLINE_SCENE_MAX];
};

#define SPLIT_SCREEN_MAX 4

enum ppc_config_id_type {
	PPC_CONFIG_ID_F_MODE = 0,
	PPC_CONFIG_ID_M_MODE = 1,
	PPC_CONFIG_ID_G_MODE = 2,
	PPC_CONFIG_ID_CNT = 3
};

struct dpu_ppc_config_id_rect_info {
	uint32_t id;
	struct dkmd_rect_coord rect;
};

/**
 * @brief color_modes is must be enum color_mode's value
 *
 */
struct product_config {
	// fix information
	uint32_t width;
	uint32_t sub_width[SPLIT_SCREEN_MAX];
	uint32_t height;
	uint32_t sub_height[SPLIT_SCREEN_MAX];
	union driver_bits drv_feature; // driver and device feature

	uint16_t display_count;
	uint8_t split_mode;
	uint8_t split_count;
	uint16_t split_ratio[SCENE_SPLIT_MAX];
	union feature_bits feature_switch; // product feature

	// var information
	union fold_info_bits fold_info;
	uint32_t dim_info_count;
	struct rog_dim dim_info[DIMENSION_ID_MAX];
	struct rog_dim sub_dim_info[SPLIT_SCREEN_MAX][DIMENSION_ID_MAX];

	uint32_t fps_info_count;
	uint32_t fps_info[FPS_LEVEL_MAX];
	uint32_t fps_of_longv_for_dvfs;

	uint32_t color_mode_count;
	uint32_t color_modes[COLOR_MODE_COUNT_MAX];

	uint32_t dsc_out_width;
	uint32_t dsc_out_height;
	uint32_t spr_ctrl;
	// spr top bottom config for red/green/blue
	uint32_t spr_border_r_tb;
	uint32_t spr_border_g_tb;
	uint32_t spr_border_b_tb;

	struct user_panel_info user_pinfo;
	bool is_support_intra_dvfs;

	struct support_scene_info scene_info;
	uint64_t opr_policy;
	uint8_t compose_policy;
	uint8_t rsv[3];
	uint32_t dfr_mode;
	struct dpu_ppc_config_id_rect_info ppc_config_id_rect_info[PPC_CONFIG_ID_CNT];
	uint32_t ppc_config_id_cnt; // reserved for other fold panel
};
#endif
