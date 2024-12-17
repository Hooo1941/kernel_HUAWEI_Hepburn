/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020. All rights reserved.
 * Description: hyperhold remap debug implement.
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

#include <securec.h>
#include <linux/crc32.h>
#include <linux/rcupdate.h>
#include "hyperhold_remap_debug.h"
int g_hh_remap_step = 0;

static struct page *remap_debug_alloc_page(bool fast, bool nofail)
{
	struct page *page = NULL;
	if (fast) {
		page = alloc_page(GFP_ATOMIC);
		if (likely(page))
			goto out;
	}
	if (nofail)
		page = alloc_page(GFP_NOIO);
out:
	return page;
}

static void remap_debug_data_insert(struct remap_debug_io *io)
{
	int page_idx;
	u32 magic_num;
	int i;
	u32 *addr;
	int magic_start = REMAP_DEBUG_MAGIC_START / sizeof(u32);
	int magic_end = PAGE_SIZE / sizeof(u32);

	for (page_idx = 0; page_idx < EXTENT_PG_CNT; page_idx++) {
		magic_num =
			(io->ext_id << REMAP_DEBUG_MAGIC_SHIFT) | (page_idx);
		addr = (u32 *)page_to_virt(io->page[page_idx]);
		for (i = magic_start; i < magic_end; i++)
			*(addr + i) = magic_num;
		*addr = crc32(0, addr + magic_start, REMAP_DEBUG_MAGIC_SIZE);
	}
}

static void remap_debug_data_check(struct remap_debug_io *io)
{
	int page_idx;
	u32 magic_num;
	u32 *addr;
	u32 crcv;
	int magic_start = REMAP_DEBUG_MAGIC_START / sizeof(u32);

	for (page_idx = 0; page_idx < EXTENT_PG_CNT; page_idx++) {
		magic_num =
			(io->ext_id << REMAP_DEBUG_MAGIC_SHIFT) | (page_idx);
		addr = (u32 *)page_to_virt(io->page[page_idx]);
		crcv = crc32(0, addr + magic_start, REMAP_DEBUG_MAGIC_SIZE);
		if (*addr != crcv) {
			hh_print(
				HHLOG_DEBUG,
				"read ext_id %d crc error in page %d, %08x != %08x\n",
				io->ext_id, page_idx, *addr, crcv);
			return;
		} else {
			hh_print(HHLOG_DEBUG,
				 "crc %08x(%08x) magic %08x(%08x)\n", *addr,
				 crcv, *(addr + magic_start), magic_num);
		}
	}
	hh_print(HHLOG_DEBUG, "read ext_id %d check ok!\n", io->ext_id);
}

static struct remap_debug_io *hh_remap_stub_io_prepare(int ext_id, bool iswrite)
{
	struct remap_debug_io *p =
		hyperhold_malloc(sizeof(struct remap_debug_io), !iswrite, true);
	int i;

	if (!p) {
		hh_print(HHLOG_DEBUG, "%s, no mem\n", __func__);
		return NULL;
	}

	p->ext_id = ext_id;
	p->page_cnt = EXTENT_PG_CNT;
	for (i = 0; i < p->page_cnt; i++) {
		p->page[i] = remap_debug_alloc_page(!iswrite, true);
		if (!p->page[i])
			goto page_free;
	}
	return p;
page_free:
	for (i = 0; i < p->page_cnt; i++)
		__free_page(p->page[i]);
	hyperhold_free(p);
	return NULL;
}

static void hh_remap_stub_io_destory(struct remap_debug_io *dio)
{
	int i;
	if (dio) {
		for (i = 0; i < dio->page_cnt; i++)
			__free_page(dio->page[i]);
		hyperhold_free(dio);
	}
}

static void hh_remap_stub_write_endio(struct bio *bio)
{
	struct remap_debug_io *dio = bio->bi_private;
	hh_print(HHLOG_DEBUG, "ext_id %d write ok!\n", dio->ext_id);
	hh_remap_stub_io_destory(bio->bi_private);
	bio_put(bio);
}

