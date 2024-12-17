/*
 * mm_hisi/pagecache_manage_interface.h
 *
 * Copyright(C) 2004-2020 Hisilicon Technologies Co., Ltd. All rights reserved.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __HISI_PCH_MANAGE_INTERFACE_H__
#define __HISI_PCH_MANAGE_INTERFACE_H__
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/workqueue.h>
#include <linux/jbd2.h>
#include <linux/cleancache.h>
#include <../fs/ext4/ext4.h>

extern void __cfi_pch_timeout_work(struct work_struct *work);
extern void pch_timeout_work(struct work_struct *work);
extern void __cfi_pch_timeout_timer(struct timer_list *timer);
extern void pch_timeout_timer(struct timer_list *timer);
extern int __cfi_pch_emui_bootstat_proc_show(struct seq_file *m, void *v);
extern int pch_emui_bootstat_proc_show(struct seq_file *m, void *v);
extern int __cfi_pch_emui_bootstat_proc_open(struct inode *p_inode, struct file *p_file);
extern int pch_emui_bootstat_proc_open(struct inode *p_inode, struct file *p_file);
extern ssize_t __cfi_pch_emui_bootstat_proc_write(struct file *p_file,
		const char __user *userbuf, size_t count, loff_t *ppos);
extern ssize_t pch_emui_bootstat_proc_write(struct file *p_file,
		const char __user *userbuf, size_t count, loff_t *ppos);
extern int __init __cfi_hisi_page_cache_manage_init(void);
extern int __init hisi_page_cache_manage_init(void);
extern void __exit __cfi_hisi_page_cache_manage_exit(void);
extern void __exit hisi_page_cache_manage_exit(void);
extern inline bool ext4_need_verity(const struct inode *inode, pgoff_t idx);
extern inline loff_t ext4_readpage_limit(struct inode *inode);
extern void ext4_submit_bio_read(struct bio *bio);
extern int ext4_map_blocks(handle_t *handle, struct inode *inode,
		    struct ext4_map_blocks *map, int flags);
extern void ext4_set_bio_post_read_ctx(struct bio *bio, const struct inode *inode,
				pgoff_t first_idx);
extern void mpage_end_io(struct bio *bio);

#endif /* __HISI_PCH_MANAGE_INTERFACE_H__ */
