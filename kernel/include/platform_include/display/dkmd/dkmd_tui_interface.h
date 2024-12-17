/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
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

#ifndef DKMD_TUI_INTERFACE_H
#define DKMD_TUI_INTERFACE_H

#include "dkmd_dpu.h"
#include <linux/types.h>

enum tui_rotate_type {
	TUI_DISPLAY_ROT_0 = 0,
	TUI_DISPLAY_ROT_180 = 1
};

#ifdef CONFIG_TEE_TUI
bool dpu_tui_is_power_on(void);
int32_t dpu_tui_get_active_display_rect(struct dkmd_rect *active_rect);
int32_t dpu_tui_get_rotation(void);
#else
inline bool dpu_tui_is_power_on(void)
{
	return false;
}
inline int32_t dpu_tui_get_active_display_rect(struct dkmd_rect *active_rect)
{
	return 0;
}
inline int32_t dpu_tui_get_rotation(void)
{
	return 0;
}
#endif

#endif /* DKMD_TUI_INTERFACE_H */
