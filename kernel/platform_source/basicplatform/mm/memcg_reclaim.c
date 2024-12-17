/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2020. All rights reserved.
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
#include <linux/mm.h>
#include <linux/sched/mm.h>
#include <linux/module.h>
#include <linux/gfp.h>
#include <linux/kernel_stat.h>
#include <linux/swap.h>
#include <linux/pagemap.h>
#include <linux/init.h>
#include <linux/highmem.h>
#include <linux/vmpressure.h>
#include <linux/vmstat.h>
#include <linux/file.h>
#include <linux/writeback.h>
#include <linux/blkdev.h>
#include <linux/buffer_head.h>
#include <linux/mm_inline.h>
#include <linux/backing-dev.h>
#include <linux/rmap.h>
#include <linux/topology.h>
#include <linux/cpu.h>
#include <linux/cpuset.h>
#include <linux/compaction.h>
#include <linux/notifier.h>
#include <linux/rwsem.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/freezer.h>
#include <linux/memcontrol.h>
#include <linux/delayacct.h>
#include <linux/sysctl.h>
#include <linux/oom.h>
#include <linux/prefetch.h>
#include <linux/printk.h>
#include <linux/dax.h>
#include <linux/psi.h>
#include <linux/sched/loadavg.h>
#include <linux/sched.h>

#include <asm/tlbflush.h>
#include <asm/div64.h>
#include <linux/atomic.h>
#include <linux/swapops.h>
#include <linux/balloon_compaction.h>
#include <trace/events/vmscan.h>
#include <uapi/linux/sched/types.h>

#ifdef CONFIG_MEMCG_PROTECT_LRU
#include <linux/protect_lru.h>
#endif
#include <linux/hp/hyperhold_inf.h>
#ifdef CONFIG_HW_RECLAIM_ACCT
#include <chipset_common/reclaim_acct/reclaim_acct.h>
#endif

#include <linux/hp/memcg_policy.h>
#include <linux/mm/parallel_swapd.h>
#ifdef CONFIG_MM_AMA
#include <platform_include/basicplatform/linux/mem_ama.h>
#endif

#ifdef CONFIG_PARA_SWAPD
#include <platform_include/cee/linux/core_sched_extend.h>
#endif

#include <internal.h>
#include "memcg_policy_internal.h"
#include <linux/numa.h>

#ifndef CONFIG_KSWAPD_DEBUG
/* anon:file cost pre refault cost ratio. use static to avoid iaware change */
#ifdef CONFIG_PARA_SWAPD
#define STATIC_REFAULT_SWAPPINESS 160
#else
#define STATIC_REFAULT_SWAPPINESS 170
#endif
/*
 * memcg anon refault threshold ratio, with swappiness,
 * means anon cost: file cost=4:1
 */
#define STATIC_REFAULT_MEMCG_RATIO 80
#else
#ifdef CONFIG_PARA_SWAPD
int hp_refault_swappiness = 160;
#else
int hp_refault_swappiness = 170;
#endif
int hp_refault_memcg_ratio = 80;
#define STATIC_REFAULT_SWAPPINESS hp_refault_swappiness
#define STATIC_REFAULT_MEMCG_RATIO hp_refault_memcg_ratio
#endif

int third_order_swappiness_ratio = 60;

#define SHRINK_SYS_ANON_MIN_SCORE 300
#define SHRINK_SYS_ANON_MAX_SCORE 400

#ifdef CONFIG_PARA_SWAPD
static pid_t parak_kswapd_pid = -1;
#endif

static int get_hp_swappiness(void)
{
	return hyperhold_enable() ? STATIC_REFAULT_SWAPPINESS : vm_swappiness;
}

static inline struct lruvec *__mem_cgroup_lruvec(struct mem_cgroup *memcg,
					       struct pglist_data *pgdat)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
	return mem_cgroup_lruvec(memcg, pgdat);
#else
	return mem_cgroup_lruvec(pgdat, memcg);
#endif
}

static inline unsigned long __totalram_pages(void)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
	return totalram_pages();
#else
	return totalram_pages;
#endif
}

static inline bool __inactive_is_low(struct lruvec *lruvec, enum lru_list lru,
	bool file, struct scan_control *sc, bool trace)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
	return inactive_is_low(lruvec, lru);
#else
	return inactive_list_is_low(lruvec, file, sc, trace);
#endif
}

static unsigned long __shrink_page_list(struct list_head *page_list,
					struct pglist_data *pgdat,
					struct scan_control *sc,
					struct reclaim_stat *stat,
					bool force_reclaim)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
	return shrink_page_list(page_list, pgdat, sc, stat, force_reclaim);
#else
	return shrink_page_list(page_list, pgdat, sc, TTU_IGNORE_ACCESS, stat, force_reclaim);
#endif
}

static void zswapd_shrink_active_list(unsigned long nr_to_scan,
	struct lruvec *lruvec, struct scan_control *sc, enum lru_list lru)
{
	unsigned long nr_scanned;
	unsigned long nr_taken;
	LIST_HEAD(l_hold);
	LIST_HEAD(l_inactive);
	struct page *page = NULL;
	unsigned int nr_deactivate;
	isolate_mode_t isolate_mode = 0;
	struct pglist_data *pgdat = lruvec_pgdat(lruvec);

	lru_add_drain();

	spin_lock_irq(&pgdat->lru_lock);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
	(void)isolate_mode;
	nr_taken = isolate_lru_pages(nr_to_scan, lruvec, &l_hold,
				     &nr_scanned, sc, lru);
#else
	nr_taken = isolate_lru_pages(nr_to_scan, lruvec, &l_hold,
				     &nr_scanned, sc, isolate_mode, lru);
#endif
	__mod_node_page_state(pgdat, NR_ISOLATED_ANON, nr_taken);

	__count_vm_events(PGREFILL, nr_scanned);
	count_memcg_events(lruvec_memcg(lruvec), PGREFILL, nr_scanned);
	spin_unlock_irq(&pgdat->lru_lock);

	while (!list_empty(&l_hold)) {
		cond_resched();
		page = lru_to_page(&l_hold);
		list_del(&page->lru);

		if (unlikely(!page_evictable(page))) {
			putback_lru_page(page);
			continue;
		}

		ClearPageActive(page); /* we are de-activating */
		SetPageWorkingset(page);
		list_add(&page->lru, &l_inactive);
	}
	spin_lock_irq(&pgdat->lru_lock);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
	nr_deactivate = move_pages_to_lru(lruvec, &l_inactive);
	list_splice(&l_inactive, &l_hold);
#else
	nr_deactivate = move_active_pages_to_lru(lruvec, &l_inactive,
			&l_hold, lru - LRU_ACTIVE);
#endif
	/*lint -e501*/
	__mod_node_page_state(pgdat, NR_ISOLATED_ANON, -nr_taken);
	/*lint +e501*/
	spin_unlock_irq(&pgdat->lru_lock);
	mem_cgroup_uncharge_list(&l_hold);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
	free_unref_page_list(&l_hold);
#else
	free_hot_cold_page_list(&l_hold, true);
#endif
	trace_mm_vmscan_lru_zswapd_shrink_active(pgdat->node_id,
			nr_taken, nr_deactivate, sc->priority);
}

static unsigned long move_pages_to_page_list(struct lruvec *lruvec,
		enum lru_list lru, struct list_head *page_list)
{
	unsigned long nr_isolated = 0;
	struct page *page = NULL;
	struct list_head *src = &lruvec->lists[lru];

