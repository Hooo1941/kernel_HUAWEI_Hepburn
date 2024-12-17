/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020. All rights reserved.
 * Description: hyperhold header file
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
 * Author:	He Biao <hebiao6@huawei.com>
 *		Wang Cheng Ke <wangchengke2@huawei.com>
 *		Wang Fa <fa.wang@huawei.com>
 *
 * Create: 2020-4-16
 *
 */

#ifndef HYPERHOLD_INTERNAL_H
#define HYPERHOLD_INTERNAL_H

#include <uapi/linux/fs.h>
#include <linux/hp/hyperhold_inf.h>
#include <linux/zsmalloc.h>
#include "zram_drv.h"

#ifdef CONFIG_HP_TURBO
#define EXTENT_SHIFT		12
#define NO_PKG_EXTENT_NUM	1
#define PKG_EXTENT_NUM_SHIFT	3
#define PKG_EXTENT_NUM		8

#define PKG_EXTENT_SHIFT	(EXTENT_SHIFT + PKG_EXTENT_NUM_SHIFT)
#define PKG_EXTENT_OFFSET_SHIFT ((sizeof(unsigned long) << 3) - PKG_EXTENT_NUM_SHIFT)
#define PKG_EXTENT_ADDR_MASK 	((1UL << PKG_EXTENT_OFFSET_SHIFT) - 1)
#define PKG_EXTENT_OFFSET_MASK 	(PKG_EXTENT_NUM - 1)

#define HP_EXT_NUM_PER_MBYTE (256UL)
#else
#define EXTENT_SHIFT		15
#define NO_PKG_EXTENT_NUM	1
#define PKG_EXTENT_NUM_SHIFT    0
#define PKG_EXTENT_NUM		1

#define HP_EXT_NUM_PER_MBYTE (32UL)
#endif

#define EXTENT_SIZE		(1UL << EXTENT_SHIFT)
#define EXTENT_PG_CNT		(EXTENT_SIZE >> PAGE_SHIFT)
#define EXTENT_SECTOR_SIZE	(EXTENT_PG_CNT << 3)

#define PKG_EXTENT_SIZE		(EXTENT_SIZE << PKG_EXTENT_NUM_SHIFT)
#define PKG_EXTENT_PG_CNT	(PKG_EXTENT_SIZE >> PAGE_SHIFT)

#define MAX_FAIL_RECORD_NUM 	10

#define BLOCK_SECTOR_SHIFT	3

#define MBYTE_TO_BYTE_FACTOR (1024UL * 1024UL)
#define MIN_PER_HP_FILE_SZ_BYTE (2UL * 1024UL * 1024UL)
#define MAX_PER_HP_FILE_SZ_BYTE (128UL * 1024UL * 1024UL)
#define get_exts(size) (((size) / MBYTE_TO_BYTE_FACTOR) * HP_EXT_NUM_PER_MBYTE)
#define get_divide_size(size) ((size) / 2)

#define MIN_RECLAIM_ZRAM_SZ	(1024 * 1024)
#define FT_RECLAIM_SZ		500

#define HYPERHOLD_SECTOR_SHIFT 9
#define HYPERHOLD_PAGE_SIZE_SECTOR (PAGE_SIZE >> HYPERHOLD_SECTOR_SHIFT)

enum {
	HHLOG_ERR = 0,
	HHLOG_WARN,
	HHLOG_INFO,
	HHLOG_DEBUG,
	HHLOG_MAX
};

static inline void pr_none(void) {}

#define pt(f, ...)	pr_err("<%s>:"f, __func__, ##__VA_ARGS__)
#define cur_lvl()	hyperhold_loglevel()

