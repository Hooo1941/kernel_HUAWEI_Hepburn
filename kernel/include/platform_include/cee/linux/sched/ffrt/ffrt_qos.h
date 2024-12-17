/* SPDX-License-Identifier: GPL-2.0 */
/*
 * ffrt qos ctrl interface
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef _FFRT_QOS_H
#define _FFRT_QOS_H
#include <linux/sched.h>
#include <linux/fs.h>

long do_qos_ctrl_ioctl(struct file *file, unsigned int cmd, unsigned long arg);

#endif /* _FFRT_QOS_H */
