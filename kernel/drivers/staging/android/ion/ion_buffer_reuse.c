/*
 * drivers/staging/android/ion/ion_buffer_reuse.c
 *
 * Copyright (C) 2011 Google, Inc.
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
#include <linux/err.h>
#include <linux/list.h>
#include <linux/mm.h>
#include <linux/mm_types.h>
#include <linux/slab.h>
#include <linux/seq_file.h>
#include <linux/tracepoint.h>

#include <linux/bg_dmabuf_reclaim/bg_dmabuf_ops.h>

#include <securec.h>

#include "ion.h"
#include "ion_buffer_reuse.h"

#define HEAP_REUSE_CONFIG_MAX_LEN 100
#define HEAP_REUSE_CONFIG_MAX_SIZE (20 * HEAP_REUSE_CONFIG_MAX_LEN)

int sysctl_ion_heap_trace_buf_reuse = 0;
int sysctl_ion_heap_max_zero_orders_in_reuse = 5000;
int sysctl_ion_heap_buffer_min_order_in_reuse = 8;

#define ion_heap_trace_buf_reuse(fmt, args...)				\
do {									\
	if (sysctl_ion_heap_trace_buf_reuse)				\
		trace_printk(fmt, ##args);				\
} while (0)

extern int sysctl_ion_heap_trace_heap_drain;
#define ion_heap_trace_heap_drain(fmt, args...)				\
do {									\
	if (sysctl_ion_heap_trace_heap_drain)				\
		trace_printk(fmt, ##args);				\
} while (0)

static int ion_heap_get_reuse_order(unsigned long size)
{
	unsigned int order = get_order(size);
	if (order >= ION_REUSE_LIST_MAX_ORDER)
		order = ION_REUSE_LIST_MAX_ORDER - 1;

	return order;
}

size_t ion_heap_reuselist_size(struct ion_heap *heap)
{
	size_t size = 0;

	if (!(heap->flags & ION_HEAP_FLAG_FREE_BUF_REUSE))
		return size;

	spin_lock(&heap->reuse_lock);
	size = heap->reuse_list_size;
	spin_unlock(&heap->reuse_lock);

	return size;
}

static bool ion_heap_can_reuse_buffer(struct ion_heap *heap, struct ion_buffer *buffer)
{
#ifdef CONFIG_ION_WIDE_INFO
	struct ion_buffer_stats stats;
#endif
	bool reuse = false;

	if (!(heap->flags & ION_HEAP_FLAG_FREE_BUF_REUSE))
		return false;

	/* Just return if reuse for buffer is not allowed */
	if (!buffer->reuse)
		return false;

	/*
	 * If we reuse buffers that can be BG
	 * we need to clear buffer->reuse
	 * flag. Otherwise this buffer can
	 * be added to reuse list during reclaim
	 * and we can have SG list reclaimed
	 * during reuse/buffer zero in future.
	 */
	if (is_bg_buffer(buffer))
		return false;

	/* Don't add any CMA buffer to reuse list */
	if (buffer->nr_cma_pages)
		return false;

	if (ion_heap_get_reuse_order(buffer->size) <
			sysctl_ion_heap_buffer_min_order_in_reuse)
		return false;

	read_lock(&heap->reuse_config_lock);
	if (heap->reuse_list_size_watermark == 0)
		reuse = false;
	else
		reuse = buffer->reuse;
	read_unlock(&heap->reuse_config_lock);
#ifdef CONFIG_ION_WIDE_INFO
	if (reuse && heap->get_buffer_stats &&
			heap->get_buffer_stats(buffer, &stats) == 0) {
		if (sysctl_ion_heap_max_zero_orders_in_reuse &&
				stats.nr_pages[0] >
				sysctl_ion_heap_max_zero_orders_in_reuse)
			reuse = false;
	}
#endif

	return reuse;
}

static void ion_heap_add_buffer_stats(struct ion_buffer *buffer)
{
	struct sg_table *table = buffer->sg_table;
	struct scatterlist *sg;
	int i;

	for_each_sg(table->sgl, sg, table->nents, i) {
		struct page *page = sg_page(sg);
		zone_page_state_add(1 << compound_order(page), page_zone(page),
				    NR_IONCACHE_PAGES);
		node_page_state_add(1 << compound_order(page), page_pgdat(page),
				    NR_NODE_IONCACHES);
	}
}

static void ion_heap_rm_buffer_stats(struct ion_buffer *buffer)
{
	struct sg_table *table = buffer->sg_table;
	struct scatterlist *sg;
	int i;

	for_each_sg(table->sgl, sg, table->nents, i) {
		struct page *page = sg_page(sg);
		zone_page_state_add(-(1 << compound_order(page)), page_zone(page),
				    NR_IONCACHE_PAGES);
		node_page_state_add(-(1 << compound_order(page)), page_pgdat(page),
				    NR_NODE_IONCACHES);
	}
}

