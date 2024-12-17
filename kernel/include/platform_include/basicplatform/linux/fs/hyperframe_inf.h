/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022. All rights reserved.
 * Description: hyperframe info file
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
 * Create: 2022-06-16
 *
 */

#ifndef _LINUX_HF_INF_H
#define _LINUX_HF_INF_H

struct hyperframe_cfg {
	unsigned int hf_size;
	unsigned int hf_health;
	int hf_max_usage;
	int hf_ctx_alloc;
	int hf_req_alloc;
	struct workqueue_struct *async_wq_read;
	struct workqueue_struct *async_wq_write;
	struct workqueue_struct *async_wq_cq;
};

struct hyperframe_ctx {
	struct delayed_work work_read;
	struct delayed_work work_write;
	struct delayed_work work_cq;
	struct list_head list_read;
	struct list_head list_write;
	struct list_head list_cq;
	spinlock_t lock_read;
	spinlock_t lock_write;
	spinlock_t lock_cq;
	atomic_t read_cnt;
	atomic_t write_cnt;
	atomic_t cq_cnt;
	atomic_t cq_comp_cnt;
	bool hf_enable;
};

struct hyperframe_req {
	struct io_kiocb *req;
	struct kiocb *kiocb;
	u64 addr;
	struct page **pages;
	struct hyperframe_ctx *hf_ctx;
	struct dma_buf *dma_buf;
	struct dma_buf_attachment *attach;
	struct sg_table *sgt;
	struct list_head req_list;
	atomic_t in_list;
	unsigned long long open_size;
	void __user *size_addr;
	unsigned long rw_size;
	u64 cancel;
	loff_t ion_fd;
	int file_fd;
	ssize_t ret;
};

struct hyperframe_info {
	u64 fsize;
};

static inline bool hyperframe_rw(u8 opcode)
{
	return (opcode == IORING_OP_HYPERFRAME_READ ||
			opcode == IORING_OP_HYPERFRAME_WRITE);
}

static inline bool hyperframe_opcode(u8 opcode)
{
	return (opcode >= IORING_OP_HYPERFRAME_OPEN &&
			opcode <= IORING_OP_HYPERFRAME_CLOSE);
}

#endif /* _LINUX_HF_INF_H */
