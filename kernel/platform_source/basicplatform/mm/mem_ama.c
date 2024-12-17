/*
 * Copyright (c) 2022-2023 HiSilicon Technologies Co., Ltd.
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * File Name:    mem_ama.c
 */

#include <linux/dma-buf.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/fdtable.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/mempolicy.h>
#include <linux/module.h>
#include <linux/namei.h>
#include <linux/nodemask.h>
#include <linux/oom.h>
#include <linux/pid.h>
#include <linux/proc_fs.h>
#include <linux/statfs.h>
#include <linux/swap.h>
#include <linux/sysctl.h>
#include <linux/version.h>
#include <linux/workqueue.h>
#include <linux/sched/mm.h>
#include <platform_include/basicplatform/linux/mem_ama.h>

#ifdef CONFIG_ION
#include <linux/ion/mm_ion.h>
#endif

#ifdef CONFIG_HW_RECLAIM_ACCT
#include <chipset_common/reclaim_acct/reclaim_acct.h>
#endif

#include "sched.h"

/* Portion of address space to scan in MB */
unsigned int sysctl_ama_migrate_scan_size = 32;

/* Scan @scan_size MB every @scan_period after an initial @scan_delay in ms */
unsigned int sysctl_ama_migrate_scan_delay = 1000;

/* AMA kshrink must check whether pegs not enough to reclaim */
unsigned int sysctl_ama_shrink_watermark = 10 * SZ_1M / PAGE_SIZE;

/* ksrhinkd delay in ms */
unsigned int sysctl_ama_kshrinkd_scan_delay = 100;

/*
 * This struct is the work time that the PTEs will be marked
 * pte_none. Page hinting faults will gather statistics and
 * migrate pages to new nodes if necessary.
 */
struct ama_migrate_desc {
	unsigned int		ama_preferred_nid;

	unsigned long		ama_scan_seq;
	unsigned int		ama_scan_period;
	unsigned long		ama_scan_size;

	/* work flag prevents two kworker setting pte */
	struct work_struct	ama_work;
	atomic_t		ama_work_wait;

	unsigned long		total_ama_faults;

	/*
	 * faults_locality tracks if faults recorded during the last
	 * scan window were remote/local or failed to migrate. The task scan
	 * period is adapted based on the locality of the faults with different
	 * weights depending on whether they were shared or private faults
	 */
	unsigned long	ama_faults_locality[(1 << CONFIG_NODES_SHIFT) + 1];
	unsigned long	ama_pages_migrated;
};

struct ama_dbuf_process_size_args {
	struct task_struct *tsk;
	size_t *total_dbuf_size;
};

static struct mempolicy ama_default_policy[AMA_TYPE_SUM];
static struct ama_migrate_desc *ama_migrate_info;

unsigned long last_kshrinkd_time;

/*
 * mempolicy for kernel large alloc
 */
struct page *ama_alloc_kernel_large(gfp_t gfp, unsigned order)
{
	struct mempolicy *pol = NULL;

	pol = &ama_default_policy[AMA_KALLOC];

	if (!atomic_read(&pol->refcnt))
		return __alloc_pages_node(0, gfp, order);

	/*
	 * set cureent allowed pol->node_mask
	 */
	nodes_setall(current->mems_allowed);

	return __alloc_pages_nodemask(gfp, order,
				pol->v.preferred_node,
				&pol->w.cpuset_mems_allowed);
}

/*
 * mempolicy for filemap page cache
 */
struct page *ama_alloc_page_cache(gfp_t gfp, unsigned order)
{
	struct mempolicy *pol = NULL;

	pol = &ama_default_policy[AMA_FILE];

	if (!atomic_read(&pol->refcnt))
		return __alloc_pages_node(0, gfp, order);

	/*
	 * set cureent allowed pol->node_mask
	 */
	nodes_setall(current->mems_allowed);

	return __alloc_pages_nodemask(gfp, order,
				pol->v.preferred_node,
				&pol->w.cpuset_mems_allowed);
}

/*
 * mempolicy for vma anon page
 */
struct page *ama_alloc_anon_pages(gfp_t gfp, unsigned order)
{
	struct mempolicy *pol = &ama_default_policy[AMA_ANON];
	struct task_struct *tsk = current;
	struct page *page;

