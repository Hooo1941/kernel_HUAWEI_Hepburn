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

#define DSC_1_1_EN
// define CONFIG_ES

static char g_on_cmd0[124][2] = {
	{0xFE, 0xD2},
	{0x4F, 0x08},
	{0x50, 0x11},
	{0x51, 0xAB},
	{0x52, 0x30},
	{0x53, 0x08},
	{0x54, 0xB0},
	{0x55, 0x04},
	{0x56, 0xE0},
	{0x58, 0x00},
	{0x59, 0x10},
	{0x5A, 0x04},
	{0x5B, 0xE0},
	{0x5C, 0x02},
	{0x5D, 0x00},
	{0x5E, 0x20},
	{0x5F, 0x02},
	{0x60, 0x1D},
	{0x61, 0x00},
	{0x62, 0x11},
	{0x63, 0x0C},
	{0x64, 0x06},
	{0x65, 0x67},
	{0x66, 0x02},
	{0x67, 0xC0},
	{0x68, 0x18},
	{0x69, 0x00},
	{0x6A, 0x10},
	{0x6B, 0xF0},
	{0x6C, 0x07},
	{0x6D, 0x10},
	{0x6E, 0x20},
	{0x6F, 0x00},
	{0x70, 0x06},
	{0x71, 0x0F},
	{0x72, 0x0F},
	{0x73, 0x33},
	{0x74, 0x0E},
	{0x75, 0x1C},
	{0x76, 0x2A},
	{0x77, 0x38},
	{0x78, 0x46},
	{0x79, 0x54},
	{0x7A, 0x62},
	{0x7B, 0x69},
	{0x7C, 0x70},
	{0x7D, 0x77},
	{0x7E, 0x79},
	{0x7F, 0x7B},
	{0x80, 0x7D},
	{0x81, 0x7E},
	{0x82, 0x02},
	{0x83, 0x02},
	{0x84, 0x22},
	{0x85, 0x00},
	{0x86, 0x2A},
	{0x87, 0x40},
	{0x88, 0x2A},
	{0x89, 0xBE},
	{0x8A, 0x3A},
	{0x8B, 0xFC},
	{0x8C, 0x3A},
	{0x8D, 0xFA},
	{0x8E, 0x3A},
	{0x8F, 0xF8},
	{0x90, 0x3B},
	{0x91, 0x38},
	{0x92, 0x3B},
	{0x93, 0x78},
	{0x94, 0x3B},
	{0x95, 0xB6},
	{0x96, 0x4B},
	{0x97, 0xF6},
	{0x98, 0x4C},
	{0x99, 0x34},
	{0x9A, 0x4C},
	{0x9B, 0x74},
	{0x9C, 0x5C},
	{0x9D, 0x74},
	{0x9E, 0x8C},
	{0x9F, 0xF4},
	{0xA2, 0x04},
	{0xA3, 0xE0},
	{0xA4, 0x00},
	{0xA5, 0x00},
	{0xA6, 0x00},
	{0xA7, 0x00},
	{0xA9, 0x00},
	{0xAA, 0x00},
	{0xA0, 0x80},
	{0xFE, 0x98},
	{0x14, 0x01},
	{0xFE, 0x57},
	{0xC2, 0x00},
	{0xFE, 0x84},
	{0xC3, 0x80},
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
	{0xFE, 0xA1},
	{0xCD, 0x00},
	{0xCA, 0x80},
	{0xFE, 0xD4},
	{0x0D, 0x00},
	{0xFE, 0xB6},
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
	0x05, 0x10,
};

static char g_on_cmd6[] = {
	0x06, 0x10,
};

static char g_on_cmd7[] = {
	0x07, 0x10,
};

static char g_on_cmd8[] = {
	0xFE, 0x40,
};

static char g_on_cmd9[] = {
	0xBD, 0x00,
};

static char g_on_cmd10[] = {
	0xFE, 0xD2,
};

