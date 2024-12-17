/*
 * lcd_kit_fac.h
 *
 * lcdkit factory function for lcd driver
 *
 * Copyright (c) 2022-2024 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#ifndef _LCD_KIT_FAC_H_
#define _LCD_KIT_FAC_H_
#include "lcd_kit_common.h"

int lcd_factory_init(int panel_id, struct device_node *np);
#endif