	/*
	 * set cureent allowed pol->node_mask
	 */
	nodes_setall(current->mems_allowed);
	/*
	 * Preferred alloc anon pages from
	 * Node0 for foreground process.
	 * And Other anon alloced by anon mempolicy.
	 */
	if (uclamp_latency_sensitive(tsk))
		page = __alloc_pages_nodemask(gfp, order,
					      LOCAL_NODE0, &pol->w.cpuset_mems_allowed);
	else
		page = __alloc_pages_nodemask(gfp, order,
					pol->v.preferred_node, &pol->w.cpuset_mems_allowed);

	return page;
}

/*
 * Dtsi set all LOCAL NODE is NODE1
 * Zram page must be in NODE1 by GFP_THISNODE
 */
struct page *ama_alloc_zram_page(gfp_t gfp)
{
	return alloc_page(gfp | __GFP_THISNODE);
}

/*
 * For feature, we need to identify big bw ion
 * buffer and independently set up two interfaces
 * for big bw and normal bw buffer.
 */
struct page *ama_alloc_meida_pages(gfp_t gfp, unsigned order)
{
	struct mempolicy *pol = &ama_default_policy[AMA_MEDIA];

	nodes_setall(current->mems_allowed);
	/*
	 * First if DDR bandwidth overflow,
	 * All pages need to alloc interleaved.
	 */
	if (pol->mode == MPOL_INTERLEAVE)
		return alloc_page_interleave_exp(gfp, order,
				     interleave_nodes_exp(pol));
	return  __alloc_pages_nodemask(gfp, order,
				pol->v.preferred_node, &pol->w.cpuset_mems_allowed);
}

unsigned long ama_alloc_media_pages_bulk_list(gfp_t gfp,
		unsigned long nr_pages, struct list_head *page_list)
{
	struct mempolicy *pol = &ama_default_policy[AMA_MEDIA];

	nodes_setall(current->mems_allowed);
	/*
	 * First if DDR bandwidth overflow,
	 * All pages need to alloc interleaved.
	 */
	if (pol->mode == MPOL_INTERLEAVE)
		return alloc_page_interleave_bulk_list_exp(gfp, nr_pages,
				interleave_nodes_exp(pol), page_list);
	return __alloc_pages_bulk(gfp, pol->v.preferred_node, &pol->w.cpuset_mems_allowed,
			nr_pages, page_list, NULL);
}

/*
 * Some page alloc interfaces require special processing
 *
 * e.g.
 *   devm_kmalloc uses slub_alloc(preferred node1) func.
 * but some special driver(pcie for example) needed memory
 * section between 0G ~ 4G for Hardware access.
 *
 * So, here we apply some special page alloc func.
 */
struct devres *ama_alloc_dr(dr_release_t release,
			     size_t size, gfp_t gfp)
{
	return alloc_dr_exp(release, size, gfp, LOCAL_NODE0);
}

/*
 * When do_fault,
 * if the pte of page is none and vma is right,
 * we migrate this page to another Node.
 */
