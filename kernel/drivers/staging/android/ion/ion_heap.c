/*
 * drivers/staging/android/ion/ion_heap.c
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

#define pr_fmt(fmt) "[ION: ]" fmt

#include <linux/err.h>
#include <linux/freezer.h>
#include <linux/kthread.h>
#include <linux/mm.h>
#include <linux/rtmutex.h>
#include <linux/sched.h>
#include <uapi/linux/sched/types.h>
#include <linux/scatterlist.h>
#include <linux/seq_file.h>
#include <linux/vmalloc.h>
#ifdef CONFIG_MM_LB
#include <linux/mm_lb/mm_lb.h>
#endif
#if defined(CONFIG_MM_SVM) || defined(CONFIG_ARM_SMMU_V3) || defined(CONFIG_MM_SMMU_V3)
#include <linux/iommu/mm_svm.h>
#endif
#include <linux/bg_dmabuf_reclaim/bg_dmabuf_ops.h>

#include "ion.h"
#include "ion_buffer_reuse.h"

int sysctl_ion_heap_trace_heap_drain = 0;
int sysctl_ion_heap_trace_heap_shrink = 0;

#define ion_heap_trace_heap_drain(fmt, args...)				\
do {									\
	if (sysctl_ion_heap_trace_heap_drain)				\
		trace_printk(fmt, ##args);				\
} while (0)

#define ion_heap_trace_heap_shrink(fmt, args...)			\
do {									\
	if (sysctl_ion_heap_trace_heap_shrink)				\
		trace_printk(fmt, ##args);				\
} while (0)

static inline bool is_release_ion_mem(struct ion_buffer *buffer)
{
	return is_bg_buffer_release((void *)buffer);
}

void *ion_heap_map_kernel(struct ion_heap *heap,
			  struct ion_buffer *buffer)
{
	struct scatterlist *sg;
	int i, j;
	void *vaddr;
	pgprot_t pgprot;
	struct sg_table *table = buffer->sg_table;
	unsigned int npages = PAGE_ALIGN(buffer->size) / PAGE_SIZE;
	struct page **pages = vmalloc(sizeof(struct page *) * npages);
	struct page **tmp = pages;

	if (!pages)
		return ERR_PTR(-ENOMEM);

	if (is_release_ion_mem(buffer)) {
		vfree(pages);
		WARN(1, "%s buf has already release\n", __func__);
		return ERR_PTR(-EINVAL);
	}

	if (buffer->flags & ION_FLAG_CACHED)
		pgprot = PAGE_KERNEL;
	else
		pgprot = pgprot_writecombine(PAGE_KERNEL);

#ifdef CONFIG_MM_LB
	if (buffer->plc_id) {
		pr_info("%s:magic-%lu,lb_pid-%u\n", __func__,
			buffer->magic, buffer->plc_id);
		lb_pid_prot_build(buffer->plc_id, &pgprot);
	}
#endif

	for_each_sg(table->sgl, sg, table->nents, i) {
		int npages_this_entry = PAGE_ALIGN(sg->length) / PAGE_SIZE;
		struct page *page = sg_page(sg);

		BUG_ON(i >= npages);
		for (j = 0; j < npages_this_entry; j++)
			*(tmp++) = page++;
	}
	vaddr = vmap(pages, npages, VM_MAP, pgprot);
	vfree(pages);

	if (!vaddr)
		return ERR_PTR(-ENOMEM);

	/* record info error should not effect kmap result */
	(void)record_bg_buffer_kva_info((void*)buffer, vaddr);

	return vaddr;
}

void ion_heap_unmap_kernel(struct ion_heap *heap,
			   struct ion_buffer *buffer)
{
	/* bg_buffer reclaim unmap func is same as vunmap */
	if (is_release_ion_mem(buffer))
		WARN(1, "%s buf has already release\n", __func__);

	delete_bg_buffer_kva_info((void*)buffer, buffer->vaddr);
	vunmap(buffer->vaddr);
}

#ifdef CONFIG_MM_LB
static int remap_lb_pfn_range(struct ion_buffer *buffer, struct page *page,
	struct vm_area_struct *vma, unsigned long addr, unsigned long len)
{
	int ret = 0;
	unsigned int gid_idx;
	pgprot_t newprot = vma->vm_page_prot;
#ifdef CONFIG_MM_LB_V550
	unsigned int osh_flag;
#endif