	while (!list_empty(src)) {
		page = lru_to_page(src);
		if (PageUnevictable(page))
			continue;

		if (likely(get_page_unless_zero(page))) {
			if (isolate_lru_page(page)) {
				put_page(page);
				continue;
			}
			put_page(page);
		} else {
			continue;
		}

		if (PageUnevictable(page)) {
			putback_lru_page(page);
			continue;
		}

		/*
		 * MADV_FREE clears pte dirty bit and then marks the page
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

		list_add(&page->lru, page_list);
		nr_isolated++;
	}
	return nr_isolated;
}

#ifdef CONFIG_RAMTURBO
bool is_swap_full(void)
{
	const unsigned int percent_constant = 100;
	long free_swap_pages;

	if (total_swap_pages == 0)
		return true;

	free_swap_pages = get_nr_swap_pages();
	return ((total_swap_pages - free_swap_pages) * percent_constant >=
		total_swap_pages * get_swap_full_ratio());
}

unsigned long all_active_reclaim_by_memcg(struct pglist_data *pgdat,
		struct mem_cgroup *memcg, unsigned int active_ratio)
{
	LIST_HEAD(page_list);
	struct lruvec *lruvec = __mem_cgroup_lruvec(memcg, pgdat);
	const unsigned int percent_constant = 100;
	unsigned long nr_reclaimed;
	unsigned long nr_to_scan;
	unsigned long nr_to_shrink_active_size, nr_shrink_active;
	struct page *page = NULL;
	unsigned long start_active, start_inactive, end_active, end_inactive;
	struct reclaim_stat stat = {};
	struct scan_control sc = {
		.gfp_mask = GFP_KERNEL,
		.may_writepage = 1,
		.may_unmap = 1,
		.may_swap = 1,
	};

	start_active = jiffies;
	nr_to_shrink_active_size = lruvec_lru_size(lruvec,
			LRU_ACTIVE_ANON, MAX_NR_ZONES) * active_ratio / percent_constant;
	nr_shrink_active = nr_to_shrink_active_size;
	while (nr_to_shrink_active_size > 0) {
		nr_to_scan = min(nr_to_shrink_active_size, SWAP_CLUSTER_MAX);
		nr_to_shrink_active_size -= nr_to_scan;
		zswapd_shrink_active_list(nr_to_scan,
			lruvec, &sc, LRU_ACTIVE_ANON);
	}
	end_active = jiffies;
	pr_debug("RAMTURBO: %s active_lru shrinked %lupages(%luBytes) takes %lums",
		__func__, nr_shrink_active,
		nr_shrink_active * PAGE_SIZE,
		jiffies_to_msecs(end_active - start_active));

	start_inactive = jiffies;
	move_pages_to_page_list(lruvec,
			LRU_INACTIVE_ANON, &page_list);
	nr_reclaimed = __shrink_page_list(&page_list, NULL, &sc, &stat, true);

	while (!list_empty(&page_list)) {
		page = lru_to_page(&page_list);
		list_del(&page->lru);
		putback_lru_page(page);
	}

	end_inactive = jiffies;

	pr_debug("RAMTURBO: %s inactive_lru shrinked %lupages(%luBytes) takes %lums",
		__func__, nr_reclaimed, nr_reclaimed * PAGE_SIZE,
		jiffies_to_msecs(end_inactive - start_inactive));

	return nr_reclaimed;
}
#endif

unsigned long reclaim_all_anon_memcg(struct pglist_data *pgdat,
		struct mem_cgroup *memcg)
{
	LIST_HEAD(page_list);
	struct lruvec *lruvec = __mem_cgroup_lruvec(memcg, pgdat);
	unsigned long nr_reclaimed;
	struct page *page = NULL;
	struct reclaim_stat stat = {};
	struct scan_control sc = {
		.gfp_mask = GFP_KERNEL,
		.may_writepage = 1,
		.may_unmap = 1,
		.may_swap = 1,
	};

	move_pages_to_page_list(lruvec,
			LRU_INACTIVE_ANON, &page_list);
	nr_reclaimed = __shrink_page_list(&page_list, NULL, &sc, &stat, true);

	while (!list_empty(&page_list)) {
		page = lru_to_page(&page_list);
		list_del(&page->lru);
		putback_lru_page(page);
	}

	return nr_reclaimed;
}

u64 get_memcg_file_pages(struct mem_cgroup *memcg)
{
	int nid;
	pg_data_t *pgdat = NULL;
	struct lruvec *lruvec = NULL;
	u64 memcg_file = 0;

	for_each_online_node(nid) {
		pgdat = NODE_DATA(nid);
		lruvec = __mem_cgroup_lruvec(memcg, pgdat);
		memcg_file += lruvec_lru_size(lruvec, LRU_ACTIVE_FILE, MAX_NR_ZONES);
		memcg_file += lruvec_lru_size(lruvec, LRU_INACTIVE_FILE, MAX_NR_ZONES);
	}

	return memcg_file;
}

u64 get_memcg_anon_pages(struct mem_cgroup *memcg)
{
	int nid;
	pg_data_t *pgdat = NULL;
	struct lruvec *lruvec = NULL;
	u64 memcg_anon = 0;

	for_each_online_node(nid) {
		pgdat = NODE_DATA(nid);
		lruvec = __mem_cgroup_lruvec(memcg, pgdat);
		memcg_anon += lruvec_lru_size(lruvec, LRU_ACTIVE_ANON, MAX_NR_ZONES);
		memcg_anon += lruvec_lru_size(lruvec, LRU_INACTIVE_ANON, MAX_NR_ZONES);
	}

	return memcg_anon;
}

u64 get_sys_anon_size_memcgs(void)
{
	u64 score;
	u64 total_system_anon = 0;
	struct mem_cgroup *memcg = NULL;

	while ((memcg = get_next_memcg(memcg))) {
		score = (u64)atomic64_read(&memcg->memcg_reclaimed.app_score);
		if (score < SHRINK_SYS_ANON_MIN_SCORE || score > SHRINK_SYS_ANON_MAX_SCORE)
			continue;

		total_system_anon += get_memcg_anon_pages(memcg);
	}
	total_system_anon = total_system_anon * PAGE_SIZE / SZ_1M;

	return total_system_anon;
}

void force_shrink_sys_anon_memcgs(void)
{
	int nid;
	u64 score;
	u64 memcg_anon = 0;
	u64 total_reclaimed = 0;
	u64 shrink_root_thre_pg = get_shrink_root_thre_sz() * SZ_1M / PAGE_SIZE;
	u64 shrink_sys_tgt_pg = get_shrink_sys_tgt_sz() * SZ_1M / PAGE_SIZE;
	u64 shrink_sys_skip_pg = get_shrink_sys_skip_sz() * SZ_1M / PAGE_SIZE;
	unsigned int emergency_swap_ratio = get_emergency_swap_ratio();
	struct mem_cgroup *memcg = NULL;
	pg_data_t *pgdat = NULL;

	if (!shrink_sys_anon_enabled())
		return;

	if (get_sys_anon_size_memcgs() < get_shrink_sys_thre_sz())
		return;

	pr_info("force_shrink_sys, total size %llu MB", get_sys_anon_size_memcgs());
	while ((memcg = get_next_memcg(memcg))) {
		score = (u64)atomic64_read(&memcg->memcg_reclaimed.app_score);
		if (score >= SHRINK_SYS_ANON_MIN_SCORE &&
			(unsigned int)atomic_read(&memcg->memcg_reclaimed.ub_zram2ufs_ratio) < emergency_swap_ratio)
			atomic_set(&memcg->memcg_reclaimed.ub_zram2ufs_ratio, emergency_swap_ratio);
		if (score < SHRINK_SYS_ANON_MIN_SCORE || score > SHRINK_SYS_ANON_MAX_SCORE)
			continue;

		memcg_anon = get_memcg_anon_pages(memcg);
		if (memcg_anon < shrink_sys_skip_pg)
			continue;
		if (memcg == root_mem_cgroup && memcg_anon < shrink_root_thre_pg)
			continue;

		for_each_online_node(nid) {
			pgdat = NODE_DATA(nid);
			total_reclaimed += reclaim_all_anon_memcg(pgdat, memcg);
		}
		pr_info("%s, reclaim memcg %s, origin size %llu MB, total_reclaimed %llu MB",
			__func__, memcg->name,
			memcg_anon * PAGE_SIZE / SZ_1M,
			total_reclaimed * PAGE_SIZE / SZ_1M);

		if (total_reclaimed >= shrink_sys_tgt_pg)
			break;
	}
	count_vm_event(SHRINK_SYS_ANON);
	count_vm_events(SHRINK_SYS_ANON_PAGES, total_reclaimed);
}

unsigned long reclaim_all_anon_memcg_prelaunch(struct pglist_data *pgdat,
		struct mem_cgroup *memcg)
{
	LIST_HEAD(page_list);
	struct lruvec *lruvec = __mem_cgroup_lruvec(memcg, pgdat);
	unsigned long nr_reclaimed;
	unsigned long nr_to_scan;
	unsigned long lru_active_anon_size;
	struct page *page = NULL;
	struct reclaim_stat stat = {};
	struct scan_control sc = {
		.gfp_mask = GFP_KERNEL,
		.may_writepage = 1,
		.may_unmap = 1,
		.may_swap = 1,
	};

	lru_active_anon_size = lruvec_lru_size(lruvec, LRU_ACTIVE_ANON, MAX_NR_ZONES);
	while (lru_active_anon_size) {
		nr_to_scan = min(lru_active_anon_size, SWAP_CLUSTER_MAX);
		lru_active_anon_size -= nr_to_scan;
		zswapd_shrink_active_list(nr_to_scan, lruvec, &sc, LRU_ACTIVE_ANON);
	}
	move_pages_to_page_list(lruvec,
			LRU_INACTIVE_ANON, &page_list);
	nr_reclaimed = __shrink_page_list(&page_list, NULL, &sc, &stat, true);
	while (!list_empty(&page_list)) {
		page = lru_to_page(&page_list);
		list_del(&page->lru);
		putback_lru_page(page);
	}

	return nr_reclaimed;
}

#ifdef CONFIG_HYPERHOLD_FILE_LRU
static inline bool is_swap_not_allowed(struct scan_control *sc, int swappiness)
{
	return !sc->may_swap || !swappiness || !get_nr_swap_pages();
}

#ifdef CONFIG_DIRECT_SWAPPINESS
static inline int get_direct_swappiness(void)
{
#if defined(CONFIG_HP_CORE) && defined(CONFIG_HYPERHOLD_ZSWAPD)
	return hyperhold_enable() ? 0 : direct_vm_swappiness;
#else
	return direct_vm_swappiness;
#endif
}
#endif

static bool is_kswapd_high_load(void)
{
	bool fit_cpu = false;
	int policy;
	unsigned int cpu_id;
	unsigned long current_task_util;

	policy = get_kswapd_track_load_policy();
	cpu_id = smp_processor_id();
	switch (policy) {
	case TRACK_BIG_CPU:
		fit_cpu = is_max_cap_orig_cpu(cpu_id);
		break;
	case TRACK_FAST_CPU:
		fit_cpu = (test_fast_cpu(cpu_id) == 1);
		break;
	case TRACK_ALL_CPU:
		fit_cpu = true;
		break;
	default:
		return false;
	}

	current_task_util = get_task_util(current);
	trace_mm_kswapd_current_task_util(current_task_util, fit_cpu, cpu_id);
	return fit_cpu && current_task_util > get_kswapd_high_load_threshold();
}

static void get_scan_count_hyperhold(struct pglist_data *pgdat,
		struct scan_control *sc, unsigned long *nr,
		unsigned long *lru_pages)
{
	int swappiness = get_hp_swappiness();
	struct lruvec *lruvec = node_lruvec(pgdat);
	u64 fraction[2];
	u64 denominator;
	enum scan_balance scan_balance;
	unsigned long anon_cost, file_cost, total_cost;
	unsigned long ap, fp;
	enum lru_list lru;
	unsigned long pgdatfile;

#ifdef CONFIG_DIRECT_SWAPPINESS
	/* Set kswapd's swappiness to 0 if direct reclaim is happening. */
	if (!current_is_kswapd()) {
		swappiness = get_direct_swappiness();
	} else {
		inc_kswapd_called_cnt();
		if (is_in_direct_reclaim()) {
			inc_kswapd_in_dr_cnt();
			swappiness = 0;
		} else if (is_kswapd_high_load()) {
			inc_kswapd_high_load_cnt();
			swappiness = get_kswapd_high_load_swappiness();
		}
	}

	pgdatfile = node_page_state(pgdat, NR_ACTIVE_FILE) +
		node_page_state(pgdat, NR_INACTIVE_FILE);
	if (pgdatfile <= 20 * SZ_1M / PAGE_SIZE)
		swappiness = 0;
#endif

	/*
	 * There are many 3-order allocations which trigger memory reclaim,
	 * such as network skb_alloc and kmem cache alloc and so on. In most
	 * case, freepages are relatively high but only lack of 3-order pages,
	 * swap is not really needed. So just lower it to make it quick and
	 * save some energy.
	 */
	if (!hyperhold_enable() && current_is_kswapd() && sc->origin_order == 3)
		swappiness = swappiness * third_order_swappiness_ratio / 100;

	/* If we have no swap space, do not bother scanning anon pages. */
	if (is_swap_not_allowed(sc, swappiness)) {
		scan_balance = SCAN_FILE;
		goto out;
	}

	/*
	 * Do not apply any pressure balancing cleverness when the
	 * system is close to OOM, scan both anon and file equally
	 * (unless the swappiness setting disagrees with swapping).
	 */
	if (!sc->priority && swappiness) {
		scan_balance = SCAN_EQUAL;
		goto out;
	}

	if (sc->file_is_tiny) {
		scan_balance = SCAN_ANON;
		goto out;
	}

	if (sc->cache_trim_mode) {
		scan_balance = SCAN_FILE;
		goto out;
	}

#ifdef CONFIG_PARA_SWAPD
	if (hyperhold_enable() && get_parak_enable() &&
		(current->pid == parak_kswapd_pid) &&
		!get_high_avail_buffers_value()) {
		scan_balance = SCAN_FILE;
		goto out;
	}
#endif

	scan_balance = SCAN_FRACT;

	/*
	 * Calculate the pressure balance between anon and file pages.
	 *
	 * The amount of pressure we put on each LRU is inversely
	 * proportional to the cost of reclaiming each list, as
	 * determined by the share of pages that are refaulting, times
	 * the relative IO cost of bringing back a swapped out
	 * anonymous page vs reloading a filesystem page (swappiness).
	 *
	 * Although we limit that influence to ensure no list gets
	 * left behind completely: at least a third of the pressure is
	 * applied, before swappiness.
	 *
	 * With swappiness at 100, anon and file have equal IO cost.
	 */
	total_cost = sc->anon_cost + sc->file_cost;
	anon_cost = total_cost + sc->anon_cost;
	file_cost = total_cost + sc->file_cost;
	total_cost = anon_cost + file_cost;

	ap = swappiness * (total_cost + 1);
	ap /= anon_cost + 1;

	fp = (200 - swappiness) * (total_cost + 1);
	fp /= file_cost + 1;

	fraction[0] = ap;
	fraction[1] = fp;
	denominator = ap + fp;
out:
	*lru_pages = 0;
	for_each_evictable_lru(lru) {
		int file; /*lint !e578*/
		unsigned long size;
		unsigned long scan;

		file = is_file_lru(lru);
		size = lruvec_lru_size(lruvec, lru, sc->reclaim_idx);
		scan = size >> (unsigned int)sc->priority;
		/*
		 * If the cgroup's already been deleted, make sure to
		 * scrape out the remaining cache.
		 */
		switch (scan_balance) {
		case SCAN_EQUAL:
			/* Scan lists relative to size */
			break;
		case SCAN_FRACT:
			/*
			 * Scan types proportional to swappiness and
			 * their relative recent reclaim efficiency.
			 * Make sure we don't miss the last page
			 * because of a round-off error.
			 */
			scan = DIV64_U64_ROUND_UP(scan * fraction[file],
					denominator); /*lint !e644*/
			break;
		case SCAN_FILE:
		case SCAN_ANON:
			/* Scan one type exclusively */
			if ((scan_balance == SCAN_FILE) != file) {
				size = 0;
				scan = 0;
			}
			break;
		default:
			break;
		}

		*lru_pages += size;
		nr[lru] = scan;
	}
}

#define ISOLATE_LIMIT_CNT 5
void shrink_anon_memcg(struct pglist_data *pgdat,
		struct mem_cgroup *memcg, struct scan_control *sc,
		unsigned long *nr)
{
	struct lruvec *lruvec = __mem_cgroup_lruvec(memcg, pgdat);
	unsigned long nr_to_scan;
	enum lru_list lru;
	unsigned long nr_reclaimed = 0;
	struct blk_plug plug;

	blk_start_plug(&plug);

	while (nr[LRU_INACTIVE_ANON] || nr[LRU_ACTIVE_ANON]) {
		for (lru = 0; lru <= LRU_ACTIVE_ANON; lru++) {
			if (nr[lru]) {
				nr_to_scan = min(nr[lru], SWAP_CLUSTER_MAX);
				nr[lru] -= nr_to_scan;
				nr_reclaimed +=
					shrink_list(lru, nr_to_scan,
							lruvec, sc);
			}
		}
		if (sc->nr_reclaimed >= sc->nr_to_reclaim ||
				(sc->isolate_count > ISOLATE_LIMIT_CNT &&
				sc->invoker == DIRECT_RECLAIM))
			break;
	}
	blk_finish_plug(&plug);
	sc->nr_reclaimed += nr_reclaimed;
	sc->nr_reclaimed_anon += nr_reclaimed;
}

#ifdef CONFIG_PARA_SWAPD
static void shrink_anon_legacy(struct pglist_data *pgdat,
		struct scan_control *sc, unsigned long *nr)
#else
static void shrink_anon(struct pglist_data *pgdat,
		struct scan_control *sc, unsigned long *nr)
#endif
{
	unsigned long reclaimed;
	unsigned long scanned;
	struct mem_cgroup *memcg = NULL;
	unsigned long nr_memcg[NR_LRU_LISTS];
	unsigned long nr_node_active = lruvec_lru_size(
			node_lruvec(pgdat), LRU_ACTIVE_ANON, MAX_NR_ZONES);
	unsigned long nr_node_inactive = lruvec_lru_size(
			node_lruvec(pgdat), LRU_INACTIVE_ANON, MAX_NR_ZONES);

	if (nr[LRU_ACTIVE_ANON] == 0 && nr[LRU_INACTIVE_ANON] == 0)
		return;

	trace_shrinklists_start(sc->nr_reclaimed);

	while ((memcg = get_next_memcg(memcg))) {
		struct lruvec *lruvec = __mem_cgroup_lruvec(memcg, pgdat);

		reclaimed = sc->nr_reclaimed;
		scanned = sc->nr_scanned;

		nr_memcg[LRU_ACTIVE_ANON] = nr[LRU_ACTIVE_ANON] *
			lruvec_lru_size(lruvec, LRU_ACTIVE_ANON,
					MAX_NR_ZONES) / (nr_node_active + 1);
		nr_memcg[LRU_INACTIVE_ANON] = nr[LRU_INACTIVE_ANON] *
			lruvec_lru_size(lruvec, LRU_INACTIVE_ANON,
					MAX_NR_ZONES) / (nr_node_inactive + 1);
		nr_memcg[LRU_ACTIVE_FILE] = 0;
		nr_memcg[LRU_INACTIVE_FILE] = 0;

		shrink_anon_memcg(pgdat, memcg, sc, nr_memcg);

		vmpressure(sc->gfp_mask, memcg, false,
				sc->nr_scanned - scanned,
				sc->nr_reclaimed - reclaimed);

		if (sc->nr_reclaimed >= sc->nr_to_reclaim ||
			(sc->isolate_count > ISOLATE_LIMIT_CNT &&
			sc->invoker == DIRECT_RECLAIM)) {
			get_next_memcg_break(memcg);
			break;
		}
	}

	trace_shrinklists_end(sc->nr_reclaimed);
}

#ifdef CONFIG_PARA_SWAPD
#define PARAK_TASK_MAX 2

static LIST_HEAD(parak_memcg_list);
static DEFINE_SEMAPHORE(parak_sem);
wait_queue_head_t parak_kswapd_wait;
atomic_t parak_kswapd_wait_flag;
struct mm_parad_task parak_tasks[PARAK_TASK_MAX];
unsigned long parak_task_mask = 0;

void parak_set_kswapd_pid(pid_t pid)
{
	parak_kswapd_pid = pid;
}

static void parak_shrink_memcg(struct mm_parad_entry *entry)
{
	struct mem_cgroup *memcg  = entry->memcg;
	struct pglist_data *pgdat = entry->pgdat;
	struct scan_control sc;
	unsigned long nr_scanned, nr_reclaimed, nr_reclaimed_anon,
		nr_scanned_file, nr_scanned_anon,
		start = parad_get_counter();
	unsigned int isolate_count;

	spin_lock_irq(&pgdat->lru_lock);
	sc = *entry->sc;
	spin_unlock_irq(&pgdat->lru_lock);

	if (sc.nr_reclaimed >= sc.nr_to_reclaim)
		goto out;

	nr_scanned        = sc.nr_scanned;
	nr_scanned_file   = sc.nr_scanned_file;
	nr_scanned_anon   = sc.nr_scanned_anon;
	nr_reclaimed      = sc.nr_reclaimed;
	nr_reclaimed_anon = sc.nr_reclaimed_anon;
	isolate_count     = sc.isolate_count;

	shrink_anon_memcg(pgdat, memcg, &sc, entry->nr);

	vmpressure(sc.gfp_mask, memcg, false,
			sc.nr_scanned - nr_scanned,
			sc.nr_reclaimed - nr_reclaimed);

	/* give back the real reclaimed info of per memcg to k/zswapd sc */
	spin_lock_irq(&pgdat->lru_lock);
	entry->sc->nr_scanned        += sc.nr_scanned - nr_scanned;
	entry->sc->nr_reclaimed      += sc.nr_reclaimed - nr_reclaimed;
	entry->sc->nr_reclaimed_anon += sc.nr_reclaimed_anon - nr_reclaimed_anon;
	entry->sc->isolate_count     += sc.isolate_count - isolate_count;
	entry->sc->nr_scanned_file   += sc.nr_scanned_file - nr_scanned_file;
	entry->sc->nr_scanned_anon   += sc.nr_scanned_anon - nr_scanned_anon;
	spin_unlock_irq(&pgdat->lru_lock);

out:
	parad_stat_add(PARAD_KSWAPD_WORK, parad_get_counter() - start);
	clear_bit(F_PARAD_RECLAIMING, &entry->flags);
	css_put(&memcg->css);
}

static int parak_thread(void *p)
{
	struct mm_parad_entry *entry = NULL;
	struct mm_parad_task *parak_task = (struct mm_parad_task *)p;

	set_freezable();

	while (!kthread_should_stop()) {
		(void)wait_event_freezable(parak_task->parad_wait,
				atomic_read(&parak_task->parad_wait_flag));
		down(&parak_sem);
		if (!test_bit(parak_task->idx, &parak_task_mask))
			set_bit(parak_task->idx, &parak_task_mask);

		entry = list_first_entry_or_null(
			&parak_memcg_list, struct mm_parad_entry, list);
		if (!entry) {
			atomic_set(&parak_task->parad_wait_flag, 0);
			clear_bit(parak_task->idx, &parak_task_mask);
			if (parak_task_mask) {
				up(&parak_sem);
				continue;
			}
			atomic_set(&parak_kswapd_wait_flag, 1);
			wake_up_interruptible_all(&parak_kswapd_wait);
			up(&parak_sem);
			continue;
		}
		list_del(&entry->list);
		up(&parak_sem);

		parak_shrink_memcg(entry);
	}

	return 0;
}

int parak_task_create(void)
{
	int i;
	struct cpumask mask = CPU_MASK_NONE;
	unsigned int sched_priority = MAX_PRIO - 5;

	for_each_possible_cpu(i) {
		if (topology_physical_package_id(i) == 1)
			cpumask_set_cpu(i, &mask);
	}

	init_waitqueue_head(&parak_kswapd_wait);
	atomic_set(&parak_kswapd_wait_flag, 0);

	for (i = 0; i < PARAK_TASK_MAX; i++) {
		struct mm_parad_task *parak_task = &parak_tasks[i];

		init_waitqueue_head(&parak_task->parad_wait);
		atomic_set(&parak_task->parad_wait_flag, 0);
		parak_task->idx = i;

		parak_task->task = kthread_run(
			parak_thread, parak_task, "para_kswapd%d", i);
		if (IS_ERR(parak_task->task)) {
			pr_err("failed to start para_kswapd%d\n", i);
			return PTR_ERR(parak_task->task);
		}

		set_cpus_allowed_ptr(parak_task->task, &mask);
		set_user_nice(parak_task->task, PRIO_TO_NICE(sched_priority));
	}

	for (i = 1; i < PARAK_TASK_MAX; i++)
		sched_core_share_prefer(parak_tasks[i].task, parak_tasks[0].task);

	return 0;
}

static bool parak_task_ok(void)
{
	unsigned int i;

	for (i = 0; i < PARAK_TASK_MAX; i++) {
		if (IS_ERR(parak_tasks[i].task))
			return false;
	}

	return true;
}

static bool parak_has_sleeper(unsigned int max)
{
	unsigned int i;
	unsigned int cnt = 0;

	while (true) {
		for (i = 0; i < max; i++) {
			if (!wq_has_sleeper(&parak_tasks[i].parad_wait))
				break;
			cnt++;
			if (cnt == max)
				return true;
		}
		set_current_state(TASK_UNINTERRUPTIBLE);
		schedule_timeout(4);
		set_current_state(TASK_RUNNING);
		cnt = 0;
	}
	return true;
}

static void parak_wake_up(unsigned int max)
{
	unsigned int i;

	for (i = 0; i < max; i++) {
		struct mm_parad_task *parak_task = &parak_tasks[i];

		if (wq_has_sleeper(&parak_task->parad_wait)) {
			atomic_set(&parak_task->parad_wait_flag, 1);
			wake_up_interruptible(&parak_task->parad_wait);
		}
	}
}

static void shrink_anon(struct pglist_data *pgdat,
		struct scan_control *sc, unsigned long *nr)
{
	struct mem_cgroup *memcg = NULL;
	struct mm_parad_entry *entry = NULL;
	unsigned long nr_node_active = lruvec_lru_size(
			node_lruvec(pgdat), LRU_ACTIVE_ANON, MAX_NR_ZONES);
	unsigned long nr_node_inactive = lruvec_lru_size(
			node_lruvec(pgdat), LRU_INACTIVE_ANON, MAX_NR_ZONES);
	unsigned long nr_scanned = sc->nr_scanned,
				nr_reclaimed = sc->nr_reclaimed,
				start = parad_get_counter();

