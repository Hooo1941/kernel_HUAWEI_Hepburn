/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023. All rights reserved.
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
 */
#define pr_fmt(fmt) "[HYPERHOLD]" fmt

#include <linux/kernel.h>
#include <linux/hp/hyperhold_inf.h>

#include "zram_drv.h"
#include "hyperhold.h"
#include "hyperhold_internal.h"
#include "hyperhold_area.h"

static int hyperhold_turbo_para_check(struct hyperhold_turbo_para *para,
	struct zram *zram, enum hyperhold_scenario scenario)
{
	int index;

	if (!para || !para->done || !zram || !zram->area) {
		hh_print(HHLOG_ERR, "para or zram is null!\n");
		return -EINVAL;
	}

	if (!para->page_num) {
		hh_print(HHLOG_ERR, "page_num is zero!\n");
		return -EINVAL;
	}

	if ((scenario == HYPERHOLD_FAULT_OUT) &&
		(para->page_num != NO_PKG_EXTENT_NUM)) {
		hh_print(HHLOG_ERR, "page_num is err!\n");
		return -EINVAL;
	}

	if (!para->exts) {
		hh_print(HHLOG_ERR, "exts is null\n");
		return -EINVAL;
	}

	for (index = 0; index < para->page_num; ++index) {
		if ((para->exts[index].ext_id < 0) ||
			(para->exts[index].ext_id >= zram->area->nr_exts)) {
			hh_print(HHLOG_ERR, "ext_id[%d] is err\n", index);
			return -EINVAL;
		}

		if (!para->exts[index].page) {
			hh_print(HHLOG_ERR, "pages[%d] is null\n", index);
			return -EINVAL;
		}
	}

	return 0;
}

static void hyperhold_turbo_flush_done(struct hyperhold_entry *io_entry,
	int err)
{
	struct hyperhold_turbo_ext ext;
	hyperhold_io_done_fn *done = (hyperhold_io_done_fn *)io_entry->manager_private;

	ext.ext_id = io_entry->ext_id;
	ext.page = io_entry->dest_pages[0];
	ext.private = io_entry->private;

	if (done)
		done(err, &ext);

	hyperhold_free(io_entry->dest_pages);
	hyperhold_free(io_entry);
}

static void hyperhold_turbo_free_pagepool(struct schedule_para *sched)
{
	struct page *free_page = NULL;

	spin_lock(&sched->priv.page_pool.page_pool_lock);
	while (!list_empty(&sched->priv.page_pool.page_pool_list)) {
		free_page = list_first_entry(
			&sched->priv.page_pool.page_pool_list,
				struct page, lru);
		list_del_init(&free_page->lru);
		__free_page(free_page);
	}
	spin_unlock(&sched->priv.page_pool.page_pool_lock);
}

static void hyperhold_turbo_plug_complete(void *data)
{
	struct schedule_para *sched  = (struct schedule_para *)data;

	hyperhold_turbo_free_pagepool(sched);

	hyperhold_perf_end(&sched->record);

	hyperhold_free(sched);
}

static void *hyperhold_turbo_init_plug(struct zram *zram,
	enum hyperhold_scenario scenario, struct schedule_para *sched,
	bool wait_io_finish_flag)
{
	struct hyperhold_io io_para;

	io_para.bdev = zram->bdev;
	io_para.scenario = scenario;
	io_para.private = (void *)sched;
	io_para.record = &sched->record;
	INIT_LIST_HEAD(&sched->priv.page_pool.page_pool_list);
	spin_lock_init(&sched->priv.page_pool.page_pool_lock);
	io_para.complete_notify = hyperhold_turbo_plug_complete;
	io_para.done_callback = hyperhold_turbo_flush_done;
	sched->io_buf.pool = NULL;
	sched->io_buf.zram = zram;
	sched->priv.zram = zram;
	sched->priv.scenario = io_para.scenario;
	return hyperhold_req_init(&io_para, wait_io_finish_flag);
}

static int hyperhold_turbo_init(struct schedule_para **out_sched,
	struct zram *zram, enum hyperhold_scenario scenario,
	struct hyperhold_turbo_para *para)
{
	struct schedule_para *sched = NULL;
	ktime_t start = ktime_get();
	unsigned long long start_ravg_sum = hyperhold_get_ravg_sum();

	sched = hyperhold_malloc(sizeof(struct schedule_para), true,
		(scenario == HYPERHOLD_FAULT_OUT));
	if (unlikely(!sched)) {
		hh_print(HHLOG_ERR, "alloc sched failed!\n");
		hyperhold_stat_alloc_fail(scenario, -ENOMEM);

		return -ENOMEM;
	}

	hyperhold_perf_start(&sched->record, start, start_ravg_sum, scenario);

	hyperhold_perf_lat_start(&sched->record, HYPERHOLD_INIT);
	sched->io_handler = hyperhold_turbo_init_plug(zram, scenario, sched,
		para->sync_flag);
	hyperhold_perf_lat_end(&sched->record, HYPERHOLD_INIT);
	if (unlikely(!sched->io_handler)) {
		hh_print(HHLOG_ERR, "plug start failed!\n");
		hyperhold_perf_end(&sched->record);
		hyperhold_free(sched);
		hyperhold_stat_alloc_fail(scenario, -ENOMEM);

		return -ENOMEM;
	}

	*out_sched = sched;

	return 0;
}

