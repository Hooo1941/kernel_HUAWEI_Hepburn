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

#ifndef DPU_PPC_STATUS_MANAGER_H
#define DPU_PPC_STATUS_MANAGER_H

#include "dpu_comp_ppc.h"
#include "dpu_comp_mgr.h"
#include "dpu_connector.h"
#include "dkmd_log.h"

enum PPC_EVENT {
	PPC_EVENT_DPU_IDLE,
	PPC_EVENT_DPU_POWER_ON,
	PPC_EVENT_DPU_POWER_OFF,
	PPC_EVENT_PRESENT,
	PPC_EVENT_WAIT_SWITCH,
	PPC_EVENT_VSYNC_OK,
	PPC_EVENT_WAIT_VSYNC_TIMEOUT,
	PPC_EVENT_UNDERFLOW_CLEAR,
	PPC_EVENT_DONE,
};

enum PPC_STATUS {
	// ppc status can be switched before first present of dpu power on or idle status or vsync isr notify
	PPC_STATUS_SWITCHABLE,
	PPC_STATUS_UNSWITCHABLE,
	PPC_STATUS_WAIT_VSYNC,
	PPC_STATUS_RESUME,
};

void process_ppc_event(struct dpu_composer *dpu_comp, uint32_t ppc_event);
int32_t wait_event_switch_ppc_status(struct comp_online_present *present, struct dpu_connector *connector);

static inline bool is_ppc_status_switchable(struct dpu_comp_ppc_ctrl *ppc_ctrl)
{
	if (unlikely(!ppc_ctrl)) {
		dpu_pr_err("ppc_ctrl is null");
		return false;
	}

	return (ppc_ctrl->ppc_status == PPC_STATUS_SWITCHABLE);
}

static inline bool is_ppc_status_wait_vsync(struct dpu_comp_ppc_ctrl *ppc_ctrl)
{
	if (unlikely(!ppc_ctrl)) {
		dpu_pr_err("ppc_ctrl is null");
		return false;
	}

	return (ppc_ctrl->ppc_status == PPC_STATUS_WAIT_VSYNC);
}

static inline bool is_ppc_status_resume(struct dpu_comp_ppc_ctrl *ppc_ctrl)
{
	if (unlikely(!ppc_ctrl)) {
		dpu_pr_err("ppc_ctrl is null");
		return false;
	}

	return (ppc_ctrl->ppc_status == PPC_STATUS_RESUME);
}

#endif
