 /* Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
  *
  * This program is free software; you can redistribute it and/or modify
  * it under the terms of the GNU General Public License version 2 and
  * only version 2 as published by the Free Software Foundation.
  *
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  *
  */

#include "dpu_conn_mgr.h"
#include "panel_mgr.h"
#include "panel_drv.h"
#include "dsc_output_calc.h"

#define DSC_1_1_EN 1

#ifdef DSC_1_1_EN
static char g_cmd_1[] = {
	0xB9,
	0x83,  0x12, 0x1A, 0x55, 0x00,
};

static char g_cmd_2[] = {
	0xBD,
	0x00,
};

static char g_cmd_3[] = {
	0x51,
	0x08, 0x00,
};

static char g_cmd_4[] = {
	0x53,
	0x24,
};

static char g_cmd_5[] = {
	0xB1, 0x1C, 0x6B, 0x6B, 0x27, 0xE7, 0x00, 0x1B, 0x25,
	0x21, 0x21, 0x2D, 0x2D, 0x17, 0x33, 0x31, 0x40, 0xCD, 0xFF, 0x1A, 0x05, 0x15, 0x98, 0x00,
	0x88, 0x7F, 0xFF, 0xFF, 0xCF, 0x1A, 0xCC, 0x02, 0x00,
};

static char g_cmd_6[] = {
	0xD1,
	0x37, 0x03, 0x0C, 0xFD,
};

static char g_cmd_7[] = {
	0xB2,
	0x00 ,0x6A, 0x40, 0x00, 0x00, 0x14, 0x98, 0x60, 0x3C, 0x02, 0x80, 0x21, 0x21,
	0x00, 0x00, 0xF0, 0x27,
};

static char g_cmd_8[] = {
	0xE2,
	0x10,
};

static char g_cmd_9[] = {
	0xC0, 0x23, 0x23, 0xCC, 0x22, 0x99, 0xD8,
};

static char g_cmd_10[] = {
	0xB4, 0x46, 0x06, 0x0C, 0xBE, 0x0C, 0xBE, 0x09, 0x46, 0x0F, 0x57, 0x0F, 0x57, 0x03,
	0x4A, 0x00, 0x00, 0x04, 0x0C, 0x00, 0x18, 0x01, 0x06, 0x08, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0xFF, 0x10, 0x00, 0x02, 0x14,
	0x14, 0x14, 0x14,
};

static char g_cmd_11[] = {
	0xBD,
	0x03,
};

static char g_cmd_12[] = {
	0xE1,
	0x01, 0x3F,
};

static char g_cmd_13[] = {
	0xBD,
	0x00,
};

static char g_cmd_14[] = {
	0xE9,
	0xE2,
};

static char g_cmd_15[] = {
	0xE7,
	0x49,
};

static char g_cmd_16[] = {
	0xE9,
	0x3F,
};

static char g_cmd_17[] = {
	0xD3,
	0x00, 0xC0, 0x08, 0x08, 0x08, 0x04, 0x04, 0x04, 0x16, 0x02, 0x07, 0x07, 0x07, 0x31, 0x13, 0x19, 0x12, 0x12,
	0x03, 0x03, 0x03, 0x32, 0x10, 0x18, 0x00, 0x11, 0x32, 0x10, 0x03, 0x00, 0x03, 0x32, 0x10, 0x03, 0x00, 0x03,
	0x00, 0x00, 0xFF, 0x00,
};

static char g_cmd_18[] = {
	0xE1, 0x11, 0x00, 0x00, 0x89, 0x30, 0x80, 0x0A, 0x00, 0x03, 0x20, 0x00, 0x14, 0x03, 0x20, 0x03, 0x20,
	0x02, 0x00, 0x02, 0x91, 0x00, 0x20, 0x02, 0x47, 0x00, 0x0B, 0x00, 0x0C, 0x05, 0x0E, 0x03, 0x68, 0x18,
	0x00, 0x10, 0xE0, 0x03, 0x0C, 0x20, 0x00, 0x06, 0x0B, 0x0B, 0x33, 0x0E, 0x1C, 0x2A, 0x38, 0x46, 0x54,
	0x62, 0x69, 0x70, 0x77, 0x79, 0x7B, 0x7D, 0x7E, 0x01, 0x02, 0x01, 0x00, 0x09,
};

static char g_cmd_19[] = {
	0xE7, 0x17, 0x08, 0x08, 0x2C, 0x46, 0x1E, 0x02, 0x23, 0x5D, 0x02, 0xC9, 0x00, 0x00, 0x00,
	0x00, 0x12, 0x05, 0x02, 0x02, 0x07, 0x10, 0x10, 0x00, 0x1D, 0xB9, 0x23, 0xB9, 0x00, 0x33, 0x02, 0x88,
};

static char g_cmd_20[] = {
	0xBD,
	0x01,
};

static char g_cmd_21[] = {
	0xE7,
	0x02, 0x00, 0xB2, 0x01, 0x56, 0x07, 0x56, 0x08, 0x48, 0x14, 0xFD, 0x26,
};

static char g_cmd_22[] = {
	0xBD,
	0x02,
};

