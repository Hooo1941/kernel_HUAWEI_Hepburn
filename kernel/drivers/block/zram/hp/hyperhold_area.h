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

#ifndef _HYPERHOLD_AREA_H
#define _HYPERHOLD_AREA_H

#include <linux/memcontrol.h>
#include <linux/rbtree.h>
#include "hyperhold_list.h"
#ifdef CONFIG_HH_REMAP
#include "hyperhold_remap.h"
#endif

struct hyperhold_area {
	unsigned long size;
	int nr_objs;
	int nr_exts;
	int nr_mcgs;

#ifndef CONFIG_HH_REMAP
	unsigned long *bitmap;
	atomic_t last_alloc_bit;
#else
	struct hh_zone *zones;
	struct list_head free_zones;
	spinlock_t free_op_lock;
	struct list_head used_zones;
	spinlock_t used_op_lock;
	struct list_head remap_zones;
	struct hh_zone *cur_zone;
	spinlock_t cur_op_lock;
#endif

	struct hh_list_table *ext_table;
	struct hh_list_head *ext;

	struct hh_list_table *obj_table;
	struct hh_list_head *rmap;
	struct hh_list_head *lru;

	atomic_t stored_exts;
	atomic_t *ext_stored_pages;
	atomic64_t *ext_stored_size;

	unsigned int mcg_id_cnt[MEM_CGROUP_ID_MAX + 1];

	struct hyperhold_cache *cache;
	struct shrinker cache_shrinker;
	atomic64_t cache_shrinker_runs;
	atomic64_t cache_shrinker_reclaim_pages;

	atomic64_t cache_move_runs;
	atomic64_t cache_move_exts;

	bool fault_out_cache_enable;
	bool batch_out_cache_enable;
	bool reclaim_in_cache_enable;

	struct work_struct hyperhold_discard_work;
	struct rb_root discard_tree;

	spinlock_t hyperhold_discard_lock;
};

struct discard_type {
	struct rb_node node;
	int extent_id;
};

#define AREA_WATERMARK_HIGH 80
#define AREA_WATERMARK_LOW 70

int hp_high_ext_num(int tot);
int hp_low_ext_num(int tot);

int esentry_pgid(unsigned long e);
int esentry_pgoff(unsigned long e);
struct mem_cgroup *get_memcg_with_css_get(unsigned short mcg_id);

struct mem_cgroup *get_mem_cgroup(unsigned short mcg_id);

int obj_idx(struct hyperhold_area *area, int idx);
int ext_idx(struct hyperhold_area *area, int idx);
int mcg_idx(struct hyperhold_area *area, int idx);

void free_hyperhold_area(struct hyperhold_area *area);
struct hyperhold_area *alloc_hyperhold_area(unsigned long ori_size,
					    unsigned long comp_size);
void hyperhold_free_pkg_extent(struct hyperhold_area *area, int ext_id, bool discard_wq);
int hyperhold_alloc_pkg_extent(struct hyperhold_area *area, struct mem_cgroup *mcg);
int get_extent(struct hyperhold_area *area, int ext_id);
void put_extent(struct hyperhold_area *area, int ext_id);
int get_memcg_extent(struct hyperhold_area *area, struct mem_cgroup *mcg,
		bool (*filter)(struct hyperhold_area *area, int ext_id));
int get_memcg_zram_entry(struct hyperhold_area *area, struct mem_cgroup *mcg);
int get_extent_zram_entry(struct hyperhold_area *area, int ext_id);

unsigned long hyperhold_shrink_cache(struct hyperhold_area *area,
		unsigned long nr_pages);
bool hp_cache_init(struct hyperhold_area *area);
#ifdef CONFIG_HH_REMAP
int extent_id2bit(struct hyperhold_area *area, int id);
int extent_bit2id(struct hyperhold_area *area, int bit);
struct discard_type *discard_search(struct rb_root *root, int extent_id);
#endif

#endif
