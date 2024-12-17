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
#ifndef DPU_COMP_ESD_H
#define DPU_COMP_ESD_H

#include <linux/workqueue.h>
#include <linux/hrtimer.h>
#include <linux/semaphore.h>
#include "dkmd_dpu.h"
#include "dkmd_esd_panel_info.h"

/* esd check period-->5000ms */
#define ESD_CHECK_TIME_PERIOD 5000
#define DPU_ESD_CHECK_MAX_COUNT 3
#define ESD_RECOVERY_MAX_COUNT 5
#define ESD_WAIT_MIPI_AVAILABLE_TIMEOUT 64

struct dpu_composer;
struct comp_online_present;

enum ESD_RECOVER_STATE {
	ESD_RECOVER_STATE_NONE = 0,
	ESD_RECOVER_STATE_START = 1,
	ESD_RECOVER_STATE_COMPLETE = 2,
};

enum esd_event {
	ESD_EVENT_START = 0,
	ESD_EVENT_END = 1,
	ESD_EVENT_REFRESH = 2,
};

struct dpu_esd_ctrl {
	int esd_inited;
	struct hrtimer esd_hrtimer;
	struct workqueue_struct *esd_check_wq;
	struct work_struct esd_check_work;

	struct dpu_composer *dpu_comp;
	struct comp_online_present *present;

	atomic_t esd_happened;
	enum ESD_RECOVER_STATE esd_recover_state;
	atomic_t esd_check_is_doing;
	struct semaphore esd_recover_sem;
};

bool check_esd_happend(struct dpu_composer *dpu_comp);
void dpu_comp_esd_register(struct dpu_composer *dpu_comp, struct comp_online_present *present);
void dpu_comp_esd_unregister(struct dpu_composer *dpu_comp);
void restart_esd_timer(struct dpu_composer *dpu_comp);
void start_esd_timer(struct dpu_composer *dpu_comp);
void cancel_esd_timer(struct dpu_composer *dpu_comp);
#endif