	if (!parak_task_ok() || get_parak_enable() == 0) {
		shrink_anon_legacy(pgdat, sc, nr);
		goto out;
	}

	if (nr[LRU_ACTIVE_ANON] == 0 && nr[LRU_INACTIVE_ANON] == 0)
		return;

	trace_shrinklists_start(sc->nr_reclaimed);

	while ((memcg = get_next_memcg(memcg))) {
		struct lruvec *lruvec = __mem_cgroup_lruvec(memcg, pgdat);

		if (sc->nr_reclaimed >= sc->nr_to_reclaim) {
			get_next_memcg_break(memcg);
			break;
		}

		if (test_and_set_bit(F_PARAD_RECLAIMING, &memcg->zpara.flags))
			continue;

		entry = &memcg->zpara;
		entry->nr[LRU_ACTIVE_ANON] = nr[LRU_ACTIVE_ANON] *
			lruvec_lru_size(lruvec, LRU_ACTIVE_ANON,
					MAX_NR_ZONES) / (nr_node_active + 1);
		entry->nr[LRU_INACTIVE_ANON] = nr[LRU_INACTIVE_ANON] *
			lruvec_lru_size(lruvec, LRU_INACTIVE_ANON,
					MAX_NR_ZONES) / (nr_node_inactive + 1);
		entry->nr[LRU_ACTIVE_FILE] = 0;
		entry->nr[LRU_INACTIVE_FILE] = 0;

		if (!entry->nr[LRU_ACTIVE_ANON] && !entry->nr[LRU_INACTIVE_ANON]) {
			clear_bit(F_PARAD_RECLAIMING, &entry->flags);
			continue;
		}

		entry->memcg = memcg;
		entry->pgdat = pgdat;
		entry->sc    = sc;
		css_get(&memcg->css);

		down(&parak_sem);
		list_add_tail(&entry->list, &parak_memcg_list);
		up(&parak_sem);
	}