#define hh_print(l, f, ...) \
	(l <= cur_lvl() ? pt(f, ##__VA_ARGS__) : pr_none())

bool hyperhold_discard_enable(void);
void hyperhold_set_discard_enable(bool en);
struct block_device *hyperhold_bdev(void);
int hyperhold_loglevel(void);

enum hyperhold_scenario {
	HYPERHOLD_RECLAIM_IN = 0,
	HYPERHOLD_FAULT_OUT,
	HYPERHOLD_BATCH_OUT,
	HYPERHOLD_PRE_OUT,
	HYPERHOLD_SCENARIO_BUTT
};

enum hyperhold_key_point {
	HYPERHOLD_START = 0,
	HYPERHOLD_INIT,
	HYPERHOLD_IOENTRY_ALLOC,
	HYPERHOLD_FIND_EXTENT,
	HYPERHOLD_IO_EXTENT,
	HYPERHOLD_SEGMENT_ALLOC,
	HYPERHOLD_BIO_ALLOC,
	HYPERHOLD_SUBMIT_BIO,
	HYPERHOLD_END_IO,
	HYPERHOLD_SCHED_WORK,
	HYPERHOLD_END_WORK,
	HYPERHOLD_CALL_BACK,
	HYPERHOLD_WAKE_UP,
	HYPERHOLD_ZRAM_LOCK,
	HYPERHOLD_DONE,
	HYPERHOLD_KYE_POINT_BUTT
};

enum hyperhold_cache_level {
	HYPERHOLD_CACHE_FAULT_OUT = 1,
	HYPERHOLD_CACHE_BATCH_OUT = 2,
	HYPERHOLD_CACHE_RECLAIM_IN = 4,
};

enum hyperhold_fail_point {
	HYPERHOLD_FAULT_OUT_INIT_FAIL = 0,
	HYPERHOLD_FAULT_OUT_ENTRY_ALLOC_FAIL,
	HYPERHOLD_FAULT_OUT_IO_ENTRY_PARA_FAIL,
	HYPERHOLD_FAULT_OUT_SEGMENT_ALLOC_FAIL,
	HYPERHOLD_FAULT_OUT_BIO_ALLOC_FAIL,
	HYPERHOLD_FAULT_OUT_BIO_ADD_FAIL,
	HYPERHOLD_FAULT_OUT_IO_FAIL,
	HYPERHOLD_FAIL_POINT_BUTT
};

enum hp_sapce_type {
	HP_PATTION_SPACE = 0,
	HP_FILE_SPACE,
	HP_SPACE_TYPE_BUTT
};

struct hyperhold_fail_record {
	unsigned char task_comm[TASK_COMM_LEN];
	enum hyperhold_fail_point point;
	ktime_t time;
	u32 index;
	int ext_id;
};

struct hyperhold_fail_record_info {
	int num;
	spinlock_t lock;
	struct hyperhold_fail_record record[MAX_FAIL_RECORD_NUM];
};

struct hyperhold_key_point_info {
	unsigned int record_cnt;
	unsigned int end_cnt;
	ktime_t first_time;
	ktime_t last_time;
	s64 proc_total_time;
	s64 proc_max_time;
	unsigned long long last_ravg_sum;
	unsigned long long proc_ravg_sum;
	spinlock_t time_lock;
};

struct hyperhold_key_point_record {
	struct timer_list lat_monitor;
	unsigned long warning_threshold;
	int page_cnt;
	int segment_cnt;
	int nice;
	bool timeout_flag;
	unsigned char task_comm[TASK_COMM_LEN];
	struct task_struct *task;
	enum hyperhold_scenario scenario;
	struct hyperhold_key_point_info key_point[HYPERHOLD_KYE_POINT_BUTT];
};

struct hyperhold_lat_stat {
	atomic64_t total_lat;
	atomic64_t max_lat;
	atomic64_t timeout_cnt;
};

struct hyperhold_stat {
	atomic64_t reclaimin_cnt;
	atomic64_t reclaimin_bytes;
	atomic64_t reclaimin_pages;
#ifdef CONFIG_RAMTURBO
#ifdef CONFIG_HYPERHOLD_ZSWAPD
	atomic64_t zswapd_reclaimin_bytes;
#endif
	atomic64_t app_active_reclaimin_bytes;
#endif
	atomic64_t reclaimin_infight;
	atomic64_t batchout_cnt;
	atomic64_t batchout_bytes;
	atomic64_t batchout_pages;
	atomic64_t batchout_inflight;
	atomic64_t fault_cnt;
	atomic64_t hyperhold_fault_cnt;
	atomic64_t reout_pages;
	atomic64_t reout_bytes;
	atomic64_t zram_stored_pages;
	atomic64_t zram_stored_size;
	atomic64_t stored_pages;
	atomic64_t stored_size;
	atomic64_t notify_free;
	atomic64_t frag_cnt;
	atomic64_t mcg_cnt;
	atomic64_t ext_cnt;
	atomic64_t miss_free;
	atomic64_t mcgid_clear;
	atomic64_t io_fail_cnt[HYPERHOLD_SCENARIO_BUTT];
	atomic64_t alloc_fail_cnt[HYPERHOLD_SCENARIO_BUTT];
	struct hyperhold_lat_stat lat[HYPERHOLD_SCENARIO_BUTT];
	struct hyperhold_fail_record_info record;

	atomic64_t discard_total_cnt;
	atomic64_t discard_comp_cnt;
	atomic64_t discard_fail_cnt;
	atomic64_t discard_success_cnt;
};

struct hyperhold_page_pool {
	struct list_head page_pool_list;
	spinlock_t page_pool_lock;
};

struct hyperhold_buffer {
	struct zram *zram;
	struct hyperhold_page_pool *pool;
	struct page **dest_pages;
};

struct hyperhold_entry {
	int ext_id;
	sector_t addr;
	struct page **dest_pages;
	int pages_sz;
	struct list_head list;
	void *private;
	void *manager_private;
};

struct io_priv {
	struct zram *zram;
	enum hyperhold_scenario scenario;
	struct hyperhold_page_pool page_pool;
};

struct schedule_para {
	void *io_handler;
	struct hyperhold_entry *io_entry;
	struct hyperhold_buffer io_buf;
	struct io_priv priv;
	struct hyperhold_key_point_record record;
};

struct hyperhold_io {
	struct block_device *bdev;
	enum hyperhold_scenario scenario;
	void (*done_callback)(struct hyperhold_entry *, int);
	void (*complete_notify)(void *);
	void *private;
	struct hyperhold_key_point_record *record;
};

struct space_para_cfg {
	unsigned long space_type;
	unsigned long max_size;
};

struct space_info_para {
	unsigned long min_file_num;
	unsigned long max_file_num;
	unsigned long alloced_file_num;
	unsigned long max_file_size;
	unsigned long min_file_size;
	unsigned long file_magic;
	unsigned long alloced_exts;
	unsigned long max_pages;
	unsigned long max_exts;
	unsigned long used_exts;
	spinlock_t exts_lock;
	struct hyperhold_file_ops *file_ops;
	sector_t *start_sector;
	unsigned long *exts_sum;
	struct block_device *userdata_bdev;
};

struct hyperhold_file_ops {
	int (*alloc)(const char *file_path, unsigned long long size);
	int (*recur_alloc)(const char *file_path, unsigned long long size);
	int (*check)(const char *file_path);
	void (*free)(const char *file_path, unsigned long long size);
	int (*get_sector)(const char *file_path, sector_t *start_sector);
};

void *hyperhold_malloc(size_t size, bool fast, bool nofail);

void hyperhold_free(const void *mem);

unsigned long hyperhold_zsmalloc(struct zs_pool *zs_pool,
			size_t size, struct hyperhold_page_pool *pool);

struct page *hyperhold_alloc_page(
		struct hyperhold_page_pool *pool, gfp_t gfp,
		bool fast, bool nofail);

void hyperhold_page_recycle(struct page *page,
		struct hyperhold_page_pool *pool);

struct hyperhold_stat *hyperhold_get_stat_obj(void);

struct zram *hyperhold_get_global_zram(void);

void hp_cache_show(struct seq_file *m, struct zram *zram);

unsigned long hyperhold_cache_pages(struct zram *zram);

int hyperhold_manager_init(struct zram *zram);

void hyperhold_manager_memcg_init(struct mem_cgroup *mcg, struct zram *zram);

void hyperhold_manager_memcg_deinit(struct mem_cgroup *mcg);

void hyperhold_zram_lru_add(struct zram *zram, u32 index,
					struct mem_cgroup *mcg);

#ifdef CONFIG_CRYPTO_DELTA
void hyperhold_change_size_of_page(struct zram *zram, u32 index,
					u32 size_diff);
#endif

void hyperhold_zram_lru_del(struct zram *zram, u32 index);

unsigned long hyperhold_extent_create(struct mem_cgroup *mcg,
					int *ext_id,
					struct hyperhold_buffer *buf,
					void **private);

void hyperhold_extent_register(void *private);

void hyperhold_extent_objs_del(struct zram *zram, u32 index);

int hyperhold_find_extent_by_idx(
	unsigned long eswpentry, struct hyperhold_buffer *buf, void **private);

int hyperhold_find_extent_by_memcg(
			struct mem_cgroup *mcg,
			struct hyperhold_buffer *buf, void **private);

void hyperhold_extent_destroy(void *private, enum hyperhold_scenario scenario);

void hyperhold_extent_exception(enum hyperhold_scenario scenario,
					void *private);

void hyperhold_manager_deinit(struct zram *zram);

struct mem_cgroup *hyperhold_zram_get_memcg(struct zram *zram, u32 index);

int hyperhold_set_cache_level(struct zram *zram, int level);

int hyperhold_cache_decomp(struct zram *zram, u32 index, struct page *page);

int hyperhold_cache_out(struct zram *zram, u32 index,
		unsigned long addr, bool unpin);

unsigned long hyperhold_drop_cache(struct zram *zram, unsigned long nr_pages);

int hyperhold_move_cache(struct zram *zram, int nr_exts);

void hyperhold_cache_state(struct zram *zram, char *buf, ssize_t *size);

int hyperhold_schedule_init(void);

void *hyperhold_plug_start(struct hyperhold_io *io_para);
void *hyperhold_req_init(struct hyperhold_io *io_para,
	bool wait_io_finish_flag);

int hyperhold_io_extent(void *io_handler,
	struct hyperhold_entry *io_entry);

int hyperhold_read_extent(void *io_handler,
				struct hyperhold_entry *io_entry);

int hyperhold_write_extent(void *io_handler,
				struct hyperhold_entry *io_entry);

#ifdef CONFIG_RAMTURBO
int hyperhold_plug_finish(void *io_handler,
		enum hyperhold_reclaim_source source);
#else
int hyperhold_plug_finish(void *io_handler);
#endif

void hyperhold_perf_start(
	struct hyperhold_key_point_record *record,
	ktime_t stsrt, unsigned long long start_ravg_sum,
	enum hyperhold_scenario scenario);

void hyperhold_perf_end(struct hyperhold_key_point_record *record);

void hyperhold_perf_lat_start(
	struct hyperhold_key_point_record *record,
	enum hyperhold_key_point type);

void hyperhold_perf_lat_end(
	struct hyperhold_key_point_record *record,
	enum hyperhold_key_point type);

void hyperhold_perf_lat_point(
	struct hyperhold_key_point_record *record,
	enum hyperhold_key_point type);

void hyperhold_perf_async_perf(
	struct hyperhold_key_point_record *record,
	enum hyperhold_key_point type, ktime_t start,
	unsigned long long start_ravg_sum);

void hyperhold_perf_io_stat(
	struct hyperhold_key_point_record *record, int page_cnt,
	int segment_cnt);

static inline unsigned long long hyperhold_get_ravg_sum(void)
{
#ifdef CONFIG_TASK_RAVG_SUM
	return current->ravg.ravg_sum;
#else
	return 0;
#endif
}

void hyperhold_fail_record(enum hyperhold_fail_point point,
	u32 index, int ext_id, unsigned char *task_comm);

bool hyperhold_reclaim_in_enable(void);
struct workqueue_struct *hyperhold_get_reclaim_workqueue(void);

void memcg_idle_count(struct zram *zram);
void memcg_idle_inc(struct zram *zram, u32 index);
void memcg_idle_dec(struct zram *zram, u32 index);

sector_t hyperhold_get_sector(int ext_id);

void hyperhold_area_set_bitmap(struct zram *zram, int index);
void hyperhold_area_clear_bitmap(struct zram *zram, int index);
bool hyperhold_is_file_space(void);

struct hyperhold_file_ops *hyperhold_get_file_ops(void);

struct space_para_cfg *hyperhold_space_para(void);
struct space_info_para *hyperhold_space_info(void);
struct mutex *hyperhold_segment_mutex(void);
void hyperhold_set_cfg(void);
bool hyperhold_get_cfg(void);
#ifdef CONFIG_DFX_DEBUG_FS
bool hyperhold_get_nolimit(void);
#endif

void hyperhold_expend_space(void);
int hyperhold_alloc_space(struct zram *zram);
void hyperhold_set_default_space_proc(void);
int hyperhold_free_space(struct block_device *bdev);
void hyperhold_space_sector_deinit(void);
int hyperhold_space_sector_init(void);
int hyperhold_used_exts_num_inc(void);
void hyperhold_used_exts_num_dec(void);
bool hyperhold_space_is_full(int add_num);
int hyperhold_set_space(struct block_device *bdev,
	struct blk_hp_cmd *cmd);

int esentry_pkg_extid(unsigned long e);
void hyperhold_stat_alloc_fail(enum hyperhold_scenario scenario,
	int err);
#endif
