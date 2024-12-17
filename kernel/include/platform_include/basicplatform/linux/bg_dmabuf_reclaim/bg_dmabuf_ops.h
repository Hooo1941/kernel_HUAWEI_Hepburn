/*
 * bg_dmabuf_ops.h
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
 */

#ifndef _BG_DMABUF_OPS_H
#define _BG_DMABUF_OPS_H

#include <linux/sysfs.h>
#include <linux/mm_types.h>
#include <linux/dma-buf.h>

enum bg_buf_cb_type {
	BG_BUF_CB_GPU_MAP = 0,
	BG_BUF_CB_GPU_UNMAP = 1,
	BG_BUF_CB_MAX
};

struct bg_buf_gpu_callback {
	int (*cb)(struct dma_buf *dmabuf);
};

#ifdef CONFIG_BG_DMABUF_RECLAIM
bool is_bg_buffer(void *buffer);
void bg_buffer_destroy(void *buffer);
int bg_buffer_register_callback(enum bg_buf_cb_type type,
	int (*cb)(struct dma_buf *dmabuf));
void register_bg_buffer_operation(struct kobject *kobj);
void bg_buffer_register_vma_ops(void *buffer, struct vm_area_struct *vma);
bool is_bg_dmabuf(struct dma_buf *dmabuf);
bool is_bg_dmabuf_release(struct dma_buf *dmabuf);
int bg_op_reclaim_buffer(struct dma_buf *dmabuf);
int bg_op_realloc_buffer(struct dma_buf *dmabuf);
bool bg_dmabuf_down_read(struct dma_buf *dmabuf);
void bg_dmabuf_up_read(struct dma_buf *dmabuf);
int bg_buffer_pin_reclaim(struct dma_buf *dmabuf);
int bg_buffer_unpin_reclaim(struct dma_buf *dmabuf);
#else
static inline bool is_bg_buffer(void *buffer)
{
	return false;
}
static inline void bg_buffer_destroy(void *buffer)
{
}
static inline int bg_buffer_register_callback(enum bg_buf_cb_type type,
	int (*cb)(struct dma_buf *dmabuf))
{
	return 0;
}
static inline void register_bg_buffer_operation(struct kobject *kobj)
{
}
static inline void bg_buffer_register_vma_ops(void *buffer,
	struct vm_area_struct *vma)
{
}
static inline bool is_bg_dmabuf(struct dma_buf *dmabuf)
{
	return false;
}
static inline bool is_bg_dmabuf_release(struct dma_buf *dmabuf)
{
	return false;
}
static inline int bg_op_reclaim_buffer(struct dma_buf *dmabuf)
{
	return 0;
}
static inline int bg_op_realloc_buffer(struct dma_buf *dmabuf)
{
	return 0;
}
static inline bool bg_dmabuf_down_read(struct dma_buf *dmabuf)
{
	return false;
}
static inline void bg_dmabuf_up_read(struct dma_buf *dmabuf)
{
}
static inline int bg_buffer_pin_reclaim(struct dma_buf *dmabuf)
{
	return 0;
}
static inline int bg_buffer_unpin_reclaim(struct dma_buf *dmabuf)
{
	return 0;
}
#endif /* CONFIG_BG_DMABUF_RECLAIM */

#endif /* _BG_DMABUF_OPS_H */