vm_fault_t do_ama_page(struct vm_fault *vmf)
{
	struct vm_area_struct *vma = vmf->vma;
	struct page *page = NULL;
	int page_nid = NUMA_NO_NODE;
	int last_cpupid;
	int target_nid;
	int migrated = 0;
	pte_t pte, old_pte;
	bool was_writable = pte_savedwrite(vmf->orig_pte);
	unsigned int flags = 0;

	/*
	 * The "pte" at this point cannot be used safely without
	 * validation through pte_unmap_same(). It's of NUMA type but
	 * the pfn may be screwed if the read is non atomic.
	 */
	if (!pte_spinlock_exp(vmf))
		return VM_FAULT_RETRY;
	if (unlikely(!pte_same(*vmf->pte, vmf->orig_pte))) {
		pte_unmap_unlock(vmf->pte, vmf->ptl);
		goto out;
	}

	/*
	 * Make it present again, Depending on how arch implementes non
	 * accessible ptes, some can allow access by kernel mode.
	 */
	old_pte = ptep_modify_prot_start(vma, vmf->address, vmf->pte);
	pte = pte_modify(old_pte, vmf->vma_page_prot);
	pte = pte_mkyoung(pte);
	if (was_writable)
		pte = pte_mkwrite(pte);
	ptep_modify_prot_commit(vma, vmf->address, vmf->pte, old_pte, pte);
	update_mmu_cache(vma, vmf->address, vmf->pte);

	page = _vm_normal_page(vma, vmf->address, pte, vmf->vma_flags);
	if (!page) {
		pte_unmap_unlock(vmf->pte, vmf->ptl);
		return 0;
	}

	/* TODO: handle PTE-mapped THP */
	if (PageCompound(page)) {
		pte_unmap_unlock(vmf->pte, vmf->ptl);
		return 0;
	}

	/*
	 * Avoid grouping on RO pages in general. RO pages shouldn't hurt as
	 * much anyway since they can be in shared cache state. This misses
	 * the case where a mapping is writable but the process never writes
	 * to it but pte_write gets cleared during protection updates and
	 * pte_dirty has unpredictable behaviour between PTE scan updates,
	 * background writeback, dirty balancing and application behaviour.
	 */
	if (!pte_write(pte))
		flags |= TNF_NO_GROUP;

	/*
	 * Flag if the page is shared between multiple address spaces. This
	 * is later used when determining whether to group tasks together
	 */
	if (page_mapcount(page) > 1 && (vmf->vma_flags & VM_SHARED))
		flags |= TNF_SHARED;

	last_cpupid = page_cpupid_last(page);
	page_nid = page_to_nid(page);
	target_nid = LOCAL_NODE0;
	pte_unmap_unlock(vmf->pte, vmf->ptl);
	if (target_nid == page_nid) {
		pr_err("%s: page nid%d pfn 0x%llx", __func__, page_nid, page_to_pfn(page));
		put_page(page);
		goto out;
	}

	/* Migrate to the requested node */
	migrated = migrate_misplaced_page(page, vmf, target_nid);
	if (migrated) {
		page_nid = target_nid;
		flags |= TNF_MIGRATED;
	} else {
		flags |= TNF_MIGRATE_FAIL;
	}

out:
	return 0;
}

/*
 * This is used to mark a range of virtual addresses to be inaccessible.
 * These are later cleared by a NUMA hinting fault. Depending on these
 * faults, pages may be migrated for better NUMA placement.
 *
 * This is assuming that NUMA faults are handled using PROT_NONE. If
 * an architecture makes a different choice, it will need further
 * changes to the core.
 */
#if defined(CONFIG_HYPERHOLD) && defined(CONFIG_MM_AMA_V2)
static unsigned long change_prot_ama(struct vm_area_struct *vma,
			unsigned long addr, unsigned long end)
{
	unsigned long nr_updated;

	vm_write_begin(vma);
	nr_updated = change_protection(vma, addr, end, PAGE_NONE, MM_CP_PROT_NUMA);
	if (nr_updated)
		count_vm_numa_events(NUMA_PTE_UPDATES, nr_updated);
	vm_write_end(vma);

	return nr_updated;
}

static bool ama_walk_vma_list(struct mm_struct *mm)
{
	struct vm_area_struct *vma;
	unsigned long start, end;
	unsigned long nr_pte_updates = 0;
	long pages, virtpages;
	bool ret = false;

	if (!mmap_read_trylock(mm))
		return ret;

	pages = ama_migrate_info->ama_scan_size;
	pages <<= 20 - PAGE_SHIFT; /* MB in pages */
	virtpages = pages * 8;	   /* Scan up to this much virtual space */
	if (!pages)
		goto out;

	start = 0;
	vma = find_vma(mm, start);
	if (!vma) {
		start = 0;
		vma = mm->mmap;
	}

	for (; vma; vma = vma->vm_next) {
		if (!vma_migratable(vma) || !vma_policy_mof(vma) ||
			is_vm_hugetlb_page(vma) || (vma->vm_flags & VM_MIXEDMAP)) {
			continue;
		}

		/*
		 * Shared library pages mapped by multiple processes are not
		 * migrated as it is expected they are cache replicated. Avoid
		 * hinting faults in read-only file-backed mappings or the vdso
		 * as migrating the pages will be of marginal benefit.
		 */
		if (!vma->vm_mm ||
		    (vma->vm_file && (vma->vm_flags & (VM_READ|VM_WRITE)) == (VM_READ)))
			continue;

		/*
		 * Skip inaccessible VMAs to avoid any confusion between
		 * PROT_NONE and NUMA hinting ptes
		 */
		if (!vma_is_accessible(vma))
			continue;

		do {
			start = max(start, vma->vm_start);
			end = ALIGN(start + (pages << PAGE_SHIFT), HPAGE_SIZE);
			end = min(end, vma->vm_end);

			/*
			 * Try to scan sysctl_numa_balancing_size worth of
			 * hpages that have at least one present PTE that
			 * is not already pte-numa. If the VMA contains
			 * areas that are unused or already full of prot_numa
			 * PTEs, scan up to virtpages, to skip through those
			 * areas faster.
			 */
			nr_pte_updates = change_prot_ama(vma, start, end);
			if (nr_pte_updates)
				pages -= (end - start) >> PAGE_SHIFT;
			virtpages -= (end - start) >> PAGE_SHIFT;

			start = end;
			if (pages <= 0 || virtpages <= 0)
				goto out;

			cond_resched();
		} while (end != vma->vm_end);
	}
	ret = true;

out:
	mmap_read_unlock(mm);
	return ret;
}