bool ion_heap_try_reuselist_add(struct ion_heap *heap, struct ion_buffer *buffer)
{
	unsigned int order;
	struct ion_buffer *tmp;
	struct ion_buffer *buf;
	struct list_head pushed_out;
	unsigned long sz_pushed_out = 0;

	if (!ion_heap_can_reuse_buffer(heap, buffer))
		return false;

	ion_buffer_before_reuse_list(buffer);
	order = ion_heap_get_reuse_order(buffer->size);

	INIT_LIST_HEAD(&pushed_out);

	spin_lock(&heap->reuse_lock);
	list_add(&buffer->list, &heap->reuse_list[order]);
	list_add_tail(&buffer->lru, &heap->lru_list);
	heap->reuse_list_size += buffer->size;
	/* Update memory stats */
	ion_heap_add_buffer_stats(buffer);

	if (heap->reuse_list_size > heap->reuse_list_size_watermark) {
		list_for_each_entry_safe(buf, tmp, &heap->lru_list, lru) {
			list_del(&buf->lru);
			list_del(&buf->list);
			list_add(&buf->list, &pushed_out);

			heap->reuse_list_size -= buf->size;
			sz_pushed_out += buf->size;
			if (heap->reuse_list_size <= heap->reuse_list_size_watermark)
				break;
		}
	}
	spin_unlock(&heap->reuse_lock);

	if (!sz_pushed_out)
		return true;

	list_for_each_entry_safe(buf, tmp, &pushed_out, list) {
		list_del(&buf->list);
		/* Update memory stats */
		ion_heap_rm_buffer_stats(buf);
		ion_buffer_destroy(buf);
	}

	return true;
}

size_t ion_heap_drain_from_reuselist(struct ion_heap *heap, size_t size,
				     bool skip_pools)
{
	size_t sz_drained = 0;
	struct list_head drained;
	struct ion_buffer *buffer = NULL;
	struct ion_buffer *tmp_buffer = NULL;

	if (!(heap->flags & ION_HEAP_FLAG_FREE_BUF_REUSE))
		return 0;

	if (ion_heap_reuselist_size(heap) == 0)
		return 0;

	ion_heap_trace_heap_drain("DRAIN REUSE:Type=%d, Expected=%zu, Available=%zu\n",
				  heap->type, size, heap->reuse_list_size);
	INIT_LIST_HEAD(&drained);
	spin_lock(&heap->reuse_lock);
	while (sz_drained < size) {
		if (list_empty(&heap->lru_list))
			break;

		buffer = list_first_entry(&heap->lru_list, struct ion_buffer, lru);
		list_del(&buffer->list);
		list_del(&buffer->lru);
		list_add(&buffer->list, &drained);
		sz_drained += buffer->size;

		if (skip_pools)
			buffer->private_flags |= ION_PRIV_FLAG_SHRINKER_FREE;
	}
	heap->reuse_list_size -= sz_drained;
	spin_unlock(&heap->reuse_lock);

	list_for_each_entry_safe(buffer, tmp_buffer, &drained, list) {
		list_del(&buffer->list);
		/* Update memory stats */
		ion_heap_rm_buffer_stats(buffer);
		ion_buffer_destroy(buffer);
	}

	ion_heap_trace_heap_drain("DRAIN REUSE:Type=%d, Reclaimed=%zu, Available=%zu\n",
				  heap->type, sz_drained, heap->reuse_list_size);
	return sz_drained;
}

struct ion_buffer *ion_heap_alloc_from_reuselist(struct ion_heap *heap, size_t size)
{
	unsigned int order;
	struct ion_buffer *tmp = NULL;
	struct ion_buffer *buffer = NULL;

	if (!(heap->flags & ION_HEAP_FLAG_FREE_BUF_REUSE))
		return NULL;

	ion_heap_trace_buf_reuse("REUSE:Type=%d,Size=%zu, heap->reuse_list_size=%zu\n",
				 heap->type, size, heap->reuse_list_size);
	order = ion_heap_get_reuse_order(size);
	spin_lock(&heap->reuse_lock);
	list_for_each_entry_safe(buffer, tmp, &heap->reuse_list[order], list) {
		if (PAGE_ALIGN(buffer->size) != PAGE_ALIGN(size))
			continue;

		list_del(&buffer->list);
		list_del(&buffer->lru);
		heap->reuse_list_size -= buffer->size;
		/* Update memory stats */
		ion_heap_rm_buffer_stats(buffer);

		goto unlock;
	}

	buffer = NULL;
unlock:
	spin_unlock(&heap->reuse_lock);

	if (buffer != NULL)
		ion_heap_trace_buf_reuse("REUSE:Type=%d, Found size=%zu\n", heap->type, size);
	else
		ion_heap_trace_buf_reuse("REUSE:Type=%d, Not found size=%zu\n", heap->type, size);

	return buffer;
}

