/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: function of prefetch memory instructions in sched module.
 */
#ifndef __PREFECTCH_SCHED_H
#define __PREFECTCH_SCHED_H

#include "prefetch.h"

#ifdef CONFIG_SOFTWARE_PREFETCH_TTWU
#define RESERVED_FLAG reserved
#define PREV_WINDOW ravg.prev_window_cpu[0]
#define CURR_WINDOW ravg.curr_window_cpu[0]
#elif defined(CONFIG_SOFTWARE_PREFETCH_TTWU_LTS)
#define RESERVED_FLAG walt_flags
#define PREV_WINDOW ravg.prev_window
#define CURR_WINDOW ravg.curr_window
#endif

#if defined(CONFIG_SOFTWARE_PREFETCH_TTWU) || defined(CONFIG_SOFTWARE_PREFETCH_TTWU_LTS)
static __always_inline void ttwu_prfm_target_task_front(const struct task_struct *ptr)
{
#ifdef CONFIG_CGROUPS
	prfm_member("PLDL1KEEP", ptr, struct task_struct, cgroups);
#endif
#ifdef CONFIG_HISI_EAS_SCHED
	prfm_member("PLDL1KEEP", ptr, struct task_struct, last_enqueued_ts);
#endif
	prfm_member("PSTL1KEEP", ptr, struct task_struct, pi_lock);
#ifdef CONFIG_THREAD_INFO_IN_TASK
	prfm_member("PSTL1KEEP", ptr, struct task_struct, thread_info);
	prfm_member("PLDL1KEEP", ptr, struct task_struct, cpu);
#endif
#ifdef CONFIG_SMP
	prfm_member("PLDL1KEEP", ptr, struct task_struct, wake_entry);
#endif
#ifdef CONFIG_SCHED_WALT
#ifdef CONFIG_TASK_RAVG_SUM
	prfm_member("PSTL1KEEP", ptr, struct task_struct, ravg.ravg_sum);
#endif
	prfm_member("PSTL1KEEP", ptr, struct task_struct, PREV_WINDOW);
	prfm_member("PSTL1KEEP", ptr, struct task_struct, ravg.curr_window);
	prfm_member("PSTL1KEEP", ptr, struct task_struct, ravg.load_sum);
	prfm_member("PSTL1KEEP", ptr, struct task_struct, ravg.curr_load);
#endif
	prfm_member("PLDL1KEEP", ptr, struct task_struct, nr_cpus_allowed);
}

static __always_inline void ttwu_prfm_target_task_back(const struct task_struct *ptr)
{
	prfm_member("PSTL1KEEP", ptr, struct task_struct, se.load);
	prfm_member("PSTL1KEEP", ptr, struct task_struct, se.vruntime);
#ifdef CONFIG_SCHEDSTATS
	prfm_member("PSTL1KEEP", ptr, struct task_struct,
		se.statistics.sleep_max);
	prfm_member("PSTL1KEEP", ptr, struct task_struct,
		se.statistics.nr_wakeups);
	prfm_member("PSTL1KEEP", ptr, struct task_struct,
		se.statistics.nr_wakeups_migrate);
#endif
#ifdef CONFIG_FAIR_GROUP_SCHED
	prfm_member("PSTL1KEEP", ptr, struct task_struct, se.parent);
#endif
#ifdef CONFIG_SMP
	prfm_member("PSTL1KEEP", ptr, struct task_struct, se.avg.util_sum);
#endif
	/*
	 * cannot compute offset of bit-field 'in_iowait',
	 * so use atomic_flags to replace.
	 */
	prfm_member("PLDL1KEEP", ptr, struct task_struct, atomic_flags);
	prfm_member("PSTL1KEEP", ptr, struct task_struct, sched_info);
	prfm_member("PLDL1KEEP", ptr, struct task_struct, pid);
#ifdef CONFIG_PSI
	prfm_member("PSTL1KEEP", ptr, struct task_struct, psi_flags);
#endif
}

