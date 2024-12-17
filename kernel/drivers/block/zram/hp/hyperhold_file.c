/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
 * Description: hyperhold implement
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
 * Author:	Wang Fa <fa.wang@huawei.com>
 *
 * Create: 2021-10-18
 *
 */
#define pr_fmt(fmt) "[HYPERHOLD]" fmt

#include <linux/kernel.h>
#include <linux/blkdev.h>
#include <linux/atomic.h>
#include <linux/memcontrol.h>
#include <linux/fs.h>
#include <linux/mount.h>
#include <linux/file.h>
#include <linux/dcache.h>
#include <linux/types.h>
#include <linux/syscalls.h> /* sys_open, sys_read; sys_close */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kthread.h>

#include "zram_drv.h"
#include "hyperhold.h"
#include "hyperhold_internal.h"

#define F2FS_IOCTL_MAGIC	0xf5
#define F2FS_IOC_HP_PREALLOC	_IOWR(F2FS_IOCTL_MAGIC, 31, __u32)

static void hyperhold_update_exts(unsigned long long size)
{
	struct space_info_para *space_info = hyperhold_space_info();
	unsigned int cur_file_exts = get_exts(size);

	if (space_info->alloced_file_num == 0)
		space_info->exts_sum[0] = cur_file_exts - 1;
	else
		space_info->exts_sum[space_info->alloced_file_num] = cur_file_exts +
			space_info->exts_sum[space_info->alloced_file_num - 1];

	space_info->alloced_exts =
		space_info->exts_sum[space_info->alloced_file_num] + 1;
	space_info->alloced_file_num++;
}

static int hyperhold_file_check(const char* file_path)
{
	int ret = 0;
	struct file *file = NULL;
	struct address_space *mapping = NULL;
	unsigned long start, end;
	unsigned long long file_size;
	mm_segment_t old_fs = get_fs();

	set_fs(KERNEL_DS);
	if (ksys_access(file_path, 0)) {
		ret = -ENOENT;
		goto out;
	}

	file = filp_open(file_path, O_RDONLY | O_LARGEFILE, S_IRUSR);
	if (unlikely(IS_ERR_OR_NULL(file))) {
		hh_print(HHLOG_ERR, "open hp file failed!\n");
		ret = -ENOENT;
		goto out;
	}

	mapping = file->f_mapping;
	if (unlikely(!mapping || !mapping->a_ops)) {
		hh_print(HHLOG_ERR, "No address mapping operations!\n");
		ret = -ENOENT;
		goto close;
	}

	start = mapping->a_ops->bmap(mapping, 0);
	file_size = (unsigned long long)i_size_read(file_inode(file));
	end = mapping->a_ops->bmap(mapping, (file_size >> PAGE_SHIFT) - 1);
	if ((end - start != (file_size >> PAGE_SHIFT) - 1) ||
		(!file_size || (file_size % MIN_PER_HP_FILE_SZ_BYTE))) {
		hh_print(HHLOG_ERR, "hyperhold file size error: %lu!\n", file_size);
		ret = -EINVAL;
	}

	hyperhold_update_exts(file_size);
close:
	filp_close(file, NULL);
out:
	set_fs(old_fs);
	return ret;
}

static int hyperhold_file_alloc_proc(const char *file_path,
	unsigned long long size)
{
	int ret;
	struct file *file = NULL;
	struct hp_file_cfg cfg;
	int flag = O_CREAT | O_RDONLY | O_LARGEFILE;

	file = filp_open(file_path, flag, S_IRUSR);
	if (unlikely(IS_ERR_OR_NULL(file))) {
		hh_print(HHLOG_ERR, "open %s failed! ret = %ld\n", file_path,
			 PTR_ERR(file));
		return -ENOENT;
	}
	cfg.pin = 1;
	cfg.len = size;
	cfg.addr = 0;

	/* set the hp file pinned and preallocate for it */
	ret = file->f_op->unlocked_ioctl(file, F2FS_IOC_HP_PREALLOC,
					 (unsigned long)&cfg);
	filp_close(file, NULL);
	if (unlikely(ret)) {
		hh_print(HHLOG_ERR, "unlocked_ioctl %s failed! ret = %ld\n",
			file_path, PTR_ERR(file));
		ksys_unlink(file_path);
		return ret;
	}

	return 0;
}

