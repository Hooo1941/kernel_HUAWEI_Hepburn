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

#ifndef DKMD_FENCE_UTILS_H
#define DKMD_FENCE_UTILS_H

#include <linux/types.h>
#include <linux/dma-fence.h>

int32_t dkmd_fence_get_fence_fd(struct dma_fence *fence);
int32_t dkmd_fence_signal_fence(int32_t fence_fd);
#endif