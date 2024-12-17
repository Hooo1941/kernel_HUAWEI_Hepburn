/*
 * memcheck_log_gpumem.h
 *
 * Get gpumem info function
 *
 * Copyright (c) 2022 Huawei Technologies Co., Ltd.
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

#ifndef _MEMCHECK_LOG_GPUMEM_H
#define _MEMCHECK_LOG_GPUMEM_H

int memcheck_gpumem_createfs(void);
void memcheck_gpumem_info_show(void);
int memcheck_gpumem_get_usage(pid_t pid, u64 *mapped, u64 *unmapped);

#endif /* _MEMCHECK_LOG_GPUMEM_H */