static bool ama_walk_mm_list(struct mem_cgroup *memcg, int nid)
{
	struct mm_struct *mm = NULL;
	struct css_task_iter it;
	struct task_struct *task;
	bool ret = false;

	css_task_iter_start(&memcg->css, 0, &it);
	while (task = css_task_iter_next(&it)) {
		mm = get_task_mm(task);
		if (!mm || !uclamp_latency_sensitive(task))
			continue;

		ret = true;
		if (ama_walk_vma_list(mm))
			break;
	}
	css_task_iter_end(&it);

	return ret;
}
#endif

static void ama_walk_memcg_list(int nid)
{
#if defined(CONFIG_HYPERHOLD) && defined(CONFIG_MM_AMA_V2)
	struct mem_cgroup *memcg = NULL;

	while ((memcg = get_next_memcg(memcg))) {
		if (memcg == root_mem_cgroup)
			continue;

#ifdef CONFIG_MEMCG_PROTECT_LRU
		/* Skip if it is a protect memcg. */
		if (is_prot_memcg(memcg, false))
			continue;
#endif
		if (ama_walk_mm_list(memcg, nid))
			break;
	}
#endif
}

/*
 * This is a AMA kworker
 * when free pages were high enough on NodeX,
 * we check and migrate User pages to NodeX.
 */
static void task_ama_work(struct work_struct *work)
{
	unsigned long migrate, next_scan, now = jiffies;
	int i;

	/*
	 * Enforce maximal scan/migration frequency..
	 */
	migrate = ama_migrate_info->ama_scan_seq;
	if (time_before(now, migrate))
		return;

	/*
	 * migrate   next_scan         now
	 *    |----------|--------------|
	 * migrate is last work sched jiffer time.
	 * next_scan means that next work running time
	 * must be after this time.
	 * So now must be after next_scan.
	 */
	next_scan = migrate + msecs_to_jiffies(ama_migrate_info->ama_scan_period);
	if (time_before(now, next_scan))
		return;
	ama_migrate_info->ama_scan_seq = now;

	for (i = LOCAL_NODE1; i < LOCAL_NODE_SUM; i++)
		ama_walk_memcg_list(i);
}

void wake_ama_kworker(pg_data_t *pgdat)
{
	pg_data_t *pgdat1 = NODE_DATA(LOCAL_NODE1);
	int nid = pgdat->node_id;
	unsigned long pgdatfile;
	unsigned long pgdatanon;
	unsigned long pgdatfree;

	if (!is_node_part0(nid)) {
		pr_err("%s: nid%d wake mistake\n",
			__func__, nid);
		return;
	}

	pgdatfree = sum_zone_node_page_state(pgdat->node_id,
					NR_FREE_PAGES);
	pgdatfile = node_page_state(pgdat1, NR_ACTIVE_FILE) +
		node_page_state(pgdat1, NR_INACTIVE_FILE);
	pgdatanon = node_page_state(pgdat1, NR_ACTIVE_ANON) +
		node_page_state(pgdat1, NR_INACTIVE_ANON);
	if (pgdatfree < 600 * SZ_1M / PAGE_SIZE ||
	    pgdatfile < 50 * SZ_1M / PAGE_SIZE ||
		pgdatanon < 50 * SZ_1M / PAGE_SIZE)
		return;

	/*
	 * Check whether the work is running.
	 */
	if (atomic_read(&ama_migrate_info->ama_work_wait))
		return;
	atomic_set(&ama_migrate_info->ama_work_wait, 1);

	schedule_work(&ama_migrate_info->ama_work);
	flush_work(&ama_migrate_info->ama_work);

	atomic_set(&ama_migrate_info->ama_work_wait, 0);
}

