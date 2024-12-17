/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023. All rights reserved.
 *
 * Description: Debug improper memory alloc behaviours
 * Author: Li Xilun <lixilun1@huawei.com>
 * Create: 2023-03-23
 */

#ifndef _IMPROPER_ALLOC_H
#define _IMPROPER_ALLOC_H

#include <linux/gfp.h>
#include <linux/seq_file.h>

void improper_alloc_show(struct seq_file *m);
void improper_alloc_hook(gfp_t gfp_mask, unsigned int order, size_t size);

#endif