static __always_inline void ttwu_prfm_cur_task(const struct task_struct *ptr)
{
#ifdef CONFIG_PREEMPT_RCU
	prfm_member("PSTL1KEEP", ptr, struct task_struct, rcu_read_lock_nesting);
#endif
#ifdef CONFIG_SCHED_RTG
	prfm_member("PLDL1KEEP", ptr, struct task_struct, grp);
#endif
#ifdef CONFIG_SCHED_WALT
#ifdef CONFIG_TASK_RAVG_SUM
	prfm_member("PSTL1KEEP", ptr, struct task_struct, ravg.ravg_sum);
#endif
	prfm_member("PSTL1KEEP", ptr, struct task_struct, CURR_WINDOW);
	prfm_member("PSTL1KEEP", ptr, struct task_struct, ravg.curr_window);
	prfm_member("PSTL1KEEP", ptr, struct task_struct, ravg.load_sum);
	prfm_member("PSTL1KEEP", ptr, struct task_struct, ravg.curr_load);
#endif
	prfm_member("PLDL1KEEP", ptr, struct task_struct, policy);
#ifdef CONFIG_SMP
	prfm_member("PLDL1KEEP", ptr, struct task_struct, wake_entry);
#endif
#ifdef CONFIG_CGROUPS
	prfm_member("PLDL1KEEP", ptr, struct task_struct, cgroups);
#endif
}

static __always_inline void ttwu_prfm_cur_rq(const struct rq *ptr)
{
	prfm_member("PSTL1KEEP", ptr, struct rq, __lock);
	prfm_member("PLDL1KEEP", ptr, struct rq, clock_update_flags);
#ifdef CONFIG_SCHED_WALT
	prfm_member("PSTL1KEEP", ptr, struct rq, curr_runnable_sum);
#ifdef CONFIG_SOFTWARE_PREFETCH_TTWU
	prfm_member("PSTL1KEEP", ptr, struct rq, walt_update_lock);
#endif
#endif
#ifdef CONFIG_SCHED_PRED_LOAD
	prfm_member("PLDL1KEEP", ptr, struct rq, predl_window_start);
#endif
	prfm_member("PLDL1KEEP", ptr, struct rq, cpu);
	prfm_member("PLDL1KEEP", ptr, struct rq, cpu_capacity_orig);
#ifdef CONFIG_SCHED_TOP_TASK
	prfm_member("PSTL1KEEP", ptr, struct rq, curr_table);
#endif
#ifdef CONFIG_SCHEDSTATS
	prfm_member("PSTL1KEEP", ptr, struct rq, ttwu_count);
#endif
}

static __always_inline void ttwu_prfm_target_rq(const struct rq *ptr)
{
	prfm_member("PLDL1KEEP", ptr, struct rq, cpu);
#if defined(CONFIG_SOFTWARE_PREFETCH_TTWU) && defined(CONFIG_SCHED_WALT)
	prfm_member("PSTL1KEEP", ptr, struct rq, walt_update_lock);
#endif
#ifdef CONFIG_SCHED_TOP_TASK
	prfm_member("PLDL1KEEP", ptr, struct rq, curr_table);
#endif
	prfm_member("PLDL1KEEP", ptr, struct rq, idle_stamp);
#ifdef CONFIG_UCLAMP_TASK
	prfm_member("PSTL1KEEP", ptr, struct rq, uclamp_flags);
#endif
	prfm_member("PSTL1KEEP", ptr, struct rq, cfs.load.weight);
	prfm_member("PLDL1KEEP", ptr, struct rq, cfs.curr);
#ifdef CONFIG_SMP
	prfm_member("PSTL1KEEP", ptr, struct rq, cfs.avg.load_sum);
#endif
	prfm_member("PLDL1KEEP", ptr, struct rq, cfs.removed.nr);
#ifdef CONFIG_FAIR_GROUP_SCHED
	prfm_member("PSTL1KEEP", ptr, struct rq, cfs.prop_runnable_sum);
	prfm_member("PLDL1KEEP", ptr, struct rq, cfs.tg);
	prfm_member("PLDL1KEEP", ptr, struct rq, tmp_alone_branch);
#endif
}

