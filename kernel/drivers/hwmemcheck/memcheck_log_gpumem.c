/*
 * memtrace_gpumem.c
 *
 * Get gpumem info function
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

#include <linux/seq_file.h>
#include <linux/proc_fs.h>
#include <linux/version.h>
#include "kgsl.h"
#include "kgsl_device.h"
#include "kgsl_sharedmem.h"
#if (KERNEL_VERSION(5, 4, 0) >= LINUX_VERSION_CODE)
#include "memcheck_ioctl.h"

static const char *memtype_str(int memtype)
{
	if (memtype == KGSL_MEM_ENTRY_KERNEL)
		return "gpumem";
	else if (memtype == KGSL_MEM_ENTRY_USER)
		return "usermem";
	else if (memtype == KGSL_MEM_ENTRY_ION)
		return "ion";

	return "unknown";
}
#endif

#define MAX_GPUMEM_SIZE (1450 * 1024 * 1024)
u64 gpu_get_total_used(void);

#if (KERNEL_VERSION(5, 10, 0) <= LINUX_VERSION_CODE)
#define	memcheck_err(format, ...)	\
	pr_err("MemCheck[%s %d] " format, __func__, __LINE__, ##__VA_ARGS__)
#define	memcheck_info(format, ...)	\
	pr_info("MemCheck[%s %d] " format, __func__, __LINE__, ##__VA_ARGS__)
#endif

static int gpumem_print_entry(struct seq_file *s, void *ptr, struct task_struct *task, u64 *sum)
{
	struct kgsl_mem_entry *entry = ptr;
	char usage[16];
	struct kgsl_memdesc *m = &entry->memdesc;
	unsigned int usermem_type;

	usermem_type = kgsl_memdesc_usermem_type(m);
	if (usermem_type == KGSL_MEM_ENTRY_ION)
		return 0;

	kgsl_get_memory_usage(usage, sizeof(usage), m->flags);
	if (s)
		seq_printf(s, "%16s %6d %16llu %5d %10s %16s\n",
			   task->comm, task->tgid, m->size, entry->id,
			   memtype_str(usermem_type), usage);
	else
		memcheck_info("%16s %6d %16llu %5d %10s %16s\n",
			task->comm, task->tgid, m->size, entry->id,
			memtype_str(usermem_type), usage);
	*sum += m->size;

	return 0;
}

static void gpumem_info_print_header(struct seq_file *s)
{
	if (s) {
		seq_puts(s, "Process GPU info:\n");
		seq_puts(s, "----------------------------------------------------\n");
		seq_printf(s, "%16s %6s %16s %5s %10s %16s\n",
			   "task", "tgid", "size", "id", "type", "usage");
	} else {
		memcheck_info("Process GPU info:\n");
		memcheck_info("----------------------------------------------------\n");
		memcheck_info("%16s %6s %16s %5s %10s %16s\n",
			"task", "tgid", "size", "id", "type", "usage");
	}
}

static void gpumem_info_print_foot(struct seq_file *s)
{
	if (s)
		seq_puts(s, "----------------------------------------------------\n");
	else
		memcheck_info("----------------------------------------------------\n");
}

static int gpumem_info_func(struct seq_file *s, void *ptr, pid_t pid, u64 *mapped, u64 *total)
{
	struct kgsl_process_private *tmp = NULL;
	struct kgsl_process_private *cur_priv = NULL;

#if (KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE)
	read_lock(&kgsl_driver.proclist_lock);
#else
	spin_lock(&kgsl_driver.proclist_lock);
#endif
	list_for_each_entry(tmp, &kgsl_driver.process_list, list) {
		int id = 0;
		u64 sum = 0;
		struct task_struct *task = NULL;
		struct kgsl_mem_entry *entry = NULL;

		cur_priv = tmp;
		task = get_pid_task(cur_priv->pid, PIDTYPE_PID);
		if (!task)
			continue;
		if (pid && mapped && total) {
			if (pid == task->tgid) {
				*mapped = atomic64_read(&cur_priv->gpumem_mapped);
				*total = atomic64_read(&cur_priv->stats[KGSL_MEM_ENTRY_KERNEL].cur);
				put_task_struct(task);
				goto found;
			} else {
				put_task_struct(task);
				continue;
			}
		}
		spin_lock(&cur_priv->mem_lock);
		idr_for_each_entry(&cur_priv->mem_idr, entry, id) {
			kgsl_mem_entry_get(entry);
			gpumem_print_entry(s, entry, task, &sum);
			kgsl_mem_entry_put(entry);
		}
		spin_unlock(&cur_priv->mem_lock);
		if (!s)
			memcheck_info("Total GPU usage for %s is %zu\n", task->comm, sum);
		put_task_struct(task);
	}
found:
#if (KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE)
	read_unlock(&kgsl_driver.proclist_lock);
#else
	spin_unlock(&kgsl_driver.proclist_lock);
#endif
	return 0;
}

static int gpumem_info_show(struct seq_file *s, void *ptr)
{
	unsigned long old_ns = ktime_get();

	gpumem_info_print_header(s);
	gpumem_info_func(s, ptr, 0, NULL, NULL);
	gpumem_info_print_foot(s);

	memcheck_info("take %ld ns", ktime_get() - old_ns);

	return 0;
}

void memcheck_gpumem_info_show(void)
{
#if (KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE)
	u64 total = gpu_get_total_used() * PAGE_SIZE;

	memcheck_info("Total GPU usage is %llu\n", total);
	if (total < MAX_GPUMEM_SIZE)
		return;
#endif
	gpumem_info_show(NULL, NULL);
}
EXPORT_SYMBOL(memcheck_gpumem_info_show);

int memcheck_gpumem_get_usage(pid_t pid, u64 *mapped, u64 *total)
{
	return gpumem_info_func(NULL, NULL, pid, mapped, total);
}

static int gpumem_info_open(struct inode *inode, struct file *file)
{
	return single_open(file, gpumem_info_show, PDE_DATA(inode));
}

#if (KERNEL_VERSION(5, 10, 0) <= LINUX_VERSION_CODE)
static const struct proc_ops gpumem_info_fops = {
	.proc_open = gpumem_info_open,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
};
#else
static const struct file_operations gpumem_info_fops = {
	.open = gpumem_info_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};
#endif

int memcheck_gpumem_createfs(void)
{
	struct proc_dir_entry *entry = NULL;

	entry = proc_create_data("gpumem_process_info", 0444,
		NULL, &gpumem_info_fops, NULL);
	if (!entry)
		memcheck_err("Failed to create gpu buffer debug info\n");

	return (!entry ? -EFAULT : 0);
}
