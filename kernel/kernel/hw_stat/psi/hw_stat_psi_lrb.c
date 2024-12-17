/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: hw_stat psi lrb_in_use
 * Author: Fan Weiwei <fanweiwei1@huawei.com>
 * Create: 2023-03-16
 */
#include "hw_stat_psi_lrb.h"
#include "hw_stat_psi.h"

#include <linux/bitmap.h>
#include <linux/bits.h>
#include <linux/hw_stat.h>
#include <linux/sched/loadavg.h>
#include <linux/seq_file.h>

static DEFINE_MUTEX(g_avgs_lock);

static struct hw_stat_psi_lrb_mngt g_psi_lrb_mngt = {
	.avgs_lock = &g_avgs_lock,
	.highload_count = ATOMIC64_INIT(0),
	.total_count = ATOMIC64_INIT(0),
	.last_highload_count = 0,
	.last_total_count = 0,
	.last_update_time = INITIAL_JIFFIES,
	.next_update_time = INITIAL_JIFFIES + HW_STAT_PSI_PERIOD,
	.lrb_psi = {0, 0, 0}
};

void hw_stat_lrb_in_use_changed(unsigned long lrb_in_use)
{
	const u32 lrb_in_use_highload = 28;

	atomic64_inc(&g_psi_lrb_mngt.total_count);
	if (bitmap_weight(&lrb_in_use, sizeof(lrb_in_use) * BITS_PER_BYTE) >= lrb_in_use_highload)
		atomic64_inc(&g_psi_lrb_mngt.highload_count);
}

void update_psi_lrb(void)
{
	u32 missed_periods = 0;
	u64 expires, now;
	unsigned long pct;
	u64 period_highload_count, period_total_count;
	const u64 percent_100 = 100;

	mutex_lock(g_psi_lrb_mngt.avgs_lock);
	now = jiffies_64;
	if (time_before(now, g_psi_lrb_mngt.next_update_time)) {
		mutex_unlock(g_psi_lrb_mngt.avgs_lock);
		return;
	}

	/* lrb_in_use may be unmodified for multiple periods */
	expires = g_psi_lrb_mngt.next_update_time;
	if (now - expires >= HW_STAT_PSI_PERIOD)
		missed_periods = div_u64(now - expires, HW_STAT_PSI_PERIOD);

	/* calc highload percent during last update_psi_lrb time to now */
	period_highload_count = atomic64_read(&g_psi_lrb_mngt.highload_count) -
		g_psi_lrb_mngt.last_highload_count;
	period_total_count = atomic64_read(&g_psi_lrb_mngt.total_count) -
		g_psi_lrb_mngt.last_total_count;
	if (period_total_count == 0) {
		pct = 0;
		period_highload_count = 0;
	} else if (period_highload_count >= period_total_count) {
		pct = percent_100 * FIXED_1;
		period_highload_count = period_total_count;
	} else {
		pct = div_u64(period_highload_count * percent_100, period_total_count);
		pct *= FIXED_1;
	}

	/* update psi */
	g_psi_lrb_mngt.lrb_psi[RECENT_10S] = calc_load_n(
		g_psi_lrb_mngt.lrb_psi[RECENT_10S], HW_STAT_EXP_10S, pct, missed_periods + 1);
	g_psi_lrb_mngt.lrb_psi[RECENT_60S] = calc_load_n(
		g_psi_lrb_mngt.lrb_psi[RECENT_60S], HW_STAT_EXP_60S, pct, missed_periods + 1);
	g_psi_lrb_mngt.lrb_psi[RECENT_300S] = calc_load_n(
		g_psi_lrb_mngt.lrb_psi[RECENT_300S], HW_STAT_EXP_300S, pct, missed_periods + 1);

	g_psi_lrb_mngt.last_highload_count += period_highload_count;
	g_psi_lrb_mngt.last_total_count += period_total_count;
	g_psi_lrb_mngt.last_update_time = now;
	g_psi_lrb_mngt.next_update_time = expires + ((1 + missed_periods) * HW_STAT_PSI_PERIOD);

	mutex_unlock(g_psi_lrb_mngt.avgs_lock);
}

static int psi_lrb_proc_show(struct seq_file *seq, void *p)
{
	update_psi_lrb();

	seq_printf(seq,
		"avg10=%lu.%02lu avg60=%lu.%02lu avg300=%lu.%02lu\n",
		LOAD_INT(g_psi_lrb_mngt.lrb_psi[RECENT_10S]),
		LOAD_FRAC(g_psi_lrb_mngt.lrb_psi[RECENT_10S]),
		LOAD_INT(g_psi_lrb_mngt.lrb_psi[RECENT_60S]),
		LOAD_FRAC(g_psi_lrb_mngt.lrb_psi[RECENT_60S]),
		LOAD_INT(g_psi_lrb_mngt.lrb_psi[RECENT_300S]),
		LOAD_FRAC(g_psi_lrb_mngt.lrb_psi[RECENT_300S]));
	seq_printf(seq,
		"highload_count=%llu total_count=%llu\n",
		g_psi_lrb_mngt.highload_count,
		g_psi_lrb_mngt.total_count);
	return 0;
}

void hw_stat_psi_lrb_init(struct proc_dir_entry *parent)
{
	proc_create_single("psi_lrb", S_IRUSR, parent, psi_lrb_proc_show);
}