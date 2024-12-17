/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_CORE_SCHED_EXT_H
#define _LINUX_CORE_SCHED_EXT_H

#include <linux/refcount.h>
#include <linux/types.h>
#include <linux/cpumask.h>
#include <linux/threads.h>
#include <linux/sched.h>

/*
 * A simple wrapper around refcount. An allocated sched_core_cookie's
 * address is used to compute the cookie of the task.
 */
struct sched_core_cookie {
	refcount_t refcnt;
};

struct sched_core_prefer {
	refcount_t refcnt;
	struct cpumask active_mask;
	atomic_t nr_active[NR_CPUS];
	atomic_t total_active;
};

extern struct sched_core_cookie root_core_cookie;
extern unsigned int sched_core_set_cookie_by_uid;

#ifdef CONFIG_SCHED_CORE_PREFER
int sched_core_share_prefer(struct task_struct *to, struct task_struct *from);
#else
static inline int sched_core_share_prefer(struct task_struct *to,
					  struct task_struct *from)
{
	return 0;
}
#endif

#endif
