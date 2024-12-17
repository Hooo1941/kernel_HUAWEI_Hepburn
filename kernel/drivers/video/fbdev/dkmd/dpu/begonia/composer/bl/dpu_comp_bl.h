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
#ifndef DPU_COMP_BL_H
#define DPU_COMP_BL_H

#include <linux/types.h>
#include <linux/ktime.h>
#include <linux/semaphore.h>
#include <linux/workqueue.h>
#include <linux/leds.h>
#include "chrdev/dkmd_sysfs.h"
#include "peri/dkmd_connector.h"

extern unsigned int get_boot_into_recovery_flag(void);

/*
 * each composer have a bl_ctrl
 */
struct dpu_bl_ctrl {
	uint32_t is_recovery_mode;
	int32_t bl_updated;       /* after the first frame fresh to set backlight then bl_updated set 1 */
	uint32_t bl_level;
	uint32_t bl_level_old;

	ktime_t bl_timestamp;
	struct delayed_work bl_worker;
	struct semaphore bl_sem;

	struct dkmd_connector_info *conn_info;
	struct dpu_composer *parent_composer;
};

/*
 * init bl_ctrl，and add backlight attrs
 * bl_ctrl
 * attrs，dpu_composer's attrs
 */
void dpu_backlight_init(struct dpu_bl_ctrl *bl_ctrl, struct dkmd_attr *attrs, struct dpu_composer *dpu_comp);


/*
 * set bl value which was saved at dpu_bl_ctrl
 * bl_ctrl, dpu_composer's bl_ctrl
 * enforce, false: not enforce
 *         true: enforce
 */
void dpu_backlight_update(struct dpu_bl_ctrl *bl_ctrl, bool enforce);

/*
 * close backlight
 */
void dpu_backlight_cancel(struct dpu_bl_ctrl *bl_ctrl);

/**
 * update bl_level
 *
 */
void dpu_backlight_update_level(struct dpu_bl_ctrl *bl_ctrl, struct disp_effect_params *effect_params);

/**
 * debug level
 *
 */
void dpu_backlight_debug_level(struct disp_effect_params *effect_params);
#endif