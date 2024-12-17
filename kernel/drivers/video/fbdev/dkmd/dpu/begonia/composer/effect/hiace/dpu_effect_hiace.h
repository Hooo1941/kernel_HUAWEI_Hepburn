/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
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

#ifndef DPU_EFFECT_HIACE_H
#define DPU_EFFECT_HIACE_H

#include <linux/wait.h>
#include <linux/mutex.h>
#include <linux/kthread.h>
#include <linux/semaphore.h>

#include "dkmd_dpu.h"

#define HIACE_HIST_TIMEOUT_MS 100000

struct composer;
struct dpu_composer;

struct hiace_hist_status {
	wait_queue_head_t wq_hist;
	struct mutex hist_lock;
	uint32_t valid_info; // bit0:local bit1:fna bit2:global
	bool new_hist;
	bool hist_stop; // fb blank
	char reservd[2];
};

struct dkmd_hiace_ctrl {
	struct semaphore hist_lock_sem;
	struct mutex hiace_lut_lock;
	struct dkmd_hiace_lut_data lut;

	struct dkmd_hiace_hist_data hist;
	struct hiace_hist_status status;
	struct mutex hist_status_lock;
	uint32_t sel_hdr_ab_shadow;
	uint32_t dpp_offset;

	bool hist_status_inited;
	bool hiace_lock_init;
	bool is_last_hist_read_intime;
	char reservd[1];

	void (*init_params)(struct dpu_composer *dpu_comp);
	void (*blank)(struct dkmd_hiace_ctrl *ctrl);
	void (*unblank)(struct dkmd_hiace_ctrl *ctrl);
	void (*queue_work)(struct dpu_composer *dpu_comp);
	void (*lut_apply)(struct dpu_composer *dpu_comp);

	int32_t (*set_lut)(struct dkmd_hiace_ctrl *ctrl, const void __user* argp);
	int32_t (*get_hist)(struct dkmd_hiace_ctrl *ctrl, void __user* argp);
	int32_t (*wake_up_hist)(struct dkmd_hiace_ctrl *ctrl);
};

enum ENUM_CE_MODE {
	CE_MODE_DISABLE = 0,
	CE_MODE_VIDEO,
	CE_MODE_IMAGE,
	CE_MODE_SINGLE,
	CE_MODE_COUNT,
};

void dpu_effect_hiace_init(struct dpu_composer *dpu_comp, uint32_t dpp_offset);
void dpu_effect_hiace_deinit(struct dpu_composer *dpu_comp);

#endif