	gid_idx = lb_page_to_gid(page);
	newprot = pgprot_lb(newprot, gid_idx);
	
#ifdef CONFIG_MM_LB_V550
	if (lb_get_page_osh_flag(page))
		newprot = __pgprot_modify(newprot, PTE_SH_MASK, PTE_OUTER_SHARED);
#endif
	
	ret = remap_pfn_range(vma, addr, page_to_pfn(page), len, newprot);

	return ret;
}
#endif

int ion_heap_map_user(struct ion_heap *heap, struct ion_buffer *buffer,
		      struct vm_area_struct *vma)
{
	struct sg_table *table = buffer->sg_table;
	unsigned long addr = vma->vm_start;
	unsigned long offset = vma->vm_pgoff * PAGE_SIZE;
	struct scatterlist *sg;
	int i;
	int ret;

	if (is_release_ion_mem(buffer)) {
		WARN(1, "%s buf has already release\n", __func__);
		return -EINVAL;
	}

#ifdef CONFIG_MM_LB
	if (buffer->plc_id)
		pr_info("%s:magic-%lu,start-0x%lx,end-0x%lx\n", __func__,
			buffer->magic, vma->vm_start, vma->vm_end);
	/*
	 * Delete the lb-prot-build process, and do it in remap_lb_pfn_range for
	 * merging the NPU gid process with the other nonzero gids.
	 * As a result, we can get free from checking specific GID and coding with branches.
	 */
#endif

	/*
	 * when buffer is bg_reclaim type, vm_ops is vma_init.dummy_vm_ops
	 * just replace with bg_buffer ops
	 */
	(void)bg_buffer_register_vma_ops((void *)buffer, vma);

	for_each_sg(table->sgl, sg, table->nents, i) {
		struct page *page = sg_page(sg);
		unsigned long remainder = vma->vm_end - addr;
		unsigned long len = sg->length;

		if (offset >= sg->length) {
			offset -= sg->length;
			continue;
		} else if (offset) {
			page += offset / PAGE_SIZE;
			len = sg->length - offset;
			offset = 0;
		}
		len = min(len, remainder);

#ifdef CONFIG_MM_LB
		/*
		 * Merge the NPU gid process with other nonzero gid.
		 * As a result, we can get free from checking specific GID and coding with branches.
		 */
		ret = remap_lb_pfn_range(buffer, page, vma, addr, len);
#else
		ret = remap_pfn_range(vma, addr, page_to_pfn(page), len,
				vma->vm_page_prot);
#endif
		if (ret)
			return ret;
		addr += len;
		if (addr >= vma->vm_end)
			goto done;
	}

done:
#if defined(CONFIG_MM_SVM) || defined(CONFIG_ARM_SMMU_V3) || defined(CONFIG_MM_SMMU_V3)
	if (test_bit(MMF_SVM, &vma->vm_mm->flags))
		mm_svm_flush_cache(vma->vm_mm,
				     vma->vm_start,
				     vma->vm_end - vma->vm_start);
#endif
	/* record info error should not effect mmap result */
	(void)record_bg_buffer_uva_info((void *)buffer, vma);

	return 0;
}

static int ion_heap_clear_pages(struct page **pages, unsigned int num, pgprot_t pgprot)
{
	void *addr = vmap(pages, num, VM_MAP, pgprot);

	if (!addr)
		return -ENOMEM;
	memset(addr, 0, PAGE_SIZE * num);
	vunmap(addr);

	return 0;
}

static int ion_heap_sglist_zero(struct scatterlist *sgl, unsigned int nents,
				pgprot_t pgprot)
{
	unsigned int p = 0;
	int ret = 0;
	struct sg_page_iter piter;
	struct page *pages[32];

	for_each_sg_page(sgl, &piter, nents, 0) {
		pages[p++] = sg_page_iter_page(&piter);
		if (p == ARRAY_SIZE(pages)) {
			ret = ion_heap_clear_pages(pages, p, pgprot);
			if (ret)
				return ret;
			p = 0;
		}
	}
	if (p)
		ret = ion_heap_clear_pages(pages, p, pgprot);

	return ret;
}

