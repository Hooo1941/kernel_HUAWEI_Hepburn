/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#ifndef HVGR_IOCTL_MEM_H
#define HVGR_IOCTL_MEM_H

#include "linux/types.h"
#include "apicmd/hvgr_ioctl.h"

#define HVGR_IOCTL_CMD_ALLOC_MEM        (0u)
#define HVGR_IOCTL_CMD_FREE_MEM         (1u)
#define HVGR_IOCTL_CMD_MAP_MEM          (2u)
#define HVGR_IOCTL_CMD_UNMAP_MEM        (3u)
#define HVGR_IOCTL_CMD_IMPROT_MEM       (4u)
#define HVGR_IOCTL_CMD_SYNC_MEM         (5u)
#define HVGR_IOCTL_CMD_QUERY_MEM        (6u)
#define HVGR_IOCTL_CMD_RESIZE_MEM       (7u)
#define HVGR_IOCTL_CMD_GET_FD_SIZE_MEM  (8u)
#define HVGR_IOCTL_CMD_IMPORT_USER_MEM  (9u)
#define HVGR_IOCTL_CMD_ALIAS_MEM        (10u)
#define HVGR_IOCTL_CMD_ADD_PROFILE_MEM  (11u)
#define HVGR_IOCTL_CMD_UVA_OFFSET_MEM   (12u)
#define HVGR_IOCTL_CMD_SPARSE_BIND_MEM  (13u)
#define HVGR_IOCTL_CMD_SPARSE_UNBIND_MEM (14u)
#define HVGR_IOCTL_CMD_CHANGE_ATTR_MEM   (15u)
#define HVGR_IOCTL_CMD_CHANGE_PGID_ATTR  (16u)

#define HVGR_MEM_PROPERTY_GPU_MEM       (1u)
#define HVGR_MEM_PROPERTY_SAME_VA       (2u)
#define HVGR_MEM_PROPERTY_4G_VA         (3u)
#define HVGR_MEM_PROPERTY_2M_PAGE       (4u)
#define HVGR_MEM_PROPERTY_REPLAY        (5u)
#define HVGR_MEM_PROPERTY_SPARSE        (6u)
#define HVGR_MEM_PROPERTY_JIT           (7u)
#define HVGR_MEM_PROPERTY_DRIVER        (8u)
#define HVGR_MEM_PROPERTY_IMPORT        (9u)
#define HVGR_MEM_PROPERTY_ALIAS         (10u)
#define HVGR_MEM_PROPERTY_MAX           (11u)

#define HVGR_MEM_ATTR_CPU_RD            ((u64)1 << 0)
#define HVGR_MEM_ATTR_CPU_WR            ((u64)1 << 1)
#define HVGR_MEM_ATTR_CPU_CACHEABLE     ((u64)1 << 2)
#define HVGR_MEM_ATTR_KMD_ACCESS        ((u64)1 << 3)

#define HVGR_MEM_ATTR_GPU_RD            ((u64)1 << 4)
#define HVGR_MEM_ATTR_GPU_WR            ((u64)1 << 5)
#define HVGR_MEM_ATTR_GPU_EXEC          ((u64)1 << 6)
#define HVGR_MEM_ATTR_GPU_CACHEABLE     ((u64)1 << 7)
#define HVGR_MEM_ATTR_GROWABLE          ((u64)1 << 8)
#define HVGR_MEM_ATTR_SECURE            ((u64)1 << 9)

#define HVGR_MEM_ATTR_COHERENT_SYSTEM   ((u64)1 << 10)
#define HVGR_MEM_ATTR_COHERENT_LOCAL    ((u64)1 << 11)
#define HVGR_MEM_ATTR_COHERENT_SYS_REQ  ((u64)1 << 12)

#define HVGR_MEM_ATTR_OUTER_RD_ALLOCATE ((u64)1 << 13)
#define HVGR_MEM_ATTR_OUTER_WR_ALLOCATE ((u64)1 << 14)