static char g_on_cmd11[] = {
	0x35, 0x0F,
};

static char g_on_cmd12[] = {
	0xFE, 0x00,
};

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

static char g_on_cmd14[] = {
	0xC2, 0x08,
};

static char g_on_cmd15[] = {
	0x35, 0x00,
};

static char g_on_cmd16[] = {
	0x11,
};

static char g_on_cmd17[] = {
	0x29,
};

static char g_on_cmd18[] = {
	0x51, 0x00, 0x00,
};
/* Power ON Sequence end */

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

static struct dsi_cmd_desc lcd_display_on_cmds[] = {
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[0]), g_on_cmd0[0]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[1]), g_on_cmd0[1]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[2]), g_on_cmd0[2]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[3]), g_on_cmd0[3]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[4]), g_on_cmd0[4]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[5]), g_on_cmd0[5]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[6]), g_on_cmd0[6]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[7]), g_on_cmd0[7]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[8]), g_on_cmd0[8]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[9]), g_on_cmd0[9]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[10]), g_on_cmd0[10]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[11]), g_on_cmd0[11]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[12]), g_on_cmd0[12]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[13]), g_on_cmd0[13]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[14]), g_on_cmd0[14]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[15]), g_on_cmd0[15]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[16]), g_on_cmd0[16]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[17]), g_on_cmd0[17]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[18]), g_on_cmd0[18]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[19]), g_on_cmd0[19]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[20]), g_on_cmd0[20]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[21]), g_on_cmd0[21]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[22]), g_on_cmd0[22]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[23]), g_on_cmd0[23]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[24]), g_on_cmd0[24]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[25]), g_on_cmd0[25]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[26]), g_on_cmd0[26]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[27]), g_on_cmd0[27]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[28]), g_on_cmd0[28]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[29]), g_on_cmd0[29]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[30]), g_on_cmd0[30]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[31]), g_on_cmd0[31]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[32]), g_on_cmd0[32]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[33]), g_on_cmd0[33]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[34]), g_on_cmd0[34]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[35]), g_on_cmd0[35]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[36]), g_on_cmd0[36]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[37]), g_on_cmd0[37]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[38]), g_on_cmd0[38]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[39]), g_on_cmd0[39]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[40]), g_on_cmd0[40]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[41]), g_on_cmd0[41]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[42]), g_on_cmd0[42]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[43]), g_on_cmd0[43]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[44]), g_on_cmd0[44]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[45]), g_on_cmd0[45]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[46]), g_on_cmd0[46]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[47]), g_on_cmd0[47]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[48]), g_on_cmd0[48]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[49]), g_on_cmd0[49]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[50]), g_on_cmd0[50]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[51]), g_on_cmd0[51]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[52]), g_on_cmd0[52]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[53]), g_on_cmd0[53]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[54]), g_on_cmd0[54]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[55]), g_on_cmd0[55]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[56]), g_on_cmd0[56]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[57]), g_on_cmd0[57]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[58]), g_on_cmd0[58]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[59]), g_on_cmd0[59]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[60]), g_on_cmd0[60]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[61]), g_on_cmd0[61]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[62]), g_on_cmd0[62]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[63]), g_on_cmd0[63]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[64]), g_on_cmd0[64]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[65]), g_on_cmd0[65]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[66]), g_on_cmd0[66]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[67]), g_on_cmd0[67]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[68]), g_on_cmd0[68]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[69]), g_on_cmd0[69]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[70]), g_on_cmd0[70]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[71]), g_on_cmd0[71]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[72]), g_on_cmd0[72]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[73]), g_on_cmd0[73]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[74]), g_on_cmd0[74]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[75]), g_on_cmd0[75]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[76]), g_on_cmd0[76]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[77]), g_on_cmd0[77]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[78]), g_on_cmd0[78]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[79]), g_on_cmd0[79]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[80]), g_on_cmd0[80]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[81]), g_on_cmd0[81]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[82]), g_on_cmd0[82]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[83]), g_on_cmd0[83]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[84]), g_on_cmd0[84]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[85]), g_on_cmd0[85]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[86]), g_on_cmd0[86]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[87]), g_on_cmd0[87]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[88]), g_on_cmd0[88]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[89]), g_on_cmd0[89]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[90]), g_on_cmd0[90]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[91]), g_on_cmd0[91]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[92]), g_on_cmd0[92]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[93]), g_on_cmd0[93]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[94]), g_on_cmd0[94]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[95]), g_on_cmd0[95]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[96]), g_on_cmd0[96]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[97]), g_on_cmd0[97]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[98]), g_on_cmd0[98]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[99]), g_on_cmd0[99]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[100]), g_on_cmd0[100]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[101]), g_on_cmd0[101]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[102]), g_on_cmd0[102]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[103]), g_on_cmd0[103]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[104]), g_on_cmd0[104]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[105]), g_on_cmd0[105]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[106]), g_on_cmd0[106]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[107]), g_on_cmd0[107]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[108]), g_on_cmd0[108]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[109]), g_on_cmd0[109]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[110]), g_on_cmd0[110]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[111]), g_on_cmd0[111]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[112]), g_on_cmd0[112]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[113]), g_on_cmd0[113]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[114]), g_on_cmd0[114]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[115]), g_on_cmd0[115]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[116]), g_on_cmd0[116]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[117]), g_on_cmd0[117]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[118]), g_on_cmd0[118]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[119]), g_on_cmd0[119]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[120]), g_on_cmd0[120]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[121]), g_on_cmd0[121]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[122]), g_on_cmd0[122]},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0[123]), g_on_cmd0[123]},

	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd1), g_on_cmd1},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd2), g_on_cmd2},

	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd3), g_on_cmd3},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd4), g_on_cmd4},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd5), g_on_cmd5},

	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd6), g_on_cmd6},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd7},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd8), g_on_cmd8},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd9), g_on_cmd9},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd10), g_on_cmd10},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd11), g_on_cmd11},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd12), g_on_cmd12},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd13), g_on_cmd13},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd14), g_on_cmd14},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd15), g_on_cmd15},
	{DTYPE_DCS_WRITE, 0, 120, WAIT_TYPE_MS, sizeof(g_on_cmd16), g_on_cmd16},
	{DTYPE_DCS_WRITE, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd17), g_on_cmd17},
	{DTYPE_DCS_LWRITE, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd18), g_on_cmd18},

