/* Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
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
#include "dsc/dsc_output_calc.h"
#include "dsc/dsc_algorithm_manager.h"

#define DSC_1_1_EN 1

// ---------cphy-----------
static char g_on_cmd0[] = {
	0xFE, 0xB6,
};

static char g_on_cmd1[] = {
	0x01, 0xC1,
};

static char g_on_cmd2[] = {
	0x02, 0x10,
};

static char g_on_cmd3[] = {
	0x03, 0x10,
};

static char g_on_cmd4[] = {
	0x04, 0x10,
};

static char g_on_cmd5[] = {
	0x05, 0x1C,
};

static char g_on_cmd6[] = {
	0x06, 0x1C,
};

static char g_on_cmd7[] = {
	0x07, 0x1C,
};

// -----3tiro--------
static char g_on_cmd8[] = {
	0xFE, 0xD2,
};

static char g_on_cmd9[] = {
	0x35, 0x0F,
};

// --------3tiro-add------
static char g_on_cmd9_0[] = {
	0xFE, 0xF9,
};

static char g_on_cmd9_1[] = {
	0x00, 0x9F,
};

static char g_on_cmd9_2[] = {
	0x1D, 0xC1,
};

static char g_on_cmd9_3[] = {
	0xF7, 0x01,
};

// ---------mipi----------
static char g_on_cmd12[] = {
	0x44, 0x08, 0xC0,
};


// --------vesa-on----------
#ifdef DSC_1_1_EN
static char g_on_cmd13[] = {
	0xFA, 0x01,
};
#else
/* dsc enable: 0x01  dsc disable: 0x07 */
static char g_on_cmd13[] = {
	0xFA, 0x07,
};
#endif


// -------TE-ON-----------
static char g_on_cmd14[] = {
	0xC2, 0x08,
};

static char g_on_cmd15[] = {
	0x35, 0x00,
};

// ------display-on---------
static char g_on_cmd17[] = {
	0x11,
};

static char g_on_cmd18[] = {
	0x29,
};

// --------120HZ-add---------
static char g_on_cmd10_0[] = {
	0xFE, 0x44,
};
static char g_on_cmd10_1[] = {
	0x49, 0x55,
};
static char g_on_cmd10_2[] = {
	0x4A, 0x55,
};
static char g_on_cmd10_3[] = {
	0x4B, 0x55,
};
static char g_on_cmd10_4[] = {
	0xFE, 0x40,
};
static char g_on_cmd10_5[] = {
	0xBD, 0x4,
};

// --------120HZ---------
static char g_on_cmd10[] = {
	0xFE, 0x00,
};

static char g_on_cmd11[] = {
	0x8A, 0x10,
};

