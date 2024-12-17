/*
 * Copyright (c) 2015, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/swap.h>
#include <linux/sort.h>
#include <linux/oom.h>
#include <linux/sched.h>
#include <linux/rcupdate.h>
#include <linux/notifier.h>
#include <linux/vmpressure.h>
#include <linux/pagewalk.h>
#include <linux/swapops.h>
#include <linux/rmap.h>
#include <linux/mm_inline.h>
#include <linux/ctype.h>
#include <linux/sched/mm.h>
#include <securec.h>
#include "../../../fs/proc/internal.h"

#define CREATE_TRACE_POINTS
#include <trace/events/process_reclaim.h>

#define MAX_SWAP_TASKS SWAP_CLUSTER_MAX

extern vm_fault_t do_swap_page(struct vm_fault *vmf);

static void swap_fn(struct work_struct *work);
DECLARE_WORK(swap_work, swap_fn);

/* User knob to enable/disable process reclaim feature */
static int enable_process_reclaim;
module_param_named(enable_process_reclaim, enable_process_reclaim, int, 0644);

/* The max number of pages tried to be reclaimed in a single run */
int per_swap_size = SWAP_CLUSTER_MAX * 32;
module_param_named(per_swap_size, per_swap_size, int, 0644);

int reclaim_avg_efficiency;
module_param_named(reclaim_avg_efficiency, reclaim_avg_efficiency, int, 0444);

/* The vmpressure region where process reclaim operates */
static unsigned long pressure_min = 50;
static unsigned long pressure_max = 90;
module_param_named(pressure_min, pressure_min, ulong, 0644);
module_param_named(pressure_max, pressure_max, ulong, 0644);

/*
 * Scheduling process reclaim workqueue unecessarily
 * when the reclaim efficiency is low does not make
 * sense. We try to detect a drop in efficiency and
 * disable reclaim for a time period. This period and the
 * period for which we monitor a drop in efficiency is
 * defined by swap_eff_win. swap_opt_eff is the optimal
 * efficincy used as theshold for this.
 */
static int swap_eff_win = 2;
module_param_named(swap_eff_win, swap_eff_win, int, 0644);

static int swap_opt_eff = 50;
module_param_named(swap_opt_eff, swap_opt_eff, int, 0644);

static atomic_t skip_reclaim = ATOMIC_INIT(0);
/* Not atomic since only a single instance of swap_fn run at a time */
static int monitor_eff;

struct selected_task {
	struct task_struct *p;
	int tasksize;
	short oom_score_adj;
};

int selected_cmp(const void *a, const void *b)
{
	const struct selected_task *x = a;
	const struct selected_task *y = b;
	int ret;

	ret = x->tasksize < y->tasksize ? -1 : 1;

	return ret;
}

static int test_task_flag(struct task_struct *p, int flag)
{
	struct task_struct *t = p;

	rcu_read_lock();
	for_each_thread(p, t) {
		task_lock(t);
		if (test_tsk_thread_flag(t, flag)) {
			task_unlock(t);
			rcu_read_unlock();
			return 1;
		}
		task_unlock(t);
	}
	rcu_read_unlock();

	return 0;
}

