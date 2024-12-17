/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: IRC switch control interface.
 */

#ifndef __LCD_KIT_PANEL_OD__
#define __LCD_KIT_PANEL_OD__

#include <linux/kernel.h>

#include "lcd_kit_common.h"

void lcd_kit_parse_panel_od(int panel_id, struct device_node *np);
int display_engine_set_param_panel_od(int panel_id, bool panel_od_enable);

#endif