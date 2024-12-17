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
 * Create: 2021-12-15
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
#ifdef CONFIG_DFX_DEBUG_FS
#include <platform_include/basicplatform/linux/rdr_platform.h>
#endif
#include <linux/hp/hyperhold_inf.h>

#include "zram_drv.h"
#include "hyperhold.h"
#include "hyperhold_internal.h"

#define HP_PARTITION_PATH "/dev/block/by-name/hyperhold"
#define USERDATA_PARTITION_PATH "/dev/block/by-name/userdata"
#define HP_FILE_PATH "/data/vendor/8ac9cd7ecba7097c176cfde5f1dbc544"
#define MAX_HP_FILE_PATH_LEN 128
#define MAX_HP_FILE_SZ_MBYTE (5UL * 1024UL)
#define DEFAULT_HP_FILE_SZ_MBYTE (2UL * 1024UL)
#define MAX_HP_FILE_NUM MAX_HP_FILE_SZ_MBYTE
#define MAX_PER_HP_FILE_SZ_MBYTE (128UL)
#define MIN_PER_HP_FILE_SZ_MBYTE (2UL)

static void hyperhold_space_set_zram(struct zram *zram,
	struct block_device *bdev, unsigned long nr_pages)
{
	down_write(&zram->init_lock);
	zram->bdev = bdev;
	zram->backing_dev = NULL;
	zram->nr_pages = nr_pages;
	up_write(&zram->init_lock);
}

static sector_t hyperhold_get_partition_sector(int ext_id)
{
	return ext_id * EXTENT_SECTOR_SIZE;
}

static void hyperhold_get_file_index(unsigned long *exts_sum, unsigned int ext_id,
	unsigned long file_num, unsigned long *file_index, unsigned long *file_offset)
{
	unsigned long start = 0;
	unsigned long end = file_num - 1;
	unsigned long middle;

	while (start < end) {
		middle = start + (end - start) / 2;
		if (exts_sum[middle] == ext_id) {
			*file_index = middle;
			goto out;
		}

		if (exts_sum[middle] + 1 == ext_id) {
			*file_index = middle + 1;
			goto out;
		}

		if (exts_sum[middle] > ext_id) {
			end = middle;
			continue;
		}
		if (exts_sum[middle] < ext_id) {
			start = middle + 1;
			continue;
		}
	}

	*file_index = start;
out:
	if (*file_index == 0)
		*file_offset = ext_id;
	else
		*file_offset = ext_id - exts_sum[*file_index - 1] - 1;
}

static sector_t hyperhold_get_file_sector(int ext_id)
{
	unsigned long file_index;
	unsigned long file_offset;
	struct space_info_para *space_info = hyperhold_space_info();

	if (unlikely(ext_id < 0)) {
		hh_print(HHLOG_ERR, "ext_id error: %d\n", ext_id);
		return 0;
	}

	if (unlikely(!space_info->alloced_file_num)) {
		hh_print(HHLOG_ERR, "alloced file num is zero\n");
		return 0;
	}

	hyperhold_get_file_index(space_info->exts_sum, (unsigned int)ext_id,
		space_info->alloced_file_num, &file_index, &file_offset);
	if (unlikely(!space_info->start_sector ||
		!space_info->start_sector[file_index])) {
		hh_print(HHLOG_ERR, "start_sector is err, file_index %lu\n",
			file_index);
		return 0;
	}

	return space_info->start_sector[file_index] +
		file_offset * EXTENT_SECTOR_SIZE;
}

static int hyperhold_get_file_path(char *buff, unsigned int idnex)
{
	int ret;

	ret = snprintf(buff, MAX_HP_FILE_PATH_LEN, "%s_%u",
		HP_FILE_PATH, idnex);
	if (ret <= 0)
		return -EAGAIN;

	return 0;
}

static int hyperhold_file_ops_check(struct space_info_para *space_info)
{
	if (!space_info->file_ops || !space_info->file_ops->alloc ||
		!space_info->file_ops->recur_alloc || !space_info->file_ops->check ||
		!space_info->file_ops->free || !space_info->file_ops->get_sector)
		return -EINVAL;

	return 0;
}

