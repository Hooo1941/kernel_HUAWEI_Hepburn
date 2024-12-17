/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2001-2023. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#include <linux/sched/rt.h>
#include <linux/mm.h>
#include <chipset_common/linux/mm_policy.h>

unsigned int hw_mm_policy __read_mostly;
unsigned int get_hw_mm_policy()
{
	return hw_mm_policy;
}
EXPORT_SYMBOL(get_hw_mm_policy);

/* cache buffer for display */
static atomic_long_t g_bufptr = ATOMIC_LONG_INIT(0);

u16 *get_display_buffer(u32 size, bool force)
{
	u16 *buf = force ? (u16 *)atomic_long_xchg(&g_bufptr, 0) : NULL;
	return buf ? buf : kvzalloc(size, GFP_KERNEL);
}
EXPORT_SYMBOL(get_display_buffer);

u16 *put_display_buffer(u16 *buf, bool force)
{
	return force ? (u16 *)atomic_long_xchg(&g_bufptr,
	    (long)(uintptr_t)buf) : buf;
}
EXPORT_SYMBOL(put_display_buffer);
