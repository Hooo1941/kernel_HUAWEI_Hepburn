/* Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
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

/* if test other fuction, please open:
	"#define BIST_MODE" or "#define ENBALE_DSC" or "#define OP_INITIAL_CODE_ONLY" */
#define NSV_ASIC
/*******************************************************************************
 ** Power ON/OFF Sequence(sleep mode to Normal mode) begin
 */

//////////////Power on initial code start/////////////////////////

// adjust demura always on
static char g_lcdkit_on_cmd_0[] = {
	0xF0, 0x55, 0xAA, 0x52, 0x08, 0x00,
};
static char g_lcdkit_on_cmd_1[] = {
	0x6F, 0x01,
};
static char g_lcdkit_on_cmd_2[] = {
	0xC0, 0xB3,
};
static char g_lcdkit_on_cmd_3[] = {
	0x6F, 0x11,
};
static char g_lcdkit_on_cmd_4[] = {
	0xC0, 0x30,
};

// adjust source power-on sequence
static char g_lcdkit_on_cmd_5[] = {
	0xF0, 0x55, 0xAA, 0x52, 0x08, 0x01,
};
static char g_lcdkit_on_cmd_6[] = {
	0x6F, 0x10,
};
static char g_lcdkit_on_cmd_7[] = {
	0xD8, 0x1F,
};

// lower DDIC power
static char g_lcdkit_on_cmd_8[] = {
	0xF0, 0x55, 0xAA, 0x52, 0x08, 0x00,
};
static char g_lcdkit_on_cmd_9[] = {
	0xC3, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0xFF,
};

static char g_lcdkit_on_cmd_10[] = {
	0xF0, 0x55, 0xAA, 0x52, 0x08, 0x03,
};
static char g_lcdkit_on_cmd_11[] = {
	0x6F, 0x1E,
};
static char g_lcdkit_on_cmd_12[] = {
	0xB0, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xF0,
};
static char g_lcdkit_on_cmd_13[] = {
	0x6F, 0x24,
};
static char g_lcdkit_on_cmd_14[] = {
	0xB0, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xF0,
};
static char g_lcdkit_on_cmd_15[] = {
	0x6F, 0x2A,
};
static char g_lcdkit_on_cmd_16[] = {
	0xB0, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xF0,
};
static char g_lcdkit_on_cmd_17[] = {
	0x6F, 0x30,
};
static char g_lcdkit_on_cmd_18[] = {
	0xB0, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xF0,
};
static char g_lcdkit_on_cmd_19[] = {
	0x6F, 0x36,
};
static char g_lcdkit_on_cmd_20[] = {
	0xB0, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xF0,
};

static char g_lcdkit_on_cmd_21[] = {
	0xF0, 0x55, 0xAA, 0x52, 0x08, 0x01,
};
static char g_lcdkit_on_cmd_22[] = {
	0xB3, 0x12, 0x05, 0x22, 0x22,
};

static char g_lcdkit_on_cmd_23[] = {
	0xF0, 0x55, 0xAA, 0x52, 0x08, 0x01,
};
static char g_lcdkit_on_cmd_24[] = {
	0xB4, 0x02, 0x05, 0x22, 0x22,
};

static char g_lcdkit_on_cmd_25[] = {
	0xF0, 0x55, 0xAA, 0x52, 0x08, 0x00,
};
static char g_lcdkit_on_cmd_26[] = {
	0xC6, 0x22, 0x22, 0x22, 0x22, 0x00, 0x02, 0x00, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22,
};
static char g_lcdkit_on_cmd_27[] = {
	0xFF, 0xAA, 0x55, 0xA5, 0x81,
};
static char g_lcdkit_on_cmd_28[] = {
	0x6F, 0x0C,
};
static char g_lcdkit_on_cmd_29[] = {
	0xF3, 0x3F,
};

// lvd test code
static char g_lcdkit_on_cmd_30[] = {
	0xF0, 0x55, 0xAA, 0x52, 0x08, 0x01,
};
static char g_lcdkit_on_cmd_31[] = {
	0xC7, 0xA0,
};
static char g_lcdkit_on_cmd_32[] = {
	0x6F, 0x03,
};
static char g_lcdkit_on_cmd_33[] = {
	0xC7, 0x18,
};

// DDIC spr
static char g_lcdkit_on_cmd_34[] = {
	0xF0, 0x55, 0xAA, 0x52, 0x08, 0x07,
};
static char g_lcdkit_on_cmd_35[] = {
	0xB3, 0x00,
};

// elvss power-on delay 2 frames
static char g_lcdkit_on_cmd_36[] = {
	0xF0, 0x55, 0xAA, 0x52, 0x08, 0x01,
};
static char g_lcdkit_on_cmd_37[] = {
	0xCD, 0x05, 0x51,
};

// pmic param config
static char g_lcdkit_on_cmd_38[] = {
	0xF0, 0x55, 0xAA, 0x52, 0x08, 0x00,
};
static char g_lcdkit_on_cmd_39[] = {
	0x6F, 0x08,
};
static char g_lcdkit_on_cmd_40[] = {
	0xB5, 0x50,
};

/////////////////////////////////////////////////////
// Manufacture Command Enable, page disable
static char g_on_cmd0[] = {
	0xF0, 0x55, 0xAA, 0x52, 0x00, 0x00,
};

// CMD3, 0xFF??
static char g_on_cmd1[] = {
	0xFF, 0xAA, 0x55, 0xA5, 0x80,
};

// Manufacture Command Enable, Page 4
static char g_on_cmd2[] = {
	0xF0, 0x55, 0xAA, 0x52, 0x08, 0x04,
};

// Set Index of Parameter for MIPI
static char g_on_cmd3[] = {
	0x6F, 0x20
};

// ???
static char g_on_cmd4[] = {
	0xF7, 0x32,
};

// CMD4, 0xFF??
static char g_on_cmd5[] = {
	0xFF, 0xAA, 0x55, 0xA5, 0x81,
};

