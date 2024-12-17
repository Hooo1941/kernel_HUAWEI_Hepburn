/*
 * reclaim_acct.c
 *
 * Memory reclaim delay accounting
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#define pr_fmt(fmt) "reclaimacct: " fmt

#include <chipset_common/reclaim_acct/reclaim_acct.h>

#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/moduleparam.h>
#include <linux/ratelimit.h>
#include <linux/sched/clock.h>
#include <linux/sched/task.h>
#include <linux/slab.h>
#include <linux/swap.h>
#include <linux/types.h>
#include <platform/trace/hooks/memcheck.h>
#include <linux/time64.h>
#include <linux/fs.h>

#ifdef CONFIG_SPLIT_SHRINKER
#include <linux/split_shrinker.h>
#endif

#include <log/log_usertype.h>
#include <securec.h>

#include "internal.h"
#include "reclaimacct_show.h"

/*
 * Memory pool of reclaim_acct. Maximum of NR_POOLMEMBER structs can be
 * used at the same time. Once memory pool is used up, the record should
 * be simply abandoned without any runtime error.
 */

const char *reclaim_type_str[RECLAIM_TYPES] = {
	DIRECT_RECLAIM_STR,
	KSWAPD_NODE_STR,
	ZSWAPD_NODE_STR,
	KSHRINKD_NODE_STR
};

const char *stub_name[NR_RA_STUBS] = {
	DIRECT_RECLAIM_STR,
	DRAIN_ALL_PAGES_STR,
	SHRINK_FILE_LIST_STR,
	SHRINK_ANON_LIST_STR,
	SHRINK_SLAB_STR,
	SOFT_LIMIT_STR
};

static u64 g_delay_max[RECLAIM_TYPES];
static DEFINE_SPINLOCK(g_delay_max_lock);

/* Define a mem pool of NR_POOLMEMBER pointers */
#define NR_POOLMEMBER 128
static struct reclaim_acct *g_mempool[NR_POOLMEMBER];
static int g_mempool_index = NR_POOLMEMBER - 1;
static DEFINE_SPINLOCK(g_mempool_lock);

/* Once initialized, the variable should never be changed */
static bool g_reclaimacct_is_off = true;
static int g_reclaimacct_disable = 1;
static int g_reclaim_trace_enable;
static int g_kswapd_trace_enable;
static int g_kswapd_delay_threshold = DEFAULT_DELAY_THRESHOLD;

static inline void tracing_mark_write(int pid, const char *name,
	bool trace_begin, void *scan, unsigned long reclaim)
{
	if (likely(!g_reclaim_trace_enable))
		return;

	if (trace_begin) {
		if (!scan)
			trace_printk("B|%d|%s\n", pid, name);
		else
			trace_printk("B|%d|shrink_slab_%pS\n", pid, scan);
	} else {
		trace_printk("E|%d\n", pid);
		trace_printk("C|%d|%s reclaim|%lu\n", pid, name, reclaim);
	}
}

static inline void reclaimacct_trace_begin(const char *name, void *scan, unsigned long reclaim)
{
	return tracing_mark_write(current->tgid, name, true, scan, reclaim);
}

static inline void reclaimacct_trace_end(const char *name, void *scan, unsigned long reclaim)
{
	return tracing_mark_write(current->tgid, name, false, scan, reclaim);
}

#ifdef CONFIG_SCHEDSTATS
void get_ra_sched_blocked_info(struct task_struct *tsk, u64 blocked_time)
{
	void *pt = NULL;

	if (likely(!tsk->reclaim_acct))
		return;

	pt = (void *)get_wchan(tsk);
	if (blocked_time > SCHED_BLOCK_THRESHOLD * NSEC_PER_MSEC &&
		blocked_time > tsk->reclaim_acct->sched_blocked_max_time) {
		tsk->reclaim_acct->p_stack = pt;
		tsk->reclaim_acct->sched_blocked_max_time = blocked_time;
	}
}
#endif

