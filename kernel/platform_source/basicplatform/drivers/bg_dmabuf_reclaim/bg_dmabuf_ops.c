/*
 * bg_dmabuf_ops.c
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
#include <asm/tlb.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/iommu/mm_iommu.h>
#include <linux/mm.h>

#include <linux/bg_dmabuf_reclaim/bg_dmabuf_ops.h>
#include <linux/bg_dmabuf_reclaim/bg_dmabuf_info_manager.h>

#include "bg_internal_buf_interface.h"
#include "bg_dmabuf_test.h"

static atomic_long_t bg_buffer_free_size = ATOMIC_INIT(0);

struct bg_op_time_stamp {
	ktime_t _stime;
	ktime_t _lock_time;
	ktime_t _sem_time;
	ktime_t _mem_time;
	ktime_t _kva_op_time;
	ktime_t _uva_op_time;
	ktime_t _iova_op_time;
	ktime_t _gva_op_time;
};

static struct bg_buf_gpu_callback g_gpu_cb[BG_BUF_CB_MAX];
static DEFINE_MUTEX(g_gpu_cb_mutex);

/*
 * bg_buffer_register_callback - register gpu map & unmap func, used by gpu driver
 * @type: index of callback func in g_gpu_cb
 * @cb: map or unmap callback func
 *
 * return 0 means success, otherwise means error
 */
int bg_buffer_register_callback(enum bg_buf_cb_type type,
	int (*cb)(struct dma_buf *dmabuf))
{
	unsigned int index = type;

	if (cb == NULL || type >= BG_BUF_CB_MAX)
		return -EINVAL;

	mutex_lock(&g_gpu_cb_mutex);

	if (g_gpu_cb[index].cb != NULL) {
		bg_buf_pr_err("%s, has register callback func\n", __func__);
		mutex_unlock(&g_gpu_cb_mutex);
		return -EEXIST;
	}

	g_gpu_cb[index].cb = cb;
	mutex_unlock(&g_gpu_cb_mutex);
	return 0;
}
EXPORT_SYMBOL(bg_buffer_register_callback);

/*
 * The current GPU-involved process consists of two parts:
 * [1] This interface should not be invoked to reclaim the memory of the output-buffer.
 * If the GPU output buffer is reclaimed, the framework is required to solve the problem.
 * [2] When the GPU use dmabuf, it calls dmabuf-attach first. If bg_reclaim is also called,
 * this call is intercepted in the bg_reclaim func process. Therefore, this problem doesn't occur
 * even if the map function of the GPU is not called.
 *
 * GPU invoking has retrieval overhead, so we stub this process and enable it when required.
 */
static int bg_buffer_gpu_ops(struct dma_buf *dmabuf, enum bg_buf_cb_type type)
{
	if (type >= BG_BUF_CB_MAX || g_gpu_cb[type].cb == NULL) {
		bg_buf_pr_err("%s input error, type-%d\n", __func__, type);
		return -EINVAL;
	}
	return g_gpu_cb[type].cb(dmabuf);
}

unsigned long get_bg_buffer_released_size(void)
{
	return (unsigned long)atomic_long_read(&bg_buffer_free_size);
}

static inline struct dma_buf *__file_to_dma_buf(struct file *file)
{
	return file->private_data;
}

bool is_bg_buffer(void *buffer)
{
	return (get_info_from_bg_buffer(buffer) != NULL);
}

bool is_bg_dmabuf(struct    dma_buf *dmabuf)
{
	void *buffer = get_buffer_ptr_from_dmabuf(dmabuf);

	return (get_info_from_bg_buffer(buffer) != NULL);
}

bool is_bg_dmabuf_release(struct dma_buf *dmabuf)
{
	void *buffer = get_buffer_ptr_from_dmabuf(dmabuf);

	if (!buffer || !is_buffer_valid(buffer))
		return false;

	return is_bg_buffer_release(buffer);
}

void bg_buffer_destroy(void *buffer)
{
	bool has_release_mem = false;

	buffer_lock(buffer);

	has_release_mem = is_bg_buffer_release((void *)buffer);
	if (has_release_mem) {
		bg_buf_pr_info("%s ion_buffer already free, no need to free\n", __func__);
		goto out;
	}

	buffer_free_memory(buffer);

	atomic_long_sub(get_buffer_size(buffer), &bg_buffer_free_size);
out:
	free_bg_buffer_info((void*)buffer);
	buffer_unlock(buffer);
}

static int __get_dmabuf_from_vma(struct vm_area_struct *vma, struct dma_buf **dma_buf)
{
	struct file *file = vma->vm_file;
	struct dma_buf *dmabuf = NULL;

	if (!file || !is_dma_buf_file(file))
		return -EINVAL;

	dmabuf = __file_to_dma_buf(file);
	if (IS_ERR_OR_NULL(dmabuf))
		return -ENODEV;

	*dma_buf = dmabuf;
	return 0;
}

