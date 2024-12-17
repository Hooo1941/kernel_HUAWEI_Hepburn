/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
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

#ifndef DKMD_TIMELINE_H
#define DKMD_TIMELINE_H

#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/notifier.h>
#include <linux/kref.h>
#include <linux/kthread.h>

#include "dkmd_timeline_listener.h"
#include "dkmd_log.h"

#define DKMD_SYNC_NAME_SIZE 64

/**
 * @brief release fence is created on next_value, and signaled on pt_value
 * The relationship between them is :
 *     pt_value += ++next_value
 */
struct dkmd_timeline {
	struct kref kref;
	char name[DKMD_SYNC_NAME_SIZE];
	uint32_t listening_isr_bit;
	uint32_t isr_unmask_bit;
	bool routine_enabled;
	void *parent;
	void *isr;

	spinlock_t value_lock;
	uint64_t pt_value;
	uint32_t next_value;
	uint32_t inc_step;
	uint64_t context;
	ktime_t notify_timestamp;

	/* isr will notify timeline to handle event */
	struct notifier_block *notifier;

	struct mutex list_lock;
	/* list to struct dkmd_timeline_listener */
	struct list_head listener_list;

	struct kthread_worker *present_handle_worker;
	struct kthread_work timeline_sync_work;
	bool need_reset;
};

static inline void dkmd_timeline_set_reset_flag(struct dkmd_timeline *timeline, bool need_reset)
{
	unsigned long flags;

	spin_lock_irqsave(&timeline->value_lock, flags);
	timeline->need_reset = need_reset;
	spin_unlock_irqrestore(&timeline->value_lock, flags);
}

static inline void dkmd_timeline_add_listener(struct dkmd_timeline *timeline, struct dkmd_timeline_listener *node)
{
	mutex_lock(&timeline->list_lock);
	list_add_tail(&node->list_node, &timeline->listener_list);
	mutex_unlock(&timeline->list_lock);
}

static inline void dkmd_timeline_dec_next_value(struct dkmd_timeline *timeline)
{
	unsigned long flags;

	spin_lock_irqsave(&timeline->value_lock, flags);
	if (timeline->next_value > 0)
		--timeline->next_value;

	if (g_debug_fence_timeline)
		dpu_pr_info("pt_value = %llu, inc_step = %u, next_value = %u",
				    timeline->pt_value, timeline->inc_step, timeline->next_value);
	spin_unlock_irqrestore(&timeline->value_lock, flags);
}

static inline uint64_t dkmd_timeline_get_next_value(struct dkmd_timeline *timeline)
{
	unsigned long flags;
	uint64_t value;

	spin_lock_irqsave(&timeline->value_lock, flags);
	++timeline->next_value;
	value = timeline->pt_value + timeline->next_value;

	if (g_debug_fence_timeline)
		dpu_pr_info("pt_value = %llu, next_value = %u", timeline->pt_value, timeline->next_value);
	spin_unlock_irqrestore(&timeline->value_lock, flags);

	return value;
}

static inline void dkmd_timeline_inc_step(struct dkmd_timeline *timeline)
{
	unsigned long flags;

	spin_lock_irqsave(&timeline->value_lock, flags);
	++timeline->inc_step;
	spin_unlock_irqrestore(&timeline->value_lock, flags);
}

static inline void dkmd_timeline_enable_routine(struct dkmd_timeline *timeline)
{
	timeline->routine_enabled = true;
}

static inline void dkmd_timeline_disable_routine(struct dkmd_timeline *timeline)
{
	timeline->routine_enabled = false;
}

uint64_t dkmd_timeline_get_pt_value(struct dkmd_timeline *timeline);
struct dkmd_timeline_listener *dkmd_timeline_alloc_listener(struct dkmd_timeline_listener_ops *listener_ops,
	void *listener_data, uint64_t value);

void dkmd_timeline_init(struct dkmd_timeline *timeline, const char *name, void *parent);
void dkmd_timeline_resync_pt(struct dkmd_timeline *timeline, uint32_t value);

#endif /* DKMD_TIMELINE_H */
