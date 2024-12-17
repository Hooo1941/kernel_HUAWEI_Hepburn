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
#ifdef CONFIG_DFX_DEBUG_FS
#include <linux/debugfs.h>
#include <linux/proc_fs.h>
#endif
#include <linux/seq_file.h>
#include <securec.h>

#include "hvgr_debugfs_api.h"
#include "hvgr_dm_api.h"
#include "hvgr_log_api.h"

#define HVGR_MEM_PROFILE_MAX_SIZE ((64 + ((80 + (56 * 64)) * 3 * 15) + (56 * 6)))

#ifdef CONFIG_DFX_DEBUG_FS
static int hvgr_mem_profile_show(struct seq_file *sfile, void *data)
{
	struct hvgr_ctx *ctx = sfile->private;

	mutex_lock(&ctx->mem_ctx.profile_mutex);
	seq_write(sfile, ctx->mem_profile_buf, (size_t)ctx->mem_profile_size);
	seq_putc(sfile, '\n');
	mutex_unlock(&ctx->mem_ctx.profile_mutex);
	return 0;
}

/*
 *  File operations related to debugfs entry for mem_profile
 */
static int hvgr_mem_profile_open(struct inode *in, struct file *file)
{
	return single_open(file, hvgr_mem_profile_show, in->i_private);
}

static const struct file_operations hvgr_mem_profile_fops = {
	.open = hvgr_mem_profile_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};
#endif

long hvgr_mem_add_profile(struct hvgr_ctx * const ctx, uint64_t user_buf, uint64_t size)
{
	char *kbuf = NULL;
#ifdef CONFIG_DFX_DEBUG_FS
	struct dentry *profile = NULL;
#endif

	if (unlikely(user_buf == 0 || size == 0 || size > HVGR_MEM_PROFILE_MAX_SIZE)) {
		hvgr_err(ctx->gdev, HVGR_MEM, "mem profile size %lu addr 0x%lx error",
			size, user_buf);
		return -EINVAL;
	}

	mutex_lock(&ctx->mem_ctx.profile_mutex);

	kbuf = kvzalloc(size, GFP_KERNEL);
	if (kbuf == NULL) {
		mutex_unlock(&ctx->mem_ctx.profile_mutex);
		hvgr_err(ctx->gdev, HVGR_MEM, "mem profile malloc fail");
		return -ENOMEM;
	}

	if (copy_from_user(kbuf, u64_to_user_ptr(user_buf), size) != 0) {
		mutex_unlock(&ctx->mem_ctx.profile_mutex);
		hvgr_err(ctx->gdev, HVGR_MEM, "mem profile copy from user fail");
		kvfree(kbuf);
		return -EFAULT;
	}

#ifdef CONFIG_DFX_DEBUG_FS
	if (!hvgr_ctx_flag(ctx, HVGR_CTX_FLAG_MEM_PROFILE_ADDED)) {
		profile = debugfs_create_file("mem_profile", S_IRUGO, ctx->debugfs_dir, ctx,
			&hvgr_mem_profile_fops);
		if (profile == NULL) {
			mutex_unlock(&ctx->mem_ctx.profile_mutex);
			hvgr_err(ctx->gdev, HVGR_MEM, "mem profile create file fail");
			kvfree(kbuf);
			return -EFAULT;
		}

		hvgr_ctx_flag_set(ctx, HVGR_CTX_FLAG_MEM_PROFILE_ADDED);
	}
#endif

	if (ctx->mem_profile_buf != NULL)
		kvfree(ctx->mem_profile_buf);

	ctx->mem_profile_buf = kbuf;
	ctx->mem_profile_size = size;

	mutex_unlock(&ctx->mem_ctx.profile_mutex);
	return 0;
}

void hvgr_mem_remove_profile(struct hvgr_ctx * const ctx)
{
	mutex_lock(&ctx->mem_ctx.profile_mutex);

	if (ctx->mem_profile_buf != NULL)
		kvfree(ctx->mem_profile_buf);

	ctx->mem_profile_buf = NULL;
	ctx->mem_profile_size = 0;
	mutex_unlock(&ctx->mem_ctx.profile_mutex);
}

static void hvgr_mem_profile_kmsg(struct hvgr_ctx *ctx, u64 used, u64 used_grow, u64 used_pt)
{
	char *buff = kvzalloc(ctx->mem_profile_size + 1, GFP_KERNEL);
	if (buff == NULL)
		return;

	pr_err("ctx_%u %10u %10u used summary:%llu grow:%llu user:%llu\n",ctx->id, ctx->pid,
		ctx->tgid, used, used_grow, used - used_pt);

	if (ctx->mem_profile_buf) {
		(void)memcpy_s(buff, ctx->mem_profile_size,
			ctx->mem_profile_buf, ctx->mem_profile_size);
		pr_err("%s\n", buff);
	}

	kvfree(buff);
}

