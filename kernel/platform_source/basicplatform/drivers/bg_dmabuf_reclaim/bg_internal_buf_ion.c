/*
 * bg_internel_buf_ion.c
 *
 * Copyright (C) 2022 Hisilicon Technologies Co., Ltd.
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
 */
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/mm.h>
#include <linux/platform_device.h>
#include <linux/vmstat.h>
#include <linux/swap.h>
#include <linux/ion/mm_ion.h>
#ifdef CONFIG_BG_DMABUF_DROP_CACHE
#include <linux/kthread.h>
#include <linux/workqueue.h>
#endif

#include "ion.h"
#include "mm_ion_priv.h"
#include "bg_internal_buf_ion.h"

static struct bg_dmabuf_info *get_info_from_ion_buffer(void *ptr)
{
	struct ion_buffer *buffer = (struct ion_buffer *)ptr;

	return buffer->info;
}

static void set_info_to_ion_buffer(void *ptr, struct bg_dmabuf_info *info)
{
	struct ion_buffer *buffer = (struct ion_buffer *)ptr;

	buffer->info = info;
}

static bool is_ion_buffer_valid(void *ptr)
{
	struct ion_buffer *buffer = (struct ion_buffer *)ptr;

	if ((buffer->heap->id != ION_SYSTEM_HEAP_ID) ||
		(buffer->flags & ION_FLAG_MM_LB_MASK) ||
		(buffer->flags & ION_FLAG_SECURE_BUFFER) ||
		!(buffer->flags & ION_FLAG_BACKGROUND_SHRINK_BUFFER))
		return false;
	else
		return true;
}

static void *get_ion_buffer_ptr_from_dmabuf(struct dma_buf *dmabuf)
{
	if (!is_ion_dma_buf(dmabuf))
		return NULL;

	return dmabuf->priv;
}

static void ion_buffer_mutex_lock(void *ptr)
{
	struct ion_buffer *buffer = (struct ion_buffer *)ptr;

	mutex_lock(&buffer->lock);
}

static void ion_buffer_mutex_unlock(void *ptr)
{
	struct ion_buffer *buffer = (struct ion_buffer *)ptr;

	mutex_unlock(&buffer->lock);
}

static size_t get_ion_buffer_size(void *ptr)
{
	struct ion_buffer *buffer = (struct ion_buffer *)ptr;

	return buffer->size;
}

#ifdef CONFIG_BG_DMABUF_DROP_CACHE
/*
 * If the ION alloc is too slow when the memory is low, enable the DROP_CACHE feature
 * The probability of this problem is uncertain. Therefore, do not enable this feature right now
 */
struct task_struct *g_bg_ion_free_tsk;
wait_queue_head_t g_bg_ion_wq;
atomic_t g_bg_ion_tsk_run_flag = ATOMIC_INIT(0);
static inline unsigned long __bg_get_uncount_file_page(void)
{
	return global_node_page_state(NR_SHMEM) +
		global_node_page_state(NR_UNEVICTABLE) +
		total_swapcache_pages();
}

static void bg_ion_prepare_memory(void)
{
	unsigned long file_page_watermark = SZ_128M / SZ_4K;
	unsigned long file_page;

	bg_buf_pr_debug("%s ++\n", __func__);

	/* drop cache when file page larger then 128M */
	file_page = global_node_page_state(NR_FILE_PAGES) - __bg_get_uncount_file_page();
	if (file_page > file_page_watermark) {
		ion_drop_pagecache();
		drop_slab();
		bg_buf_pr_info("%s after drop file cache\n", __func__);
		show_mem(0, NULL);
	}

	bg_buf_pr_debug("%s -- file_page-%ld watermark-%ld\n",
		__func__, file_page, file_page_watermark);
}

static int bg_ion_free_kthread(void *p)
{
	int ret;

	while (!kthread_should_stop()) {
		ret = wait_event_interruptible(g_bg_ion_wq, atomic_read(&g_bg_ion_tsk_run_flag));
		if (ret)
			continue;

		bg_buf_pr_info("%s free_kthread\n", __func__);
		bg_ion_prepare_memory();
		atomic_set(&g_bg_ion_tsk_run_flag, 0);
	}
	return 0;
}

void bg_ion_create_free_thread(void)
{
	g_bg_ion_free_tsk = kthread_run(bg_ion_free_kthread, NULL, "bg_ion_free");
	if (IS_ERR(g_bg_ion_free_tsk)) {
		bg_buf_pr_err("bg_ion_free_tsk create err\n");
		g_bg_ion_free_tsk = NULL;
		return;
	}

	init_waitqueue_head(&g_bg_ion_wq);
	atomic_set(&g_bg_ion_tsk_run_flag, 0);
	bg_buf_pr_debug("%s\n", __func__);
}

void wakeup_bg_ion_free_tsk(void)
{
	if (!g_bg_ion_free_tsk) {
		bg_buf_pr_debug("%s bg_free_tsk is NULL\n", __func__);
		bg_ion_create_free_thread();
	}
	atomic_set(&g_bg_ion_tsk_run_flag, 1);
	wake_up_interruptible(&g_bg_ion_wq);
}
#endif /* CONFIG_BG_DMABUF_DROP_CACHE */