static char g_cmd_23[] = {
	0xE7, 0x08, 0x08, 0x01, 0x03, 0x01, 0x03, 0x07, 0x02, 0x02, 0x47, 0x00,
	0x47, 0x81, 0x02, 0x40, 0x00, 0x18, 0x4A, 0x06, 0x05, 0x04, 0x03, 0x02,
	0x01, 0x00, 0x00, 0x03, 0x02, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static char g_cmd_24[] = {
	0xBD,
	0x00,
};

static char g_cmd_25[] = {
	0xBF, 0xFD, 0x00, 0x80, 0x9C, 0x36, 0x00, 0x81, 0x0C,
};

static char g_cmd_26[] = {
	0xCD,
	0x81, 0x00, 0x80, 0x77, 0x00, 0x01, 0x00,
};

static char g_cmd_27[] = {
	0xBD,
	0x01,
};

static char g_cmd_28[] = {
	0xE4,
	0xE1, 0xE1, 0xE1, 0xE1, 0xE1, 0xE1, 0xE1, 0xE1, 0xC7, 0xB2, 0xA0, 0x90, 0x81, 0x75, 0x69, 0x5F, 0x55, 0x4C, 0x44,
	0x3D, 0x36, 0x2F, 0x2A, 0x24, 0x1E, 0x19, 0x14, 0x10, 0x09, 0x08, 0x07, 0x54, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
	0x55,
};

static char g_cmd_29[] = {
	0xBD,
	0x03,
};

static char g_cmd_30[] = {
	0xE4,
	0xAA, 0xD4, 0xFF, 0x2A, 0x55, 0x7F, 0xAA, 0xD4, 0xFF, 0xEA, 0xFF, 0x03,
};

static char g_cmd_31[] = {
	0xBD,
	0x00,
};

static char g_cmd_32[] = {
	0xBE, 0x01, 0x35, 0x00,
};

static char g_cmd_33[] = {
	0xD9,
	0x5F,
};

static char g_cmd_test_0[] = {
	0xBD,
	0x00,
};

static char g_cmd_test_1[] = {
	0xE9,
	0xC2,
};

static char g_cmd_test_2[] = {
	0xBA,
	0x83,
};

static char g_cmd_test_3[] = {
	0xE9,
	0x3F,
};

static char g_cmd_test_4[] = {
	0xE9,
	0xC7,
};

static char g_cmd_test_5[] = {
	0xBA,
	0xFF, 0x3F,
};

static char g_cmd_test_6[] = {
	0xE9,
	0x3F,
};

static char g_cmd_37[] = {
	0x11,
};

static char g_cmd_38[] = {
	0x29,
};

static char g_cmd_39[] = {
	0x51,
	0x04, 0xFF,
};

static char g_cmd_40[] = {
	0x51,
	0x01,
};

static char g_cmd_41[] = {
	0x53,
	0x24,
};

static struct dsi_cmd_desc g_lcd_display_on_cmds[] = {
	{DTYPE_DCS_LWRITE, 0,10, WAIT_TYPE_US, sizeof(g_cmd_1), g_cmd_1},
	{DTYPE_DCS_LWRITE, 0,10, WAIT_TYPE_US, sizeof(g_cmd_2), g_cmd_2},
	{DTYPE_DCS_LWRITE, 0,10, WAIT_TYPE_US, sizeof(g_cmd_3), g_cmd_3},
	{DTYPE_DCS_LWRITE, 0,10, WAIT_TYPE_US, sizeof(g_cmd_4), g_cmd_4},
	{DTYPE_DCS_LWRITE, 0,10, WAIT_TYPE_US, sizeof(g_cmd_5), g_cmd_5},
	{DTYPE_DCS_LWRITE, 0,10, WAIT_TYPE_US, sizeof(g_cmd_6), g_cmd_6},
	{DTYPE_DCS_LWRITE, 0,10, WAIT_TYPE_US, sizeof(g_cmd_7), g_cmd_7},
	{DTYPE_DCS_LWRITE, 0,10, WAIT_TYPE_US, sizeof(g_cmd_8), g_cmd_8},
	{DTYPE_DCS_LWRITE, 0,10, WAIT_TYPE_US, sizeof(g_cmd_9), g_cmd_9},
	{DTYPE_DCS_LWRITE, 0,10, WAIT_TYPE_US, sizeof(g_cmd_10), g_cmd_10},
	{DTYPE_DCS_LWRITE, 0,10, WAIT_TYPE_US, sizeof(g_cmd_11), g_cmd_11},
	{DTYPE_DCS_LWRITE, 0,10, WAIT_TYPE_US, sizeof(g_cmd_12), g_cmd_12},
	{DTYPE_DCS_LWRITE, 0,10, WAIT_TYPE_US, sizeof(g_cmd_13), g_cmd_13},
	{DTYPE_DCS_LWRITE, 0,10, WAIT_TYPE_US, sizeof(g_cmd_14), g_cmd_14},
	{DTYPE_DCS_LWRITE, 0,10, WAIT_TYPE_US, sizeof(g_cmd_15), g_cmd_15},
	{DTYPE_DCS_LWRITE, 0,10, WAIT_TYPE_US, sizeof(g_cmd_16), g_cmd_16},
	{DTYPE_DCS_LWRITE, 0,10, WAIT_TYPE_US, sizeof(g_cmd_17), g_cmd_17},
	{DTYPE_DCS_LWRITE, 0,10, WAIT_TYPE_US, sizeof(g_cmd_18), g_cmd_18},
	{DTYPE_DCS_LWRITE, 0,10, WAIT_TYPE_US, sizeof(g_cmd_19), g_cmd_19},
	{DTYPE_DCS_LWRITE, 0,10, WAIT_TYPE_US, sizeof(g_cmd_20), g_cmd_20},
	{DTYPE_DCS_LWRITE, 0,10, WAIT_TYPE_US, sizeof(g_cmd_21), g_cmd_21},

	{DTYPE_DCS_LWRITE, 0,10, WAIT_TYPE_US, sizeof(g_cmd_22), g_cmd_22},
	{DTYPE_DCS_LWRITE, 0,10, WAIT_TYPE_US, sizeof(g_cmd_23), g_cmd_23},
	{DTYPE_DCS_LWRITE, 0,10, WAIT_TYPE_US, sizeof(g_cmd_24), g_cmd_24},
	{DTYPE_DCS_LWRITE, 0,10, WAIT_TYPE_US, sizeof(g_cmd_25), g_cmd_25},
	{DTYPE_DCS_LWRITE, 0,10, WAIT_TYPE_US, sizeof(g_cmd_26), g_cmd_26},
	{DTYPE_DCS_LWRITE, 0,10, WAIT_TYPE_US, sizeof(g_cmd_27), g_cmd_27},

	{DTYPE_DCS_LWRITE, 0,10, WAIT_TYPE_US, sizeof(g_cmd_28), g_cmd_28},
	{DTYPE_DCS_LWRITE, 0,10, WAIT_TYPE_US, sizeof(g_cmd_29), g_cmd_29},
	{DTYPE_DCS_LWRITE, 0,10, WAIT_TYPE_US, sizeof(g_cmd_30), g_cmd_30},
	{DTYPE_DCS_LWRITE, 0,10, WAIT_TYPE_US, sizeof(g_cmd_31), g_cmd_31},
	{DTYPE_DCS_LWRITE, 0,10, WAIT_TYPE_US, sizeof(g_cmd_32), g_cmd_32},

	{DTYPE_DCS_LWRITE, 0,10, WAIT_TYPE_US, sizeof(g_cmd_33), g_cmd_33},

	{DTYPE_DCS_LWRITE, 0,10, WAIT_TYPE_US, sizeof(g_cmd_test_0), g_cmd_test_0},
	{DTYPE_DCS_LWRITE, 0,10, WAIT_TYPE_US, sizeof(g_cmd_test_1), g_cmd_test_1},
	{DTYPE_DCS_LWRITE, 0,10, WAIT_TYPE_US, sizeof(g_cmd_test_2), g_cmd_test_2},
	{DTYPE_DCS_LWRITE, 0,10, WAIT_TYPE_US, sizeof(g_cmd_test_3), g_cmd_test_3},
	{DTYPE_DCS_LWRITE, 0,10, WAIT_TYPE_US, sizeof(g_cmd_test_4), g_cmd_test_4},
	{DTYPE_DCS_LWRITE, 0,10, WAIT_TYPE_US, sizeof(g_cmd_test_5), g_cmd_test_5},
	{DTYPE_DCS_LWRITE, 0,10, WAIT_TYPE_US, sizeof(g_cmd_test_6), g_cmd_test_6},

	{DTYPE_DCS_WRITE, 0,150, WAIT_TYPE_MS, sizeof(g_cmd_37), g_cmd_37},
	{DTYPE_DCS_WRITE, 0,10, WAIT_TYPE_US, sizeof(g_cmd_38), g_cmd_38},
	{DTYPE_DCS_LWRITE, 0,10, WAIT_TYPE_US, sizeof(g_cmd_39), g_cmd_39},
	{DTYPE_DCS_LWRITE, 0,10, WAIT_TYPE_US, sizeof(g_cmd_40), g_cmd_40},
	{DTYPE_DCS_LWRITE, 0,21, WAIT_TYPE_MS, sizeof(g_cmd_41), g_cmd_41},

};

static char g_display_off[] = {
	0x10,
};

static struct dsi_cmd_desc g_lcd_display_off_cmds[] = {
	{DTYPE_DCS_WRITE, 0, 128, WAIT_TYPE_MS, sizeof(g_display_off), g_display_off},
};

#else
static char g_cmd_1[] = {
	0xb9,
	0x83,  0x12, 0x1A, 0x55, 0x00,
};

static char g_cmd_2[] = {
	0x51,
	0x08, 0x00,
};

static char g_cmd_3[] = {
	0x53,
	0x24,
};

static char g_cmd_4[] = {
	0xBD,
	0x00,
};

static char g_cmd_5[] = {
	0xB1,
	0x1C, 0x6B, 0x6B, 0x27, 0xE7, 0x00, 0x1B, 0x11, 0x21, 0x21, 0x2D, 0x2D, 0x17,
	0x33, 0x31, 0x40, 0xCD, 0xFF, 0x1A, 0x05, 0x15, 0x98, 0x00, 0x88, 0x7F, 0xFF,
	0xFF, 0xCF, 0x1A, 0xCC, 0x02, 0x00,
};

static char g_cmd_6[] = {
	0xD1,
	0x37, 0x03, 0x0C, 0xFD,
};

static char g_cmd_7[] = {
	0xE2,
	0x20,
};

static char g_cmd_8[] = {
	0xB2,
	0x00 ,0x6A, 0x40, 0x00, 0x00, 0x14, 0x98, 0x60, 0x3C, 0x02, 0x80, 0x21, 0x21,
	0x00, 0x00, 0x10, 0x27,
};

static char g_cmd_9[] = {
	0xBD,
	0x03,
};

static char g_cmd_10[] = {
	0xE1,
	0x00, 0x3F,
};

static char g_cmd_11[] = {
	0xBD,
	0x00,
};

static char g_cmd_12[] = {
	0xE9,
	0xE2,
};

static char g_cmd_13[] = {
	0xE7,
	0x49,
};

static char g_cmd_14[] = {
	0xE9,
	0x3F,
};

static char g_cmd_15[] = {
	0xD3,
	0x00, 0xC0, 0x08, 0x08, 0x08, 0x04, 0x04, 0x04, 0x16, 0x02, 0x07, 0x07, 0x07, 0x31, 0x13, 0x16, 0x12, 0x12,
	0x03, 0x03, 0x03, 0x32, 0x10, 0x15, 0x00, 0x11, 0x32, 0x10, 0x03, 0x00, 0x03, 0x32, 0x10, 0x03, 0x00, 0x03,
	0x00, 0x00, 0xFF, 0x00,
};

static char g_cmd_16[] = {
	0xBD,
	0x02,
};

static char g_cmd_17[] = {
	0xE2,
	0x80, 0x05, 0x1C, 0xBE, 0x09, 0x8D, 0x0F, 0x57, 0x03, 0x87, 0x06, 0x10, 0x32, 0x06, 0x15, 0x00, 0x00, 0x14,
	0x14, 0x14, 0x14, 0x00, 0x00, 0x00, 0x01, 0x10, 0x10, 0x16, 0x28, 0x3C, 0x03, 0x23, 0x5D, 0x02, 0x02, 0x00,
	0x00, 0x48, 0x01, 0xAC, 0x0F, 0xAB, 0x10, 0x00, 0x32, 0x87, 0x00, 0xA1, 0x00, 0x0A, 0xCB, 0x00,
};

static char g_cmd_18[] = {
	0xBD,
	0x01,
};

static char g_cmd_19[] = {
	0xE7,
	0x02, 0x00, 0xB2, 0x01, 0x56, 0x07, 0x56, 0x08, 0x48, 0x14, 0x00, 0x26,
};

static char g_cmd_20[] = {
	0xBD,
	0x02,
};

static char g_cmd_21[] = {
	0xE7,
	0x05, 0x05, 0x01, 0x05, 0x01, 0x05, 0x04, 0x04, 0x04, 0x24, 0x00, 0x24, 0x81, 0x02, 0x40, 0x00, 0x32, 0x87,
	0x03, 0x02, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static char g_cmd_22[] = {
	0xBD,
	0x00,
};

static char g_cmd_23[] = {
	0xE9,
	0xD0,
};

static char g_cmd_24[] = {
	0xB2,
	0xF0,
};

static char g_cmd_25[] = {
	0xE9,
	0x3F,
};

static char g_cmd_26[] = {
	0xBF,
	0xFD, 0x00, 0x80, 0x9C, 0x10, 0x00, 0x81, 0x0C,
};

static char g_cmd_27[] = {
	0xBD,
	0x00,
};

static char g_cmd_28[] = {
	0xE9,
	0xC8,
};

static char g_cmd_29[] = {
	0xB1,
	0x25,
};

static char g_cmd_30[] = {
	0xE9,
	0x3F,
};

static char g_cmd_31[] = {
	0xBD,
	0x01,
};

static char g_cmd_32[] = {
	0xE4,
	0xE1, 0xE1, 0xE1, 0xE1, 0xE1, 0xE1, 0xE1, 0xE1, 0xC7, 0xB2, 0xA0, 0x90, 0x81, 0x75, 0x69, 0x5F, 0x55, 0x4C, 0x44,
	0x3D, 0x36, 0x2F, 0x2A, 0x24, 0x1E, 0x19, 0x14, 0x10, 0x09, 0x08, 0x07, 0x54, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
	0x55,
};

static char g_cmd_33[] = {
	0xBD,
	0x03,
};

static char g_cmd_34[] = {
	0xE4,
	0xAA, 0xD4, 0xFF, 0x2A, 0x55, 0x7F, 0xAA, 0xD4, 0xFF, 0xEA, 0xFF, 0x03,
};

static char g_cmd_35[] = {
	0xBD,
	0x00,
};

static char g_cmd_36[] = {
	0xBE,
	0x01, 0x35, 0x00,
};

static char g_cmd_37[] = {
	0xD9,
	0x5F,
};

static char g_cmd_38[] = {
	0xB9,
	0x00, 0x00, 0x00,
};


static char g_cmd_39[] = {
	0x11,
};

static char g_cmd_40[] = {
	0x29,
};

static char g_cmd_41[] = {
	0x51,
	0x04, 0xFF,
};

static char g_cmd_42[] = {
	0x55,
	0x01,
};

static char g_cmd_43[] = {
	0x53,
	0x24,
};

static struct dsi_cmd_desc g_lcd_display_on_cmds[] = {
	{DTYPE_DCS_LWRITE, 0,0, WAIT_TYPE_US, sizeof(g_cmd_1), g_cmd_1},
	{DTYPE_DCS_LWRITE, 0,0, WAIT_TYPE_US, sizeof(g_cmd_2), g_cmd_2},
	{DTYPE_DCS_LWRITE, 0,0, WAIT_TYPE_US, sizeof(g_cmd_3), g_cmd_3},
	{DTYPE_DCS_LWRITE, 0,0, WAIT_TYPE_US, sizeof(g_cmd_4), g_cmd_4},
	{DTYPE_DCS_LWRITE, 0,0, WAIT_TYPE_US, sizeof(g_cmd_5), g_cmd_5},
	{DTYPE_DCS_LWRITE, 0,0, WAIT_TYPE_US, sizeof(g_cmd_6), g_cmd_6},
	{DTYPE_DCS_LWRITE, 0,0, WAIT_TYPE_US, sizeof(g_cmd_7), g_cmd_7},
	{DTYPE_DCS_LWRITE, 0,0, WAIT_TYPE_US, sizeof(g_cmd_8), g_cmd_8},
	{DTYPE_DCS_LWRITE, 0,0, WAIT_TYPE_US, sizeof(g_cmd_9), g_cmd_9},
	{DTYPE_DCS_LWRITE, 0,0, WAIT_TYPE_US, sizeof(g_cmd_10), g_cmd_10},
	{DTYPE_DCS_LWRITE, 0,0, WAIT_TYPE_US, sizeof(g_cmd_11), g_cmd_11},
	{DTYPE_DCS_LWRITE, 0,0, WAIT_TYPE_US, sizeof(g_cmd_12), g_cmd_12},
	{DTYPE_DCS_LWRITE, 0,0, WAIT_TYPE_US, sizeof(g_cmd_13), g_cmd_13},
	{DTYPE_DCS_LWRITE, 0,0, WAIT_TYPE_US, sizeof(g_cmd_14), g_cmd_14},
	{DTYPE_DCS_LWRITE, 0,0, WAIT_TYPE_US, sizeof(g_cmd_15), g_cmd_15},
	{DTYPE_DCS_LWRITE, 0,0, WAIT_TYPE_US, sizeof(g_cmd_16), g_cmd_16},
	{DTYPE_DCS_LWRITE, 0,0, WAIT_TYPE_US, sizeof(g_cmd_17), g_cmd_17},
	{DTYPE_DCS_LWRITE, 0,0, WAIT_TYPE_US, sizeof(g_cmd_18), g_cmd_18},
	{DTYPE_DCS_LWRITE, 0,0, WAIT_TYPE_US, sizeof(g_cmd_19), g_cmd_19},
	{DTYPE_DCS_LWRITE, 0,0, WAIT_TYPE_US, sizeof(g_cmd_20), g_cmd_20},
	{DTYPE_DCS_LWRITE, 0,0, WAIT_TYPE_US, sizeof(g_cmd_21), g_cmd_21},

	{DTYPE_DCS_LWRITE, 0,0, WAIT_TYPE_US, sizeof(g_cmd_22), g_cmd_22},
	{DTYPE_DCS_LWRITE, 0,0, WAIT_TYPE_US, sizeof(g_cmd_23), g_cmd_23},
	{DTYPE_DCS_LWRITE, 0,0, WAIT_TYPE_US, sizeof(g_cmd_24), g_cmd_24},
	{DTYPE_DCS_LWRITE, 0,0, WAIT_TYPE_US, sizeof(g_cmd_25), g_cmd_25},
	{DTYPE_DCS_LWRITE, 0,0, WAIT_TYPE_US, sizeof(g_cmd_26), g_cmd_26},
	{DTYPE_DCS_LWRITE, 0,0, WAIT_TYPE_US, sizeof(g_cmd_27), g_cmd_27},

	{DTYPE_DCS_LWRITE, 0,0, WAIT_TYPE_US, sizeof(g_cmd_28), g_cmd_28},
	{DTYPE_DCS_LWRITE, 0,0, WAIT_TYPE_US, sizeof(g_cmd_29), g_cmd_29},
	{DTYPE_DCS_LWRITE, 0,0, WAIT_TYPE_US, sizeof(g_cmd_30), g_cmd_30},
	{DTYPE_DCS_LWRITE, 0,0, WAIT_TYPE_US, sizeof(g_cmd_31), g_cmd_31},
	{DTYPE_DCS_LWRITE, 0,0, WAIT_TYPE_US, sizeof(g_cmd_32), g_cmd_32},

	{DTYPE_DCS_LWRITE, 0,0, WAIT_TYPE_US, sizeof(g_cmd_33), g_cmd_33},
	{DTYPE_DCS_LWRITE, 0,0, WAIT_TYPE_US, sizeof(g_cmd_34), g_cmd_34},

	{DTYPE_DCS_LWRITE, 0,0, WAIT_TYPE_US, sizeof(g_cmd_35), g_cmd_35},
	{DTYPE_DCS_LWRITE, 0,0, WAIT_TYPE_US, sizeof(g_cmd_36), g_cmd_36},
	{DTYPE_DCS_LWRITE, 0,0, WAIT_TYPE_US, sizeof(g_cmd_37), g_cmd_37},
	{DTYPE_DCS_LWRITE, 0,120, WAIT_TYPE_MS, sizeof(g_cmd_38), g_cmd_38},
	{DTYPE_DCS_LWRITE, 0,120, WAIT_TYPE_MS, sizeof(g_cmd_39), g_cmd_39},
	{DTYPE_DCS_LWRITE, 0,0, WAIT_TYPE_US, sizeof(g_cmd_40), g_cmd_40},
	{DTYPE_DCS_LWRITE, 0,0, WAIT_TYPE_US, sizeof(g_cmd_41), g_cmd_41},
	{DTYPE_DCS_LWRITE, 0,0, WAIT_TYPE_US, sizeof(g_cmd_42), g_cmd_42},
	{DTYPE_DCS_LWRITE, 0,31, WAIT_TYPE_MS, sizeof(g_cmd_43), g_cmd_43},
};

static char g_display_off[] = {
	0x10,
};

static struct dsi_cmd_desc g_lcd_display_off_cmds[] = {
	{DTYPE_DCS_WRITE, 0, 128, WAIT_TYPE_MS, sizeof(g_display_off), g_display_off},
};
#endif

/*******************************************************************************
** LCD VCC
*/
#define VCC_LCDIO_NAME "lcdio-vcc"

static struct regulator *vcc_lcdio;

static struct vcc_desc lcd_vcc_init_cmds[] = {
	/* vcc get */
	{DTYPE_VCC_GET, VCC_LCDIO_NAME, &vcc_lcdio, 0, 0, WAIT_TYPE_MS, 0},

	/* io set voltage */
	{DTYPE_VCC_SET_VOLTAGE, VCC_LCDIO_NAME, &vcc_lcdio, 1800000, 1800000, WAIT_TYPE_MS, 0},
};

static struct vcc_desc lcd_vcc_finit_cmds[] = {
	/* vcc put */
	{DTYPE_VCC_PUT, VCC_LCDIO_NAME, &vcc_lcdio, 0, 0, WAIT_TYPE_MS, 0},
};

static struct vcc_desc lcd_vcc_enable_cmds[] = {
	/* vcc enable */
	{DTYPE_VCC_ENABLE, VCC_LCDIO_NAME, &vcc_lcdio, 0, 0, WAIT_TYPE_MS, 3},
};

static struct vcc_desc lcd_vcc_disable_cmds[] = {
	/* vcc disable */
	{DTYPE_VCC_DISABLE, VCC_LCDIO_NAME, &vcc_lcdio, 0, 0, WAIT_TYPE_MS, 3},
};

/*******************************************************************************
** LCD IOMUX
*/
static struct pinctrl_data pctrl;

static struct pinctrl_cmd_desc lcd_pinctrl_init_cmds[] = {
	{DTYPE_PINCTRL_GET, &pctrl, 0},
	{DTYPE_PINCTRL_STATE_GET, &pctrl, DTYPE_PINCTRL_STATE_DEFAULT},
	{DTYPE_PINCTRL_STATE_GET, &pctrl, DTYPE_PINCTRL_STATE_IDLE},
};

static struct pinctrl_cmd_desc lcd_pinctrl_normal_cmds[] = {
	{DTYPE_PINCTRL_SET, &pctrl, DTYPE_PINCTRL_STATE_DEFAULT},
};

static struct pinctrl_cmd_desc lcd_pinctrl_lowpower_cmds[] = {
	{DTYPE_PINCTRL_SET, &pctrl, DTYPE_PINCTRL_STATE_IDLE},
};

static struct pinctrl_cmd_desc lcd_pinctrl_finit_cmds[] = {
	{DTYPE_PINCTRL_PUT, &pctrl, 0},
};

/*******************************************************************************
** LCD GPIO
*/
#define GPIO_LCD_P5V5_ENABLE_NAME "gpio_lcd_p5v5_enable"
#define GPIO_LCD_N5V5_ENABLE_NAME "gpio_lcd_n5v5_enable"
#define GPIO_LCD_VDD_ENABLE_NAME "gpio_lcd_vdd_enable"
#define GPIO_LCD_BL_ENABLE_NAME "gpio_lcd_bl_enable"
#define GPIO_LCD_RESET_NAME "gpio_lcd_reset"
#define GPIO_LCD_TE0_NAME "gpio_lcd_te0"
static uint32_t gpio_lcd_p5v5_enable;
static uint32_t gpio_lcd_n5v5_enable;
static uint32_t gpio_lcd_vdd_enable;
static uint32_t gpio_lcd_bl_enable;
static uint32_t gpio_lcd_reset;
static uint32_t gpio_lcd_te0;

static struct gpio_desc asic_lcd_gpio_request_cmds[] = {
	/* AVDD_5.5V */
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0, GPIO_LCD_P5V5_ENABLE_NAME, &gpio_lcd_p5v5_enable, 0},
	/* AVEE_-5.5V */
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0, GPIO_LCD_N5V5_ENABLE_NAME, &gpio_lcd_n5v5_enable, 0},

	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0, GPIO_LCD_VDD_ENABLE_NAME, &gpio_lcd_vdd_enable, 0},
	/* backlight enable */
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0, GPIO_LCD_BL_ENABLE_NAME, &gpio_lcd_bl_enable, 0},
	/* reset */
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0, GPIO_LCD_RESET_NAME, &gpio_lcd_reset, 0},
};

