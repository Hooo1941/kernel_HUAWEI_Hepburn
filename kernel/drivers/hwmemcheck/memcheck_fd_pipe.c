/*
 * memcheck_fd_pipe.c
 *
 * Pipe leak dectector
 *
 * Copyright(C) 2023 Huawei Technologies Co., Ltd. All rights reserved.
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
#include <linux/pipe_fs_i.h>
#include <linux/seq_file.h>
#include <linux/fdtable.h>
#include <linux/printk.h>
#include <linux/sched/task.h>
#include <linux/sched/signal.h>
#include <linux/version.h>
#include "memcheck_ioctl.h"

struct pipe_info_args {
	struct seq_file *seq;
	struct task_struct *task;
};

static int pipe_info_cb(const void *data, struct file *f, unsigned int fd)
{
	struct pipe_info_args *args = (struct pipe_info_args *)data;
	struct task_struct *task = args->task;
	struct pipe_inode_info *pipe = NULL;

	pipe = get_pipe_info(f, false);
	if (!pipe)
		return 0;

	seq_printf(args->seq, "%s %u %u %s %zu %u %u %u\n",
		   task->comm, task->pid, fd, f->f_path.dentry->d_iname, file_inode(f)->i_ino,
		   pipe->max_usage, pipe->nr_accounted, pipe->ring_size);

	return 0;
}

static int pipe_info_show(struct seq_file *s, void *d)
{
	struct task_struct *task = NULL;
	struct pipe_info_args cb_args;
	unsigned long old_ns = ktime_get();

	seq_puts(s, "Process pipe info:\n");
	seq_puts(s, "----------------------------------------------------\n");
	seq_printf(s, "%s %s %s %s %s %s %s %s\n", "ProcessName", "ProcessID", "Fd", "PipeName", "inode",
		   "MaxUsage", "NumAccounted", "RingSize");

	rcu_read_lock();
	for_each_process(task) {
		if (task->flags & PF_KTHREAD)
			continue;

		cb_args.seq = s;
		cb_args.task = task;

		task_lock(task);
		iterate_fd(task->files, 0, pipe_info_cb, (void *)&cb_args);
		task_unlock(task);
	}
	rcu_read_unlock();

	memcheck_info("take %ld ns", ktime_get() - old_ns);

	return 0;
}

static int pipe_info_open(struct inode *inode, struct file *file)
{
	return single_open(file, pipe_info_show, inode->i_private);
}

#if (KERNEL_VERSION(5, 10, 0) <= LINUX_VERSION_CODE)
static const struct proc_ops pipe_info_fops = {
	.proc_open = pipe_info_open,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
};
#else
static const struct file_operations pipe_info_fops = {
	.open = pipe_info_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};
#endif

int memcheck_fd_pipe_createfs(void)
{
	struct proc_dir_entry *entry = NULL;

	entry = proc_create_data("pipe_process_info", 0444,
		NULL, &pipe_info_fops, (void *)0);
	if (!entry)
		memcheck_err("Failed to create pipe debug info\n");

	return (!entry ? -EFAULT : 0);
}
