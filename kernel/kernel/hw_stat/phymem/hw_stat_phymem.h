/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: get physical memory info
 * Author: Qin Hongwei <qinhongwei7@huawei.com>
 * Create: 2023-02-18
 */
#ifndef __HW_STAT_MEM_H
#define __HW_STAT_MEM_H

#include <linux/hw_stat.h>
#include <linux/seq_file.h>

struct physical_mem_stat {
	unsigned long poisoned;
	unsigned long corrupted;
	unsigned long ksm;
	unsigned long anon; // Number of anonymous pages
	unsigned long file; // Number of file backed page cache pages
	unsigned long null_mapping;
	unsigned long f2fs_file; // Number of F2FS file cache pages
	unsigned long ext4_file; // Number of EXT4 file cache pages
	unsigned long erofs_file; // Number of EROFS file cache pages
};

void hw_stat_init_phymem(struct proc_dir_entry *dir);
#endif