static struct gpio_desc asic_lcd_gpio_normal_cmds[] = {
	/* AVDD_5.5V */
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 20, GPIO_LCD_P5V5_ENABLE_NAME, &gpio_lcd_p5v5_enable, 1},
	/* AVEE_-5.5V */
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 20, GPIO_LCD_N5V5_ENABLE_NAME, &gpio_lcd_n5v5_enable, 1},
	/* reset */
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 5, GPIO_LCD_RESET_NAME, &gpio_lcd_reset, 1},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_US, 50, GPIO_LCD_RESET_NAME, &gpio_lcd_reset, 0},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 20, GPIO_LCD_RESET_NAME, &gpio_lcd_reset, 1},

	/* backlight enable */
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 40, GPIO_LCD_VDD_ENABLE_NAME, &gpio_lcd_vdd_enable, 1},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 5, GPIO_LCD_BL_ENABLE_NAME, &gpio_lcd_bl_enable, 1},
};

static struct gpio_desc asic_lcd_gpio_lowpower_cmds[] = {
	/* reset */
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 20, GPIO_LCD_RESET_NAME, &gpio_lcd_reset, 0},
	/* AVEE_-5.5V */
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 20, GPIO_LCD_N5V5_ENABLE_NAME, &gpio_lcd_n5v5_enable, 0},
	/* AVDD_5.5V */
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 20, GPIO_LCD_P5V5_ENABLE_NAME, &gpio_lcd_p5v5_enable, 0},
};