/* reclaimacct_alloc MUST be used with reclaimacct_free */
static struct reclaim_acct *__reclaimacct_alloc(void)
{
	struct reclaim_acct *elem = NULL;

	spin_lock(&g_mempool_lock);
	if (g_mempool_index >= 0 &&
	    g_mempool_index < NR_POOLMEMBER) {
		elem = g_mempool[g_mempool_index];
		g_mempool_index--;
	} else if (g_mempool_index == -1) {
		pr_warn_ratelimited("mempool is used up\n");
	} else {
		WARN_ONCE(1, "index %d out of range\n", g_mempool_index);
	}
	spin_unlock(&g_mempool_lock);

	return elem;
}

static struct reclaim_acct *reclaimacct_alloc(enum ra_reclaim_type type)
{
	if (is_system_reclaim(type))
		return kzalloc(sizeof(struct reclaim_acct), GFP_KERNEL);
	return __reclaimacct_alloc();
}

/* reclaimacct_free MUST be used with reclaimacct_alloc */
static void __reclaimacct_free(struct reclaim_acct *elem)
{
	spin_lock(&g_mempool_lock);
	if (g_mempool_index >= -1 &&
	    g_mempool_index < NR_POOLMEMBER - 1) {
		g_mempool_index++;
		g_mempool[g_mempool_index] = elem;
	} else {
		WARN_ONCE(1, "index %d out of range\n", g_mempool_index);
	}
	spin_unlock(&g_mempool_lock);
}

static void reclaimacct_free(struct reclaim_acct *ra, enum ra_reclaim_type type)
{
	(void)memset_s(ra, sizeof(struct reclaim_acct), 0,
		sizeof(struct reclaim_acct));

	if (!is_system_reclaim(type))
		__reclaimacct_free(ra);
}

void reclaimacct_get_nr_to_scan(const unsigned long *nr)
{
	enum lru_list lru;

	if (g_reclaimacct_is_off || g_reclaimacct_disable ||
		!current->reclaim_acct)
		return;

	for_each_evictable_lru(lru)
		current->reclaim_acct->nr_to_scanned[lru] += nr[lru];

	return;
}

void kswapd_change_block_status(void)
{
	if (g_reclaimacct_is_off || g_reclaimacct_disable ||
		!current->reclaim_acct)
		return;

	current->reclaim_acct->is_blocked = true;
}

static void show_delay_info(struct reclaim_acct *ra)
{
	u64 delay[NR_RA_STUBS];
	int i;

	for (i = 0; i < NR_RA_STUBS; i++)
		delay[i] = ra->delay[i] / NSEC_PER_MSEC;

	pr_err("delay: %s=%llums %s=%llums %s=%llums %s=%llums %s=%llums\n",
		reclaim_type_str[ra->reclaim_type], delay[RA_RECLAIM],
		SHRINK_FILE_LIST_STR, delay[RA_SHRINKFILE],
		SHRINK_ANON_LIST_STR, delay[RA_SHRINKANON],
		SHRINK_SLAB_STR, delay[RA_SHRINKSLAB],
		SOFT_LIMIT_STR, delay[RA_SOFTLIMIT]);

#ifdef CONFIG_SCHEDSTATS
	pr_err("sched: sleep=%llums runable=%llums\n",
		ra->sum_sleep_runtime / NSEC_PER_MSEC,
		ra->wait_sum / NSEC_PER_MSEC);

	if (ra->p_stack && ra->sched_blocked_max_time)
		pr_err("sched: %pS blocked max_time=%llums\n",
			ra->p_stack, ra->sched_blocked_max_time / NSEC_PER_MSEC);
#endif

	pr_err("nr_to_scan: %s=%llu %s=%llu %s=%llu %s=%llu %s=%llu",
		LRU_INACTIVE_ANON_STR, ra->nr_to_scanned[LRU_INACTIVE_ANON],
		LRU_ACTIVE_ANON_STR, ra->nr_to_scanned[LRU_ACTIVE_ANON],
		LRU_INACTIVE_FILE_STR, ra->nr_to_scanned[LRU_INACTIVE_FILE],
		LRU_ACTIVE_FILE_STR, ra->nr_to_scanned[LRU_ACTIVE_FILE],
		LRU_UNEVICTABLE_STR, ra->nr_to_scanned[LRU_UNEVICTABLE]);

	pr_err("scanned: %s=%llu %s=%llu %s=%llu %s=%llu %s=%llu\n",
		reclaim_type_str[ra->reclaim_type], ra->scanned[RA_RECLAIM],
		SHRINK_FILE_LIST_STR, ra->scanned[RA_SHRINKFILE],
		SHRINK_ANON_LIST_STR, ra->scanned[RA_SHRINKANON],
		SHRINK_SLAB_STR, ra->scanned[RA_SHRINKSLAB],
		SOFT_LIMIT_STR, ra->scanned[RA_SOFTLIMIT]);

	pr_err("reclaimed: %s=%llu %s=%llu %s=%llu %s=%llu %s=%llu\n",
		reclaim_type_str[ra->reclaim_type], ra->freed[RA_RECLAIM],
		SHRINK_FILE_LIST_STR, ra->freed[RA_SHRINKFILE],
		SHRINK_ANON_LIST_STR, ra->freed[RA_SHRINKANON],
		SHRINK_SLAB_STR, ra->freed[RA_SHRINKSLAB],
		SOFT_LIMIT_STR, ra->freed[RA_SOFTLIMIT]);

	if (ra->scan_objects) {
		bool is_fs = is_super_cache_scan(ra->scan_objects);
		pr_err("shrinker: %pS %s %s delay_max: %lluns",
			ra->scan_objects, is_fs ? ra->fs_type : "",
			is_fs ? ra->s_id : "", ra->shrinker_delay_max);
	}
}

