/*
 * memcheck_fd_fence.c
 *
 * Fence leak detector
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

#include <linux/dma-fence.h>
#include <linux/dma-fence-array.h>
#include <linux/fdtable.h>
#include <linux/fs.h>
#include <linux/printk.h>
#include <linux/proc_fs.h>
#include <linux/sched/task.h>
#include <linux/sched/signal.h>
#include <linux/seq_file.h>
#include <linux/sync_file.h>
#include <linux/version.h>
#include <net/sock.h>
#include "memcheck_ioctl.h"

struct fence_info_args {
	struct seq_file *seq;
	struct task_struct *task;
};

int is_sync_file(struct file *f);

static int fence_info_cb(const void *data, struct file *f, unsigned int fd)
{
	struct fence_info_args *args = (struct fence_info_args *)data;
	struct task_struct *task = args->task;
	struct sync_file *sync_file = NULL;
	struct dma_fence **fences = NULL;
	int i;
	int num_fences;
	char *name = NULL;

	if (!is_sync_file(f))
		return 0;

	sync_file = (struct sync_file *)f->private_data;
	get_file(sync_file->file);
	if (dma_fence_is_array(sync_file->fence)) {
		struct dma_fence_array *array = to_dma_fence_array(sync_file->fence);

		num_fences = array->num_fences;
		fences = array->fences;
	} else {
		num_fences = 1;
		fences = &sync_file->fence;
	}
	if (strlen(sync_file->user_name))
		name = sync_file->user_name;
	else
		name = "NULL";
	seq_printf(args->seq, "%s %u %u %s %zu %u\n", task->comm, task->pid, fd, name, file_inode(f)->i_ino,
		   num_fences);
	for (i = 0; i < num_fences; i++)
		seq_printf(args->seq, "    %d:%s %s %d %zu\n", i, fences[i]->ops->get_timeline_name(fences[i]),
			   fences[i]->ops->get_driver_name(fences[i]), dma_fence_get_status(fences[i]),
			   ktime_to_ns(fences[i]->timestamp));
	fput(sync_file->file);

	return 0;
}

static int fence_info_show(struct seq_file *s, void *d)
{
	struct task_struct *task = NULL;
	struct fence_info_args cb_args;
	unsigned long old_ns = ktime_get();

	seq_puts(s, "Process fence info:\n");
	seq_puts(s, "----------------------------------------------------\n");
	seq_printf(s, "%s %s %s %s %s %s\n",
		   "ProcessName", "ProcessID", "Fd", "FenceName", "inode", "FenceNum\n");
	seq_printf(s, "    %s %s %s %s\n",
		   "TimelineName", "DriverName",  "Status", "Timestamp\n");

	rcu_read_lock();
	for_each_process(task) {
		if (task->flags & PF_KTHREAD)
			continue;

		cb_args.seq = s;
		cb_args.task = task;

		task_lock(task);
		iterate_fd(task->files, 0, fence_info_cb, (void *)&cb_args);
		task_unlock(task);
	}
	rcu_read_unlock();

	memcheck_info("take %ld ns", ktime_get() - old_ns);

	return 0;
}

static int fence_info_open(struct inode *inode, struct file *file)
{
	return single_open(file, fence_info_show, inode->i_private);
}

#if (KERNEL_VERSION(5, 10, 0) <= LINUX_VERSION_CODE)
static const struct proc_ops fence_info_fops = {
	.proc_open = fence_info_open,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
};
#else
static const struct file_operations fence_info_fops = {
	.open = fence_info_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};
#endif

int memcheck_fd_fence_createfs(void)
{
	struct proc_dir_entry *entry = NULL;

	entry = proc_create_data("fence_process_info", 0444,
		NULL, &fence_info_fops, (void *)0);
	if (!entry)
		memcheck_err("Failed to create fence debug info\n");

	return (!entry ? -EFAULT : 0);
}