static void hyperhold_alloc_file_proc(unsigned long start_file_index)
{
	int ret;
	unsigned long file_index;
	char file_path[MAX_HP_FILE_PATH_LEN] = { 0 };
	struct space_info_para *space_info = hyperhold_space_info();

	if (start_file_index != space_info->alloced_file_num) {
		hh_print(HHLOG_ERR, "start file index error: %d %d\n",
			start_file_index, space_info->alloced_file_num);
		return;
	}

	if (hyperhold_file_ops_check(space_info)) {
		hh_print(HHLOG_ERR, "hyperhold_file_ops_check fail\n");
		return;
	}

	for (file_index = start_file_index; (file_index < space_info->max_file_num) &&
		(space_info->alloced_exts < space_info->max_exts);
		++file_index) {
		ret = hyperhold_get_file_path(file_path, file_index);
		if (unlikely(ret)) {
			hh_print(HHLOG_ERR, "hyperhold_get_file_path %u fail\n",
				file_index);
			break;
		}

		if (!space_info->file_ops->check(file_path))
			continue;

		ret = space_info->file_ops->recur_alloc(file_path, MAX_PER_HP_FILE_SZ_BYTE);
		if (ret)
			break;
	}

	hh_print(HHLOG_ERR, "alloced_file_num %u\n", file_index);
}

static void hyperhold_free_file_proc(unsigned long file_num,
	unsigned long start_file_index)
{
	int ret;
	unsigned long cnt;
	char file_path[MAX_HP_FILE_PATH_LEN] = { 0 };
	struct space_info_para *space_info = hyperhold_space_info();

	if (hyperhold_file_ops_check(space_info)) {
		hh_print(HHLOG_ERR, "hyperhold_file_ops_check fail\n");
		return;
	}

	for (cnt = 0; cnt < file_num; ++cnt) {
		ret = hyperhold_get_file_path(file_path, start_file_index + cnt);
		if (unlikely(ret)) {
			hh_print(HHLOG_ERR, "hyperhold_get_file_path %u fail \n", cnt);
			break;
		}

		space_info->file_ops->free(file_path, MAX_PER_HP_FILE_SZ_BYTE);
	}
}

static int hyperhold_get_bdev(const char *dir_name, struct block_device **bdev)
{
	int ret = 0;
	struct file *file = NULL;
	struct super_block *sb = NULL;
	struct space_info_para *space_info = hyperhold_space_info();

	file = filp_open(dir_name, O_RDONLY, S_IRWXU);
	if (IS_ERR_OR_NULL(file)) {
		hh_print(HHLOG_ERR, "open file %s error %ld", dir_name,
			 PTR_ERR(file));
		return -EINVAL;
	}

	sb = file->f_path.mnt->mnt_sb;
	if (!sb || (sb->s_magic != space_info->file_magic)) {
		ret = -ENOENT;
		hh_print(HHLOG_ERR, "mount info error!\n");
		goto out;
	}

	hh_print(HHLOG_DEBUG, "path = %s fstype = %s\n", dir_name,
		 sb->s_type->name);
	*bdev = sb->s_bdev;
out:
	filp_close(file, NULL);
	return ret;
}

static void hyperhold_get_userdata_bdev(struct block_device **bdev);

static int hyperhold_alloc_file_space(struct zram *zram)
{
	int ret;
	struct block_device *bdev = NULL;
	struct block_device *userdata_bdev = NULL;
	struct space_info_para *space_info = hyperhold_space_info();

	ret = hyperhold_get_bdev("/data/vendor", &bdev);
	if (unlikely(ret)) {
		hh_print(HHLOG_ERR, "hyperhold_get_bdev fail\n");
		return -EINVAL;
	}

	if (!bdev) {
		hh_print(HHLOG_ERR, "bdev is null\n");
		return -EINVAL;
	}

	hyperhold_get_userdata_bdev(&userdata_bdev);
	if (userdata_bdev)
		space_info->userdata_bdev = userdata_bdev;

	hyperhold_alloc_file_proc(0);
	hyperhold_free_file_proc(MAX_HP_FILE_NUM - space_info->alloced_file_num,
		space_info->alloced_file_num);

	hyperhold_space_set_zram(zram, bdev, space_info->max_pages);

	return 0;
}

static int hyperhold_check_space_setting(struct blk_hp_set_space *space_setting)
{
	struct space_info_para *space_info = hyperhold_space_info();

	if (space_setting->space_type >= HP_SPACE_TYPE_BUTT) {
		hh_print(HHLOG_ERR, "space_type type %d err\n", space_setting->space_type);
		return -EINVAL;
	}

	if ((space_setting->space_type == HP_FILE_SPACE) &&
		(!space_setting->max_size ||
		(space_setting->max_size > MAX_HP_FILE_SZ_MBYTE))) {
		hh_print(HHLOG_ERR, "space setting err available %lu %lu %lu\n",
			space_setting->max_size, MAX_HP_FILE_SZ_MBYTE,
			space_info->max_file_size);
		return -EINVAL;
	}

	return 0;
}

