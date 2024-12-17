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

#ifndef DKMD_COMP_H
#define DKMD_COMP_H

#include <linux/ktime.h>
#include <linux/semaphore.h>
#include "dkmd_object.h"
#include "chrdev/dkmd_sysfs.h"

#define DPU_COMPOSER_HOLD_TIME (1000UL * 3600 * 24 * 30) // 30 day

struct dkmd_base_frame;
struct dkmd_network;
struct product_config;
struct alsc_info;
struct dkmd_ffd_cfg;
struct disp_present_fence;

enum dpu_comp_type {
	DEVICE_COMP_PRIMARY_ID    = 0,
	DEVICE_COMP_BUILTIN_ID    = 1,
	DEVICE_COMP_DP_ID         = 2,
	DEVICE_COMP_HDMI_ID       = 3,
	DEVICE_COMP_VIRTUAL_ID    = 4,
	DEVICE_COMP_DP_ID_1       = 5,
	DEVICE_COMP_DP_ID_2       = 6,
	DEVICE_COMP_MAX_COUNT     = 8,
};

enum composer_off_mode {
	COMPOSER_OFF_MODE_BLANK = 0,
	COMPOSER_OFF_MODE_SUSPEND,
};

struct composer {
	struct dkmd_object_info base;
	uint32_t index;
	uint32_t comp_frame_index;
	uint32_t fastboot_display_enabled;
	uint32_t comp_err_status;

	ktime_t power_on_timestamp;
	ktime_t power_off_timestamp;

	bool power_on;
	bool is_fake_power_off;
	uint8_t reserved[2];
	struct semaphore blank_sem;
	void *device_data;

	int32_t (*set_fastboot)(struct composer *comp);
	int32_t (*on)(struct composer *comp);
	int32_t (*off)(struct composer *comp, int32_t off_mode /* off_mode */);
	void (*release)(struct composer *comp);

	int32_t (*wait_for_blank)(struct composer *comp);
	void (*restore_fast_unblank_status)(struct composer *comp);

	int32_t (*create_fence)(struct composer *comp, struct disp_present_fence *fence);
	int32_t (*present)(struct composer *comp, void *frame);
	int32_t (*release_fence)(struct composer *comp, int32_t fd);

	int32_t (*get_product_config)(struct composer *comp, struct product_config *out_config);
	int32_t (*get_sysfs_attrs)(struct composer *comp, struct dkmd_attr **out_attr);

	int32_t (*get_hdr_mean)(struct composer *comp, uint32_t *hdr_mean);

	int32_t (*register_alsc)(struct composer *comp, uint32_t index, uint32_t dpp_offset);
	int32_t (*unregister_alsc)(struct composer *comp);
	int32_t (*get_alsc_info)(struct composer *comp, struct alsc_info *info);

	int32_t (*get_fusa_info)(struct composer *comp, struct dkmd_ffd_cfg *info);

	int32_t (*effect_hiace_get_hist)(struct composer *comp, void __user* argp);
	int32_t (*effect_hiace_set_lut)(struct composer *comp, const void __user* argp);
	int32_t (*effect_wake_up_hiace_hist)(struct composer *comp);
	int32_t (*set_safe_frm_rate)(struct composer *comp, uint32_t safe_frm_rate);

	int32_t (*hdcp_increase_counter)(struct composer *comp);
	int32_t (*hdcp_decrease_counter)(struct composer *comp);

	int32_t (*set_active_rect)(struct composer *comp, uint32_t ppcConfigId);
};

#endif
