/*
 * tick_mark.c
 *
 * SR Duration
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include "pm.h"
#include "pub_def.h"
/* platform/common/lowpm/pm_common.h */
#include "pm_common.h"
#include "helper/log/lowpm_log.h"
#include "helper/register/register_ops.h"
#include "helper/dtsi/dtsi_ops.h"
#include "helper/debugfs/debugfs.h"

#include <pm_def.h>
#include <m3_rdr_ddr_map.h>
#include <securec.h>

#include <linux/pm.h>
#include <linux/init.h>
#include <linux/suspend.h>
#include <linux/syscore_ops.h>
#include <linux/platform_device.h>

#include <base/power/power.h>

#ifdef CONFIG_SR_TRACE_POINT
#ifndef SOC_SCTRL_SCPERSTAT_POR_RESERVED0_ADDR
#define SOC_SCTRL_SCPERSTAT_POR_RESERVED0_ADDR(base)	((base) + (0xE10UL))
#endif
static void trace_point(unsigned int point)
{
	write_reg(SCTRL, SR_TRACE_REG(0), point);
}
#else
static inline void trace_point(unsigned int point)
{
	no_used(point);
}
#endif

#define BYTES_PER_TICK_POS 4
#define _runtime_read(offset) runtime_read(M3_RDR_SYS_CONTEXT_LPMCU_STAT_OFFSET + (offset))
#define read_tickmark(offset) _runtime_read((offset) * BYTES_PER_TICK_POS)

#define TICK_PER_SECOND 32768
#define US_PER_SECOND 1000000
#define US_PER_TICK (US_PER_SECOND / TICK_PER_SECOND)

#define get_tickmark_diff(start, end) ((read_tickmark(end) > read_tickmark(start)) ? \
		((read_tickmark(end) - read_tickmark(start))* US_PER_TICK) : 0)
static int tickmark_show(struct seq_file *s, void *data)
{
	int i;
	const int ticks_per_line = 10;

	no_used(data);

	if (is_runtime_base_inited() != 0) {
		lp_msg(s, "runtime base not init");
		return -EINVAL;
	}

	lp_msg(s, "lpm suspend cost %d us: begin tick:0x%x, end tick:0x%x.",
		(int)(get_tickmark_diff(TICK_SYS_SUSPEND_ENTRY, TICK_DEEPSLEEP_WFI_ENTER)),
		read_tickmark(TICK_SYS_SUSPEND_ENTRY),
		read_tickmark(TICK_DEEPSLEEP_WFI_ENTER));

	lp_msg(s, "lpm resume cost %d us: begin tick:0x%x, end tick:0x%x.",
		(int)(get_tickmark_diff(TICK_SYS_WAKEUP, TICK_SYS_WAKEUP_END)),
		read_tickmark(TICK_SYS_WAKEUP),
		read_tickmark(TICK_SYS_WAKEUP_END));
#if defined(LPMCU_TICKMARK_MODEM_DRX_PROTECT)
	lp_msg(s, "lpm resume before modem init cost %d us: begin tick:0x%x, end tick:0x%x.",
		(int)(get_tickmark_diff(TICK_SYS_WAKEUP, TICK_MODEM_EARLY_INIT_ENTRY)),
		read_tickmark(TICK_SYS_WAKEUP),
		read_tickmark(TICK_MODEM_EARLY_INIT_ENTRY));
	lp_msg(s, "lpm 32k enter cost %d us: begin tick:0x%x, end tick:0x%x.",
		(int)(get_tickmark_diff(TICK_32K_ENTER_ENTRY, TICK_32K_ENTER_END)),
		read_tickmark(TICK_32K_ENTER_ENTRY),
		read_tickmark(TICK_32K_ENTER_END));
	lp_msg(s, "lpm 32k exit cost %d us: begin tick:0x%x, end tick:0x%x.",
		(int)(get_tickmark_diff(TICK_32K_EXIT_ENTRY, TICK_32K_EXIT_END)),
		read_tickmark(TICK_32K_EXIT_ENTRY),
		read_tickmark(TICK_32K_EXIT_END));
#endif
	lp_msg(s, "lpm sr tick mark: ");
	for (i = 0; i < TICK_MARK_END_FLAG; i++) {
		lp_msg_cont(s, "[%02d]0x%08x ", i, read_tickmark(i));
		if ((i + 1) % ticks_per_line == 0)
			lp_msg_cont(s, "\n");
	}
	lp_msg_cont(s, "\n");

	return 0;
}