static void swap_fn(struct work_struct *work)
{
	struct task_struct *tsk = NULL;
	struct reclaim_param rp;

	/* Pick the best MAX_SWAP_TASKS tasks in terms of anon size */
	struct selected_task selected[MAX_SWAP_TASKS] = {{0, 0, 0},};
	int si = 0;
	int i;
	int tasksize;
	int total_sz = 0;
	short min_score_adj = 360;
	int total_scan = 0;
	int total_reclaimed = 0;
	int nr_to_reclaim;
	int efficiency;

	rcu_read_lock();
	for_each_process(tsk) {
		struct task_struct *p;
		short oom_score_adj;

		if (tsk->flags & PF_KTHREAD)
			continue;

		if (test_task_flag(tsk, TIF_MEMDIE))
			continue;

		p = find_lock_task_mm(tsk);
		if (!p)
			continue;

		oom_score_adj = p->signal->oom_score_adj;
		if (oom_score_adj < min_score_adj) {
			task_unlock(p);
			continue;
		}

		tasksize = get_mm_counter(p->mm, MM_ANONPAGES);
		task_unlock(p);

		if (tasksize <= 0)
			continue;

		if (si == MAX_SWAP_TASKS) {
			sort(&selected[0], MAX_SWAP_TASKS,
					sizeof(struct selected_task),
					&selected_cmp, NULL);
			if (tasksize < selected[0].tasksize)
				continue;
			selected[0].p = p;
			selected[0].oom_score_adj = oom_score_adj;
			selected[0].tasksize = tasksize;
		} else {
			selected[si].p = p;
			selected[si].oom_score_adj = oom_score_adj;
			selected[si].tasksize = tasksize;
			si++;
		}
	}

	for (i = 0; i < si; i++)
		total_sz += selected[i].tasksize;

	/* Skip reclaim if total size is too less */
	if (total_sz < SWAP_CLUSTER_MAX) {
		rcu_read_unlock();
		return;
	}

	for (i = 0; i < si; i++)
		get_task_struct(selected[i].p);

	rcu_read_unlock();

	while (si--) {
		nr_to_reclaim =
			(selected[si].tasksize * per_swap_size) / total_sz;
		/* scan atleast a page */
		if (!nr_to_reclaim)
			nr_to_reclaim = 1;

		rp = reclaim_task_anon(selected[si].p, nr_to_reclaim);

		trace_process_reclaim(selected[si].tasksize,
				selected[si].oom_score_adj, rp.nr_scanned,
				rp.nr_reclaimed, per_swap_size, total_sz,
				nr_to_reclaim);
		total_scan += rp.nr_scanned;
		total_reclaimed += rp.nr_reclaimed;
		put_task_struct(selected[si].p);
	}

	if (total_scan) {
		efficiency = (total_reclaimed * 100) / total_scan;

		if (efficiency < swap_opt_eff) {
			if (++monitor_eff == swap_eff_win) {
				atomic_set(&skip_reclaim, swap_eff_win);
				monitor_eff = 0;
			}
		} else {
			monitor_eff = 0;
		}

		reclaim_avg_efficiency =
			(efficiency + reclaim_avg_efficiency) / 2;
		trace_process_reclaim_eff(efficiency, reclaim_avg_efficiency);
	}
}

static int vmpressure_notifier(struct notifier_block *nb,
			unsigned long action, void *data)
{
	unsigned long pressure = action;

	if (!enable_process_reclaim)
		return 0;

	if (!current_is_kswapd())
		return 0;

	if (atomic_dec_if_positive(&skip_reclaim) >= 0)
		return 0;

	if ((pressure >= pressure_min) && (pressure < pressure_max))
		if (!work_pending(&swap_work))
			queue_work(system_unbound_wq, &swap_work);
	return 0;
}

static struct notifier_block vmpr_nb = {
	.notifier_call = vmpressure_notifier,
};

static int __init process_reclaim_init(void)
{
	vmpressure_notifier_register(&vmpr_nb);
	return 0;
}

static void __exit process_reclaim_exit(void)
{
	vmpressure_notifier_unregister(&vmpr_nb);
}

module_init(process_reclaim_init);
module_exit(process_reclaim_exit);


#ifdef CONFIG_HISI_SWAP_ZDATA
static void do_lru_gettimeofday(struct timeval *tv)
{
	struct timespec64 now;
	ktime_get_real_ts64(&now);
	tv->tv_sec = now.tv_sec;
	tv->tv_usec = now.tv_nsec/1000;
}
#endif

static int swapin_pte_range(pmd_t *pmd, unsigned long addr,
			    unsigned long end, struct mm_walk *walk)
{
	struct reclaim_param *rp = walk->private;
	struct vm_area_struct *vma = rp->vma;
	pte_t *pte = NULL;
	struct vm_fault vmf = {
		.vma = vma,
		.address = addr,
		.flags = vma->vm_flags,
		.pmd = pmd,
	};
	split_huge_pmd(vma, pmd, addr);
	if (pmd_trans_unstable(pmd))
		return 0;
	pte = pte_offset_map(pmd, addr);
	for (; addr != end; pte++, addr += PAGE_SIZE) {
		vmf.orig_pte = *pte;
		if (is_swap_pte(vmf.orig_pte))
			do_swap_page(&vmf);
	}
	return 0;
}

