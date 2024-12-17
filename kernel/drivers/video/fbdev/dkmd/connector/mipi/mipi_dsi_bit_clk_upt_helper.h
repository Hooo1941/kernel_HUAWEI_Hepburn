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

#ifndef __MIPI_DSI_BIT_CLK_UPT_HELPER_H__
#define __MIPI_DSI_BIT_CLK_UPT_HELPER_H__

struct dpu_connector;

void mipi_dsi_bit_clk_upt_isr_handler(struct dpu_connector *connector);
int mipi_dsi_bit_clk_upt(struct dpu_connector *connector, const void *value);
int wait_for_mipi_resource_available(struct dpu_connector* connector);
#endif