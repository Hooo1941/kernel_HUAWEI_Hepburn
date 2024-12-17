/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
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

#ifndef COMPOSER_TUI_H
#define COMPOSER_TUI_H

#include <linux/types.h>
#include <linux/kthread.h>

struct dpu_composer;

enum tui_sec_status {
	TUI_SEC_IDLE = 0,
	TUI_SEC_RUNNING,
};

enum tui_sec_event {
	TUI_SEC_DISABLE = 0,
	TUI_SEC_ENABLE,
};

struct dpu_secure {
	bool secure_created;
	bool tui_need_switch;
	bool tui_quit_flag;
	uint32_t secure_status;
	uint32_t secure_event;
	uint32_t tui_ready_status;

	struct kthread_work secure_ctrl_work;
	struct dpu_composer *dpu_comp;
	struct semaphore tui_sem;
};

#if defined(CONFIG_TEE_TUI)
void dpu_tui_register(struct dpu_composer *dpu_comp);
void dpu_tui_unregister(struct dpu_composer *dpu_comp);
int dpu_tui_wait_quit(struct dpu_composer *dpu_comp);
void dpu_tui_update_ready_status(struct dpu_composer *dpu_comp, uint32_t tui_ready_status);
void dpu_tui_frame_start_isr_handler(struct dpu_composer *dpu_comp);
#else
#define dpu_tui_register(dpu_comp)
#define dpu_tui_unregister(dpu_comp)
#define dpu_tui_wait_quit(dpu_comp) (0)
#define dpu_tui_update_ready_status(dpu_comp, tui_ready_status)
#define dpu_tui_frame_start_isr_handler(dpu_comp)
#endif

#endif