static bool reclaim_check_page(struct reclaim_param *rp,
				struct page *page)
{
#if defined(CONFIG_TASK_PROTECT_LRU) || defined(CONFIG_MEMCG_PROTECT_LRU)
	/*
	 * don't reclaim page in protected.
	 */
	if (PageProtect(page))
		return true;
#endif

#ifdef CONFIG_TRANSPARENT_HUGEPAGE
	/* TODO: also reclaim huge page */
	if (PageTransCompound(page))
		return true;
#endif

	/*
	 * we don't reclaim page in active lru list
	 */
	if (rp->inactive_lru && (PageActive(page) ||
	    PageUnevictable(page)))
		return true;

	if (!page_evictable(page) && PageUnevictable(page))
		return true;

	return false;
}

static int reclaim_pte_range(pmd_t *pmd, unsigned long addr,
				unsigned long end, struct mm_walk *walk)
{
	struct reclaim_param *rp = walk->private;
	struct vm_area_struct *vma = rp->vma;
	pte_t *pte = NULL;
	pte_t ptent;
	spinlock_t *ptl = NULL;
	struct page *page = NULL;
	LIST_HEAD(page_list);
	int isolated;
	int reclaimed;
#ifdef CONFIG_ISOLATE_COUNT
	int file;
#endif

	split_huge_pmd(vma, pmd, addr);
	if (pmd_trans_unstable(pmd) || !rp->nr_to_reclaim)
		return 0;
cont:
#ifdef CONFIG_HISI_SWAP_ZDATA
	if (process_reclaim_need_abort(walk))
		return -EINTR;
#endif
	isolated = 0;
	pte = pte_offset_map_lock(vma->vm_mm, pmd, addr, &ptl);
	for (; addr != end; pte++, addr += PAGE_SIZE) {
		ptent = *pte;
		if (!pte_present(ptent))
			continue;

		page = vm_normal_page(vma, addr, ptent);
		if (!page)
			continue;

		if (reclaim_check_page(rp, page))
			continue;

		if (isolate_lru_page(page))
			continue;

		/* MADV_FREE clears pte dirty bit and then marks the page
		 * lazyfree (clear SwapBacked). Inbetween if this lazyfreed page
		 * is touched by user then it becomes dirty.  PPR in
		 * shrink_page_list in try_to_unmap finds the page dirty, marks
		 * it back as PageSwapBacked and skips reclaim. This can cause
		 * isolated count mismatch.
		 */
		if (PageAnon(page) && !PageSwapBacked(page)) {
			putback_lru_page(page);
			continue;
		}

		list_add(&page->lru, &page_list);
#ifdef CONFIG_ISOLATE_COUNT
		file = page_is_file_lru(page);
		inc_node_page_state(page, NR_ISOLATED_ANON +
				file);
		if (file)
			atomic_long_add(1, &p_reclaim_file_nums);
#else
		inc_node_page_state(page, NR_ISOLATED_ANON +
				page_is_file_lru(page));
#endif
		isolated++;
		rp->nr_scanned++;
		if ((isolated >= SWAP_CLUSTER_MAX) || !rp->nr_to_reclaim)
			break;
	}
	pte_unmap_unlock(pte - 1, ptl);

#ifdef CONFIG_HISI_SWAP_ZDATA
	reclaimed = (int)reclaim_pages_from_list(&page_list, vma,
				rp->hiber, &rp->nr_writedblock);
#else
	reclaimed = reclaim_pages_from_list(&page_list, vma);
#endif
	rp->nr_reclaimed += reclaimed;
	rp->nr_to_reclaim -= reclaimed;
	if (rp->nr_to_reclaim < 0)
		rp->nr_to_reclaim = 0;

	if (rp->nr_to_reclaim && (addr != end))
		goto cont;

	cond_resched();
	return 0;
}