	atomic_set(&parak_kswapd_wait_flag, 0);
	if (parak_has_sleeper(PARAK_TASK_MAX))
		parak_wake_up(PARAK_TASK_MAX);

	(void)wait_event_freezable(parak_kswapd_wait,
			atomic_read(&parak_kswapd_wait_flag));

	trace_shrinklists_end(sc->nr_reclaimed);

out:
	parad_stat_add(PARAD_KSWAPD, parad_get_counter() - start);
	parad_stat_add(PARAD_KSWAPD_NR_SCANNED, sc->nr_scanned - nr_scanned);
	parad_stat_add(PARAD_KSWAPD_NR_RECLAIMED, sc->nr_reclaimed - nr_reclaimed);
}
#endif

static void shrink_file(struct pglist_data *pgdat,
		struct scan_control *sc, unsigned long *nr)
{
	struct lruvec *lruvec = node_lruvec(pgdat);
	unsigned long nr_to_scan;
	enum lru_list lru;
	unsigned long nr_reclaimed = 0;
	struct blk_plug plug;

	blk_start_plug(&plug);

	trace_shrinklists_start(sc->nr_reclaimed);

	while (nr[LRU_ACTIVE_FILE] || nr[LRU_INACTIVE_FILE]) {
		for (lru = LRU_INACTIVE_FILE; lru <= LRU_ACTIVE_FILE; lru++) {
			if (nr[lru]) {
				nr_to_scan = min(nr[lru], SWAP_CLUSTER_MAX);
				nr[lru] -= nr_to_scan;
				nr_reclaimed +=
					shrink_list(lru,
							nr_to_scan,
							lruvec, sc);
			}
		}
	}
	blk_finish_plug(&plug);
	sc->nr_reclaimed += nr_reclaimed;
	sc->nr_reclaimed_file += nr_reclaimed;

