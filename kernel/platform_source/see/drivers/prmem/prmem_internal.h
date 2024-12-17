/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
 * Description: prmem internal header
 * Date: 2023/04/07
 */

#ifndef __PRMEM_INTERNAL_H__
#define __PRMEM_INTERNAL_H__

#include <linux/string.h>
#include <linux/kernel.h>
#include <platform_include/see/hhee_prmem.h>
#include <linux/log2.h>
#include <linux/stringify.h>
#include <platform_include/see/prmem_defs.h>
#include <linux/vmalloc.h>

/* --------------------- prmem error handling ------------------------- */

/* enum prmem_errno - error states that can be returned within prmem */
enum prmem_errno {
	PRMEM_OK = 0,
	PRMEM_VA_NOT_VALID,
	PRMEM_VA_NOT_FOUND,
	PRMEM_VA_NOT_RECLAIMABLE,
	PRMEM_NODE_NOT_FOUND,
	PRMEM_NODES_MAXED,
	PRMEM_NODES_BATCH_FAIL,
	PRMEM_POOL_UNKNOWN_TYPE,
	PRMEM_NO_SPACE,
	PRMEM_INVALID_INPUT_PARAMETERS,
	PRMEM_DB_INIT_NO_CHUNK,
	PRMEM_NO_CHUNK,
	PRMEM_NO_NODE,
	PRMEM_NO_RANGE,
	PRMEM_NO_PHYS,
	PRMEM_NO_PREP,
	PRMEM_NO_VA,
	PRMEM_NO_GROW,
	PRMEM_NO_METADATA,
	PRMEM_STRING_TOO_LONG,
	PRMEM_INVALID_ADDRESS,
	PRMEM_POOL_IS_START_WR,
	PRMEM_POOL_NOT_RECLAIMABLE,
	PRMEM_INVALID_RANGE,
	PRMEM_OVERLAP_RANGE,
	PRMEM_LEAK_PHYS,
	PRMEM_PREPARE_VA_FAIL,
	PRMEM_MEMCPY_ERR,
	PRMEM_MEMSET_ERR,
	PRMEM_ERR_NUMBERS,
};

void __prmem_err(enum prmem_errno errno, const char *filename, const char *func,
		 unsigned int line_nr);

#define prmem_err(errno)						\
	__prmem_err(errno, __FILE__, __func__, __LINE__)

#define prmem_err_ret(errno)						\
do {									\
	prmem_err(errno);						\
	return -(errno);						\
} while (0)

/*--------------------- Physical pages allocator ----------------------*/
extern struct prmem_phys_db prmem_pmalloc_phys_db;
extern struct prmem_phys_db prmem_pgtable_pages_phys_db;

#define PMALLOC_CHUNKS_NR						\
	DIV_ROUND_UP(PRMEM_PHYS_MEMORY_SIZE, PMALLOC_CHUNK_SIZE)

#define PMALLOC_PHYS_DB_PAGES_NR					\
	DIV_ROUND_UP(PRMEM_PHYS_MEMORY_SIZE, PAGE_SIZE)

/*
 * The number of pages required to support the pmalloc entries in the
 * pagetables is rounded up to double the number of ptes.
 */
#define PGTABLE_PHYS_DB_PAGES_NR					\
	DIV_ROUND_UP(PMALLOC_PHYS_DB_PAGES_NR * 2, (PAGE_SIZE / sizeof(pte_t)))

#define PGTABLE_PAGES_PER_CHUNK  ALIGN(PGTABLE_PHYS_DB_PAGES_NR * 3, (SZ_64K / PAGE_SIZE))

#define PGTABLE_CHUNKS_NR 1

#define PRMEM_PHYS_DB_PFN_NUM ((PRMEM_PHYS_MEMORY_SIZE >> PAGE_SHIFT) + PGTABLE_PAGES_PER_CHUNK)
#define PRMEM_PHYS_DB_SIZE    (PRMEM_PHYS_DB_PFN_NUM << PAGE_SHIFT)

void prmem_phys_dbs_init(void);
bool is_phys_page(struct prmem_phys_db *db, struct page *page);
int put_phys_page(struct prmem_phys_db *db, struct page *page);
int put_phys_pages(struct prmem_phys_db *db, struct prmem_node *node);
struct page *get_phys_page(struct prmem_phys_db *db);
bool get_phys_pages(struct prmem_phys_db *db, struct page **p, unsigned long pages_nr);
int prepare_mappings(struct prmem_phys_chunk *chunk, unsigned long pfn);

#endif