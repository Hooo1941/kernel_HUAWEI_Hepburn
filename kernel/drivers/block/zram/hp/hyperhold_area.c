/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020. All rights reserved.
 * Description: hyperhold implement
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

#define pr_fmt(fmt) "[HYPERHOLD]" fmt

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/zsmalloc.h>
#include <linux/blkdev.h>
#include "zram_drv.h"
#include "hyperhold_internal.h"
#include "hyperhold_area.h"
#include "hyperhold_cache.h"
#include "hyperhold_list.h"
#ifdef CONFIG_HH_REMAP_DEBUG
#include "hyperhold_remap_debug.h"
#endif

struct mem_cgroup *get_memcg_with_css_get(unsigned short mcg_id)
{
	struct mem_cgroup *mcg = NULL;

	if (!mcg_id)
		return NULL;

again:
	rcu_read_lock();
	mcg = mem_cgroup_from_id(mcg_id);
	if (mcg && !css_tryget(&mcg->css)) {
		rcu_read_unlock();
		goto again;
	}
	rcu_read_unlock();

	return mcg;
}

#ifndef CONFIG_HH_REMAP
static struct discard_type *discard_search(struct rb_root *root, int extent_id)
#else
struct discard_type *discard_search(struct rb_root *root, int extent_id)
#endif
{
	struct rb_node *node = root->rb_node;

	while (node) {
		struct discard_type *data =
			container_of(node, struct discard_type, node);
		int result;

		result = extent_id - data->extent_id;

		if (result < 0)
			node = node->rb_left;
		else if (result > 0)
			node = node->rb_right;
		else
			return data;
	}
	return NULL;
}

static int discard_insert(struct rb_root *root, struct discard_type *data)
{
	struct rb_node **new = &(root->rb_node), *parent = NULL;

	/* Figure out where to put new node */
	while (*new) {
		struct discard_type *cur =
			container_of(*new, struct discard_type, node);
		int result = data->extent_id - cur->extent_id;

		parent = *new;
		if (result < 0)
			new = &((*new)->rb_left);
		else if (result > 0)
			new = &((*new)->rb_right);
		else
			return -EEXIST;
	}

	/* Add new node and rebalance tree. */
	rb_link_node(&data->node, parent, new);
	rb_insert_color(&data->node, root);

	return 0;
}

struct mem_cgroup *get_mem_cgroup(unsigned short mcg_id)
{
	struct mem_cgroup *mcg = NULL;

	rcu_read_lock();
	mcg = mem_cgroup_from_id(mcg_id);
	rcu_read_unlock();

	return mcg;
}

static bool fragment_dec(bool prev_flag, bool next_flag,
			 struct hyperhold_stat *stat)
{
	if (prev_flag && next_flag) {
		atomic64_inc(&stat->frag_cnt);
		return false;
	}

	if (prev_flag || next_flag)
		return false;

	return true;
}

static bool fragment_inc(bool prev_flag, bool next_flag,
			 struct hyperhold_stat *stat)
{
	if (prev_flag && next_flag) {
		atomic64_dec(&stat->frag_cnt);
		return false;
	}

	if (prev_flag || next_flag)
		return false;

	return true;
}

static bool prev_is_cont(struct hyperhold_area *area, int ext_id, int mcg_id)
{
	int prev;

	if (is_first_idx(ext_idx(area, ext_id), mcg_idx(area, mcg_id),
			 area->ext_table))
		return false;
	prev = prev_idx(ext_idx(area, ext_id), area->ext_table);

	return (prev >= 0) && (ext_idx(area, ext_id) == prev + 1);
}

static bool next_is_cont(struct hyperhold_area *area, int ext_id, int mcg_id)
{
	int next;

	if (is_last_idx(ext_idx(area, ext_id), mcg_idx(area, mcg_id),
			area->ext_table))
		return false;
	next = next_idx(ext_idx(area, ext_id), area->ext_table);

	return (next >= 0) && (ext_idx(area, ext_id) + 1 == next);
}

static void ext_fragment_sub(struct hyperhold_area *area, int ext_id)
{
	bool prev_flag = false;
	bool next_flag = false;
	int mcg_id;
	struct hyperhold_stat *stat = hyperhold_get_stat_obj();

	if (!stat) {
		hh_print(HHLOG_ERR, "NULL stat\n");
		return;
	}

	if (!area->ext_table) {
		hh_print(HHLOG_ERR, "NULL table\n");
		return;
	}
	if (ext_id < 0 || ext_id >= area->nr_exts) {
		hh_print(HHLOG_ERR, "ext = %d invalid\n", ext_id);
		return;
	}

	mcg_id = hh_list_get_mcgid(ext_idx(area, ext_id), area->ext_table);
	if (mcg_id <= 0 || mcg_id >= area->nr_mcgs) {
		hh_print(HHLOG_ERR, "mcg_id = %d invalid\n", mcg_id);
		return;
	}

	atomic64_dec(&stat->ext_cnt);
	area->mcg_id_cnt[mcg_id]--;
	if (area->mcg_id_cnt[mcg_id] == 0) {
		atomic64_dec(&stat->mcg_cnt);
		atomic64_dec(&stat->frag_cnt);
		return;
	}

	prev_flag = prev_is_cont(area, ext_id, mcg_id);
	next_flag = next_is_cont(area, ext_id, mcg_id);
	if (fragment_dec(prev_flag, next_flag, stat))
		atomic64_dec(&stat->frag_cnt);
}

static void ext_fragment_add(struct hyperhold_area *area, int ext_id)
{
	bool prev_flag = false;
	bool next_flag = false;
	int mcg_id;
	struct hyperhold_stat *stat = hyperhold_get_stat_obj();

	if (!stat) {
		hh_print(HHLOG_ERR, "NULL stat\n");
		return;
	}

	if (!area->ext_table) {
		hh_print(HHLOG_ERR, "NULL table\n");
		return;
	}
	if (ext_id < 0 || ext_id >= area->nr_exts) {
		hh_print(HHLOG_ERR, "ext = %d invalid\n", ext_id);
		return;
	}

	mcg_id = hh_list_get_mcgid(ext_idx(area, ext_id), area->ext_table);
	if (mcg_id <= 0 || mcg_id >= area->nr_mcgs) {
		hh_print(HHLOG_ERR, "mcg_id = %d invalid\n", mcg_id);
		return;
	}

	atomic64_inc(&stat->ext_cnt);
	if (area->mcg_id_cnt[mcg_id] == 0) {
		area->mcg_id_cnt[mcg_id]++;
		atomic64_inc(&stat->frag_cnt);
		atomic64_inc(&stat->mcg_cnt);
		return;
	}
	area->mcg_id_cnt[mcg_id]++;

	prev_flag = prev_is_cont(area, ext_id, mcg_id);
	next_flag = next_is_cont(area, ext_id, mcg_id);
	if (fragment_inc(prev_flag, next_flag, stat))
		atomic64_inc(&stat->frag_cnt);
}

