/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 * Description: tzdriver internal functions.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */
#ifndef INTERNAL_FUNCTIONS_H
#define INTERNAL_FUNCTIONS_H

#include <linux/device.h>
#include <securec.h>
#include "teek_ns_client.h"
#include "teek_client_constants.h"

#ifndef CONFIG_TEE_FAULT_MANAGER
static inline void fault_monitor_start(int32_t type)
{
	(void)type;
	return;
}

static inline void fault_monitor_end(void)
{
	return;
}
#endif

#ifdef CONFIG_KTHREAD_AFFINITY
#include "tz_kthread_affinity.h"
#else
static inline void init_kthread_cpumask(void)
{
}

static inline void tz_kthread_bind_mask(struct task_struct *kthread)
{
	(void)kthread;
}

static inline void tz_workqueue_bind_mask(struct workqueue_struct *wq,
	uint32_t flag)
{
	(void)wq;
	(void)flag;
}
#endif


#ifdef CONFIG_LIVEPATCH_ENABLE
#include "livepatch_cmd.h"
#else
static inline int livepatch_init(const struct device *dev)
{
	(void)dev;
	return 0;
}
static inline void livepatch_down_read_sem(void)
{
}
static inline void livepatch_up_read_sem(void)
{
}
static inline void free_livepatch(void)
{
}
#endif

#ifdef CONFIG_TEE_TRACE
#include "tee_trace_event.h"
#include "tee_trace_interrupt.h"
#else
static inline void tee_trace_add_event(enum tee_event_id id, uint64_t add_info)
{
	(void)id;
	(void)add_info;
}
static inline void free_event_mem(void)
{
}
static inline void free_interrupt_trace(void)
{
}
#endif

#ifdef CONFIG_TEE_REBOOT
#include "reboot.h"
#else
static inline bool is_tee_rebooting(void)
{
	return false;
}
static inline int tee_init_reboot_thread(void)
{
	return 0;
}
static inline int tee_wake_up_reboot(void)
{
	return 0;
}
static inline void free_reboot_thread(void)
{
	return;
}
#endif

#endif
