/*
 * memcheck_stack.h
 *
 * save and read stack information
 *
 * Copyright (c) 2021-2022 Huawei Technologies Co., Ltd.
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

#ifndef _MEMCHECK_STACK_H
#define _MEMCHECK_STACK_H

#include <linux/types.h>
#include <platform/linux/memcheck.h>

#ifdef CONFIG_DFX_MEMCHECK_STACK
int memcheck_do_command(const struct track_cmd *cmd);
int memcheck_stack_read(void *buf, struct stack_info *info);
int memcheck_stack_write(const void *buf, const struct stack_info *info);
int memcheck_stack_clear(void);
int memcheck_wait_stack_ready(u16 type);
#else /* CONFIG_DFX_MEMCHECK_STACK */
static int memcheck_do_command(const struct track_cmd *cmd)
{
	return -EINVAL;
}
static inline int memcheck_stack_read(void *buf, struct stack_info *info)
{
	return -EINVAL;
}
static inline int memcheck_stack_write(const void *buf,
				       const struct stack_info *info)
{
	return -EINVAL;
}
static inline int memcheck_stack_clear(void)
{
	return -EINVAL;
}
static inline int memcheck_wait_stack_ready(u16 type)
{
	return -EINVAL;
}
#endif /* CONFIG_DFX_MEMCHECK_STACK */
#endif /* _MEMCHECK_STACK_H */