static int __get_buffer_ptr_from_vma(struct vm_area_struct *vma, void **buffer)
{
	int ret = 0;
	struct dma_buf *dmabuf = NULL;

	ret = __get_dmabuf_from_vma(vma, &dmabuf);
	if (ret)
		return ret;

	*buffer = get_buffer_ptr_from_dmabuf(dmabuf);
	if (!(*buffer))
		return -EFAULT;

	return 0;
}

static void bg_buffer_open(struct vm_area_struct *vma)
{
	int ret;
	void *buffer = NULL;

	ret = __get_buffer_ptr_from_vma(vma, &buffer);
	if (ret) {
		bg_buf_pr_err("%s get_buffer_ptr_from_vma failed, ret = %d\n", __func__, ret);
		return;
	}
	record_bg_buffer_uva_info(buffer, vma);
	bg_buf_pr_debug("%s vma-0x%pK mm-0x%pK vma_ops->open ok\n", __func__, vma, vma->vm_mm);
}

static void bg_buffer_close(struct vm_area_struct *vma)
{
	void *buffer = NULL;
	int ret;

	ret = __get_buffer_ptr_from_vma(vma, &buffer);
	if (ret) {
		bg_buf_pr_err("%s get_buffer_ptr_from_vma failed, ret = %d\n", __func__, ret);
		return;
	}
	delete_bg_buffer_uva_info(buffer, vma);
	bg_buf_pr_debug("%s vma-0x%pK mm-0x%pK vma_ops->close ok\n", __func__, vma, vma->vm_mm);
}

const struct vm_operations_struct bg_vm_ops = {
	.open = bg_buffer_open,
	.close = bg_buffer_close,
};

void bg_buffer_register_vma_ops(void *buffer, struct vm_area_struct *vma)
{
	struct bg_dmabuf_info *info = get_info_from_bg_buffer(buffer);

	/*
	 * if bg_buffer info is not exist, vm_ops means nothing
	 * otherwise, dmabuf-vma-vm_ops is dummy_vma_ops which init in vma_init
	 * so we just replace vm_ops wich bg_vm_ops
	 */
	if (info && (is_info_missing_va_record(info) == false))
		vma->vm_ops = &bg_vm_ops;
}

static int __op_get_valid_buffer_and_info(struct dma_buf *dmabuf, bool check_missing_info,
	void **_buffer, struct bg_dmabuf_info **_buffer_info)
{
	void *buffer = NULL;
	struct bg_dmabuf_info *buffer_info = NULL;

	if (IS_ERR_OR_NULL(dmabuf)) {
		bg_buf_pr_debug("%s dmabuf is error\n", __func__);
		return -EINVAL;
	}
	buffer = get_buffer_ptr_from_dmabuf(dmabuf);
	buffer_info = get_info_from_bg_buffer(buffer);
	if (!buffer_info) {
		bg_buf_pr_debug("%s get info failed\n", __func__);
		return -EINVAL;
	}

	if (check_missing_info && is_info_missing_va_record(buffer_info)) {
		bg_buf_pr_debug("%s buf miss va record, not support bg_ops\n", __func__);
		return -EAGAIN;
	}

	*_buffer = buffer;
	*_buffer_info = buffer_info;
	return 0;
}

int bg_buffer_vma_unmap_lock(struct vm_area_struct *vma, void **dma_buf)
{
	int ret = 0;
	struct dma_buf *dmabuf = NULL;
	void *buffer = NULL;
	struct bg_dmabuf_info *buffer_info = NULL;

	ret = __get_dmabuf_from_vma(vma, &dmabuf);
	if (ret) {
		bg_buf_pr_debug("%s get info failed, ret = %d\n", __func__, ret);
		return ret;
	}

	get_dma_buf(dmabuf);
	ret = __op_get_valid_buffer_and_info(dmabuf, true, &buffer, &buffer_info);
	if (ret) {
		dma_buf_put(dmabuf);
		return ret;
	}

	*dma_buf = (void *)dmabuf;
	mutex_lock(&buffer_info->unmap_lock);
	return ret;
}

void bg_buffer_vma_unmap_unlock(void *dma_buf)
{
	int ret = 0;
	struct bg_dmabuf_info *buffer_info = NULL;
	struct dma_buf *dmabuf = (struct dma_buf *)dma_buf;
	void *buffer = NULL;

	ret = __op_get_valid_buffer_and_info(dmabuf, false, &buffer, &buffer_info);
	if (ret) {
		bg_buf_pr_err("%s get info failed, ret = %d\n", __func__, ret);
		return;
	}

	mutex_unlock(&buffer_info->unmap_lock);
	dma_buf_put(dmabuf);
	return;
}

bool bg_dmabuf_down_read(struct dma_buf *dmabuf)
{
	void *buffer = NULL;
	struct bg_dmabuf_info *buffer_info = NULL;
	struct rw_semaphore *rwsem = NULL;
	int ret;

	ret = __op_get_valid_buffer_and_info(dmabuf, false, &buffer, &buffer_info);
	if (ret)
		return false;

	rwsem = &buffer_info->rwsem;
	ret = down_read_killable(rwsem);
	if (ret)
		return false;

	return true;
}