static int extent_calc_bit2id(int max, int bit)
{
	return max - bit - 1;
}

#ifndef CONFIG_HH_REMAP
static int extent_bit2id(struct hyperhold_area *area, int bit)
#else
int extent_bit2id(struct hyperhold_area *area, int bit)
#endif
{
	if (bit < 0 || bit >= area->nr_exts) {
		hh_print(HHLOG_ERR, "bit = %d invalid\n", bit);
		return -EINVAL;
	}

	return extent_calc_bit2id(area->nr_exts, bit);
}

#ifndef CONFIG_HH_REMAP
static int extent_id2bit(struct hyperhold_area *area, int id)
#else
int extent_id2bit(struct hyperhold_area *area, int id)
#endif
{
	if (id < 0 || id >= area->nr_exts) {
		hh_print(HHLOG_ERR, "id = %d invalid\n", id);
		return -EINVAL;
	}

	return area->nr_exts - id - 1;
}

int obj_idx(struct hyperhold_area *area, int idx)
{
	if (!area) {
		hh_print(HHLOG_ERR, "NULL area\n");
		return -EINVAL;
	}
	if (idx < 0 || idx >= area->nr_objs) {
		hh_print(HHLOG_ERR, "idx = %d invalid\n", idx);
		return -EINVAL;
	}

	return idx;
}

int ext_idx(struct hyperhold_area *area, int idx)
{
	if (!area) {
		hh_print(HHLOG_ERR, "NULL area\n");
		return -EINVAL;
	}
	if (idx < 0 || idx >= area->nr_exts) {
		hh_print(HHLOG_ERR, "idx = %d invalid\n", idx);
		return -EINVAL;
	}

	return idx + area->nr_objs;
}

int mcg_idx(struct hyperhold_area *area, int idx)
{
	if (!area) {
		hh_print(HHLOG_ERR, "NULL area\n");
		return -EINVAL;
	}
	if (idx <= 0 || idx >= area->nr_mcgs) {
		hh_print(HHLOG_ERR, "idx = %d invalid\n", idx);
		return -EINVAL;
	}

	return idx + area->nr_objs + area->nr_exts;
}

static struct hh_list_head *get_obj_table_node(int idx, void *private)
{
	struct hyperhold_area *area = private;

	if (!area) {
		hh_print(HHLOG_ERR, "NULL area\n");
		return NULL;
	}
	if (idx < 0) {
		hh_print(HHLOG_ERR, "idx = %d invalid\n", idx);
		return NULL;
	}
	if (idx < area->nr_objs)
		return &area->lru[idx];
	idx -= area->nr_objs;
	if (idx < area->nr_exts)
		return &area->rmap[idx];
	idx -= area->nr_exts;
	if (idx > 0 && idx < area->nr_mcgs) {
		struct mem_cgroup *mcg = get_mem_cgroup(idx);

		if (!mcg)
			goto err_out;
		return (struct hh_list_head *)(&mcg->zram_lru);
	}
err_out:
	hh_print(HHLOG_ERR, "idx = %d invalid\n", idx);

	return NULL;
}

static void free_obj_list_table(struct hyperhold_area *area)
{
	if (!area) {
		hh_print(HHLOG_ERR, "NULL area\n");
		return;
	}

	if (area->lru) {
		vfree(area->lru);
		area->lru = NULL;
	}
	if (area->rmap) {
		vfree(area->rmap);
		area->rmap = NULL;
	}

	kfree(area->obj_table);
	area->obj_table = NULL;
}

static int init_obj_list_table(struct hyperhold_area *area)
{
	int i;

	if (!area) {
		hh_print(HHLOG_ERR, "NULL area\n");
		return -EINVAL;
	}

	area->lru = vzalloc(sizeof(struct hh_list_head) * area->nr_objs);
	if (!area->lru) {
		hh_print(HHLOG_ERR, "area->lru alloc failed\n");
		goto err_out;
	}
	area->rmap = vzalloc(sizeof(struct hh_list_head) * area->nr_exts);
	if (!area->rmap) {
		hh_print(HHLOG_ERR, "area->rmap alloc failed\n");
		goto err_out;
	}
	area->obj_table = alloc_table(get_obj_table_node, area, GFP_KERNEL);
	if (!area->obj_table) {
		hh_print(HHLOG_ERR, "area->obj_table alloc failed\n");
		goto err_out;
	}
	for (i = 0; i < area->nr_objs; i++)
		hh_list_init(obj_idx(area, i), area->obj_table);
	for (i = 0; i < area->nr_exts; i++)
		hh_list_init(ext_idx(area, i), area->obj_table);

	hh_print(HHLOG_INFO, "hyperhold obj list table init OK.\n");
	return 0;
err_out:
	free_obj_list_table(area);
	hh_print(HHLOG_ERR, "hyperhold obj list table init failed.\n");

	return -ENOMEM;
}

static struct hh_list_head *get_ext_table_node(int idx, void *private)
{
	struct hyperhold_area *area = private;

	if (!area) {
		hh_print(HHLOG_ERR, "NULL area\n");
		return NULL;
	}

	if (idx < area->nr_objs)
		goto err_out;
	idx -= area->nr_objs;
	if (idx < area->nr_exts)
		return &area->ext[idx];
	idx -= area->nr_exts;
	if (idx > 0 && idx < area->nr_mcgs) {
		struct mem_cgroup *mcg = get_mem_cgroup(idx);

		if (!mcg)
			return NULL;
		return (struct hh_list_head *)(&mcg->ext_lru);
	}
err_out:
	hh_print(HHLOG_ERR, "idx = %d invalid\n", idx);

	return NULL;
}

