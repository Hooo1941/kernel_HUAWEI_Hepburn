/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */


#ifndef WQ_DEFS_H
#define WQ_DEFS_H

#include "linux/types.h"

#define WQ_OPT_NUMS 4
#define WQ_QUEUE_LENGTH 512
#define WQ_JIT_NUMS 6

typedef enum {
	WQ_OPT_NOP = 0,
	WQ_OPT_ALLOC_JIT_MEMORY,
	WQ_OPT_FREE_JIT_MEMORY,
	WQ_OPT_BIND_SPARSE_MEMORY,
	WQ_OPT_UNBIND_SPARSE_MEMORY,
	WQ_OPT_WAIT_FENCE,
	WQ_OPT_TRIGGER_FENCE,
	WQ_OPT_WAIT_SEMAPHORE,
	WQ_OPT_SET_SEMAPHORE,
	WQ_OPT_SET_SEMAPHORE_AND_NOTIFY,
	WQ_OPT_RESET_SEMAPHORE,
	WQ_OPT_BARRIER,
	WQ_OPT_SET_ATOMIC_SEMAPHORE,
	WQ_OPT_SET_ATOMIC_SEMAPHORE_AND_NOTIFY,
	WQ_OPT_WAIT_ATOMIC_SEMAPHORE,
	WQ_OPT_END,
} wq_opt_cmd;

typedef struct {
	uint32_t num;
	uint64_t mem_addr_pointer[WQ_JIT_NUMS];
	uint32_t mem_request_size[WQ_JIT_NUMS];
} wq_opt_para_jit_memory;

typedef struct {
	uint64_t sparse_addr;
	uint64_t mem_addr;
	uint64_t pages;
} wq_opt_para_sparse_mem;

typedef struct {
	uint64_t sem_addr;
	uint64_t sem_value;
	uint32_t token;
	int fd;
} wq_opt_para_fence;

typedef struct {
	uint64_t sem_addr;
	uint64_t sem_value;
	uint32_t token;
} wq_opt_para_semaphore;

typedef struct {
	uint64_t sem_addr;
	uint64_t sem_value;
	uint64_t wait_value;
	uint32_t token;
} wq_opt_para_atomic_semaphore;

typedef struct {
	wq_opt_cmd cmd;

	/* The follow parameters select by wq_opt_cmd cmd */
	union {
		wq_opt_para_jit_memory jit_memory;
		wq_opt_para_sparse_mem sparse_memory;
		wq_opt_para_fence fence;
		wq_opt_para_semaphore semaphore;
		wq_opt_para_atomic_semaphore atomic_semp;
	} paras;
} wq_work_opt;

typedef struct {
	uint32_t run_counter;   /* How many opts had been deal. */
	wq_work_opt opts[WQ_OPT_NUMS];
} wq_work_entry;

typedef struct {
	volatile uint32_t wr_idx;
	volatile uint32_t rd_idx;
	volatile uint32_t allocated_idx;
	uint64_t queue_base;
} wq_context;

#endif
