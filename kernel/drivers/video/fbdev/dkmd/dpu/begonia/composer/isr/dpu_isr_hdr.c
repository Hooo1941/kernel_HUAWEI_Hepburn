/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
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
#include "dkmd_listener.h"
#include "dpu_comp_mgr.h"
#include "dpu_isr.h"

static int32_t dpu_comp_hdr_handle_isr_notify(struct notifier_block *self, unsigned long action, void *data)
{
	struct dkmd_listener_data *listener_data = (struct dkmd_listener_data *)data;
	struct dpu_composer *dpu_comp = (struct dpu_composer *)(listener_data->data);
	char __iomem *hdr_base = NULL;
	uint32_t hdr_mean;

	hdr_base  = dpu_comp->comp_mgr->dpu_base + DPU_HDR_OFFSET;
	hdr_mean = inp32(DPU_HDR_MEAN_STT_ADDR(hdr_base));
	composer_set_hdr_mean(&dpu_comp->hdr_info, hdr_mean);
	return 0;
}

static struct notifier_block hdr_handle_isr_notifier = {
	.notifier_call = dpu_comp_hdr_handle_isr_notify,
};

void dpu_comp_hdr_handle_init(struct dkmd_isr *isr_ctrl, struct dpu_composer *dpu_comp, uint32_t listening_bit)
{
	dkmd_isr_register_listener(isr_ctrl, &hdr_handle_isr_notifier, listening_bit, dpu_comp);
}

void dpu_comp_hdr_handle_deinit(struct dkmd_isr *isr_ctrl, uint32_t listening_bit)
{
	dkmd_isr_unregister_listener(isr_ctrl, &hdr_handle_isr_notifier, listening_bit);
}
