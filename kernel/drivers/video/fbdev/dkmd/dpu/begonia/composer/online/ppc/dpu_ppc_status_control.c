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
#include "dpu_ppc_status_control.h"
#include <linux/delay.h>
#include "dkmd_object.h"

#define PPC_EVENT_TIMEOUT_MS 100
#define PPC_EVENT_MAX_DELAY_CNT 100

void process_ppc_event(struct dpu_composer *dpu_comp, uint32_t ppc_event)
{
	uint32_t current_ppc_status;
	struct comp_online_present *present = NULL;
	struct dpu_comp_ppc_ctrl *ppc_ctrl = NULL;

	dpu_check_and_no_retval(!dpu_comp, err, "dpu_comp is null");
	dpu_check_and_no_retval(!dpu_comp->conn_info, err, "conn_info is null");

	if (!is_ppc_support(&(dpu_comp->conn_info->base)))
		return;

	present = (struct comp_online_present *)dpu_comp->present_data;
	dpu_check_and_no_retval(!present, err, "present is null");

	ppc_ctrl = &present->ppc_ctrl;
	current_ppc_status = ppc_ctrl->ppc_status;

	switch (ppc_event) {
	case PPC_EVENT_DPU_POWER_ON:
	case PPC_EVENT_DPU_POWER_OFF:
	case PPC_EVENT_DONE:
		ppc_ctrl->ppc_status = PPC_STATUS_SWITCHABLE;
		break;
	case PPC_EVENT_DPU_IDLE:
		ppc_ctrl->ppc_status =
			(current_ppc_status == PPC_STATUS_UNSWITCHABLE) ? PPC_STATUS_SWITCHABLE : current_ppc_status;
		break;
	case PPC_EVENT_PRESENT:
		ppc_ctrl->ppc_status =
			(current_ppc_status == PPC_STATUS_SWITCHABLE) ? PPC_STATUS_UNSWITCHABLE : current_ppc_status;
		break;
	case PPC_EVENT_WAIT_SWITCH:
		ppc_ctrl->ppc_status =
			(current_ppc_status != PPC_STATUS_SWITCHABLE) ? PPC_STATUS_WAIT_VSYNC : current_ppc_status;
		break;
	case PPC_EVENT_VSYNC_OK:
		ppc_ctrl->ppc_status =
			(current_ppc_status == PPC_STATUS_WAIT_VSYNC) ? PPC_STATUS_SWITCHABLE : current_ppc_status;
		break;
	case PPC_EVENT_WAIT_VSYNC_TIMEOUT:
		ppc_ctrl->ppc_status = PPC_STATUS_RESUME;
		break;
	case PPC_EVENT_UNDERFLOW_CLEAR:
		ppc_ctrl->ppc_status =
			(current_ppc_status == PPC_STATUS_RESUME) ? PPC_STATUS_SWITCHABLE : current_ppc_status;
		break;
	default:
		dpu_pr_warn("invalid ppc event: %u", ppc_event);
		break;
	}
}

int32_t wait_event_switch_ppc_status(struct comp_online_present *present, struct dpu_connector *connector)
{
	int32_t ret = 0;
	uint32_t delay_count = 0;

	dpu_check_and_return(!present, -1, err, "present is null");
	dpu_check_and_return(!connector, -1, err, "connector is null");

	while (true) {
		ret = (int32_t)wait_event_interruptible_timeout(present->ppc_ctrl.ppc_cmd_start_wq,
			is_ppc_status_switchable(&present->ppc_ctrl),
			(long)msecs_to_jiffies(PPC_EVENT_TIMEOUT_MS));
		if ((ret == -ERESTARTSYS) && (++delay_count <= PPC_EVENT_MAX_DELAY_CNT))
			usleep_range(1000, 1100); /* need sleep 1ms. */
		else
			break;
	}

	if (ret == 0) {
		dpu_pr_warn("wait event timeout, vstate=0x%x", inp32(DPU_DSI_VSTATE_ADDR(connector->connector_base)));
		return -1;
	}

	return 0;
}
