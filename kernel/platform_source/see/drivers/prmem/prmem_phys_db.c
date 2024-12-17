/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
 * Description: Memory protection library - core functions
 * Date: 2020/04/15
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/vmalloc.h>
#include <platform_include/see/hkip.h>
#include <platform_include/see/prmem.h>
#include <platform_include/see/access_register_smc_id.h>
#include <securec.h>
#include <linux/version.h>
#include <linux/memblock.h>
#include <linux/arm-smccc.h>
#include "prmem_internal.h"
#ifdef CONFIG_DDRC_SUPPORT_FFA
#include "platform_include/see/kernel_ddrc_ffa_init.h"
#endif

#ifdef pr_fmt
#undef pr_fmt
#endif

#define pr_fmt(fmt) "prmem : " fmt

/* ----------------------- physical pages dbs  ----------------------- */
prmem_phys_db(prmem_pgtable_pages, DO_NOT_PREMAP);
prmem_phys_db(prmem_pmalloc, DO_PREMAP);

static __init bool phys_db_add_chunk(struct prmem_phys_chunk *chunk, bool premap)
{
#ifndef CONFIG_PRMEM_CONTINUOUS_MEMORY
	void *addr = NULL;

	/* Use "exact" to split the chunk into order 0 pages */
	addr = alloc_pages_exact(chunk->pages_per_chunk * PAGE_SIZE, GFP_KERNEL);
	if (unlikely(addr == NULL)) {
		prmem_err(PRMEM_NO_CHUNK);
		return false;
	}
	chunk->base_pfn = virt_to_pfn((uintptr_t)addr);
#endif

	if (premap && prepare_mappings(chunk, chunk->base_pfn) != 0) {
		prmem_err(PRMEM_NO_PREP);
		return false;
	}

	chunk->free_pages_nr =  chunk->pages_per_chunk;
	/* 0xFF set every bits of chunk bitmap to 1 */
	if (memset_s(chunk->map, sizeof(unsigned long) * PRMEM_MAP_SIZE, 0xFF,
		     sizeof(unsigned long) * PRMEM_MAP_SIZE) != EOK) {
		prmem_err(PRMEM_MEMCPY_ERR);
		return false;
	}
	return true;
}

static unsigned long get_chunk_pages(struct prmem_phys_chunk *c,
				     struct page **p,
				     unsigned long pages_nr)
{
	unsigned long n = 0;
	unsigned int word;
	unsigned int bit;
	unsigned long pfn;

	for (word = 0; word < PRMEM_MAP_SIZE; word++)
		for (bit = 0; bit < PRMEM_MAP_ROW_BITS; bit++)
			if ((c->free_pages_nr == 0) || (n == pages_nr)) {
				goto out;
			} else if ((c->map[word]) & (1UL << bit)) {
				c->map[word] &= ~(1UL << bit);
				pfn = c->base_pfn;
				pfn += word * PRMEM_MAP_ROW_BITS + bit;
				p[n] = pfn_to_page(pfn);
				n++;
				c->free_pages_nr--;
			}
out:
	return n;
}

static inline void get_phys_db_pages(struct prmem_phys_db *db,
				     struct page **p,
				     unsigned long pages_nr)
{
	unsigned int i;
	unsigned long n;

	for (i = 0, n = 0; (n < pages_nr) && (i < db->chunks_nr); i++)
		n += get_chunk_pages(db->chunks + i, p + n, pages_nr - n);
	db->total_free_pages_nr -= n;
	db->total_used_pages_nr += n;
	if (db->total_used_pages_nr > db->peak_total_used_pages_nr)
		db->peak_total_used_pages_nr = db->total_used_pages_nr;
}

bool get_phys_pages(struct prmem_phys_db *db, struct page **p, unsigned long pages_nr)
{
	struct prmem_phys_db tmp_db;

	if (db->total_free_pages_nr < pages_nr)
		return false;
	if (memcpy_s(&tmp_db, sizeof(struct prmem_phys_db), db,
		     sizeof(struct prmem_phys_db)) != EOK) {
		prmem_err(PRMEM_MEMCPY_ERR);
		return false;
	}
	get_phys_db_pages(&tmp_db, p, pages_nr);
	if (memcmp(&tmp_db, db, sizeof(struct prmem_phys_db)))
		wr_assign(*db, tmp_db);
	return true;
}

struct page *get_phys_page(struct prmem_phys_db *db)
{
	struct page *p = NULL;

	if (unlikely(!get_phys_pages(db, &p, 1)))
		return NULL;
	return p;
}

static bool page_in_chunk(struct page *page,
			  struct prmem_phys_chunk *chunk)
{
	unsigned long pfn = page_to_pfn(page);

	return ((chunk->base_pfn <= pfn) &&
		(pfn < (chunk->base_pfn + chunk->pages_per_chunk)));
}

static struct prmem_phys_chunk *get_page_chunk(struct prmem_phys_db *db,
	struct page *page)
{
	unsigned int c;

	for (c = 0; c < db->chunks_nr; c++)
		if (unlikely(page_in_chunk(page, db->chunks + c)))
			return db->chunks + c;
	return NULL;
}

