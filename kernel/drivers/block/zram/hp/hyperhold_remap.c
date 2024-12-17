/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020. All rights reserved.
 * Description: hyperhold remap implement.
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
 * Author:	Li Peng <lipeng113@huawei.com>
 *
 * Create: 2023-3-3
 *
 */

#include <linux/delay.h>
#include "hyperhold_remap.h"
#ifdef CONFIG_HH_REMAP_DEBUG
#include "hyperhold_remap_debug.h"
#endif

int alloc_hyperhold_bitmap_zone(struct hyperhold_area *area)
{
	int i;
	int zone_nr = BITS_TO_LONGS(area->nr_exts);
	area->zones = vzalloc(zone_nr * sizeof(struct hh_zone));
	if (!area->zones)
		return -ENOMEM;

	hh_print(HHLOG_DEBUG, "%s, zones %p, size %d\n", __func__, area->zones, zone_nr);
	INIT_LIST_HEAD(&area->free_zones);
	INIT_LIST_HEAD(&area->used_zones);
	INIT_LIST_HEAD(&area->remap_zones);
	area->cur_zone = NULL;
	for (i = 0; i < zone_nr; i++) {
		area->zones[i].bit_max = BITS_PER_TYPE(unsigned long);
		area->zones[i].zone_off = i;
		list_add_tail(&area->zones[i].list, &area->free_zones);
	}
	if (area->nr_exts % BITS_PER_TYPE(unsigned long))
		area->zones[zone_nr - 1].bit_max = area->nr_exts % BITS_PER_TYPE(unsigned long);
	spin_lock_init(&area->free_op_lock);
	spin_lock_init(&area->used_op_lock);
	spin_lock_init(&area->cur_op_lock);
	return 0;
}

bool test_bit_in_zone_bitmap_by_extent(struct hyperhold_area *area, int ext_id)
{
	int bit_nr = extent_id2bit(area, ext_id);
	int zone_idx;
	int zone_bit;

	if (bit_nr != -EINVAL) {
		zone_idx = bit_nr / BITS_PER_TYPE(unsigned long);
		zone_bit = bit_nr % BITS_PER_TYPE(unsigned long);
		hh_print(HHLOG_DEBUG, "zone_idx %d, bit %d\n", zone_idx, zone_bit);
		return test_bit(zone_bit, &area->zones[zone_idx].bitmap);
	}
	return false;
}

bool test_and_set_bit_in_zone_bitmap_by_extent(struct hyperhold_area *area, int ext_id)
{
	int bit_nr = extent_id2bit(area, ext_id);
	int zone_idx;
	int zone_bit;

	if (bit_nr != -EINVAL) {
		zone_idx = bit_nr / BITS_PER_TYPE(unsigned long);
		zone_bit = bit_nr % BITS_PER_TYPE(unsigned long);
		hh_print(HHLOG_DEBUG, "zone_idx %d, bit %d\n", zone_idx, zone_bit);
		return test_and_set_bit(zone_bit, &area->zones[zone_idx].bitmap);
	}
	return false;
}

bool test_and_clear_bit_in_zone_bitmap_by_extent(struct hyperhold_area *area, int ext_id)
{
	int bit_nr = extent_id2bit(area, ext_id);
	int zone_idx;
	int zone_bit;

	if (bit_nr != -EINVAL) {
		zone_idx = bit_nr / BITS_PER_TYPE(unsigned long);
		zone_bit = bit_nr % BITS_PER_TYPE(unsigned long);
		hh_print(HHLOG_DEBUG, "zone_idx %d, bit %d\n", zone_idx, zone_bit);
		return test_and_clear_bit(zone_bit, &area->zones[zone_idx].bitmap);
	}
	return false;
}

void mayfree_zone_bitmap_by_extent(struct hyperhold_area *area, int ext_id)
{
	int bit_nr = extent_id2bit(area, ext_id);
	int zone_idx;
	unsigned long flags;

	if (bit_nr != -EINVAL) {
		zone_idx = bit_nr / BITS_PER_TYPE(unsigned long);
		if (!hweight64(area->zones[zone_idx].bitmap)) {
			hh_print(HHLOG_DEBUG, "%s, idx %d put into freelist\n", __func__, zone_idx);
			spin_lock_irqsave(&area->used_op_lock, flags);
			list_del_init(&area->zones[zone_idx].list);
			spin_unlock(&area->used_op_lock);
			spin_lock(&area->free_op_lock);
			list_add_tail(&area->zones[zone_idx].list, &area->free_zones);
			spin_unlock_irqrestore(&area->free_op_lock, flags);
		}
	}
	return;
}

