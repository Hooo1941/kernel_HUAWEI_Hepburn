/*
 * reclaimacct_show.c
 *
 * Show memory reclaim delay accounting data
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

#include "reclaimacct_show.h"

#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/slab.h>
#include <linux/time64.h>
#include <linux/version.h>
#include <linux/sched/clock.h>

#include <securec.h>

#include <chipset_common/reclaim_acct/reclaim_acct.h>
#include "internal.h"

/* Store reclaim accounting data */
static struct reclaimacct_show {
	u64 delay[NR_DELAY_LV][NR_RA_STUBS];
	u64 count[NR_DELAY_LV][NR_RA_STUBS];
	u64 freed[NR_DELAY_LV][NR_RA_STUBS];
	u64 scanned[NR_DELAY_LV][NR_RA_STUBS];
	unsigned long (*scan_objects)(struct shrinker *, struct shrink_control *sc);
	u64 shrinker_delay_max;
	u64 delay_max;
	u64 delay_max_t;
} *g_reclaimacct_show;
static DEFINE_SPINLOCK(g_reclaimacct_show_lock);

static struct reclaim_efficiency {
	u64 time[NR_RA_STUBS];
	u64 freed[NR_RA_STUBS];
	u64 scanned[NR_RA_STUBS];
} *g_reclaim_effi;
static DEFINE_SPINLOCK(g_reclaim_effi_lock);

bool reclaimacct_initialize_show_data(void)
{
	g_reclaimacct_show = kzalloc(sizeof(struct reclaimacct_show) *
		RECLAIM_TYPES, GFP_KERNEL);
	if (!g_reclaimacct_show)
		goto fail_show;

	g_reclaim_effi = kzalloc(sizeof(struct reclaim_efficiency) *
		RECLAIM_TYPES, GFP_KERNEL);
	if (!g_reclaim_effi)
		goto fail_effi;
	return true;

fail_effi:
	kfree(g_reclaimacct_show);
	g_reclaimacct_show = NULL;

fail_show:
	return false;
}

void reclaimacct_reinitialize_show_data(void)
{
	if (g_reclaimacct_show)
		(void)memset_s(g_reclaimacct_show, sizeof(struct reclaimacct_show) * RECLAIM_TYPES,
			0, sizeof(struct reclaimacct_show) * RECLAIM_TYPES);

	if (g_reclaim_effi)
		(void)memset_s(g_reclaim_effi, sizeof(struct reclaim_efficiency) * RECLAIM_TYPES,
			0, sizeof(struct reclaim_efficiency) * RECLAIM_TYPES);
}

void reclaimacct_destroy_show_data(void)
{
	kfree(g_reclaimacct_show);
	g_reclaimacct_show = NULL;

	kfree(g_reclaim_effi);
	g_reclaim_effi = NULL;
}

static void __reclaimacct_collect_data(int level, struct reclaim_acct *ra)
{
	int i;
	int type = ra->reclaim_type;

	spin_lock(&g_reclaimacct_show_lock);
	for (i = 0; i < NR_RA_STUBS; i++) {
		g_reclaimacct_show[type].delay[level][i] += ra->delay[i];
		g_reclaimacct_show[type].count[level][i] += ra->count[i];
		g_reclaimacct_show[type].freed[level][i] += ra->freed[i];
		g_reclaimacct_show[type].scanned[level][i] += ra->scanned[i];
	}

	if (ra->shrinker_delay_max > g_reclaimacct_show[type].shrinker_delay_max) {
		g_reclaimacct_show[type].scan_objects = ra->scan_objects;
		g_reclaimacct_show[type].shrinker_delay_max = ra->shrinker_delay_max;
	}

	if (ra->delay[RA_RECLAIM] > g_reclaimacct_show[type].delay_max) {
		g_reclaimacct_show[type].delay_max = ra->delay[RA_RECLAIM];
#ifdef CONFIG_HISI_TIME
		g_reclaimacct_show[type].delay_max_t = hisi_getcurtime();
#else
		g_reclaimacct_show[type].delay_max_t = sched_clock();
#endif
	}
	spin_unlock(&g_reclaimacct_show_lock);
}

