/*
 * ulsr.h
 *
 * ulsr function
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
 *
 */

#ifndef __H_ULSR_H__
#define __H_ULSR_H__

#ifdef CONFIG_SYS_PM_ULSR

#include <linux/types.h>

bool is_ulsr(void);

#else

static inline bool is_ulsr(void) { return false; }

#endif /* CONFIG_SYS_PM_ULSR */

#endif /* __H_ULSR_H__ */