void hh_remap_stub_write(int ext_id)
{
	int i;
	struct bio *bio = bio_alloc(GFP_ATOMIC, BIO_MAX_PAGES);
	if (!bio) {
		hh_print(HHLOG_DEBUG, "bio alloc failed!\n");
		return;
	}

	struct remap_debug_io *dio = hh_remap_stub_io_prepare(ext_id, true);
	if (!dio) {
		hh_print(HHLOG_DEBUG, "dio alloc failed!\n");
		goto error_out;
	}

	remap_debug_data_insert(dio);

	bio->bi_iter.bi_sector = hyperhold_get_sector(ext_id);
	struct block_device *bdev = hyperhold_get_global_zram()->bdev;
	if (!bdev) {
		hh_print(HHLOG_DEBUG, "no bdev\n");
		goto error_out;
	}

	bio_set_dev(bio, bdev);
	bio->bi_private = dio;
	bio_set_op_attrs(bio, REQ_OP_WRITE, 0);
	bio->bi_opf |= REQ_SYNC | REQ_FG;
	bio->mas_bi_opf |= REQ_VIP;
	bio->bi_end_io = hh_remap_stub_write_endio;
	for (i = 0; i < dio->page_cnt; i++)
		bio_add_page(bio, dio->page[i], PAGE_SIZE, 0);
	submit_bio(bio);
	return;
error_out:
	hh_remap_stub_io_destory(dio);
	bio_put(bio);
	return;
}

static void hh_remap_stub_read_endio(struct bio *bio)
{
	struct remap_debug_io *dio = bio->bi_private;
	hh_print(HHLOG_DEBUG, "ext_id %d read ok!\n", dio->ext_id);
	remap_debug_data_check(dio);
	hh_remap_stub_io_destory(bio->bi_private);
	bio_put(bio);
}

void hh_remap_stub_read(int ext_id)
{
	int i;
	struct bio *bio = bio_alloc(GFP_ATOMIC, BIO_MAX_PAGES);
	if (!bio) {
		hh_print(HHLOG_DEBUG, "bio alloc failed!\n");
		return;
	}

	struct remap_debug_io *dio = hh_remap_stub_io_prepare(ext_id, false);
	if (!dio) {
		hh_print(HHLOG_DEBUG, "dio alloc failed!\n");
		goto error_out;
	}

	bio->bi_iter.bi_sector = hyperhold_get_sector(ext_id);
	struct block_device *bdev = hyperhold_get_global_zram()->bdev;
	if (!bdev) {
		hh_print(HHLOG_DEBUG, "no bdev\n");
		goto error_out;
	}

	bio_set_dev(bio, bdev);
	bio->bi_private = dio;
	bio_set_op_attrs(bio, REQ_OP_READ, 0);
	bio->bi_opf |= REQ_SYNC | REQ_FG;
	bio->mas_bi_opf |= REQ_VIP;
	bio->bi_end_io = hh_remap_stub_read_endio;
	for (i = 0; i < dio->page_cnt; i++)
		bio_add_page(bio, dio->page[i], PAGE_SIZE, 0);
	submit_bio(bio);
	return;
error_out:
	hh_remap_stub_io_destory(dio);
	bio_put(bio);
	return;
}

void hh_remap_stub_discrete(void)
{
	struct hyperhold_area *area = hyperhold_get_global_zram()->area;
	if (!area) {
		hh_print(HHLOG_DEBUG, "hyperhold not init yet!\n");
		return;
	}

	g_hh_remap_step = 2;
	int nr = area->nr_exts / 2;
	int bit;
	int ext_id;
	do {
		bit = alloc_bitmap_with_zones(area, hyperhold_discard_enable());
		if (bit < 0) {
			hh_print(HHLOG_DEBUG,
				 "alloc bitmap failed, bit = %d, nr = %d\n",
				 bit, nr);
			g_hh_remap_step = 0;
			return;
		}
		ext_id = extent_bit2id(area, bit);
		hh_remap_stub_write(ext_id);
	} while (--nr);
	g_hh_remap_step = 0;
}

