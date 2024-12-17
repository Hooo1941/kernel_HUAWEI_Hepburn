/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#ifndef HVGR_MEM_PROFILE_H
#define HVGR_MEM_PROFILE_H

#include <linux/types.h>
#include <linux/seq_file.h>

int hvgr_mem_profile_all_show(struct seq_file *sfile, void *data);

unsigned long hvgr_mem_statics_used_pages(void);

#endif
