/*
 * memcheck_log_ion.c
 *
 * Get ION memory info function
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

#include <linux/dma-buf.h>
#include <linux/fdtable.h>
#include <linux/fs.h>
#include <linux/printk.h>
#include <linux/proc_fs.h>
#include <linux/sched/task.h>
#include <linux/sched/signal.h>
#include <linux/seq_file.h>
#include <linux/version.h>
#include <linux/pid.h>
#include <linux/pid_namespace.h>
#include <platform/trace/hooks/memcheck.h>
#ifndef CONFIG_DFX_OHOS
#include <linux/ion.h>
#if (KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE)
#include "ion_private.h"
#endif
#endif
#include "memcheck_ioctl.h"


#define ION_EXP_NAME_CUT 19 // The first 19 characters of the exp_name do not need to be printed.

struct ion_info_args {
	struct seq_file *seq;
	struct task_struct *task;
	size_t sum;
};

static void ion_info_print_header(struct seq_file *s)
{
	if (s) {
		seq_puts(s, "Process ION heap info:\n");
		seq_puts(s, "----------------------------------------------------\n");
		seq_printf(s, "%s %s %s %s %s %s %s %-.16s\n",
			   "ProcessName", "ProcessID",
			   "fd", "size", "magic", "buf->tgid",
			   "buf->pid", "buf->task_comm");
	} else {
		memcheck_info("Process ION heap info:\n");
		memcheck_info("----------------------------------------------------\n");
		memcheck_info("%s %s %s %s %s %s %s %-.16s\n",
			      "ProcessName", "ProcessID",
			      "Fd", "Size", "Magic", "buf->tgid",
			      "buf->pid", "buf->task_comm");
	}
}

static void ion_info_print_foot(struct seq_file *s)
{
	if (s)
		seq_puts(s, "----------------------------------------------------\n");
	else
		memcheck_info("----------------------------------------------------\n");
}

static struct dma_buf *file_to_dma_buf(struct file *file)
{
	return file->private_data;
}

static int ion_info_cb(const void *data, struct file *f, unsigned int fd)
{
#if (KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE)
	struct ion_info_args *args = (struct ion_info_args *)data;
	struct task_struct *task = args->task;
#endif
	struct dma_buf *dbuf = NULL;

	if (!is_dma_buf_file(f))
		return 0;

	dbuf = file_to_dma_buf(f);
	if (!dbuf)
		return 0;

#if (KERNEL_VERSION(5, 4, 0) >= LINUX_VERSION_CODE)
	if (dbuf->owner != THIS_MODULE)
		return 0;
#endif
#if (KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE)
	args->sum += dbuf->size;
	if (args->seq)
		seq_printf(args->seq, "%s %u %u %zu %u %u %u %-.8s-%-.8s\n",
			   task->comm, task->pid, fd, dbuf->size,
			   file_inode(dbuf->file)->i_ino, dbuf->tgid, dbuf->pid,
			   dbuf->exp_task_comm, dbuf->exp_thread_comm);
	else
		memcheck_info("%s %u %u %zu %u %u %u %-.8s-%-.8s\n",
			      task->comm, task->pid, fd, dbuf->size,
			      file_inode(dbuf->file)->i_ino, dbuf->tgid, dbuf->pid,
			      dbuf->exp_task_comm, dbuf->exp_thread_comm);
#endif
	return 0;
}

static int ion_info_show(struct seq_file *s, void *d)
{
	struct task_struct *task = NULL;
	struct ion_info_args cb_args;
	unsigned long old_ns = ktime_get();

	ion_info_print_header(s);

	rcu_read_lock();
	for_each_process(task) {
		if (task->flags & PF_KTHREAD)
			continue;

		cb_args.sum = 0;
		cb_args.seq = s;
		cb_args.task = task;

		task_lock(task);
		iterate_fd(task->files, 0, ion_info_cb, (void *)&cb_args);
		if (!s && cb_args.sum)
			memcheck_info("Total ION usage for %s is %zu\n", task->comm,
				      cb_args.sum);
		task_unlock(task);
	}
	rcu_read_unlock();

	ion_info_print_foot(s);
	memcheck_info("take %ld ns", ktime_get() - old_ns);

	return 0;
}

void memcheck_ion_info_show(void)
{
#ifdef CONFIG_ANDROID_VENDOR_HOOKS
	memcheck_info("Total ION usage is %llu\n", dma_buf_get_total_size());
#endif
	ion_info_show(NULL, NULL);
}
EXPORT_SYMBOL(memcheck_ion_info_show);

static int ion_info_open(struct inode *inode, struct file *file)
{
	return single_open(file, ion_info_show, PDE_DATA(file_inode(file)));
}

struct dma_loop_args {
	struct seq_file *seq;
	struct dma_buf *d_buf;
	void *priv_args;
};

static inline void list_for_each_dma_buf_do(void (*func)(struct dma_loop_args *),
					    struct dma_loop_args *args) {
	struct dma_buf *d_buf = NULL;
	struct dma_buf_list *d_buf_list = get_dma_buf_list();
	bool func_is_null = (func == NULL);

	mutex_lock(&d_buf_list->lock);
	list_for_each_entry(d_buf, &d_buf_list->head, list_node) {
		if (!d_buf || !d_buf->priv)
			continue;

		if (mutex_lock_interruptible(&d_buf->lock)) {
			pr_info("error locking dma buffer object: skipping\n");
			continue;
		}

		if (!func_is_null) {
			args->d_buf = d_buf;
			func(args);
		}

		mutex_unlock(&d_buf->lock);
	}
	mutex_unlock(&d_buf_list->lock);
}

struct dma_sum_args {
	unsigned int task_tgid;
	unsigned long sum;
	struct task_struct *task;
};

static inline void get_tgid_dma_buf_size(struct dma_loop_args *args)
{
	struct dma_buf *d_buf = args->d_buf;
	struct dma_sum_args *dma_sum_info = (struct dma_sum_args *)args->priv_args;
	unsigned int current_tgid = dma_sum_info->task_tgid;

	if (current_tgid == d_buf->tgid)
		dma_sum_info->sum += d_buf->size;
}

int proc_graphic_mem_operations(struct seq_file *m, struct pid_namespace *ns,
			  struct pid *pid, struct task_struct *task)
{
	struct dma_loop_args args;
	struct dma_sum_args sum_args;
	u64 mapped = 0;
	u64 gpu_total = 0;
	u64 graphic_total;

	sum_args.task_tgid = task->tgid;
	sum_args.sum = 0;
	args.priv_args = &sum_args;

	list_for_each_dma_buf_do(get_tgid_dma_buf_size, &args);
#ifdef CONFIG_ANDROID_VENDOR_HOOKS
	trace_android_mm_memcheck_gpumem_get_usage(task->tgid, &mapped, &gpu_total);
#endif
	graphic_total = sum_args.sum + gpu_total;
	seq_printf(m, "%llu %lu %llu %llu\n", graphic_total, sum_args.sum, mapped, gpu_total - mapped);
	return 0;
}

static inline void print_tgid_dma_info(struct dma_loop_args *args)
{
	struct dma_buf *d_buf = args->d_buf;

	seq_printf(args->seq, "%16.s %16u %16zu\n", d_buf->exp_name,
		   d_buf->tgid, d_buf->size);
}

static int ion_memtrack_show(struct seq_file *s, void *d)
{
	struct dma_loop_args args;

	args.seq = s;
	list_for_each_dma_buf_do(print_tgid_dma_info, &args);
	return 0;
}

static int ion_memtrack_open(struct inode *inode, struct file *file)
{
	return single_open(file, ion_memtrack_show, PDE_DATA(file_inode(file)));
}


#if (KERNEL_VERSION(5, 10, 0) <= LINUX_VERSION_CODE)
static const struct proc_ops ion_info_fops = {
	.proc_open = ion_info_open,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
};

static const struct proc_ops memtrack_fops = {
	.proc_open = ion_memtrack_open,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
};
#else
static const struct file_operations ion_info_fops = {
	.open = ion_info_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static const struct file_operations memtrack_fops = {
	.open = ion_memtrack_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};
#endif

int memcheck_ion_createfs(void)
{
	struct proc_dir_entry *entry = NULL;

	entry = proc_create_data("ion_process_info", 0444,
		NULL, &ion_info_fops, NULL);
	if (!entry)
		memcheck_err("Failed to create ion buffer debug info\n");

	entry = proc_create_data("memtrack", 0444,
		NULL, &memtrack_fops, NULL);
	if (!entry)
		memcheck_err("Failed to create heap debug memtrack\n");

	return (!entry ? -EFAULT : 0);
}