static struct gpio_desc asic_lcd_gpio_free_cmds[] = {

	/* AVDD_5.5V */
	{DTYPE_GPIO_FREE, WAIT_TYPE_US, 0, GPIO_LCD_P5V5_ENABLE_NAME, &gpio_lcd_p5v5_enable, 0},
	/* AVEE_-5.5V */
	{DTYPE_GPIO_FREE, WAIT_TYPE_US, 0, GPIO_LCD_N5V5_ENABLE_NAME, &gpio_lcd_n5v5_enable, 0},
	/* reset */
	{DTYPE_GPIO_FREE, WAIT_TYPE_US, 0, GPIO_LCD_RESET_NAME, &gpio_lcd_reset, 0},
};

/* dsi param initialized value from panel spec */
static void mipi_lcd_init_dsi_param(struct dkmd_connector_info *pinfo, struct mipi_panel_info *mipi)
{
#ifdef DSC_1_1_EN
	mipi->hsa = 40;
	mipi->hbp = 60;
	mipi->dpi_hsize = 202;
	mipi->hline_time = 362;
	mipi->vsa = 4;
	mipi->vbp = 70;
	mipi->vfp = 210;
	mipi->pxl_clk_rate = 494 * 1000000UL;
	mipi->dsi_bit_clk = 494;
#else
	mipi->hsa = 20;
	mipi->hbp = 40;
	mipi->dpi_hsize = 602;
	mipi->hline_time = 727;
	mipi->vsa = 4;
	mipi->vbp = 70;
	mipi->vfp = 168;
	mipi->pxl_clk_rate = 490 * 1000000UL;
	mipi->dsi_bit_clk = 490;
#endif

	mipi->dsi_bit_clk_upt = mipi->dsi_bit_clk;
	mipi->dsi_bit_clk_default = mipi->dsi_bit_clk;

	mipi->pxl_clk_rate_div = 1;
	mipi->dsi_bit_clk_upt_support = 0;

	mipi->clk_post_adjust = 100;
	mipi->lane_nums = DSI_4_LANES;
	mipi->color_mode = DSI_24BITS_1;

	mipi->dsi_version = DSI_1_1_VERSION;
	mipi->phy_mode = DPHY_MODE;

	mipi->vc = 0;
	mipi->max_tx_esc_clk = 10 * 1000000;

	mipi->burst_mode = DSI_NON_BURST_SYNC_EVENTS;
	mipi->non_continue_en = 0;
}

