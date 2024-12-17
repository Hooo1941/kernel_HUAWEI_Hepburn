/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */
#ifndef RESERVED_MEMPOOOL_H
#define RESERVED_MEMPOOOL_H

#include <linux/kernel.h>
#include <linux/types.h>

int load_reserved_mem(void);
void unmap_res_mem(void);
void *reserved_mem_alloc(size_t size);
void free_reserved_mempool(void);
int reserved_mempool_init(void);
void reserved_mem_free(const void *ptr);
bool exist_res_mem(void);
unsigned long res_mem_virt_to_phys(unsigned long vaddr);
unsigned int get_res_mem_slice_size(void);
#endif