static void _flush_ion_buffer(size_t len, unsigned long flags, struct ion_buffer *buffer)
{
	struct platform_device *mm_ion_dev = get_mm_ion_platform_device();

	if (!(flags & ION_FLAG_CACHED) && !(flags & ION_FLAG_SECURE_BUFFER)) {
#ifndef CONFIG_MM_LB_FULL_COHERENCY
		if (len >= MM_ION_FLUSH_ALL_CPUS_CACHES)
			ion_flush_all_cpus_caches();
		else
#endif
			dma_sync_sg_for_cpu(&mm_ion_dev->dev,
				buffer->sg_table->sgl,
				buffer->sg_table->nents, DMA_FROM_DEVICE);
	}
}

static void __ion_buffer_prep_memory(void)
{
	unsigned long long free_ram = global_zone_page_state(NR_FREE_PAGES);
	unsigned long long ion_cache = global_zone_page_state(NR_IONCACHE_PAGES);
	unsigned long watermark = SZ_128M / SZ_4K;

	if (free_ram < watermark || ion_cache == 0) {
		bg_buf_pr_info("%s free_ram is %lld kB ION_Cache %lld kB",
			__func__, free_ram, ion_cache);
#ifdef CONFIG_BG_DMABUF_DROP_CACHE
		bg_buf_pr_debug("%s tick free thread\n", __func__);
		wakeup_bg_ion_free_tsk();
#endif
	}
}

static int ion_buffer_alloc_memory(void *ptr)
{
	struct ion_buffer *buffer = (struct ion_buffer *)ptr;
	struct ion_heap *heap = buffer->heap;
	unsigned long flags = buffer->flags;
	unsigned long len = buffer->size;
	struct scatterlist *sg = NULL;
	int ret, i;

	__ion_buffer_prep_memory();

	ret = heap->ops->allocate(heap, buffer, len, flags);
	if (ret) {
		bg_buf_pr_info("%s allocate failed, wait 10ms and retry\n", __func__);
		/* wait 10ms for mem alloc retry */
		msleep(10);

		ret = heap->ops->allocate(heap, buffer, len, flags);
		if (ret) {
			bg_buf_pr_err("%s retry mem alloc failed\n", __func__);
			return -ENOMEM;
		}
	}

	if (!buffer->sg_table) {
		WARN_ONCE(1, "%s This heap needs to set the sgtable", __func__);
		return -ENOMEM;
	}

	for_each_sg(buffer->sg_table->sgl, sg, buffer->sg_table->nents, i) {
		sg_dma_address(sg) = sg_phys(sg);
		sg_dma_len(sg) = sg->length;
#ifdef CONFIG_H_MM_PAGE_TRACE
		SetPageION(sg_page(sg));
#endif
		bg_buf_pr_debug("%s sg-addr [0x%llx] size[0x%llx]\n",
			__func__, sg_phys(sg), sg->length);
	}

	_flush_ion_buffer(len, flags, buffer);

	add_ion_total_size(buffer->size);

	bg_buf_pr_debug("%s buffer-size[0x%llx] total-0x%llx\n",
		__func__, buffer->size, get_ion_total_size());
	return 0;
}

static void ion_buffer_free_memory(void *ptr)
{
	struct ion_buffer *buffer = (struct ion_buffer *)ptr;

	sub_ion_total_size(buffer->size);

	/* only free buffer->sg_table & sg_table pages */
	buffer->heap->ops->free(buffer);

	bg_buf_pr_debug("%s ion-buffer-free[0x%llx], total[0x%llx]\n",
		__func__, buffer->size, get_ion_total_size());
}

static struct sg_table *get_ion_buffer_sg_table(void *ptr)
{
	struct ion_buffer *buffer = (struct ion_buffer *)ptr;

	return buffer->sg_table;
}

static pgprot_t get_ion_buffer_kmap_prot(void *ptr)
{
	struct ion_buffer *buffer = (struct ion_buffer *)ptr;
	pgprot_t pgprot;

	if (buffer->flags & ION_FLAG_CACHED)
		pgprot = PAGE_KERNEL;
	else
		pgprot = pgprot_writecombine(PAGE_KERNEL);
	return pgprot;
}

static struct bg_buffer_ops ion_buffer_ops = {
	.get_info = get_info_from_ion_buffer,
	.set_info = set_info_to_ion_buffer,
	.is_buffer_valid = is_ion_buffer_valid,
	.get_buffer_ptr_from_dmabuf = get_ion_buffer_ptr_from_dmabuf,
	.buffer_lock = ion_buffer_mutex_lock,
	.buffer_unlock = ion_buffer_mutex_unlock,
	.get_size = get_ion_buffer_size,
	.alloc_memory = ion_buffer_alloc_memory,
	.free_memory = ion_buffer_free_memory,
	.get_sg_table = get_ion_buffer_sg_table,
	.get_kmap_prot = get_ion_buffer_kmap_prot,
};

void internal_ion_buffer_init(struct bg_buffer_ops **ops)
{
	*ops = &ion_buffer_ops;
}