enum reclaim_type {
	RECLAIM_FILE,
	RECLAIM_ANON,
	RECLAIM_ALL,
	RECLAIM_RANGE,
	RECLAIM_SOFT,
	RECLAIM_INACTIVE,
	RECLAIM_SWAPIN,
};

struct reclaim_param reclaim_task_anon(struct task_struct *task,
		int nr_to_reclaim)
{
	struct mm_struct *mm = NULL;
	struct vm_area_struct *vma = NULL;
	struct mm_walk reclaim_walk = {
		.ops = &(struct mm_walk_ops){.pmd_entry = reclaim_pte_range},
	};
	struct reclaim_param rp;

	rp.nr_reclaimed = 0;
	rp.nr_scanned = 0;
	get_task_struct(task);
	mm = get_task_mm(task);
	if (!mm)
		goto out;

	reclaim_walk.mm = mm;

	rp.nr_to_reclaim = nr_to_reclaim;
	reclaim_walk.private = &rp;

	down_read(&mm->mmap_lock);
	for (vma = mm->mmap; vma; vma = vma->vm_next) {
		if (is_vm_hugetlb_page(vma))
			continue;

		if (vma->vm_file)
			continue;

		if (!rp.nr_to_reclaim)
			break;

		rp.vma = vma;
		walk_page_range(mm, vma->vm_start, vma->vm_end,
			reclaim_walk.ops, reclaim_walk.private);
	}

	flush_tlb_mm(mm);
	up_read(&mm->mmap_lock);
	mmput(mm);
out:
	put_task_struct(task);
	return rp;
}

#ifdef CONFIG_RAMTURBO
int all_active_reclaim_by_task(struct task_struct *task)
{
	struct reclaim_param rp;
	if (!task)
		return 0;

	rp = reclaim_task_anon(task, INT_MAX);
	return rp.nr_reclaimed;
}
#endif

