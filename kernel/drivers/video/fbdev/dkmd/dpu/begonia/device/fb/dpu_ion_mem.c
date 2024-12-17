/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <asm/cacheflush.h>
#include "dpu_ion_mem.h"
#include "dpu_fb.h"
#include "res_mgr.h"
#include "dkmd_log.h"
#include "gfxdev_utils.h"

void *dpu_iommu_map_kernel(struct sg_table *sg_table, size_t size)
{
	void *vaddr = NULL;
	struct scatterlist *sg = NULL;
	int32_t npages = (int32_t)(PAGE_ALIGN(size) / PAGE_SIZE);
	struct page **pages = vmalloc(sizeof(struct page *) * npages);
	struct page **tmp = pages;
	struct page *page = NULL;
	pgprot_t pgprot;
	int32_t npages_this_entry;
	uint32_t i;
	int32_t j;

	if (IS_ERR_OR_NULL(pages)) {
		dpu_pr_err("vmalloc failed");
		return NULL;
	}

	if (!sg_table) {
		dpu_pr_err("sg_table is NULL");
		vfree(pages);
		return NULL;
	}

	for_each_sg(sg_table->sgl, sg, sg_table->nents, i) {
		npages_this_entry = (int32_t)(PAGE_ALIGN(sg->length) / PAGE_SIZE);
		page = sg_page(sg);

		dpu_assert(i >= (uint32_t)npages);
		for (j = 0; j < npages_this_entry; j++)
			*(tmp++) = page++;
	}

	pgprot = pgprot_writecombine(PAGE_KERNEL);
	vaddr = vmap(pages, (uint32_t)npages, VM_MAP, pgprot);
	vfree(pages);
	if (!vaddr) {
		dpu_pr_err("vmap failed");
		return NULL;
	}

	return vaddr;
}

void dpu_iommu_unmap_kernel(const void *vaddr)
{
	vunmap(vaddr);
}

unsigned long dpu_alloc_fb_buffer(struct fb_info *info)
{
	struct sg_table *sg = NULL;
	struct device_fb *dpu_fb = NULL;
	struct composer *comp = NULL;
	unsigned long buf_addr;
	unsigned long buf_size;
	size_t buf_len;

	if (unlikely(!info)) {
		dpu_pr_err("fb_info is NULL");
		return 0;
	}

	comp = (struct composer *)info->par;
	if (unlikely(!comp)) {
		dpu_pr_err("fb_info's par is NULL");
		return 0;
	}

	dpu_fb = (struct device_fb *)comp->device_data;
	if (unlikely(!dpu_fb)) {
		dpu_pr_err("dpu_fb is null");
		return 0;
	}

	buf_len = info->fix.smem_len;  /* align to PAGE_SIZE */
	sg = dpu_dma_alloc_mem((uint32_t)buf_len);
	if (!sg) {
		dpu_pr_err("dpu_dma_alloc_mem failed!");
		return 0;
	}

	buf_addr = mm_iommu_map_sg(dpu_res_get_device(), sg->sgl, 0, &buf_size);
	if (buf_addr == 0) {
		dpu_pr_err("mm_iommu_map_sg failed!");
		dpu_dma_free_mem(sg);
		return 0;
	}
	dpu_pr_info("scene%d alloc framebuffer map sg 0x%zxB succuss", dpu_fb->pinfo->pipe_sw_itfch_idx, buf_size);

	info->screen_base = dpu_iommu_map_kernel(sg, buf_len);
	if (!info->screen_base) {
		dpu_pr_err("dpufb_iommu_map_kernel failed!");
		mm_iommu_unmap_sg(dpu_res_get_device(), sg->sgl, buf_addr);
		dpu_dma_free_mem(sg);
		return 0;
	}

	__inval_dcache_area(info->screen_base, (uint32_t)buf_len); /* before user using the physic addr, dcache the addr */

	info->fix.smem_start = buf_addr;
	info->screen_size = buf_len;
	dpu_fb->fb_sg_table = sg;
	dpu_fb->fb_mem_acquired = true;

	return buf_addr;
}