static __always_inline void feec_prfm_target_rq_front(const struct rq *ptr)
{
#ifdef CONFIG_SCHED_WALT
	prfm_member("PLDL1KEEP", ptr, struct rq, avg_irqload);
#endif
#ifdef CONFIG_HISI_EAS_SCHED
	prfm_member("PLDL1KEEP", ptr, struct rq, RESERVED_FLAG);
#endif
}

static __always_inline void feec_prfm_target_rq_back(const struct rq *ptr)
{
#ifdef CONFIG_SCHED_WALT
#ifdef CONFIG_SOFTWARE_PREFETCH_SCHEDULE
	prfm_member("PLDL1KEEP", ptr, struct rq, cumulative_runnable_avg);
#elif defined(CONFIG_SOFTWARE_PREFETCH_SCHEDULE_LTS)
	prfm_member("PLDL1KEEP", &ptr->walt_stats, struct walt_sched_stats, cumulative_runnable_avg_scaled);
#endif
#endif
	prfm_data("PLDL1KEEP", ptr,
		offsetof(struct rq, cfs) + offsetof(struct cfs_rq, avg));
	prfm_member("PLDL1KEEP", ptr, struct rq, idle);
	prfm_member("PLDL1KEEP", ptr, struct rq, cpu_capacity_orig);

#ifdef CONFIG_UCLAMP_TASK
	prfm_member("PSTL1KEEP", ptr, struct rq, uclamp[0].value);
	prfm_member("PSTL1KEEP", ptr, struct rq, uclamp[1].value);
#endif
}

#else /* !SOFTWARE_PREFETCH_TTWU & !SOFTWARE_PREFETCH_TTWU_LTS */
static __always_inline void ttwu_prfm_target_task_front(const struct task_struct *ptr) {}
static __always_inline void ttwu_prfm_target_task_back(const struct task_struct *ptr) {}
static __always_inline void ttwu_prfm_cur_task(const struct task_struct *ptr) {}
static __always_inline void ttwu_prfm_cur_rq(const struct rq *ptr) {}
static __always_inline void ttwu_prfm_target_rq(const struct rq *ptr) {}
static __always_inline void feec_prfm_target_rq_front(const struct rq *ptr) {}
static __always_inline void feec_prfm_target_rq_back(const struct rq *ptr) {}
#endif /* SOFTWARE_PREFETCH_TTWU | SOFTWARE_PREFETCH_TTWU_LTS */

#if defined(CONFIG_SOFTWARE_PREFETCH_SCHEDULE) || defined(CONFIG_SOFTWARE_PREFETCH_SCHEDULE_LTS)
static __always_inline void schedule_prfm_prev_task_front(const struct task_struct *ptr)
{
#ifdef CONFIG_SMP
	prfm_member("PLDL1KEEP", ptr, struct task_struct, wakee_flip_decay_ts);
	prfm_member("PLDL1KEEP", ptr, struct task_struct, on_cpu);
	prfm_member("PLDL1KEEP", &ptr->se, struct sched_entity, avg);
#endif
#ifdef CONFIG_FAIR_GROUP_SCHED
	prfm_member("PLDL1KEEP", &ptr->se, struct sched_entity, cfs_rq);
#endif
#ifdef CONFIG_SCHED_RTG
	prfm_member("PLDL1KEEP", ptr, struct task_struct, grp);
#endif
#ifdef CONFIG_PREEMPT_RCU
	prfm_member("PSTL1KEEP", ptr, struct task_struct, rcu_read_unlock_special);
#endif
	prfm_member("PSTL1KEEP", &ptr->se, struct sched_entity, on_rq);
	prfm_member("PSTL1KEEP", &ptr->se, struct sched_entity, statistics);
#ifdef CONFIG_SCHED_WALT
	prfm_member("PSTL1KEEP", &ptr->ravg, struct ravg, mark_start);
#endif
}