static char g_on_cmd6[] = {
	0xFF, 0xAA, 0x55, 0xA5, 0x00,
};

// tearing effect on
static char g_on_cmd7[] = {
	0x35,
};

// wirte CTRL display
static char g_on_cmd8[] = {
	0x53, 0x20,
};

// write display brightness
static char g_on_cmd9[] = {
	0x51, 0x00, 0x00, 0x00, 0x00,
};

// column address set (X=1223)
static char g_on_cmd10[] = {
	0x2A, 0x00, 0x00, 0x04, 0xC7,
};

// row address set(Y=2699)
static char g_on_cmd11[] = {
	0x2B, 0x00, 0x00, 0x0A, 0x8B,
};

#ifdef ENBALE_DSC
// PPS table
static char g_on_cmd12[] = {
	0x91,
	// 0xAB, 0x28, 0x00, 0x0C, 0xC2, 0x00, 0x03, 0x6A, 0x01, 0x8E, 0x00, 0x11, 0x08, 0xBB, 0x03, 0xB4, 0x10, 0xF0,
	// dsc1.1, height:60,
	// 0xAB, 0x28, 0x00, 0x3C, 0xC2, 0x00, 0x02, 0x32, 0x06, 0x31, 0x00, 0x08, 0x01, 0xA1, 0x01, 0x7F, 0x10, 0xF0,
	// dsc1.1, height:60, same with dss, 3.75x
	0xAB, 0x28, 0x00, 0x3C, 0xF2, 0x00, 0x02, 0x7F, 0x05, 0x7B, 0x00, 0x08, 0x02, 0x09, 0x01, 0x7F, 0x10, 0xF0,
	// dsc1.1, height:60, same with dss, 3x
	// 0x8B, 0x28, 0x00, 0x3C, 0xF2, 0x00, 0x02, 0x7F, 0x05, 0x7B, 0x00, 0x08, 0x02, 0x09, 0x01, 0x7F, 0x10, 0xF0,
};

// Data Compression Method Selection
static char g_on_cmd13[] = {
	0x90, 0x11,
};

// Compression Mode
static char g_on_cmd14[] = {
	0x03, 0x01,
};
#else
// Data Compression Method Selection
static char g_on_cmd13[] = {
	0x90, 0x02,
};

// Compression Mode
static char g_on_cmd14[] = {
	0x03, 0x00,
};
#endif

// Memory write
static char g_on_cmd15[] = {
	0x2C
};

// ELVSS swire offset
static char g_on_cmd16[] = {
	0x82, 0xAA,
};

// Frame Rate Control
#ifdef DFR_90
static char g_on_cmd17[] = {
	0x2F, 0x02,
};
#else
static char g_on_cmd17[] = {
	0x2F, 0x01,
};
#endif

#ifndef BIST_MODE
// sleep out
static char g_on_cmd18[] = {
	0x11
};

// display on
static char g_on_cmd19[] = {
	0x29
};

#else

// CMD2 Page7 enable, ???
static char g_bist_mode_0[] = {
	0xF0, 0x55, 0xAA, 0x52, 0x08, 0x07,
};

// Round IP disable, ???
static char g_bist_mode_1[] = {
	0xC0, 0x00,
};

// CMD2 Page0 enable
static char g_bist_mode_2[] = {
	0xF0, 0x55, 0xAA, 0x52, 0x08, 0x00,
};

// free-run, W, Black, Red, Green, Blue
static char g_bist_mode_3[] = {
	0xEF, 0x01, 0x00, 0xFF, 0xFF, 0xFF, 0x1F, 0xFE, 0x05, 0x00, 0x01, 0x02, 0x03, 0x04,
};

// BIST mode on
static char g_bist_mode_4[] = {
	0xEE, 0x87, 0x78, 0x02, 0x40,
};
#endif

static char g_bl_level[] = {
	0x51, 0xf, 0xff,
};

//////////////Power on initial code end/////////////////////////


//////////////Power off code start/////////////////////////
static char g_display_off[] = {
	0x28,
};

static char g_enter_sleep[] = {
	0x10,
};
//////////////Power off code end/////////////////////////