static void hyperhold_fill_turbo_entry(struct hyperhold_entry *io_entry,
	struct hyperhold_turbo_ext *ext, hyperhold_io_done_fn *done)
{
	io_entry->ext_id = ext->ext_id;
	io_entry->addr = hyperhold_get_sector(ext->ext_id);
	io_entry->dest_pages[0] = ext->page;
	io_entry->pages_sz = NO_PKG_EXTENT_NUM;
	io_entry->private = ext->private;
	io_entry->manager_private = (void *)done;
}

static int hyperhold_turbo_io_extent(struct schedule_para *sched,
	enum hyperhold_scenario scenario, struct hyperhold_turbo_ext *ext,
	hyperhold_io_done_fn *done)
{
	int ret;

	hyperhold_perf_lat_start(&sched->record, HYPERHOLD_IOENTRY_ALLOC);
	sched->io_entry = hyperhold_malloc(sizeof(struct hyperhold_entry),
		true, (scenario == HYPERHOLD_FAULT_OUT));
	if (unlikely(!sched->io_entry)) {
		hyperhold_perf_lat_end(&sched->record, HYPERHOLD_IOENTRY_ALLOC);
		hh_print(HHLOG_ERR, "alloc io entry failed!\n");
		hyperhold_stat_alloc_fail(scenario, -ENOMEM);

		return -ENOMEM;
	}

	sched->io_entry->dest_pages = hyperhold_malloc(sizeof(struct page **),
		true, (scenario == HYPERHOLD_FAULT_OUT));
	hyperhold_perf_lat_end(&sched->record, HYPERHOLD_IOENTRY_ALLOC);
	if (unlikely(!sched->io_entry->dest_pages)) {
		hh_print(HHLOG_ERR, "alloc io dest_pages failed!\n");
		hyperhold_free(sched->io_entry);
		sched->io_entry = NULL;
		hyperhold_stat_alloc_fail(scenario, -ENOMEM);

		return -ENOMEM;
	}

	hyperhold_perf_lat_start(&sched->record, HYPERHOLD_FIND_EXTENT);
	hyperhold_fill_turbo_entry(sched->io_entry, ext, done);
	hyperhold_perf_lat_end(&sched->record, HYPERHOLD_FIND_EXTENT);

	hyperhold_perf_lat_start(&sched->record, HYPERHOLD_IO_EXTENT);
	ret = hyperhold_io_extent(sched->io_handler, sched->io_entry);
	hyperhold_perf_lat_end(&sched->record, HYPERHOLD_IO_EXTENT);
	if (unlikely(ret)) {
		hh_print(HHLOG_ERR,
			"hyperhold io failed! %d\n", ret);
		hyperhold_stat_alloc_fail(scenario, ret);

		return -EIO;
	}

	return 0;
}

static void hyperhold_turbo_io_end_proc(struct hyperhold_turbo_para *para,
	int num)
{
	int index;

	if (!para || !para->done) {
		hh_print(HHLOG_ERR, "para or done is null!\n");
		return;
	}

	for (index = num; index < para->page_num; ++index)
		para->done(-EIO, &para->exts[index]);
}

static int hyperhold_turbo_io(struct hyperhold_turbo_para *para,
	enum hyperhold_scenario scenario)
{
	int ret;
	int num = 0;
	struct schedule_para *sched = NULL;
	struct zram *zram = hyperhold_get_global_zram();

	if (hyperhold_turbo_para_check(para, zram, scenario))
		return num;

	ret = hyperhold_turbo_init(&sched, zram, scenario, para);
	if (unlikely(ret))
		return num;

	for (num = 0; num < para->page_num; ++num) {
		ret = hyperhold_turbo_io_extent(sched, scenario,
			&para->exts[num], para->done);
		if (unlikely(ret)) {
			if (ret == -EIO)
				num++;
			break;
		}
	}

#ifdef CONFIG_RAMTURBO
	ret = hyperhold_plug_finish(sched->io_handler,
		RECLAIM_SOURCE_DEFAULT);
#else
	ret = hyperhold_plug_finish(sched->io_handler);
#endif
	if (unlikely(ret)) {
		hh_print(HHLOG_ERR,
			"hyperhold scenario %d failed! %d\n", scenario, ret);
		hyperhold_stat_alloc_fail(scenario, ret);
	}

	return num;
}

int hyperhold_turbo_reclaim_in(struct hyperhold_turbo_para *para)
{
	int num = 0;

	if (!hyperhold_enable() || !hyperhold_reclaim_in_enable())
		goto out;

	num = hyperhold_turbo_io(para, HYPERHOLD_RECLAIM_IN);
out:
	hyperhold_turbo_io_end_proc(para, num);

	return num;
}

int hyperhold_turbo_batch_out(struct hyperhold_turbo_para *para)
{
	int num = 0;

	if (!hyperhold_enable())
		goto out;

	num = hyperhold_turbo_io(para, HYPERHOLD_BATCH_OUT);
out:
	hyperhold_turbo_io_end_proc(para, num);

	return num;
}

int hyperhold_turbo_fault_out(struct hyperhold_turbo_para *para)
{
	int num = 0;

	if (!hyperhold_enable())
		goto out;

	num = hyperhold_turbo_io(para, HYPERHOLD_FAULT_OUT);
out:
	hyperhold_turbo_io_end_proc(para, num);

	return num;
}