static void print_trace_info(struct reclaim_acct *ra)
{
	u64 delay[NR_RA_STUBS];
	int i;

	for (i = 0; i < NR_RA_STUBS; i++)
		delay[i] = ra->delay[i] / NSEC_PER_MSEC;

	trace_printk(
		"reclaimacct: Kswapd delay %s=%llums %s=%llums %s=%llums %s=%llums\n",
		KSWAPD_NODE_STR, delay[RA_RECLAIM],
		SHRINK_FILE_LIST_STR, delay[RA_SHRINKFILE],
		SHRINK_ANON_LIST_STR, delay[RA_SHRINKANON],
		SHRINK_SLAB_STR, delay[RA_SHRINKSLAB]);

#ifdef CONFIG_SCHEDSTATS
	trace_printk("reclaimacct: Kswapd sched sleep=%llums runable=%llums\n",
		ra->sum_sleep_runtime / NSEC_PER_MSEC,
		ra->wait_sum / NSEC_PER_MSEC);

	if (ra->p_stack && ra->sched_blocked_max_time)
		trace_printk("reclaimacct: Kswapd sched %pS blocked max_time=%llums\n",
			ra->p_stack, ra->sched_blocked_max_time / NSEC_PER_MSEC);
#endif

	trace_printk(
		"reclaimacct: Kswapd nr_to_scan %s=%llu %s=%llu %s=%llu %s=%llu %s=%llu",
		LRU_INACTIVE_ANON_STR, ra->nr_to_scanned[LRU_INACTIVE_ANON],
		LRU_ACTIVE_ANON_STR, ra->nr_to_scanned[LRU_ACTIVE_ANON],
		LRU_INACTIVE_FILE_STR, ra->nr_to_scanned[LRU_INACTIVE_FILE],
		LRU_ACTIVE_FILE_STR, ra->nr_to_scanned[LRU_ACTIVE_FILE],
		LRU_UNEVICTABLE_STR, ra->nr_to_scanned[LRU_UNEVICTABLE]);

	trace_printk(
		"reclaimacct: Kswapd scanned %s=%llu %s=%llu %s=%llu %s=%llu\n",
		KSWAPD_NODE_STR, ra->scanned[RA_RECLAIM],
		SHRINK_FILE_LIST_STR, ra->scanned[RA_SHRINKFILE],
		SHRINK_ANON_LIST_STR, ra->scanned[RA_SHRINKANON],
		SHRINK_SLAB_STR, ra->scanned[RA_SHRINKSLAB]);

	trace_printk(
		"reclaimacct: Kswapd reclaimed %s=%llu %s=%llu %s=%llu %s=%llu\n",
		KSWAPD_NODE_STR, ra->freed[RA_RECLAIM],
		SHRINK_FILE_LIST_STR, ra->freed[RA_SHRINKFILE],
		SHRINK_ANON_LIST_STR, ra->freed[RA_SHRINKANON],
		SHRINK_SLAB_STR, ra->freed[RA_SHRINKSLAB]);

	if (ra->scan_objects) {
		bool is_fs = is_super_cache_scan(ra->scan_objects);
		trace_printk("reclaimacct: Kswapd shrinker: %pS %s %s delay_max: %lluns",
			ra->scan_objects, is_fs ? ra->fs_type : "",
			is_fs ? ra->s_id : "", ra->shrinker_delay_max);
	}
}

