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

#ifndef __PANEL_DEV_H__
#define __PANEL_DEV_H__

#include <linux/types.h>
#include <linux/platform_device.h>

#include "panel_drv.h"

void panel_drv_data_setup(struct panel_drv_private *priv, struct device_node *np);
void panel_dev_data_setup(struct panel_drv_private *priv);
int32_t panel_probe_sub(struct panel_drv_private *priv);

extern struct panel_match_data nt35521_panel_info;
extern struct panel_match_data td4322_panel_info;
extern struct panel_match_data nt37700p_panel_info;
extern struct panel_match_data nt37800a_panel_info;
extern struct panel_match_data nt37290_panel_info;
extern struct panel_match_data nt37701_brq_panel_info;
extern struct panel_match_data nt37701a_panel_info;
extern struct panel_match_data rm69091_panel_info;
extern struct panel_match_data hx5293_panel_info;
extern struct panel_match_data nt36870_panel_info;
extern struct panel_match_data visionox310_panel_info;
extern struct panel_match_data boe7p847_panel_info;
extern struct panel_match_data visionox_6p39_panel_info;
extern struct panel_match_data boe_6p39_panel_info;
extern struct panel_match_data hx83121_panel_info;

#endif
