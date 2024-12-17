/*
 * lcd_kit_sysfs_hs.h
 *
 * lcdkit sys node function for lcd driver head file
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

#ifndef _LCD_KIT_SYSFS_DKMD_H_
#define _LCD_KIT_SYSFS_DKMD_H_
#include "lcd_kit_common.h"

/* oem info */
#define OEM_INFO_SIZE_MAX  500
#define OEM_INFO_BLOCK_NUM 1
/* 2d barcode */
#define BARCODE_LENGTH    46
#define BARCODE_BLOCK_NUM 3
#define BARCODE_BLOCK_LEN 16
/* project id */
#define PROJECT_ID_LENGTH 10
#define FPS_ELEM_NUM 2
/* panel dieid */
#define PANEL_DIEID_INFO_SIZE_MAX 500
#define PANEL_MAX 10
#define LCD_REG_LENGTH_MAX 500
#define ORDER_DELAY 200
/* panel product type to 4 for recovery at GRL */
#define PRODUCT_TYPE_GREENLAND_RECOVERY 4

enum oem_type {
	PROJECT_ID_TYPE,
	BARCODE_2D_TYPE,
};

enum panel_num {
	SINGLE_PANEL = 1,
	DUAL_PANELS = 2,
};

/* struct */
struct oem_info_cmd {
	unsigned char type;
	int (*func)(int panel_id, char *oem_data);
};

int lcd_kit_sysfs_init(struct device *dev);
#endif