#ifdef BIST_MODE
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_bist_cmd0), g_bist_cmd0},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_bist_cmd1), g_bist_cmd1},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_bist_cmd2), g_bist_cmd2},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_bist_cmd3), g_bist_cmd3},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_bist_cmd4), g_bist_cmd4},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_bist_cmd5), g_bist_cmd5},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_bist_cmd6), g_bist_cmd6},
#endif
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
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 10, GPIO_AMOLED_SW_NAME, &g_gpio_amoled_sw, 0},
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
#ifdef CONFIG_ES
	g_gpio_amoled_sw = 253;
#else
	g_gpio_amoled_sw = 335;
#endif

	dpu_pr_info("used gpio:[rst: %d, vcc3v1: %d, vcc1v8: %d,vcc1v2: %d,te0: %d]\n",
		g_gpio_amoled_reset, g_gpio_amoled_vcc3v1, g_gpio_amoled_vcc1v8, g_gpio_amoled_vcc1v2, g_gpio_amoled_te0);

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
	dsc->dsc_info.pic_height = 2224;
	dsc->dsc_info.pic_width = 2496;
	dsc->dsc_info.slice_height = 16;
	dsc->dsc_info.slice_width = 1248;
	dsc->dsc_info.chunk_size = 0x04e0;
	dsc->dsc_info.initial_dec_delay = 0x0370;
	dsc->dsc_info.initial_scale_value = 0x20;
	dsc->dsc_info.scale_increment_interval = 0x021d;
	dsc->dsc_info.scale_decrement_interval = 0x011;
	dsc->dsc_info.nfl_bpg_offset = 0x0667;
	dsc->dsc_info.slice_bpg_offset = 0x02c0;
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
	dsc->dsc_info.rc_range[9].min_qp = 8;
	dsc->dsc_info.rc_range[9].max_qp = 14;
	dsc->dsc_info.rc_range[9].offset = 54;
	// DSC_RC_RANGE_PARAM5, 0x2ab62af4
	dsc->dsc_info.rc_range[10].min_qp = 9;
	dsc->dsc_info.rc_range[10].max_qp = 14;
	dsc->dsc_info.rc_range[10].offset = 54;
	dsc->dsc_info.rc_range[11].min_qp = 9;
	dsc->dsc_info.rc_range[11].max_qp = 15;
	dsc->dsc_info.rc_range[11].offset = 54;
	// DSC_RC_RANGE_PARAM6, 0x2af44b34
	dsc->dsc_info.rc_range[12].min_qp = 9;
	dsc->dsc_info.rc_range[12].max_qp = 15;
	dsc->dsc_info.rc_range[12].offset = 52;
	dsc->dsc_info.rc_range[13].min_qp = 12;
	dsc->dsc_info.rc_range[13].max_qp = 16;
	dsc->dsc_info.rc_range[13].offset = 52;
	// DSC_RC_RANGE_PARAM7, 0x63740000
	dsc->dsc_info.rc_range[14].min_qp = 16;
	dsc->dsc_info.rc_range[14].max_qp = 17;
	dsc->dsc_info.rc_range[14].offset = 52;
}
#endif

