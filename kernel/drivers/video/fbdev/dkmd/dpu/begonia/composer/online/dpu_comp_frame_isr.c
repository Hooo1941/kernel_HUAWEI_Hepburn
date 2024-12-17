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

#include "dpu_comp_frame_isr.h"
#include "dpu_comp_mgr.h"

static int32_t dpu_comp_frame_start_isr_notify(struct notifier_block *self, unsigned long action, void *data)
{
	struct dkmd_listener_data *listener_data = (struct dkmd_listener_data *)data;
	struct dpu_composer *dpu_comp = NULL;
	struct comp_online_present *present = NULL;

	if (unlikely(!listener_data)) {
		dpu_pr_err("listener_data is null");
		return -1;
	}

	dpu_comp = (struct dpu_composer *)(listener_data->data);

	if (unlikely(!dpu_comp)) {
		dpu_pr_err("dpu_comp is null");
		return -1;
	}

	present = (struct comp_online_present *)dpu_comp->present_data;

	if (unlikely(!present)) {
		dpu_pr_err("present is null");
		return -1;
	}

	present->frame_start_flag = 1;
	dpu_tui_frame_start_isr_handler(dpu_comp);
	return 0;
}

static struct notifier_block frame_start_isr_notifier = {
	.notifier_call = dpu_comp_frame_start_isr_notify,
};

void dpu_comp_frame_start_isr_init(struct dkmd_isr *isr, struct dpu_composer *dpu_comp, uint32_t listening_bit)
{
	struct comp_online_present *present = NULL;
	if (unlikely(!isr)) {
		dpu_pr_err("isr is null pointer");
		return;
	}

	if (unlikely(!dpu_comp)) {
		dpu_pr_err("dpu_comp is null pointer");
		return;
	}

	present = (struct comp_online_present *)dpu_comp->present_data;

	if (unlikely(!present)) {
		dpu_pr_err("present is null");
		return;
	}

	present->frame_start_flag = 0;
	dkmd_isr_register_listener(isr, &frame_start_isr_notifier, listening_bit, dpu_comp);
}

void dpu_comp_frame_start_isr_deinit(struct dkmd_isr *isr, uint32_t listening_bit)
{
	dkmd_isr_unregister_listener(isr, &frame_start_isr_notifier, listening_bit);
}

static int32_t dpu_comp_frame_end_isr_notify(struct notifier_block *self, unsigned long action, void *data)
{
	struct dkmd_listener_data *listener_data = (struct dkmd_listener_data *)data;
	struct dpu_composer *dpu_comp = NULL;
	struct comp_online_present *present = NULL;

	if (unlikely(!listener_data)) {
		dpu_pr_err("listener_data is null");
		return -1;
	}

	dpu_comp = (struct dpu_composer *)(listener_data->data);
	if (unlikely(!dpu_comp)) {
		dpu_pr_err("dpu_comp is null");
		return -1;
	}

	present = (struct comp_online_present *)dpu_comp->present_data;
	if (unlikely(!present)) {
		dpu_pr_err("present is null");
		return -1;
	}

	present->frame_start_flag = 0;
	return 0;
}

static struct notifier_block frame_end_isr_notifier = {
	.notifier_call = dpu_comp_frame_end_isr_notify,
};

void dpu_comp_frame_end_isr_init(struct dkmd_isr *isr, struct dpu_composer *dpu_comp, uint32_t listening_bit)
{
	dkmd_isr_register_listener(isr, &frame_end_isr_notifier, listening_bit, dpu_comp);
}

void dpu_comp_frame_end_isr_deinit(struct dkmd_isr *isr, uint32_t listening_bit)
{
	dkmd_isr_unregister_listener(isr, &frame_end_isr_notifier, listening_bit);
}