static struct dsi_cmd_desc g_lcd_display_on_cmds[] = {
#ifndef OP_INITIAL_CODE_ONLY
	{ DTYPE_DCS_LWRITE, 0, 10, WAIT_TYPE_US, sizeof(g_lcdkit_on_cmd_0), g_lcdkit_on_cmd_0 },
	{ DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_lcdkit_on_cmd_1), g_lcdkit_on_cmd_1 },
	{ DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_lcdkit_on_cmd_2), g_lcdkit_on_cmd_2 },
	{ DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_lcdkit_on_cmd_3), g_lcdkit_on_cmd_3 },
	{ DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_lcdkit_on_cmd_4), g_lcdkit_on_cmd_4 },
	{ DTYPE_DCS_LWRITE, 0, 10, WAIT_TYPE_US, sizeof(g_lcdkit_on_cmd_5), g_lcdkit_on_cmd_5 },
	{ DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_lcdkit_on_cmd_6), g_lcdkit_on_cmd_6 },
	{ DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_lcdkit_on_cmd_7), g_lcdkit_on_cmd_7 },
	{ DTYPE_DCS_LWRITE, 0, 10, WAIT_TYPE_US, sizeof(g_lcdkit_on_cmd_8), g_lcdkit_on_cmd_8 },
	{ DTYPE_DCS_LWRITE, 0, 10, WAIT_TYPE_US, sizeof(g_lcdkit_on_cmd_9), g_lcdkit_on_cmd_9 },
	{ DTYPE_DCS_LWRITE, 0, 10, WAIT_TYPE_US, sizeof(g_lcdkit_on_cmd_10), g_lcdkit_on_cmd_10 },
	{ DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_lcdkit_on_cmd_11), g_lcdkit_on_cmd_11 },
	{ DTYPE_DCS_LWRITE, 0, 10, WAIT_TYPE_US, sizeof(g_lcdkit_on_cmd_12), g_lcdkit_on_cmd_12 },
	{ DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_lcdkit_on_cmd_13), g_lcdkit_on_cmd_13 },
	{ DTYPE_DCS_LWRITE, 0, 10, WAIT_TYPE_US, sizeof(g_lcdkit_on_cmd_14), g_lcdkit_on_cmd_14 },
	{ DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_lcdkit_on_cmd_15), g_lcdkit_on_cmd_15 },
	{ DTYPE_DCS_LWRITE, 0, 10, WAIT_TYPE_US, sizeof(g_lcdkit_on_cmd_16), g_lcdkit_on_cmd_16 },
	{ DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_lcdkit_on_cmd_17), g_lcdkit_on_cmd_17 },
	{ DTYPE_DCS_LWRITE, 0, 10, WAIT_TYPE_US, sizeof(g_lcdkit_on_cmd_18), g_lcdkit_on_cmd_18 },
	{ DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_lcdkit_on_cmd_19), g_lcdkit_on_cmd_19 },
	{ DTYPE_DCS_LWRITE, 0, 10, WAIT_TYPE_US, sizeof(g_lcdkit_on_cmd_20), g_lcdkit_on_cmd_20 },
	{ DTYPE_DCS_LWRITE, 0, 10, WAIT_TYPE_US, sizeof(g_lcdkit_on_cmd_21), g_lcdkit_on_cmd_21 },
	{ DTYPE_DCS_LWRITE, 0, 10, WAIT_TYPE_US, sizeof(g_lcdkit_on_cmd_22), g_lcdkit_on_cmd_22 },
	{ DTYPE_DCS_LWRITE, 0, 10, WAIT_TYPE_US, sizeof(g_lcdkit_on_cmd_23), g_lcdkit_on_cmd_23 },
	{ DTYPE_DCS_LWRITE, 0, 10, WAIT_TYPE_US, sizeof(g_lcdkit_on_cmd_24), g_lcdkit_on_cmd_24 },
	{ DTYPE_DCS_LWRITE, 0, 10, WAIT_TYPE_US, sizeof(g_lcdkit_on_cmd_25), g_lcdkit_on_cmd_25 },
	{ DTYPE_DCS_LWRITE, 0, 10, WAIT_TYPE_US, sizeof(g_lcdkit_on_cmd_26), g_lcdkit_on_cmd_26 },
	{ DTYPE_DCS_LWRITE, 0, 10, WAIT_TYPE_US, sizeof(g_lcdkit_on_cmd_27), g_lcdkit_on_cmd_27 },
	{ DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_lcdkit_on_cmd_28), g_lcdkit_on_cmd_28 },
	{ DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_lcdkit_on_cmd_29), g_lcdkit_on_cmd_29 },
	{ DTYPE_DCS_LWRITE, 0, 10, WAIT_TYPE_US, sizeof(g_lcdkit_on_cmd_30), g_lcdkit_on_cmd_30 },
	{ DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_lcdkit_on_cmd_31), g_lcdkit_on_cmd_31 },
	{ DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_lcdkit_on_cmd_32), g_lcdkit_on_cmd_32 },
	{ DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_lcdkit_on_cmd_33), g_lcdkit_on_cmd_33 },
	{ DTYPE_DCS_LWRITE, 0, 10, WAIT_TYPE_US, sizeof(g_lcdkit_on_cmd_34), g_lcdkit_on_cmd_34 },
	{ DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_lcdkit_on_cmd_35), g_lcdkit_on_cmd_35 },
	{ DTYPE_DCS_LWRITE, 0, 10, WAIT_TYPE_US, sizeof(g_lcdkit_on_cmd_36), g_lcdkit_on_cmd_36 },
	{ DTYPE_DCS_LWRITE, 0, 10, WAIT_TYPE_US, sizeof(g_lcdkit_on_cmd_37), g_lcdkit_on_cmd_37 },
#endif
	{ DTYPE_DCS_LWRITE, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0),   g_on_cmd0 },
	{ DTYPE_DCS_LWRITE, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd1),   g_on_cmd1 },
	{ DTYPE_DCS_LWRITE, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd2),   g_on_cmd2 },
	{ DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd3),   g_on_cmd3 },
	{ DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd4),   g_on_cmd4 },
	{ DTYPE_DCS_LWRITE, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd5),   g_on_cmd5 },
	{ DTYPE_DCS_LWRITE, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd6),   g_on_cmd6 },
	{ DTYPE_DCS_WRITE, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7),   g_on_cmd7 },
	{ DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd8),   g_on_cmd8 },
	{ DTYPE_DCS_LWRITE, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd9),   g_on_cmd9 },
	{ DTYPE_DCS_LWRITE, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd10), g_on_cmd10 },
	{ DTYPE_DCS_LWRITE, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd11), g_on_cmd11 },
#ifdef ENBALE_DSC
	{ DTYPE_DCS_LWRITE, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd12), g_on_cmd12 },
#endif
	{ DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd13), g_on_cmd13 },
	{ DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd14), g_on_cmd14 },
	{ DTYPE_DCS_WRITE, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd15), g_on_cmd15 },
	{ DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd16), g_on_cmd16 },
	{ DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd17), g_on_cmd17 },
#ifndef OP_INITIAL_CODE_ONLY
	{ DTYPE_DCS_LWRITE, 0, 10, WAIT_TYPE_US, sizeof(g_lcdkit_on_cmd_38), g_lcdkit_on_cmd_38 },
	{ DTYPE_DCS_LWRITE, 0, 10, WAIT_TYPE_US, sizeof(g_lcdkit_on_cmd_39), g_lcdkit_on_cmd_39 },
	{ DTYPE_DCS_LWRITE, 0, 10, WAIT_TYPE_US, sizeof(g_lcdkit_on_cmd_40), g_lcdkit_on_cmd_40 },
