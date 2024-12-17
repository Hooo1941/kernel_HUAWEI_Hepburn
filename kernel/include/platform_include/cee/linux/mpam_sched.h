/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __MPAM_SCHED_H
#define __MPAM_SCHED_H

#ifdef CONFIG_MPAM

#include <linux/sched.h>
#include <linux/jump_label.h>

/**
 * struct mpam_pqr_state - State cache for the PQR MSR
 * @cur_partid:	        The cached curent partid
 * @default_partid:	The user assigned cached default partid
 *
 * The cache also helps to avoid pointless updates if the value does
 * not change.
 */
struct mpam_pqr_state {
	u32			cur_partid;
	u32			default_partid;
};


void mpam_sched_in(void);

int mpam_cpus_write(u32 partid, u32 cpus_mask);

int mpam_partid_task_set(u32 partid, pid_t pid);

int mpam_partid_task_remove(u32 partid);

#else

static inline void mpam_sched_in(void) {}

static inline int mpam_cpus_write(u32 partid, u32 cpus_mask)
{
	return 0;
}

static inline int mpam_partid_task_set(u32 partid, pid_t pid)
{
	return 0;
}

static inline int mpam_partid_task_remove(u32 partid)
{
	return 0;
}

#endif /* CONFIG_MPAM */

#endif /* __MPAM_SCHED_H */