static bool ama_task_equal_check(struct task_struct *new,
				struct task_struct *old)
{
	if (!old)
		return false;

	if (new == old)
		return true;

	if (new->tgid == old->tgid)
		return true;

	if (new->pid == old->pid)
		return true;

	return false;
}

static void ama_kill_selected(struct task_struct *selected,
			unsigned long selected_task_size)
{
	task_lock(selected);
	send_sig(SIGKILL, selected, 0);
	task_unlock(selected);

	pr_err("%s : Killing '%s':%d, tgid=%d, adj %hd\n"
		"   to free %ldkB on behalf of '%s' %d \n",
		__func__,
		selected->comm, selected->pid, selected->tgid,
		selected->signal->oom_score_adj,
		selected_task_size * (long)(PAGE_SIZE / 1024),
		current->comm, current->pid);
}

static int ama_dbuf_process_size_cb(const void *data,
				struct file *f, unsigned int fd)
{
	const struct ama_dbuf_process_size_args *args = data;
	struct dma_buf    *dbuf;

	if (!is_dma_buf_file(f))
		return 0;

	dbuf = f->private_data;
	if (!dbuf)
		return 0;

	*args->total_dbuf_size += dbuf->size;
	return 0;
}

static unsigned long ama_direct_kill(void)
{
	struct ama_dbuf_process_size_args cb_args;
	struct task_struct *p = NULL;
	struct task_struct *tsk = NULL;
	struct task_struct *selected = NULL;
	short oom_score_adj = 0;
	short oom_process_adj = 500;
	unsigned long selected_task_size = 0;
	int nr_to_kill = AMA_MAX_KILL_NUM;
	unsigned long task_size = 0;
	unsigned long rem = 0;

	rcu_read_lock();
	do {
		for_each_process(tsk) {
			if (tsk->flags & PF_KTHREAD)
				continue;

			p = find_lock_task_mm(tsk);
			if (!p)
				continue;

			if (ama_task_equal_check(p, selected)) {
				task_unlock(p);
				continue;
			}

			if (test_tsk_thread_flag(p, TIF_MEMDIE)) {
				task_unlock(p);
				continue;
			}

			if ((unsigned long)p->state & TASK_UNINTERRUPTIBLE) {
				task_unlock(p);
				continue;
			}

			oom_score_adj = p->signal->oom_score_adj;
			if (oom_score_adj < oom_process_adj) {
				task_unlock(p);
				continue;
			}

			if (task_lmk_waiting(p)){
				task_unlock(p);
				continue;
			}

			task_size = get_mm_rss(p->mm);
			cb_args.tsk = tsk;
			cb_args.total_dbuf_size = &task_size;
			iterate_fd(tsk->files, 0,
				ama_dbuf_process_size_cb, (void *)&cb_args);

			task_unlock(p);

			if (task_size <= 0)
				continue;

			if (selected && (task_size < selected_task_size))
				continue;

			oom_score_adj = 0;
			selected = p;
			selected_task_size = task_size;
		}

		if (selected) {
			ama_kill_selected(selected, selected_task_size);
			rem += selected_task_size;
		}

		p = NULL;
		tsk = NULL;
		selected_task_size = 0;
	} while (--nr_to_kill);

	rcu_read_unlock();

	return rem;
}

static bool ama_check_avail_buffer(pg_data_t *pgdat)
{
	struct zone *zone = NULL;
	unsigned long watermark = 0;
	unsigned long free_pages = 0;
	int i;

	/*
	 * AMA kshrink only support per node per zone
	 */
	for (i = MAX_NR_ZONES - 1; i >= 0; i--) {
		zone = pgdat->node_zones + i;
		if (!managed_zone(zone))
			continue;

		watermark = high_wmark_pages(zone);
		free_pages = zone_page_state(zone, NR_FREE_PAGES);
	}

	if (!watermark || free_pages > watermark)
		return true;
	return false;
}