#define HVGR_MEM_ATTR_CPU_OUT_SHAREABLE ((u64)1 << 15)

#define HVGR_MEM_SC_ATTR_RD_NOT_ALLOCATE 0u
#define HVGR_MEM_SC_ATTR_RD_ALLOCATE     1u
#define HVGR_MEM_SC_ATTR_RD_DEALLOCATE_INVALID 2u
#define HVGR_MEM_SC_ATTR_RD_DEALLOCATE_CLEAN_INVALID 3u

#define HVGR_MEM_SC_ATTR_WR_NOT_ALLOCATE 0u
#define HVGR_MEM_SC_ATTR_WR_ALLOCATE 1u
#define HVGR_MEM_SC_ATTR_WR_ALLOCATE_NO_REFILL 2u

#define HVGR_MEM_SC_ATTR_RD_POLICY_MASK ((u32)0x3)
#define hvgr_mem_get_sc_rd_policy(x)    ((x) & HVGR_MEM_SC_ATTR_RD_POLICY_MASK)
#define hvgr_mem_set_sc_rd_policy(x)    ((u32)(x) & 0x3)

#define HVGR_MEM_SC_ATTR_WR_POLICY_MASK ((u32)0x3 << 2)
#define hvgr_mem_get_sc_wr_policy(x)    (((x) & HVGR_MEM_SC_ATTR_WR_POLICY_MASK) >> 2)
#define hvgr_mem_set_sc_wr_policy(x)    (((u32)(x) & 0x3) << 2)

#define HVGR_MEM_ATTR_SC_PGID_MASK      ((u64)0x3F << 49)
#define hvgr_mem_get_sc_pgid(x)         (((x) & HVGR_MEM_ATTR_SC_PGID_MASK) >> 49)
#define hvgr_mem_set_sc_pgid(x)         (((u64)(x) & 0x3F) << 49)

#define HVGR_MEM_ATTR_SC_ID_MASK        ((u64)0xFF << 55)
#define hvgr_mem_get_sc_id(x)           (((x) & HVGR_MEM_ATTR_SC_ID_MASK) >> 55)
#define hvgr_mem_set_sc_id(x)           (((u64)(x) & 0xFF) << 55)

/*
 * This flag is used by kmd and cannot be used by umd.
 * When page table is setting up, this flag notifies that
 * current memory is sparse.
 */
#define HVGR_MEM_ATTR_MEM_SPARSE        ((u64)1 << 63)

#define HVGR_MEM_ATTR_SC_BIT_START      10
#define HVGR_MEM_ATTR_SC_BIT_NUMS       8
#define HVGR_MEM_ATTR_SC_BIT_MASK       ((1ull << HVGR_MEM_ATTR_SC_BIT_NUMS) - 1)

#define hvgr_mem_attr_sc(gid)           (((gid) & HVGR_MEM_ATTR_SC_BIT_MASK) << \
	HVGR_MEM_ATTR_SC_BIT_START)

#define HVGR_MEM_ATTR_BIT_NUM           (HVGR_MEM_ATTR_SC_BIT_START + HVGR_MEM_ATTR_SC_BIT_NUMS)
#define HVGR_MEM_ATTR_MASK              ((1ull << HVGR_MEM_ATTR_BIT_NUM) - 1)

#define HVGR_MEM_SYNC_TO_CPU            0x1
#define HVGR_MEM_SYNC_TO_DEV            0x2

typedef union {
	struct {
		/* defined with HVGR_MEM_PROPERTY_ */
		uint64_t property;

		/* defined with HVGR_MEM_ATTR_ */
		uint64_t attribute;

		uint32_t pages;
		uint32_t max_pages;
		uint32_t extent;
		/* For some types of mem, va can be specified, e.g. replay mem. */
		uint64_t va;

		uint32_t phy_pages;
	} in;
	struct {
		uint64_t attr;
		uint64_t uva;
		uint64_t gva;
	} out;
} hvgr_ioctl_para_alloc_memory;

