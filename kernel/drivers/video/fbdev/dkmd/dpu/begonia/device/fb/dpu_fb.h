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

#ifndef DPU_FB_H
#define DPU_FB_H

#include <linux/types.h>
#include "dkmd_comp.h"
#include "gfxdev_pandisplay_utils.h"

/**
 * @brief private data for fb device
 * If you want to add some private data, can be added to this,
 * rather than adding to the composer's public data structure
 * such as 'fb_mem_acquired' maybe not needed in other composer
 * device
 */
struct device_fb {
	uint32_t index;
	atomic_t ref_cnt;
	bool fb_mem_acquired;
	uint32_t bpp;

	struct fb_info *fbi_info;
	struct sg_table *fb_sg_table;
	const struct dkmd_object_info *pinfo;

	struct composer *composer;
};

struct composer *get_comp_from_fb_device(struct device *dev);

int32_t fb_device_register(struct composer *comp);
void fb_device_unregister(struct composer *comp);
void fb_device_shutdown(struct composer *comp);

#endif