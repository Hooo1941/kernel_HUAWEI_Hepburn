/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#include "hvgr_mem_debugfs.h"

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#include <linux/proc_fs.h>
#include <securec.h>

#include "hvgr_debugfs_api.h"
#include "hvgr_dm_api.h"
#include "hvgr_log_api.h"

static int hvgr_mem_gpu_memory_show(struct seq_file *file, void *data)
{
	struct hvgr_ctx *tmp = NULL;
	struct hvgr_ctx *ctx_tmp = NULL;
	struct hvgr_device *gdev = hvgr_get_device();

	if (gdev == NULL)
		return 0;

	seq_printf(file, "%-16s  %10u\n", gdev->devname, atomic_read(&(gdev->mem_dev.used_pages)));

	mutex_lock(&gdev->dm_dev.ctx_list_mutex);
	list_for_each_entry_safe(ctx_tmp, tmp, &gdev->dm_dev.ctx_list, dev_link) {
		seq_printf(file, "  kmd-ctx_%u %10u %10u %10u %10u\n",
			ctx_tmp->id, ctx_tmp->pid, ctx_tmp->tgid,
			atomic_read(&(ctx_tmp->mem_ctx.used_pages)),
			atomic_read(&(ctx_tmp->mem_ctx.used_pages)));
	}
	mutex_unlock(&gdev->dm_dev.ctx_list_mutex);

	return 0;
}

static int hvgr_mem_gpu_memory_open(struct inode *in, struct file *file)
{
	return single_open(file, hvgr_mem_gpu_memory_show, NULL);
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0))
static const struct file_operations hvgr_mem_gpu_memory_fops = {
	.open = hvgr_mem_gpu_memory_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};
#else
static const struct proc_ops hvgr_mem_gpu_memory_fops = {
	.proc_open = hvgr_mem_gpu_memory_open,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
};
#endif

void hvgr_mtrack_debugfs_init(struct hvgr_device * const gdev)
{
	gdev->mem_dev.proc_gpu_memory = proc_create("gpu_memory", S_IRUGO, NULL,
		&hvgr_mem_gpu_memory_fops);
	if (gdev->mem_dev.proc_gpu_memory == NULL)
		hvgr_err(gdev, HVGR_MEM, "create proc gpu_memory fail");
}

void hvgr_mtrack_debugfs_term(struct hvgr_device * const gdev)
{
	if (gdev->mem_dev.proc_gpu_memory != NULL)
		proc_remove(gdev->mem_dev.proc_gpu_memory);
}