// ---------10bit--boe------------
static char g_on_cmd23[112][2] = {
	{0xFE, 0xD2},
	{0x4F, 0x08},
	{0x50, 0x11},
	{0x51, 0xab},
	{0x52, 0x30},
	{0x53, 0x09},
	{0x54, 0xc8},
	{0x55, 0x04},
	{0x56, 0x38},
	{0x58, 0x00},
	{0x59, 0x08},
	{0x5a, 0x02},
	{0x5b, 0x1c},
	{0x5c, 0x02},
	{0x5d, 0x00},
	{0x5e, 0x20},
	{0x5f, 0x00},
	{0x60, 0xbb},
	{0x61, 0x00},
	{0x62, 0x07},
	{0x63, 0x0c},
	{0x64, 0x0d},
	{0x65, 0xb7},
	{0x66, 0x0c},
	{0x67, 0xb7},
	{0x68, 0x18},
	{0x69, 0x00},
	{0x6a, 0x10},
	{0x6b, 0xf0},
	{0x6c, 0x07},
	{0x6d, 0x10},
	{0x6e, 0x20},
	{0x6f, 0x00},
	{0x70, 0x06},
	{0x71, 0x0f},
	{0x72, 0x0f},
	{0x73, 0x33},
	{0x74, 0x0e},
	{0x75, 0x1c},
	{0x76, 0x2a},
	{0x77, 0x38},
	{0x78, 0x46},
	{0x79, 0x54},
	{0x7a, 0x62},
	{0x7b, 0x69},
	{0x7c, 0x70},
	{0x7d, 0x77},
	{0x7e, 0x79},
	{0x7f, 0x7b},
	{0x80, 0x7d},
	{0x81, 0x7e},
	{0x82, 0x02},
	{0x83, 0x02},
	{0x84, 0x22},
	{0x85, 0x00},
	{0x86, 0x2a},
	{0x87, 0x40},
	{0x88, 0x2a},
	{0x89, 0xbe},
	{0x8a, 0x3a},
	{0x8b, 0xfc},
	{0x8c, 0x3a},
	{0x8d, 0xfa},
	{0x8e, 0x3a},
	{0x8f, 0xf8},
	{0x90, 0x3b},
	{0x91, 0x38},
	{0x92, 0x3b},
	{0x93, 0x78},
	{0x94, 0x3b},
	{0x95, 0xb6},
	{0x96, 0x4b},
	{0x97, 0xf6},
	{0x98, 0x4c},
	{0x99, 0x34},
	{0x9a, 0x4c},
	{0x9b, 0x74},
	{0x9c, 0x5c},
	{0x9d, 0x74},
	{0x9e, 0x8c},
	{0x9f, 0xf4},
	{0xa2, 0x02},
	{0xa3, 0x1c},
	{0xa4, 0x00},
	{0xa5, 0x00},
	{0xa6, 0x00},
	{0xa7, 0x00},
	{0xa9, 0x00},
	{0xaa, 0x00},
	{0xa0, 0x80},
	{0xFE, 0x83},
	{0x0C, 0x4E},
	{0x0D, 0xFE},
	{0x0E, 0xEE},
	{0x0F, 0xFE},
	{0x10, 0xFE},
	{0x11, 0xEE},
	{0x12, 0xFE},
	{0x13, 0xFE},
	{0x14, 0xEE},
	{0x15, 0xFE},
	{0x16, 0xFE},
	{0x17, 0xEE},
	{0x18, 0xFE},
	{0x19, 0xFE},
	{0x1A, 0xEE},
	{0x1B, 0xFE},
	{0x1C, 0xFE},
	{0xFE, 0x7E},
	{0x50, 0xEE},
	{0x51, 0xFE},
	{0x52, 0xFE},
};

#ifdef BIST_MODE
static char g_bist_cmd0[] = {
	0xFE, 0x00,
};

static char g_bist_cmd1[] = {
	0xFA, 0x07,
};

static char g_bist_cmd2[] = {
	0XFE, 0xD0,
};

static char g_bist_cmd3[] = {
	0x37, 0x90,
};

static char g_bist_cmd4[] = {
	0x43, 0x1F,
};

static char g_bist_cmd5[] = {
	0x44, 0x00,
};

static char g_bist_cmd6[] = {
	0x3A, 0xC8,
};
#endif

static struct dsi_cmd_desc lcd_display_on_cmds_boe_12[] = {
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0), g_on_cmd0},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd1), g_on_cmd1},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd2), g_on_cmd2},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd3), g_on_cmd3},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd4), g_on_cmd4},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd5), g_on_cmd5},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd6), g_on_cmd6},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd7},

	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[0]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[1]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[2]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[3]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[4]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[5]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[6]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[7]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[8]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[9]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[10]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[11]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[12]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[13]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[14]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[15]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[16]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[17]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[18]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[19]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[20]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[21]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[22]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[23]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[24]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[25]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[26]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[27]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[28]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[29]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[30]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[31]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[32]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[33]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[34]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[35]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[36]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[37]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[38]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[39]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[40]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[41]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[42]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[43]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[44]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[45]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[46]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[47]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[48]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[49]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[50]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[51]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[52]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[53]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[54]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[55]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[56]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[57]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[58]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[59]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[60]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[61]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[62]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[63]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[64]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[65]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[66]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[67]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[68]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[69]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[70]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[71]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[72]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[73]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[74]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[75]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[76]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[77]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[78]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[79]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[80]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[81]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[82]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[83]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[84]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[85]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[86]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[87]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[88]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[89]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[90]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[91]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[92]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[93]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[94]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[95]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[96]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[97]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[98]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[99]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[100]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[101]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[102]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[103]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[104]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[105]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[106]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[107]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[108]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[109]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[110]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd23[111]},

	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd8},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd9},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd9_0},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd9_1},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd9_2},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd9_3},

	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd10_0},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd10_1},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd10_2},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd10_3},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd10_4},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd10_5},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd10},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd11},

	{DTYPE_DCS_LWRITE, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd12), g_on_cmd12},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd13), g_on_cmd13},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd14), g_on_cmd14},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd15), g_on_cmd15},

	{DTYPE_DCS_WRITE, 0, 120, WAIT_TYPE_MS, sizeof(g_on_cmd17), g_on_cmd17},
	{DTYPE_DCS_WRITE, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd18), g_on_cmd18},
};