	trace_shrinklists_end(sc->nr_reclaimed);
}

bool shrink_node_hyperhold(pg_data_t *pgdat, struct scan_control *sc)
{
	struct reclaim_state *reclaim_state = current->reclaim_state;
	unsigned long nr_reclaimed, nr_scanned;
	unsigned long refaults, file;
	bool reclaimable = false;
	struct lruvec *target_lruvec = node_lruvec(pgdat);
	unsigned long total_high_wmark = 0;
	unsigned long free, anon;
	int z;

#ifdef CONFIG_MEMCG_PROTECT_LRU
	shrink_prot_memcg_by_overratio();
#endif
	do {
		/* Get scan count for file and anon */
		unsigned long node_lru_pages = 0;
		unsigned long nr[NR_LRU_LISTS] = {0};

		nr_reclaimed = sc->nr_reclaimed;
		nr_scanned = sc->nr_scanned;

		/*
		 * Determine the scan balance between anon and file LRUs.
		 */
		spin_lock_irq(&pgdat->lru_lock);
		sc->anon_cost =
			lruvec_page_state(target_lruvec, WORKINGSET_ANON_COST);
		sc->file_cost =
			lruvec_page_state(target_lruvec, WORKINGSET_FILE_COST);
		spin_unlock_irq(&pgdat->lru_lock);

		/*
		 * Target desirable inactive:active list ratios for the anon
		 * and file LRU lists.
		 */
		if (!sc->force_deactivate) {
			refaults = lruvec_page_state(target_lruvec,
					WORKINGSET_ACTIVATE_ANON);
			if (refaults != target_lruvec->refaults[0] ||
				__inactive_is_low(target_lruvec, LRU_INACTIVE_ANON,
				false, sc, false))
				sc->may_deactivate |= DEACTIVATE_ANON;
			else
				sc->may_deactivate &= ~DEACTIVATE_ANON;

			/*
			 * When refaults are being observed, it means a new
			 * workingset is being established. Deactivate to get
			 * rid of any stale active pages quickly.
			 */
			refaults = lruvec_page_state(target_lruvec,
					WORKINGSET_ACTIVATE_FILE);
			if (refaults != target_lruvec->refaults[1] ||
				__inactive_is_low(target_lruvec, LRU_INACTIVE_FILE,
				true, sc, false))
				sc->may_deactivate |= DEACTIVATE_FILE;
			else
				sc->may_deactivate &= ~DEACTIVATE_FILE;
		} else {
			sc->may_deactivate = DEACTIVATE_ANON | DEACTIVATE_FILE;
		}

		/*
		 * If we have plenty of inactive file pages that aren't
		 * thrashing, try to reclaim those first before touching
		 * anonymous pages.
		 */
		file = lruvec_page_state(target_lruvec, NR_INACTIVE_FILE);
		if ((file >> sc->priority) && (!(sc->may_deactivate & DEACTIVATE_FILE)))
			sc->cache_trim_mode = 1;
		else
			sc->cache_trim_mode = 0;

		free = sum_zone_node_page_state(pgdat->node_id, NR_FREE_PAGES);
		file = node_page_state(pgdat, NR_ACTIVE_FILE) +
			node_page_state(pgdat, NR_INACTIVE_FILE);

		for (z = 0; z < MAX_NR_ZONES; z++) {
			struct zone *zone = &pgdat->node_zones[z];

			if (!managed_zone(zone))
				continue;

#ifdef CONFIG_ZONE_MEDIA
			if (IS_MEIDA_ZONE_IDX(z))
				continue;
#endif

			total_high_wmark += high_wmark_pages(zone);
		}

		/*
		 * Consider anon: if that's low too, this isn't a
		 * runaway file reclaim problem, but rather just
		 * extreme pressure. Reclaim as per usual then.
		 */
		anon = node_page_state(pgdat, NR_INACTIVE_ANON);

		sc->file_is_tiny =
			file + free <= total_high_wmark &&
			!(sc->may_deactivate & DEACTIVATE_ANON) &&
			anon >> sc->priority;

		get_scan_count_hyperhold(pgdat, sc, nr, &node_lru_pages);

#ifdef CONFIG_HW_RECLAIM_ACCT
		reclaimacct_get_nr_to_scan(nr);
#endif

		/* Shrink the Total-File-LRU */
		shrink_file(pgdat, sc, nr);

#ifdef CONFIG_HW_RECLAIM_ACCT
		/* Check whether kswapd is blocked by dr after shrink_file */
		if (current_is_kswapd() && waitqueue_active(&pgdat->pfmemalloc_wait))
			kswapd_change_block_status();
#endif

		/* Shrink Anon by iterating score_list */
#ifdef CONFIG_PARA_SWAPD
		if (sc->invoker == KSWAPD && current->pid == parak_kswapd_pid)
			shrink_anon(pgdat, sc, nr);
		else
			shrink_anon_legacy(pgdat, sc, nr);
#else
		shrink_anon(pgdat, sc, nr);
#endif

#ifdef CONFIG_HW_RECLAIM_ACCT
		/* Check whether kswapd is blocked by dr after shrink_anon */
		if (current_is_kswapd() && waitqueue_active(&pgdat->pfmemalloc_wait))
			kswapd_change_block_status();
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
		shrink_slab(sc->gfp_mask, pgdat->node_id, root_mem_cgroup,
				sc->priority);
#else
		shrink_slab(sc->gfp_mask, pgdat->node_id, NULL,
				sc->nr_scanned - nr_scanned,
				node_lru_pages);
#endif

		if (reclaim_state) {
			sc->nr_reclaimed += reclaim_state->reclaimed_slab;
			reclaim_state->reclaimed_slab = 0;
		}

		/* Record the subtree's reclaim efficiency */
		vmpressure(sc->gfp_mask, sc->target_mem_cgroup, true,
			   sc->nr_scanned - nr_scanned,
			   sc->nr_reclaimed - nr_reclaimed);

		if (sc->nr_reclaimed - nr_reclaimed)
			reclaimable = true;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
	} while (should_continue_reclaim(pgdat, sc->nr_reclaimed - nr_reclaimed, sc));
#else
	} while (should_continue_reclaim(pgdat, sc->nr_reclaimed - nr_reclaimed,
					 sc->nr_scanned - nr_scanned, sc));
#endif
	/*
	 * Kswapd gives up on balancing particular nodes after too
	 * many failures to reclaim anything from them and goes to
	 * sleep. On reclaim progress, reset the failure counter. A
	 * successful direct reclaim run will revive a dormant kswapd.
	 */
	if (reclaimable)
		pgdat->kswapd_failures = 0;

	return reclaimable;
}
#endif

#ifdef CONFIG_RAMTURBO
/* Kswapd_pid is the pid of kswapd. -1 means kswapd is not initialized or stops. */
static atomic_t kswapd_pid = ATOMIC_INIT(-1);

pid_t get_kswapd_pid(void)
{
	return atomic_read(&kswapd_pid);
}

void set_kswapd_pid(pid_t pid)
{
	atomic_set(&kswapd_pid, pid);
}
#endif

#ifdef CONFIG_HYPERHOLD_ZSWAPD
static pid_t zswapd_pid = -1;

static void update_zswapd_empty_skip_interval(pg_data_t *pgdat)
{
	const unsigned int increase_rate = 2;
	bool empty = pgdat->last_round_is_empty;
	unsigned long long skip_interval =
			pgdat->zswapd_skip_interval;

	count_vm_event(ZSWAPD_EMPTY_ROUND);
	if (empty)
		skip_interval = min(skip_interval *
			increase_rate,
			get_max_skip_interval_value());/*lint !e666*/
	else
		skip_interval =
			get_empty_round_skip_interval_value();

	pgdat->last_round_is_empty = true;
	pgdat->zswapd_skip_interval = skip_interval;
}

static unsigned int calc_sys_cur_avail_buffers(int nid)
{
	const unsigned int percent_constant = 100;
	pg_data_t *pgdat = NODE_DATA(nid);
	unsigned long freemem;
	unsigned long active_file;
	unsigned long inactive_file;
	unsigned long ion;
	unsigned long buffers;
	unsigned long protect = 0;
	unsigned long reserve_protect;
#ifdef CONFIG_ZONE_MEDIA
	unsigned long free_zone_media = calc_media_zone_cmafree_node(nid);
#endif

	freemem = sum_zone_node_page_state(pgdat->node_id, NR_FREE_PAGES);
	active_file = node_page_state(pgdat, NR_ACTIVE_FILE);
	inactive_file = node_page_state(pgdat, NR_INACTIVE_FILE);
	ion = node_page_state(pgdat, NR_NODE_IONCACHES);

#ifdef CONFIG_ZONE_MEDIA
	freemem = freemem > free_zone_media ? (freemem - free_zone_media) : 0;
#endif

	buffers = freemem + inactive_file * get_inactive_file_ratio_value() /
		percent_constant + active_file * get_active_file_ratio_value() /
		percent_constant + ion;

#ifdef CONFIG_DMABUF_HEAPS
	buffers += global_zone_page_state(NR_DMAHEAPCACHE_PAGES) / MAX_NUMNODES;
#endif
#ifdef CONFIG_HP_CORE
	buffers += get_hyperhold_cache_pages() / MAX_NUMNODES;
#endif
#ifdef CONFIG_TASK_PROTECT_LRU
	protect = global_zone_page_state(NR_PROTECT_ACTIVE_FILE) +
		     global_zone_page_state(NR_PROTECT_INACTIVE_FILE);
#elif defined(CONFIG_MEMCG_PROTECT_LRU)
	protect = get_protected_pages();
#endif
	reserve_protect = pgdat->totalreserve_pages + protect / MAX_NUMNODES;
	buffers = buffers > reserve_protect ? buffers - reserve_protect : 0;
	return (buffers * PAGE_SIZE / SZ_1M);  /* pages to mb */
}

static unsigned long get_mempsi(void)
{
	return get_psi(&psi_system, PSI_MEM);
}

static unsigned long get_iopsi(void)
{
	return get_psi(&psi_system, PSI_IO);
}

static unsigned long get_cpupsi(void)
{
	return get_psi(&psi_system, PSI_CPU);
}

void zswapd_status_show(struct seq_file *m)
{
	unsigned long mempsi, iopsi, cpupsi;
	unsigned int buffers = 0;
	int nid;

	mempsi = get_mempsi();
	iopsi = get_iopsi();
	cpupsi = get_cpupsi();

	for_each_node_state(nid, N_MEMORY)
		buffers += calc_sys_cur_avail_buffers(nid);

	seq_printf(m, "buffer_size:%u\n", buffers);
	seq_printf(m, "CPUPSI_avg10:%lu.%02lu\n",
		LOAD_INT(cpupsi), LOAD_FRAC(cpupsi));
	seq_printf(m, "MEMPSI_avg10:%lu.%02lu\n",
		LOAD_INT(mempsi), LOAD_FRAC(mempsi));
	seq_printf(m, "IOPSI_avg10:%lu.%02lu\n",
		LOAD_INT(iopsi), LOAD_FRAC(iopsi));
}

pid_t get_zswapd_pid(void)
{
	return zswapd_pid;
}

#ifdef CONFIG_MM_AMA
#define AMA_SUIT_RATIO	(8)
#define AMA_SUIT_BASE	(10)
#endif

static inline bool _buffer_is_suitable(int nid, u32 avail_buffers)
{
	u32 curr_buffers;

	if (avail_buffers == 0)
		return true;

	curr_buffers = calc_sys_cur_avail_buffers(nid);

#ifdef CONFIG_MM_AMA
	if (is_node_part1(nid))
		avail_buffers = avail_buffers * AMA_SUIT_RATIO / AMA_SUIT_BASE;
#endif

	return curr_buffers >= avail_buffers;
}

static inline bool min_buffer_is_suitable(int nid)
{
	return _buffer_is_suitable(nid, get_min_avail_buffers_value());
}

static inline bool buffer_is_suitable(int nid)
{
	return _buffer_is_suitable(nid, get_avail_buffers_value());
}

static inline bool high_buffer_is_suitable(int nid)
{
	return _buffer_is_suitable(nid, get_high_avail_buffers_value());
}

static int __lruvec_anon_refault_ratio(struct lruvec *lruvec, int swappiness)
{
	unsigned long anon_cost, file_cost, total_cost;
	unsigned long ap, fp;

	swappiness = get_hp_swappiness();

	/* there is no anon cost, can skip */
	anon_cost = lruvec_page_state(lruvec, WORKINGSET_ANON_COST);
	if (!anon_cost)
		return -1;

	file_cost = lruvec_page_state(lruvec, WORKINGSET_FILE_COST);
	total_cost = anon_cost + file_cost;
	anon_cost += total_cost;
	file_cost += total_cost;
	total_cost = anon_cost + file_cost;

	ap = swappiness * (total_cost + 1);
	ap /= anon_cost + 1;
	fp = (200 - swappiness) * (total_cost + 1);
	fp /= file_cost + 1;

	/*
	 * ap and fp means need scan count.
	 * so, if anon is under high memory pressure, the ap will be low.
	 * But our zswapd is need to find wether anon is in high pressure,
	 * use fp be numerator can reflect this situation
	 */

	return (int)((fp * 100) / (ap + fp));
}

static bool get_memcg_anon_refault_status(struct mem_cgroup *memcg,
					  pg_data_t *pgdat)
{
	struct lruvec *lruvec = NULL;
	int ratio;

	if (!memcg)
		return false;

	lruvec = __mem_cgroup_lruvec(memcg, pgdat);
	ratio = __lruvec_anon_refault_ratio(lruvec, memcg->swappiness);
	if (ratio < 0)
		return false;

	return ratio > STATIC_REFAULT_MEMCG_RATIO;
}

