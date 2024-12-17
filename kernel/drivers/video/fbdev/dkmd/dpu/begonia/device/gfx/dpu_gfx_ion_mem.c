/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
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

#include <linux/mutex.h>
#include "dpu_ion_mem.h"
#include "res_mgr.h"
#include "dkmd_log.h"
#include "dpu_gfx.h"
#include "gfxdev_utils.h"

static unsigned long dpu_alloc_gfx_buffer(struct composer *comp)
{
	struct sg_table *sg = NULL;
	struct device_gfx *gfx_dev = NULL;
	gfxdev_fix_screeninfo *fix;
	unsigned long buf_addr;
	unsigned long buf_size;
	size_t buf_len;

	if (unlikely(!comp)) {
		dpu_pr_err("comp's par is NULL");
		return 0;
	}

	gfx_dev = (struct device_gfx *)comp->device_data;
	if (unlikely(!gfx_dev)) {
		dpu_pr_err("gfx_dev is null");
		return 0;
	}

	fix = &gfx_dev->gfx_fix;
	if (unlikely(!fix)) {
		dpu_pr_err("fix is null");
		return 0;
	}

	buf_len = fix->smem_len;  /* align to PAGE_SIZE */
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
	dpu_pr_info("scene%d alloc framebuffer map sg 0x%zxB succuss", gfx_dev->pinfo->pipe_sw_itfch_idx, buf_size);

	fix->smem_start = buf_addr;

	gfx_dev->gfx_sg_table = sg;
	gfx_dev->gfx_mem_acquired = true;

	return buf_addr;
}

void dpu_free_gfx_buffer(struct composer *comp)
{
	struct device_gfx *gfx_dev;

	if (unlikely(!comp)) {
		dpu_pr_err("comp is null");
		return;
	}

	gfx_dev = (struct device_gfx *)comp->device_data;
	if (unlikely(!gfx_dev)) {
		dpu_pr_err("gfx_dev is null");
		return;
	}

	mutex_lock(&gfx_dev->lock);
	if (gfx_dev->gfx_sg_table == NULL) {
		dpu_pr_warn("not need unmap");
		mutex_unlock(&gfx_dev->lock);
		return;
	}

	mm_iommu_unmap_sg(dpu_res_get_device(), gfx_dev->gfx_sg_table->sgl, gfx_dev->gfx_fix.smem_start);
	dpu_dma_free_mem(gfx_dev->gfx_sg_table);
	gfx_dev->gfx_sg_table = NULL;
	gfx_dev->gfx_mem_acquired = false;
	gfx_dev->gfx_fix.smem_start = 0;

	mutex_unlock(&gfx_dev->lock);
	return;
}

static int32_t dpu_mmap_table(struct composer *comp, struct vm_area_struct *vma)
{
	struct device_gfx *gfx_dev = (struct device_gfx *)comp->device_data;
	struct sg_table *table = gfx_dev->gfx_sg_table;

	return gfxdev_mmap_table(table, &gfx_dev->gfx_fix, vma);
}

int32_t dpu_gfx_mmap(struct file *filp, struct vm_area_struct *vma)
{
	int32_t ret;
	struct device_gfx *gfx_dev = NULL;
	struct composer *comp = NULL;

	dpu_pr_info("+");
	if (unlikely(!filp)) {
		dpu_pr_err("filp is null");
		return -EINVAL;
	}

	gfx_dev = (struct device_gfx *)filp->private_data;
	if (unlikely(!gfx_dev)) {
		dpu_pr_err("gfx_dev is null");
		return -EINVAL;
	}

	comp = gfx_dev->composer;
	if (unlikely(!comp)) {
		dpu_pr_err("composer is NULL");
		return -EINVAL;
	}
	mutex_lock(&gfx_dev->lock);

	if (!gfx_dev->gfx_mem_acquired) {
		if (dpu_alloc_gfx_buffer(comp) == 0) {
			dpu_pr_err("dpu alloc gfx buffer failed!");
			ret = -ENOMEM;
			goto OUT;
		}
		gfx_dev->gfx_mem_acquired = true;
	}

	ret = dpu_mmap_table(comp, vma);

OUT:
	mutex_unlock(&gfx_dev->lock);
	return ret;
}