void bg_dmabuf_up_read(struct dma_buf *dmabuf)
{
	void *buffer = NULL;
	struct bg_dmabuf_info *buffer_info = NULL;
	struct rw_semaphore *rwsem = NULL;
	int ret;

	ret = __op_get_valid_buffer_and_info(dmabuf, false, &buffer, &buffer_info);
	if (ret) {
		WARN(1, "%s bg_dmabuf rwsem up failed\n", __func__);
		return;
	}

	rwsem = &buffer_info->rwsem;
	up_read(rwsem);
}

static void __bg_buffer_kva_init_pages(void *buffer, struct page **pages, int npages)
{
	struct sg_table *table = NULL;
	struct page **tmp = pages;
	struct scatterlist *sg = NULL;
	int i, j;

	table = get_buffer_sg_table(buffer);
	if (!table) {
		bg_buf_pr_err("%s sg_table is NULL\n", __func__);
		return;
	}

	for_each_sg(table->sgl, sg, table->nents, i) {
		int npages_this_entry = PAGE_ALIGN(sg->length) / PAGE_SIZE;
		struct page *page = sg_page(sg);

		BUG_ON(i >= npages);
		for (j = 0; j < npages_this_entry; j++)
			*(tmp++) = page++;
	}
}

static int __bg_buffer_kva_map(void *buffer,
	struct dma_buf *dmabuf, struct bg_dmabuf_info *info)
{
	struct bg_dmabuf_kva_info *kva_info = NULL;
	struct list_head *pos = NULL;
	struct list_head *q = NULL;
	pgprot_t pgprot = get_buffer_kmap_prot(buffer);
	int npages = PAGE_ALIGN(get_buffer_size(buffer)) / PAGE_SIZE;
	struct page **pages = vmalloc(sizeof(struct page *) * npages);
	int map_failed = 0;

	if (!pages)
		return -ENOMEM;

	bg_buf_pr_debug("%s in\n", __func__);

	__bg_buffer_kva_init_pages(buffer, pages, npages);

	mutex_lock(&info->kva_info_mutex);
	list_for_each_safe(pos, q, &info->kva_info) {
		struct vm_struct *area = NULL;
		int ret;

		kva_info = list_entry(pos, struct bg_dmabuf_kva_info, list);
		if (kva_info->state == BG_BUFFER_MAP) {
			bg_buf_pr_info("%s iova[0x%pK] has not unmap, no need to map\n",
				__func__, kva_info->vaddr);
			continue;
		}

		area = find_vm_area(kva_info->vaddr);
		if (unlikely(!area)) {
			map_failed++;
			bg_buf_pr_err("%s Trying to find vm_area nonexistent (0x%pK)\n",
				__func__, kva_info->vaddr);
			continue;
		}

		ret = map_kernel_range((unsigned long)area->addr, (unsigned long)npages << PAGE_SHIFT,
			pgprot_nx(pgprot), pages);
		if (ret < 0) {
			map_failed++;
			bg_buf_pr_err("%s map_kernel_range failed\n", __func__);
		} else {
			kva_info->state = BG_BUFFER_MAP;
			bg_buf_pr_debug("%s vaddr-0x%pK \n", __func__, kva_info->vaddr);
		}
	}
	mutex_unlock(&info->kva_info_mutex);
	vfree(pages);
	bg_buf_pr_debug("%s ok\n", __func__);
	return map_failed;
}

static int __bg_buffer_kva_unmap(void *buffer, struct dma_buf *dmabuf, struct bg_dmabuf_info *info)
{
	struct bg_dmabuf_kva_info *kva_info = NULL;
	int npages = PAGE_ALIGN(get_buffer_size(buffer)) / PAGE_SIZE;
	struct list_head *pos = NULL;
	struct list_head *q = NULL;
	void *addr = NULL;

	bg_buf_pr_debug("%s in\n", __func__);

	mutex_lock(&info->kva_info_mutex);
	list_for_each_safe(pos, q, &info->kva_info) {
		kva_info = list_entry(pos, struct bg_dmabuf_kva_info, list);
		addr = kva_info->vaddr;
		if (!PAGE_ALIGNED(addr) || unlikely(!find_vm_area(addr))) {
			bg_buf_pr_err("%s Trying to unmap() bad addr\n", __func__);
			mutex_unlock(&info->kva_info_mutex);
			return -EINVAL;
		}

		unmap_kernel_range((unsigned long)(uintptr_t)addr, (unsigned long)npages << PAGE_SHIFT);

		kva_info->state = BG_BUFFER_UNMAP;
		bg_buf_pr_debug("%s unmap start[0x%pK] size[0x%llx]\n",
			__func__, addr, (unsigned long)npages << PAGE_SHIFT);
	}
	mutex_unlock(&info->kva_info_mutex);
	bg_buf_pr_debug("%s ok\n", __func__);
	return 0;
}