static __always_inline void schedule_prfm_prev_task_middle(const struct task_struct *ptr)
{
#ifdef CONFIG_SCHED_WALT
#ifdef CONFIG_SCHED_MIGRATE_SPREAD_LOAD
	prfm_member("PLDL1KEEP", &ptr->ravg, struct ravg, curr_cpus);
#endif
	prfm_member("PSTL1KEEP", &ptr->ravg, struct ravg, curr_window);
	prfm_member("PSTL1KEEP", &ptr->ravg, struct ravg, curr_window_cpu);
#ifdef CONFIG_SCHED_TOP_TASK
	prfm_member("PSTL1KEEP", &ptr->ravg, struct ravg, load_sum);
#endif
#endif
	prfm_member("PLDL1KEEP", ptr, struct task_struct, policy);
	prfm_member("PLDL1KEEP", ptr, struct task_struct, pid);
#ifdef CONFIG_CGROUPS
	prfm_member("PLDL1KEEP", ptr, struct task_struct, cgroups);
#endif
	prfm_member("PLDL1KEEP", ptr, struct task_struct, mm);
	prfm_member("PSTL1KEEP", ptr, struct task_struct, nivcsw);
#ifdef CONFIG_PSI
	prfm_member("PSTL1KEEP", ptr, struct task_struct, psi_flags);
#endif
	prfm_member("PSTL1KEEP", ptr, struct task_struct, sched_info);
	prfm_member("PSTL1KEEP", &ptr->thread, struct thread_struct, uw);
}

static __always_inline void schedule_prfm_prev_task_back(const struct task_struct *ptr)
{
	prfm_member("PLDL1KEEP", &ptr->thread, struct thread_struct, fault_code);
#ifdef CONFIG_SOFTWARE_PREFETCH_SCHEDULE
	prfm_member("PLDL1KEEP", &ptr->thread, struct thread_struct, sctlr_user);
#endif
	prfm_member("PSTL1KEEP", &ptr->thread.uw.fpsimd_state, struct user_fpsimd_state, vregs[4]);
	prfm_member("PSTL1KEEP", &ptr->thread.uw.fpsimd_state, struct user_fpsimd_state, vregs[8]);
	prfm_member("PSTL1KEEP", &ptr->thread.uw.fpsimd_state, struct user_fpsimd_state, vregs[12]);
	prfm_member("PSTL1KEEP", &ptr->thread.uw.fpsimd_state, struct user_fpsimd_state, vregs[16]);
	prfm_member("PSTL1KEEP", &ptr->thread.uw.fpsimd_state, struct user_fpsimd_state, vregs[20]);
	prfm_member("PSTL1KEEP", &ptr->thread.uw.fpsimd_state, struct user_fpsimd_state, vregs[24]);
	prfm_member("PSTL1KEEP", &ptr->thread.uw.fpsimd_state, struct user_fpsimd_state, vregs[28]);
	prfm_member("PSTL1KEEP", &ptr->thread.uw.fpsimd_state, struct user_fpsimd_state, fpcr);
#if defined(CONFIG_HW_QOS_THREAD) || defined(CONFIG_HW_QOS_SCHED)
	prfm_member("PSTL1KEEP", ptr, struct task_struct, trans_flags);
#endif
}

static __always_inline void schedule_prfm_next_task_front(const struct task_struct *ptr)
{
#ifdef CONFIG_SCHED_RTG
	prfm_member("PLDL1KEEP", ptr, struct task_struct, grp);
#endif
#ifdef CONFIG_L3CACHE_PARTITION_CTRL
	prfm_member("PLDL1KEEP", ptr, struct task_struct, l3c_part);
#endif
#ifdef CONFIG_SMP
	prfm_member("PLDL1KEEP", ptr, struct task_struct, wakee_flip_decay_ts);
#endif
#ifdef CONFIG_THREAD_INFO_IN_TASK
	prfm_member("PSTL1KEEP", ptr, struct task_struct, thread_info);
#endif
#ifdef CONFIG_SCHED_WALT
	prfm_member("PSTL1KEEP", &ptr->ravg, struct ravg, mark_start);
	prfm_member("PSTL1KEEP", &ptr->ravg, struct ravg, curr_window_cpu);
#ifdef	CONFIG_SCHED_TOP_TASK
	prfm_member("PSTL1KEEP", &ptr->ravg, struct ravg, load_sum);
#endif
	prfm_member("PSTL1KEEP", &ptr->ravg, struct ravg, curr_window);
#ifdef CONFIG_SCHED_MIGRATE_SPREAD_LOAD
	prfm_member("PSTL1KEEP", &ptr->ravg, struct ravg, curr_cpus);
#endif
#endif
}