static void print_reclaim_info(struct reclaim_acct *ra)
{
	show_delay_info(ra);
	pr_info("-------------------------------------------");

	spin_lock(&g_delay_max_lock);
	if (ra->delay[RA_RECLAIM] > g_delay_max[ra->reclaim_type]) {
		g_delay_max[ra->reclaim_type] = ra->delay[RA_RECLAIM];
		show_mem(SHOW_MEM_FILTER_NODES, NULL);
	}
	spin_unlock(&g_delay_max_lock);
}

static void get_fs_info(struct reclaim_acct *ra, struct shrinker *shrinker)
{
	struct super_block *sb = NULL;

	if (!is_super_cache_scan(shrinker->scan_objects))
		return;

	sb = container_of(shrinker, struct super_block, s_shrink);

	if (sb->s_type && sb->s_type->name)
		strlcpy(ra->fs_type, sb->s_type->name, sizeof(ra->fs_type));
	strlcpy(ra->s_id, sb->s_id, sizeof(ra->s_id));
}

static void trace_slab_delay_info(struct reclaim_acct *ra, struct shrinker *shrinker, u64 delay)
{
	bool is_fs = is_super_cache_scan(shrinker->scan_objects);

	trace_printk("shrinker: %pS %s %s delayed: %llums",
		shrinker->scan_objects, is_fs ? ra->fs_type : "",
		is_fs ? ra->s_id : "", delay / NSEC_PER_MSEC);
}

static void __reclaimacct_end(struct reclaim_acct *ra,  u64 freed,
	u64 scanned, enum reclaimacct_stubs stub, struct shrinker *shrinker)
{
	u64 now, ns, start, delay_threshold, block_threshold;
	int type = ra->reclaim_type;

	start = ra->start[stub];
	now = ktime_get_ns();
	if (now < start)
		return;

	ns = now - start;
	if (ns < DELAY_LV5 || is_system_reclaim(type)) {
		ra->delay[stub] += ns;
		ra->count[stub]++;
		ra->freed[stub] += freed;
		ra->scanned[stub] += scanned;
		/* Collect the data of substage into the data of the whole process.
		 * Unit:page. Values reclaimed by the slab are not included.
		 */
		if (stub != RA_SHRINKSLAB) {
			ra->freed[RA_RECLAIM] += freed;
			ra->scanned[RA_RECLAIM] += scanned;
		}
	}

	if (shrinker) {
		get_fs_info(ra, shrinker);

		if (ns > DELAY_LV2)
			trace_slab_delay_info(ra, shrinker, ns);

		if (ns > ra->shrinker_delay_max) {
			ra->scan_objects = shrinker->scan_objects;
			ra->shrinker_delay_max = ns;
		}
	}

	if (g_kswapd_delay_threshold < 0) {
		pr_info(" kswapd_delay_threshold negative, reset with 500ms");
		g_kswapd_delay_threshold = DEFAULT_DELAY_THRESHOLD;
	}

	delay_threshold = g_kswapd_delay_threshold * NSEC_PER_MSEC;
	block_threshold = KWSAPD_BLOCK_DELAY_THRESHOLD * NSEC_PER_MSEC;

	if (ns > DELAY_LV4 && ns < DELAY_LV5) {
		stub_name[0] = reclaim_type_str[type];
		pr_warn_ratelimited("%s timeout:%luns\n", stub_name[stub], ns);

		if (shrinker)
			pr_warn_ratelimited("shrinker=%ps\n", shrinker->scan_objects);

		if (stub == RA_RECLAIM)
			print_reclaim_info(ra);
	} else if (type == KSWAPD_RECLAIM && (ra->delay[RA_RECLAIM] > delay_threshold ||
		(ra->is_blocked && ra->delay[RA_RECLAIM] > block_threshold))) {
		print_reclaim_info(ra);
	}

	if (g_kswapd_trace_enable && stub == RA_RECLAIM && type == KSWAPD_RECLAIM)
		print_trace_info(ra);
}

