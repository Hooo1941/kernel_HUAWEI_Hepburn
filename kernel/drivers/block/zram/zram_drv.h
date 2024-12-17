/*
 * Compressed RAM block device
 *
 * Copyright (C) 2008, 2009, 2010  Nitin Gupta
 *               2012, 2013 Minchan Kim
 *
 * This code is released using a dual license strategy: BSD/GPL
 * You can choose the licence that better fits your requirements.
 *
 * Released under the terms of 3-clause BSD License
 * Released under the terms of GNU General Public License Version 2.0
 *
 */

#ifndef _ZRAM_DRV_H_
#define _ZRAM_DRV_H_

#include <linux/rwsem.h>
#include <linux/zsmalloc.h>
#include <linux/crypto.h>
#ifdef CONFIG_ZRAM_DEDUP
#include <linux/cache.h>
#include <linux/zpool.h>
#endif

#ifdef CONFIG_CRYPTO_DELTA
#include <linux/idr.h>
#include <linux/radix-tree.h>
#include <linux/sched.h>
#include <linux/workqueue.h>
#include "sddc/zcomp_ext.h"
#else
#include "zcomp.h"
#endif

#define SECTORS_PER_PAGE_SHIFT	(PAGE_SHIFT - SECTOR_SHIFT)
#define SECTORS_PER_PAGE	(1 << SECTORS_PER_PAGE_SHIFT)
#define ZRAM_LOGICAL_BLOCK_SHIFT 12
#define ZRAM_LOGICAL_BLOCK_SIZE	(1 << ZRAM_LOGICAL_BLOCK_SHIFT)
#define ZRAM_SECTOR_PER_LOGICAL_BLOCK	\
	(1 << (ZRAM_LOGICAL_BLOCK_SHIFT - SECTOR_SHIFT))

#define ZRAM_WB_FLAGS_DISABLE		(0)
#define WRITEBACK_LIMIT_CYCLE_DEFAULT	(24 * 60 * 60)
#define WRITEBACK_LIMIT_CYCLE_MIN	(24 * 60 * 60)
#define WRITEBACK_LIMIT_MAX_DEFAULT	(500 * 1024 * 1024 / 4096)
#define WRITEBACK_LIMIT_MAX_MAX		(1024 * 1024 * 1024 / 4096)

/*
 * The lower ZRAM_FLAG_SHIFT bits of table.flags is for
 * object size (excluding header), the higher bits is for
 * zram_pageflags.
 *
 * zram is mainly used for memory efficiency so we want to keep memory
 * footprint small so we can squeeze size and flags into a field.
 * The lower ZRAM_FLAG_SHIFT bits is for object size (excluding header),
 * the higher bits is for zram_pageflags.
 */
#define ZRAM_FLAG_SHIFT 24

/*
 * The ZRAM_TABLE_PROXY_SHIFT defines the size of proxy part of the zram table (
 * for referencing pages are used as reference ones for delta-compression or deduplication).
 * It is defined as part of disksize.
 */
#ifdef CONFIG_CRYPTO_DELTA
#define ZRAM_TABLE_PROXY_SHIFT 3
#endif

/* Flags for zram pages (table[page_no].flags) */
enum zram_pageflags {
	/* zram slot is locked */
	ZRAM_LOCK = ZRAM_FLAG_SHIFT,
	ZRAM_SAME,	/* Page consists the same element */
	ZRAM_WB,	/* page is stored on backing_device */
	ZRAM_UNDER_WB,	/* page is under writeback */
	ZRAM_HUGE,	/* Incompressible page */
#ifdef CONFIG_HP_CORE
	ZRAM_BATCHING_OUT,
	ZRAM_FROM_HYPERHOLD,
	ZRAM_MCGID_CLEAR,
#endif
	ZRAM_IDLE_FAST, /* not accessed page since last idle_fast marking */
	ZRAM_IDLE,	/* not accessed page since last idle marking */
#ifdef CONFIG_ZRAM_DEDUP
	ZRAM_INDIRECT_HANDLE,
#endif
#ifdef CONFIG_CRYPTO_DELTA
	ZRAM_SDDC_UNIQUE,
	ZRAM_SDDC_DELTA,
	ZRAM_SDDC_DUPLICATE,
#endif
	__NR_ZRAM_PAGEFLAGS,
};

/*-- Data structures */

/* Allocated for each disk page */
struct zram_table_entry {
	union {
		unsigned long handle;
		unsigned long element;
#ifdef CONFIG_CRYPTO_DELTA
		unsigned long dup_page_id;
#endif
	};
	unsigned long flags;
#ifdef CONFIG_ZRAM_MEMORY_TRACKING
	ktime_t ac_time;
#endif
};