void reclaimacct_collect_data(void)
{
	int i;
	const u64 delay[NR_DELAY_LV] = {
		DELAY_LV0, DELAY_LV1, DELAY_LV2, DELAY_LV3, DELAY_LV4, DELAY_LV5
	};

	if (!g_reclaimacct_show || !current->reclaim_acct)
		return;

	for (i = 0; i < NR_DELAY_LV; i++) {
		if (current->reclaim_acct->delay[RA_RECLAIM] < delay[i]) {
			__reclaimacct_collect_data(i, current->reclaim_acct);
			break;
		}
	}
}

static void reclaimacct_show_info(struct seq_file *m, void *v, struct reclaimacct_show *show)
{
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
	int i, j, k;

	seq_puts(m, "watch_point(unit:ms/-)\t\t0-5ms\t\t5-10ms\t\t");
	seq_puts(m, "10-50ms\t\t50-100ms\t100-2000ms\t2000-50000ms\n");

	for (k = 0; k < RECLAIM_TYPES; k++) {
		stub_name[0] = reclaim_type_str[k];
		for (i = 0; i < NR_RA_STUBS; i++) {
			/* k/zswapd and kshrinkd do not contain drain_all_pages() */
			if ((i == RA_DRAINALLPAGES) && is_system_reclaim(k))
				continue;

			seq_printf(m, "%s_delay\t\t", stub_name[i]);
			for (j = 0; j < NR_DELAY_LV; j++)
				seq_printf(m, "%-15lu ", show[k].delay[j][i] / NSEC_PER_MSEC);
			seq_puts(m, "\n");

			seq_printf(m, "%s_count\t\t", stub_name[i]);
			for (j = 0; j < NR_DELAY_LV; j++)
				seq_printf(m, "%-15lu ", show[k].count[j][i]);
			seq_puts(m, "\n");

			seq_printf(m, "%s_freed\t\t", stub_name[i]);
			for (j = 0; j < NR_DELAY_LV; j++)
				seq_printf(m, "%-15lu ", show[k].freed[j][i]);
			seq_puts(m, "\n");

			seq_printf(m, "%s_scanned\t\t", stub_name[i]);
			for (j = 0; j < NR_DELAY_LV; j++)
				seq_printf(m, "%-15lu ", show[k].scanned[j][i]);
			seq_puts(m, "\n");
		}

		if (show[k].scan_objects)
			seq_printf(m, "shrinker: %pS shrinker_delay_max:%lu",
				show[k].scan_objects, show[k].shrinker_delay_max);

		seq_printf(m, " Max delay:%lu Happened:%lu\n", show[k].delay_max,
			show[k].delay_max_t);
		seq_printf(m, "-------------------------------------------\n");
	}
}

static int reclaimacct_proc_show(struct seq_file *m, void *v)
{
	struct reclaimacct_show show[RECLAIM_TYPES];

	if (!g_reclaimacct_show)
		return 0;

	spin_lock(&g_reclaimacct_show_lock);
	(void)memcpy_s(&show, sizeof(show), g_reclaimacct_show, sizeof(show));
	spin_unlock(&g_reclaimacct_show_lock);

	reclaimacct_show_info(m, v, show);

	return 0;
}

static int reclaimacct_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, reclaimacct_proc_show, NULL);
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0))
static const struct file_operations reclaimacct_proc_fops = {
	.open = reclaimacct_proc_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
	};
#else
static const struct proc_ops reclaimacct_proc_fops = {
	.proc_open = reclaimacct_proc_open,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
};
#endif

static int kswapdacct_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, reclaimacct_proc_show, NULL);
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0))
static const struct file_operations kswapdacct_proc_fops = {
	.open = kswapdacct_proc_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};