static bool ama_check_min_water(pg_data_t *pgdat)
{
	struct zone *zone = NULL;
	unsigned long watermark = 0;
	unsigned long free_pages = 0;
	int i;

	/*
	 * AMA kshrink only support per node per zone
	 */
	for (i = MAX_NR_ZONES - 1; i >= 0; i--) {
		zone = pgdat->node_zones + i;
		if (!managed_zone(zone))
			continue;

		watermark += min_wmark_pages(zone);
		free_pages += zone_page_state(zone, NR_FREE_PAGES);
	}

	if (!watermark || free_pages > watermark / 3)
		return true;
	return false;
}

void wakeup_kshrinkd(pg_data_t *pgdat)
{
	unsigned long curr_interval = 0;

	if (IS_ERR(pgdat->kshrinkd))
		return;

	if (!wq_has_sleeper(&pgdat->kshrinkd_wait))
		return;

	if (ama_check_avail_buffer(pgdat))
		return;

	curr_interval = jiffies_to_msecs(jiffies - last_kshrinkd_time);
	if (curr_interval < sysctl_ama_kshrinkd_scan_delay) {
#ifdef CONFIG_HYPERHOLD
		count_vm_event(KSHRINKD_SKIP);
#endif
		return;
	}

#ifdef CONFIG_HYPERHOLD
	count_vm_event(KSHRINKD_WAKEUP);
#endif
	atomic_set(&pgdat->kshrinkd_wait_flag, 1);
	wake_up_interruptible(&pgdat->kshrinkd_wait);
}

void wake_all_kshrinkd(void)
{
	pg_data_t *pgdat = NULL;
	int nid;

	for_each_online_node(nid) {
		if (is_node_part0(nid))
			continue;

		pgdat = NODE_DATA(nid);
		wakeup_kshrinkd(pgdat);
	}
}

static bool ama_check_buffer_avail(pg_data_t *pgdat)
{
	unsigned long pgdatfile;
	unsigned long pgdatanon;

	unsigned long file_mapped = node_page_state(pgdat, NR_FILE_MAPPED);
	unsigned long file_lru = node_page_state(pgdat, NR_INACTIVE_FILE) +
				node_page_state(pgdat, NR_ACTIVE_FILE);

	pgdatfile = (file_lru > file_mapped) ? (file_lru - file_mapped) : 0;
	pgdatanon = node_page_state(pgdat, NR_ACTIVE_ANON) +
			node_page_state(pgdat, NR_INACTIVE_ANON);
	if (pgdatfile <= sysctl_ama_shrink_watermark &&
		pgdatanon <= sysctl_ama_shrink_watermark * 3)
		return false;

	return true;
}

static void ama_buffer_kill_check(pg_data_t *pgdat)
{
	unsigned long pgdatfree;
	unsigned long pgdation;

	pgdatfree = sum_zone_node_page_state(pgdat->node_id, NR_FREE_PAGES);
	pgdation = mm_ion_total();
	if (pgdatfree < SZ_1M / PAGE_SIZE &&
		pgdation > (SZ_1G + SZ_1G / 2))
		ama_direct_kill();
}

static bool ama_ion_cache_check(pg_data_t *pgdat)
{
	unsigned long ion_cache;

	ion_cache = node_page_state(pgdat, NR_NODE_IONCACHES) +
			node_page_state_pages(pgdat, NR_SLAB_RECLAIMABLE_B);
	if (ion_cache > (100 * SZ_1M / PAGE_SIZE))
		return true;

	return false;
}
static void ama_set_task_reclaim_state(struct task_struct *task,
					struct reclaim_state *rs)
{
	WARN_ON_ONCE(rs && task->reclaim_state);

	WARN_ON_ONCE(!rs && !task->reclaim_state);

	task->reclaim_state = rs;
}

