/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023. All rights reserved.
 *
 * Description: Debug improper memory alloc behaviours
 * Author: Li Xilun <lixilun1@huawei.com>
 * Create: 2023-03-23
 */

#ifndef _IMPROPER_ALLOC_FAULT_H
#define _IMPROPER_ALLOC_FAULT_H

#include <linux/sysctl.h>
#include <linux/stddef.h>

extern int g_fault_thread_running;
int improper_fault_handler(struct ctl_table *ctl, int write,
			   void *buffer, size_t *lenp, loff_t *ppos);

#endif
