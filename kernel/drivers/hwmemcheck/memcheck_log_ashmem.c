/*
 * memcheck_log_ashmem.c
 *
 * Get ashmem info function
 *
 * Copyright(C) 2022 Huawei Technologies Co., Ltd. All rights reserved.
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
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/fdtable.h>
#include <linux/printk.h>
#include <linux/sched/task.h>
#include <linux/sched/signal.h>
#include <linux/version.h>
#include "ashmem.h"
#include "memcheck_ioctl.h"

#define MAX_ASHMEM_SIZE (800 * 1024 * 1024)

struct ashmem_info_args {
	struct seq_file *seq;
	struct task_struct *task;
	size_t sum;
};

static int ashmem_info_cb(const void *data, struct file *f, unsigned int fd)
{
	struct ashmem_info_args *args = (struct ashmem_info_args *)data;
	struct task_struct *task = args->task;
	size_t size;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
	if (!is_ashmem_file(f))
#else
	if (!is_ashmem(f))
#endif
		return 0;

	size = get_ashmem_size_by_file(f);
	args->sum += size;
	if (args->seq)
		seq_printf(args->seq, "%s %u %u %s %zu %u\n", task->comm, task->pid,
			   fd, get_ashmem_name_by_file(f), file_inode(f)->i_ino, size);
	else
		memcheck_info("%s %u %u %s %zu %u\n", task->comm, task->pid, fd,
			get_ashmem_name_by_file(f), file_inode(f)->i_ino, size);

	return 0;
}

static int ashmem_info_show(struct seq_file *s, void *d)
{
	struct task_struct *task = NULL;
	struct ashmem_info_args cb_args;
	unsigned long old_ns = ktime_get();

	if (s) {
		seq_puts(s, "Process ashmem info:\n");
		seq_puts(s, "----------------------------------------------------\n");
		seq_printf(s, "%s %s %s %s %s %s\n",
			   "Process_name", "Process_ID",
			   "fd", "ashmem_name", "inode", "size");
	} else {
		memcheck_info("Process ashmem info:\n");
		memcheck_info("----------------------------------------------------\n");
		memcheck_info("%s %s %s %s %s %s\n",
			"Process_name", "Process_ID",
			"fd", "ashmem_name", "inode", "size");
	}

	ashmem_mutex_lock();
	rcu_read_lock();
	for_each_process(task) {
		if (task->flags & PF_KTHREAD)
			continue;

		cb_args.sum = 0;
		cb_args.seq = s;
		cb_args.task = task;

		task_lock(task);
		iterate_fd(task->files, 0, ashmem_info_cb, (void *)&cb_args);
		if (!s && cb_args.sum)
			memcheck_info("Total ashmem usage for %s is %zu\n",
				task->comm, cb_args.sum);
		task_unlock(task);
	}
	rcu_read_unlock();
	ashmem_mutex_unlock();

	if (s)
		seq_puts(s, "----------------------------------------------------\n");
	else
		memcheck_info("----------------------------------------------------\n");

	memcheck_info("take %ld ns", ktime_get() - old_ns);

	return 0;
}

void memcheck_ashmem_info_show(void)
{
	u64 total = ashmem_get_total_size();

	memcheck_info("Total Ashmem usage is %llu\n", total);
	if (total < MAX_ASHMEM_SIZE)
		return;
	ashmem_info_show(NULL, NULL);
}
EXPORT_SYMBOL(memcheck_ashmem_info_show);

static int ashmem_info_open(struct inode *inode, struct file *file)
{
	return single_open(file, ashmem_info_show, inode->i_private);
}

#if (KERNEL_VERSION(5, 10, 0) <= LINUX_VERSION_CODE)
static const struct proc_ops ashmem_info_fops = {
	.proc_open = ashmem_info_open,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
};
#else
static const struct file_operations ashmem_info_fops = {
	.open = ashmem_info_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};
#endif

int memcheck_ashmem_createfs(void)
{
	struct proc_dir_entry *entry = NULL;

	entry = proc_create_data("ashmem_process_info", 0444,
		NULL, &ashmem_info_fops, (void *)0);
	if (!entry)
		memcheck_err("Failed to create ashmem debug info\n");

	return (!entry ? -EFAULT : 0);
}