/* Power OFF Sequence */
static char g_off_cmd0[] = {
	0xFE, 0x00,
};

static char g_off_cmd1[] = {
	0x28,
};

static char g_off_cmd2[] = {
	0x10,
};
static struct dsi_cmd_desc lcd_display_off_cmds[] = {
	{ DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_off_cmd0), g_off_cmd0 },
	{ DTYPE_DCS_WRITE, 0, 10, WAIT_TYPE_US, sizeof(g_off_cmd1), g_off_cmd1 },
	{ DTYPE_DCS_WRITE, 0, 100, WAIT_TYPE_MS, sizeof(g_off_cmd2), g_off_cmd2 },
};

/*******************************************************************************
 ** LCD GPIO
 */
#define GPIO_AMOLED_RESET_NAME  "gpio_amoled_reset"
#define GPIO_AMOLED_VCC1V2_NAME "gpio_amoled_vcc1v2"
#define GPIO_AMOLED_VCC1V8_NAME "gpio_amoled_vcc1v8"
#define GPIO_AMOLED_VCC3V1_NAME "gpio_amoled_vcc3v1"
#define GPIO_AMOLED_SW_NAME     "gpio_amoled_sw"
#define GPIO_AMOLED_TE0_NAME    "gpio_amoled_te0"

static uint32_t g_gpio_amoled_te0;
static uint32_t g_gpio_amoled_vcc1v8;
static uint32_t g_gpio_amoled_vcc1v2;
static uint32_t g_gpio_amoled_reset;
static uint32_t g_gpio_amoled_sw;
static uint32_t g_gpio_amoled_vcc3v1;

static struct gpio_desc g_asic_lcd_gpio_request_cmds[] = {
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0, GPIO_AMOLED_SW_NAME, &g_gpio_amoled_sw, 0},
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0, GPIO_AMOLED_VCC1V8_NAME, &g_gpio_amoled_vcc1v8, 0},
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0, GPIO_AMOLED_VCC1V2_NAME, &g_gpio_amoled_vcc1v2, 0},
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0, GPIO_AMOLED_VCC3V1_NAME, &g_gpio_amoled_vcc3v1, 0},
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0, GPIO_AMOLED_RESET_NAME, &g_gpio_amoled_reset, 0},
};

static struct gpio_desc g_asic_lcd_gpio_normal_cmds[] = {
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 10, GPIO_AMOLED_SW_NAME, &g_gpio_amoled_sw, 1},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 5, GPIO_AMOLED_VCC1V8_NAME, &g_gpio_amoled_vcc1v8, 1},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 5, GPIO_AMOLED_VCC1V2_NAME, &g_gpio_amoled_vcc1v2, 1},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 20, GPIO_AMOLED_VCC3V1_NAME, &g_gpio_amoled_vcc3v1, 1},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 5, GPIO_AMOLED_RESET_NAME, &g_gpio_amoled_reset, 1},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 5, GPIO_AMOLED_RESET_NAME, &g_gpio_amoled_reset, 0},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 35, GPIO_AMOLED_RESET_NAME, &g_gpio_amoled_reset, 1},
};

static struct gpio_desc g_asic_lcd_gpio_lowpower_cmds[] = {
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 5, GPIO_AMOLED_RESET_NAME, &g_gpio_amoled_reset, 0},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 5, GPIO_AMOLED_VCC3V1_NAME, &g_gpio_amoled_vcc3v1, 0},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 5, GPIO_AMOLED_VCC1V2_NAME, &g_gpio_amoled_vcc1v2, 0},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 10, GPIO_AMOLED_VCC1V8_NAME, &g_gpio_amoled_vcc1v8, 0},
};

static struct gpio_desc g_asic_lcd_gpio_free_cmds[] = {
	{DTYPE_GPIO_FREE, WAIT_TYPE_MS, 5, GPIO_AMOLED_RESET_NAME, &g_gpio_amoled_reset, 0},
	{DTYPE_GPIO_FREE, WAIT_TYPE_MS, 5, GPIO_AMOLED_VCC3V1_NAME, &g_gpio_amoled_vcc3v1, 0},
	{DTYPE_GPIO_FREE, WAIT_TYPE_MS, 5, GPIO_AMOLED_VCC1V2_NAME, &g_gpio_amoled_vcc1v2, 0},
	{DTYPE_GPIO_FREE, WAIT_TYPE_MS, 10, GPIO_AMOLED_VCC1V8_NAME, &g_gpio_amoled_vcc1v8, 0},
};

