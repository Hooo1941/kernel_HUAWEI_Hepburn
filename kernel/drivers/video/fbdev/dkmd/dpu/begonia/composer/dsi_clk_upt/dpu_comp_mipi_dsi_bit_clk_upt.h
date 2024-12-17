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

#ifndef __MIPI_DSI_BIT_CLK_UPT_H__
#define __MIPI_DSI_BIT_CLK_UPT_H__
#include "dpu_comp_mgr.h"

#define MIPI_DSI_BIT_CLK_STR1 "00001"
#define MIPI_DSI_BIT_CLK_STR2 "00010"
#define MIPI_DSI_BIT_CLK_STR3 "00100"
#define MIPI_DSI_BIT_CLK_STR4 "01000"
#define MIPI_DSI_BIT_CLK_STR5 "10000"

#define MIPI_CLK_UPDT_TIMEOUT 30

void dpu_comp_mipi_dsi_bit_clk_upt_init(struct dpu_composer *dpu_comp);
#endif