#ifdef CONFIG_CRYPTO_DELTA
struct zram_table_entry_ref_counter {
	atomic_t ref_count; /* duplicates and similars */
};

#ifdef CONFIG_HP_CORE
/* For tracking reference pages indexes for delta encoded pages
 * moved to eswap.
 * We store ref page id in the first 4 bytes of the zsmalloc slot rignt before
 * the encoded page. Once a page is moved to eswap it is deleted from zram.
 * Whenever a delete request comes for any particular page moved to eswap we
 * need to obtain the ref page id to decrement ref count of the appropriate
 * reference page. This is why this cache is needed */
struct zram_sddc_ref_obj {
	u32 ref_index;
};

struct zram_sddc_ref_obj_cache {
	struct radix_tree_root tree;
	spinlock_t lock;
};
#endif
#endif

struct zram_stats {
	atomic64_t compr_data_size;	/* compressed size of pages stored */
	atomic64_t num_reads;	/* failed + successful */
	atomic64_t num_writes;	/* --do-- */
	atomic64_t failed_reads;	/* can happen when memory is too low */
	atomic64_t failed_writes;	/* can happen when memory is too low */
	atomic64_t invalid_io;	/* non-page-aligned I/O requests */
	atomic64_t notify_free;	/* no. of swap slot free notifications */
	atomic64_t same_pages;		/* no. of same element filled pages */
	atomic64_t huge_pages;		/* no. of huge pages */
	atomic64_t idle_pages;          /* no. of idle pages */
	atomic64_t idle_fast_pages;     /* no. of idle_fast pages */
	atomic64_t pages_stored;	/* no. of pages currently stored */
	atomic_long_t max_used_pages;	/* no. of maximum pages stored */
	atomic64_t writestall;		/* no. of write slow paths */
	atomic64_t miss_free;		/* no. of missed free */
#ifdef	CONFIG_ZRAM_WRITEBACK
	atomic64_t bd_count;		/* no. of pages in backing device */
	atomic64_t bd_reads;		/* no. of reads from backing device */
	atomic64_t bd_writes;		/* no. of writes from backing device */
#endif
#ifdef CONFIG_CRYPTO_DELTA
	atomic64_t num_unique; /* no. of pages encoded as unique */
	atomic64_t num_duplicate; /* no. of duplicated pages */
	atomic64_t num_delta; /* no. of pages encoded as delta */
	atomic64_t max_proxy_pages; /* maximum no. of proxy pages */
#endif
};

#ifdef CONFIG_ZRAM_DEDUP
struct zram_indirect_handle {
	struct hlist_bl_node	node;
	unsigned long		handle;
	atomic_t		refs;
	u32			hash;
};

struct zram_hashtable_head {
	struct hlist_bl_head	head;
};

struct zram_dedup {
	struct zram_hashtable_head	*buckets;
	int				nbuckets;
	unsigned long			nr_pages;
	atomic64_t			dedups;
};
#endif

#ifdef CONFIG_CRYPTO_DELTA
struct zram_sddc_recompress_node {
	struct hlist_bl_node    node;
	u32                     index;
};

struct zram_sddc_recompress {
	struct hlist_bl_head    head;
	struct hlist_bl_node   *last;
	unsigned long           count;
};
#endif

struct zram {
	struct zram_table_entry *table;
	struct zs_pool *mem_pool;
#ifdef CONFIG_ZRAM_DEDUP
	struct zram_dedup *dedup;
#endif
#ifdef CONFIG_CRYPTO_DELTA
	struct zcomp_ext *comp;
	struct zram_table_entry_ref_counter *ref_counter;
#ifdef CONFIG_HP_CORE
	struct zram_sddc_ref_obj_cache *ref_obj_cache;
#endif
#else
	struct zcomp *comp;
#endif
	struct gendisk *disk;
	/* Prevent concurrent execution of device init */
	struct rw_semaphore init_lock;
	/*
	 * the number of pages zram can consume for storing compressed data
	 */
	unsigned long limit_pages;
#ifdef CONFIG_CRYPTO_DELTA
	struct ida proxy_ida;
	unsigned int page_proxy_id_min;
	unsigned int page_proxy_id_max;
	atomic64_t alloced_pages_proxy;
#endif