static __always_inline void schedule_prfm_next_task_back(const struct task_struct *ptr)
{
	prfm_member("PLDL1KEEP", ptr, struct task_struct, pid);
#ifdef CONFIG_CGROUPS
	prfm_member("PLDL1KEEP", ptr, struct task_struct, cgroups);
#endif
	prfm_member("PLDL1KEEP", &ptr->thread, struct thread_struct, uw);
	prfm_member("PLDL1KEEP", &ptr->thread, struct thread_struct, fault_code);
#ifdef CONFIG_SOFTWARE_PREFETCH_SCHEDULE
	prfm_member("PLDL1KEEP", &ptr->thread, struct thread_struct, sctlr_user);
#endif
#ifdef CONFIG_PSI
	prfm_member("PSTL1KEEP", ptr, struct task_struct, psi_flags);
#endif
	prfm_member("PSTL1KEEP", ptr, struct task_struct, sched_info);
#if defined(CONFIG_HW_QOS_THREAD) || defined(CONFIG_HW_QOS_SCHED)
	prfm_member("PSTL1KEEP", ptr, struct task_struct, trans_flags);
#endif
	prfm_member("PSTL1KEEP", ptr, struct task_struct, active_mm);
}

static __always_inline void schedule_prfm_curr_rq_front(const struct rq *ptr)
{
	prfm_member("PLDL1KEEP", ptr, struct rq, clock_task);
#ifdef CONFIG_FAIR_GROUP_SCHED
	prfm_member("PLDL1KEEP", &ptr->cfs, struct cfs_rq, rq);
#endif
#ifdef CONFIG_SMP
	prfm_member("PLDL1KEEP", &ptr->cfs, struct cfs_rq, removed);
	prfm_member("PLDL1KEEP", &ptr->cfs, struct cfs_rq, avg);
#endif
	prfm_member("PSTL1KEEP", ptr, struct rq, nr_running);
	prfm_member("PSTL1KEEP", &ptr->cfs, struct cfs_rq, nr_running);
	prfm_member("PSTL1KEEP", &ptr->cfs, struct cfs_rq, curr);
#ifdef CONFIG_HISI_EAS_SCHED
	prfm_member("PSTL1KEEP", ptr, struct rq, sync_waiting);
#endif
#ifdef CONFIG_SCHED_WALT
#ifdef CONFIG_SOFTWARE_PREFETCH_TTWU
	prfm_member("PSTL1KEEP", ptr, struct rq, walt_update_lock);
#endif
	prfm_member("PSTL1KEEP", ptr, struct rq, avg_irqload);
#endif
}

static __always_inline void schedule_prfm_curr_rq_back(const struct rq *ptr)
{
#ifdef CONFIG_SMP
	prfm_member("PLDL1KEEP", ptr, struct rq, cpu);
	prfm_member("PLDL1KEEP", ptr, struct rq, balance_callback);
#endif
#ifdef CONFIG_SCHED_TOP_TASK
	prfm_member("PLDL1KEEP", ptr, struct rq, top_task_stats_empty_window);
#endif
#ifdef CONFIG_HISI_EAS_SCHED
	prfm_member("PLDL1KEEP", ptr, struct rq, cluster);
#endif
#ifdef CONFIG_FAIR_GROUP_SCHED
	prfm_member("PLDL1KEEP", ptr, struct rq, tmp_alone_branch);
#endif
	prfm_member("PSTL1KEEP", ptr, struct rq, nr_switches);
}

