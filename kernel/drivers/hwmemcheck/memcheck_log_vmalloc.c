/*
 * memcheck_log_vmalloc.c
 *
 * Get vmalloc memory info function
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
#include <linux/printk.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/version.h>
#include <linux/vmalloc.h>
#include "memcheck_ioctl.h"
#include "memcheck_log_vmalloc.h"

#define MAX_VMALLOC_SIZE (700 * 1024 * 1024)

static int vmalloc_info_show(struct seq_file *s, void *d)
{
	struct vmap_area *va = NULL;
	struct vm_struct *v = NULL;
	unsigned long old_ns = ktime_get();

	if (s) {
		seq_puts(s, "Vmalloc info:\n");
		seq_puts(s, "----------------------------------------------------\n");
		seq_printf(s, "%15s %50s %10s\n", "Size", "PC", "Pages");
	} else {
		memcheck_info("Vmalloc info:\n");
		memcheck_info("----------------------------------------------------\n");
		memcheck_info("%15s %50s %10s\n", "Size", "PC", "Pages");
	}

	vmap_lock();
	list_for_each_entry(va, vmap_get_list(), list) {
		v = va->vm;
		if (!v || !(v->flags & VM_ALLOC))
			continue;
		if (s)
			seq_printf(s, "%15ld %50pS %10d\n", v->size, v->caller, v->nr_pages);
		else
			memcheck_info("%15ld %50pS %10d\n", v->size, v->caller, v->nr_pages);
	}
	vmap_unlock();

	if (s)
		seq_puts(s, "----------------------------------------------------\n");
	else
		memcheck_info("----------------------------------------------------\n");

	memcheck_info("take %ld ns", ktime_get() - old_ns);

	return 0;
}

void memcheck_vmalloc_info_show(void)
{
	u64 total = vmalloc_nr_pages() * PAGE_SIZE;

	memcheck_info("Total Vmalloc usage is %llu\n", total);
	if (total < MAX_VMALLOC_SIZE)
		return;
	vmalloc_info_show(NULL, NULL);
}
EXPORT_SYMBOL(memcheck_vmalloc_info_show);

static int vmalloc_info_open(struct inode *inode, struct file *file)
{
	return single_open(file, vmalloc_info_show, PDE_DATA(file_inode(file)));
}

#if (KERNEL_VERSION(5, 10, 0) <= LINUX_VERSION_CODE)
static const struct proc_ops vmalloc_info_fops = {
	.proc_open = vmalloc_info_open,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
};
#else
static const struct file_operations vmalloc_info_fops = {
	.open = vmalloc_info_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};
#endif

int memcheck_vmalloc_createfs(void)
{
	struct proc_dir_entry *entry = NULL;

	entry = proc_create_data("vmalloc_info", 0444,
		NULL, &vmalloc_info_fops, NULL);
	if (!entry)
		memcheck_err("Failed to create vmalloc debug info\n");

	return (!entry ? -EFAULT : 0);
}