/* new plat add smoke protect, be careful when modify */
#if defined(LPMCU_TICKMARK_MODEM_DRX_PROTECT)
static int g_suspend_timeout = 5000;
static int g_resume_timeout = 5500;
static int g_sleep_enter_timeout = 400;
static int g_sleep_exit_timeout = 600;

enum tickmark_error_type {
	SUSPEND_TIMEOUT,
	RESUME_TIMEOUT,
	SLEEP_ENTER_TIMEOUT,
	SLEEP_EXIT_TIMEOUT,
	TICKMARK_RETURN_MAX,
};

static int tickmark_show_smoke_show(struct seq_file *s, void *data)
{
	int smoke_result = 0;
	int suspend_cost;
	int resume_cost;
	int sleep_enter_cost;
	int sleep_exit_cost;

	no_used(data);

	suspend_cost = (int)(get_tickmark_diff(TICK_SYS_SUSPEND_ENTRY, TICK_DEEPSLEEP_WFI_ENTER));
	resume_cost = (int)(get_tickmark_diff(TICK_SYS_WAKEUP, TICK_MODEM_EARLY_INIT_ENTRY));
	sleep_enter_cost = (int)(get_tickmark_diff(TICK_32K_ENTER_ENTRY, TICK_32K_ENTER_END));
	sleep_exit_cost = (int)(get_tickmark_diff(TICK_32K_EXIT_ENTRY, TICK_32K_EXIT_END));

	if (suspend_cost > g_suspend_timeout )
		smoke_result |= (1 << SUSPEND_TIMEOUT);

	if (resume_cost > g_resume_timeout)
		smoke_result |= (1 << RESUME_TIMEOUT);

	if (sleep_enter_cost > g_sleep_enter_timeout)
		smoke_result |= (1 << SLEEP_ENTER_TIMEOUT);

	if (sleep_exit_cost > g_sleep_exit_timeout)
		smoke_result |= (1 << SLEEP_EXIT_TIMEOUT);

	lp_msg(s, "%d, %d[%d], %d[%d], %d[%d], %d[%d]", smoke_result, suspend_cost, g_suspend_timeout,
			resume_cost, g_resume_timeout, sleep_enter_cost, g_sleep_enter_timeout,
			sleep_exit_cost, g_sleep_exit_timeout);

	return 0;
}
#define INPUT_BUF_LEN 64
static ssize_t tickmark_show_smoke_store(struct seq_file *s, const char __user *buffer,
			     size_t count, loff_t *ppos)
{
	int suspend_timeout;
	int resume_timeout;
	int sleep_enter_timeout;
	int sleep_exit_timeout;

	no_used(s);

	const int correct_columns = 4;

	char input_buf[INPUT_BUF_LEN] = {0};

	if (count >= INPUT_BUF_LEN) {
		lp_err("the input content is too long");
		return -EINVAL;
	}

	if (buffer != NULL && copy_from_user(input_buf, buffer, count) != 0) {
		lp_err("internal error: copy mem fail");
		return -ENOMEM;
	}

	input_buf[count] = '\0';
	suspend_timeout = 0;
	resume_timeout = 0;

	if (sscanf_s(input_buf, "%d, %d, %d, %d", &suspend_timeout, &resume_timeout,
				&sleep_enter_timeout, &sleep_exit_timeout) != correct_columns) {
		lp_err("invalid input");
		return -EINVAL;
	}

	*ppos += count;
	g_suspend_timeout = suspend_timeout;
	g_resume_timeout = resume_timeout;
	g_sleep_enter_timeout = sleep_enter_timeout;
	g_sleep_exit_timeout = sleep_exit_timeout;

	lp_info("change sr suspend timeout[%d]us resume timeout[%d]us sleep_enter timeout[%d]us sleep exit timeout[%d]us",
			g_suspend_timeout, g_resume_timeout, g_sleep_enter_timeout, g_sleep_exit_timeout);

	return count;
}
#endif

static int tickmark_suspend(void)
{
	trace_point(700);

	return 0;
}

static int tickmark_resume(void)
{
	trace_point(8800);

	return tickmark_show(NO_SEQFILE, NULL);
}

static struct sr_mntn g_sr_mntn_tickmark = {
	.owner = "tick_mark",
	.enable = true,
	.suspend = tickmark_suspend,
	.resume = tickmark_resume,
};

static const struct lowpm_debugdir g_lpwpm_debugfs_tickmark = {
	.dir = "lowpm_func",
	.files = {
		{"tick_mark", 0400, tickmark_show, NULL},
/* new plat add smoke protect, be careful when modify */
#if defined(LPMCU_TICKMARK_MODEM_DRX_PROTECT)
		{"tick_mark_smoke", 0600, tickmark_show_smoke_show, tickmark_show_smoke_store},
#endif
		{},
	},
};

