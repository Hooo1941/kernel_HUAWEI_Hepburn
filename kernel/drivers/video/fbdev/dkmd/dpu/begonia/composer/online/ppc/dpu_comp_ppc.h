/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2025. All rights reserved.
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

#ifndef DPU_COMP_PPC_H
#define DPU_COMP_PPC_H

#include <linux/wait.h>
#include <linux/notifier.h>

struct comp_online_present;
struct dpu_composer;
struct dpu_comp_ppc_ctrl {
	void *priv_data;
	struct dpu_composer *dpu_comp;
	wait_queue_head_t wait;
	wait_queue_head_t ppc_cmd_start_wq;
	uint32_t dacc_send_cmd_done;
	uint32_t ppc_status;

	int32_t (*setup_data)(struct dpu_comp_ppc_ctrl *ppc_ctrl);
	void (*release_data)(struct dpu_comp_ppc_ctrl *ppc_ctrl);
	int32_t (*set_active_rect)(struct dpu_comp_ppc_ctrl *ppc_ctrl, uint32_t ppc_config_id);
};

void dpu_comp_ppc_ctrl_setup(struct dpu_composer *dpu_comp, struct comp_online_present *present);
int32_t dpu_comp_preprocess_set_active_rect(struct dpu_composer *dpu_comp,
	uint32_t ppc_config_id, bool *is_need_switch);

#endif
