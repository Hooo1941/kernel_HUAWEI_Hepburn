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

#ifndef __DKMD_DP_INTERFACE_H__
#define __DKMD_DP_INTERFACE_H__

#include <linux/types.h>

/* this struct expose for external include */

struct dp_hook_ops {
	int (*save_dp_edid)(uint8_t *edid_buf, uint32_t buf_len);
	bool (*is_valid_timing_info)(uint16_t hactive_pixels, uint16_t vactive_pixels, uint32_t fps);
};

#ifdef CONFIG_DKMD_DPU_DP
void dpu_dp_hook_register(struct dp_hook_ops *ops);
#else
inline void dpu_dp_hook_register(struct dp_hook_ops *ops)
{
	return;
}
#endif

#endif /* __DKMD_DP_INTERFACE_H__ */
