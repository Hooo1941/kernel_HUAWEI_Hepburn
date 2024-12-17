/*
 * bg_internal_buf_interface.c
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

#include "bg_internal_buf_ion.h"

struct bg_buffer_ops dummy_buffer_ops;
struct bg_buffer_ops *buffer_ops = &dummy_buffer_ops;
/* debug print enable, modify by ecall func */
bool bg_debug_print_enable = false;

void bg_enable_debug_print(bool is_enable)
{
	bg_debug_print_enable = is_enable;
}

struct bg_dmabuf_info *get_info_from_buffer(void *ptr)
{
	if (buffer_ops->get_info)
		return buffer_ops->get_info(ptr);
	else
		return NULL;
}

void set_info_to_buffer(void *ptr, struct bg_dmabuf_info *info)
{
	if (buffer_ops->set_info)
		buffer_ops->set_info(ptr, info);
}

bool is_buffer_valid(void *ptr)
{
	if (buffer_ops->is_buffer_valid)
		return buffer_ops->is_buffer_valid(ptr);
	else
		return false;
}

void *get_buffer_ptr_from_dmabuf(struct dma_buf *dmabuf)
{
	if (buffer_ops->get_buffer_ptr_from_dmabuf)
		return buffer_ops->get_buffer_ptr_from_dmabuf(dmabuf);
	else
		return NULL;
}

void buffer_lock(void *ptr)
{
	if (buffer_ops->buffer_lock)
		buffer_ops->buffer_lock(ptr);
}

void buffer_unlock(void *ptr)
{
	if (buffer_ops->buffer_unlock)
		buffer_ops->buffer_unlock(ptr);
}

size_t get_buffer_size(void *ptr)
{
	if (buffer_ops->get_size)
		return buffer_ops->get_size(ptr);
	else
		return 0;
}

int buffer_alloc_memory(void *ptr)
{
	if (buffer_ops->alloc_memory)
		return buffer_ops->alloc_memory(ptr);
	else
		return -EINVAL;
}

void buffer_free_memory(void *ptr)
{
	if (buffer_ops->free_memory)
		buffer_ops->free_memory(ptr);
}

struct sg_table *get_buffer_sg_table(void *ptr)
{
	if (buffer_ops->get_sg_table)
		return buffer_ops->get_sg_table(ptr);
	else
		return NULL;
}

pgprot_t get_buffer_kmap_prot(void *ptr)
{
	if (buffer_ops->get_kmap_prot) {
		return buffer_ops->get_kmap_prot(ptr);
	} else {
		bg_buf_pr_err("%s error!, kmap_prot must register\n", __func__);
		return pgprot_writecombine(PAGE_KERNEL);
	}
}

void internal_buffer_init(void)
{
#ifdef CONFIG_ION
	internal_ion_buffer_init(&buffer_ops);
#endif
}

