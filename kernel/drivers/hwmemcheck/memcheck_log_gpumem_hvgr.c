/*
 * memtrace_gpumem.c
 *
 * Get gpumem info function
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

#include <linux/seq_file.h>
#include <linux/proc_fs.h>
#include <linux/version.h>

#include "include/hvgr_mem_profile.h"
#include "memcheck_ioctl.h"

#define MAX_GPUMEM_SIZE (1450 * 1024 * 1024)

void memcheck_gpumem_info_show(void)
{
	u64 total;

	total = ((u64)hvgr_mem_statics_used_pages()) * PAGE_SIZE;
	memcheck_info("Total GPU usage is %llu\n", total);
	if (total < MAX_GPUMEM_SIZE)
		return;
	hvgr_mem_profile_all_show(NULL, NULL);
}

static int gpumem_info_open(struct inode *inode, struct file *file)
{
	return single_open(file, hvgr_mem_profile_all_show, PDE_DATA(inode));
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
