/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#ifndef HVGR_CQ_DEFS_H
#define HVGR_CQ_DEFS_H

#include "linux/types.h"
#include "apicmd/hvgr_ioctl_mem.h"

#define CQ_QUEUE_ID_INVALID                          0
#define CQ_CHANNEL_NUM                               2
#define CQ_CHANNEL0                                  0
#define CQ_CHANNEL1                                  1
#define CQ_CHANNEL_ID_INVALID                        ((uint32_t)-1)
#define CQ_QUEUE_NUM_PER_CTX                         2

#define CQ_HW_WRITEABLE_CONTEXT_SIZE                 (8 * 1024)

#define CQ_ERR_FLOW_TOKEN_VALUE              0xFFFFFFFFU

#define TOKEN_NUMS    4096u
#define CQ_SYNC_TOKEN_NUMS    TOKEN_NUMS

#define CQ_ERROR              1
#define CQ_ERROR_NONE         0

enum cq_irq_counter_idx {
	CQ_IRQ_COUNTER_DONE = 0,
	CQ_IRQ_COUNTER_SVD,
	CQ_IRQ_COUNTER_EMPTY_TIMEOUT,
	CQ_IRQ_COUNTER_SEMAPHORE_TIMEOUT,
	CQ_IRQ_COUNTER_STOP,
	CQ_IRQ_COUNTER_MAX,
};

/* The cq_status share between KMD and UMD */
struct hvgr_cq_status {
	uint32_t err_flags;
};

struct cq_ring_buf {
	uint64_t reserve1;
	uint64_t ring_buf_w_ptr;
	uint64_t ring_buf_r_ptr;
	uint8_t reserve2[40]; /* reserve 320bits */

	uint8_t jcd_array[4][64]; /* Jcd buffer size is 512bits * 4 */

	/* Follow memory is ptr_buf array */
};

/* Hardware need start address align to 64Bytes */
#define CQ_RINGBUF_ALIGN_BYTES 64u
#define cq_ringbuf_align(size) \
	(((size) + (CQ_RINGBUF_ALIGN_BYTES - 1u)) & (~(CQ_RINGBUF_ALIGN_BYTES - 1u)))

/* Job ptr_buffer size is 256bits, unit is byte */
#define CQ_RINGBUF_ENTRY_SIZE 32u

#define CQ_REND_RING_ENTRY_NUMS     16384u
#define CQ_REND_RING_BUF_SIZE \
	cq_ringbuf_align(sizeof(struct cq_ring_buf) + \
		CQ_RINGBUF_ENTRY_SIZE * CQ_REND_RING_ENTRY_NUMS)

#define CQ_BIN_RING_ENTRY_NUMS      1024u
#define CQ_BIN_RING_BUF_SIZE \
	cq_ringbuf_align(sizeof(struct cq_ring_buf) + \
		CQ_RINGBUF_ENTRY_SIZE * CQ_BIN_RING_ENTRY_NUMS)

#define CQ_COMPUTER_RING_ENTRY_NUMS 128u
#define CQ_COMPUTER_RING_BUF_SIZE \
	cq_ringbuf_align(sizeof(struct cq_ring_buf) + \
		CQ_RINGBUF_ENTRY_SIZE * CQ_COMPUTER_RING_ENTRY_NUMS)

#define CQ_BVHG_RING_ENTRY_NUMS     128u
#define CQ_BVHG_RING_BUF_SIZE \
	cq_ringbuf_align(sizeof(struct cq_ring_buf) + \
		CQ_RINGBUF_ENTRY_SIZE * CQ_BVHG_RING_ENTRY_NUMS)

#define CQ_RT_RING_ENTRY_NUMS       8192u
#define CQ_RT_RING_BUF_SIZE \
	cq_ringbuf_align(sizeof(struct cq_ring_buf) + CQ_RINGBUF_ENTRY_SIZE * CQ_RT_RING_ENTRY_NUMS)

#define CQ_CMD_ENTRY_BUFFER_MEM_PROP HVGR_MEM_PROPERTY_SAME_VA

struct cq_sw_writeable_context {
	uint64_t hw_context_address;
	uint64_t queue_base_address;
	uint32_t queue_max_num_entries;
	uint32_t queue_put_index;
	uint32_t queue_get_index;
	uint32_t priority;

	uint8_t reserve1[32]; /* reserve 256bit */

	struct {
		uint64_t rend_ring_buf_struct_ptr;
		uint32_t rend_ring_buf_struct_size;
		uint32_t reserve0;

		uint64_t bin_ring_buf_struct_ptr;
		uint32_t bin_ring_buf_struct_size;
		uint32_t reserve1;

		uint64_t comp_ring_buf_struct_ptr;
		uint32_t comp_ring_buf_struct_size;
		uint32_t reserve2;

		uint64_t bvhg_ring_buf_struct_ptr;
		uint32_t bvhg_ring_buf_struct_size;
		uint32_t reserve3;

		uint64_t rt_ring_buf_struct_ptr;
		uint32_t rt_ring_buf_struct_size;
		uint32_t reserve4;
	} ring_buffer;
};

enum cq_priority {
	CQ_PRIORITY_LOW = 0,
	CQ_PRIORITY_HIGH,
	CQ_PRIORITY_SPECIAL_LOW,
	CQ_PRIORITY_MAX,
};

#endif