#endif
#ifndef BIST_MODE
	{ DTYPE_DCS_WRITE, 0, 150, WAIT_TYPE_MS, sizeof(g_on_cmd18), g_on_cmd18 },
	{ DTYPE_DCS_WRITE, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd19), g_on_cmd19 },
	{ DTYPE_DCS_LWRITE, 0, 20, WAIT_TYPE_US, sizeof(g_bl_level), g_bl_level },
#else
	{ DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_MS, sizeof(g_bist_mode_0), g_bist_mode_0 },
	{ DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_MS, sizeof(g_bist_mode_1), g_bist_mode_1 },
	{ DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_MS, sizeof(g_bist_mode_2), g_bist_mode_2 },
	{ DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_MS, sizeof(g_bist_mode_3), g_bist_mode_3 },
	{ DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_MS, sizeof(g_bist_mode_4), g_bist_mode_4 },
#endif

};

static struct dsi_cmd_desc g_lcd_display_off_cmds[] = {
	{ DTYPE_DCS_WRITE, 0, 0, WAIT_TYPE_MS, sizeof(g_display_off), g_display_off },
	{ DTYPE_DCS_WRITE, 0, 120, WAIT_TYPE_MS, sizeof(g_enter_sleep), g_enter_sleep },
};

/*******************************************************************************
 ** LCD GPIO
 */
#define GPIO_AMOLED_RESET_NAME "gpio_amoled_reset"
#define GPIO_AMOLED_VCC1V8_NAME "gpio_amoled_vcc1v8"
#define GPIO_AMOLED_VCC1V2_NAME "gpio_amoled_vcc1v2"
#define GPIO_AMOLED_VCC3V1_NAME "gpio_amoled_vcc3v1"
#define GPIO_AMOLED_TE0_NAME "gpio_amoled_te0"

static uint32_t g_gpio_amoled_reset;
static uint32_t g_gpio_amoled_vcc1v8;
static uint32_t g_gpio_amoled_vcc1v2;
static uint32_t g_gpio_amoled_vcc3v1;
static uint32_t g_gpio_amoled_te0;


static struct gpio_desc g_fpga_lcd_gpio_request_cmds[] = {
	/* vcc3v1 */
	{ DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0, GPIO_AMOLED_VCC3V1_NAME, &g_gpio_amoled_vcc3v1, 0 },
	/* vcc1v8 */
	{ DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0, GPIO_AMOLED_VCC1V8_NAME, &g_gpio_amoled_vcc1v8, 0 },

	/* reset */
	{ DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0, GPIO_AMOLED_RESET_NAME, &g_gpio_amoled_reset, 0 },
};

static struct gpio_desc g_fpga_lcd_gpio_normal_cmds[] = {
	/* vcc1v8 enable */
	{ DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 2, GPIO_AMOLED_VCC1V8_NAME, &g_gpio_amoled_vcc1v8, 1 },
	/* vcc3v1 enable */
	{ DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 15, GPIO_AMOLED_VCC3V1_NAME, &g_gpio_amoled_vcc3v1, 1 },
	/* reset */
	{ DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 5, GPIO_AMOLED_RESET_NAME, &g_gpio_amoled_reset, 1 },
	{ DTYPE_GPIO_OUTPUT, WAIT_TYPE_US, 50, GPIO_AMOLED_RESET_NAME, &g_gpio_amoled_reset, 0 },
	{ DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 15, GPIO_AMOLED_RESET_NAME, &g_gpio_amoled_reset, 1 },
	/* backlight enable */
};

static struct gpio_desc g_fpga_lcd_gpio_lowpower_cmds[] = {
	/* vcc1v8 disable */
	{ DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 2, GPIO_AMOLED_VCC1V8_NAME, &g_gpio_amoled_vcc1v8, 0 },
	/* vcc3v1 disable */
	{ DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 15, GPIO_AMOLED_VCC3V1_NAME, &g_gpio_amoled_vcc3v1, 0 },
	/* reset */
	{ DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 5, GPIO_AMOLED_RESET_NAME, &g_gpio_amoled_reset, 0 },
};

static struct gpio_desc g_fpga_lcd_gpio_free_cmds[] = {
	/* vcc3v1 */
	{ DTYPE_GPIO_FREE, WAIT_TYPE_MS, 0, GPIO_AMOLED_VCC3V1_NAME, &g_gpio_amoled_vcc3v1, 0 },
	/* vcc1v8 */
	{ DTYPE_GPIO_FREE, WAIT_TYPE_MS, 0, GPIO_AMOLED_VCC1V8_NAME, &g_gpio_amoled_vcc1v8, 0 },
	/* reset */
	{ DTYPE_GPIO_FREE, WAIT_TYPE_MS, 0, GPIO_AMOLED_RESET_NAME, &g_gpio_amoled_reset, 0 },
};

#ifdef NSV_ASIC
static struct gpio_desc g_asic_lcd_gpio_request_cmds[] = {
	{ DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0, GPIO_AMOLED_VCC1V2_NAME, &g_gpio_amoled_vcc1v2, 0},
	{ DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0, GPIO_AMOLED_VCC3V1_NAME, &g_gpio_amoled_vcc3v1, 0 },
	{ DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0, GPIO_AMOLED_RESET_NAME, &g_gpio_amoled_reset, 0 },
};

static struct gpio_desc g_asic_lcd_gpio_free_cmds[] = {
	{ DTYPE_GPIO_FREE, WAIT_TYPE_MS, 0, GPIO_AMOLED_VCC1V2_NAME, &g_gpio_amoled_vcc1v2, 0},
	{ DTYPE_GPIO_FREE, WAIT_TYPE_US, 50, GPIO_AMOLED_VCC3V1_NAME, &g_gpio_amoled_vcc3v1, 0 },
	{ DTYPE_GPIO_FREE, WAIT_TYPE_US, 50, GPIO_AMOLED_RESET_NAME, &g_gpio_amoled_reset, 0 },
};

