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

#ifndef DPU_GFX_H
#define DPU_GFX_H

#include "dkmd_chrdev.h"
#include "dkmd_comp.h"
#include "gfxdev_pandisplay_utils.h"

struct device_gfx {
	uint32_t index;
	atomic_t ref_cnt;

	bool be_connected; /* for dp device */
	bool gfx_mem_acquired;

	struct dkmd_chrdev chrdev;
	struct sg_table *gfx_sg_table;
	gfxdev_fix_screeninfo gfx_fix;
	gfxdev_var_screeninfo gfx_var;

	/* for protected buffer mmap,
	 * one thread may is freeing the buffer, and another thread
	 * is using the buffer
	 */
	struct mutex lock;
	const struct dkmd_object_info *pinfo;
	struct composer *composer;
};

struct composer *get_comp_from_gfx_device(struct device *dev);
int32_t gfx_device_register(struct composer *comp);
void gfx_device_unregister(struct composer *comp);
#endif