static int sr_tick_pm_notify(struct notifier_block *nb, unsigned long mode, void *_unused)
{
	no_used(_unused);

	switch (mode) {
	case PM_SUSPEND_PREPARE:
		trace_point(66);
		pmu_write_sr_tick(KERNEL_SUSPEND_PREPARE);
		lp_crit("ap suspend start");
		break;
	case PM_POST_SUSPEND:
		trace_point(8999);
		pmu_write_sr_tick(KERNEL_RESUME_OUT);
		lp_crit("ap resume end");
		break;
	default:
		break;
	}

	return 0;
}

static struct notifier_block sr_tick_pm_nb = {
	.notifier_call = sr_tick_pm_notify,
};

static __init int init_sr_mntn_tickmark(void)
{
	int ret;

	if (sr_unsupported())
		return 0;

	ret = init_runtime_base();
	if (ret != 0) {
		lp_err("init runtime base failed");
		return ret;
	}

	ret = lowpm_create_debugfs(&g_lpwpm_debugfs_tickmark);
	if (ret != 0) {
		lp_err("create debug sr file failed");
		return ret;
	}

	ret = register_pm_notifier(&sr_tick_pm_nb);
	if (ret != 0) {
		lp_err("register_pm_notifier failed");
		return -ENODEV;
	}

	ret = register_sr_mntn(&g_sr_mntn_tickmark, SR_MNTN_PRIO_L);
	if (ret != 0) {
		lp_err("register mntn module failed");
		return ret;
	}

	if (map_reg_base(SCTRL) != 0) {
		lp_err("map sctrl reg failed");
		return -ENODATA;
	}

	lp_crit("success");

	return 0;
}

late_initcall(init_sr_mntn_tickmark);

#ifdef CONFIG_SR_TRACE_POINT
static int tickmark_syscore_suspend(void)
{
	trace_point(600);
	lp_crit("++");
	return 0;
}
static void tickmark_syscore_resume(void)
{
	trace_point(8900);
	lp_crit("--");
}

static struct syscore_ops tickmark_syscore_ops = {
	.suspend = tickmark_syscore_suspend,
	.resume = tickmark_syscore_resume,
};

static int sr_tickmark_prepare(struct device *dev)
{
	no_used(dev);

	trace_point(100);
	lp_crit("");
	return 0;
}
static int sr_tickmark_suspend(struct device *dev)
{
	no_used(dev);

	trace_point(200);
	lp_crit("");
	return 0;
}
static int sr_tickmark_suspend_late(struct device *dev)
{
	no_used(dev);

	trace_point(300);
	lp_crit("");
	return 0;
}
static int sr_tickmark_suspend_noirq(struct device *dev)
{
	no_used(dev);

	trace_point(400);
	lp_crit("");
	return 0;
}
static int sr_tickmark_resume_noirq(struct device *dev)
{
	no_used(dev);

	trace_point(8700);
	lp_crit("");
	return 0;
}
static int sr_tickmark_resume_early(struct device *dev)
{
	no_used(dev);

	trace_point(8600);
	lp_crit("");
	return 0;
}
static int sr_tickmark_resume(struct device *dev)
{
	no_used(dev);

	trace_point(8500);
	lp_crit("");
	return 0;
}
static void sr_tickmark_complete(struct device *dev)
{
	no_used(dev);

	trace_point(8400);
	lp_crit("");
}

static struct dev_pm_domain g_sr_tickmark_pm_domain = {
	{
		.prepare = sr_tickmark_prepare,
		.suspend  = sr_tickmark_suspend,
		.suspend_late = sr_tickmark_suspend_late,
		.suspend_noirq  = sr_tickmark_suspend_noirq,
		.resume_noirq = sr_tickmark_resume_noirq,
		.resume_early  = sr_tickmark_resume_early,
		.resume = sr_tickmark_resume,
		.complete  = sr_tickmark_complete,
	},
};
static struct device g_sr_tickmark_dev = {
	.init_name = "sr_tickmark",
	.pm_domain = &g_sr_tickmark_pm_domain,
};

static __init int init_sr_tickmark_dev(void)
{
	/* NOTE: devices_kset is NULL which used in device_initialize */
	device_initialize(&g_sr_tickmark_dev);
	device_pm_add(&g_sr_tickmark_dev);

	register_syscore_ops(&tickmark_syscore_ops);

	lp_crit("success");

	return 0;
}
early_initcall(init_sr_tickmark_dev);
#endif