void reclaimacct_tsk_init(struct task_struct *tsk)
{
	if (tsk)
		tsk->reclaim_acct = NULL;
}

/* Reinitialize in case parent's non-null pointer was duped */
void reclaimacct_init(void)
{
	reclaimacct_tsk_init(&init_task);
}

void reclaimacct_substage_start(enum reclaimacct_stubs stub, struct shrinker *shrinker)
{
	reclaimacct_trace_begin(stub_name[stub], shrinker ? shrinker->scan_objects : NULL, 0);

	if (!current->reclaim_acct)
		return;

	current->reclaim_acct->start[stub] = ktime_get_ns();
}

void reclaimacct_substage_end(enum reclaimacct_stubs stub, unsigned long freed,
	unsigned long scanned, struct shrinker *shrinker)
{
	reclaimacct_trace_end(stub_name[stub], shrinker ? shrinker->scan_objects : NULL, freed);

	if (!current->reclaim_acct)
		return;

	__reclaimacct_end(current->reclaim_acct, freed, scanned, stub, shrinker);
}

static void reclaimacct_directreclaim_end(struct reclaim_acct *ra)
{
	reclaimacct_free(ra, ra->reclaim_type);
	current->reclaim_acct = NULL;
}

static void reclaimacct_system_reclaim_end(struct reclaim_acct *ra)
{
	reclaimacct_free(ra, ra->reclaim_type);
}

void reclaimacct_start(enum ra_reclaim_type type)
{
	reclaimacct_trace_begin(reclaim_type_str[type], NULL, 0);

	if (g_reclaimacct_disable || g_reclaimacct_is_off)
		return;

	if (!current->reclaim_acct) {
		current->reclaim_acct =  reclaimacct_alloc(type);
		if (!current->reclaim_acct)
			return;
	}
	current->reclaim_acct->reclaim_type = type;
	current->reclaim_acct->start[RA_RECLAIM] = ktime_get_ns();

#ifdef CONFIG_SCHEDSTATS
	current->reclaim_acct->sum_sleep_runtime = current->se.statistics.sum_sleep_runtime;
	current->reclaim_acct->wait_sum = current->se.statistics.wait_sum;
#endif
}

/* The caller should make sure start, total, count and func are not NULL */
void reclaimacct_end(enum ra_reclaim_type type)
{
	reclaimacct_trace_end(reclaim_type_str[type], NULL, 0);

	if (!current->reclaim_acct)
		return;

#ifdef CONFIG_SCHEDSTATS
	current->reclaim_acct->sum_sleep_runtime = current->se.statistics.sum_sleep_runtime -
		current->reclaim_acct->sum_sleep_runtime;
	current->reclaim_acct->wait_sum = current->se.statistics.wait_sum -
		current->reclaim_acct->wait_sum;
#endif

	__reclaimacct_end(current->reclaim_acct, 0, 0, RA_RECLAIM, NULL);

	reclaimacct_collect_data();

	reclaimacct_collect_reclaim_efficiency();

	if (is_system_reclaim(type))
		reclaimacct_system_reclaim_end(current->reclaim_acct);
	else
		reclaimacct_directreclaim_end(current->reclaim_acct);
}

