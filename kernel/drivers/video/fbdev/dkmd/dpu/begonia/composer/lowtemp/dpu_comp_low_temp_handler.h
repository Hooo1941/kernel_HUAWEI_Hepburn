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
#ifndef DPU_COMP_LOW_TEMP_HANDLER_H
#define DPU_COMP_LOW_TEMP_HANDLER_H
#include <linux/workqueue.h>

struct dpu_composer;
struct comp_online_present;

struct dpu_low_temp_ctrl {
	uint32_t low_temp_state_pre;
	struct workqueue_struct *low_temp_wq;
	struct work_struct low_temp_work;
	struct dpu_composer *dpu_comp;
};

void reset_low_temperature_state(struct dpu_composer *dpu_comp);
uint32_t check_low_temperature(void);
void dpu_low_temp_register(struct dpu_composer *dpu_comp, struct comp_online_present *present);
void dpu_low_temp_unregister(struct dpu_composer *dpu_comp, struct comp_online_present *present);
#endif