static void kshrinkd_shrink_node(pg_data_t *pgdat)
{
	struct scan_control sc = {
		.gfp_mask = GFP_KERNEL,
		.order = 0,
		.priority = DEF_PRIORITY / 3,
		.may_writepage = !laptop_mode,
		.may_unmap = 1,
		.may_swap = 1,
		.reclaim_idx = MAX_NR_ZONES - 1,
#ifdef CONFIG_HYPERHOLD
		.invoker = KSWAPD,
#endif
	};
	unsigned long pflags;
	bool retry;

	ama_set_task_reclaim_state(current, &sc.reclaim_state);
	psi_memstall_enter(&pflags);
	__fs_reclaim_acquire();

	sc.nr_to_reclaim = 0;
#ifdef CONFIG_HYPERHOLD
	sc.nr_reclaimed_anon = 0;
	sc.nr_reclaimed_file = 0;
	sc.nr_scanned_anon = 0;
	sc.nr_scanned_file = 0;
#endif

	do {
		bool ret = false;

		retry = false;
		sc.nr_reclaimed = 0;
		sc.nr_scanned = 0;

		if (!ama_check_buffer_avail(pgdat)) {
#ifdef CONFIG_HYPERHOLD
			count_vm_event(KSHRINKD_LOW);
#endif
			if (ama_ion_cache_check(pgdat))
				shrink_slab(GFP_KERNEL, pgdat->node_id, root_mem_cgroup, 0);
			ama_buffer_kill_check(pgdat);
			pgdat->kswapd_failures++;
			break;
		}

		sc.nr_to_reclaim += SWAP_CLUSTER_MAX;
#ifdef CONFIG_HYPERHOLD_FILE_LRU
		shrink_node_hyperhold(pgdat, &sc);
#else
		ama_shrink_node(pgdat, &sc);
#endif

		if (waitqueue_active(&pgdat->pfmemalloc_wait) &&
				ama_check_min_water(pgdat))
			wake_up_all(&pgdat->pfmemalloc_wait);

		__fs_reclaim_release();
		ret = try_to_freeze();
		__fs_reclaim_acquire();
		if (ret || kthread_should_stop())
			break;

		if (ama_check_min_water(pgdat))
			break;

		if (sc.nr_reclaimed < sc.nr_to_reclaim ||
			sc.nr_scanned < sc.nr_to_reclaim) {
#ifdef CONFIG_HYPERHOLD
			count_vm_event(KSHRINKD_RETRY);
#endif
			sc.priority--;
			retry = true;
		}
	} while (sc.priority >= 1 && retry);

	if (sc.priority < 1 && retry) {
#ifdef CONFIG_HYPERHOLD
		count_vm_event(KSHRINKD_FAIL);
#endif
		pgdat->kswapd_failures = MAX_RECLAIM_RETRIES;
	}

	__fs_reclaim_release();
	psi_memstall_leave(&pflags);
	ama_set_task_reclaim_state(current, NULL);
	last_kshrinkd_time = jiffies;

#ifdef CONFIG_HYPERHOLD
	count_vm_events(KSHRINKD_RECLAIMED_ANON, sc.nr_reclaimed_anon);
	count_vm_events(KSHRINKD_RECLAIMED_FILE, sc.nr_reclaimed_file);
	count_vm_events(KSHRINKD_SCAN_ANON, sc.nr_scanned_anon);
	count_vm_events(KSHRINKD_SCAN_FILE, sc.nr_scanned_file);
#endif
}

static int kshrinkd(void *p)
{
	pg_data_t *pgdat = (pg_data_t *)p;
	struct task_struct *tsk = current;
	const struct cpumask *cpumask = cpu_online_mask;

	if (!cpumask_empty(cpumask))
		set_cpus_allowed_ptr(tsk, cpumask);

	tsk->flags |= PF_MEMALLOC | PF_SWAPWRITE | PF_KSWAPD;
	set_freezable();

	while (!kthread_should_stop()) {
		if (waitqueue_active(&pgdat->pfmemalloc_wait))
			wake_up_all(&pgdat->pfmemalloc_wait);

		(void)wait_event_freezable(pgdat->kshrinkd_wait,
			atomic_read(&pgdat->kshrinkd_wait_flag));
		atomic_set(&pgdat->kshrinkd_wait_flag, 0);

#ifdef CONFIG_HYPERHOLD
		count_vm_event(KSHRINKD_RUN);
#endif

#ifdef CONFIG_HW_RECLAIM_ACCT
		reclaimacct_start(KSHRINKD_RECLAIM);
#endif
		kshrinkd_shrink_node(pgdat);
#ifdef CONFIG_HW_RECLAIM_ACCT
		reclaimacct_end(KSHRINKD_RECLAIM);
#endif
	}

	tsk->flags &= ~(PF_MEMALLOC | PF_SWAPWRITE | PF_KSWAPD);
#ifdef CONFIG_HW_RECLAIM_ACCT
	reclaimacct_destroy();
#endif
	return 0;
}