void reclaimacct_destroy(void)
{
	if (!current->reclaim_acct)
		return;

	if (!is_system_reclaim(current->reclaim_acct->reclaim_type))
		return;

	kfree(current->reclaim_acct);
	current->reclaim_acct = NULL;
}

/* Reclaim accounting module initialize */
static int reclaimacct_init_handle(void *p)
{
	int i;
	int alloc_cnt;
#ifdef CONFIG_LOG_EXCEPTION
	unsigned int beta_flag;
#else
	unsigned int beta_flag = BETA_USER;
#endif

#ifdef CONFIG_LOG_EXCEPTION
	/* Try 60 times, wait 120s at most */
	for (i = 0; i < 60; i++) {
		beta_flag = get_logusertype_flag();
		/* Non-zero value means it is initialized */
		if (beta_flag != 0)
			break;
		/* Sleep 2 seconds */
		msleep(2000);
	}
#endif

	/* Enable in china and oversea beta version */
	if (beta_flag != BETA_USER && beta_flag != OVERSEA_USER) {
		pr_err("non-beta user\n");
		goto reclaimacct_disabled;
	}

	/* Init only in non-beta version to save memory */
	if (!reclaimacct_initialize_show_data())
		goto alloc_show_failed;

	alloc_cnt = 0; /* For safe */
	for (i = 0; i < NR_POOLMEMBER; i++) {
		g_mempool[i] = kzalloc(sizeof(struct reclaim_acct),
				       GFP_KERNEL);
		if (!g_mempool[i]) {
			alloc_cnt = i;
			goto alloc_acct_failed;
		}
	}

	g_reclaimacct_is_off = false;
	pr_info("reclaimacct enabled\n");
	return 0;

alloc_acct_failed:
	for (i = 0; i < alloc_cnt; i++) {
		kfree(g_mempool[i]);
		g_mempool[i] = NULL;
	}
	reclaimacct_destroy_show_data();
alloc_show_failed:
reclaimacct_disabled:
	g_reclaimacct_is_off = true;
	pr_err("reclaimacct disabled\n");
	return 0;
}

static int __init reclaimacct_module_init(void)
{
	struct task_struct *task = NULL;

	task = kthread_run(reclaimacct_init_handle, NULL, "reclaimacct_init");
	if (IS_ERR(task))
		pr_err("run reclaimacct_init failed\n");
	else
		pr_info("run reclaimacct_init successfully\n");
	return 0;
}

late_initcall(reclaimacct_module_init);

static int reclaimacct_disable_set(const char *val, const struct kernel_param *kp)
{
	int ret;

	ret = param_set_int(val, kp);
	if (ret)
		return ret;

	if (!g_reclaimacct_disable)
		reclaimacct_reinitialize_show_data();
	return 0;
}

static const struct kernel_param_ops reclaimacct_disable_ops = {
	.set = reclaimacct_disable_set,
	.get = param_get_int,
};

static int kswapd_delay_threshold_set(const char *val, const struct kernel_param *kp)
{
	int ret;
	int ori_threshold = g_kswapd_delay_threshold;

	ret = param_set_int(val, kp);
	if (ret)
		return ret;

	if (g_kswapd_delay_threshold < 0)
		g_kswapd_delay_threshold = ori_threshold;

	return 0;
}

static const struct kernel_param_ops kswapd_delay_threshold_ops = {
	.set = kswapd_delay_threshold_set,
	.get = param_get_int,
};

module_param_cb(reclaimacct_disable, &reclaimacct_disable_ops, &g_reclaimacct_disable, 0644);
module_param_cb(kswapd_delay_threshold, &kswapd_delay_threshold_ops, &g_kswapd_delay_threshold, 0644);
module_param_named(reclaim_trace_enable, g_reclaim_trace_enable, int, 0644);
module_param_named(kswapd_trace_enable, g_kswapd_trace_enable, int, 0644);