static struct gpio_desc g_asic_lcd_gpio_normal_cmds[] = {
	{ DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 2, GPIO_AMOLED_VCC1V2_NAME, &g_gpio_amoled_vcc1v2, 1},
	{ DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 20, GPIO_AMOLED_VCC3V1_NAME, &g_gpio_amoled_vcc3v1, 1 },
	{ DTYPE_GPIO_OUTPUT, WAIT_TYPE_US, 50, GPIO_AMOLED_RESET_NAME, &g_gpio_amoled_reset, 1 },
	{ DTYPE_GPIO_OUTPUT, WAIT_TYPE_US, 50, GPIO_AMOLED_RESET_NAME, &g_gpio_amoled_reset, 0 },
	{ DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 15, GPIO_AMOLED_RESET_NAME, &g_gpio_amoled_reset, 1 },
};

static struct gpio_desc g_asic_lcd_gpio_lowpower_cmds[] = {
	{ DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 20, GPIO_AMOLED_RESET_NAME, &g_gpio_amoled_reset, 0 },
	{ DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 20, GPIO_AMOLED_RESET_NAME, &g_gpio_amoled_vcc3v1, 0 },
	{ DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 10, GPIO_AMOLED_VCC1V2_NAME, &g_gpio_amoled_vcc1v2, 0},
};
#endif
/*******************************************************************************
 ** LCD VCC
 */
#define VCC_LCDIO_NAME	"lcdio-vcc"

static struct regulator *g_vcc_lcdio;

#ifdef NSV_ASIC
static struct vcc_desc g_lcd_vcc_init_cmds[] = {
	/* vcc get */
	{DTYPE_VCC_GET, VCC_LCDIO_NAME, &g_vcc_lcdio, 0, 0, WAIT_TYPE_MS, 0},

	/* vcc set voltage */
	/* io set voltage */
	{ DTYPE_VCC_SET_VOLTAGE, VCC_LCDIO_NAME, &g_vcc_lcdio, 1850000, 1850000, WAIT_TYPE_MS, 0 },
};

static struct vcc_desc g_lcd_vcc_finit_cmds[] = {
	/* vcc put */
	{ DTYPE_VCC_PUT, VCC_LCDIO_NAME, &g_vcc_lcdio, 0, 0, WAIT_TYPE_MS, 0 },
};

static struct vcc_desc g_lcd_vcc_enable_cmds[] = {
	/* vcc enable */
	{ DTYPE_VCC_ENABLE, VCC_LCDIO_NAME, &g_vcc_lcdio, 0, 0, WAIT_TYPE_MS, 3 },
};

static struct vcc_desc g_lcd_vcc_disable_cmds[] = {
	/* vcc enable */
	{ DTYPE_VCC_DISABLE, VCC_LCDIO_NAME, &g_vcc_lcdio, 0, 0, WAIT_TYPE_MS, 3 },
};
#endif
/*******************************************************************************
 ** LCD IOMUX
 */
static struct pinctrl_data g_pctrl;

static struct pinctrl_cmd_desc g_lcd_pinctrl_init_cmds[] = {
	{DTYPE_PINCTRL_GET, &g_pctrl, 0},
	{DTYPE_PINCTRL_STATE_GET, &g_pctrl, DTYPE_PINCTRL_STATE_DEFAULT},
	{DTYPE_PINCTRL_STATE_GET, &g_pctrl, DTYPE_PINCTRL_STATE_IDLE},
};

static struct pinctrl_cmd_desc g_lcd_pinctrl_normal_cmds[] = {
	{DTYPE_PINCTRL_SET, &g_pctrl, DTYPE_PINCTRL_STATE_DEFAULT},
};

static struct pinctrl_cmd_desc g_lcd_pinctrl_lowpower_cmds[] = {
	{DTYPE_PINCTRL_SET, &g_pctrl, DTYPE_PINCTRL_STATE_IDLE},
};

static struct pinctrl_cmd_desc g_lcd_pinctrl_finit_cmds[] = {
	{DTYPE_PINCTRL_PUT, &g_pctrl, 0},
};