int hvgr_mem_profile_all_show(struct seq_file *sfile, void *data)
{
	struct hvgr_device *dev = hvgr_get_device();
	struct hvgr_ctx *tmp = NULL;
	struct hvgr_ctx *ctx = NULL;
	u64 used;
	u64 used_grow;
	u64 used_pt;

	if (unlikely(dev == NULL))
		return -EINVAL;

	mutex_lock(&dev->dm_dev.ctx_list_mutex);
	list_for_each_entry_safe(ctx, tmp, &dev->dm_dev.ctx_list, dev_link) {
		mutex_lock(&ctx->mem_ctx.profile_mutex);
		used = ((u64)atomic_read(&(ctx->mem_ctx.used_pages))) * PAGE_SIZE;
		used_grow = ((u64)atomic_read(&(ctx->mem_ctx.used_growable_pages))) * PAGE_SIZE;
		used_pt = ((u64)atomic_read(&(ctx->mem_ctx.used_pt_pages))) * PAGE_SIZE;
		if (sfile != NULL) {
			seq_printf(sfile,
				"ctx_%u %10u %10u used summary:%llu grow:%llu user:%llu\n",
				ctx->id, ctx->pid, ctx->tgid, used, used_grow, used - used_pt);
			if (ctx->mem_profile_buf) {
				seq_write(sfile, ctx->mem_profile_buf,
					(size_t)ctx->mem_profile_size);
				seq_printf(sfile, "%s", "\n");
			}
		} else {
			hvgr_mem_profile_kmsg(ctx, used, used_grow, used_pt);
		}
		mutex_unlock(&ctx->mem_ctx.profile_mutex);
	}
	mutex_unlock(&dev->dm_dev.ctx_list_mutex);

	return 0;
}

#ifdef CONFIG_DFX_DEBUG_FS
static uint64_t hvgr_mem_get_jit_pool_pages(struct hvgr_ctx *ctx)
{
	struct hvgr_mem_area *iter = NULL;
	struct hvgr_mem_area *temp = NULL;
	uint64_t jit_pages = 0;

	mutex_lock(&ctx->mem_ctx.jit_shrink_mutex);
	list_for_each_entry_safe(iter, temp, &ctx->mem_ctx.jit_pool_head, jit_node)
		jit_pages += iter->pages;
	mutex_unlock(&ctx->mem_ctx.jit_shrink_mutex);

	return jit_pages;
}

ssize_t hvgr_mem_show_page_statics(void *data, char * const buf)
{
	ssize_t ret = 0;
	struct hvgr_ctx *tmp = NULL;
	struct hvgr_ctx *ctx_tmp = NULL;
	struct hvgr_pool *pool = NULL;
	struct hvgr_device *gdev = (struct hvgr_device *)data;
	struct hvgr_mem_dev *mem_dev = &gdev->mem_dev;
	uint32_t pool_pages = 0;
	uint64_t jit_pages = 0;
	uint32_t ctx_num = 0;

	ret += scnprintf(buf + ret, PAGE_SIZE - ret,
		"alloc pages : %lu free pages: %lu sync pages: %lu clear pages: %lu grow pages: %lu\n",
		mem_dev->statics.alloc_pages, mem_dev->statics.free_pages,
		mem_dev->statics.pool_sync_pages, mem_dev->statics.pool_clear_pages,
		mem_dev->statics.grow_pages);
#ifdef HVGR_FEATURE_SYSTEM_CACHE
	ret += scnprintf(buf + ret, PAGE_SIZE - ret,
		"sc alloc pages : %lu free pages: %lu\n",
		mem_dev->statics.sc_alloc_pages, mem_dev->statics.sc_free_pages);
#endif
	ret += scnprintf(buf + ret, PAGE_SIZE - ret,
		"mmu alloc pages : %lu free pages: %lu free: %lu\n",
		mem_dev->statics.mmu_alloc_pages, mem_dev->statics.mmu_free_pages,
		mem_dev->statics.mmu_free);

	pool = &gdev->mem_dev.mem_pool.pool;

	ret += scnprintf(buf + ret, PAGE_SIZE - ret, "dev pool pages: %u\n", pool->cur_size);

	mutex_lock(&gdev->dm_dev.ctx_list_mutex);
	list_for_each_entry_safe(ctx_tmp, tmp, &gdev->dm_dev.ctx_list, dev_link) {
		pool = &ctx_tmp->mem_ctx.mem_pool.pool;
		pool_pages += pool->cur_size;
		jit_pages += hvgr_mem_get_jit_pool_pages(ctx_tmp);
		ctx_num++;
	}
	mutex_unlock(&gdev->dm_dev.ctx_list_mutex);

	ret += scnprintf(buf + ret, PAGE_SIZE - ret, "ctx num:%u pool pages:%u jit pages:%llu\n",
		ctx_num, pool_pages, jit_pages);

	return ret;
}

ssize_t hvgr_mem_clear_page_statics(void *data, const char *buf, size_t count)
{
	struct hvgr_device *gdev = (struct hvgr_device *)data;
	struct hvgr_mem_statics *statics = &gdev->mem_dev.statics;

	if (memset_s(statics, sizeof(*statics), 0, sizeof(*statics)) != 0)
		return -EINVAL;

	return (ssize_t)count;
}

hvgr_debugfs_read_write_declare(page_statics, hvgr_mem_show_page_statics,
	hvgr_mem_clear_page_statics);

void hvgr_mem_debugfs_init(struct hvgr_device * const gdev)
{
	hvgr_debugfs_register(gdev, "pages", gdev->mem_dev.mem_dir, &page_statics);
}

#else

void hvgr_mem_debugfs_init(struct hvgr_device * const gdev)
{
}

#endif
