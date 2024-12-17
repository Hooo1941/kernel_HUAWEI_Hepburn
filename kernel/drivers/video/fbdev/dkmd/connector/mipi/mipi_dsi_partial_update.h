/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
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

#ifndef __MIPI_DSI_PARTIAL_UPDATE_H__
#define __MIPI_DSI_PARTIAL_UPDATE_H__

struct dpu_connector;

int32_t mipi_dsi_partial_update(struct dpu_connector *connector, const void *value);
int32_t mipi_dsi_reset_partial_update(struct dpu_connector *connector, const void *value);
int32_t mipi_dsi_panel_partial_ctrl_set_reg(struct dpu_connector *connector, const void *value);
#endif