static int __bg_buffer_uva_map(void *buffer,
	struct dma_buf *dmabuf, struct bg_dmabuf_info *info)
{
	struct bg_dmabuf_uva_info *uva_info = NULL;
	struct list_head *pos = NULL;
	struct list_head *q = NULL;
	struct sg_table *table = NULL;
	int map_failed = 0;
	int i;

	bg_buf_pr_debug("%s in\n", __func__);

	table = get_buffer_sg_table(buffer);
	if (!table) {
		bg_buf_pr_err("%s sg_table is NULL\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&info->unmap_lock);
	mutex_lock(&info->uva_info_mutex);
	list_for_each_safe(pos, q, &info->uva_info) {
		struct vm_area_struct *vma = NULL;
		struct scatterlist *sg = NULL;
		unsigned long addr, offset;
		int ret = 0;

		uva_info = list_entry(pos, struct bg_dmabuf_uva_info, list);
		vma = uva_info->vma;
		addr = vma->vm_start;
		offset = vma->vm_pgoff * PAGE_SIZE;

		bg_buf_pr_debug("%s task-%s vma-[0x%pK] addr[0x%llx] len[0x%llx] offset[0x%llx]\n",
			__func__, uva_info->task_name, vma, addr,
			vma->vm_end - vma->vm_start, offset);

		if (uva_info->state == BG_BUFFER_MAP) {
			bg_buf_pr_info("%s vma-[0x%pK] uva[0x%llx] has not unmap, no need to map\n",
				__func__, vma, vma->vm_start);
			continue;
		}

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

			ret = remap_pfn_range(vma, addr, page_to_pfn(page), len,
				vma->vm_page_prot);
			if (ret) {
				bg_buf_pr_err("%s ion mmap failed, addr-0x%llx, len-0x%llx\n",
					__func__, addr, len);
				break;
			}

			addr += len;
			if (addr >= vma->vm_end)
				break;
		}
		if (ret)
			map_failed++;
		else
			uva_info->state = BG_BUFFER_MAP;
	}

	mutex_unlock(&info->uva_info_mutex);
	mutex_unlock(&info->unmap_lock);
	bg_buf_pr_debug("%s ok\n", __func__);
	return map_failed;
}

static void bg_buffer_unmap_region(struct mm_struct *mm,
		struct vm_area_struct *vma, struct vm_area_struct *prev,
		unsigned long start, unsigned long end)
{
	struct vm_area_struct *next = vma->vm_next;
	struct mmu_gather tlb;

	lru_add_drain();
	tlb_gather_mmu(&tlb, mm, start, end);
	update_hiwater_rss(mm);
	unmap_vmas(&tlb, vma, start, end);
	/* not unlink file tree, not as free_pgtables */
	free_pgd_range(&tlb, vma->vm_start, vma->vm_end,
				prev ? prev->vm_end : FIRST_USER_ADDRESS,
				next ? next->vm_start : USER_PGTABLES_CEILING);
	tlb_finish_mmu(&tlb, start, end);
}

static int __bg_buffer_uva_unmap(struct dma_buf *dmabuf, struct bg_dmabuf_info *info)
{
	struct vm_area_struct *vma = NULL;
	struct vm_area_struct *prev = NULL;
	struct list_head *pos = NULL;
	struct list_head *q = NULL;
	struct bg_dmabuf_uva_info *uva_info = NULL;
	struct mm_struct *mm = NULL;

	bg_buf_pr_debug("%s in\n", __func__);
	mutex_lock(&info->unmap_lock);
	mutex_lock(&info->uva_info_mutex);
	list_for_each_safe(pos, q, &info->uva_info) {
		uva_info = list_entry(pos, struct bg_dmabuf_uva_info, list);
		vma = uva_info->vma;
		prev = vma->vm_prev;
		mm = uva_info->mm;

		bg_buf_pr_debug("%s task-%s addr-0x%llx end-0x%llx vm_start-0x%llx vm_end-0x%llx\n",
			__func__, uva_info->task_name,
			vma->vm_start, vma->vm_end,
			vma->vm_start, vma->vm_end);

		bg_buffer_unmap_region(mm, vma, prev, vma->vm_start, vma->vm_end);
		uva_info->state = BG_BUFFER_UNMAP;
	}
	mutex_unlock(&info->uva_info_mutex);
	mutex_unlock(&info->unmap_lock);
	bg_buf_pr_debug("%s ok\n", __func__);
	return 0;
}

static int __bg_buffer_iova_map(void *buffer, struct dma_buf *dmabuf, struct bg_dmabuf_info *info)
{
	struct list_head *pos = NULL;
	struct list_head *q = NULL;
	struct bg_dmabuf_iova_info *iova_info = NULL;
	struct sg_table *sgt = get_buffer_sg_table(buffer);
	unsigned long ret;
	int map_failed = 0;
	unsigned long out_size = 0;

	bg_buf_pr_debug("%s in\n", __func__);
	mutex_lock(&info->iova_info_mutex);
	list_for_each_safe(pos, q, &info->iova_info) {
		iova_info = list_entry(pos, struct bg_dmabuf_iova_info, list);

		if (iova_info->state == BG_BUFFER_MAP) {
			bg_buf_pr_info("%s iova[0x%llx] has not unmap, no need to map\n",
				__func__, iova_info->iova);
			continue;
		}

		ret = kernel_iommu_map_bg_dmabuf(iova_info->dev, dmabuf, sgt,
			iova_info->iova, iova_info->prot, &out_size);
		if (!ret || out_size != iova_info->length) {
			map_failed++;
			bg_buf_pr_err("%s dev-%s iova-0x%llx out_size-0x%llx length-0x%llx map failed\n",
				__func__, dev_name(iova_info->dev), iova_info->iova, out_size, iova_info->length);
			continue;
		}

		iova_info->state = BG_BUFFER_MAP;
		bg_buf_pr_debug("%s dev-%s iova-0x%llx length-0x%llx out-0x%llx\n",
			__func__, dev_name(iova_info->dev), iova_info->iova, iova_info->length, out_size);
	}
	mutex_unlock(&info->iova_info_mutex);
	bg_buf_pr_debug("%s ok\n", __func__);
	return map_failed;
}

static int __bg_buffer_iova_unmap(struct dma_buf *dmabuf, struct bg_dmabuf_info *info)
{
	struct list_head *pos = NULL;
	struct list_head *q = NULL;
	struct bg_dmabuf_iova_info *iova_info = NULL;
	int ret;

	bg_buf_pr_debug("%s in\n", __func__);
	mutex_lock(&info->iova_info_mutex);
	list_for_each_safe(pos, q, &info->iova_info) {
		iova_info = list_entry(pos, struct bg_dmabuf_iova_info, list);
		ret = kernel_iommu_unmap_bg_dmabuf(iova_info->dev, dmabuf, iova_info->iova);
		if (ret) {
			bg_buf_pr_err("%s dev-%s iova-0x%llx length-0x%llx unmap failed\n",
				__func__, dev_name(iova_info->dev), iova_info->iova, iova_info->length);
			mutex_unlock(&info->iova_info_mutex);
			return -EFAULT;
		}

		iova_info->state = BG_BUFFER_UNMAP;
		bg_buf_pr_debug("%s dev-%s iova-0x%llx len-0x%llx prot-%d\n",
			__func__, dev_name(iova_info->dev), iova_info->iova, iova_info->length, iova_info->prot);
	}
	mutex_unlock(&info->iova_info_mutex);
	bg_buf_pr_debug("%s ok\n", __func__);
	return 0;
}

static int __buffer_realloc_memory(void *buffer)
{
	int ret;
	size_t buffer_size = get_buffer_size(buffer);

	ret = buffer_alloc_memory(buffer);
	if (ret) {
		bg_buf_pr_err("%s alloc failed\n", __func__);
		return -ENOMEM;
	}

	atomic_long_sub(buffer_size, &bg_buffer_free_size);

	bg_buf_pr_debug("%s buffer-size[0x%llx] total-free[0x%llx]\n",
		__func__, buffer_size, atomic_long_read(&bg_buffer_free_size));
	return 0;
}

static void __buffer_reclaim_memory(void *buffer)
{
	size_t buffer_size = get_buffer_size(buffer);

	buffer_free_memory(buffer);

	atomic_long_add(buffer_size, &bg_buffer_free_size);

	bg_buf_pr_debug("%s buffer-size[0x%llx] total-free[0x%llx]\n",
		__func__, buffer_size, atomic_long_read(&bg_buffer_free_size));
}

static void __print_reclaim_cost(void *buffer, struct bg_op_time_stamp *t)
{
	bg_buf_pr_info("bg_reclaim size-0x%lx COST(us): lock-%lld sem-%lld mem-%lld total-%lld\n",
		get_buffer_size(buffer),
		ktime_us_delta(t->_lock_time, t->_iova_op_time),
		ktime_us_delta(t->_sem_time, t->_stime),
		ktime_us_delta(t->_mem_time, t->_uva_op_time),
		ktime_us_delta(ktime_get(), t->_stime));
	bg_buf_pr_info("bg_reclaim COST(us): kva-%lld uva-%lld iova-%lld gva-%lld\n",
		ktime_us_delta(t->_kva_op_time, t->_lock_time),
		ktime_us_delta(t->_uva_op_time, t->_kva_op_time),
		ktime_us_delta(t->_iova_op_time, t->_gva_op_time),
		ktime_us_delta(t->_gva_op_time, t->_sem_time));
}

static void __print_realloc_cost(void *buffer, struct bg_op_time_stamp *t)
{
	bg_buf_pr_info("bg_realloc size-0x%lx COST(us): lock-%lld sem-%lld mem-%lld total-%lld\n",
		get_buffer_size(buffer),
		ktime_us_delta(t->_lock_time, t->_sem_time),
		ktime_us_delta(t->_sem_time, t->_stime),
		ktime_us_delta(t->_mem_time, t->_lock_time),
		ktime_us_delta(ktime_get(), t->_stime));
	bg_buf_pr_info("bg_realloc COST(us): kva-%lld uva-%lld iova-%lld gva-%lld\n",
		ktime_us_delta(t->_kva_op_time, t->_mem_time),
		ktime_us_delta(t->_uva_op_time, t->_kva_op_time),
		ktime_us_delta(t->_iova_op_time, t->_uva_op_time),
		ktime_us_delta(t->_gva_op_time, t->_iova_op_time));
}

/*
 * If buffer attach device or miss va record, return -EINVAL
 * If buffer bg_reclaim but has been released, return -EINVAL
 * If buffer bg_realloc but has not been released, return -EINVAL
 * Otherwise, return success
 */
static int __bg_check_buf_stat(struct bg_dmabuf_info *buffer_info, bool is_reclaim)
{
	int ret = 0;
	int attach_num, is_missing_info, is_release;

	attach_num = atomic_read(&buffer_info->attach_num);
	is_missing_info = atomic_read(&buffer_info->is_missing_info);
	if (attach_num ||is_missing_info)
		ret = -EINVAL;

	is_release = atomic_read(&buffer_info->is_release);
	if (is_reclaim && is_release)
		ret = -EINVAL;
	else if (!is_reclaim && (is_release == 0))
		ret = -EINVAL;

	if (ret)
		bg_buf_pr_info("%s no need to [%s], attch-%d, missing-%d release-%d\n",
			__func__, is_reclaim ? "reclaim" : "realloc",
			attach_num, is_missing_info, is_release);

	return ret;
}

/*
 * return: 0 means succeess
 * 	-EFAULT means exec error and restore error
 * 	-ENOEXEC means exec error but restore succeess
 */
static int __bg_buf_cpu_unmap(void *buffer, struct dma_buf *dmabuf,
	struct bg_dmabuf_info *buffer_info, struct bg_op_time_stamp *t)
{
	int ret;
	int result = -ENOEXEC;