bool hyperhold_space_is_full(int add_num)
{
	bool is_full;
	unsigned long flags;
	struct space_info_para *space_info = hyperhold_space_info();

	if (!hyperhold_is_file_space())
		return false;

	spin_lock_irqsave(&space_info->exts_lock, flags);
	is_full = ((space_info->used_exts + (unsigned long)add_num) > space_info->alloced_exts);
	spin_unlock_irqrestore(&space_info->exts_lock, flags);
	return is_full;
}

int hyperhold_used_exts_num_inc(void)
{
	unsigned long flags;
	struct space_info_para *space_info = hyperhold_space_info();

	if (!hyperhold_is_file_space())
		return 0;

	spin_lock_irqsave(&space_info->exts_lock, flags);
	if (space_info->used_exts >= space_info->alloced_exts) {
		spin_unlock_irqrestore(&space_info->exts_lock, flags);
		return -EFAULT;
	}

	space_info->used_exts++;
	spin_unlock_irqrestore(&space_info->exts_lock, flags);
	return 0;
}

void hyperhold_used_exts_num_dec(void)
{
	unsigned long flags;
	struct space_info_para *space_info = hyperhold_space_info();

	if (!hyperhold_is_file_space())
		return;

	spin_lock_irqsave(&space_info->exts_lock, flags);
	if (likely(space_info->used_exts))
		space_info->used_exts--;
	else
		hh_print(HHLOG_ERR, "used_exts_num is zero\n");
	spin_unlock_irqrestore(&space_info->exts_lock, flags);
}

static void hyperhold_save_space_setting(struct blk_hp_set_space *space_setting)
{
	struct space_para_cfg *space_para = hyperhold_space_para();
	struct space_info_para *space_info = hyperhold_space_info();
	unsigned long total_file_size;

	space_para->space_type = space_setting->space_type;
	spin_lock_init(&space_info->exts_lock);
	if (space_para->space_type == HP_FILE_SPACE) {
		space_para->max_size = space_setting->max_size;
		space_info->min_file_num = (space_para->max_size +
			space_info->max_file_size - 1) / space_info->max_file_size;
		space_info->max_file_num = (space_para->max_size +
			space_info->min_file_size - 1) / space_info->min_file_size;
		total_file_size = space_info->min_file_num * space_info->max_file_size;
		space_info->max_pages =
			(total_file_size * MBYTE_TO_BYTE_FACTOR) >> PAGE_SHIFT;
		space_info->max_exts = (space_info->max_pages << PAGE_SHIFT) >> EXTENT_SHIFT;
		space_info->alloced_file_num = 0;
		space_info->used_exts = 0;
		space_info->alloced_exts = 0;
		space_info->userdata_bdev = NULL;

		space_info->exts_sum = vzalloc(space_info->max_file_num * sizeof(unsigned long));
		if (!space_info->exts_sum)
			hh_print(HHLOG_ERR, "exts_num alloc failed\n");
	}

	hh_print(HHLOG_ERR, "space_type %lu max_size %lu\n",
		space_para->space_type, space_para->max_size);
	hh_print(HHLOG_ERR, "max_file_num %lu min_file_num %lu alloced_file_num %lu\n",
		space_info->max_file_num, space_info->min_file_num, space_info->alloced_file_num);
	hh_print(HHLOG_ERR, "max_pages %lu max_exts %lu\n",
		space_info->max_pages, space_info->max_exts);
}

static int hyperhold_get_file_para(void)
{
	struct space_info_para *space_info = hyperhold_space_info();
	space_info->max_file_size = MAX_PER_HP_FILE_SZ_MBYTE;
	space_info->min_file_size = MIN_PER_HP_FILE_SZ_MBYTE;
	space_info->file_ops = hyperhold_get_file_ops();
	space_info->file_magic = F2FS_SUPER_MAGIC;

	return 0;
}

static int hyperhold_file_para_init(struct block_device *bdev)
{
	struct request_queue *q = bdev_get_queue(bdev);

	if (!blk_queue_query_unistore_enable(q))
		return hyperhold_get_file_para();

	hh_print(HHLOG_ERR, "file partition is not ready for unistore\n");
	return -EFAULT;
}

static int hyperhold_get_space_setting(struct blk_hp_set_space *space_setting,
	struct blk_hp_cmd *cmd)
{
	if (!cmd->cust_argp) {
		hh_print(HHLOG_ERR, "cust_argp is null\n");
		return -EFAULT;
	}

	if (copy_from_user(space_setting, cmd->cust_argp, sizeof(struct blk_hp_set_space))) {
		hh_print(HHLOG_ERR, "copy_from_user failed\n");
		return -EFAULT;
	}