/* dsi param initialized value from panel spec */
static void mipi_lcd_init_dsi_param(struct dkmd_connector_info *pinfo, struct mipi_panel_info *mipi)
{
	mipi->hsa = 15;
	mipi->hbp = 15;
	mipi->dpi_hsize = 185;
	mipi->hline_time = 255;
	mipi->vsa = 18;
	mipi->vbp = 70;
	mipi->vfp = 28;

	mipi->pxl_clk_rate = 192 * 1000000UL;
	mipi->dsi_bit_clk = 257;

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

	/* 1. config base object info
	 * would be used for framebuffer setup
	 */
	pinfo->base.xres = 2496;
	pinfo->base.yres = 2224;

	/* When calculating DPI needs the following parameters */
	pinfo->base.width = 158;
	pinfo->base.height = 141;

	// TODO: caculate fps by mipi timing para
	pinfo->base.fps = 60;

	pinfo->base.lcd_te_idx = 1;
	/* fold panel parameters */
	pinfo->base.fold_type = 1;
	dpu_pr_info("panel_type=%#x, name %s", pinfo->base.type, pinfo->base.name);

	/* 2. config connector info
	 * would be used for dsi & composer setup
	 */
	mipi_lcd_init_dsi_param(pinfo, &get_primary_connector(pinfo)->mipi);
#ifdef DSC_1_1_EN
	dsc_param_set(pinfo, &get_primary_connector(pinfo)->dsc);
#endif

	pinfo->dirty_region_updt_support = 0;
	pinfo->vsync_ctrl_type = 0;

	/* 3. config panel private info
	 * would be used for panel setup
	 */
	pinfo->bl_info.bl_min = 12;
	pinfo->bl_info.bl_max = 4095;
	pinfo->bl_info.bl_default = 2047;
	priv->mipi_brightness_para_type = MIPI_BL_PARA1_DBV8_AND_PARA2_DBV0;
	priv->disp_on_cmds = lcd_display_on_cmds;
	priv->disp_on_cmds_len = ARRAY_SIZE(lcd_display_on_cmds);
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

panel_device_match_data(boe7p847_panel_info, PANEL_BOE_7P847_ID, panel_of_device_setup, panel_of_device_release);

MODULE_LICENSE("GPL");