/**
 * Charge the node anon refault status.
 * For now, we can use anon cost to show refault effectiveness.
 * So, is anon refault too hard? it's denpend on the anon cost's ratio
 * in total cost -- which contains file and anon's cost.
 * Return True if area anon refault is too hard and skip zswapd shrink.
 */
static bool get_area_anon_refault_status(pg_data_t *pgdat)
{
	struct lruvec *lruvec = node_lruvec(pgdat);
	int ratio;

	ratio = __lruvec_anon_refault_ratio(lruvec, vm_swappiness);
	if (ratio < 0)
		return false;

	return ratio > get_area_anon_refault_threshold_value();
}

#define UNSET_ZRAM_WM_RATIO 0
static int get_zswapd_eswap_policy(void)
{
	if (get_zram_wm_ratio_value() == UNSET_ZRAM_WM_RATIO)
		return CHECK_BUFFER_ONLY;
	else
		return CHECK_BUFFER_ZRAMRAITO_BOTH;
}

#define DEFAULT_ZRAM_WM_RATIO 37
static u64 get_policy_zram_wm_ratio(void)
{
	enum zswapd_eswap_policy policy = get_zswapd_eswap_policy();

	if (policy == CHECK_BUFFER_ONLY)
		return DEFAULT_ZRAM_WM_RATIO;
	else if (policy == CHECK_BUFFER_ZRAMRAITO_BOTH)
		return get_zram_wm_ratio_value();

	return DEFAULT_ZRAM_WM_RATIO;
}

static u64 get_zram_current_watermark(void)
{
	long long diff_buffers = 0;
	const unsigned int percent_constant = 10;
	u64 nr_total;
	u64 zram_wm_ratio = get_policy_zram_wm_ratio();
	int nid;

	if (get_avail_buffers_value() == 0)
		return zram_wm_ratio;

	nr_total = __totalram_pages();
	for_each_node_state(nid, N_MEMORY)
		diff_buffers += (long long)calc_sys_cur_avail_buffers(nid);
	diff_buffers = (long long)get_avail_buffers_value() * MAX_NUMNODES -
		diff_buffers; /* B_target - B_current */
	diff_buffers *= SZ_1M / PAGE_SIZE; /* MB to page */
	diff_buffers *=
		(long long)get_compress_ratio_value(); /* after_comp to before_comp */
	diff_buffers = diff_buffers * percent_constant /
		nr_total; /* page to ratio */
	/*lint -e666 */

	return min(zram_wm_ratio, zram_wm_ratio - diff_buffers);
	/*lint +e666 */
}

bool zram_watermark_ok(void)
{
	const unsigned int percent_constant = 100;
	u64 curr_ratio;
	u64 nr_zram_used;
	u64 nr_wm;

	curr_ratio = get_zram_current_watermark();
	nr_zram_used = hyperhold_get_zram_used_pages();
	nr_wm = __totalram_pages() * curr_ratio / percent_constant;
	return nr_zram_used > nr_wm;
}

#define SWAP_MORE_ZRAM (50 * (SZ_1M))
static u64 zram_watermark_diff(void)
{
	const unsigned int percent_constant = 100;
	u64 curr_ratio;
	u64 nr_zram_used;
	u64 nr_wm;

	curr_ratio = get_zram_current_watermark();
	nr_zram_used = hyperhold_get_zram_used_pages();
	nr_wm = __totalram_pages() * curr_ratio / percent_constant;
	if (nr_zram_used > nr_wm)
		return (nr_zram_used - nr_wm) * PAGE_SIZE + SWAP_MORE_ZRAM;

	return 0;
}

static u64 zswapd_buffer_diff(void)
{
	u64 avail;
	u64 curr_buffers = 0;
	int nid;

	avail = get_high_avail_buffers_value() * MAX_NUMNODES;
	for_each_node_state(nid, N_MEMORY)
		curr_buffers += calc_sys_cur_avail_buffers(nid);
	if (curr_buffers < avail)
		return (avail - curr_buffers) * SZ_1M;

	return 0;
}

void wakeup_zswapd(pg_data_t *pgdat)
{
	unsigned long curr_interval;

	if (IS_ERR(pgdat->zswapd))
		return;

	if (!wq_has_sleeper(&pgdat->zswapd_wait))
		return;

	/* wake up when the buffer is lower than min_avail_buffer */
	if (min_buffer_is_suitable(pgdat->node_id)) {
		parad_stat_add(PARAD_ZSWAPD_SUITABLE, 1);
		return;
	}

	curr_interval =
		jiffies_to_msecs(jiffies - pgdat->last_zswapd_time);
	if (curr_interval < pgdat->zswapd_skip_interval) {
		count_vm_event(ZSWAPD_EMPTY_ROUND_SKIP_TIMES);
		parad_stat_add(PARAD_ZSWAPD_INTERVAL, 1);
		return;
	}

	atomic_set(&pgdat->zswapd_wait_flag, 1);
	parad_stat_add(PARAD_ZSWAPD_REALWAKE, 1);
	wake_up_interruptible(&pgdat->zswapd_wait);
}

void wake_all_zswapd(void)
{
	pg_data_t *pgdat = NULL;
	int nid;

	for_each_online_node(nid) {
		pgdat = NODE_DATA(nid);
		wakeup_zswapd(pgdat);
	}
}

static unsigned long zswapd_shrink_list(enum lru_list lru,
		unsigned long nr_to_scan, struct lruvec *lruvec,
		struct scan_control *sc)
{
#ifdef CONFIG_HW_RECLAIM_ACCT
	unsigned long nr_reclaimed;
	unsigned long nr_scanned = sc->nr_scanned;
	struct reclaim_acct *tmp_ra = NULL;

	reclaimacct_substage_start(RA_SHRINKANON, NULL);
#endif
	if (is_active_lru(lru)) {
		if (__inactive_is_low(lruvec, lru, is_file_lru(lru), sc, true))
			zswapd_shrink_active_list(nr_to_scan, lruvec, sc, lru);
#ifdef CONFIG_HW_RECLAIM_ACCT
		reclaimacct_substage_end(RA_SHRINKANON, 0, 0, NULL);
#endif
		return 0;
	}

#ifdef CONFIG_HW_RECLAIM_ACCT
	/*
	 * If the zswapd procedure contains direct_reclaim, the data in
	 * struct reclaim_acct is cleared after the direct_reclaim is complete.
	 * Retain the data in struct reclaim_acct to prevent this scenario.
	 */
	tmp_ra = current->reclaim_acct;
	current->reclaim_acct = NULL;

	nr_reclaimed = shrink_inactive_list(nr_to_scan, lruvec, sc, lru);

	current->reclaim_acct = tmp_ra;
	tmp_ra = NULL;

	reclaimacct_substage_end(RA_SHRINKANON, nr_reclaimed, sc->nr_scanned - nr_scanned, NULL);
	return nr_reclaimed;
#else
	return shrink_inactive_list(nr_to_scan, lruvec, sc, lru);
#endif
}

static void zswapd_shrink_anon_memcg(struct pglist_data *pgdat,
		struct mem_cgroup *memcg, struct scan_control *sc,
		unsigned long *nr)
{
	struct lruvec *lruvec = __mem_cgroup_lruvec(memcg, pgdat);
	unsigned long nr_to_scan;
	enum lru_list lru;
	unsigned long nr_reclaimed = 0;
	struct blk_plug plug;

	blk_start_plug(&plug);

	while (nr[LRU_INACTIVE_ANON] || nr[LRU_ACTIVE_ANON]) {
		if (high_buffer_is_suitable(pgdat->node_id))
			break;
		for (lru = 0; lru <= LRU_ACTIVE_ANON; lru++) {
			if (nr[lru]) {
				nr_to_scan = min(nr[lru], SWAP_CLUSTER_MAX);
				nr[lru] -= nr_to_scan;
				nr_reclaimed +=
					zswapd_shrink_list(lru,
							nr_to_scan,
							lruvec, sc);
			}
		}
	}
	blk_finish_plug(&plug);
	sc->nr_reclaimed += nr_reclaimed;
}

#ifdef CONFIG_PARA_SWAPD
static bool zswapd_shrink_anon_legacy(pg_data_t *pgdat, struct scan_control *sc)
#else
static bool zswapd_shrink_anon(pg_data_t *pgdat, struct scan_control *sc)
#endif
{
	unsigned long nr[NR_LRU_LISTS];
	struct mem_cgroup *memcg = NULL;
	const u32 percent_constant = 100;

	while ((memcg = get_next_memcg(memcg))) {
		struct lruvec *lruvec = __mem_cgroup_lruvec(memcg, pgdat);
		u64 nr_active, nr_inactive;
		u64 nr_zram, nr_eswap;
		u64 zram_ratio;

		/* reclaim and try to meet the high buffer watermark */
		if (high_buffer_is_suitable(pgdat->node_id)) {
			get_next_memcg_break(memcg);
			break;
		}

#ifdef CONFIG_MEMCG_PROTECT_LRU
		/* Skip if it is a protect memcg. */
		if (is_prot_memcg(memcg, false))
			continue;
#endif

		if (get_memcg_anon_refault_status(memcg, pgdat)) {
			count_vm_event(ZSWAPD_MEMCG_REFAULT_SKIP);
			continue;
		}

		nr_active = lruvec_lru_size(lruvec, LRU_ACTIVE_ANON,
				MAX_NR_ZONES);
		nr_inactive = lruvec_lru_size(lruvec,
				LRU_INACTIVE_ANON, MAX_NR_ZONES);
		nr_zram = hyperhold_read_mcg_stats(memcg, MCG_ZRAM_PG_SZ);
		nr_eswap =
			hyperhold_read_mcg_stats(memcg, MCG_DISK_STORED_PG_SZ);

		zram_ratio = (nr_zram + nr_eswap) * percent_constant /
			(nr_inactive + nr_active + nr_zram + nr_eswap + 1);

		if (zram_ratio >= (u32)atomic_read(
				&memcg->memcg_reclaimed.ub_mem2zram_ratio)) {
			count_vm_event(ZSWAPD_MEMCG_RATIO_SKIP);
			continue;
		}

		nr[LRU_ACTIVE_ANON] = nr_active >> (unsigned int)sc->priority;
		nr[LRU_INACTIVE_ANON] =
				nr_inactive >> (unsigned int)sc->priority;
		nr[LRU_ACTIVE_FILE] = 0;
		nr[LRU_INACTIVE_FILE] = 0;
#ifdef CONFIG_HYPERHOLD_FILE_LRU
		zswapd_shrink_anon_memcg(pgdat, memcg, sc, nr);
#else
		shrink_node_memcg(pgdat, memcg, sc, nr);
#endif

		if (sc->nr_reclaimed >= sc->nr_to_reclaim) {
			get_next_memcg_break(memcg);
			break;
		}
	}

	return sc->nr_scanned >= sc->nr_to_reclaim;
}