static void panel_drv_private_data_setup(struct panel_drv_private *priv, struct device_node *np)
{
	if (priv->connector_info.base.fpga_flag == 1) {
		g_gpio_amoled_reset = (uint32_t)of_get_named_gpio(np, "gpios", 0);
		g_gpio_amoled_vcc3v1 = (uint32_t)of_get_named_gpio(np, "gpios", 1);
		g_gpio_amoled_vcc1v8 = (uint32_t)of_get_named_gpio(np, "gpios", 2);

		dpu_pr_info("used gpio:[rst: %d, vcc3v1: %d, tp1v8: %d]\n",
			g_gpio_amoled_reset, g_gpio_amoled_vcc3v1, g_gpio_amoled_vcc1v8);

		priv->gpio_request_cmds = g_fpga_lcd_gpio_request_cmds;
		priv->gpio_request_cmds_len = ARRAY_SIZE(g_fpga_lcd_gpio_request_cmds);
		priv->gpio_free_cmds = g_fpga_lcd_gpio_free_cmds;
		priv->gpio_free_cmds_len = ARRAY_SIZE(g_fpga_lcd_gpio_free_cmds);

		priv->gpio_normal_cmds = g_fpga_lcd_gpio_normal_cmds;
		priv->gpio_normal_cmds_len = ARRAY_SIZE(g_fpga_lcd_gpio_normal_cmds);
		priv->gpio_lowpower_cmds = g_fpga_lcd_gpio_lowpower_cmds;
		priv->gpio_lowpower_cmds_len = ARRAY_SIZE(g_fpga_lcd_gpio_lowpower_cmds);
	} else {
		g_gpio_amoled_reset = (uint32_t)of_get_named_gpio(np, "gpios", 0);
		g_gpio_amoled_vcc3v1 = (uint32_t)of_get_named_gpio(np, "gpios", 1);
		g_gpio_amoled_te0 = (uint32_t)of_get_named_gpio(np, "gpios", 2);
	#ifdef NSV_ASIC
		g_gpio_amoled_vcc1v2 = (uint32_t)of_get_named_gpio(np, "gpios", 3);
	#endif
		dpu_pr_info("used gpio:[rst: %d, vcc3v1: %d, te0: %d]\n",
			g_gpio_amoled_reset, g_gpio_amoled_vcc3v1, g_gpio_amoled_te0);

		priv->gpio_request_cmds = g_asic_lcd_gpio_request_cmds;
		priv->gpio_request_cmds_len = ARRAY_SIZE(g_asic_lcd_gpio_request_cmds);
		priv->gpio_free_cmds = g_asic_lcd_gpio_free_cmds;
		priv->gpio_free_cmds_len = ARRAY_SIZE(g_asic_lcd_gpio_free_cmds);

		priv->gpio_normal_cmds = g_asic_lcd_gpio_normal_cmds;
		priv->gpio_normal_cmds_len = ARRAY_SIZE(g_asic_lcd_gpio_normal_cmds);
		priv->gpio_lowpower_cmds = g_asic_lcd_gpio_lowpower_cmds;
		priv->gpio_lowpower_cmds_len = ARRAY_SIZE(g_asic_lcd_gpio_lowpower_cmds);

		priv->vcc_init_cmds = g_lcd_vcc_init_cmds;
		priv->vcc_init_cmds_len = ARRAY_SIZE(g_lcd_vcc_init_cmds);
		priv->vcc_finit_cmds = g_lcd_vcc_finit_cmds;
		priv->vcc_finit_cmds_len = ARRAY_SIZE(g_lcd_vcc_finit_cmds);

		priv->vcc_enable_cmds = g_lcd_vcc_enable_cmds;
		priv->vcc_enable_cmds_len = ARRAY_SIZE(g_lcd_vcc_enable_cmds);
		priv->vcc_disable_cmds = g_lcd_vcc_disable_cmds;
		priv->vcc_disable_cmds_len = ARRAY_SIZE(g_lcd_vcc_disable_cmds);

		priv->pinctrl_init_cmds = g_lcd_pinctrl_init_cmds;
		priv->pinctrl_init_cmds_len = ARRAY_SIZE(g_lcd_pinctrl_init_cmds);
		priv->pinctrl_finit_cmds = g_lcd_pinctrl_finit_cmds;
		priv->pinctrl_finit_cmds_len = ARRAY_SIZE(g_lcd_pinctrl_finit_cmds);

		priv->pinctrl_normal_cmds = g_lcd_pinctrl_normal_cmds;
		priv->pinctrl_normal_cmds_len = ARRAY_SIZE(g_lcd_pinctrl_normal_cmds);
		priv->pinctrl_lowpower_cmds = g_lcd_pinctrl_lowpower_cmds;
		priv->pinctrl_lowpower_cmds_len = ARRAY_SIZE(g_lcd_pinctrl_lowpower_cmds);
	}
}

