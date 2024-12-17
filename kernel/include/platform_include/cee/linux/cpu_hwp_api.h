// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2023-2023 Huawei Technologies Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#ifndef __CPU_HWP_API_H__
#define __CPU_HWP_API_H__

#include <linux/types.h>

#ifdef CONFIG_CPU_HWP_CTRL
int perf_ctrl_set_task_cpu_hwp(void __user *uarg);
#else
static inline int perf_ctrl_set_task_cpu_hwp(void __user *uarg)
{
	return 0;
}
#endif /* CONFIG_CPU_HWP_CTRL */

#endif /* __CPU_HWP_API_H__ */