void dpu_free_fb_buffer(struct fb_info *info)
{
	struct device_fb *dpu_fb = NULL;
	struct composer *comp = NULL;

	if (unlikely(!info)) {
		dpu_pr_err("fb_info is NULL");
		return;
	}

	comp = (struct composer *)info->par;
	if (unlikely(!comp)) {
		dpu_pr_err("fb_info's par is NULL");
		return;
	}

	dpu_fb = (struct device_fb *)comp->device_data;
	if (unlikely(!dpu_fb)) {
		dpu_pr_err("dpu_fb is null");
		return;
	}

	if (likely(dpu_fb->fb_sg_table) && likely(info->screen_base)) {
		dpu_iommu_unmap_kernel(info->screen_base);
		mm_iommu_unmap_sg(dpu_res_get_device(), dpu_fb->fb_sg_table->sgl, info->fix.smem_start);
		dpu_dma_free_mem(dpu_fb->fb_sg_table);

		dpu_fb->fb_sg_table = NULL;
		info->screen_base = 0;
		info->fix.smem_start = 0;
		dpu_fb->fb_mem_acquired = false;
	}
}

static int32_t dpu_mmap_table(struct fb_info *info, struct vm_area_struct *vma)
{
	struct composer *comp = NULL;
	struct device_fb *dpu_fb = NULL;
	struct sg_table *table = NULL;

	if (unlikely(!info)) {
		dpu_pr_err("info is NULL");
		return -EINVAL;
	}
	comp = info->par;

	if (unlikely(!comp)) {
		dpu_pr_err("comp is NULL");
		return -EINVAL;
	}
	dpu_fb = (struct device_fb *)comp->device_data;

	if (unlikely(!dpu_fb)) {
		dpu_pr_err("dpu_fb is NULL");
		return -EINVAL;
	}
	table = dpu_fb->fb_sg_table;

	if (unlikely(!table)) {
		dpu_pr_err("table is NULL");
		return -EINVAL;
	}

	return gfxdev_mmap_table(table, &info->fix, vma);
}

int32_t dpu_fb_mmap(struct fb_info *info, struct vm_area_struct *vma)
{
	int32_t ret;
	struct device_fb *dpu_fb = NULL;
	struct composer *comp = NULL;

	dpu_pr_info("+");
	if (unlikely(!info)) {
		dpu_pr_err("fb_info is NULL");
		return -EINVAL;
	}

	if (unlikely(!vma)) {
		dpu_pr_err("vma is NULL");
		return -EINVAL;
	}

	comp = (struct composer *)info->par;
	if (unlikely(!comp)) {
		dpu_pr_err("fb_info's par is NULL");
		return -EINVAL;
	}

	dpu_fb = (struct device_fb *)comp->device_data;
	if (unlikely(!dpu_fb)) {
		dpu_pr_err("dpu_fb is null");
		return -EINVAL;
	}

	if (unlikely(lock_fb_info(info) == 0))
		return -ENODEV;

	if (!dpu_fb->fb_mem_acquired) {
		if (dpu_alloc_fb_buffer(info) == 0) {
			dpu_pr_err("dpu_alloc_fb_buffer failed!");
			ret = -ENOMEM;
			goto OUT;
		}
		dpu_fb->fb_mem_acquired = true;
	}

	ret = dpu_mmap_table(info, vma);

OUT:
	unlock_fb_info(info);
	return ret;
}

#ifdef CONFIG_FB_KERNEL_LOGO
int dpu_fb_buffer_alloc_kernel(struct fb_info *info)
{
	int ret = 0;
	if (info == NULL) {
		pr_info("PM: fb_info is null\n");
		return -EINVAL;
	}
 
	struct composer *comp = (struct composer *)info->par;
	if (unlikely(!comp)) {
		pr_info("PM: fb_info's par is NULL\n");
		return -EINVAL;
	}
 
	struct device_fb *dpu_fb = (struct device_fb *)comp->device_data;
	if (unlikely(!dpu_fb)) {
		pr_info("PM: dpu_fb is null\n");
		return -EINVAL;
	}
 
	if (unlikely(lock_fb_info(info) == 0))
		return -ENODEV;
 
	if (!dpu_fb->fb_mem_acquired) {
		if (dpu_alloc_fb_buffer(info) == 0) {
			pr_info("PM: dpu_alloc_fb_buffer failed!\n");
			ret = -ENOMEM;
			goto OUT;
		}
		dpu_fb->fb_mem_acquired = true;
	}
OUT:
	unlock_fb_info(info);
	return ret;
}
#endif