#ifdef CONFIG_PARA_SWAPD
static LIST_HEAD(parad_memcg_list);
static DEFINE_SEMAPHORE(parad_sem);
wait_queue_head_t parad_zswapd_wait;
atomic_t parad_zswapd_wait_flag;
struct mm_parad_task parad_tasks[PARAD_TASK_MAX];
unsigned long parad_task_mask;

static void parad_shrink_memcg(struct mm_parad_entry *entry)
{
	struct mem_cgroup *memcg  = entry->memcg;
	struct pglist_data *pgdat = entry->pgdat;
	struct scan_control sc;
	unsigned long nr_scanned, nr_reclaimed, nr_reclaimed_anon,
		nr_scanned_file, nr_scanned_anon,
		start = parad_get_counter();
	unsigned int isolate_count;

	spin_lock_irq(&pgdat->lru_lock);
	sc = *entry->sc;
	spin_unlock_irq(&pgdat->lru_lock);

	nr_scanned        = sc.nr_scanned;
	nr_scanned_file   = sc.nr_scanned_file;
	nr_scanned_anon   = sc.nr_scanned_anon;
	nr_reclaimed      = sc.nr_reclaimed;
	nr_reclaimed_anon = sc.nr_reclaimed_anon;
	isolate_count     = sc.isolate_count;

	zswapd_shrink_anon_memcg(pgdat, memcg, &sc, entry->nr);

	/* give back the real reclaimed info of per memcg to k/zswapd sc */
	spin_lock_irq(&pgdat->lru_lock);
	entry->sc->nr_scanned        += sc.nr_scanned - nr_scanned;
	entry->sc->nr_reclaimed      += sc.nr_reclaimed - nr_reclaimed;
	entry->sc->nr_reclaimed_anon += sc.nr_reclaimed_anon - nr_reclaimed_anon;
	entry->sc->isolate_count     += sc.isolate_count - isolate_count;
	entry->sc->nr_scanned_file   += sc.nr_scanned_file - nr_scanned_file;
	entry->sc->nr_scanned_anon   += sc.nr_scanned_anon - nr_scanned_anon;
	spin_unlock_irq(&pgdat->lru_lock);

	parad_stat_add(PARAD_ZSWAPD_WORK, parad_get_counter() - start);
	clear_bit(F_PARAD_RECLAIMING, &entry->flags);
	css_put(&memcg->css);
}

static int parad_thread(void *p)
{
	struct mm_parad_entry *entry = NULL;
	struct mm_parad_task *parad_task = (struct mm_parad_task *)p;

	set_freezable();

	while (!kthread_should_stop()) {
		(void)wait_event_freezable(parad_task->parad_wait,
				atomic_read(&parad_task->parad_wait_flag));
		down(&parad_sem);
		if (!test_bit(parad_task->idx, &parad_task_mask))
			set_bit(parad_task->idx, &parad_task_mask);

		entry = list_first_entry_or_null(
			&parad_memcg_list, struct mm_parad_entry, list);
		if (!entry) {
			atomic_set(&parad_task->parad_wait_flag, 0);
			clear_bit(parad_task->idx, &parad_task_mask);
			if (parad_task_mask) {
				up(&parad_sem);
				continue;
			}
			atomic_set(&parad_zswapd_wait_flag, 1);
			wake_up_interruptible_all(&parad_zswapd_wait);
			up(&parad_sem);
			continue;
		}
		list_del(&entry->list);
		up(&parad_sem);

		parad_shrink_memcg(entry);
	}

	return 0;
}

int parad_task_create(void)
{
	int i;
	unsigned int sched_priority = MAX_PRIO - 5;
	struct cpumask mask = CPU_MASK_NONE;
	int unmask[] = { 8, 9 };

	for_each_possible_cpu(i) {
		if (topology_physical_package_id(i) == 1)
			cpumask_set_cpu(i, &mask);
	}
	for (i = 0; i < ARRAY_SIZE(unmask); i++)
		cpumask_clear_cpu(unmask[i], &mask);

	init_waitqueue_head(&parad_zswapd_wait);
	atomic_set(&parad_zswapd_wait_flag, 0);

	for (i = 0; i < PARAD_TASK_MAX; i++) {
		struct mm_parad_task *parad_task = &parad_tasks[i];

		init_waitqueue_head(&parad_task->parad_wait);
		atomic_set(&parad_task->parad_wait_flag, 0);
		parad_task->idx = i;

		parad_task->task = kthread_create(
			parad_thread, parad_task, "para_zswapd%d", i);
		if (IS_ERR(parad_task->task)) {
			pr_err("failed to start para_zswapd%d\n", i);
			return PTR_ERR(parad_task->task);
		}

		set_cpus_allowed_ptr(parad_task->task, &mask);
		set_user_nice(parad_task->task, PRIO_TO_NICE(sched_priority));
		wake_up_process(parad_task->task);
	}

	for (i = 1; i < PARAD_TASK_MAX; i++)
		sched_core_share_prefer(parad_tasks[i].task, parad_tasks[0].task);

	return 0;
}

static bool parad_task_ok(void)
{
	unsigned int i;

	for (i = 0; i < PARAD_TASK_MAX; i++) {
		if (IS_ERR(parad_tasks[i].task))
			return false;
	}

	return true;
}

static bool parad_has_sleeper(unsigned int max)
{
	unsigned int i;
	unsigned int cnt = 0;

	while (true) {
		for (i = 0; i < max; i++) {
			if (!wq_has_sleeper(&parad_tasks[i].parad_wait))
				break;
			cnt++;
			if (cnt == max)
				return true;
		}
		set_current_state(TASK_UNINTERRUPTIBLE);
		schedule_timeout(4);
		set_current_state(TASK_RUNNING);
		cnt = 0;
	}
	return true;
}

static void parad_wake_up(unsigned int max)
{
	unsigned int i;

	for (i = 0; i < max; i++) {
		struct mm_parad_task *parad_task = &parad_tasks[i];

		if (wq_has_sleeper(&parad_task->parad_wait)) {
			atomic_set(&parad_task->parad_wait_flag, 1);
			wake_up_interruptible(&parad_task->parad_wait);
		}
	}
}

static bool zswapd_shrink_anon(pg_data_t *pgdat, struct scan_control *sc)
{
	bool ret;
	struct mem_cgroup *memcg = NULL;
	struct mm_parad_entry *entry = NULL;
	const unsigned int percent_constant = 100;

	unsigned long nr_scanned = sc->nr_scanned,
		      nr_reclaimed = sc->nr_reclaimed,
		      start = parad_get_counter();

	if (!parad_task_ok() || get_parad_enable() == 0) {
		ret = zswapd_shrink_anon_legacy(pgdat, sc);
		goto out;
	}

#ifdef CONFIG_HW_RECLAIM_ACCT
	reclaimacct_substage_start(RA_SHRINKANON, NULL);
#endif

	while ((memcg = get_next_memcg(memcg))) {
#ifdef CONFIG_MEMCG_SKIP_RECLAIM
		if (memcg->skip_reclaim_anon)
			continue;
#endif
		u64 nr_active, nr_inactive, nr_zram, nr_eswap, zram_ratio;
		struct lruvec *lruvec = __mem_cgroup_lruvec(memcg, pgdat);

		/* reclaim and try to meet the high buffer watermark */
		if (high_buffer_is_suitable(pgdat->node_id)) {
			get_next_memcg_break(memcg);
			break;
		}

		/*
		 * WARNING: need lru_lock of sc ?
		 **/
		if (sc->nr_reclaimed >= sc->nr_to_reclaim) {
			get_next_memcg_break(memcg);
			break;
		}

#ifdef CONFIG_MEMCG_PROTECT_LRU
		/* Skip if it is a protect memcg. */
		if (is_prot_memcg(memcg, false))
			continue;
#endif

		if (get_memcg_anon_refault_status(memcg, pgdat)) {
			count_vm_event(ZSWAPD_MEMCG_REFAULT_SKIP);
			continue;
		}

		nr_active = lruvec_lru_size(lruvec, LRU_ACTIVE_ANON,
				MAX_NR_ZONES);
		nr_inactive = lruvec_lru_size(lruvec,
				LRU_INACTIVE_ANON, MAX_NR_ZONES);
		nr_zram = hyperhold_read_mcg_stats(memcg, MCG_ZRAM_PG_SZ);
		nr_eswap =
			hyperhold_read_mcg_stats(memcg, MCG_DISK_STORED_PG_SZ);

		zram_ratio = (nr_zram + nr_eswap) * percent_constant /
			(nr_inactive + nr_active + nr_zram + nr_eswap + 1);

		if (zram_ratio >= (u32)atomic_read(
				&memcg->memcg_reclaimed.ub_mem2zram_ratio)) {
			count_vm_event(ZSWAPD_MEMCG_RATIO_SKIP);
			continue;
		}

		if (test_and_set_bit(F_PARAD_RECLAIMING, &memcg->zpara.flags))
			continue;

		entry = &memcg->zpara;
		entry->nr[LRU_ACTIVE_ANON] = nr_active >> (unsigned int)sc->priority;
		entry->nr[LRU_INACTIVE_ANON] =
				nr_inactive >> (unsigned int)sc->priority;
		entry->nr[LRU_ACTIVE_FILE] = 0;
		entry->nr[LRU_INACTIVE_FILE] = 0;

		if (!entry->nr[LRU_ACTIVE_ANON] && !entry->nr[LRU_INACTIVE_ANON]) {
			clear_bit(F_PARAD_RECLAIMING, &entry->flags);
			continue;
		}

		entry->memcg = memcg;
		entry->pgdat = pgdat;
		entry->sc    = sc;
		css_get(&memcg->css);

		down(&parad_sem);
		list_add_tail(&entry->list, &parad_memcg_list);
		up(&parad_sem);
	}

	atomic_set(&parad_zswapd_wait_flag, 0);
	if (parad_has_sleeper(PARAD_TASK_MAX))
		parad_wake_up(PARAD_TASK_MAX);

	(void)wait_event_freezable(parad_zswapd_wait,
			atomic_read(&parad_zswapd_wait_flag));

#ifdef CONFIG_HW_RECLAIM_ACCT
	reclaimacct_substage_end(RA_SHRINKANON,
		sc->nr_reclaimed - nr_reclaimed, sc->nr_scanned - nr_scanned, NULL);
#endif

	ret = sc->nr_scanned >= sc->nr_to_reclaim;
out:
	parad_stat_add(PARAD_ZSWAPD, parad_get_counter() - start);
	parad_stat_add(PARAD_ZSWAPD_NR_SCANNED, sc->nr_scanned - nr_scanned);
	parad_stat_add(PARAD_ZSWAPD_NR_RECLAIMED, sc->nr_reclaimed - nr_reclaimed);

	return ret;
}
#endif