static __always_inline void schedule_prfm_last_task(const struct list_head *tasks)
{
	struct task_struct *ptr = list_last_entry(tasks, struct task_struct, se.group_node);

	prfm_member("PLDL1KEEP", ptr, struct task_struct, pid);
	prfm_member("PLDL1KEEP", ptr, struct task_struct, cpus_ptr);
#ifdef CONFIG_FAIR_GROUP_SCHED
	prfm_member("PLDL1KEEP", &ptr->se, struct sched_entity, cfs_rq);
#endif
#ifdef CONFIG_SMP
	prfm_member("PLDL1KEEP", &ptr->se, struct sched_entity, avg);
#endif
#ifdef CONFIG_SCHED_WALT
	prfm_member("PLDL1KEEP", &ptr->ravg, struct ravg, demand);
#endif
	prfm_member("PLDL1KEEP", ptr, struct task_struct, flags);
#ifdef CONFIG_SCHEDSTATS
	prfm_member("PLDL1KEEP", &ptr->se.statistics, struct sched_statistics,
		nr_failed_migrations_affine);
#endif
	prfm_member("PLDL1KEEP", ptr, struct task_struct, sched_class);
	prfm_member("PLDL1KEEP", &ptr->se, struct sched_entity, exec_start);
}

static __always_inline void schedule_prfm_balance_rq(const struct rq *ptr)
{
#ifdef CONFIG_SMP
	prfm_member("PLDL1KEEP", &ptr->cfs, struct cfs_rq, avg);
#endif
#ifdef CONFIG_SCHED_WALT
#ifdef CONFIG_SOFTWARE_PREFETCH_SCHEDULE
	prfm_member("PLDL1KEEP", ptr, struct rq, cumulative_runnable_avg);
#elif defined(CONFIG_SOFTWARE_PREFETCH_SCHEDULE_LTS)
	prfm_member("PLDL1KEEP", &ptr->walt_stats, struct walt_sched_stats, cumulative_runnable_avg_scaled);
#endif
#endif
#ifdef CONFIG_SMP
	prfm_member("PLDL1KEEP", ptr, struct rq, cpu_capacity_orig);
	prfm_member("PLDL1KEEP", ptr, struct rq, cpu_capacity);
#endif
	prfm_member("PLDL1KEEP", &ptr->cfs, struct cfs_rq, h_nr_running);
	prfm_member("PLDL1KEEP", ptr, struct rq, nr_running);
#ifdef CONFIG_HISI_EAS_SCHED
	prfm_member("PLDL1KEEP", ptr, struct rq, cluster);
#endif
	prfm_member("PLDL1KEEP", ptr, struct rq, curr);
}
#else /* !CONFIG_SOFTWARE_PREFETCH_SCHEDULE & !CONFIG_SOFTWARE_PREFETCH_SCHEDULE_LTS */
static __always_inline void schedule_prfm_prev_task_front(const struct task_struct *ptr) {}
static __always_inline void schedule_prfm_prev_task_middle(const struct task_struct *ptr) {}
static __always_inline void schedule_prfm_prev_task_back(const struct task_struct *ptr) {}
static __always_inline void schedule_prfm_next_task_front(const struct task_struct *ptr) {}
static __always_inline void schedule_prfm_next_task_back(const struct task_struct *ptr) {}
static __always_inline void schedule_prfm_curr_rq_front(const struct rq *ptr) {}
static __always_inline void schedule_prfm_curr_rq_back(const struct rq *ptr) {}
static __always_inline void schedule_prfm_last_task(const struct list_head *tasks) {}
static __always_inline void schedule_prfm_balance_rq(const struct rq *ptr) {}
#endif /* CONFIG_SOFTWARE_PREFETCH_SCHEDULE | CONFIG_SOFTWARE_PREFETCH_SCHEDULE_LTS */
#endif