#ifdef ENBALE_DSC
static void dsc_config(struct dkmd_connector_info *pinfo, struct dsc_calc_info *dsc)
{
	// IFBC_TYPE_VESA2X_SINGLE
	// IFBC_TYPE_VESA2X_DUAL
	// IFBC_TYPE_VESA3X_DUAL;
	// IFBC_TYPE_VESA3X_SINGLE;
	// IFBC_TYPE_VESA3_75X_SINGLE;
	// IFBC_TYPE_VESA3_75X_DUAL
	pinfo->ifbc_type = IFBC_TYPE_VESA3_75X_DUAL;
	dsc->dsc_en = 1;

	dsc->dsc_info.dsc_version_major = 1;
	dsc->dsc_info.dsc_version_minor = 1;
	dsc->dsc_info.pic_height = pinfo->base.yres;
	dsc->dsc_info.pic_width = pinfo->base.xres;
	dsc->dsc_info.dsc_bpc = 10; // 10:3.75x; 8:3x
	dsc->dsc_info.dsc_bpp = 8;
	dsc->dsc_info.slice_height = 60;
	dsc->dsc_info.slice_width = pinfo->base.xres / 2;
	dsc->dsc_info.chunk_size = dsc->dsc_info.dsc_bpp * (dsc->dsc_info.slice_width) / 8;

	/* DSC_CTRL */
	dsc->dsc_info.block_pred_enable = 1;
	dsc->dsc_info.linebuf_depth = 11;

	/* INITIAL_DELAY */
	dsc->dsc_info.initial_dec_delay = 0x027f;
	dsc->dsc_info.initial_xmit_delay = 512;
	/* RC_PARAM0 */
	dsc->dsc_info.initial_scale_value = 0x20;
	dsc->dsc_info.scale_increment_interval = 0x057b;
	/* RC_PARAM1 */
	dsc->dsc_info.scale_decrement_interval = 0x8;
	dsc->dsc_info.first_line_bpg_offset = 15;
	/* RC_PARAM2 */
	dsc->dsc_info.nfl_bpg_offset = 0x209;
	dsc->dsc_info.slice_bpg_offset = 0x17f;
	/* RC_PARAM3 */
	dsc->dsc_info.initial_offset = 6144;
	dsc->dsc_info.final_offset = 0x10f0;
	/* FLATNESS_QP_TH */
	dsc->dsc_info.flatness_min_qp = 7;
	dsc->dsc_info.flatness_max_qp = 16;
	/* DSC_PARAM4 */
	dsc->dsc_info.rc_edge_factor = 0x6;
	dsc->dsc_info.rc_model_size = 8192;
	/* DSC_RC_PARAM5: 0x330f0f */
	dsc->dsc_info.rc_tgt_offset_lo = (0x330f0f >> 20) & 0xF;
	dsc->dsc_info.rc_tgt_offset_hi = (0x330f0f >> 16) & 0xF;
	dsc->dsc_info.rc_quant_incr_limit1 = (0x330f0f >> 8) & 0x1F;
	dsc->dsc_info.rc_quant_incr_limit0 = (0x330f0f >> 0) & 0x1F;
	/* DSC_RC_BUF_THRESH0: 0xe1c2a38 */
	dsc->dsc_info.rc_buf_thresh[0] = (0xe1c2a38 >> 24) & 0xFF;
	dsc->dsc_info.rc_buf_thresh[1] = (0xe1c2a38 >> 16) & 0xFF;
	dsc->dsc_info.rc_buf_thresh[2] = (0xe1c2a38 >> 8) & 0xFF;
	dsc->dsc_info.rc_buf_thresh[3] = (0xe1c2a38 >> 0) & 0xFF;
	/* DSC_RC_BUF_THRESH1: 0x46546269 */
	dsc->dsc_info.rc_buf_thresh[4] = (0x46546269 >> 24) & 0xFF;
	dsc->dsc_info.rc_buf_thresh[5] = (0x46546269 >> 16) & 0xFF;
	dsc->dsc_info.rc_buf_thresh[6] = (0x46546269 >> 8) & 0xFF;
	dsc->dsc_info.rc_buf_thresh[7] = (0x46546269 >> 0) & 0xFF;
	/* DSC_RC_BUF_THRESH2: 0x7077797b */
	dsc->dsc_info.rc_buf_thresh[8] = (0x7077797b >> 24) & 0xFF;
	dsc->dsc_info.rc_buf_thresh[9] = (0x7077797b >> 16) & 0xFF;
	dsc->dsc_info.rc_buf_thresh[10] = (0x7077797b >> 8) & 0xFF;
	dsc->dsc_info.rc_buf_thresh[11] = (0x7077797b >> 0) & 0xFF;
	/* DSC_RC_BUF_THRESH3: 0x7d7e0000 */
	dsc->dsc_info.rc_buf_thresh[12] = (0x7d7e0000 >> 24) & 0xFF;
	dsc->dsc_info.rc_buf_thresh[13] = (0x7d7e0000 >> 16) & 0xFF;
	/* DSC_RC_RANGE_PARAM0: 0x2022200 */
	dsc->dsc_info.rc_range[0].min_qp = (0x2022200 >> 27) & 0x1F;
	dsc->dsc_info.rc_range[0].max_qp = (0x2022200 >> 22) & 0x1F;
	dsc->dsc_info.rc_range[0].offset = (0x2022200 >> 16) & 0x3F;
	dsc->dsc_info.rc_range[1].min_qp = (0x2022200 >> 11) & 0x1F;
	dsc->dsc_info.rc_range[1].max_qp = (0x2022200 >> 6) & 0x1F;
	dsc->dsc_info.rc_range[1].offset = (0x2022200 >> 0) & 0x3F;
	/* DSC_RC_RANGE_PARAM1: 0x2a402abe */
	dsc->dsc_info.rc_range[2].min_qp = (0x2a402abe >> 27) & 0x1F;
	dsc->dsc_info.rc_range[2].max_qp = (0x2a402abe >> 22) & 0x1F;
	dsc->dsc_info.rc_range[2].offset = (0x2a402abe >> 16) & 0x3F;
	dsc->dsc_info.rc_range[3].min_qp = (0x2a402abe >> 11) & 0x1F;
	dsc->dsc_info.rc_range[3].max_qp = (0x2a402abe >> 6) & 0x1F;
	dsc->dsc_info.rc_range[3].offset = (0x2a402abe >> 0) & 0x3F;
	/* DSC_RC_RANGE_PARAM2, 0x3afc3afa */
	dsc->dsc_info.rc_range[4].min_qp = (0x3afc3afa >> 27) & 0x1F;
	dsc->dsc_info.rc_range[4].max_qp = (0x3afc3afa >> 22) & 0x1F;
	dsc->dsc_info.rc_range[4].offset = (0x3afc3afa >> 16) & 0x3F;
	dsc->dsc_info.rc_range[5].min_qp = (0x3afc3afa >> 11) & 0x1F;
	dsc->dsc_info.rc_range[5].max_qp = (0x3afc3afa >> 6) & 0x1F;
	dsc->dsc_info.rc_range[5].offset = (0x3afc3afa >> 0) & 0x3F;
	/* DSC_RC_RANGE_PARAM3, 0x3af83b38 */
	dsc->dsc_info.rc_range[6].min_qp = (0x3af83b38 >> 27) & 0x1F;
	dsc->dsc_info.rc_range[6].max_qp = (0x3af83b38 >> 22) & 0x1F;
	dsc->dsc_info.rc_range[6].offset = (0x3af83b38 >> 16) & 0x3F;
	dsc->dsc_info.rc_range[7].min_qp = (0x3af83b38 >> 11) & 0x1F;
	dsc->dsc_info.rc_range[7].max_qp = (0x3af83b38 >> 6) & 0x1F;
	dsc->dsc_info.rc_range[7].offset = (0x3af83b38 >> 0) & 0x3F;
	/* DSC_RC_RANGE_PARAM4, 0x3b783bb6 */
	dsc->dsc_info.rc_range[8].min_qp = (0x3b783bb6 >> 27) & 0x1F;
	dsc->dsc_info.rc_range[8].max_qp = (0x3b783bb6 >> 22) & 0x1F;
	dsc->dsc_info.rc_range[8].offset = (0x3b783bb6 >> 16) & 0x3F;
	dsc->dsc_info.rc_range[9].min_qp = (0x3b783bb6 >> 11) & 0x1F;
	dsc->dsc_info.rc_range[9].max_qp = (0x3b783bb6 >> 6) & 0x1F;
	dsc->dsc_info.rc_range[9].offset = (0x3b783bb6 >> 0) & 0x3F;
	/* DSC_RC_RANGE_PARAM5, 0x4bf64c34 */
	dsc->dsc_info.rc_range[10].min_qp = (0x4bf64c34 >> 27) & 0x1F;
	dsc->dsc_info.rc_range[10].max_qp = (0x4bf64c34 >> 22) & 0x1F;
	dsc->dsc_info.rc_range[10].offset = (0x4bf64c34 >> 16) & 0x3F;
	dsc->dsc_info.rc_range[11].min_qp = (0x4bf64c34 >> 11) & 0x1F;
	dsc->dsc_info.rc_range[11].max_qp = (0x4bf64c34 >> 6) & 0x1F;
	dsc->dsc_info.rc_range[11].offset = (0x4bf64c34 >> 0) & 0x3F;
	/* DSC_RC_RANGE_PARAM6, 0x4c745c74 */
	dsc->dsc_info.rc_range[12].min_qp = (0x4c745c74 >> 27) & 0x1F;
	dsc->dsc_info.rc_range[12].max_qp = (0x4c745c74 >> 22) & 0x1F;
	dsc->dsc_info.rc_range[12].offset = (0x4c745c74 >> 16) & 0x3F;
	dsc->dsc_info.rc_range[13].min_qp = (0x4c745c74 >> 11) & 0x1F;
	dsc->dsc_info.rc_range[13].max_qp = (0x4c745c74 >> 6) & 0x1F;
	dsc->dsc_info.rc_range[13].offset = (0x4c745c74 >> 0) & 0x3F;
	/* DSC_RC_RANGE_PARAM7, 0x8cf40000 */
	dsc->dsc_info.rc_range[14].min_qp = (0x8cf40000 >> 27) & 0x1F;
	dsc->dsc_info.rc_range[14].max_qp = (0x8cf40000 >> 22) & 0x1F;
	dsc->dsc_info.rc_range[14].offset = (0x8cf40000 >> 16) & 0x3F;
}
#endif

