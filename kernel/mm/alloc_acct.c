/*
 * alloc_acct.c
 *
 * Alloc delay accounting
 *
 * Copyright (c) 2022-2022 Huawei Technologies Co., Ltd
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

#define pr_fmt(fmt) "alloc_acct: " fmt

#include <linux/alloc_acct.h>

#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/moduleparam.h>
#include <linux/mm.h>
#include <linux/sched/clock.h>
#include <linux/sched/task.h>
#include <linux/slab.h>
#include <linux/types.h>

#include <log/log_usertype.h>

/* Store alloc accounting data */
static struct alloc_acct_show {
	u64 delay[NR_DELAY_LV][NR_ALL_STUBS];
	u64 count[NR_DELAY_LV][NR_ALL_STUBS];
} *g_alloc_acct_show;

static DEFINE_SPINLOCK(g_alloc_acct_show_lock);

/* Once initialized, the variable should never be changed */
static int alloc_acct_is_off = 1;
static int alloc_acct_disable = 1;
static int alloc_acct_debug_disable = 1;

static const u64 ns_to_ms = 1000000;
static const u64 delay_level[NR_DELAY_LV] = {
	DELAY_LV0, DELAY_LV1, DELAY_LV2, DELAY_LV3, DELAY_LV4, DELAY_LV5, DELAY_LV6
};
void alloc_acct_allocpage_start(void)
{
	if (alloc_acct_disable || alloc_acct_is_off)
		return;
	current->alloc_start_time = ktime_get_ns();
}

void alloc_acct_pgfault_start(void)
{
	if (alloc_acct_disable || alloc_acct_debug_disable || alloc_acct_is_off)
		return;

	current->pgfault_start_time = ktime_get_ns();
}

void alloc_acct_spf_start(void)
{
	if (alloc_acct_disable || alloc_acct_debug_disable || alloc_acct_is_off)
		return;

	current->pgfault_start_time = ktime_get_ns();
}

static void alloc_page_data_collect(unsigned int order)
{
	int i;
	u64 alloc_delay;

	u64 now = ktime_get_ns();
	if (now < current->alloc_start_time)
		return;
	alloc_delay = now - current->alloc_start_time;
	if (alloc_delay > DELAY_LV4 * 5) { // 500ms
		pr_info("allocacct: allocation order %u cost %llu ms", order, alloc_delay / ns_to_ms);
		show_mem(SHOW_MEM_FILTER_NODES, NULL);
	}
	if (alloc_acct_debug_disable)
		return;
	if (!spin_trylock(&g_alloc_acct_show_lock))
		return;
	for (i = 0; i < NR_DELAY_LV; i++) {
		if (alloc_delay < delay_level[i]) {
			g_alloc_acct_show->delay[i][order] +=
				alloc_delay;
			g_alloc_acct_show->count[i][order] += 1;
			break;
		}
	}
	spin_unlock(&g_alloc_acct_show_lock);
}

static void pgfault_data_collect(unsigned int type)
{
	int i;
	u64 pgfault_delay;
	u64 now = ktime_get_ns();
	if (now < current->pgfault_start_time)
		return;
	pgfault_delay = now - current->pgfault_start_time;
	if (!spin_trylock(&g_alloc_acct_show_lock))
		return;
	for (i = 0; i < NR_DELAY_LV; i++) {
		if (pgfault_delay < delay_level[i]) {
			g_alloc_acct_show->delay[i][MAX_ALLOC_ORDER + 1 + type] +=
				pgfault_delay;
			g_alloc_acct_show->count[i][MAX_ALLOC_ORDER + 1 + type] += 1;
			break;
		}
	}
	spin_unlock(&g_alloc_acct_show_lock);
}

void alloc_acct_allocpage_end(unsigned int order)
{
	if (alloc_acct_disable || alloc_acct_is_off || current->alloc_start_time == 0)
		return;

	if (order > MAX_ALLOC_ORDER)
		order = MAX_ALLOC_ORDER;

	alloc_page_data_collect(order);
	current->alloc_start_time = 0;
}

void alloc_acct_pgfault_end(void)
{
	if (alloc_acct_disable || alloc_acct_is_off ||
		alloc_acct_debug_disable || current->pgfault_start_time == 0)
		return;

	pgfault_data_collect(AA_PGFAULT);
	current->pgfault_start_time = 0;
}

void alloc_acct_spf_end(void)
{
	if (alloc_acct_disable || alloc_acct_is_off ||
		alloc_acct_debug_disable || current->pgfault_start_time == 0)
		return;

	pgfault_data_collect(AA_SPECULATIVE_PGFAULT);
	current->pgfault_start_time = 0;
}

u64 alloc_acct_get_data(enum aa_show_type type, int level, int stub)
{
	u64 ret;

	if (alloc_acct_is_off || !g_alloc_acct_show)
		return 0;

	spin_lock(&g_alloc_acct_show_lock);
	switch (type) {
	case AA_DELAY:
		ret = g_alloc_acct_show->delay[level][stub];
		break;
	case AA_COUNT:
		ret = g_alloc_acct_show->count[level][stub];
		break;
	default: /* impossible */
		ret = 0;
		break;
	}
	spin_unlock(&g_alloc_acct_show_lock);
	return ret;
}

/* Alloc accounting module initialize */
static int alloc_acct_init_handle(void *p)
{
	int i;
	unsigned int beta_flag = BETA_USER;
#if defined(CONFIG_LOG_EXCEPTION) || defined(CONFIG_LOG_USERTYPE)
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
		goto alloc_acct_disabled;
	}

	/* Init only in beta version to save memory */
	g_alloc_acct_show = kzalloc(sizeof(struct alloc_acct_show),
				     GFP_KERNEL);
	if (!g_alloc_acct_show)
		goto alloc_acct_disabled;

	alloc_acct_is_off = 0;
	pr_info("alloc_acct enabled\n");
	return 0;

alloc_acct_disabled:
	alloc_acct_is_off = 1;
	pr_err("alloc_acct disabled\n");
	return 0;
}

static int __init alloc_acct_module_init(void)
{
	struct task_struct *task = NULL;

	task = kthread_run(alloc_acct_init_handle, NULL, "alloc_acct_init");
	if (IS_ERR(task))
		pr_err("run alloc_acct_init failed\n");
	else
		pr_info("run alloc_acct_init successfully\n");
	return 0;
}

late_initcall(alloc_acct_module_init);

module_param_named(alloc_acct_disable, alloc_acct_disable, int, 0644);
module_param_named(alloc_acct_debug_disable, alloc_acct_debug_disable, int, 0644);
