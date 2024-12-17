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
#ifndef GFXDEV_UTILS_H
#define GFXDEV_UTILS_H
#include "dkmd_comp.h"
#include "gfxdev_pandisplay_utils.h"

int32_t dpu_gfxdev_create_fence(struct composer *comp, void __user *argp);
int32_t dpu_gfxdev_release_fence(struct composer *comp, void __user *argp);
int32_t dpu_gfxdev_blank(struct composer *comp, int32_t blank_mode);
int32_t dpu_gfxdev_present(struct composer *comp, const struct dkmd_object_info *pinfo, const void __user* argp);
int32_t dpu_gfxdev_set_safe_frm_rate(struct composer *comp, void __user *argp);
int32_t dpu_gfxdev_set_active_rect(struct composer *comp, void __user *argp);

int32_t gfxdev_mmap_table(struct sg_table *table, gfxdev_fix_screeninfo* fix, struct vm_area_struct *vma);
struct iommu_page_info *dpu_dma_create_node(void);
struct sg_table *dpu_dma_alloc_mem(unsigned size);
void dpu_dma_free_mem(struct sg_table *table);
bool dpu_gfxdev_set_fastboot(struct composer *comp);
#endif