#define HVGR_IOCTL_ALLOC_MEM \
	_IOWR(HVGR_IOCTL_MODULE_MEM, HVGR_IOCTL_CMD_ALLOC_MEM, hvgr_ioctl_para_alloc_memory)

typedef union {
	struct {
		uint64_t gva;
	} in;
} hvgr_ioctl_para_free_memory;

#define HVGR_IOCTL_FREE_MEM \
	_IOW(HVGR_IOCTL_MODULE_MEM, HVGR_IOCTL_CMD_FREE_MEM, hvgr_ioctl_para_free_memory)

typedef union {
	struct {
		/* defined with HVGR_MEM_ATTR_ */
		uint64_t attribute;

		uint64_t gva;
		uint64_t pages;
		int fd;
	} in;
	struct {
		uint64_t result;
		uint64_t uva;
		uint64_t gva;
	} out;
} hvgr_ioctl_para_import_memory;

#define HVGR_IOCTL_IMPORT_MEMORY \
	_IOWR(HVGR_IOCTL_MODULE_MEM, HVGR_IOCTL_CMD_IMPROT_MEM, hvgr_ioctl_para_import_memory)

typedef union {
	struct {
		/* defined with HVGR_MEM_ATTR_ */
		uint64_t attribute;

		uint64_t gva;
		uint64_t pages;
	} in;
	struct {
		uint64_t uva;
	} out;
} hvgr_ioctl_para_map_memory;

#define HVGR_IOCTL_MAP_MEM \
	_IOWR(HVGR_IOCTL_MODULE_MEM, HVGR_IOCTL_CMD_MAP_MEM, hvgr_ioctl_para_map_memory)

typedef struct {
	struct {
		uint64_t uva;
	} in;
} hvgr_ioctl_para_unmap_memory;

#define HVGR_IOCTL_UNMAP_MEM \
	_IOW(HVGR_IOCTL_MODULE_MEM, HVGR_IOCTL_CMD_UNMAP_MEM, hvgr_ioctl_para_unmap_memory)

typedef struct {
	struct {
		uint64_t uva;
		uint64_t size;
		uint64_t direct;
	} in;
} hvgr_ioctl_para_sync_memory;

#define HVGR_IOCTL_SYNC_MEM \
	_IOW(HVGR_IOCTL_MODULE_MEM, HVGR_IOCTL_CMD_SYNC_MEM, hvgr_ioctl_para_sync_memory)

typedef union {
	struct {
		uint64_t gva;
	} in;
	struct {
		uint64_t property;
		uint64_t attribute;
		uint64_t pages;
		uint64_t max_pages;
		uint64_t va;
		uint64_t phy_pages;
	} out;
} hvgr_ioctl_para_query_memory;

#define HVGR_IOCTL_QUERY_MEM \
	_IOWR(HVGR_IOCTL_MODULE_MEM, HVGR_IOCTL_CMD_QUERY_MEM, hvgr_ioctl_para_query_memory)

typedef struct {
	struct {
		uint64_t gva;
		uint64_t pages;
	} in;
} hvgr_ioctl_para_resize_memory;

#define HVGR_IOCTL_RESIZE_MEM \
	_IOW(HVGR_IOCTL_MODULE_MEM, HVGR_IOCTL_CMD_RESIZE_MEM, hvgr_ioctl_para_resize_memory)

typedef union {
	struct {
		int fd;
	} in;
	struct {
		uint64_t pages;
	} out;
} hvgr_ioctl_para_get_fd_size_memory;

#define HVGR_IOCTL_GET_FD_SIZE_MEM \
	_IOWR(HVGR_IOCTL_MODULE_MEM, HVGR_IOCTL_CMD_GET_FD_SIZE_MEM, \
	hvgr_ioctl_para_get_fd_size_memory)

