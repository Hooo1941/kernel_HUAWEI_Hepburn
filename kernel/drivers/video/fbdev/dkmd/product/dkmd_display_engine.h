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

#ifndef DKMD_DISPLAY_ENGINE_H
#define DKMD_DISPLAY_ENGINE_H

#if defined(CONFIG_LCD_KIT_DRIVER)
void dpu_display_engine_vsync_queue_work(void);

#else
#define dpu_display_engine_vsync_queue_work()
#endif

#endif