	return 0;
}

static void hyperhold_expand_space_proc(void)
{
	struct space_info_para *space_info = hyperhold_space_info();
	if (space_info->alloced_file_num < space_info->max_file_num &&
		space_info->alloced_exts < space_info->max_exts)
		hyperhold_alloc_file_proc(space_info->alloced_file_num);
}

static void hyperhold_close_bdev(struct block_device *bdev,
					struct file *backing_dev)
{
	if (bdev)
		blkdev_put(bdev, FMODE_READ | FMODE_WRITE | FMODE_EXCL);

	if (backing_dev)
		filp_close(backing_dev, NULL);
}

static struct file *hyperhold_open_bdev(const char *file_name, int flags)
{
	struct file *backing_dev = NULL;

	backing_dev = filp_open(file_name, flags, 0);
	if (unlikely(IS_ERR(backing_dev))) {
		hh_print(HHLOG_ERR, "open the %s failed! eno = %lld\n",
					file_name, PTR_ERR(backing_dev));
		backing_dev = NULL;
		return NULL;
	}

	if (unlikely(!S_ISBLK(backing_dev->f_mapping->host->i_mode))) {
		hh_print(HHLOG_ERR, "%s isn't a blk device\n", file_name);
#ifdef CONFIG_DFX_DEBUG_FS
		rdr_syserr_process_for_ap((u32)MODID_AP_S_PANIC_STORAGE,
			0ull, 0ull);
#endif
		hyperhold_close_bdev(NULL, backing_dev);
		return NULL;
	}

	return backing_dev;
}

static int hyperhold_bind(struct zram *zram, const char *file_name)
{
	struct file *backing_dev = NULL;
	struct inode *inode = NULL;
	unsigned long nr_pages;
	struct block_device *bdev = NULL;
	int err;

	backing_dev = hyperhold_open_bdev(file_name, O_RDWR | O_LARGEFILE);
	if (unlikely(!backing_dev))
		return -EINVAL;

	inode = backing_dev->f_mapping->host;

	bdev = bdgrab(I_BDEV(inode));
	err = blkdev_get(bdev, FMODE_READ | FMODE_WRITE | FMODE_EXCL, zram);
	if (unlikely(err < 0)) {
		hh_print(HHLOG_ERR, "%s blkdev_get failed! eno = %d\n",
					file_name, err);
#ifdef CONFIG_DFX_DEBUG_FS
		rdr_syserr_process_for_ap((u32)MODID_AP_S_PANIC_STORAGE,
			0ull, 0ull);
#endif
		bdev = NULL;
		goto out;
	}

	nr_pages = (unsigned long)i_size_read(inode) >> PAGE_SHIFT;

	err = set_blocksize(bdev, PAGE_SIZE);
	if (unlikely(err)) {
		hh_print(HHLOG_ERR,
			"%s set blocksize failed! eno = %d\n", file_name, err);
#ifdef CONFIG_DFX_DEBUG_FS
		rdr_syserr_process_for_ap((u32)MODID_AP_S_PANIC_STORAGE,
			0ull, 0ull);
#endif
		goto out;
	}

	hyperhold_space_set_zram(zram, bdev, nr_pages);

	return 0;
out:
	hyperhold_close_bdev(bdev, backing_dev);

	return err;
}

static int hyperhold_alloc_partition_space(struct zram *zram)
{
	return hyperhold_bind(zram, HP_PARTITION_PATH);
}

static void hyperhold_get_userdata_bdev(struct block_device **bdev)
{
	struct file *backing_dev = NULL;
	struct inode *inode = NULL;

	backing_dev = hyperhold_open_bdev(USERDATA_PARTITION_PATH, O_RDONLY);
	if (unlikely(!backing_dev))
		return;

	inode = backing_dev->f_mapping->host;
	*bdev = bdgrab(I_BDEV(inode));
}

static enum hp_sapce_type hyperhold_default_space_type(void)
{
#ifdef CONFIG_HP_DEFAULT_FILE_SPACE
	struct block_device *bdev = NULL;
	struct request_queue *q = NULL;

	hyperhold_get_userdata_bdev(&bdev);
	if (!bdev) {
		hh_print(HHLOG_ERR, "bdev is null\n");
		return HP_PATTION_SPACE;
	}

	q = bdev_get_queue(bdev);
	if (blk_queue_query_unistore_enable(q)) {
		hh_print(HHLOG_ERR, "unistore devices\n");
		return HP_PATTION_SPACE;
	}