/*
 * tsk start will be called by init and node-hot-add.
 */
static int kshrinkd_run(int nid)
{
	pg_data_t *pgdat = NODE_DATA(nid);
	const unsigned int priority_less = 5;
	struct sched_param param = {
		.sched_priority = MAX_PRIO - priority_less,
	};
	int ret = 0;

	if (pgdat->kshrinkd || is_node_part0(nid))
		return ret;

	atomic_set(&pgdat->kshrinkd_wait_flag, 0);
	pgdat->kshrinkd = kthread_create(kshrinkd, pgdat, "kshrinkd%d", nid);
	if (IS_ERR(pgdat->kshrinkd)) {
		pr_err("kshrinkd tsk create failed on node %d\n", nid);
		ret = PTR_ERR(pgdat->kshrinkd);
		pgdat->kshrinkd = NULL;
		return ret;
	}

	sched_setscheduler_nocheck(pgdat->kshrinkd, SCHED_NORMAL, &param);
	set_user_nice(pgdat->kshrinkd, PRIO_TO_NICE(param.sched_priority));
	wake_up_process(pgdat->kshrinkd);

	return ret;
}

/*
 * Called by memory hotplug when all memory in a node is offlined.  Caller must
 * hold mem_hotplug_begin/end().
 */
void kshrinkd_stop(int nid)
{
	struct task_struct *kshrinkd = NODE_DATA(nid)->kshrinkd;

	if (kshrinkd) {
		kthread_stop(kshrinkd);
		NODE_DATA(nid)->kshrinkd = NULL;
	}
}

/*
 * set all cpus bindings.
 */
static int kshrinkd_cpu_online(unsigned int cpu)
{
	int nid;

	for_each_node_state(nid, N_MEMORY) {
		pg_data_t *pgdat = NODE_DATA(nid);
		const struct cpumask *mask;

		if (is_node_part0(nid))
			continue;
		mask = cpu_online_mask;
		if (cpumask_any_and(cpu_online_mask, mask) < nr_cpu_ids)
			/* One of our CPUs online: restore mask */
			set_cpus_allowed_ptr(pgdat->kshrinkd, mask);
	}
	return 0;
}

static void ama_policy_per_init(void)
{
	int i;

	for (i = 0; i < AMA_TYPE_SUM; i++) {
		struct mempolicy *pol = &ama_default_policy[i];

		atomic_set(&pol->refcnt, 1);
		pol->mode = MPOL_PREFERRED;
		/* sometimes need to be MPOL_F_LOCAL */
		pol->flags = MPOL_F_MOF | MPOL_F_MORON;

		nodes_setall(pol->w.cpuset_mems_allowed);

		if (i >= AMA_MEDIA)
			pol->v.preferred_node = LOCAL_NODE1;
		else
			pol->v.preferred_node = LOCAL_NODE0;
	}
}

static int ama_desc_init(void)
{
	ama_migrate_info = kzalloc(sizeof(struct ama_migrate_desc),
							GFP_KERNEL);
	if (!ama_migrate_info)
		return -ENOMEM;

	ama_migrate_info->ama_scan_seq		= 0;
	ama_migrate_info->ama_scan_period	= sysctl_ama_migrate_scan_delay;
	ama_migrate_info->ama_scan_size		= sysctl_ama_migrate_scan_size;

	INIT_WORK(&ama_migrate_info->ama_work, task_ama_work);
	atomic_set(&ama_migrate_info->ama_work_wait, 0);

	return 0;
}

static int __init ama_init(void)
{
	int nid;
	int ret;

	ama_policy_per_init();

	ret = ama_desc_init();
	if (ret) {
		pr_err("%s: ama_desc init failed\n", __func__);
		return ret;
	}

	ret = cpuhp_setup_state_nocalls(CPUHP_AP_ONLINE_DYN,
					"mm/kshrinkd:online",
					kshrinkd_cpu_online,
					NULL);
	if (ret < 0) {
		kfree(ama_migrate_info);
		ama_migrate_info = NULL;
		pr_err("%s: register hotplug callbacks failed.\n",
			__func__);
		return ret;
	}

	for_each_node_state(nid, N_MEMORY)
		kshrinkd_run(nid);

	return 0;
}
module_init(ama_init)

MODULE_DESCRIPTION("Memory AMA Evo-policy");
MODULE_LICENSE("GPL");
