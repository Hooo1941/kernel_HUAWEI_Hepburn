/*
 * platform_source/basicplatform/drivers/hisi_lmk/lowmem_dbg.c
 *
 * Copyright (C) 2004-2020 Hisilicon Technologies Co., Ltd. All rights reserved.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#define pr_fmt(fmt) "hisi_lowmem: " fmt

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/oom.h>
#include <linux/sched.h>
#include <linux/sched/debug.h>
#include <linux/sysrq.h>
#include <linux/rcupdate.h>
#include <linux/notifier.h>
#include <linux/mutex.h>
#include <linux/delay.h>
#include <linux/swap.h>
#include <linux/fs.h>
#include <linux/jiffies.h>
#include <linux/workqueue.h>
#include <linux/freezer.h>
#include <linux/ksm.h>
#include <linux/version.h>
#include <linux/skbuff.h>
#include <linux/sched/debug.h>
#ifdef CONFIG_SLUB
#include <linux/slub_def.h>
#endif
#include <linux/mm/lowmem_killer.h>
#ifdef CONFIG_DFX_MEMCHECK
#include <platform/trace/hooks/memcheck.h>
#endif

#define LMK_PRT_TSK_RSS 10000
#define LMK_INTERVAL 3

/* SERVICE_ADJ(5) * OOM_SCORE_ADJ_MAX / -OOM_DISABLE */
#define LMK_SERVICE_ADJ 500

static unsigned long long last_jiffs;

static void lowmem_dump(struct work_struct *work);

static DEFINE_MUTEX(lowmem_dump_mutex);
static DECLARE_WORK(lowmem_dbg_wk, lowmem_dump);
static DECLARE_WORK(lowmem_dbg_verbose_wk, lowmem_dump);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0))
static unsigned long get_tsk_nr_ptes_value(struct task_struct *task)
{
	unsigned long tsk_nr_ptes;

#ifdef CONFIG_MMU
	tsk_nr_ptes = (unsigned long)atomic_long_read(&task->mm->pgtables_bytes);
#else
	tsk_nr_ptes = 0;
#endif
	return tsk_nr_ptes;
}
#else
static unsigned long get_tsk_nr_ptes_value(struct task_struct *task)
{
	return (unsigned long)atomic_long_read(&task->mm->nr_ptes);
}
#endif

static void dump_tasks(bool verbose)
{
	struct task_struct *p = NULL;
	struct task_struct *task = NULL;
	short tsk_oom_adj;
	unsigned long tsk_nr_ptes;
	char task_state = 0;
	char frozen_mark = ' ';

#ifdef CONFIG_PM_PEAK
	pr_info("[ pid ]   uid  tgid total_vm    rss nptes  swap pmpeak adj s name\n");
#else
	pr_info("[ pid ]   uid  tgid total_vm    rss nptes  swap   adj s name\n");
#endif

	rcu_read_lock();
	for_each_process(p) {
		task = find_lock_task_mm(p);
		if (!task) {
			/*
			 * This is a kthread or all of p's threads have already
			 * detached their mm's.  There's no need to report
			 * them; they can't be oom killed anyway.
			 */
			continue;
		}

		tsk_oom_adj = task->signal->oom_score_adj;
		if (!verbose && tsk_oom_adj &&
		    (tsk_oom_adj <= LMK_SERVICE_ADJ) &&
		    (get_mm_rss(task->mm) + get_mm_counter(task->mm, MM_SWAPENTS) < LMK_PRT_TSK_RSS)) {
			task_unlock(task);
			continue;
		}

		tsk_nr_ptes = get_tsk_nr_ptes_value(task);
		task_state = task_state_to_char(task);
		frozen_mark = frozen(task) ? '*' : ' ';

#ifdef CONFIG_PM_PEAK
		pr_info("[%5d] %5d %5d %8lu %6lu %5lu %5lu %8lu %5hd %c %s%c\n",
#else
		pr_info("[%5d] %5d %5d %8lu %6lu %5lu %5lu %5hd %c %s%c\n",
#endif
			task->pid, from_kuid(&init_user_ns, task_uid(task)),
			task->tgid, task->mm->total_vm, get_mm_rss(task->mm),
			tsk_nr_ptes,
			get_mm_counter(task->mm, MM_SWAPENTS),
#ifdef CONFIG_PM_PEAK
			task->mm->hiwater_pm,
#endif
			tsk_oom_adj,
			task_state,
			task->comm,
			frozen_mark);
		task_unlock(task);
	}
	rcu_read_unlock();
}

static void dump_kswapd(int nid, const char *stage)
{
	pg_data_t *pgdat = NODE_DATA(nid);

	if (!pgdat || !(pgdat->kswapd))
		return;

	pr_err("Show kswapd%d %s stack:\n", nid, stage);
	sched_show_task(pgdat->kswapd);
}

static void show_kswapd_stack(const char *stage)
{
	int nid;

	for_each_node_state(nid, N_MEMORY) {
		if (nid != 0)
			continue;
		dump_kswapd(nid, stage);
	}
}

static void lowmem_dump(struct work_struct *work)
{
	bool verbose = (work == &lowmem_dbg_verbose_wk) ? true : false;

	mutex_lock(&lowmem_dump_mutex);
	show_kswapd_stack("start");
	show_mem(SHOW_MEM_FILTER_NODES, NULL);
	show_oom_notifier_mem_info();
	dump_tasks(verbose);
#ifdef CONFIG_SLABINFO
	show_slab(verbose);
#endif
#ifdef CONFIG_DFX_MEMCHECK
#ifdef CONFIG_DFX_OHOS
	/* OHOS Kernel alloc information is temporarily printed */
	trace_hck_mm_mem_stats_show(0);
#else
	if (verbose)
		trace_hck_mm_mem_stats_show(0);
#endif
#endif
	show_kswapd_stack("end");

	/* Kswapd may be stuck in interrupt, so show cpu running stack */
	pr_err("Show cpu running state when lowmem:\n");
	__handle_sysrq('l', false);
	mutex_unlock(&lowmem_dump_mutex);
}

void hisi_lowmem_dbg(short oom_score_adj)
{
	unsigned long long jiffs = get_jiffies_64();

	if (oom_score_adj == 0) {
		schedule_work(&lowmem_dbg_verbose_wk);
	} else if (time_after64(jiffs, (last_jiffs + LMK_INTERVAL * HZ))) {
		last_jiffs = get_jiffies_64();
		schedule_work(&lowmem_dbg_wk);
	}
}

void hisi_lowmem_dbg_timeout(struct task_struct *p, struct task_struct *leader)
{
	struct task_struct *t = NULL;

	pr_info("timeout '%s' %d\n", leader->comm, leader->pid);
	for_each_thread(p, t) {
		pr_info("  pid=%d tgid=%d %s mm=%d frozen=%d 0x%lx 0x%x\n",
			t->pid, t->tgid, t->comm, t->mm ? 1 : 0,
			frozen(t), t->state, t->flags);
	}
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5,4,0))
	show_stack(leader, NULL, KERN_ERR);
#else
	show_stack(leader, NULL);
#endif
}
