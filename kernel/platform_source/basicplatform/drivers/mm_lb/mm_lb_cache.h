/*
 * Copyright (c) 2022-2022 Huawei Technologies Co., Ltd.
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

#ifndef __MM_LB_CACHE_H
#define __MM_LB_CACHE_H

#include <linux/mm_types.h>

int gid_bypass_control(u32 cmd, u32 gid, u32 quota, u32 plc);
int power_control(u32 flag);
int sec_gid_set(u32 cmd, u32 gid, u32 quota);
unsigned int get_lb_lite(void);
unsigned int get_lb_efuse(void);

#endif
