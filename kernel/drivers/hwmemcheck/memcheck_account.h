/*
 * memcheck_account.h
 *
 * Get account memory infomation
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

#ifndef _MEMCHECK_ACCOUNT_H
#define _MEMCHECK_ACCOUNT_H

struct kmem_cache;

#ifdef CONFIG_DFX_MEMCHECK
int memcheck_createfs(void);
void memcheck_stats_show(void);
void memcheck_slub_obj_report(struct kmem_cache *s);
void memcheck_lowmem_report(struct task_struct *p, unsigned long total);

#else /* CONFIG_DFX_MEMCHECK */
static inline int memcheck_createfs(void)
{
	return -EINVAL;
}
static inline void memcheck_stats_show(void)
{
}
static inline void memcheck_slub_obj_report(struct kmem_cache *s)
{
}
static inline void memcheck_lowmem_report(struct task_struct *p, unsigned long total)
{
}
#endif /* CONFIG_DFX_MEMCHECK */
#endif /* _MEMCHECK_ACCOUNT_H */