	ret = __bg_buffer_kva_unmap(buffer, dmabuf, buffer_info);
	if (ret) {
		bg_buf_pr_err("%s kva unmap failed\n", __func__);
		goto kva_unmap_failed;
	}
	t->_kva_op_time = ktime_get();

	ret = __bg_buffer_uva_unmap(dmabuf, buffer_info);
	if (ret) {
		bg_buf_pr_err("%s uva unmap failed\n", __func__);
		goto uva_unmap_failed;
	}
	t->_uva_op_time = ktime_get();
	return 0;

uva_unmap_failed:
	ret = __bg_buffer_uva_map(buffer, dmabuf, buffer_info);
	if (ret) {
		bg_buf_pr_info("%s uva map failed\n", __func__);
		result = -EFAULT;
	}
kva_unmap_failed:
	/* recover the page table as much as we can */
	ret = __bg_buffer_kva_map(buffer, dmabuf, buffer_info);
	if (ret) {
		bg_buf_pr_info("%s kva map failed\n", __func__);
		result = -EFAULT;
	}
	return result;
}

static int __bg_buf_cpu_map(void *buffer, struct dma_buf *dmabuf,
	struct bg_dmabuf_info *buffer_info, struct bg_op_time_stamp *t)
{
	int ret;
	int result = 0;

