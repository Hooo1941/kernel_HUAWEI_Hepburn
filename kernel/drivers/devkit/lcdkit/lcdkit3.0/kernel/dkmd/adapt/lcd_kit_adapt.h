/*
 * lcd_kit_adapt.h
 *
 * lcdkit adapt function for lcd driver head file
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

#ifndef __LCD_KIT_ADAPT_H_
#define __LCD_KIT_ADAPT_H_
#include <linux/types.h>
#include "lcd_kit_common.h"

#define GEN_VID_LP_CMD BIT(24)
#define READ_MAX 100
#define BUF_MAX (4 * READ_MAX)
#define BIT_NUM_PER_BYTE 8
#define RIGHT_SHIFT_BYTE_NUM 3
/* generic read/write */
#define DTYPE_GEN_WRITE  0x03 /* short write, 0 parameter */
#define DTYPE_GEN_WRITE1 0x13 /* short write, 1 parameter */
#define DTYPE_GEN_WRITE2 0x23 /* short write, 2 parameter */
#define DTYPE_GEN_LWRITE 0x29 /* long write */
#define DTYPE_GEN_READ   0x04 /* long read, 0 parameter */
#define DTYPE_GEN_READ1  0x14 /* long read, 1 parameter */
#define DTYPE_GEN_READ2  0x24 /* long read, 2 parameter */
/* dcs read/write */
#define DTYPE_DCS_WRITE  0x05 /* short write, 0 parameter */
#define DTYPE_DCS_WRITE1 0x15 /* short write, 1 parameter */
#define DTYPE_DCS_READ   0x06 /* read */
#define DTYPE_DCS_LWRITE 0x39 /* long write */
#define DTYPE_DSC_LWRITE 0x0A /* dsc dsi1.2 vase3x long write */

enum {
	NO_LOCK,
	LOCK,
};

int lcd_kit_adapt_init(void);
int lcd_kit_cmds_rx(int panel_id, void *hld, unsigned char *out, int out_len,
	struct lcd_kit_dsi_panel_cmds *cmds);
int lcd_kit_cmds_tx(int panel_id, void *hld, struct lcd_kit_dsi_panel_cmds *cmds);
int lcd_kit_dual_diff_cmds_tx(int panel_id, void *hld, struct lcd_kit_dsi_panel_cmds *cmds0,
	struct lcd_kit_dsi_panel_cmds *cmds1);
int lcd_kit_dual_cmds_rx(int panel_id, void *hld, unsigned char *out0, unsigned char *out1,
	int out_len, struct lcd_kit_dsi_panel_cmds *cmds);
int lcd_kit_dual_diff_cmds_rx(int panel_id, unsigned char *out0, unsigned char *out1,
	int out_len, struct lcd_kit_dsi_panel_cmds *cmds0, struct lcd_kit_dsi_panel_cmds *cmds1);
bool lcd_kit_is_dual_mipi_panel(int panel_id);
#endif