int alloc_bitmap_with_zones(struct hyperhold_area *area, bool discard)
{
	int bit;
	int ext_id;
	struct discard_type *data = NULL;
	int last_bit;
	struct hh_zone *cur;
	unsigned long flags;

	if (unlikely(!area->zones)) {
		hh_print(HHLOG_ERR, "remap zones not init yet.\n");
		return -EINVAL;
	}
retry:
	spin_lock_irqsave(&area->cur_op_lock, flags);
	cur = area->cur_zone;
	if (likely(cur)) {
		spin_unlock_irqrestore(&area->cur_op_lock, flags);
		last_bit = atomic_read(&cur->last_alloc_bit);
inner_retry:
		bit = find_next_zero_bit(&cur->bitmap, cur->bit_max, last_bit);
		if (bit == cur->bit_max) {
			atomic_set(&cur->last_alloc_bit, 0);
			spin_lock_irqsave(&area->cur_op_lock, flags);
			if (cur == area->cur_zone)
				area->cur_zone = NULL;
			spin_unlock_irqrestore(&area->cur_op_lock, flags);
			goto retry;
		}
#ifdef CONFIG_HH_REMAP_DEBUG
		if (g_hh_remap_step && !(bit % g_hh_remap_step)) {
			hh_print(HHLOG_DEBUG, "flag %d bit %d drop!\n", g_hh_remap_step, bit);
			atomic_set(&cur->last_alloc_bit, bit + 1);
			goto retry;
		}
#endif
		if (test_and_set_bit(bit, &cur->bitmap))
			goto inner_retry;

		if (hyperhold_used_exts_num_inc()) {
			test_and_clear_bit(bit, &cur->bitmap);
			hh_print(HHLOG_ERR, "hyperhold_used_exts_num_inc err.\n");
			return -ENOSPC;
		}
		atomic_set(&cur->last_alloc_bit, bit);
		bit += BITS_PER_TYPE(unsigned long) * cur->zone_off;
	} else {
		spin_lock(&area->free_op_lock);
		if (likely(!list_empty(&area->free_zones))) {
			area->cur_zone = list_first_entry(&area->free_zones, struct hh_zone, list);
			list_del_init(&area->cur_zone->list);
			spin_unlock(&area->free_op_lock);
			spin_lock(&area->used_op_lock);
			list_add_tail(&area->cur_zone->list, &area->used_zones);
			spin_unlock(&area->used_op_lock);
			spin_unlock_irqrestore(&area->cur_op_lock, flags);
			goto retry;
		} else {
			spin_unlock(&area->free_op_lock);
			spin_lock(&area->used_op_lock);
			list_for_each_entry (cur, &area->used_zones, list) {
#ifndef CONFIG_HH_REMAP_DEBUG
				if (hweight64(cur->bitmap) < cur->bit_max) {
#else
				if ((g_hh_remap_step && (hweight64(cur->bitmap) < (cur->bit_max / g_hh_remap_step))) ||
				    (!g_hh_remap_step && (hweight64(cur->bitmap) < cur->bit_max))) {
#endif
					atomic_set(&cur->last_alloc_bit, 0);
					area->cur_zone = cur;
					spin_unlock(&area->used_op_lock);
					spin_unlock_irqrestore(&area->cur_op_lock, flags);
					goto retry;
				}
			}
			spin_unlock(&area->used_op_lock);
			spin_unlock_irqrestore(&area->cur_op_lock, flags);
			hh_print(HHLOG_ERR, "no bit can alloc.\n");
			return -ENOSPC;
		}
	}

	if (discard) {
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

	return bit;
}

static struct hh_zone *remap_get_last_used_zone(struct hyperhold_area *area)
{
	struct hh_zone *last, *tmp;
	unsigned long flags;

	spin_lock_irqsave(&area->cur_op_lock, flags);
	spin_lock(&area->used_op_lock);
	list_for_each_entry_safe_reverse (last, tmp, &area->used_zones, list) {
		if (last != area->cur_zone) {
			list_del_init(&last->list);
			list_add_tail(&last->list, &area->remap_zones);
			spin_unlock(&area->used_op_lock);
			spin_unlock_irqrestore(&area->cur_op_lock, flags);
			return last;
		}
	}
	spin_unlock(&area->used_op_lock);
	spin_unlock_irqrestore(&area->cur_op_lock, flags);
	return NULL;
}

static int remap_alloc_bit(struct hyperhold_area *area, bool discard)
{
	struct hh_zone *cur;
	int bit;
	int ext_id;
	struct discard_type *data = NULL;
	unsigned long flags;

	spin_lock_irqsave(&area->used_op_lock, flags);
	list_for_each_entry (cur, &area->used_zones, list) {
retry:
		bit = find_next_zero_bit(&cur->bitmap, cur->bit_max, 0);
		if (bit == cur->bit_max)
			continue;
		if (test_and_set_bit(bit, &cur->bitmap))
			goto retry;
		spin_unlock_irqrestore(&area->used_op_lock, flags);
		if (hyperhold_used_exts_num_inc()) {
			test_and_clear_bit(bit, &cur->bitmap);
			hh_print(HHLOG_ERR, "hyperhold_used_exts_num_inc err.\n");
			return -ENOSPC;
		}
		hh_print(HHLOG_DEBUG, "remap alloc bit %d, zone %d\n", bit, cur->zone_off);
		bit += BITS_PER_TYPE(unsigned long) * cur->zone_off;

		if (discard) {
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
		return bit;
	}
	spin_unlock_irqrestore(&area->used_op_lock, flags);
	return -ENOSPC;
}

static void remap_unalloc_bit(struct hyperhold_area *area, int ext_id)
{
	hyperhold_used_exts_num_dec();
	if (!test_and_clear_bit_in_zone_bitmap_by_extent(area, ext_id)) {
		hh_print(HHLOG_ERR, "bit not set, ext = %d\n", ext_id);
		WARN_ON_ONCE(1);
	}
}

static void remap_extent_mginfo(struct hyperhold_area *area, int src_bit, int dst_bit, bool succ)
{
	/* here should be compele before real use */
	if (succ)
		hyperhold_free_pkg_extent(area, extent_bit2id(area, src_bit), hyperhold_discard_enable());
	else
		hyperhold_free_pkg_extent(area, extent_bit2id(area, dst_bit), hyperhold_discard_enable());
}

static int remap_aftprocess(struct hyperhold_area *area, struct list_head *head, bool succ)
{
	struct remap_map *map, *tmp;
	struct hh_zone *fzone, *tzone;
	unsigned long flags;

	list_for_each_entry_safe (map, tmp, head, list) {
		remap_extent_mginfo(area, map->src_bit, map->dst_bit, succ);
		list_del_init(&map->list);
		hyperhold_free(map);
	}

	spin_lock_irqsave(&area->used_op_lock, flags);
	list_for_each_entry_safe (fzone, tzone, &area->remap_zones, list) {
		list_del_init(&fzone->list);
		if (hweight64(fzone->bitmap)) {
			list_add_tail(&fzone->list, &area->used_zones);
		}
		else {
			spin_lock(&area->free_op_lock);
			list_add_tail(&fzone->list, &area->free_zones);
			spin_unlock(&area->free_op_lock);
		}
		hh_print(HHLOG_DEBUG, "zone %d move to working list, %d\n", fzone->zone_off, hweight64(fzone->bitmap));
	}
	spin_unlock_irqrestore(&area->used_op_lock, flags);
	return 0;
}

static struct remap_map *remap_get_bit_parir_info(struct hyperhold_area *area, struct hh_zone *cur_zone, int *offset)
{
	struct remap_map *map = NULL;
	int src_bit;
	int dst_bit;

	src_bit = find_next_bit(&cur_zone->bitmap, cur_zone->bit_max, *offset);
	*offset = src_bit + 1;
	if (src_bit < cur_zone->bit_max) {
		dst_bit = remap_alloc_bit(area, hyperhold_discard_enable());
		if (dst_bit >= 0) {
			map = hyperhold_malloc(sizeof(struct remap_map), false, true);
			if (map) {
				map->dst_bit = dst_bit;
				map->src_bit = src_bit + BITS_PER_TYPE(unsigned long) * cur_zone->zone_off;
				hh_print(HHLOG_DEBUG, "src %d => dst %d\n", map->src_bit, map->dst_bit);
			} else {
				hh_print(HHLOG_DEBUG, "no mem\n");
				remap_unalloc_bit(area, dst_bit);
			}
		} else {
			hh_print(HHLOG_DEBUG, "alloc remap bit failed. %d\n", dst_bit);
		}
	}

	return map;
}

static int remap_get_pairs(struct hyperhold_area *area, int require_seqsize_in_zone, struct remap_io *rio)
{
	int acture_remap_zone_cnt = 0;
	struct hh_zone *src_zone;
	int start_bit;
	struct remap_map *map;

	while (acture_remap_zone_cnt < require_seqsize_in_zone) {
		src_zone = remap_get_last_used_zone(area);
		if (src_zone) {
			hh_print(HHLOG_DEBUG, "remap src zone. off %d\n", src_zone->zone_off);
			start_bit = 0;
			while (start_bit < src_zone->bit_max) {
				map = remap_get_bit_parir_info(area, src_zone, &start_bit);
				if (map) {
					list_add_tail(&map->list, &rio->pair_list);
					rio->cnt++;
				} else {
					break;
				}
			}
			acture_remap_zone_cnt++;
		} else {
			hh_print(HHLOG_DEBUG, "no other zone ready to remap\n");
			break;
		}
	}
	return acture_remap_zone_cnt;
}

static void remap_back_done(struct stor_dev_data_remap_response_info info, void *data)
{
	struct remap_io *rio = (struct remap_io *)data;

	hh_print(HHLOG_DEBUG, "remap %s!\n", !info.status ? "success" : "failed");
	if (info.status)
		hh_print(HHLOG_DEBUG, "failed reason %d, length %d! rio length %d %d\n",
			info.fail_reason, info.length, rio->info.source_buffer_num, rio->info.dest_buffer_num);
	if (rio) {
		remap_aftprocess(rio->area, &rio->pair_list, !info.status);

		hyperhold_free(rio->info.source_buffer);
		hyperhold_free(rio->info.dest_buffer);
		hyperhold_free(rio);
	}
}

static int remap_get_drv_info(struct remap_io *rio)
{
	struct remap_map *map;
	rio->info.source_buffer_num = rio->cnt;
	rio->info.dest_buffer_num = rio->cnt;
	int pair_cnt = 0;

	rio->info.source_buffer =
		hyperhold_malloc(rio->cnt * sizeof(struct stor_dev_data_remap_source_buffer), false, true);
	rio->info.dest_buffer =
		hyperhold_malloc(rio->cnt * sizeof(struct stor_dev_data_remap_source_buffer), false, true);
	if (!rio->info.source_buffer || !rio->info.dest_buffer) {
		hh_print(HHLOG_DEBUG, "no mem\n");
		return -ENOMEM;
	}
	list_for_each_entry (map, &rio->pair_list, list) {
		rio->info.source_buffer[pair_cnt].data_source_addr =
			hyperhold_get_sector(extent_bit2id(rio->area, map->src_bit)) >> 3;
		rio->info.source_buffer[pair_cnt].source_length = EXTENT_PG_CNT;
		rio->info.dest_buffer[pair_cnt].data_source_addr =
			hyperhold_get_sector(extent_bit2id(rio->area, map->dst_bit)) >> 3;
		rio->info.dest_buffer[pair_cnt].source_length = EXTENT_PG_CNT;
		pair_cnt++;
		hh_print(HHLOG_DEBUG, "extenid remap(No. %d): %d => %d\n", pair_cnt, extent_bit2id(rio->area, map->src_bit),
			 extent_bit2id(rio->area, map->dst_bit));
	}
	rio->info.data_remap_done_info.private_data = rio;
	rio->info.data_remap_done_info.done = remap_back_done;

	return 0;
}

#define REMAP_RETRY_MAX 5
int remap_bitmap_with_zones(struct hyperhold_area *area, int require_seqsize_in_zone)
{
	struct remap_io *rio;
	int acture_remap_zone_cnt = 0;
	int ret;
	int retry_cnt = 0;

	rio = hyperhold_malloc(sizeof(struct remap_io), false, true);
	if (!rio)
		return 0;
	rio->bdev = hyperhold_get_global_zram()->bdev;
	rio->area = area;
	INIT_LIST_HEAD(&rio->pair_list);

	acture_remap_zone_cnt = remap_get_pairs(area, require_seqsize_in_zone, rio);
	if (!acture_remap_zone_cnt)
		goto  error_ret;

do_remap:
	ret = remap_get_drv_info(rio);
	if (ret)
		goto error_ret;
retry:
#if defined(CONFIG_MAS_BLK) && defined(CONFIG_MAS_UNISTORE_PRESERVE)
	ret = mas_blk_data_remap(rio->bdev, &rio->info);
	if (ret) {
		if ((ret == -EACCES) && (retry_cnt++ < REMAP_RETRY_MAX)) {
			udelay(500);
			goto retry;
		}
		hh_print(HHLOG_ERR, "mas_blk_data_remap return %d\n", ret);
		goto error_ret;
	}
	return acture_remap_zone_cnt;
#else
	hh_print(HHLOG_ERR, "don't support hyperhold remap on non-unistore device.\n");
#endif
error_ret:
	remap_aftprocess(rio->area, &rio->pair_list, false);
	hyperhold_free(rio);
	return 0;
}