static void put_chunk_page(struct page *page,
			   struct prmem_phys_chunk *chunk)
{
	unsigned long offset = page_to_pfn(page) - chunk->base_pfn;
	unsigned long word = (unsigned long)(offset / PRMEM_MAP_ROW_BITS);
	unsigned long bit = (unsigned long)(offset % PRMEM_MAP_ROW_BITS);

	chunk->map[word] |= (1UL << bit);
	chunk->free_pages_nr++;
}

static int put_phys_db_page(struct prmem_phys_db *db,
			    struct page *page)
{
	struct prmem_phys_chunk *chunk = NULL;

	chunk = get_page_chunk(db, page);
	if (unlikely(chunk == NULL)) {
		prmem_err(PRMEM_LEAK_PHYS);
		return -PRMEM_LEAK_PHYS;
	}
	put_chunk_page(page, chunk);
	db->total_free_pages_nr++;
	db->total_used_pages_nr--;
	return 0;
}

int put_phys_pages(struct prmem_phys_db *db, struct prmem_node *node)
{
	struct prmem_phys_db tmp_db;
	unsigned long base = node->start;
	unsigned long offset = 0;

	if (memcpy_s(&tmp_db, sizeof(struct prmem_phys_db), db,
		   sizeof(struct prmem_phys_db)) != EOK) {
		prmem_err(PRMEM_MEMCPY_ERR);
		goto out;
	}
	for (offset = 0; offset < node->size; offset += PAGE_SIZE) {
		struct page *p = NULL;

		p = vmalloc_to_page((void *)(uintptr_t)(base + offset));
		if (unlikely(!p)) {
			prmem_err(PRMEM_LEAK_PHYS);
			goto out;
		}
		if (unlikely(put_phys_db_page(&tmp_db, p)))
			goto out;
	}
out:
	if (memcmp(&tmp_db, db, sizeof(struct prmem_phys_db)))
		wr_assign(*db, tmp_db);
	return (node->size - offset) / PAGE_SIZE;
}

int put_phys_page(struct prmem_phys_db *db, struct page *page)
{
	struct prmem_phys_db tmp_db;
	bool retval = false;

	if (memcpy_s(&tmp_db, sizeof(struct prmem_phys_db), db,
		   sizeof(struct prmem_phys_db)) != EOK) {
		prmem_err(PRMEM_MEMCPY_ERR);
		return false;
	}
	retval = put_phys_db_page(&tmp_db, page);
	if (memcmp(&tmp_db, db, sizeof(struct prmem_phys_db)))
		wr_assign(*db, tmp_db);
	return retval;
}

bool is_phys_page(struct prmem_phys_db *db, struct page *page)
{
	return !!get_page_chunk(db, page);
}

/* -------------------------- phys db init ------------------------------- */

static void __init prmem_phys_db_init(struct prmem_phys_db *db)
{
	unsigned int i;

	for (i = 0; i < db->chunks_nr; i++) {
		struct prmem_phys_chunk *chunk;

		chunk = db->chunks + i;
		chunk->pages_per_chunk = db->db_pages_per_chunk;
#ifdef CONFIG_PRMEM_CONTINUOUS_MEMORY
		chunk->base_pfn = db->start_pfn + (i * chunk->pages_per_chunk);
#endif
		if (likely(phys_db_add_chunk(chunk, db->premap))) {
			db->total_free_pages_nr += db->db_pages_per_chunk;
		} else {
			prmem_err(PRMEM_DB_INIT_NO_CHUNK);
			BUG();
		}
	}
}

#ifdef CONFIG_PRMEM_CONTINUOUS_MEMORY
static phys_addr_t last_memblock_start;
static phys_addr_t last_memblock_end;
static void * prmem_alternative_db_vaddr;
unsigned long prmem_start_pfn;

void __init prmem_alternative_alloc(void)
{
	prmem_alternative_db_vaddr = memblock_alloc(PRMEM_PHYS_DB_SIZE, SZ_2M);
	WARN(!prmem_alternative_db_vaddr, "prmem temporarily alloc memory fail, ignore this err");
}

static void __init prmem_get_last_memblock_addr(void)
{
	u64 i;
	phys_addr_t start;
	phys_addr_t end;

	for_each_mem_range(i, &start, &end) {
		if (start >= end)
			break;
		if (start > last_memblock_start) {
			last_memblock_start = start;
			last_memblock_end = end;
		}
	}
}

