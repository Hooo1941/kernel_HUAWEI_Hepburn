/*
 * fbe_hibernate.h
 *
 * fbe hibernate driver header
 *
 * Copyright (c) 2024-2024 Huawei Technologies Co., Ltd.
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
#ifndef _FBE_HIBERNATE_H_
#define _FBE_HIBERNATE_H_

#include <linux/types.h>

extern u32 hibernate_get_slot(u32 *slot);

#endif // _FBE_HIBERNATE_H_