#else
static const struct proc_ops kswapdacct_proc_fops = {
	.proc_open = kswapdacct_proc_open,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
};
#endif

static void __reclaimacct_collect_reclaim_efficiency(
	struct reclaim_acct *ra, enum ra_reclaim_type type)
{
	int i;

	ra->freed[RA_RECLAIM] = ra->freed[RA_SHRINKFILE] + ra->freed[RA_SHRINKANON];
	ra->scanned[RA_RECLAIM] = ra->scanned[RA_SHRINKFILE] + ra->scanned[RA_SHRINKANON];

	/* system_reclaim(kswapd/zswapd) is single thread, do not need lock */
	if (!is_system_reclaim(type))
		spin_lock(&g_reclaim_effi_lock);

	for (i = 0; i < NR_RA_STUBS; i++) {
		g_reclaim_effi[type].time[i] += ra->delay[i];
		g_reclaim_effi[type].freed[i] += ra->freed[i];
		g_reclaim_effi[type].scanned[i] += ra->scanned[i];
	}

	if (!is_system_reclaim(type))
		spin_unlock(&g_reclaim_effi_lock);
}

void reclaimacct_collect_reclaim_efficiency(void)
{
	if (!g_reclaim_effi || !current->reclaim_acct)
		return;

	__reclaimacct_collect_reclaim_efficiency(current->reclaim_acct,
		current->reclaim_acct->reclaim_type);
}

static int reclaim_efficiency_proc_show(struct seq_file *m, void *v)
{
	int i;
	int j;
	struct reclaim_efficiency effi[RECLAIM_TYPES];
	const char *stage[NR_RA_STUBS] = {
		"total_process",
		"drain_pages  ",
		"shrink_file  ",
		"shrink_anon  ",
		"shrink_slab  ",
		"soft_limit   "
	};
	const char *type[RECLAIM_TYPES] = {
		"direct reclaim",
		"kswapd        ",
		"zswapd        ",
		"kshrinkd      "
	};

	if (!g_reclaim_effi)
		return 0;

	spin_lock(&g_reclaim_effi_lock);
	(void)memcpy_s(&effi, sizeof(effi), g_reclaim_effi, sizeof(effi));
	spin_unlock(&g_reclaim_effi_lock);

	for (i = 0; i < RECLAIM_TYPES; i++) {
		seq_printf(m, "%s time(ms)\t\tscanned(page/obj)\t   ", type[i]);
		seq_printf(m, "freed(page/obj)\n");
		seq_printf(m, "%s  %-16llu %llu/%-24llu %llu/%-15llu\n", stage[0],
			effi[i].time[0] / NSEC_PER_MSEC,
			effi[i].scanned[0], effi[i].scanned[RA_SHRINKSLAB],
			effi[i].freed[0],  effi[i].freed[RA_SHRINKSLAB]);
		for (j = 1; j < NR_RA_STUBS; j++)
			seq_printf(m, "%s  %-16llu %-26llu %-15llu\n", stage[j],
				effi[i].time[j] / NSEC_PER_MSEC,
				effi[i].scanned[j],
				effi[i].freed[j]);
	}

	return 0;
}

static int reclaim_efficiency_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, reclaim_efficiency_proc_show, NULL);
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0))
static const struct file_operations reclaim_effi_proc_fops = {
	.open = reclaim_efficiency_proc_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};
#else
static const struct proc_ops reclaim_effi_proc_fops = {
	.proc_open = reclaim_efficiency_proc_open,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
};
#endif

static int __init proc_reclaimacct_init(void)
{
	proc_create("reclaimacct", 0440, NULL, &reclaimacct_proc_fops);
	proc_create("kswapdacct", 0440, NULL, &kswapdacct_proc_fops);
	proc_create("reclaim_efficiency", 0440, NULL, &reclaim_effi_proc_fops);
	return 0;
}
fs_initcall(proc_reclaimacct_init);
