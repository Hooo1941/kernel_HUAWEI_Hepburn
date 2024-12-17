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
#ifndef DKMD_NOTIFY_H
#define DKMD_NOTIFY_H

#include <linux/notifier.h>

struct dkmd_event {
	void *data;
	int panel_id;
	int value;
	uint32_t ffd_result;
};

enum dkmd_event_type {
	DKMD_EVENT_BLANK = 0,
	DKMD_EVENT_FFD_ENABLE,
	DKMD_EVENT_FFD_DISABLE,
	DKMD_EVENT_FFD_RESULT,
	DKMD_EVENT_SET_ACTIVE_RECT,
	DKMD_EVENT_MAX,
};

extern int dkmd_register_client(struct notifier_block *nb);
extern int dkmd_unregister_client(struct notifier_block *nb);
extern int dkmd_notifier_call_chain(unsigned long val, void *v);

#endif