#define REMAP_PRINT_BUF_SIZE 70
#define REMAP_PRINT_BUF_LIMIT 64
#define REMAP_PRINT_PER_LINE 8
void hh_remap_stub_check_bitmap(int mode)
{
	int i;
	int zone_idx;
	struct hh_zone *cur;
	struct hyperhold_area *area = hyperhold_get_global_zram()->area;
	unsigned long flags;

	switch (mode) {
	case 0:
		for (i = 0; i < area->nr_exts;
		     i += BITS_PER_TYPE(unsigned long)) {
			zone_idx = i / BITS_PER_TYPE(unsigned long);
			hh_print(HHLOG_DEBUG, "%08d: %016lx\n", zone_idx,
				 area->zones[zone_idx].bitmap);
		}
		break;
	case 1:
		hh_print(HHLOG_DEBUG, "free zones:\n");
		i = 0;
		char buf[REMAP_PRINT_BUF_SIZE];
		int len = 0;
		spin_lock_irqsave(&area->free_op_lock, flags);
		list_for_each_entry (cur, &area->free_zones, list) {
			len += snprintf_s(buf + len, REMAP_PRINT_BUF_LIMIT - len,
					REMAP_PRINT_BUF_LIMIT - len - 1, "%4d",
					cur->zone_off);
			if (len > REMAP_PRINT_BUF_LIMIT ||
			    ++i >= REMAP_PRINT_PER_LINE) {
				buf[len] = 0;
				hh_print(HHLOG_DEBUG, "%s\n", buf);
				len = 0;
				i = 0;
			}
		}
		spin_unlock_irqrestore(&area->free_op_lock, flags);
		if (i || len) {
			buf[len] = 0;
			hh_print(HHLOG_DEBUG, "%s\n", buf);
			len = 0;
			i = 0;
		}
		hh_print(HHLOG_DEBUG, "used zones:\n");
		spin_lock_irqsave(&area->used_op_lock, flags);
		list_for_each_entry (cur, &area->used_zones, list) {
			len += snprintf_s(buf + len, REMAP_PRINT_BUF_LIMIT - len,
					REMAP_PRINT_BUF_LIMIT - len - 1, "%4d",
					cur->zone_off);
			if (len > REMAP_PRINT_BUF_LIMIT ||
			    ++i >= REMAP_PRINT_PER_LINE) {
				buf[len] = 0;
				hh_print(HHLOG_DEBUG, "%s\n", buf);
				len = 0;
				i = 0;
			}
		}
		if (i || len) {
			buf[len] = 0;
			hh_print(HHLOG_DEBUG, "%s\n", buf);
			len = 0;
			i = 0;
		}
		hh_print(HHLOG_DEBUG, "remap zones:\n");
		list_for_each_entry (cur, &area->remap_zones, list) {
			len += snprintf_s(buf + len, REMAP_PRINT_BUF_LIMIT - len,
					REMAP_PRINT_BUF_LIMIT - len - 1, "%4d",
					cur->zone_off);
			if (len > REMAP_PRINT_BUF_LIMIT ||
			    ++i >= REMAP_PRINT_PER_LINE) {
				buf[len] = 0;
				hh_print(HHLOG_DEBUG, "%s\n", buf);
				len = 0;
				i = 0;
			}
		}
		spin_unlock_irqrestore(&area->used_op_lock, flags);
		if (i || len) {
			buf[len] = 0;
			hh_print(HHLOG_DEBUG, "%s\n", buf);
			len = 0;
			i = 0;
		}
		break;
	default:
		hh_print(HHLOG_DEBUG, "unkown command.\n");
		break;
	}
}

void hh_remap_stub_remap(u32 cnt)
{
	int ret;
	struct hyperhold_area *area = hyperhold_get_global_zram()->area;

	ret = remap_bitmap_with_zones(area, cnt);
	hh_print(HHLOG_DEBUG, "require %d acture %d\n", cnt, ret);
}