/*******************************************************************************
 */
static void panel_drv_private_data_setup(struct panel_drv_private *priv, struct device_node *np)
{
	gpio_lcd_p5v5_enable = (uint32_t)of_get_named_gpio(np, "gpios", 0);
	gpio_lcd_n5v5_enable = (uint32_t)of_get_named_gpio(np, "gpios", 1);
	gpio_lcd_reset = (uint32_t)of_get_named_gpio(np, "gpios", 2);
	gpio_lcd_vdd_enable = (uint32_t)of_get_named_gpio(np, "gpios", 3);
	gpio_lcd_bl_enable = (uint32_t)of_get_named_gpio(np, "gpios", 4);
	gpio_lcd_te0 = (uint32_t)of_get_named_gpio(np, "gpios", 5);

	dpu_pr_info("used gpio:[p5v5: %d, n5v5: %d, rst: %d, vdd: %d, bl: %d, te0: %d]\n",
		gpio_lcd_p5v5_enable, gpio_lcd_n5v5_enable, gpio_lcd_reset, gpio_lcd_vdd_enable, gpio_lcd_bl_enable, gpio_lcd_te0);

	priv->gpio_request_cmds = asic_lcd_gpio_request_cmds;
	priv->gpio_request_cmds_len = ARRAY_SIZE(asic_lcd_gpio_request_cmds);
	priv->gpio_free_cmds = asic_lcd_gpio_free_cmds;
	priv->gpio_free_cmds_len = ARRAY_SIZE(asic_lcd_gpio_free_cmds);

	priv->gpio_normal_cmds = asic_lcd_gpio_normal_cmds;
	priv->gpio_normal_cmds_len = ARRAY_SIZE(asic_lcd_gpio_normal_cmds);
	priv->gpio_lowpower_cmds = asic_lcd_gpio_lowpower_cmds;
	priv->gpio_lowpower_cmds_len = ARRAY_SIZE(asic_lcd_gpio_lowpower_cmds);

	priv->vcc_init_cmds = lcd_vcc_init_cmds;
	priv->vcc_init_cmds_len = ARRAY_SIZE(lcd_vcc_init_cmds);
	priv->vcc_finit_cmds = lcd_vcc_finit_cmds;
	priv->vcc_finit_cmds_len = ARRAY_SIZE(lcd_vcc_finit_cmds);

	priv->vcc_enable_cmds = lcd_vcc_enable_cmds;
	priv->vcc_enable_cmds_len = ARRAY_SIZE(lcd_vcc_enable_cmds);
	priv->vcc_disable_cmds = lcd_vcc_disable_cmds;
	priv->vcc_disable_cmds_len = ARRAY_SIZE(lcd_vcc_disable_cmds);

	priv->pinctrl_init_cmds = lcd_pinctrl_init_cmds;
	priv->pinctrl_init_cmds_len = ARRAY_SIZE(lcd_pinctrl_init_cmds);
	priv->pinctrl_finit_cmds = lcd_pinctrl_finit_cmds;
	priv->pinctrl_finit_cmds_len = ARRAY_SIZE(lcd_pinctrl_finit_cmds);

	priv->pinctrl_normal_cmds = lcd_pinctrl_normal_cmds;
	priv->pinctrl_normal_cmds_len = ARRAY_SIZE(lcd_pinctrl_normal_cmds);
	priv->pinctrl_lowpower_cmds = lcd_pinctrl_lowpower_cmds;
	priv->pinctrl_lowpower_cmds_len = ARRAY_SIZE(lcd_pinctrl_lowpower_cmds);
}