	struct zram_stats stats;
	/*
	 * This is the limit on amount of *uncompressed* worth of data
	 * we can store in a disk.
	 */
	u64 disksize;	/* bytes */
	char compressor[CRYPTO_MAX_ALG_NAME];
	/*
	 * zram is claimed so open request will be failed
	 */
	bool claim; /* Protected by bdev->bd_mutex */
	struct file *backing_dev;
#if (defined CONFIG_ZRAM_WRITEBACK) || (defined CONFIG_ZRAM_DEDUP) || (defined CONFIG_ZRAM_NON_COMPRESS)
	spinlock_t wb_limit_lock;
#endif
#ifdef CONFIG_ZRAM_WRITEBACK
	bool wb_limit_enable;
	unsigned long wb_flags;
	s64 bd_wb_limit;
	s64 bd_wb_limit_max;
	int bd_wb_limit_cycle;
	unsigned long pre_wb_limit_time;
	unsigned long *bitmap;
#endif
#if (defined CONFIG_ZRAM_WRITEBACK) || (defined CONFIG_HP_CORE)
	struct block_device *bdev;
	unsigned int old_block_size;
	unsigned long nr_pages;
#endif
#ifdef CONFIG_HP_CORE
	struct hyperhold_area *area;
#endif
#ifdef CONFIG_ZRAM_MEMORY_TRACKING
	struct dentry *debugfs_dir;
#endif
#ifdef CONFIG_CRYPTO_DELTA
	struct zram_sddc_recompress *recompress_pended;
	struct work_struct sddc_work;
	bool async_enable;
#endif
#ifdef CONFIG_ZRAM_DEDUP
	bool dedup_enable;
#endif
#ifdef CONFIG_ZRAM_NON_COMPRESS
	bool noncompress_enable;
#endif
};

size_t get_huge_class_size(void);
#ifdef CONFIG_HP_CORE
static inline int zram_slot_trylock(struct zram *zram, u32 index)
{
	return bit_spin_trylock(ZRAM_LOCK, &zram->table[index].flags);
}

static inline void zram_slot_lock(struct zram *zram, u32 index)
{
	bit_spin_lock(ZRAM_LOCK, &zram->table[index].flags);
}

static inline void zram_slot_unlock(struct zram *zram, u32 index)
{
	bit_spin_unlock(ZRAM_LOCK, &zram->table[index].flags);
}

static inline unsigned long zram_get_handle(struct zram *zram, u32 index)
{
	return zram->table[index].handle;
}

static inline void zram_set_handle(struct zram *zram,
					u32 index, unsigned long handle)
{
	zram->table[index].handle = handle;
}

static inline bool zram_test_flag(struct zram *zram, u32 index,
			enum zram_pageflags flag)
{
	return zram->table[index].flags & BIT(flag);
}

static inline void zram_set_flag(struct zram *zram, u32 index,
			enum zram_pageflags flag)
{
	zram->table[index].flags |= BIT(flag);
}

static inline void zram_clear_flag(struct zram *zram, u32 index,
			enum zram_pageflags flag)
{
	zram->table[index].flags &= ~BIT(flag);
}

#ifdef CONFIG_ZRAM_DEDUP
unsigned long zram_get_direct_handle(struct zram *zram, u32 index);
void zram_free_handle(struct zram *zram, u32 index);
#endif

static inline void zram_set_element(struct zram *zram, u32 index,
			unsigned long element)
{
	zram->table[index].element = element;
}

static inline unsigned long zram_get_element(struct zram *zram, u32 index)
{
	return zram->table[index].element;
}

static inline size_t zram_get_obj_size(struct zram *zram, u32 index)
{
	return zram->table[index].flags & (BIT(ZRAM_FLAG_SHIFT) - 1);
}

static inline void zram_set_obj_size(struct zram *zram,
					u32 index, size_t size)
{
	unsigned long flags = zram->table[index].flags >> ZRAM_FLAG_SHIFT;

	zram->table[index].flags = (flags << ZRAM_FLAG_SHIFT) | size;
}

#ifdef CONFIG_CRYPTO_DELTA
int32_t zram_alloc_proxy(struct zram *zram);
void zram_free_proxy(struct zram *zram, uint32_t id);
void dec_and_test_proxy(struct zram *zram, uint32_t ref_index);
#ifdef CONFIG_HP_CORE
int zram_sddc_ref_obj_cache_push(
				struct zram_sddc_ref_obj_cache* cache,
				u32 index,
				u32 ref_index);

int zram_sddc_ref_obj_cache_pop(
				struct zram_sddc_ref_obj_cache* cache,
				u32 index,
				u32 *ref_index);
#endif
#endif

#endif
#endif
