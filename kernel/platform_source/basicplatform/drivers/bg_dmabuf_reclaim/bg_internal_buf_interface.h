/*
 * bg_internal_buf_interface.h
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
#ifndef _BG_INTERNAL_BUF_INTERFACE_H
#define _BG_INTERNAL_BUF_INTERFACE_H

#include <linux/bg_dmabuf_reclaim/bg_dmabuf_info_manager.h>

#define bg_buf_pr_err(fmt, args...)	pr_err("[bg_buffer] ERR " fmt, ##args)

#define bg_buf_pr_info(fmt, args...)	pr_info("[bg_buffer] INFO " fmt, ##args)

#ifdef CONFIG_BG_DMABUF_TEST
extern bool bg_debug_print_enable;
#define bg_buf_pr_debug(fmt, args...) ({ \
	if (bg_debug_print_enable) \
		pr_info("[bg_buffer] DBG " fmt, ##args); \
})
#else
#define bg_buf_pr_debug(fmt, args...) ({ })
#endif

/**
 * struct bg_buffer_ops -  background buffer ops and capabilities, these ops must implement
 * @get_info: get bg_buffer va info struct
 * @set_info: set bg_buffer va info struct
 * @is_buffer_valid: check bg_buffer is valid or not, such as not system heap buffer
 * @get_buffer_ptr_from_dmabuf: get bg_buffer from dmabuf
 * @buffer_lock: bg_buffer lock operation
 * @buffer_unlock: bg_buffer lock operation
 * @get_size: get bg_buffer size
 * @alloc_memory: bg_buffer alloc memory, used by bg_realloc func
 * @free_memory: bg_buffer free memory, used by bg_reclaim func
 * @get_sg_table: get bg_buffer memory sg_table struct
 * @get_kmap_prot: get kmap prot, used by bg_realloc func
 */
struct bg_buffer_ops {
	struct bg_dmabuf_info *(*get_info)(void *ptr);
	void (*set_info)(void *ptr, struct bg_dmabuf_info *info);
	bool (*is_buffer_valid)(void *ptr);
	void *(*get_buffer_ptr_from_dmabuf)(struct dma_buf *dmabuf);
	void (*buffer_lock)(void *ptr);
	void (*buffer_unlock)(void *ptr);
	size_t (*get_size)(void *ptr);
	int (*alloc_memory)(void *ptr);
	void (*free_memory)(void *ptr);
	struct sg_table *(*get_sg_table)(void *ptr);
	pgprot_t (*get_kmap_prot)(void *ptr);
};

struct bg_dmabuf_info *get_info_from_buffer(void *ptr);
void set_info_to_buffer(void *ptr, struct bg_dmabuf_info *info);
bool is_buffer_valid(void *ptr);
void *get_buffer_ptr_from_dmabuf(struct dma_buf *dmabuf);
void buffer_lock(void *ptr);
void buffer_unlock(void *ptr);
size_t get_buffer_size(void *ptr);
int buffer_alloc_memory(void *ptr);
void buffer_free_memory(void *ptr);
struct sg_table *get_buffer_sg_table(void *ptr);
void buffer_invalid_attachment(void *ptr);
void buffer_restore_attachment(void *ptr);
pgprot_t get_buffer_kmap_prot(void *ptr);
void internal_buffer_init(void);

#endif /* _BG_INTERNAL_BUF_INTERFACE_H */