static void hyperhold_file_unlink(const char *file_path)
{
	int ret;

	ret = ksys_unlink(file_path);
	if (ret)
		hh_print(HHLOG_ERR, "hp file unlink fail! ret = %d", ret);
}

static int hyperhold_file_alloc(const char *file_path, unsigned long long size)
{
	int ret;
	mm_segment_t old_fs = get_fs();

	set_fs(KERNEL_DS);
	ret = hyperhold_file_alloc_proc(file_path, size);
	set_fs(old_fs);

	return ret;
}

#if defined(CONFIG_DFX_DEBUG_FS) && defined(CONFIG_HP_DEFAULT_FILE_SPACE)
static void hyperhold_debug_size(unsigned long long *size)
{
	unsigned int rand;

	/* get rand num from 1 to 100 */
	rand = get_random_int() % 100 + 1;
	/* 40% probability to divide file size */
	if ((rand <= 40) && (*size > MIN_PER_HP_FILE_SZ_BYTE))
		*size = get_divide_size(*size);
}
#endif

static int hyperhold_file_recursive_alloc(
		const char *file_path, unsigned long long size)
{
	int ret;
	struct space_info_para *space_info = hyperhold_space_info();

	while (space_info->alloced_exts + get_exts(size) > space_info->max_exts)
		size = get_divide_size(size);

	while (size >= MIN_PER_HP_FILE_SZ_BYTE) {
#if defined(CONFIG_DFX_DEBUG_FS) && defined(CONFIG_HP_DEFAULT_FILE_SPACE)
		hyperhold_debug_size(&size);
#endif
		space_info->file_ops->free(file_path, size);
		ret = space_info->file_ops->alloc(file_path, size);
		if (ret) {
			/* There is no enough space, just divide */
			size = get_divide_size(size);
			continue;
		}

		hyperhold_update_exts(size);
		return 0;
	}

	hh_print(HHLOG_ERR, "recursive alloc hp file failed with min_size: %llu!\n", size);
	return -ENOSPC;
}

static void hyperhold_file_free(const char *file_path, unsigned long long size)
{
	mm_segment_t old_fs;

	old_fs = get_fs();
	set_fs(KERNEL_DS);
	if (!ksys_access(file_path, 0))
		hyperhold_file_unlink(file_path);
	set_fs(old_fs);
}

static int hyperhold_file_get_sector(const char *file_path, sector_t *start_sector)
{
	struct file *file = NULL;
	struct address_space *mapping = NULL;
	unsigned long start;

	file = filp_open(file_path, O_RDONLY | O_LARGEFILE, S_IRUSR);
	if (unlikely(IS_ERR_OR_NULL(file))) {
		hh_print(HHLOG_ERR, "open hp file failed!\n");
		return -ENOENT;
	}

	mapping = file->f_mapping;
	if (unlikely(!mapping || !mapping->a_ops)) {
		hh_print(HHLOG_ERR, "No address mapping operations!\n");
		filp_close(file, NULL);
		return -ENOENT;
	}

	start = mapping->a_ops->bmap(mapping, 0);

	filp_close(file, NULL);

	*start_sector = start << BLOCK_SECTOR_SHIFT;

	return 0;
}

static struct hyperhold_file_ops hyperhold_file_ops = {
	.alloc = hyperhold_file_alloc,
	.recur_alloc = hyperhold_file_recursive_alloc,
	.check = hyperhold_file_check,
	.free = hyperhold_file_free,
	.get_sector = hyperhold_file_get_sector,
};

struct hyperhold_file_ops *hyperhold_get_file_ops(void)
{
	return &hyperhold_file_ops;
}