	ret = __bg_buffer_kva_map(buffer, dmabuf, buffer_info);
	if (ret) {
		/* map as much as we can, in case some undefined access */
		bg_buf_pr_err("%s kva map failed\n", __func__);
		result = -EFAULT;
	}
	t->_kva_op_time = ktime_get();

	ret = __bg_buffer_uva_map(buffer, dmabuf, buffer_info);
	if (ret) {
		/* map as much as we can, in case some undefined access */
		bg_buf_pr_err("%s uva map failed\n", __func__);
		result = -EFAULT;
	}
	t->_uva_op_time = ktime_get();

	return result;
}

/*
 * return: 0 means succeess
 * 	-EFAULT means exec error and restore error
 * 	-ENOEXEC means exec error but restore succeess
 */
static int __bg_buf_device_unmap(void *buffer, struct dma_buf *dmabuf,
	struct bg_dmabuf_info *buffer_info, struct bg_op_time_stamp *t)
{
	int ret;
	int result = -ENOEXEC;

	ret = bg_buffer_gpu_ops(dmabuf, BG_BUF_CB_GPU_UNMAP);
	if (ret) {
		bg_buf_pr_err("%s gva unmap failed\n", __func__);
		goto gva_unmap_failed;
	}
	t->_gva_op_time = ktime_get();

	ret = __bg_buffer_iova_unmap(dmabuf, buffer_info);
	if (ret) {
		bg_buf_pr_err("%s iova unmap failed\n", __func__);
		goto iova_unmap_failed;
	}
	t->_iova_op_time = ktime_get();
	return 0;

iova_unmap_failed:
	ret = __bg_buffer_iova_map(buffer, dmabuf, buffer_info);
	if (ret) {
		bg_buf_pr_err("%s iova map failed\n", __func__);
		result = -EFAULT;
	}
gva_unmap_failed:
	ret = bg_buffer_gpu_ops(dmabuf, BG_BUF_CB_GPU_MAP);
	if (ret) {
		bg_buf_pr_err("%s gva map failed\n", __func__);
		result = -EFAULT;
	}
	return result;
}

static int __bg_buf_device_map(void *buffer, struct dma_buf *dmabuf,
	struct bg_dmabuf_info *buffer_info, struct bg_op_time_stamp *t)
{
	int ret;
	int result = 0;