/*******************************************************************************
 */
static void panel_drv_private_data_setup(struct panel_drv_private *priv, struct device_node *np)
{
	g_gpio_amoled_vcc3v1 = (uint32_t)of_get_named_gpio(np, "gpios", 0);
	g_gpio_amoled_vcc1v8 = (uint32_t)of_get_named_gpio(np, "gpios", 1);
	g_gpio_amoled_vcc1v2 = (uint32_t)of_get_named_gpio(np, "gpios", 2);
	g_gpio_amoled_reset = (uint32_t)of_get_named_gpio(np, "gpios", 3);
	g_gpio_amoled_te0 = (uint32_t)of_get_named_gpio(np, "gpios", 4);
	g_gpio_amoled_sw = (uint32_t)of_get_named_gpio(np, "gpios", 5);

	dpu_pr_info("used gpio:[rst: %d, vcc3v1: %d, vcc1v8: %d,vcc1v2: %d,te0: %d, sw: %d]\n",
		g_gpio_amoled_reset, g_gpio_amoled_vcc3v1, g_gpio_amoled_vcc1v8,
		g_gpio_amoled_vcc1v2, g_gpio_amoled_te0, g_gpio_amoled_sw);

	priv->gpio_request_cmds = g_asic_lcd_gpio_request_cmds;
	priv->gpio_request_cmds_len = ARRAY_SIZE(g_asic_lcd_gpio_request_cmds);
	priv->gpio_free_cmds = g_asic_lcd_gpio_free_cmds;
	priv->gpio_free_cmds_len = ARRAY_SIZE(g_asic_lcd_gpio_free_cmds);

	priv->gpio_normal_cmds = g_asic_lcd_gpio_normal_cmds;
	priv->gpio_normal_cmds_len = ARRAY_SIZE(g_asic_lcd_gpio_normal_cmds);
	priv->gpio_lowpower_cmds = g_asic_lcd_gpio_lowpower_cmds;
	priv->gpio_lowpower_cmds_len = ARRAY_SIZE(g_asic_lcd_gpio_lowpower_cmds);
}