size_t ion_heap_reuselist_drain(struct ion_heap *heap, size_t size)
{
	if (size == 0)
		size = ULONG_MAX;

	return ion_heap_drain_from_reuselist(heap, size, true);
}

static ssize_t ion_heap_reuse_config_write(struct file *file, const char __user *buf,
					   size_t len, loff_t *offset)
{
	struct ion_heap *heap = file_inode(file)->i_private;
	char *kbuf = NULL;
	char *token = NULL;
	char *kbuf_internal = NULL;
	int idx, bytes_read, ret = 0;

	if (len > HEAP_REUSE_CONFIG_MAX_SIZE)
		return -EINVAL;

	kbuf = kmalloc(HEAP_REUSE_CONFIG_MAX_SIZE + 1, GFP_KERNEL);
	if (!kbuf)
		return -ENOMEM;

	if (copy_from_user(kbuf, buf, len)) {
		ret = -EFAULT;
		goto free_kbuf;
	}

	idx = 0;
	kbuf[HEAP_REUSE_CONFIG_MAX_SIZE] = '\0';

	ion_heap_reuselist_drain(heap, 0);

	write_lock(&heap->reuse_config_lock);
	if (sscanf_s(kbuf, "%lu %n", &heap->reuse_list_size_watermark,
				&bytes_read) != 1) {
		ret = -EINVAL;
		write_unlock(&heap->reuse_config_lock);
		goto free_kbuf;
	}
	kbuf += bytes_read;

	kbuf_internal = kbuf;
	while ((token = strsep(&kbuf_internal, "\n")) != NULL &&
			idx < HEAP_REUSE_CONFIG_MAX_LEN) {
		if (token[0] == '\0')
			continue;
		strncpy(heap->reuse_config[idx].task_comm, token, TASK_COMM_LEN);
		idx++;
	}

	heap->reuse_config_len = idx;
#ifdef CONFIG_ION_WIDE_INFO
	spin_lock(&heap->stats_lock);
	heap->sz_allocated = 0;
	heap->sz_reused = 0;
	spin_unlock(&heap->stats_lock);
#endif
	write_unlock(&heap->reuse_config_lock);

free_kbuf:
	kfree(kbuf);
	return ret ? : len;
}

static int ion_heap_reuse_config_show(struct seq_file *m, void *data)
{
	int i;
	struct ion_heap *heap = m->private;

	seq_printf(m, "Buffer reuse configuration:\n");
	read_lock(&heap->reuse_config_lock);
	seq_printf(m, "Reuse watermark: %lu\n", heap->reuse_list_size_watermark);
	seq_printf(m, "Group leaders list:\n");
	for (i = 0; i < heap->reuse_config_len; i++)
		seq_printf(m, "%s\n", heap->reuse_config[i].task_comm);
	read_unlock(&heap->reuse_config_lock);

	return 0;
}

static int ion_heap_reuse_config_open(struct inode *inode, struct file *file)
{
	struct ion_heap *heap = inode->i_private;

	return single_open(file, ion_heap_reuse_config_show, heap);
}

static const struct file_operations ion_heap_reuse_config_debugfs_ops = {
	.open = ion_heap_reuse_config_open,
	.read = seq_read,
	.write = ion_heap_reuse_config_write,
	.llseek = seq_lseek,
	.release = single_release,
};

#define ION_HEAP_REUSE_WATERMARK 419430400
static char *ion_heap_reuse_service_list[] = {
	"CameraDaemon",
	"allocator@2.0-s",
	NULL
};

static void ion_heap_reuse_default_config(struct ion_heap *heap)
{
	int i = 0;

	heap->reuse_config_len = 0;
	heap->reuse_list_size_watermark = ION_HEAP_REUSE_WATERMARK;

	while (ion_heap_reuse_service_list[i]) {
		strcpy_s(heap->reuse_config[i].task_comm, TASK_COMM_LEN,
			 ion_heap_reuse_service_list[i]);
		i++;
	}
	heap->reuse_config_len = i;
}

int ion_heap_init_buf_reuse(struct ion_heap *heap)
{
	int order;

	heap->reuse_config = kmalloc(HEAP_REUSE_CONFIG_MAX_LEN *
			sizeof(struct reuse_config), GFP_KERNEL);
	if (!heap->reuse_config)
		return -ENOMEM;

	for (order = 0; order < ION_REUSE_LIST_MAX_ORDER; order++)
		INIT_LIST_HEAD(&heap->reuse_list[order]);
#ifdef CONFIG_ION_WIDE_INFO
	heap->sz_allocated = 0;
	heap->sz_reused = 0;
#endif
	INIT_LIST_HEAD(&heap->lru_list);
	heap->reuse_list_size_watermark = 0;

	ion_heap_reuse_default_config(heap);

	rwlock_init(&heap->reuse_config_lock);
	heap->reuse_config_fops = &ion_heap_reuse_config_debugfs_ops;

	return 0;
}
