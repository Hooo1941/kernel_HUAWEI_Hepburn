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
#ifndef DPU_COMP_FUSA_HANDLER_H
#define DPU_COMP_FUSA_HANDLER_H

#include "dkmd_ffd.h"
#include <linux/workqueue.h>
#include <linux/spinlock.h>

struct dpu_composer;
struct composer;

#define MODE0_ROI_CFG_BYTES_CONSTRAINT (20)

struct dpu_fusa_ctrl {
	bool fusa_inited;
	bool ffd_en;
	struct dkmd_ffd_cfg ffd_cfg;
	struct dpu_composer *dpu_comp;
	uint32_t ffd_result; // bit0 indicates roi0, and so on
	spinlock_t ffd_result_lock;
	struct workqueue_struct *ffd_result_wq;
	struct work_struct ffd_result_work;
	struct workqueue_struct *ffd_event_wq;
	struct work_struct ffd_event_work;
};

int32_t dpu_fusa_get_ffd_info(struct composer *comp, struct dkmd_ffd_cfg *info);
void dpu_comp_fusa_register(struct dpu_composer *dpu_comp);
void dpu_comp_fusa_unregister(struct dpu_composer *dpu_comp);
#endif