static int __init prmem_try_alloc_contig_range(unsigned long *start_pfn, unsigned long *end_pfn)
{
	int rc = -1;
#ifndef CONFIG_HHEE_OHOS
	struct acr_info dummy;
#endif
	unsigned long pfn;
	unsigned long pfn_shift = SZ_16M >> PAGE_SHIFT;
	unsigned long last_memblk_start_pfn = PHYS_PFN(last_memblock_start);
	unsigned long last_memblk_end_pfn = PHYS_PFN(last_memblock_end);

	for (pfn = last_memblk_start_pfn; pfn < last_memblk_end_pfn; pfn += pfn_shift) {
#ifdef CONFIG_HHEE_OHOS
		rc = alloc_contig_range(pfn, pfn + PRMEM_PHYS_DB_PFN_NUM, MIGRATE_MOVABLE,
					GFP_KERNEL);
#else
		rc = alloc_contig_range(pfn, pfn + PRMEM_PHYS_DB_PFN_NUM, MIGRATE_MOVABLE,
					GFP_KERNEL, &dummy);
#endif
		if (rc == 0) {
			*start_pfn = pfn;
			*end_pfn = pfn + PRMEM_PHYS_DB_PFN_NUM;
			pr_err("prmem_try_alloc_contig_range success, start_phys 0x%lx, end_phys 0x%lx\n",
				PFN_PHYS(pfn), PFN_PHYS(pfn + PRMEM_PHYS_DB_PFN_NUM));
			break;
		}
	}
	return rc;
}

/*
 * traps to bl31 domain,
 * to set relative reggisters to finish kernel prmem secure region protection
 */
void __init configure_prmem_secure_region(void)
{
	unsigned long phys_addr;
	unsigned long size;

	phys_addr = prmem_start_pfn;
	size = (unsigned long)(PRMEM_PHYS_DB_PFN_NUM << PAGE_SHIFT);
#ifdef CONFIG_DEBUG_HKIP_PRMEM
	pr_err("prmem secure region start 0x%lx, size 0x%lx\n", phys_addr, size);
#endif

#ifdef CONFIG_DDRC_SUPPORT_FFA
	struct ffa_send_direct_data args = {0};
	args.data0 = ACCESS_REGISTER_FN_PRMEM_ID;
	args.data1 = upper_32_bits(phys_addr);
	args.data2 = lower_32_bits(phys_addr);
	args.data3 = size;

	(void)ffa_ddrc_send_msg(&args);
	WARN(args.data0 != 0, "configure prmem secure region failed");
#else
	struct arm_smccc_res res = {0};

	arm_smccc_smc((u64)ACCESS_REGISTER_FN_PRMEM_ID, phys_addr, size,
		      0, 0, 0, 0, 0, &res);
	WARN(res.a0 != 0, "configure prmem secure region failed");
#endif
}
#endif

/*
 * Note: prmem_phys_dbs_init() is invoked *before* __wr data becomes
 * actually wr, so it performs direct memory writes to targets which later
 * on will become write protected.
 *
 * Note: the order of iniitalization is important, because the
 * initalization of the pmalloc physical pages db will need the
 * availability of the page tables physical pages db, so
 * *** DO NOT CHANGE THE ORDER OF INITIALIZATION OF THE PHYS DBs ***
 */
void __init prmem_phys_dbs_init(void)
{
#ifdef CONFIG_PRMEM_CONTINUOUS_MEMORY
	int rc;
	int ret;
#ifndef CONFIG_HHEE_OHOS
	struct acr_info dummy;
#endif
	bool use_alternative_db = false;
	unsigned long start_pfn = PHYS_PFN(last_memblock_start);
	unsigned long end_pfn = start_pfn + PRMEM_PHYS_DB_PFN_NUM;
	unsigned long prmem_pfn_num = PRMEM_PHYS_MEMORY_SIZE >> PAGE_SHIFT;

	prmem_get_last_memblock_addr();
#ifdef CONFIG_HHEE_OHOS
	rc = alloc_contig_range(start_pfn, end_pfn, MIGRATE_MOVABLE, GFP_KERNEL);
#else
	rc = alloc_contig_range(start_pfn, end_pfn, MIGRATE_MOVABLE, GFP_KERNEL, &dummy);
#endif
	if (rc != 0) {
		pr_err("prmem alloc contig range busy\n");
		ret = prmem_try_alloc_contig_range(&start_pfn, &end_pfn);
		if (WARN(ret != 0, "prmem try alloc fail, use alternative mem"))
			use_alternative_db = true;
	}

	if (use_alternative_db)
		start_pfn = virt_to_pfn((uintptr_t)prmem_alternative_db_vaddr);
	else
		memblock_free_late(__pa((uintptr_t)prmem_alternative_db_vaddr), PRMEM_PHYS_DB_SIZE);

	prmem_start_pfn = PFN_PHYS(start_pfn);
	prmem_pgtable_pages_phys_db.start_pfn = (unsigned long)(start_pfn + prmem_pfn_num);
	prmem_pmalloc_phys_db.start_pfn = start_pfn;
#endif

	prmem_pgtable_pages_phys_db.chunks_nr = PGTABLE_CHUNKS_NR;
	prmem_pgtable_pages_phys_db.db_pages_per_chunk = (unsigned long)PGTABLE_PAGES_PER_CHUNK;
	prmem_phys_db_init(&prmem_pgtable_pages_phys_db);

	prmem_pmalloc_phys_db.chunks_nr = PMALLOC_CHUNKS_NR;
	prmem_pmalloc_phys_db.db_pages_per_chunk = PMALLOC_PAGES_PER_CHUNK;
	prmem_phys_db_init(&prmem_pmalloc_phys_db);
}