typedef union {
	struct {
		/* defined with HVGR_MEM_ATTR_ */
		uint64_t attribute;
		uint64_t address;
		uint64_t size;
	} in;
	struct {
		uint64_t gva;
		uint64_t pages;
	} out;
} hvgr_ioctl_para_import_user_memory;

#define HVGR_IOCTL_IMPORT_USER_MEMORY \
	_IOWR(HVGR_IOCTL_MODULE_MEM, HVGR_IOCTL_CMD_IMPORT_USER_MEM, \
	hvgr_ioctl_para_import_user_memory)

struct hvgr_mem_alias_info {
	uint64_t gva;
	uint64_t offset;
	uint64_t pages;
};

typedef union {
	struct {
		uint64_t attribute;
		/*
		 * The memory to be aliased is multiple scattered memory.
		 * stride is the pages of the largest memory.
		 * count indicates the number of scattered memories.
		 * alias_info_ptr is the pointer of alias info.
		 */
		uint64_t stride;
		uint64_t count;
		uint64_t alias_info_ptr;
	} in;
	struct {
		uint64_t gva;
		uint64_t pages;
	} out;
} hvgr_ioctl_para_alias_memory;

#define HVGR_IOCTL_ALIAS_MEMORY \
	_IOWR(HVGR_IOCTL_MODULE_MEM, HVGR_IOCTL_CMD_ALIAS_MEM, hvgr_ioctl_para_alias_memory)

typedef struct {
	struct {
		uint64_t user_buf;
		uint64_t size;
	} in;
} hvgr_ioctl_para_profile_memory;

#define HVGR_IOCTL_ADD_PROFILE_MEM \
	_IOW(HVGR_IOCTL_MODULE_MEM, HVGR_IOCTL_CMD_ADD_PROFILE_MEM, hvgr_ioctl_para_profile_memory)

typedef union {
	struct {
		uint64_t uva;
		uint64_t size;
	} in;
	struct {
		uint64_t offset;
	} out;
} hvgr_ioctl_para_uva_offset_memory;

#define HVGR_IOCTL_UVA_OFFSET_MEMORY \
	_IOWR(HVGR_IOCTL_MODULE_MEM, HVGR_IOCTL_CMD_UVA_OFFSET_MEM, \
	hvgr_ioctl_para_uva_offset_memory)

typedef struct {
	uint64_t sparse_addr;
	uint64_t mem_addr;
	uint64_t pages;
} hvgr_ioctl_para_sparse_bind;

#define HVGR_IOCTL_SPARSE_MEM_BIND \
	_IOW(HVGR_IOCTL_MODULE_MEM, HVGR_IOCTL_CMD_SPARSE_BIND_MEM, hvgr_ioctl_para_sparse_bind)

typedef struct {
	uint64_t sparse_addr;
	uint64_t pages;
} hvgr_ioctl_para_sparse_unbind;

#define HVGR_IOCTL_SPARSE_MEM_UNBIND \
	_IOW(HVGR_IOCTL_MODULE_MEM, HVGR_IOCTL_CMD_SPARSE_UNBIND_MEM, hvgr_ioctl_para_sparse_unbind)

typedef struct {
	uint64_t gva;
	uint64_t attribute;
} hvgr_ioctl_para_change_memory_attr;

#define HVGR_IOCTL_CHANGE_MEM_ATTR \
	_IOW(HVGR_IOCTL_MODULE_MEM, HVGR_IOCTL_CMD_CHANGE_ATTR_MEM, \
	hvgr_ioctl_para_change_memory_attr)

typedef struct {
	uint32_t pgid;
	uint32_t attribute;
} hvgr_ioctl_para_change_pgid_attr;

#define HVGR_IOCTL_CHANGE_PGID_ATTR \
	_IOW(HVGR_IOCTL_MODULE_MEM, HVGR_IOCTL_CMD_CHANGE_PGID_ATTR, \
	hvgr_ioctl_para_change_pgid_attr)

#endif