static void free_ext_list_table(struct hyperhold_area *area)
{
	if (!area) {
		hh_print(HHLOG_ERR, "NULL area\n");
		return;
	}

	if (area->ext)
		vfree(area->ext);

	kfree(area->ext_table);
}

static int init_ext_list_table(struct hyperhold_area *area)
{
	int i;

	if (!area) {
		hh_print(HHLOG_ERR, "NULL area\n");
		return -EINVAL;
	}
	area->ext = vzalloc(sizeof(struct hh_list_head) * area->nr_exts);
	if (!area->ext)
		goto err_out;
	area->ext_table = alloc_table(get_ext_table_node, area, GFP_KERNEL);
	if (!area->ext_table)
		goto err_out;
	for (i = 0; i < area->nr_exts; i++)
		hh_list_init(ext_idx(area, i), area->ext_table);
	hh_print(HHLOG_INFO, "hyperhold ext list table init OK.\n");
	return 0;
err_out:
	free_ext_list_table(area);
	hh_print(HHLOG_ERR, "hyperhold ext list table init failed.\n");

	return -ENOMEM;
}

unsigned long hyperhold_memcg_shrink_cache(struct hyperhold_area *area,
		struct mem_cgroup *mcg,
		unsigned long nr_pages)
{
	int idx;
	unsigned long freed = 0;
	int mcg_id = mcg->id.id;

	hh_lock_list(mcg_idx(area, mcg_id), area->ext_table);
	hh_list_for_each_entry_reverse(idx, mcg_idx(area, mcg_id),
					area->ext_table) {
		int ext_id = idx - area->nr_objs;
		long size = atomic64_read(&area->ext_stored_size[ext_id]);
		if (size < CACHE_SIZE_LOW)
			continue;
		if (!hyperhold_cache_delete(area->cache, ext_id)) {
			freed += PKG_EXTENT_PG_CNT;
			if (freed >= nr_pages)
				break;
		}
	}
	hh_unlock_list(mcg_idx(area, mcg_id), area->ext_table);

	return freed;
}

unsigned long hyperhold_shrink_cache(struct hyperhold_area *area,
		unsigned long nr_pages)
{
	unsigned long freed = 0;
	struct mem_cgroup *mcg = NULL;

	for (mcg = get_next_memcg(NULL); mcg; mcg = get_next_memcg(mcg)) {
		if (atomic64_read(&mcg->memcg_reclaimed.app_score) == 0)
			break;
		if (!mcg->zram)
			continue;
		freed += hyperhold_memcg_shrink_cache(area, mcg,
			nr_pages - freed);
		if (freed >= nr_pages)
			break;
	}

	if (mcg) {
		get_next_memcg_break(mcg);
		mcg = NULL;
	}

	return freed;
}

unsigned long hyperhold_cache_count_objs(struct shrinker *sh,
		struct shrink_control *sc)
{
	struct hyperhold_area *area =
			container_of(sh, struct hyperhold_area, cache_shrinker);

	hh_print(HHLOG_DEBUG, "run cache counter\n");
	return (unsigned long)area->cache->nr_cached_exts * PKG_EXTENT_PG_CNT;
}

unsigned long hyperhold_cache_scan_objs(struct shrinker *sh,
		struct shrink_control *sc)
{
	struct hyperhold_area *area =
			container_of(sh, struct hyperhold_area, cache_shrinker);
	unsigned long freed = 0;

	atomic64_inc(&area->cache_shrinker_runs);
	hh_print(HHLOG_DEBUG, "run cache shrinker, total %ld, nr = %lu\n",
		atomic64_read(&area->cache_shrinker_runs), sc->nr_to_scan);
	freed = hyperhold_shrink_cache(area, sc->nr_to_scan);
	atomic64_add(freed, &area->cache_shrinker_reclaim_pages);
	hh_print(HHLOG_DEBUG, "cache shrinker reclaimed %ld, total %ld\n",
		freed, atomic64_read(&area->cache_shrinker_reclaim_pages));
	if (!freed)
		return SHRINK_STOP;
	return freed;
}

void free_hyperhold_area(struct hyperhold_area *area)
{
	if (!area) {
		hh_print(HHLOG_ERR, "NULL area\n");
		return;
	}
#ifndef CONFIG_HH_REMAP
	vfree(area->bitmap);
#else
	vfree(area->zones);
#endif
	vfree(area->ext_stored_pages);
	vfree(area->ext_stored_size);
	hyperhold_cache_free(area->cache);
	unregister_shrinker(&area->cache_shrinker);
	free_obj_list_table(area);
	free_ext_list_table(area);
	vfree(area);
}

static struct discard_type *hyperhold_discard_first(const struct rb_root *root,
					     struct hyperhold_area *area)
{
	int ret;
	struct rb_node *node = NULL;
	struct discard_type *data = NULL;

	spin_lock(&area->hyperhold_discard_lock);
	/* get the first discard extent (in sort order) */
	node = rb_first(root);
	if (!node) {
		hh_print(HHLOG_ERR, "rb_tree null\n");
		spin_unlock(&area->hyperhold_discard_lock);
		return NULL;
	}
	data = rb_entry(node, struct discard_type, node);
#ifndef CONFIG_HH_REMAP
	ret = test_and_set_bit(extent_id2bit(area, data->extent_id), area->bitmap);
#else
	ret = test_and_set_bit_in_zone_bitmap_by_extent(area, data->extent_id);
#endif
	if (ret) {
		hh_print(HHLOG_DEBUG, "rb_first_bit is set\n");
		spin_unlock(&area->hyperhold_discard_lock);
		return NULL;
	}
	spin_unlock(&area->hyperhold_discard_lock);

	return data;
}

struct discard_type *hyperhold_discard_next(const struct rb_node *node,
					    struct hyperhold_area *area)
{
	int ret;
	struct discard_type *data = NULL;

	spin_lock(&area->hyperhold_discard_lock);
	node = rb_next(node);
	if (!node) {
		hh_print(HHLOG_DEBUG, "rb_next_bit is null\n");
		spin_unlock(&area->hyperhold_discard_lock);
		return NULL;
	}
	data = rb_entry(node, struct discard_type, node);
#ifndef CONFIG_HH_REMAP
	ret = test_and_set_bit(extent_id2bit(area, data->extent_id),
			       area->bitmap);
#else
	ret = test_and_set_bit_in_zone_bitmap_by_extent(area, data->extent_id);
#endif
	if (ret) {
		hh_print(HHLOG_DEBUG, "rb_next_bit is set\n");
		spin_unlock(&area->hyperhold_discard_lock);
		return NULL;
	}
	spin_unlock(&area->hyperhold_discard_lock);