static void mipi_lcd_init_dsi_param(struct dkmd_connector_info *pinfo, struct mipi_panel_info *mipi)
{
	if (pinfo->base.fpga_flag == 1) {
		mipi->hsa = 13;
		mipi->hbp = 5;
		mipi->dpi_hsize = 338;
		mipi->hline_time = 1000;
		mipi->vsa = 30;
		mipi->vbp = 70;
		mipi->vfp = 20;

		mipi->dsi_bit_clk = 160;
		mipi->pxl_clk_rate = 20 * 1000000UL;
	} else {
#ifdef ENABLE_DSC
		dpu_pr_info("[37701_brq probe] udp mipi param set dsc en\n");
		mipi->hsa = 32;
		mipi->hbp = 32;
		mipi->dpi_hsize = 248;
		mipi->hline_time = 1024;
		mipi->vsa = 4;
		mipi->vbp = 12;
		mipi->vfp = 16;

		mipi->dsi_bit_clk = 672;
		mipi->pxl_clk_rate = 168 * 1000000UL;
#else
		dpu_pr_info("[37701_brq probe] udp mipi param set no dsc\n");
		mipi->hsa = 32;
		mipi->hbp = 32;
		mipi->dpi_hsize = 920;
		mipi->hline_time = 1024;
		mipi->vsa = 4;
		mipi->vbp = 12;
		mipi->vfp = 16;

		mipi->dsi_bit_clk = 672;
		mipi->pxl_clk_rate = 168 * 1000000UL;
#endif
	}

	mipi->dsi_bit_clk_upt_support = 0;
	mipi->dsi_bit_clk_upt = mipi->dsi_bit_clk;
	mipi->dsi_bit_clk_default = mipi->dsi_bit_clk;
	mipi->pxl_clk_rate_div = 1;

	mipi->clk_post_adjust = 16;
	mipi->lane_nums = DSI_4_LANES;
	mipi->color_mode = DSI_24BITS_1;

	mipi->vc = 0;
	mipi->max_tx_esc_clk = 10 * 1000000;
	mipi->burst_mode = DSI_BURST_SYNC_PULSES_1;
	mipi->non_continue_en = 1;
	mipi->phy_mode = DPHY_MODE;
	mipi->dsi_version = DSI_1_1_VERSION;
}

static int32_t panel_of_device_setup(struct panel_drv_private *priv)
{
	int32_t ret;
	struct dkmd_connector_info *pinfo = &priv->connector_info;
	struct device_node *np = priv->pdev->dev.of_node;
	struct dpu_connector *connector = NULL;

	connector = get_primary_connector(pinfo);
	if (!connector) {
		dpu_pr_err("connector_id=%u is not available!\n", pinfo->connector_idx[PRIMARY_CONNECT_CHN_IDX]);
		return -1;
	}

	dpu_pr_info("enter!\n");

	/* Inheritance based processing */
	panel_base_of_device_setup(priv);
	panel_drv_private_data_setup(priv, np);
	pinfo->ifbc_type = IFBC_TYPE_NONE;

	/* 1. config base object info
	 * would be used for framebuffer setup
	 */
	pinfo->base.xres = 1224;
	pinfo->base.yres = 2700;

	/* When calculating DPI needs the following parameters */
	pinfo->base.width = 70;
	pinfo->base.height = 155;

	/* report to surfaceFlinger, remain: caculate fps by mipi timing para */
#ifdef DFR120
	pinfo->base.fps = 120;
#else
	pinfo->base.fps = 60;
#endif

	/* 2. config connector info
	 * would be used for dsi & composer setup
	 */
#ifdef ENBALE_DSC
	dsc_config(pinfo, &connector->dsc);
#endif
	mipi_lcd_init_dsi_param(pinfo, &connector->mipi);

	/* dsi or composer need this param */
	pinfo->dirty_region_updt_support = 0;

	/* 3. config panel private info
	 * would be used for panel setup
	 */
	pinfo->bl_info.bl_min = 24;
	pinfo->bl_info.bl_max = 10000;
	pinfo->bl_info.bl_default = 2047;
	priv->mipi_brightness_para_type = MIPI_BL_PARA1_DBV8_AND_PARA2_DBV0;

	priv->disp_on_cmds = g_lcd_display_on_cmds;
	priv->disp_on_cmds_len = (uint32_t)ARRAY_SIZE(g_lcd_display_on_cmds);
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

panel_device_match_data(nt37701a_panel_info, PANEL_NT37701A_ID, panel_of_device_setup, panel_of_device_release);

MODULE_LICENSE("GPL");
