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

static int32_t dpu_comp_alsc_handle_isr_notify(struct notifier_block *self, unsigned long action, void *data)
{
	struct dkmd_listener_data *listener_data = (struct dkmd_listener_data *)data;
	struct dpu_composer *dpu_comp = (struct dpu_composer *)(listener_data->data);

	dpu_pr_debug("action=%#x, enter", action);

	if (dpu_comp->alsc == NULL) {
		dpu_pr_debug("not support alsc");
		return 0;
	}

	return dpu_effect_alsc_store_data(dpu_comp->alsc, (uint32_t)action);
}

static struct notifier_block alsc_handle_isr_notifier = {
	.notifier_call = dpu_comp_alsc_handle_isr_notify,
};

void dpu_comp_alsc_handle_init(struct dkmd_isr *isr_ctrl, struct dpu_composer *dpu_comp, uint32_t listening_bit)
{
	dkmd_isr_register_listener(isr_ctrl, &alsc_handle_isr_notifier, listening_bit, dpu_comp);
}

void dpu_comp_alsc_handle_deinit(struct dkmd_isr *isr_ctrl, uint32_t listening_bit)
{
	dkmd_isr_unregister_listener(isr_ctrl, &alsc_handle_isr_notifier, listening_bit);
}