/*
 * alloc_acct_show.c
 *
 * Show memory alloc delay accounting data
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

#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/version.h>
#include <linux/alloc_acct.h>

static int alloc_acct_proc_show(struct seq_file *m, void *v)
{
	const char *stub_name[2] = {
		PGFAULT_STR,
		SPECULATIVE_PGFAULT_STR,
	};
	int i, j;
	u64 delay;
	u64 count;
	const u64 ns_to_ms = 1000000;

	seq_puts(m, "watch_point(unit:ms/-)\t\t0-1ms\t\t1-5ms\t\t5-10ms\t\t");
	seq_puts(m, "10-50ms\t\t50-100ms\t100-1000ms\t1000-20000ms\n");

	for (i = 0; i < NR_ALL_STUBS; i++) {
		if (i <= MAX_ALLOC_ORDER)
			seq_printf(m, "delay_order%-21u", i);
		else
			seq_printf(m, "delay_%-26s", stub_name[i - MAX_ALLOC_ORDER - 1]);
		for (j = 0; j < NR_DELAY_LV; j++) {
			delay = alloc_acct_get_data(AA_DELAY, j, i) / ns_to_ms;
			seq_printf(m, "%-15lu ", delay);
		}
		seq_puts(m, "\n");

		if (i <= MAX_ALLOC_ORDER)
			seq_printf(m, "count_order%-21u", i);
		else
			seq_printf(m, "count_%-26s", stub_name[i - MAX_ALLOC_ORDER - 1]);

		for (j = 0; j < NR_DELAY_LV; j++) {
			count = alloc_acct_get_data(AA_COUNT, j, i);
			seq_printf(m, "%-15lu ", count);
		}
		seq_puts(m, "\n");
	}
	return 0;
}

static int alloc_acct_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, alloc_acct_proc_show, NULL);
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0))
static const struct file_operations alloc_acct_proc_fops = {
	.open = alloc_acct_proc_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};
#else
static const struct proc_ops alloc_acct_proc_fops = {
	.proc_open = alloc_acct_proc_open,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
};
#endif


static int __init proc_alloc_acct_init(void)
{
	proc_create("alloc_acct", 0440, NULL, &alloc_acct_proc_fops);
	return 0;
}
fs_initcall(proc_alloc_acct_init);