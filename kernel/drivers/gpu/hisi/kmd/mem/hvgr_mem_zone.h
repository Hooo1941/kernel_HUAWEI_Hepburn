/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#ifndef HVGR_MEM_ZONE_H
#define HVGR_MEM_ZONE_H

#include "hvgr_defs.h"
/* 32bit app gpu mem addr manage */
/* replay mem: 0x1 0000 0000 ~ 0x3 FFFF FFFF */
#define HVGR_MEM_REPLAY_START ((uint64_t)0x100000000)
#define HVGR_MEM_REPLAY_SIZE  ((uint64_t)12 * 1024 * 1024 * 1024)

/* jit mem: 0x4 0000 0000 ~ 0x4 FFFF FFFF */
#define HVGR_MEM_JIT_START (HVGR_MEM_REPLAY_START + HVGR_MEM_REPLAY_SIZE)
#define HVGR_MEM_JIT_SIZE  ((uint64_t)4 * 1024 * 1024 * 1024)

/* 2M mem: 0x5 0000 0000 ~ 0x5 1FFF FFFF */
#define HVGR_MEM_2M_START (HVGR_MEM_JIT_START + HVGR_MEM_JIT_SIZE)
#define HVGR_MEM_2M_SIZE  ((uint64_t)512 * 1024 * 1024)

/* import mem: 0x5 2000 0000 ~  0x5 9FFF FFFF */
#define HVGR_MEM_IMPORT_START (HVGR_MEM_2M_START + HVGR_MEM_2M_SIZE)
#define HVGR_MEM_IMPORT_SIZE  ((uint64_t)2 * 1024 * 1024 * 1024)


/* gpu mem: 0x5 A000 0000 ~ 0x7F FFFF FFFF */
#define HVGR_MEM_GPU_START (HVGR_MEM_IMPORT_START + HVGR_MEM_IMPORT_SIZE)
#define HVGR_MEM_GPU_SIZE  (((uint64_t)1 << 39) - HVGR_MEM_GPU_START)

bool hvgr_mem_zone_init_proc(struct hvgr_mem_ctx * const pmem);

void hvgr_mem_zone_term_proc(struct hvgr_mem_ctx * const pmem);

bool hvgr_mem_zone_create(struct hvgr_mem_ctx * const pmem,
	uint64_t prot, struct hvgr_zone_opts const * const opts,
	void * const para);

void hvgr_mem_zone_destroy(struct hvgr_mem_ctx * const pmem,
	uint64_t prot);

struct hvgr_mem_zone *hvgr_mem_zone_get_zone(
	struct hvgr_mem_ctx * const pmem, uint64_t prot);

bool hvgr_mem_zone_alloc_memory(struct hvgr_mem_zone * const zone,
	struct hvgr_mem_area * const area);

void hvgr_mem_zone_free_memory(struct hvgr_mem_area * const area);

bool hvgr_mem_zone_import_memory(struct hvgr_mem_zone * const zone,
	struct hvgr_mem_area * const area, int fd);

struct hvgr_mem_area *hvgr_mem_zone_find_area_by_gva(
	struct hvgr_mem_ctx * const pmem, uint64_t gva);

struct hvgr_mem_area *hvgr_mem_zone_find_area_by_gva_range(
	struct hvgr_mem_ctx * const pmem, uint64_t gva);

struct hvgr_mem_area *hvgr_mem_zone_find_area_by_uva(
	struct hvgr_mem_ctx * const pmem, uint64_t uva);

struct hvgr_mem_area *hvgr_mem_zone_find_area_by_uva_range(
	struct hvgr_mem_ctx * const pmem, uint64_t uva);
#endif