static ssize_t reclaim_write(struct file *file, const char __user *buf,
				size_t count, loff_t *ppos)
{
	struct task_struct *task = NULL;
	char buffer[200];
	struct mm_struct *mm = NULL;
	struct vm_area_struct *vma = NULL;
	enum reclaim_type type;
	char *type_buf = NULL;
	struct mm_walk reclaim_walk = {
		.ops = &(struct mm_walk_ops){.pmd_entry = reclaim_pte_range},
	};
	unsigned long start = 0;
	unsigned long end = 0;
	struct reclaim_param rp;
#ifdef CONFIG_HISI_SWAP_ZDATA
	int walk_ret;
	struct timeval start_time = {0,0};
	struct timeval stop_time;
	s64 elapsed_centisecs64;
	rp.hiber = false;
#endif

	(void)memset_s(buffer, sizeof(buffer), 0, sizeof(buffer));
	if (count > sizeof(buffer) - 1)
		count = sizeof(buffer) - 1;

	if (copy_from_user(buffer, buf, count))
		return -EFAULT;

	type_buf = strstrip(buffer);
	if (!strcmp(type_buf, "soft"))
		type = RECLAIM_SOFT;
	else if (!strcmp(type_buf, "inactive"))
		type = RECLAIM_INACTIVE;
	else if (!strcmp(type_buf, "file"))
		type = RECLAIM_FILE;
	else if (!strcmp(type_buf, "anon"))
		type = RECLAIM_ANON;
	else if (!strcmp(type_buf, "all"))
		type = RECLAIM_ALL;
	else if (!strcmp(type_buf, "swapin"))
		type = RECLAIM_SWAPIN;
#ifdef CONFIG_HISI_SWAP_ZDATA
	else if (!strcmp(type_buf, "hiber")) {
		type = RECLAIM_ALL;
		rp.hiber = true;
	} else if (!strcmp(type_buf, "hiber_anon")) {
		type = RECLAIM_ANON;
		rp.hiber = true;
	} else if (!strcmp(type_buf, "hiber_file")) {
		type = RECLAIM_FILE;
		rp.hiber = true;
	}
#endif
	else if (isdigit(*type_buf))
		type = RECLAIM_RANGE;
	else
		goto out_err;

	rp.inactive_lru = false;

	if (type == RECLAIM_RANGE) {
		char *token;
		unsigned long long len, len_in, tmp;
		token = strsep(&type_buf, " ");
		if (!token)
			goto out_err;
		tmp = memparse(token, &token);
		if (tmp & ~PAGE_MASK || tmp > ULONG_MAX)
			goto out_err;
		start = tmp;

		token = strsep(&type_buf, " ");
		if (!token)
			goto out_err;
		len_in = memparse(token, &token);
		len = (len_in + ~PAGE_MASK) & PAGE_MASK;
		if (len > ULONG_MAX)
			goto out_err;
		/*
		 * Check to see whether len was rounded up from small -ve
		 * to zero.
		 */
		if (len_in && !len)
			goto out_err;

		end = start + len;
		if (end < start)
			goto out_err;
	}

	task = get_proc_task(file->f_path.dentry->d_inode);
	if (!task)
		return -ESRCH;

	mm = get_task_mm(task);
	if (!mm)
		goto out;

#ifdef CONFIG_MM_SMART_RECLAIM
	/* here we add a soft shrinker for reclaim */
	if (type == RECLAIM_SOFT) {
		smart_soft_shrink(mm);
		mmput(mm);
		goto out;
	}
#endif

	if (type == RECLAIM_INACTIVE)
		rp.inactive_lru = true;

	reclaim_walk.mm = mm;

	rp.nr_to_reclaim = INT_MAX;
	rp.nr_reclaimed = 0;
	reclaim_walk.private = &rp;
#ifdef CONFIG_HISI_SWAP_ZDATA
	rp.nr_writedblock = 0;
	if (rp.hiber)
		do_lru_gettimeofday(&start_time);
#endif

	down_read(&mm->mmap_lock);
	if (type == RECLAIM_RANGE) {
		vma = find_vma(mm, start);
		while (vma) {
			if (vma->vm_start > end)
				break;
			if (is_vm_hugetlb_page(vma))
				continue;

			rp.vma = vma;
			walk_page_range(mm, max(vma->vm_start, start),
					min(vma->vm_end, end),
					reclaim_walk.ops, reclaim_walk.private);
			vma = vma->vm_next;
		}
	} else if (type == RECLAIM_SWAPIN) {
		for (vma = mm->mmap; vma; vma = vma->vm_next) {
			reclaim_walk.ops = &(struct mm_walk_ops){.pmd_entry = swapin_pte_range};
			rp.vma = vma;
			reclaim_walk.private = &rp;
			walk_page_range(mm, vma->vm_start, vma->vm_end,
					reclaim_walk.ops, reclaim_walk.private);
		}
	} else {
		for (vma = mm->mmap; vma; vma = vma->vm_next) {
			if (is_vm_hugetlb_page(vma))
				continue;

			if (type == RECLAIM_ANON && vma->vm_file)
				continue;

			if (type == RECLAIM_FILE && !vma->vm_file)
				continue;

			rp.vma = vma;
#ifdef CONFIG_HISI_SWAP_ZDATA
			walk_ret = walk_page_range(mm, vma->vm_start, vma->vm_end,
				reclaim_walk.ops, reclaim_walk.private);
			if ((walk_ret == -EINTR) && rp.hiber)
				break;
#else
			walk_page_range(mm, vma->vm_start, vma->vm_end,
				reclaim_walk.ops, reclaim_walk.private);
#endif
		}
	}

	flush_tlb_mm(mm);
	up_read(&mm->mmap_lock);
	mmput(mm);
#ifdef CONFIG_HISI_SWAP_ZDATA
	if (rp.hiber) {
		do_lru_gettimeofday(&stop_time);
		elapsed_centisecs64 = timeval_to_ns(&stop_time) -
					timeval_to_ns(&start_time);

		process_reclaim_result_write(task, (unsigned)rp.nr_reclaimed,
			rp.nr_writedblock, elapsed_centisecs64);
	}
#endif
out:
	put_task_struct(task);
	return count;

out_err:
	return -EINVAL;
}

const struct file_operations proc_reclaim_operations = {
	.write		= reclaim_write,
	.llseek		= noop_llseek,
};