#if defined(DSC_1_1_EN)
static void dsc_param_set(struct dkmd_connector_info *pinfo, struct dsc_calc_info *dsc)
{
	pinfo->ifbc_type = IFBC_TYPE_VESA3_75X_DUAL;
	dsc->dsc_en = 1;
	dsc->dual_dsc_en = 1;

	dsc->dsc_info.dsc_version_major = 1;
	dsc->dsc_info.dsc_version_minor = 1;
	dsc->dsc_info.pic_height = 2504;
	dsc->dsc_info.pic_width = 1080;
	dsc->dsc_info.slice_height = 8;
	dsc->dsc_info.slice_width = 540;
	dsc->dsc_info.chunk_size = 0x021c;
	dsc->dsc_info.initial_dec_delay = 0x020e;
	dsc->dsc_info.initial_scale_value = 0x20;
	dsc->dsc_info.scale_increment_interval = 0x00bb;
	dsc->dsc_info.scale_decrement_interval = 0x007;
	dsc->dsc_info.nfl_bpg_offset = 0x0db7;
	dsc->dsc_info.slice_bpg_offset = 0x0cb7;
	dsc->dsc_info.final_offset = 0x10f0;

	// dsc parameter info
	dsc->dsc_info.dsc_bpc = 10;
	dsc->dsc_info.dsc_bpp = 8;
	dsc->dsc_info.initial_xmit_delay = 0x200;
	dsc->dsc_info.first_line_bpg_offset = 0xc;

	// DSC_CTRL
	dsc->dsc_info.block_pred_enable = 1;
	dsc->dsc_info.linebuf_depth = 11;
	// RC_PARAM3
	dsc->dsc_info.initial_offset = 0x1800;
	// FLATNESS_QP_TH
	dsc->dsc_info.flatness_min_qp = 7;
	dsc->dsc_info.flatness_max_qp = 16;
	// DSC_PARAM4
	dsc->dsc_info.rc_edge_factor = 6;
	dsc->dsc_info.rc_model_size = 8192;
	// DSC_RC_PARAM5: 0x330b0b
	dsc->dsc_info.rc_tgt_offset_lo = 3;
	dsc->dsc_info.rc_tgt_offset_hi = 3;
	dsc->dsc_info.rc_quant_incr_limit1 = 15;
	dsc->dsc_info.rc_quant_incr_limit0 = 15;
	// DSC_RC_BUF_THRESH0: 0xe1c2a38
	dsc->dsc_info.rc_buf_thresh[0] = 14;
	dsc->dsc_info.rc_buf_thresh[1] = 28;
	dsc->dsc_info.rc_buf_thresh[2] = 42;
	dsc->dsc_info.rc_buf_thresh[3] = 56;
	// DSC_RC_BUF_THRESH1: 0x46546269
	dsc->dsc_info.rc_buf_thresh[4] = 70;
	dsc->dsc_info.rc_buf_thresh[5] = 84;
	dsc->dsc_info.rc_buf_thresh[6] = 98;
	dsc->dsc_info.rc_buf_thresh[7] = 105;
	// DSC_RC_BUF_THRESH2: 0x7077797b
	dsc->dsc_info.rc_buf_thresh[8] = 112;
	dsc->dsc_info.rc_buf_thresh[9] = 119;
	dsc->dsc_info.rc_buf_thresh[10] = 121;
	dsc->dsc_info.rc_buf_thresh[11] = 123;
	// DSC_RC_BUF_THRESH3: 0x7d7e0000
	dsc->dsc_info.rc_buf_thresh[12] = 125;
	dsc->dsc_info.rc_buf_thresh[13] = 126;
	// DSC_RC_RANGE_PARAM0: 0x1020100
	dsc->dsc_info.rc_range[0].min_qp = 0;
	dsc->dsc_info.rc_range[0].max_qp = 8;
	dsc->dsc_info.rc_range[0].offset = 2;
	dsc->dsc_info.rc_range[1].min_qp = 4;
	dsc->dsc_info.rc_range[1].max_qp = 8;
	dsc->dsc_info.rc_range[1].offset = 0;
	// DSC_RC_RANGE_PARAM1: 0x94009be
	dsc->dsc_info.rc_range[2].min_qp = 5;
	dsc->dsc_info.rc_range[2].max_qp = 9;
	dsc->dsc_info.rc_range[2].offset = 0;
	dsc->dsc_info.rc_range[3].min_qp = 5;
	dsc->dsc_info.rc_range[3].max_qp = 10;
	dsc->dsc_info.rc_range[3].offset = 62;
	// DSC_RC_RANGE_PARAM2, 0x19fc19fa
	dsc->dsc_info.rc_range[4].min_qp = 7;
	dsc->dsc_info.rc_range[4].max_qp = 11;
	dsc->dsc_info.rc_range[4].offset = 60;
	dsc->dsc_info.rc_range[5].min_qp = 7;
	dsc->dsc_info.rc_range[5].max_qp = 11;
	dsc->dsc_info.rc_range[5].offset = 58;
	// DSC_RC_RANGE_PARAM3, 0x19f81a38
	dsc->dsc_info.rc_range[6].min_qp = 7;
	dsc->dsc_info.rc_range[6].max_qp = 11;
	dsc->dsc_info.rc_range[6].offset = 56;
	dsc->dsc_info.rc_range[7].min_qp = 7;
	dsc->dsc_info.rc_range[7].max_qp = 12;
	dsc->dsc_info.rc_range[7].offset = 56;
	// DSC_RC_RANGE_PARAM4, 0x1a781ab6
	dsc->dsc_info.rc_range[8].min_qp = 7;
	dsc->dsc_info.rc_range[8].max_qp = 13;
	dsc->dsc_info.rc_range[8].offset = 56;
	dsc->dsc_info.rc_range[9].min_qp = 7;
	dsc->dsc_info.rc_range[9].max_qp = 14;
	dsc->dsc_info.rc_range[9].offset = 54;
	// DSC_RC_RANGE_PARAM5, 0x2ab62af4
	dsc->dsc_info.rc_range[10].min_qp = 9;
	dsc->dsc_info.rc_range[10].max_qp = 15;
	dsc->dsc_info.rc_range[10].offset = 54;
	dsc->dsc_info.rc_range[11].min_qp = 9;
	dsc->dsc_info.rc_range[11].max_qp = 16;
	dsc->dsc_info.rc_range[11].offset = 52;
	// DSC_RC_RANGE_PARAM6, 0x2af44b34
	dsc->dsc_info.rc_range[12].min_qp = 9;
	dsc->dsc_info.rc_range[12].max_qp = 17;
	dsc->dsc_info.rc_range[12].offset = 52;
	dsc->dsc_info.rc_range[13].min_qp = 11;
	dsc->dsc_info.rc_range[13].max_qp = 17;
	dsc->dsc_info.rc_range[13].offset = 52;
	// DSC_RC_RANGE_PARAM7, 0x63740000
	dsc->dsc_info.rc_range[14].min_qp = 17;
	dsc->dsc_info.rc_range[14].max_qp = 19;
	dsc->dsc_info.rc_range[14].offset = 52;
}
#endif