/*******************************************************************************
*/
static void dsc_config_initial(struct dkmd_connector_info *pinfo, struct dsc_calc_info *dsc)
{
	dpu_pr_info("+\n");

	pinfo->ifbc_type = IFBC_TYPE_VESA3X_DUAL;
	dsc->dsc_en = 1;

	dsc->dsc_info.dsc_version_major = 1;
	dsc->dsc_info.dsc_version_minor = 1;
	dsc->dsc_info.pic_height = pinfo->base.yres;
	dsc->dsc_info.pic_width = 800;
	dsc->dsc_info.slice_height = 20;
	dsc->dsc_info.slice_width = 800;
	dsc->dsc_info.chunk_size = 0x320;
	dsc->dsc_info.initial_dec_delay = 0x0291;
	dsc->dsc_info.initial_scale_value = 0x20;
	dsc->dsc_info.scale_increment_interval = 0x0247;
	dsc->dsc_info.scale_decrement_interval = 11;
	dsc->dsc_info.nfl_bpg_offset = 0x50e;
	dsc->dsc_info.slice_bpg_offset = 0x368;
	dsc->dsc_info.final_offset = 0x10e0;

	// dsc parameter info
	dsc->dsc_info.dsc_bpc = 8;
	dsc->dsc_info.dsc_bpp = 0x8;
	dsc->dsc_info.initial_xmit_delay = 512;
	dsc->dsc_info.first_line_bpg_offset = 12;

	// DSC_CTRL
	dsc->dsc_info.block_pred_enable = 1;
	dsc->dsc_info.linebuf_depth = 9;
	// RC_PARAM3
	dsc->dsc_info.initial_offset = 6144;
	// FLATNESS_QP_TH
	dsc->dsc_info.flatness_min_qp = 3;
	dsc->dsc_info.flatness_max_qp = 12;
	// DSC_PARAM4
	dsc->dsc_info.rc_edge_factor = 0x6;
	dsc->dsc_info.rc_model_size = 8192;
	// DSC_RC_PARAM5: 0x330b0b
	dsc->dsc_info.rc_tgt_offset_lo = (0x330b0b >> 20) & 0xF;
	dsc->dsc_info.rc_tgt_offset_hi = (0x330b0b >> 16) & 0xF;
	dsc->dsc_info.rc_quant_incr_limit1 = (0x330b0b >> 8) & 0x1F;
	dsc->dsc_info.rc_quant_incr_limit0 = (0x330b0b >> 0) & 0x1F;
	// DSC_RC_BUF_THRESH0: 0xe1c2a38
	dsc->dsc_info.rc_buf_thresh[0] = (0xe1c2a38 >> 24) & 0xFF;
	dsc->dsc_info.rc_buf_thresh[1] = (0xe1c2a38 >> 16) & 0xFF;
	dsc->dsc_info.rc_buf_thresh[2] = (0xe1c2a38 >> 8) & 0xFF;
	dsc->dsc_info.rc_buf_thresh[3] = (0xe1c2a38 >> 0) & 0xFF;
	// DSC_RC_BUF_THRESH1: 0x46546269
	dsc->dsc_info.rc_buf_thresh[4] = (0x46546269 >> 24) & 0xFF;
	dsc->dsc_info.rc_buf_thresh[5] = (0x46546269 >> 16) & 0xFF;
	dsc->dsc_info.rc_buf_thresh[6] = (0x46546269 >> 8) & 0xFF;
	dsc->dsc_info.rc_buf_thresh[7] = (0x46546269 >> 0) & 0xFF;
	// DSC_RC_BUF_THRESH2: 0x7077797b
	dsc->dsc_info.rc_buf_thresh[8] = (0x7077797b >> 24) & 0xFF;
	dsc->dsc_info.rc_buf_thresh[9] = (0x7077797b >> 16) & 0xFF;
	dsc->dsc_info.rc_buf_thresh[10] = (0x7077797b >> 8) & 0xFF;
	dsc->dsc_info.rc_buf_thresh[11] = (0x7077797b >> 0) & 0xFF;
	// DSC_RC_BUF_THRESH3: 0x7d7e0000
	dsc->dsc_info.rc_buf_thresh[12] = (0x7d7e0000 >> 24) & 0xFF;
	dsc->dsc_info.rc_buf_thresh[13] = (0x7d7e0000 >> 16) & 0xFF;
	// DSC_RC_RANGE_PARAM0: 0x1020100
	dsc->dsc_info.rc_range[0].min_qp = (0x1020100 >> 27) & 0x1F;
	dsc->dsc_info.rc_range[0].max_qp = (0x1020100 >> 22) & 0x1F;
	dsc->dsc_info.rc_range[0].offset = (0x1020100 >> 16) & 0x3F;
	dsc->dsc_info.rc_range[1].min_qp = (0x1020100 >> 11) & 0x1F;
	dsc->dsc_info.rc_range[1].max_qp = (0x1020100 >> 6) & 0x1F;
	dsc->dsc_info.rc_range[1].offset = (0x1020100 >> 0) & 0x3F;
	// DSC_RC_RANGE_PARAM1: 0x94009be
	dsc->dsc_info.rc_range[2].min_qp = (0x94009be >> 27) & 0x1F;
	dsc->dsc_info.rc_range[2].max_qp = (0x94009be >> 22) & 0x1F;
	dsc->dsc_info.rc_range[2].offset = (0x94009be >> 16) & 0x3F;
	dsc->dsc_info.rc_range[3].min_qp = (0x94009be >> 11) & 0x1F;
	dsc->dsc_info.rc_range[3].max_qp = (0x94009be >> 6) & 0x1F;
	dsc->dsc_info.rc_range[3].offset = (0x94009be >> 0) & 0x3F;
	// DSC_RC_RANGE_PARAM2, 0x19fc19fa
	dsc->dsc_info.rc_range[4].min_qp = (0x19fc19fa >> 27) & 0x1F;
	dsc->dsc_info.rc_range[4].max_qp = (0x19fc19fa >> 22) & 0x1F;
	dsc->dsc_info.rc_range[4].offset = (0x19fc19fa >> 16) & 0x3F;
	dsc->dsc_info.rc_range[5].min_qp = (0x19fc19fa >> 11) & 0x1F;
	dsc->dsc_info.rc_range[5].max_qp = (0x19fc19fa >> 6) & 0x1F;
	dsc->dsc_info.rc_range[5].offset = (0x19fc19fa >> 0) & 0x3F;
	// DSC_RC_RANGE_PARAM3, 0x19f81a38
	dsc->dsc_info.rc_range[6].min_qp = (0x19f81a38 >> 27) & 0x1F;
	dsc->dsc_info.rc_range[6].max_qp = (0x19f81a38 >> 22) & 0x1F;
	dsc->dsc_info.rc_range[6].offset = (0x19f81a38 >> 16) & 0x3F;
	dsc->dsc_info.rc_range[7].min_qp = (0x19f81a38 >> 11) & 0x1F;
	dsc->dsc_info.rc_range[7].max_qp = (0x19f81a38 >> 6) & 0x1F;
	dsc->dsc_info.rc_range[7].offset = (0x19f81a38 >> 0) & 0x3F;
	// DSC_RC_RANGE_PARAM4, 0x1a781ab6
	dsc->dsc_info.rc_range[8].min_qp = (0x1a781ab6 >> 27) & 0x1F;
	dsc->dsc_info.rc_range[8].max_qp = (0x1a781ab6 >> 22) & 0x1F;
	dsc->dsc_info.rc_range[8].offset = (0x1a781ab6 >> 16) & 0x3F;
	dsc->dsc_info.rc_range[9].min_qp = (0x1a781ab6 >> 11) & 0x1F;
	dsc->dsc_info.rc_range[9].max_qp = (0x1a781ab6 >> 6) & 0x1F;
	dsc->dsc_info.rc_range[9].offset = (0x1a781ab6 >> 0) & 0x3F;
	// DSC_RC_RANGE_PARAM5, 0x2af62b34
	dsc->dsc_info.rc_range[10].min_qp = (0x2af62b34 >> 27) & 0x1F;
	dsc->dsc_info.rc_range[10].max_qp = (0x2af62b34 >> 22) & 0x1F;
	dsc->dsc_info.rc_range[10].offset = (0x2af62b34 >> 16) & 0x3F;
	dsc->dsc_info.rc_range[11].min_qp = (0x2af62b34 >> 11) & 0x1F;
	dsc->dsc_info.rc_range[11].max_qp = (0x2af62b34 >> 6) & 0x1F;
	dsc->dsc_info.rc_range[11].offset = (0x2af62b34 >> 0) & 0x3F;
	// DSC_RC_RANGE_PARAM6, 0x2b743b74
	dsc->dsc_info.rc_range[12].min_qp = (0x2b743b74 >> 27) & 0x1F;
	dsc->dsc_info.rc_range[12].max_qp = (0x2b743b74 >> 22) & 0x1F;
	dsc->dsc_info.rc_range[12].offset = (0x2b743b74 >> 16) & 0x3F;
	dsc->dsc_info.rc_range[13].min_qp = (0x2b743b74 >> 11) & 0x1F;
	dsc->dsc_info.rc_range[13].max_qp = (0x2b743b74 >> 6) & 0x1F;
	dsc->dsc_info.rc_range[13].offset = (0x2b743b74 >> 0) & 0x3F;
	// DSC_RC_RANGE_PARAM7, 0x6bf40000
	dsc->dsc_info.rc_range[14].min_qp = (0x6bf40000 >> 27) & 0x1F;
	dsc->dsc_info.rc_range[14].max_qp = (0x6bf40000 >> 22) & 0x1F;
	dsc->dsc_info.rc_range[14].offset = (0x6bf40000 >> 16) & 0x3F;

	dpu_pr_info("-\n");
}

