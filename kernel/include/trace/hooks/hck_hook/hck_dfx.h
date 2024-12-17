/*
 * hck_dfx.h
 *
 * hck dfx module header
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
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
#ifndef HCK_DFX_H
#define HCK_DFX_H

#include <linux/types.h>
#include <stdarg.h>
#include <linux/printk.h>

static inline void _hck_dfx_set_args(u64 *args, u32 args_num, ...)
{
	va_list valist;
	u32 i;
	va_start(valist, args_num);
	for (i = 0; i < args_num; i++)
		args[i] = va_arg(valist, u64);
	va_end(valist);
}

#define trace_hck_dfx(mdu, sub_name, args_num, ...) \
	do { \
		u64 args[args_num]; \
		_hck_dfx_set_args(args, args_num, ## __VA_ARGS__); \
		trace_##mdu##_##sub_name(args, args_num); \
	} while (0)

#define register_trace_hck_dfx(mdu, sub_name) \
	do { \
		register_trace_##mdu##_##sub_name(_##mdu##_##sub_name, NULL); \
	} while (0)

#define hck_dfx_set_func(mdu, sub_name) \
	_##mdu##_##sub_name(void *data, u64 *args, u32 args_num)

#define DECLARE_HCK_HOOK_DFX(mdu, sub_name) \
	DECLARE_HCK_HOOK(mdu##_##sub_name, \
			TP_PROTO(u64 *args, u32 args_num), \
			TP_ARGS(args, args_num));

#endif