/* dsi param initialized value from panel spec */
static void mipi_lcd_init_dsi_param(struct dkmd_connector_info *pinfo, struct mipi_panel_info *mipi)
{
	mipi->hsa = 10;
	mipi->hbp = 12;
	mipi->dpi_hsize = 186 + 50;
	mipi->hline_time = 274;
	mipi->vsa = 4;
	mipi->vbp = 16;
	mipi->vfp = 16;

	mipi->pxl_clk_rate = 192 * 1000000UL;
	mipi->dsi_bit_clk = 648;

	mipi->dsi_bit_clk_upt = mipi->dsi_bit_clk;
	mipi->dsi_bit_clk_default = mipi->dsi_bit_clk;

	mipi->pxl_clk_rate_div = 1;
	mipi->dsi_bit_clk_upt_support = 0;

	mipi->clk_post_adjust = 215;
	mipi->lane_nums = DSI_3_LANES;
	mipi->color_mode = DSI_24BITS_1;
	mipi->dsi_version = DSI_1_1_VERSION;
	mipi->phy_mode = CPHY_MODE;
	mipi->vc = 0;
	mipi->max_tx_esc_clk = 10 * 1000000;
	mipi->burst_mode = DSI_BURST_SYNC_PULSES_1;
	mipi->non_continue_en = 1;
}

static int32_t panel_of_device_setup(struct panel_drv_private *priv)
{
	int32_t ret;
	struct dkmd_connector_info *pinfo = &priv->connector_info;
	struct device_node *np = priv->pdev->dev.of_node;

	dpu_pr_info("enter!\n");

	/* Inheritance based processing */
	panel_base_of_device_setup(priv);
	panel_drv_private_data_setup(priv, np);

	pinfo->base.type = PANEL_MIPI_CMD;
	/* 1. config base object info
	 * would be used for framebuffer setup
	 */
	pinfo->base.xres = 1080;
	pinfo->base.yres = 2504;

	/* When calculating DPI needs the following parameters */
	pinfo->base.width = 66;
	pinfo->base.height = 153;

	pinfo->base.type |= PANEL_EXTERNAL;
	// caculate fps by mipi timing para
	pinfo->base.fps = 60;
	dpu_pr_info("panel_type=%#x, name %s", pinfo->base.type, pinfo->base.name);

	/* fold panel parameters */
	pinfo->base.fold_type = 1;

	/* 2. config connector info
	 * would be used for dsi & composer setup
	 */
	mipi_lcd_init_dsi_param(pinfo, &get_primary_connector(pinfo)->mipi);
#ifdef DSC_1_1_EN
	dsc_param_set(pinfo, &get_primary_connector(pinfo)->dsc);
#endif
	/* dsi or composer need this param */
	pinfo->dirty_region_updt_support = 0;
	pinfo->vsync_ctrl_type = 0;

	/* 3. config panel private info
	 * would be used for panel setup
	 */
	pinfo->bl_info.bl_min = 12;
	pinfo->bl_info.bl_max = 4095;
	pinfo->bl_info.bl_default = 2047;

	priv->mipi_brightness_para_type = MIPI_BL_PARA1_DBV8_AND_PARA2_DBV0;
	priv->disp_on_cmds = lcd_display_on_cmds_boe_12;
	priv->disp_on_cmds_len = ARRAY_SIZE(lcd_display_on_cmds_boe_12);

	priv->disp_off_cmds = lcd_display_off_cmds;
	priv->disp_off_cmds_len = ARRAY_SIZE(lcd_display_off_cmds);

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
	dpu_pr_info("exit!\n");
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

panel_device_match_data(boe_6p39_panel_info, PANEL_BOE_6P39_ID, panel_of_device_setup, panel_of_device_release);

MODULE_LICENSE("GPL");