	return HP_FILE_SPACE;
#else
	return HP_PATTION_SPACE;
#endif
}

void hyperhold_set_default_space_proc(void)
{
	struct blk_hp_set_space space_setting;
	struct space_info_para *space_info = hyperhold_space_info();

	if (hyperhold_get_cfg()) {
		hh_print(HHLOG_ERR, "cfg already set by ioctl\n");
		return;
	}

	space_setting.space_type = hyperhold_default_space_type();
	space_setting.max_size = DEFAULT_HP_FILE_SZ_MBYTE;
	if (space_setting.space_type != HP_FILE_SPACE) {
		spin_lock_init(&space_info->exts_lock);
		hh_print(HHLOG_DEBUG, "set type:%u\n", space_setting.space_type);
		return;
	}

	(void)hyperhold_get_file_para();
	hyperhold_save_space_setting(&space_setting);
	hyperhold_set_cfg();
}

static int hyperhold_set_space_proc(struct block_device *bdev,
	struct blk_hp_set_space *space_setting)
{
	int ret;

	if (hyperhold_get_cfg()) {
		hh_print(HHLOG_ERR, "space_para cfg again\n");
		return -EINVAL;
	}

	ret = hyperhold_check_space_setting(space_setting);
	if (ret)
		return ret;

	ret = hyperhold_file_para_init(bdev);
	if (ret)
		return ret;

	hyperhold_save_space_setting(space_setting);
	hyperhold_set_cfg();
	return 0;
}

int hyperhold_set_space(struct block_device *bdev,
	struct blk_hp_cmd *cmd)
{
	int ret;
	struct blk_hp_set_space space_setting;

	ret = hyperhold_get_space_setting(&space_setting, cmd);
	if (ret)
		return ret;

	return hyperhold_set_space_proc(bdev, &space_setting);
}

int hyperhold_free_space(struct block_device *bdev)
{
	int ret;

	if (hyperhold_enable()) {
		hh_print(HHLOG_ERR, "hyperhold enable, free fail\n");
		return -EINVAL;
	}

	ret = hyperhold_file_para_init(bdev);
	if (ret) {
		hh_print(HHLOG_ERR, "init fail\n");
		return ret;
	}

	hyperhold_free_file_proc(MAX_HP_FILE_NUM, 0);
	return 0;
}

int hyperhold_alloc_space(struct zram *zram)
{
	if (hyperhold_is_file_space())
		return hyperhold_alloc_file_space(zram);
	else
		return hyperhold_alloc_partition_space(zram);
}

void hyperhold_expend_space(void)
{
	if (hyperhold_is_file_space())
		hyperhold_expand_space_proc();
}

sector_t hyperhold_get_sector(int ext_id)
{
	if (hyperhold_is_file_space())
		return hyperhold_get_file_sector(ext_id);
	else
		return hyperhold_get_partition_sector(ext_id);
}

void hyperhold_space_sector_deinit(void)
{
	struct space_info_para *space_info = hyperhold_space_info();

	if (space_info->start_sector) {
		vfree(space_info->start_sector);
		space_info->start_sector = NULL;
	}
	if (space_info->exts_sum) {
		vfree(space_info->exts_sum);
		space_info->exts_sum = NULL;
	}
}

int hyperhold_space_sector_init(void)
{
	int ret;
	unsigned long loop;
	unsigned long file_num;
	char file_path[MAX_HP_FILE_PATH_LEN] = { 0 };
	struct space_info_para *space_info = hyperhold_space_info();

	if (!hyperhold_is_file_space())
		return 0;

	if (hyperhold_file_ops_check(space_info)) {
		hh_print(HHLOG_ERR, "hyperhold_file_ops_check fail\n");
		return -EFAULT;
	}

	space_info->start_sector = vzalloc(space_info->max_file_num * sizeof(sector_t));
	if (!space_info->start_sector) {
		hh_print(HHLOG_ERR, "start_sector alloc failed\n");
		return -ENOMEM;
	}

	file_num = space_info->alloced_file_num;
	for (loop = 0; loop < file_num; ++loop) {
		ret = hyperhold_get_file_path(file_path, loop);
		if (unlikely(ret)) {
			hh_print(HHLOG_ERR, "hyperhold_get_file_path %u fail \n", loop);
			hyperhold_space_sector_deinit();
			return -EFAULT;
		}

		ret = space_info->file_ops->get_sector(file_path,
			space_info->start_sector + loop);
		if (ret) {
			hh_print(HHLOG_ERR, "get %d start_sector failed\n", loop);
			hyperhold_space_sector_deinit();
			return -EFAULT;
		}
	}

	return 0;
}