static u64 __calc_nr_to_reclaim(int nid)
{
	u32 curr_buffers;
	u64 high_buffers;
	u64 max_reclaim_size_value;
	u64 reclaim_size = 0;

	high_buffers = get_high_avail_buffers_value();
	curr_buffers = calc_sys_cur_avail_buffers(nid);
	max_reclaim_size_value = get_zswapd_max_reclaim_size();
	if (curr_buffers < high_buffers)
		reclaim_size = high_buffers - curr_buffers;

	/* once max reclaim target is max_reclaim_size_value */
	reclaim_size = min(reclaim_size, max_reclaim_size_value);

	return reclaim_size * SZ_1M / PAGE_SIZE; /* MB to pages */
}

static void zswapd_shrink_node(pg_data_t *pgdat)
{
	struct scan_control sc = {
		.gfp_mask = GFP_KERNEL,
		.order = 0,
		.priority = DEF_PRIORITY / 2,
		.may_writepage = !laptop_mode,
		.may_unmap = 1,
		.may_swap = 1,
		.reclaim_idx = MAX_NR_ZONES - 1,
	};

	do {
		unsigned long nr_reclaimed = sc.nr_reclaimed = 0;
		bool raise_priority = true;

		/* reclaim and try to meet the high buffer watermark */
		if (high_buffer_is_suitable(pgdat->node_id))
			break;

		sc.nr_scanned = 0;
		sc.nr_to_reclaim = __calc_nr_to_reclaim(pgdat->node_id);

		if (zswapd_shrink_anon(pgdat, &sc))
			raise_priority = false;

		count_vm_events(ZSWAPD_SCANNED, sc.nr_scanned);
		count_vm_events(ZSWAPD_RECLAIMED, sc.nr_reclaimed);

		if (try_to_freeze() || kthread_should_stop())
			break;

		nr_reclaimed = sc.nr_reclaimed - nr_reclaimed;
		if (raise_priority || !nr_reclaimed)
			sc.priority--;
	} while (sc.priority >= 1);

	/*
	 * When meets the first empty round, set the interval to t.
	 * If the following round is still empty, set the interval
	 * to 2t. If the round is always empty, then 4t, 8t, and so on.
	 * But make sure the interval is not more than the max_skip_interval.
	 * Once a non-empty round occurs, reset the interval to 0.
	 */
	if (sc.nr_reclaimed < get_empty_round_check_threshold_value()) {
		update_zswapd_empty_skip_interval(pgdat);
	} else {
		pgdat->zswapd_skip_interval = 0;
		pgdat->last_round_is_empty = false;
	}
}

static u64 get_do_eswap_size(bool refault)
{
	u64 size = 0;
	u64 zram_wm_diff = 0;
	u64 buffer_diff = 0;
	enum zswapd_eswap_policy policy = get_zswapd_eswap_policy();
	if (policy == CHECK_BUFFER_ZRAMRAITO_BOTH) {
		zram_wm_diff = zram_watermark_diff();
		buffer_diff = zswapd_buffer_diff();
		size = max(zram_wm_diff, buffer_diff);
	} else if (policy == CHECK_BUFFER_ONLY &&
			(zram_watermark_ok() || refault)) {
		size = zswapd_buffer_diff();
	}

	return size;
}

static int zswapd(void *p)
{
	pg_data_t *pgdat = (pg_data_t *)p;
	struct task_struct *tsk = current;
	int shrink_slab_prio;
#ifdef CONFIG_MM_AMA
	const struct cpumask *cpumask = cpu_online_mask;
#else
	const struct cpumask *cpumask = cpumask_of_node(pgdat->node_id);
#endif

	/* save zswapd pid for schedule strategy */
	zswapd_pid = tsk->pid;

	if (!cpumask_empty(cpumask))
		set_cpus_allowed_ptr(tsk, cpumask);

	set_freezable();

	while (!kthread_should_stop()) {
		bool refault = false;
		u64 size = 0;

		/*lint -e578 */
		(void)wait_event_freezable(pgdat->zswapd_wait,
			atomic_read(&pgdat->zswapd_wait_flag));
		/*lint +e578 */
		atomic_set(&pgdat->zswapd_wait_flag, 0);

		pgdat->last_zswapd_time = jiffies;

		count_vm_event(ZSWAPD_WAKEUP);
		trace_mm_zswapd_wake(pgdat->node_id);

		if (is_swap_full()) {
			zswapd_pressure_report(LEVEL_CRITICAL);
			count_vm_event(ZSWAPD_CRITICAL_PRESS);
			update_zswapd_empty_skip_interval(pgdat);
			goto do_eswap;
		}

		zswapd_pressure_report(LEVEL_LOW);

		if (get_area_anon_refault_status(pgdat)) {
			refault = true;
			count_vm_event(ZSWAPD_REFAULT);
			goto do_eswap;
		}

#ifdef CONFIG_HW_RECLAIM_ACCT
		reclaimacct_start(ZSWAPD_RECLAIM);
#endif
		zswapd_shrink_node(pgdat);
#ifdef CONFIG_HW_RECLAIM_ACCT
		reclaimacct_end(ZSWAPD_RECLAIM);
#endif

do_eswap:
		if (!hyperhold_reclaim_work_running()) {
			size = get_do_eswap_size(refault);
			if (size >= SZ_1M) {
				count_vm_event(ZSWAPD_SWAPOUT);
#ifdef CONFIG_HP_CORE
				(void)hyperhold_reclaim_in(size);
#endif
			}
		}

		if (!buffer_is_suitable(pgdat->node_id)) {
			zswapd_pressure_report(LEVEL_MEDIUM);
			count_vm_event(ZSWAPD_MEDIUM_PRESS);
		}

		if (high_buffer_is_suitable(pgdat->node_id))
			continue;

		shrink_slab_prio = (int)get_zswapd_shrink_slab_prio();
		if (shrink_slab_prio > DEF_PRIORITY)
			continue;
#ifdef CONFIG_HW_RECLAIM_ACCT
		reclaimacct_start(ZSWAPD_RECLAIM);
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
		shrink_slab(GFP_KERNEL, pgdat->node_id, root_mem_cgroup, shrink_slab_prio);
#else
		shrink_slab(GFP_KERNEL, pgdat->node_id, NULL,
				SWAP_CLUSTER_MAX, SWAP_CLUSTER_MAX);
#endif

#ifdef CONFIG_HW_RECLAIM_ACCT
		reclaimacct_end(ZSWAPD_RECLAIM);
#endif

		trace_mm_zswapd_sleep(pgdat->node_id);
	}

#ifdef CONFIG_HW_RECLAIM_ACCT
	reclaimacct_destroy();
#endif

	return 0;
}

/*
 * This zswapd start function will be called by init and node-hot-add.
 */
int zswapd_run(int nid)
{
	pg_data_t *pgdat = NODE_DATA(nid);
	const unsigned int priority_less = 5;
	struct sched_param param = {
		.sched_priority = MAX_PRIO - priority_less,
	};

	if (pgdat->zswapd)
		return 0;

	atomic_set(&pgdat->zswapd_wait_flag, 0);
	pgdat->zswapd = kthread_create(zswapd, pgdat, "zswapd%d", nid);
	if (IS_ERR(pgdat->zswapd)) {
		pr_err("Failed to start zswapd on node %d\n", nid);
		return PTR_ERR(pgdat->zswapd);
	}

	sched_setscheduler_nocheck(pgdat->zswapd, SCHED_NORMAL, &param);
	set_user_nice(pgdat->zswapd, PRIO_TO_NICE(param.sched_priority));
	wake_up_process(pgdat->zswapd);

	return 0;
}

/*
 * Called by memory hotplug when all memory in a node is offlined.  Caller must
 * hold mem_hotplug_begin/end().
 */
void zswapd_stop(int nid)
{
	struct task_struct *zswapd = NODE_DATA(nid)->zswapd; /*lint !e578*/

	if (zswapd) {
		kthread_stop(zswapd);
		NODE_DATA(nid)->zswapd = NULL;
		zswapd_pid = -1;
	}
}

/* It's optimal to keep kswapds on the same CPUs as their memory, but
 * not required for correctness.  So if the last cpu in a node goes
 * away, we get changed to run anywhere: as the first one comes back,
 * restore their cpu bindings.
 */
static int zswapd_cpu_online(unsigned int cpu)
{
	int nid;

	for_each_node_state(nid, N_MEMORY) {
		pg_data_t *pgdat = NODE_DATA(nid);
		const struct cpumask *mask;

#ifdef CONFIG_MM_AMA
		mask = cpu_online_mask;
#else
		mask = cpumask_of_node(pgdat->node_id);
#endif
		/*lint -e574 */
		if (cpumask_any_and(cpu_online_mask, mask) < nr_cpu_ids)
		/*lint +e574 */
			/* One of our CPUs online: restore mask */
			set_cpus_allowed_ptr(pgdat->zswapd, mask);
	}
	return 0;
}

static int __init zswapd_init(void)
{
	int nid;
	int ret;

	ret = cpuhp_setup_state_nocalls(CPUHP_AP_ONLINE_DYN,
					"mm/zswapd:online", zswapd_cpu_online,
					NULL);
	if (ret < 0) {
		pr_err("zswapd: failed to register hotplug callbacks.\n");
		return ret;
	}

	for_each_node_state(nid, N_MEMORY)
		zswapd_run(nid);

	return 0;
}
module_init(zswapd_init)
#endif