/* dirty region initialized value from panel spec */
static void lcd_init_dirty_region(struct panel_drv_private *priv)
{
	priv->user_pinfo.left_align = -1;
	priv->user_pinfo.right_align = -1;
	priv->user_pinfo.top_align = 32;
	priv->user_pinfo.bottom_align = 32;
	priv->user_pinfo.w_align = -1;
	priv->user_pinfo.h_align = -1;
	priv->user_pinfo.w_min = 2160;
	priv->user_pinfo.h_min = -1;
	priv->user_pinfo.top_start = -1;
	priv->user_pinfo.bottom_start = -1;
}

static int32_t panel_of_device_setup(struct panel_drv_private *priv)
{
	int32_t ret;
	struct dkmd_connector_info *pinfo = &priv->connector_info;
	struct device_node *np = priv->pdev->dev.of_node;

	dpu_pr_info("83121 enter 700!\n");

	/* Inheritance based processing */
	panel_base_of_device_setup(priv);
	panel_drv_private_data_setup(priv, np);

	/* 1. config base object info
	 * would be used for framebuffer setup
	 */
	pinfo->base.xres = 1600; // FIXME: Modified for new panel device
	pinfo->base.yres = 2560; // FIXME: Modified for new panel device

	/* When calculating DPI needs the following parameters tdb */
	pinfo->base.width = 166; // FIXME: Modified for new panel device
	pinfo->base.height = 265; // FIXME: Modified for new panel device

	// TODO: caculate fps by mipi timing para
#ifdef DSC_1_1_EN
	pinfo->base.fps = 120;
#else
	pinfo->base.fps = 60;
#endif

#ifdef DSC_1_1_EN
	pinfo->ifbc_type = IFBC_TYPE_VESA3X_DUAL;
#else
	pinfo->ifbc_type = IFBC_TYPE_NONE;
#endif

	/* 2. config connector info
	 * would be used for dsi & composer setup
	 */
	mipi_lcd_init_dsi_param(pinfo, &get_primary_connector(pinfo)->mipi);

	/* dsc or composer need this param */
#ifdef DSC_1_1_EN
	dsc_config_initial(pinfo, &get_primary_connector(pinfo)->dsc);
#endif

	/* dsi or composer need this param */
	pinfo->dirty_region_updt_support = 0;

	/* 3. config panel private info
	 * would be used for panel setup
	 */
	pinfo->bl_info.bl_min = 12;
	pinfo->bl_info.bl_max = 2047;
	pinfo->bl_info.bl_default = 1000;
	priv->mipi_brightness_para_type = MIPI_BL_PARA1_DBV8_AND_PARA2_DBV0;

	lcd_init_dirty_region(priv);

	priv->disp_on_cmds = g_lcd_display_on_cmds;
	priv->disp_on_cmds_len = ARRAY_SIZE(g_lcd_display_on_cmds);
	priv->disp_off_cmds = g_lcd_display_off_cmds;
	priv->disp_off_cmds_len = (uint32_t)ARRAY_SIZE(g_lcd_display_off_cmds);

	if (pinfo->base.fpga_flag == 0) {
		ret = peri_vcc_cmds_tx(priv->pdev, priv->vcc_init_cmds, priv->vcc_init_cmds_len);
		if (ret != 0)
			dpu_pr_info("vcc init failed!\n");

		ret = peri_pinctrl_cmds_tx(priv->pdev, priv->pinctrl_init_cmds, priv->pinctrl_init_cmds_len);
		if (ret != 0)
			dpu_pr_info("pinctrl init failed\n");

		ret = peri_vcc_cmds_tx(priv->pdev, priv->vcc_enable_cmds, priv->vcc_enable_cmds_len);
		if (ret)
			dpu_pr_warn("vcc enable cmds handle fail!\n");
	}

	dpu_pr_info("83121 exit 700!\n");

	return 0;
}

static void panel_of_device_release(struct panel_drv_private *priv)
{
	int32_t ret = 0;

	panel_base_of_device_release(priv);

	if (priv->gpio_free_cmds && (priv->gpio_free_cmds_len > 0)) {
		ret = peri_gpio_cmds_tx(priv->gpio_free_cmds, priv->gpio_free_cmds_len);
		if (ret)
			dpu_pr_info("gpio free handle err!\n");
	}

	dpu_pr_info("exit!\n");
}

panel_device_match_data(hx83121_panel_info, PANEL_HX83121_ID, panel_of_device_setup, panel_of_device_release);

MODULE_LICENSE("GPL");