	ret = __bg_buffer_iova_map(buffer, dmabuf, buffer_info);
	if (ret) {
		bg_buf_pr_err("%s iova map failed\n", __func__);
		result = -EFAULT;
	}
	t->_iova_op_time = ktime_get();

	ret = bg_buffer_gpu_ops(dmabuf, BG_BUF_CB_GPU_MAP);
	if (ret) {
		bg_buf_pr_err("%s gva unmap failed\n", __func__);
		result = -EFAULT;
	}
	t->_gva_op_time = ktime_get();

	return result;
}

static int __bg_buf_set_in_reclaim(struct bg_dmabuf_info *buffer_info)
{
	int pin_state;

	pin_state = atomic_read(&buffer_info->pin_state);
	if (pin_state == BG_BUFFER_RECLAIM_PINNED)
		return -ENOEXEC;

	atomic_set(&buffer_info->pin_state, BG_BUFFER_IN_RECLAIM);
	return 0;
}

static inline void __bg_buf_set_out_reclaim(struct bg_dmabuf_info *buffer_info)
{
	atomic_set(&buffer_info->pin_state, BG_BUFFER_OUT_RECLAIM);
}

static int __bg_buf_set_pin_state(struct dma_buf *dmabuf, int new_pin_state)
{
	void *buffer = NULL;
	struct bg_dmabuf_info *buffer_info = NULL;
	int org_pin_state = 0;
	int ret;

	/*
	 * If the buffer does not support bg_reclaim, a success result
	 * is returned to prevent other processes from being affected
	 */
	ret = __op_get_valid_buffer_and_info(dmabuf, true, &buffer, &buffer_info);
	if (ret)
		return 0;

	ret = down_write_killable(&buffer_info->rwsem);
	if (ret) {
		bg_buf_pr_info("%s down-ret-%d, exit\n", __func__, ret);
		return ret;
	}

	org_pin_state = atomic_read(&buffer_info->pin_state);
	if (org_pin_state == BG_BUFFER_IN_RECLAIM) {
		bg_buf_pr_err("%s bg_buffer already in reclaim\n", __func__);
		ret = -ENOEXEC;
		goto out;
	}

	atomic_set(&buffer_info->pin_state, new_pin_state);
	bg_buf_pr_debug("%s bg_buffer pin_state = %d\n",
		__func__, atomic_read(&buffer_info->pin_state));
out:
	up_write(&buffer_info->rwsem);
	return ret;
}

/*
 * bg_buffer_pin_reclaim - pin dmabuf which cannot be released by bg_reclaim.
 * @dmabuf: buffer which not allow to exec bg_reclaim
 *
 * When bg_buffer_pin_reclaim success return 0, otherwise return errno
 *
 * Attention:
 * Before calling this function, you must call dma_buf_get to
 * prevent the buffer released
 */
int bg_buffer_pin_reclaim(struct dma_buf *dmabuf)
{
	int ret;

	ret = __bg_buf_set_pin_state(dmabuf, BG_BUFFER_RECLAIM_PINNED);
	if (ret)
		bg_buf_pr_err("%s __bg_buf_set_pin_state failed, ret=%d\n", __func__, ret);

	return ret;
}

/*
 * bg_buffer_unpin_reclaim - unpin dmabuf allows reclaim to be executed.
 * @dmabuf: buffer which to unpin
 *
 * When bg_buffer_unpin_reclaim success return 0, otherwise return errno
 *
 * Attention:
 * Before calling this function, you must call dma_buf_get to
 * prevent the buffer released
 */
int bg_buffer_unpin_reclaim(struct dma_buf *dmabuf)
{
	int ret;

	ret = __bg_buf_set_pin_state(dmabuf, BG_BUFFER_OUT_RECLAIM);
	if (ret)
		bg_buf_pr_err("%s __bg_buf_set_pin_state failed, ret=%d\n", __func__, ret);

	return ret;
}

/*
 * bg_op_reclaim_buffer - reclaim dmabuf memory without free va-region
 * @dmabuf: buffer which to free memory
 *
 * When reclaim success return 0, otherwise return errno
 *
 * Attention:
 * 	[1] Before calling this function, you must call dma_buf_get to
 *	prevent the buffer released
 * 	[2] After this function is called, the CPU or device is not allowed to
 *	access the VA related to dmabuf
 *	[3] When the dmabuf is mapped through the SMMU, only
 *	the kernel_iommu_map/unmap_dmabuf can be used. If other interfaces
 *	are used, this dmabuf cannot be bg_reclaim
 */
int bg_op_reclaim_buffer(struct dma_buf *dmabuf)
{
	void *buffer = NULL;
	struct bg_dmabuf_info *buffer_info = NULL;
	struct bg_op_time_stamp t = {0};
	int ret;
	int result = 0;

	t._stime = ktime_get();
	/*
	 * If the buffer does not support bg_reclaim,
	 * a success result is returned to prevent
	 * other processes from being affected
	 */
	ret = __op_get_valid_buffer_and_info(dmabuf, true, &buffer, &buffer_info);
	if (ret)
		return 0;

	ret = down_write_killable(&buffer_info->rwsem);
	if (ret) {
		bg_buf_pr_info("%s down-ret-%d, exit\n", __func__, ret);
		return 0;
	}

	ret = __bg_buf_set_in_reclaim(buffer_info);
	if (ret) {
		bg_buf_pr_err("%s pin_state already pinned\n", __func__);
		result = ret;
		goto buf_released;
	}

	t._sem_time = ktime_get();

	if (is_bg_dmabuf_release(dmabuf))
		goto buf_released;

	ret = __bg_buf_device_unmap(buffer, dmabuf, buffer_info, &t);
	if (ret) {
		if (ret != -ENOEXEC)
			result = -EFAULT;
		goto out_recliam;
	}

	buffer_lock(buffer);
	t._lock_time = ktime_get();

	/* we need check buf stat because before check is non-lock ops */
	if (__bg_check_buf_stat(buffer_info, true))
		goto out_buffer_unlock;

	atomic_set(&buffer_info->is_release, 1);
	ret = __bg_buf_cpu_unmap(buffer, dmabuf, buffer_info, &t);
	if (ret) {
		atomic_set(&buffer_info->is_release, 0);
		/* when map exec error, we should return -EFAULT to caller */
		if (ret != -ENOEXEC)
			result = -EFAULT;
		goto out_buffer_unlock;
	}

	__buffer_reclaim_memory(buffer);
	t._mem_time = ktime_get();
	buffer_unlock(buffer);

	up_write(&buffer_info->rwsem);
	__print_reclaim_cost(buffer, &t);
	return 0;

out_buffer_unlock:
	buffer_unlock(buffer);
	ret = __bg_buf_device_map(buffer, dmabuf, buffer_info, &t);
	if (ret)
		result = ret;
out_recliam:
	__bg_buf_set_out_reclaim(buffer_info);
buf_released:
	up_write(&buffer_info->rwsem);
	return result;
}

/*
 * bg_op_realloc_buffer - realloc dmabuf memory which has been bg_reclaim
 * @dmabuf: buffer which to realloc memory
 *
 * When realloc success return 0, otherwise return errno
 *
 * Attention:
 * 	[1] Before calling this function, you must call dma_buf_get to
 *	prevent the buffer released
 * 	[2] If the function fails to be executed, the va-region related to
 *	dmabuf may not be accessed
 *	[3] When the dmabuf is mapped through the SMMU, only
 *	the kernel_iommu_map/unmap_dmabuf can be used. If other interfaces
 *	are used, this dmabuf cannot be bg_realloc
 */
int bg_op_realloc_buffer(struct dma_buf *dmabuf)
{
	void *buffer = NULL;
	struct bg_dmabuf_info *buffer_info = NULL;
	struct bg_op_time_stamp t = {0};
	int ret = 0;
	int result = 0;

	/*
	 * If the buffer does not support bg_realloc,
	 * a success result is returned to prevent
	 * other processes from being affected
	 */
	ret = __op_get_valid_buffer_and_info(dmabuf, true, &buffer, &buffer_info);
	if (ret)
		return 0;

	t._stime = ktime_get();
	ret = down_write_killable(&buffer_info->rwsem);
	if (ret) {
		bg_buf_pr_info("%s down-ret-%d, exit\n", __func__, ret);
		return -EFAULT;
	}
	t._sem_time = ktime_get();

	buffer_lock(buffer);
	t._lock_time = ktime_get();

	if (__bg_check_buf_stat(buffer_info, false))
		goto buffer_unlock;

	ret = __buffer_realloc_memory(buffer);
	if (ret) {
		result = -ENOMEM;
		goto buffer_unlock;
	}
	t._mem_time = ktime_get();
	/*
	 * map as much as we can in case some undefined access, so we don't check result
	 */
	ret = __bg_buf_cpu_map(buffer, dmabuf, buffer_info, &t);
	if (ret)
		result = ret;

	atomic_set(&buffer_info->is_release, 0);
	buffer_unlock(buffer);

	ret = __bg_buf_device_map(buffer, dmabuf, buffer_info, &t);
	if (ret)
		result = ret;
	__print_realloc_cost(buffer, &t);
	goto out;

buffer_unlock:
	buffer_unlock(buffer);
out:
	__bg_buf_set_out_reclaim(buffer_info);
	up_write(&buffer_info->rwsem);
	return result;
}

void register_bg_buffer_operation(struct kobject *kobj)
{
	/* init bg buffer internal ops */
	internal_buffer_init();

	register_bg_buffer_test(kobj);
}

