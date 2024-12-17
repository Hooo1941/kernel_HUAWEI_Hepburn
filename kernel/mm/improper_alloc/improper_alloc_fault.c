// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023. All rights reserved.
 *
 * Description: Debug improper memory alloc behaviours
 * Author: Li Xilun <lixilun1@huawei.com>
 * Create: 2023-03-23
 */

#define pr_fmt(fmt) "improper_alloc_fault: " fmt

#include <improper_alloc_fault.h>

#include <linux/delay.h>
#include <linux/init.h>
#include <linux/jiffies.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/stddef.h>

#define FAULT_INJECT_INTERVAL 5

int g_fault_thread_running = 0;
static struct task_struct *g_fault_thread;

static void kmalloc_fault(size_t size, gfp_t gfp_mask, int *failures)
{
	void *mem = kmalloc(size, gfp_mask);

	if (!mem)
		(*failures)++;
	else
		kfree(mem);
}

static void alloc_pages_fault(gfp_t gfp_mask, unsigned int order, int *failures)
{
	struct page *page = alloc_pages(gfp_mask, order);

	if (!page)
		(*failures)++;
	else
		__free_pages(page, order);
}

static void inject_faults(void)
{
	int failures = 0;

	/* high order fault inject */
	alloc_pages_fault(GFP_KERNEL, 3, &failures);

	/* atomic fault inject */
	alloc_pages_fault(GFP_ATOMIC, 0, &failures);
	kmalloc_fault(64, GFP_ATOMIC, &failures);

	/* large slab fault inject, 48576 bytes will be wasted */
	kmalloc_fault(1000000, GFP_KERNEL, &failures);

	pr_err("%d faults inject failed\n", failures);
}

static int fault_inject_thread(void *data)
{
	while (!kthread_should_stop()) {
		set_current_state(TASK_INTERRUPTIBLE);
		schedule_timeout(FAULT_INJECT_INTERVAL * HZ);

		inject_faults();
	}

	return 0;
}

int improper_fault_handler(struct ctl_table *ctl, int write,
			   void *buffer, size_t *lenp, loff_t *ppos)
{
	int ret = proc_dointvec_minmax(ctl, write, buffer, lenp, ppos);

	if (!write)
		return ret;

	if (g_fault_thread_running && !g_fault_thread) {
		g_fault_thread = kthread_run(fault_inject_thread, NULL, "improper_fault");
		if (!IS_ERR(g_fault_thread)) {
			pr_info("fault_inject_thread started successfully\n");
		} else {
			pr_err("fault_inject_thread started failed\n");
			g_fault_thread = NULL;
			g_fault_thread_running = 0;
		}
	} else if (!g_fault_thread_running && g_fault_thread) {
		kthread_stop(g_fault_thread);
		g_fault_thread = NULL;
		pr_info("fault_inject_thread stopped successfully\n");
	}

	return ret;
}
