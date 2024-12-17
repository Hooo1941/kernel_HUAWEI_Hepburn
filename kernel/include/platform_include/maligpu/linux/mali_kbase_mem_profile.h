/*
  * Copyright (c) Hisilicon Technologies Co., Ltd. 2023. All rights reserved.
  *
  * This program is free software and is provided to you under the terms of the
  * GNU General Public License version 2 as published by the Free Software
  * Foundation, and any use by you of this program is subject to the terms
  * of such GNU licence.
  */

#ifndef _KBASE_MEM_PROFILE_H_
#define _KBASE_MEM_PROFILE_H_

#include <linux/types.h>
#include <linux/seq_file.h>

int kbasep_mem_profile_all_show(struct seq_file *sfile, void *data);

unsigned long kbase_mem_statics_used_pages(void);

#endif