int ion_heap_buffer_zero(struct ion_buffer *buffer)
{
	struct sg_table *table = buffer->sg_table;
	pgprot_t pgprot;

	if (buffer->flags & ION_FLAG_CACHED)
		pgprot = PAGE_KERNEL;
	else
		pgprot = pgprot_writecombine(PAGE_KERNEL);

	return ion_heap_sglist_zero(table->sgl, table->nents, pgprot);
}

int ion_heap_pages_zero(struct page *page, size_t size, pgprot_t pgprot)
{
	struct scatterlist sg;

	sg_init_table(&sg, 1);
	sg_set_page(&sg, page, size, 0);
	return ion_heap_sglist_zero(&sg, 1, pgprot);
}

void ion_heap_freelist_add(struct ion_heap *heap, struct ion_buffer *buffer)
{
	spin_lock(&heap->free_lock);
	list_add(&buffer->list, &heap->free_list);
	heap->free_list_size += buffer->size;
	spin_unlock(&heap->free_lock);
	wake_up(&heap->waitqueue);
}

size_t ion_heap_freelist_size(struct ion_heap *heap)
{
	size_t size;

	spin_lock(&heap->free_lock);
	size = heap->free_list_size;
	spin_unlock(&heap->free_lock);

	return size;
}

static size_t ion_heap_drain_from_freelist(struct ion_heap *heap, size_t size,
					   bool skip_pools)
{
	size_t sz_drained = 0;
	struct list_head drained;
	struct ion_buffer *buffer = NULL;
	struct ion_buffer *tmp_buffer = NULL;

	if (ion_heap_freelist_size(heap) == 0)
		return 0;

	ion_heap_trace_heap_drain("DRAIN:Type=%d, Expected=%zu, Available=%zu\n",
				  heap->type, size, heap->free_list_size);

	INIT_LIST_HEAD(&drained);

	spin_lock(&heap->free_lock);
	list_for_each_entry_safe(buffer, tmp_buffer, &heap->free_list, list) {
		list_del(&buffer->list);
		list_add(&buffer->list, &drained);
		sz_drained += buffer->size;

		if (skip_pools)
			buffer->private_flags |= ION_PRIV_FLAG_SHRINKER_FREE;

		if (sz_drained >= size)
			break;
	}
	heap->free_list_size -= sz_drained;
	spin_unlock(&heap->free_lock);

	list_for_each_entry_safe(buffer, tmp_buffer, &drained, list) {
		list_del(&buffer->list);
		ion_buffer_destroy(buffer);
	}

	ion_heap_trace_heap_drain("DRAIN:Type=%d, Reclaimed=%zu, Available=%zu\n",
				  heap->type, sz_drained, heap->free_list_size - sz_drained);

	return sz_drained;
}

size_t ion_heap_freelist_drain(struct ion_heap *heap, size_t size)
{
	if (size == 0)
		size = ULONG_MAX;

	return ion_heap_drain_from_freelist(heap, size, false);
}

size_t ion_heap_freelist_shrink(struct ion_heap *heap, size_t size)
{
	if (size == 0)
		size = ULONG_MAX;

	return ion_heap_drain_from_freelist(heap, size, true);
}

static int ion_heap_deferred_free(void *data)
{
	struct ion_heap *heap = data;
	struct list_head free_list;

	INIT_LIST_HEAD(&free_list);

	while (true) {
		struct ion_buffer *buffer, *tmp;

		wait_event_freezable(heap->waitqueue,
				     ion_heap_freelist_size(heap) > 0);

		spin_lock(&heap->free_lock);
		list_splice_init(&heap->free_list, &free_list);
		heap->free_list_size = 0;
		spin_unlock(&heap->free_lock);

		list_for_each_entry_safe(buffer, tmp, &free_list, list) {
			list_del(&buffer->list);
			if (ion_heap_try_reuselist_add(heap, buffer))
				continue;
			ion_buffer_destroy(buffer);
		}
	}

	return 0;
}