	return data;
}

static void hyperhold_issue_discard(int ext_id, int ext_nr,
				    struct hyperhold_stat *stat)
{
	int ret;
	sector_t sector;
	struct block_device *bdev = hyperhold_bdev();

	if (!bdev) {
		hh_print(HHLOG_ERR, "bedv is null\n");
		return;
	}

	sector = hyperhold_get_sector(ext_id);
	if (!sector) {
		hh_print(HHLOG_ERR, "sector is err\n");
		return;
	}

	ret = blkdev_issue_discard(bdev, sector,
				   ext_nr * EXTENT_SECTOR_SIZE, GFP_KERNEL, 0);
	if (ret) {
		atomic64_inc(&stat->discard_fail_cnt);
		hh_print(HHLOG_ERR, "blkdev_issue_discard failed, ret = %d\n",
			 ret);
	} else {
		atomic64_inc(&stat->discard_success_cnt);
	}
}

static void hyperhold_discard_tree_erase(struct hyperhold_area *area,
					 int ext_id, int ext_nr)
{
	int i;
	struct rb_node *node = NULL;
	struct discard_type *data = NULL;

	spin_lock(&area->hyperhold_discard_lock);
	data = discard_search(&area->discard_tree, ext_id);
	for (i = 0; (i < ext_nr) && (data != NULL); i++) {
		if ((ext_id + i) != data->extent_id)
			continue;
		hh_print(HHLOG_ERR, "erase ext_id = %d\n", data->extent_id);
#ifndef CONFIG_HH_REMAP
		if (!test_and_clear_bit(extent_id2bit(area, data->extent_id),
					area->bitmap)) {
#else
		if (!test_and_clear_bit_in_zone_bitmap_by_extent(area, data->extent_id)) {
#endif
			hh_print(HHLOG_DEBUG, "bit not set, ext = %d\n",
				 data->extent_id);
			WARN_ON_ONCE(1);
		}
#ifdef CONFIG_HH_REMAP
		mayfree_zone_bitmap_by_extent(area, data->extent_id);
#endif
		rb_erase(&data->node, &area->discard_tree);
		node = rb_next(&data->node);
		kfree(data);
		if (!node)
			break;
		data = rb_entry(node, struct discard_type, node);
	}
	spin_unlock(&area->hyperhold_discard_lock);
}

void hyperhold_do_discard(struct work_struct *work)
{
	struct discard_type *data = NULL;
	int temp_ext, temp_nr;
	struct hyperhold_area *area = container_of(work, struct hyperhold_area,
						   hyperhold_discard_work);
	struct hyperhold_stat *stat = hyperhold_get_stat_obj();

	if (!stat) {
		hh_print(HHLOG_ERR, "NULL stat\n");
		return;
	}

	/* get the first discard extent (in sort order) */
	data = hyperhold_discard_first(&area->discard_tree, area);
	if (!data) {
		hh_print(HHLOG_DEBUG, "first extent error\n");
		return;
	}
	temp_ext = data->extent_id;
	temp_nr = 1;

	while (data) {
		atomic64_inc(&stat->discard_total_cnt);
		/* get the next discard extent (in sort order) */
		data = hyperhold_discard_next(&data->node, area);
		if (!data) {
			hh_print(HHLOG_ERR, "next extent error\n");
			break;
		}

		/* merge discard extent, otherwise discard previous extent */
		if ((temp_ext + temp_nr) == data->extent_id) {
			atomic64_inc(&stat->discard_comp_cnt);
			temp_nr++;
		} else {
			hyperhold_issue_discard(temp_ext, temp_nr, stat);
			hyperhold_discard_tree_erase(area, temp_ext, temp_nr);
			temp_ext = data->extent_id;
			temp_nr = 1;
		}
	}
	/* issue discard for the last extent */
	hyperhold_issue_discard(temp_ext, temp_nr, stat);
	hyperhold_discard_tree_erase(area, temp_ext, temp_nr);
}

void hyperhold_discard_extent(struct hyperhold_area *area)
{
	if (!hyperhold_discard_enable())
		return;

	if (!work_busy(&area->hyperhold_discard_work))
		queue_work(system_freezable_wq, &area->hyperhold_discard_work);
}

static void hyperhold_discard_init(struct hyperhold_area *area)
{
	struct block_device *bdev = hyperhold_bdev();
	struct request_queue *q = NULL;

	if (!bdev) {
		hh_print(HHLOG_ERR, "bedv is null\n");
		return;
	}
	q = bdev_get_queue(bdev);
	/* enable hyperhold_discard for unistore */
	if (blk_queue_query_unistore_enable(q)) {
		hh_print(HHLOG_ERR, "unistore enable discard\n");
		hyperhold_set_discard_enable(true);
		INIT_WORK(&area->hyperhold_discard_work, hyperhold_do_discard);
		spin_lock_init(&area->hyperhold_discard_lock);
	} else {
		hh_print(HHLOG_ERR, "not unistore\n");
	}
}

bool hp_cache_init(struct hyperhold_area *area)
{
	area->fault_out_cache_enable = false;
	area->batch_out_cache_enable = false;
	area->reclaim_in_cache_enable = false;
	area->cache = hyperhold_cache_alloc(area->nr_exts);
	if (!area->cache) {
		hh_print(HHLOG_ERR, "alloc hyperhold cache failed\n");
	return false;
	}
	area->cache_shrinker.count_objects = hyperhold_cache_count_objs;
	area->cache_shrinker.scan_objects = hyperhold_cache_scan_objs;
	area->cache_shrinker.seeks = DEFAULT_SEEKS;
	if (register_shrinker(&area->cache_shrinker)) {
		hh_print(HHLOG_ERR, "register cache shrinker failed\n");
		return false;
	}
	return true;
}

struct hyperhold_area *alloc_hyperhold_area(unsigned long ori_size,
					    unsigned long comp_size)
{
	struct hyperhold_area *area = vzalloc(sizeof(struct hyperhold_area));
	if (!area) {
		hh_print(HHLOG_ERR, "area alloc failed\n");
		goto err_out;
	}
	if (comp_size & (EXTENT_SIZE - 1)) {
		hh_print(HHLOG_ERR,
			 "disksize = %ld align invalid (extent align needed)\n",
			 comp_size);
		goto err_out;
	}
#ifdef CONFIG_HH_REMAP_DEBUG
	hh_print(HHLOG_ERR, "debug mode before change comp_size = %ld\n", comp_size);
	comp_size = REMAP_DEBUG_SIZE;
	hh_print(HHLOG_ERR, "debug mode change comp_size = %d\n", REMAP_DEBUG_SIZE);
#endif
	area->size = comp_size;
	area->nr_exts = comp_size >> EXTENT_SHIFT;
	area->nr_mcgs = MEM_CGROUP_ID_MAX;
	area->nr_objs = ori_size >> PAGE_SHIFT;
#ifndef CONFIG_HH_REMAP
	area->bitmap = vzalloc(BITS_TO_LONGS(area->nr_exts) * sizeof(long));
	if (!area->bitmap) {
#else
	if (alloc_hyperhold_bitmap_zone(area)) {
#endif
		hh_print(HHLOG_ERR, "area->bitmap alloc failed\n");
		goto err_out;
	}
	area->ext_stored_pages = vzalloc(sizeof(atomic_t) * area->nr_exts);
	if (!area->ext_stored_pages) {
		hh_print(HHLOG_ERR, "area->ext_stored_pages alloc failed\n");
		goto err_out;
	}
	area->ext_stored_size = vzalloc(sizeof(atomic64_t) * area->nr_exts);
	if (!area->ext_stored_size) {
		hh_print(HHLOG_ERR, "area->ext_stored_size alloc failed\n");
		goto err_out;
	}
	if (!hp_cache_init(area))
	    goto err_out;
	if (init_obj_list_table(area)) {
		hh_print(HHLOG_ERR, "init obj list table failed\n");
		goto err_out;
	}
	if (init_ext_list_table(area)) {
		hh_print(HHLOG_ERR, "init ext list table failed\n");
		goto err_out;
	}
	hyperhold_discard_init(area);
	hh_print(HHLOG_INFO, "hyperhold_area init OK.\n");
	return area;
err_out:
	free_hyperhold_area(area);
	hh_print(HHLOG_ERR, "hyperhold_area init failed.\n");
	return NULL;
}

static void discard_insert_extent(struct hyperhold_area *area, int ext_id)
{
	int ret = -EEXIST;
	struct discard_type *newtype =
		kmalloc(sizeof(struct discard_type), GFP_ATOMIC);

	if (!newtype) {
		hh_print(HHLOG_ERR, "allocate memory failed\n");
		return;
	}
	newtype->extent_id = ext_id;

	spin_lock(&area->hyperhold_discard_lock);
#ifndef CONFIG_HH_REMAP
	if (test_bit(extent_id2bit(area, ext_id), area->bitmap))
#else
	if (test_bit_in_zone_bitmap_by_extent(area, ext_id))
#endif
		ret = discard_insert(&area->discard_tree, newtype);
	spin_unlock(&area->hyperhold_discard_lock);

	if (ret < 0) {
		hh_print(HHLOG_ERR, "rb_tree insert failed\n");
		kfree(newtype);
	}
}

static void hyperhold_free_extent(struct hyperhold_area *area, int ext_id,
	bool discard_wq, int total_num)
{
	int offset;

	if (!area) {
		hh_print(HHLOG_ERR, "NULL area\n");
		return;
	}
	if (ext_id < 0 || ext_id >= area->nr_exts) {
		hh_print(HHLOG_ERR, "INVALID ext %d\n", ext_id);
		return;
	}
	hh_print(HHLOG_DEBUG, "free ext id = %d.\n", ext_id);

	if (hyperhold_cache_delete(area->cache, ext_id) == -EBUSY) {
		hh_print(HHLOG_ERR, "delete busy cache, ext = %d\n", ext_id);
		WARN_ON_ONCE(1);
	}

	hh_list_set_mcgid(ext_idx(area, ext_id), area->ext_table, 0);

	for (offset = 0; offset < total_num; ++offset) {
		if (hyperhold_discard_enable() && discard_wq)
			discard_insert_extent(area, ext_id + offset);

		/*lint -e548*/
#ifndef CONFIG_HH_REMAP
		if (!test_and_clear_bit(extent_id2bit(area, ext_id + offset), area->bitmap)) {
#else
		if (!test_and_clear_bit_in_zone_bitmap_by_extent(area, ext_id + offset)) {
#endif
			hh_print(HHLOG_ERR, "bit not set, ext = %d\n", ext_id + offset);
			WARN_ON_ONCE(1);
		}
		hyperhold_used_exts_num_dec();
	}

	if (hyperhold_discard_enable() && discard_wq)
		hyperhold_discard_extent(area);
#ifdef CONFIG_HH_REMAP
	else
		mayfree_zone_bitmap_by_extent(area, ext_id);
#endif

	/*lint +e548*/
	atomic_sub(total_num, &area->stored_exts);
}

void hyperhold_free_pkg_extent(struct hyperhold_area *area, int ext_id,
			   bool discard_wq)
{
	hyperhold_free_extent(area, ext_id, discard_wq, PKG_EXTENT_NUM);
}

#ifndef CONFIG_HH_REMAP
static void discard_bitmap(struct hyperhold_area *area, int bit)
{
	int ext_id;
	struct discard_type *data = NULL;

	ext_id = extent_bit2id(area, bit);

	/* try to erase extent from the discard_tree */
	spin_lock(&area->hyperhold_discard_lock);
	data = discard_search(&area->discard_tree, ext_id);
	if (data) {
		hh_print(HHLOG_DEBUG, "erase exist ext_id = %d\n", ext_id);
		rb_erase(&data->node, &area->discard_tree);
		kfree(data);
	}
	spin_unlock(&area->hyperhold_discard_lock);
}

static int alloc_bitmap(unsigned long *bitmap, int max, int last_bit)
{
	int bit;
	unsigned long flags;
	struct space_info_para *space_info = hyperhold_space_info();

	if (!bitmap) {
		hh_print(HHLOG_ERR, "NULL bitmap.\n");
		return -EINVAL;
	}

	spin_lock_irqsave(&space_info->exts_lock, flags);
	if (hyperhold_is_file_space() &&
		(space_info->used_exts >= space_info->alloced_exts)) {
		hh_print(HHLOG_WARN, "space full\n");
		bit = -ENOSPC;
		goto out;
	}
retry:
	bit = find_next_zero_bit(bitmap, max, last_bit);
	if (bit == max) {
		if (last_bit == 0) {
			hh_print(HHLOG_ERR, "alloc bitmap failed.\n");
			bit = -ENOSPC;
			goto out;
		}
		last_bit = 0;
		goto retry;
	}

	if (test_and_set_bit(bit, bitmap))
		goto retry;

	if (hyperhold_is_file_space())
		space_info->used_exts++;
out:
	spin_unlock_irqrestore(&space_info->exts_lock, flags);
	return bit;
}

static int alloc_bitmap_discard(unsigned long *bitmap, int max, int last_bit,
				struct hyperhold_area *area)
{
	int bit;

	bit = alloc_bitmap(bitmap, max, last_bit);
	if (bit >= 0)
		discard_bitmap(area, bit);

	return bit;
}

#ifdef CONFIG_HP_TURBO
static void discard_pkg_bitmap(struct hyperhold_area *area, int bit)
{
	int offset;

	for (offset = 0; offset < PKG_EXTENT_NUM; ++offset)
		discard_bitmap(area, bit + offset);
}

static int alloc_set_pkg_bitmap(unsigned long *bitmap, int bit)
{
	int offset;
	int clear_offset;

	for (offset = 0; offset < PKG_EXTENT_NUM; ++offset) {
		if (test_and_set_bit(bit + offset, bitmap))
			break;
	}

	if (offset == PKG_EXTENT_NUM)
		return 0;

	for (clear_offset = 0; clear_offset <= offset; ++clear_offset)
		if (test_and_clear_bit(bit + clear_offset, bitmap))
			hh_print(HHLOG_ERR, "bit not set, bit = %d\n", bit + clear_offset);

	return -EINVAL;
}

static int hyperhold_pkg_space_check(int max, int bit)
{
	int start_id;
	int end_id;
	sector_t start_addr;
	sector_t end_addr;

	if (!hyperhold_is_file_space() || !(bit & PKG_EXTENT_OFFSET_MASK))
		return 0;

	start_id = extent_calc_bit2id(max, bit + PKG_EXTENT_NUM - 1);
	end_id = extent_calc_bit2id(max, bit);

	start_addr = hyperhold_get_sector(start_id);
	end_addr = hyperhold_get_sector(end_id);
	if ((start_addr + (PKG_EXTENT_NUM - 1) * HYPERHOLD_PAGE_SIZE_SECTOR) == end_addr)
		return 0;
	else
		return -EINVAL;
}
#endif

static int alloc_pkg_bitmap(unsigned long *bitmap, int max, int last_bit)
{
#ifdef CONFIG_HP_TURBO
	int bit;
	int next_bit;
	int end_bit;
	bool end_flag = false;
	unsigned long flags;
	struct space_info_para *space_info = hyperhold_space_info();

	if (!bitmap) {
		hh_print(HHLOG_ERR, "NULL bitmap.\n");
		return -EINVAL;
	}

	spin_lock_irqsave(&space_info->exts_lock, flags);
	if (hyperhold_is_file_space() &&
		((space_info->used_exts + PKG_EXTENT_NUM) > space_info->alloced_exts)) {
		hh_print(HHLOG_WARN, "space full\n");
		bit = -ENOSPC;
		goto out;
	}
retry:
	bit = find_next_zero_bit(bitmap, max, last_bit);
	end_bit = bit + PKG_EXTENT_NUM;
	if (end_bit > max) {
		if (end_flag) {
			hh_print(HHLOG_WARN, "alloc bitmap failed.\n");
			bit = -ENOSPC;
			goto out;
		}
		last_bit = 0;
		end_flag = true;
		goto retry;
	}

	next_bit = find_next_bit(bitmap, end_bit, bit);
	if (end_bit > next_bit) {
		last_bit = (next_bit != max) ? next_bit : 0;
		goto retry;
	}

	if (hyperhold_pkg_space_check(max, bit)) {
		last_bit = (next_bit != max) ? next_bit : 0;
		goto retry;
	}

	if (alloc_set_pkg_bitmap(bitmap, bit))
		bit = -ENOSPC;
	else if (hyperhold_is_file_space())
		space_info->used_exts += PKG_EXTENT_NUM;

out:
	spin_unlock_irqrestore(&space_info->exts_lock, flags);

	if (bit >= 0)
		bit = bit + PKG_EXTENT_NUM - 1;

	return bit;
#else
	return alloc_bitmap(bitmap, max, last_bit);
#endif
}

static int alloc_pkg_bitmap_discard(unsigned long *bitmap, int max, int last_bit,
				struct hyperhold_area *area)
{
#ifdef CONFIG_HP_TURBO
	int bit;

	bit = alloc_pkg_bitmap(bitmap, max, last_bit);
	if (bit >= 0)
		discard_pkg_bitmap(area, bit);

	return bit;
#else
	return alloc_bitmap_discard(bitmap, max, last_bit, area);
#endif
}
#endif

#ifdef CONFIG_HP_TURBO
void hyperhold_free_nopkg_extent(int ext_id)
{
	struct zram *zram = hyperhold_get_global_zram();

	if (!zram || !zram->area) {
		hh_print(HHLOG_ERR, "NULL zram or area\n");
		return;
	}

	hyperhold_free_extent(zram->area, ext_id, true, NO_PKG_EXTENT_NUM);
}

int hyperhold_alloc_nopkg_extent(void)
{
	struct zram *zram = hyperhold_get_global_zram();
	int last_bit;
	int bit;
	int ext_id;

	if (!zram || !zram->area) {
		hh_print(HHLOG_ERR, "NULL zram or area\n");
		return -EINVAL;
	}

	if (hyperhold_space_is_full(NO_PKG_EXTENT_NUM))
		return -ENOSPC;

	last_bit = atomic_read(&zram->area->last_alloc_bit);
	hh_print(HHLOG_DEBUG, "last_bit = %d.\n", last_bit);
	if (hyperhold_discard_enable())
		bit = alloc_bitmap_discard(zram->area->bitmap,
			zram->area->nr_exts, last_bit, zram->area);
	else
		bit = alloc_bitmap(zram->area->bitmap, zram->area->nr_exts,
			last_bit);

	if (bit < 0) {
		hh_print(HHLOG_ERR, "alloc bitmap failed, bit = %d\n", bit);
		return bit;
	}
	ext_id = extent_bit2id(zram->area, bit);

	atomic_set(&zram->area->last_alloc_bit, bit);
	atomic_add(NO_PKG_EXTENT_NUM, &zram->area->stored_exts);
	hh_print(HHLOG_DEBUG, "extent %d init OK.\n", ext_id);

	return ext_id;
}
#endif

int hyperhold_alloc_pkg_extent(struct hyperhold_area *area, struct mem_cgroup *mcg)
{
	int last_bit;
	int bit;
	int ext_id;
	int mcg_id;

	if (!area) {
		hh_print(HHLOG_ERR, "NULL area\n");
		return -EINVAL;
	}
	if (!mcg) {
		hh_print(HHLOG_ERR, "NULL mcg\n");
		return -EINVAL;
	}

	if (hyperhold_space_is_full(PKG_EXTENT_NUM))
		return -ENOSPC;

#ifndef CONFIG_HH_REMAP
	last_bit = atomic_read(&area->last_alloc_bit);
	hh_print(HHLOG_DEBUG, "last_bit = %d.\n", last_bit);
	if (hyperhold_discard_enable())
		bit = alloc_pkg_bitmap_discard(area->bitmap, area->nr_exts,
			last_bit, area);
	else
		bit = alloc_pkg_bitmap(area->bitmap, area->nr_exts, last_bit);
#else
	bit = alloc_bitmap_with_zones(area, hyperhold_discard_enable());
#endif
	if (bit < 0) {
		hh_print(HHLOG_ERR, "alloc bitmap failed, bit = %d\n", bit);
		return bit;
	}
	ext_id = extent_bit2id(area, bit);

	mcg_id = hh_list_get_mcgid(ext_idx(area, ext_id), area->ext_table);
	/*lint -e548*/
	if (mcg_id) {
		hh_print(HHLOG_ERR, "already has mcg %d, ext %d\n",
				mcg_id, ext_id);
		goto err_out;
	}
	/*lint +e548*/
	hh_list_set_mcgid(ext_idx(area, ext_id), area->ext_table, mcg->id.id);

#ifndef CONFIG_HH_REMAP
	atomic_set(&area->last_alloc_bit, bit);
#endif
	atomic_add(PKG_EXTENT_NUM, &area->stored_exts);
	hh_print(HHLOG_DEBUG, "extent %d init OK.\n", ext_id);
	hh_print(HHLOG_DEBUG, "mcg_id = %d, ext id = %d\n", mcg->id.id, ext_id);

	return ext_id;
err_out:
#ifndef CONFIG_HH_REMAP
	hyperhold_free_pkg_extent(area, ext_id, false);
#else
	test_and_clear_bit_in_zone_bitmap_by_extent(area, ext_id);
#endif
	WARN_ON_ONCE(1); /*lint !e548*/
	return -EBUSY;
}

int get_extent(struct hyperhold_area *area, int ext_id)
{
	int mcg_id;

	if (!area) {
		hh_print(HHLOG_ERR, "NULL area\n");
		return -EINVAL;
	}
	if (ext_id < 0 || ext_id >= area->nr_exts) {
		hh_print(HHLOG_ERR, "ext = %d invalid\n", ext_id);
		return -EINVAL;
	}

	if (!hh_list_clear_priv(ext_idx(area, ext_id), area->ext_table))
		return -EBUSY;
	mcg_id = hh_list_get_mcgid(ext_idx(area, ext_id), area->ext_table);
	if (mcg_id) {
		ext_fragment_sub(area, ext_id);
		hh_list_del(ext_idx(area, ext_id), mcg_idx(area, mcg_id),
			    area->ext_table);
	}
	hh_print(HHLOG_DEBUG, "ext id = %d\n", ext_id);

	return ext_id;
}

void put_extent(struct hyperhold_area *area, int ext_id)
{
	int mcg_id;

	if (!area) {
		hh_print(HHLOG_ERR, "NULL area\n");
		return;
	}
	if (ext_id < 0 || ext_id >= area->nr_exts) {
		hh_print(HHLOG_ERR, "ext = %d invalid\n", ext_id);
		return;
	}

	mcg_id = hh_list_get_mcgid(ext_idx(area, ext_id), area->ext_table);
	if (mcg_id) {
		hh_lock_list(mcg_idx(area, mcg_id), area->ext_table);
		hh_list_add_nolock(ext_idx(area, ext_id), mcg_idx(area, mcg_id),
			area->ext_table);
		ext_fragment_add(area, ext_id);
		hh_unlock_list(mcg_idx(area, mcg_id), area->ext_table);
	}
	/*lint -e548*/
	if (!hh_list_set_priv(ext_idx(area, ext_id), area->ext_table)) {
		hh_print(HHLOG_ERR, "private not set, ext = %d\n", ext_id);
		WARN_ON_ONCE(1);
		return;
	}
	/*lint +e548*/
	hh_print(HHLOG_DEBUG, "put extent %d.\n", ext_id);
}

int get_memcg_extent(struct hyperhold_area *area, struct mem_cgroup *mcg,
		bool (*filter)(struct hyperhold_area *area, int ext_id))
{
	int mcg_id;
	int ext_id = -ENOENT;
	int idx;

	if (!area) {
		hh_print(HHLOG_ERR, "NULL area\n");
		return -EINVAL;
	}
	if (!area->ext_table) {
		hh_print(HHLOG_ERR, "NULL table\n");
		return -EINVAL;
	}
	if (!mcg) {
		hh_print(HHLOG_ERR, "NULL mcg\n");
		return -EINVAL;
	}

	mcg_id = mcg->id.id;
	hh_lock_list(mcg_idx(area, mcg_id), area->ext_table);
	if (mcg != get_mem_cgroup(mcg_id)) {
		ext_id = ((struct hh_list_head*)(&mcg->ext_lru))->next;
		ext_id -= area->nr_objs;
		hh_print(HHLOG_ERR, "memcg corrupted, get ext_id %d directly.\n", ext_id);
		if (ext_id < 0 || ext_id >= area->nr_exts)
			ext_id = -ENOENT;
		goto unlock;
	}
	hh_list_for_each_entry(idx, mcg_idx(area, mcg_id), area->ext_table) { /*lint !e666*/
		if (filter && !filter(area, idx - area->nr_objs))
			continue;
		if (hh_list_clear_priv(idx, area->ext_table)) {
			ext_id = idx - area->nr_objs;
			break;
		}
	}
	if (ext_id >= 0 && ext_id < area->nr_exts) {
		ext_fragment_sub(area, ext_id);
		hh_list_del_nolock(idx, mcg_idx(area, mcg_id), area->ext_table);
		hh_print(HHLOG_DEBUG, "ext id = %d\n", ext_id);
	}
unlock:
	hh_unlock_list(mcg_idx(area, mcg_id), area->ext_table);

	return ext_id;
}

int get_memcg_zram_entry(struct hyperhold_area *area, struct mem_cgroup *mcg)
{
	int mcg_id, idx;
	int index = -ENOENT;

	if (!area) {
		hh_print(HHLOG_ERR, "NULL area\n");
		return -EINVAL;
	}
	if (!area->obj_table) {
		hh_print(HHLOG_ERR, "NULL table\n");
		return -EINVAL;
	}
	if (!mcg) {
		hh_print(HHLOG_ERR, "NULL mcg\n");
		return -EINVAL;
	}

	mcg_id = mcg->id.id;
	hh_lock_list(mcg_idx(area, mcg_id), area->obj_table);
	if (mcg != get_mem_cgroup(mcg_id)) {
		index = ((struct hh_list_head*)(&mcg->zram_lru))->next;
		hh_print(HHLOG_ERR, "memcg corrupted, get index %d directly.\n", index);
		if (index >= area->nr_objs)
			index = -ENOENT;
		goto unlock;
	}
	hh_list_for_each_entry(idx, mcg_idx(area, mcg_id), area->obj_table) { /*lint !e666*/
		index = idx;
		break;
	}
unlock:
	hh_unlock_list(mcg_idx(area, mcg_id), area->obj_table);

	return index;
}

int get_extent_zram_entry(struct hyperhold_area *area, int ext_id)
{
	int index = -ENOENT;
	int idx;

	if (!area) {
		hh_print(HHLOG_ERR, "NULL area\n");
		return -EINVAL;
	}
	if (!area->obj_table) {
		hh_print(HHLOG_ERR, "NULL table\n");
		return -EINVAL;
	}
	if (ext_id < 0 || ext_id >= area->nr_exts) {
		hh_print(HHLOG_ERR, "ext = %d invalid\n", ext_id);
		return -EINVAL;
	}

	hh_lock_list(ext_idx(area, ext_id), area->obj_table);
	hh_list_for_each_entry(idx, ext_idx(area, ext_id), area->obj_table) { /*lint !e666*/
		index = idx;
		break;
	}
	hh_unlock_list(ext_idx(area, ext_id), area->obj_table);

	return index;
}

ssize_t hyperhold_discard_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t size = 0;
	struct hyperhold_stat *stat = hyperhold_get_stat_obj();

	if (!stat) {
		hh_print(HHLOG_ERR, "NULL stat\n");
		return size;
	}

	size += scnprintf(buf + size, PAGE_SIZE,
			  "hyperhold total discard: %lld\n",
			  atomic64_read(&stat->discard_total_cnt));
	size += scnprintf(buf + size, PAGE_SIZE,
			  "hyperhold success discard: %lld\n",
			  atomic64_read(&stat->discard_success_cnt));
	size += scnprintf(buf + size, PAGE_SIZE, "hyperhold fail discard: %lld\n",
			  atomic64_read(&stat->discard_fail_cnt));
	size += scnprintf(buf + size, PAGE_SIZE, "hyperhold comp discard: %lld\n",
			  atomic64_read(&stat->discard_comp_cnt));

	return size;
}
int hp_high_ext_num(int tot)
{
	return (tot) * AREA_WATERMARK_HIGH / 100;
}

int hp_low_ext_num(int tot)
{
	return (tot) * AREA_WATERMARK_LOW / 100;
}

int esentry_pkg_extid(unsigned long e)
{
#ifdef CONFIG_HP_TURBO
	int offset = (int)(e >> PKG_EXTENT_OFFSET_SHIFT);
	int ext_id = ((e) & PKG_EXTENT_ADDR_MASK) >> EXTENT_SHIFT;

	return (((ext_id - offset) >> PKG_EXTENT_NUM_SHIFT) <<
		PKG_EXTENT_NUM_SHIFT ) + offset;
#else
	return (e) >> EXTENT_SHIFT;
#endif
}

int esentry_pgid(unsigned long e)
{
#ifdef CONFIG_HP_TURBO
	int offset = (int)(e >> PKG_EXTENT_OFFSET_SHIFT);
	int pgid = ((e & PKG_EXTENT_ADDR_MASK) &
		((1 << PKG_EXTENT_SHIFT) - 1)) >> PAGE_SHIFT;

	return (pgid + PKG_EXTENT_NUM - offset) & (PKG_EXTENT_NUM - 1);
#else
	return ((e) & ((1<<EXTENT_SHIFT) - 1))>> PAGE_SHIFT;
#endif
}

int esentry_pgoff(unsigned long e)
{
#ifdef CONFIG_HP_TURBO
	return ((e) & PKG_EXTENT_ADDR_MASK) & (PAGE_SIZE - 1);
#else
	return (e) & (PAGE_SIZE - 1);
#endif
}

void hyperhold_area_set_bitmap(struct zram *zram, int index)
{
	int ret;
	struct hyperhold_area *area = zram->area;

#ifndef CONFIG_HH_REMAP
	ret = test_and_set_bit(extent_id2bit(area, index), area->bitmap);
#else
	ret = test_and_set_bit_in_zone_bitmap_by_extent(area, index);
#endif
	if (unlikely(ret))
		hh_print(HHLOG_ERR, "set fail, index %u\n", index);
}

void hyperhold_area_clear_bitmap(struct zram *zram, int index)
{
	int ret;
	struct hyperhold_area *area = zram->area;

#ifndef CONFIG_HH_REMAP
	ret = test_and_clear_bit(extent_id2bit(area, index), area->bitmap);
#else
	ret = test_and_clear_bit_in_zone_bitmap_by_extent(area, index);
#endif
	if (unlikely(ret))
		hh_print(HHLOG_ERR, "clear fail, index %u\n", index);
}
