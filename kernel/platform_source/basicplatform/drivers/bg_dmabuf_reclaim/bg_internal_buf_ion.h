/*
 * bg_internal_buf_ion.h
 *
 * Copyright (C) 2022 Hisilicon Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#ifndef _BG_INTERNAL_BUF_ION_H
#define _BG_INTERNAL_BUF_ION_H

#include "bg_internal_buf_interface.h"

#ifdef CONFIG_ION
void internal_ion_buffer_init(struct bg_buffer_ops **ops);
#endif /* CONFIG_ION */

#endif /* _BG_INTERNAL_BUF_ION_H */
