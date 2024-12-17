/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: IRC switch control interface.
 */

#ifndef __LCD_KIT_PANEL_TEMP_COMPENSATION_
#define __LCD_KIT_PANEL_TEMP_COMPENSATION_

#include <linux/kernel.h>

#include "lcd_kit_common.h"

int display_engine_set_param_panel_temper(int panel_id, bool panel_temper_enable);

#endif