int ion_heap_init_deferred_free(struct ion_heap *heap)
{
	struct sched_param param = { .sched_priority = 0 };

	INIT_LIST_HEAD(&heap->free_list);
	init_waitqueue_head(&heap->waitqueue);
	heap->task = kthread_run(ion_heap_deferred_free, heap,
				 "%s", heap->name);
	if (IS_ERR(heap->task)) {
		pr_err("%s: creating thread for deferred free failed\n",
		       __func__);
		return PTR_ERR_OR_ZERO(heap->task);
	}
	sched_setscheduler(heap->task, SCHED_IDLE, &param);
	return 0;
}

static unsigned long ion_heap_shrink_count(struct shrinker *shrinker,
					   struct shrink_control *sc)
{
	struct ion_heap *heap = container_of(shrinker, struct ion_heap,
					     shrinker);
	size_t total = 0;

	total = ion_heap_freelist_size(heap) / PAGE_SIZE;
#ifdef CONFIG_ION_BUFFER_REUSE
	if (heap->flags & ION_HEAP_FLAG_FREE_BUF_REUSE)
		total += ion_heap_reuselist_size(heap) / PAGE_SIZE;
#endif
	if (heap->ops->shrink)
		total += heap->ops->shrink(heap, sc->gfp_mask, 0);
	return total;
}

static unsigned long ion_heap_shrink_scan(struct shrinker *shrinker,
					  struct shrink_control *sc)
{
	struct ion_heap *heap = container_of(shrinker, struct ion_heap,
					     shrinker);
	ssize_t freed = 0;
	ssize_t total_freed = 0;
	unsigned long ret = 0;
	long to_scan = sc->nr_to_scan;
	bool skip_pool = heap->ops->shrink == NULL;

	if (to_scan == 0)
		return SHRINK_STOP;

	if (heap->flags & ION_HEAP_FLAG_DEFER_FREE) {
		freed = ion_heap_drain_from_freelist(heap, to_scan * PAGE_SIZE, skip_pool) /
				PAGE_SIZE;
		ion_heap_trace_heap_shrink("SHRINK:Drain from free list:Type=%d, Required=%zu Reclaimed=%zu\n",
					   heap->type, to_scan * PAGE_SIZE, freed * PAGE_SIZE);
		to_scan -= freed;
		total_freed += freed;
		if (to_scan <= 0) {
			ret = SHRINK_STOP;
			goto out;
		}
	}

	if (!skip_pool) {
		freed = heap->ops->shrink(heap, sc->gfp_mask, to_scan);
		ion_heap_trace_heap_shrink("SHRINK:Drain page pool:Type=%d, Required=%zu Reclaimed=%zu\n",
					   heap->type, to_scan * PAGE_SIZE, freed * PAGE_SIZE);
		to_scan -= freed;
		total_freed += freed;
		if (to_scan <= 0) {
			ret = SHRINK_STOP;
			goto out;
		}
	}
#ifdef CONFIG_ION_BUFFER_REUSE
	if (heap->flags & ION_HEAP_FLAG_FREE_BUF_REUSE) {
		freed = ion_heap_drain_from_reuselist(heap, to_scan * PAGE_SIZE, skip_pool) /
				PAGE_SIZE;
		ion_heap_trace_heap_shrink("SHRINK:Drain from reuse list:Type=%d, Required=%zu Reclaimed=%zu\n",
					   heap->type, to_scan * PAGE_SIZE, freed * PAGE_SIZE);
		if (!skip_pool) {
			freed = heap->ops->shrink(heap, sc->gfp_mask, to_scan);
			ion_heap_trace_heap_shrink("SHRINK:Drain page pool:Type=%d, Required=%zu Reclaimed=%zu\n",
					   heap->type, to_scan * PAGE_SIZE, freed * PAGE_SIZE);
		}
		to_scan -= freed;
		total_freed += freed;
		if (to_scan <= 0)
			ret = SHRINK_STOP;
	}
#endif
out:
	ret = total_freed;

	return ret;
}

void ion_heap_init_shrinker(struct ion_heap *heap)
{
	heap->shrinker.count_objects = ion_heap_shrink_count;
	heap->shrinker.scan_objects = ion_heap_shrink_scan;
	heap->shrinker.seeks = DEFAULT_SEEKS;
	heap->shrinker.batch = 0;
	register_shrinker(&heap->shrinker);
}
