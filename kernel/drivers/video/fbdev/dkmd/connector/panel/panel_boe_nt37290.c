/* Copyright (c) Huawei Technologies Co., Ltd. 2019-2021. All rights reserved.
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

/* Power ON Sequence */
static char g_on_cmd0[] = {
	0xF0, 0x55, 0xAA, 0x52, 0x08, 0x00
};

static char g_on_cmd1[] = {
	0x6F, 0x07
};

static char g_on_cmd2[] = {
	0xBA, 0x00, 0x7C
};

static char g_on_cmd3[] = {
	0x6F, 0x0E
};

static char g_on_cmd4[] = {
	0xBA, 0x00, 0x7C
};

static char g_on_cmd5[] = {
	0xF0, 0x55, 0xAA, 0x52, 0x08, 0x01
};

static char g_on_cmd6[] = {
	0xCD, 0x42
};

static char g_on_cmd7[] = {
	0x6F, 0x01
};

static char g_on_cmd8[] = {
	0xC5, 0x0B, 0x0B, 0x0B
};

static char g_on_cmd9[] = {
	0xF0, 0x55, 0xAA, 0x52, 0x08, 0x03
};

static char g_on_cmd10[] = {
	0xC7, 0x00
};

static char g_on_cmd11[] = {
	0xF0, 0x55, 0xAA, 0x52, 0x08, 0x08
};

static char g_on_cmd12[] = {
	0xBD, 0x0C
};

static char g_on_cmd13[] = {
	0x6F, 0x4A
};

static char g_on_cmd14[] = {
	0xBD, 0x80
};

static char g_on_cmd15[] = {
	0x6F, 0x01
};

static char g_on_cmd16[] = {
	0xBD, 0x00, 0x00, 0x28, 0x51, 0x85, 0x40
};

static char g_on_cmd17[] = {
	0x6F, 0x0E
};

static char g_on_cmd18[] = {
	0xBD, 0x00, 0x0A, 0xD4, 0x00, 0x0A, 0xD4
};

static char g_on_cmd19[] = {
	0x6F, 0x1A
};

static char g_on_cmd20[] = {
	0xBD, 0xCF, 0xCF, 0xE1, 0xDA, 0xDA, 0xE9, 0xE6, 0xE6, 0xF0, 0xF4, 0xF4, 0xF6
};

static char g_on_cmd21[] = {
	0x6F, 0x26
};

static char g_on_cmd22[] = {
	0xBD, 0xCF, 0xCF, 0xE1, 0xDA, 0xDA, 0xE9, 0xE6, 0xE6, 0xF0, 0xF4, 0xF4, 0xF6
};

static char g_on_cmd23[] = {
	0x6F, 0x4B
};

static char g_on_cmd24[] = {
	0xBD, 0xD5, 0xD5, 0xE5, 0xE0, 0xE0, 0xED, 0xEE, 0xEE, 0xF3, 0xFA, 0xFA, 0xFA
};

static char g_on_cmd25[] = {
	0x6F, 0x57
};

static char g_on_cmd26[] = {
	0xBD, 0xD5, 0xD5, 0xE5, 0xE0, 0xE0, 0xED, 0xEE, 0xEE, 0xF3, 0xFA, 0xFA, 0xFA
};

static char g_on_cmd27[] = {
	0xFF, 0xAA, 0x55, 0xA5, 0x80
};

static char g_on_cmd28[] = {
	0x6F, 0x1B
};

static char g_on_cmd29[] = {
	0xF4, 0x55
};

static char g_on_cmd30[] = {
	0xFF, 0xAA, 0x55, 0xA5, 0x81
};

static char g_on_cmd31[] = {
	0x6F, 0x12
};

static char g_on_cmd32[] = {
	0xF5, 0x00
};

static char g_on_cmd33[] = {
	0xFF, 0xAA, 0x55, 0xA5, 0x83
};

static char g_on_cmd34[] = {
	0x6F, 0x14
};

static char g_on_cmd35[] = {
	0xF8, 0x0D
};

static char g_on_cmd36[] = {
	0x6F, 0x01
};

static char g_on_cmd37[] = {
	0xF9, 0x06
};

static char g_on_cmd38[] = {
	0x6F, 0x01
};

static char g_on_cmd39[] = {
	0xFA, 0x06
};

static char g_on_cmd40[] = {
	0x6F, 0x01
};

static char g_on_cmd41[] = {
	0xFB, 0x06
};

static char g_on_cmd42[] = {
	0x6F, 0x01
};

static char g_on_cmd43[] = {
	0xFC, 0x06
};

static char g_on_cmd44[] = {
	0x03, 0x00
};

static char g_on_cmd45[] = {
	0x90, 0x03, 0x03
};

static char g_on_cmd46[] = {
	0x91, 0x89, 0xA8, 0x00, 0x2C, 0xF2, 0x00, 0x02, 0xA4, 0x04, 0x29, 0x00, 0x09, 0x02, 0xCB, 0x01, 0xDC, 0x10, 0xF0
};

static char g_on_cmd47[] = {
	0xFF, 0xAA, 0x55, 0xA5, 0x84
};

static char g_on_cmd48[] = {
	0x6F, 0x1C
};

static char g_on_cmd49[] = {
	0xF8, 0x3A
};

static char g_on_cmd50[] = {
	0x26, 0x00
};

static char g_on_cmd51[] = {
	0x35, 0x00
};

static char g_on_cmd52[] = {
	0x3B, 0x00, 0x10, 0x00, 0x08
};

static char g_on_cmd53[] = {
	0x53, 0x20
};

static char g_on_cmd54[] = {
	0x51, 0x00, 0x00, 0x00, 0x00
};

static char g_on_cmd55[] = {
	0x2A, 0x00, 0x00, 0x05, 0x3F
};

static char g_on_cmd56[] = {
	0x2B, 0x00, 0x00, 0x0A, 0xD3
};

static char g_on_cmd57[] = {
	0x9C, 0x08
};

static char g_on_cmd58[] = {
	0x2f, 0x02
};

static char g_on_cmd59[] = {
	0x11
};

static char g_on_cmd60[] = {
	0xF0, 0x55, 0xAA, 0x52, 0x08, 0x00
};

static char g_on_cmd61[] = {
	0x6F, 0x01
};

static char g_on_cmd62[] = {
	0xBE, 0x4C
};

static char g_on_cmd63[] = {
	0xF0, 0x55, 0xAA, 0x52, 0x08, 0x01
};

static char g_on_cmd64[] = {
	0xD1, 0x06, 0x01, 0x0A
};

static char g_on_cmd65[] = {
	0x29
};

static struct dsi_cmd_desc g_lcd_display_on_cmds[] = {
	{DTYPE_DCS_LWRITE, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0), g_on_cmd0},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd1), g_on_cmd1},
	{DTYPE_DCS_LWRITE, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd2), g_on_cmd2},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd3), g_on_cmd3},
	{DTYPE_DCS_LWRITE, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd4), g_on_cmd4},
	{DTYPE_DCS_LWRITE, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd5), g_on_cmd5},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd6), g_on_cmd6},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd7},
	{DTYPE_DCS_LWRITE, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd8), g_on_cmd8},
	{DTYPE_DCS_LWRITE, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd9), g_on_cmd9},
	{DTYPE_DCS_LWRITE, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd10), g_on_cmd10},
	{DTYPE_DCS_LWRITE, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd11), g_on_cmd11},
	{DTYPE_DCS_LWRITE, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd12), g_on_cmd12},
	{DTYPE_DCS_LWRITE, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd13), g_on_cmd13},
	{DTYPE_DCS_LWRITE, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd14), g_on_cmd14},
	{DTYPE_DCS_LWRITE, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd15), g_on_cmd15},
	{DTYPE_DCS_LWRITE, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd16), g_on_cmd16},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd17), g_on_cmd17},
	{DTYPE_DCS_LWRITE, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd18), g_on_cmd18},
	{DTYPE_DCS_LWRITE, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd19), g_on_cmd19},
	{DTYPE_DCS_LWRITE, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd20), g_on_cmd20},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd21), g_on_cmd21},
	{DTYPE_DCS_LWRITE, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd22), g_on_cmd22},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd23), g_on_cmd23},
	{DTYPE_DCS_LWRITE, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd24), g_on_cmd24},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd25), g_on_cmd25},
	{DTYPE_DCS_LWRITE, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd26), g_on_cmd26},
	{DTYPE_DCS_LWRITE, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd27), g_on_cmd27},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd28), g_on_cmd28},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd29), g_on_cmd29},
	{DTYPE_DCS_LWRITE, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd30), g_on_cmd30},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd31), g_on_cmd31},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd32), g_on_cmd32},
	{DTYPE_DCS_LWRITE, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd33), g_on_cmd33},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd34), g_on_cmd34},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd35), g_on_cmd35},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd36), g_on_cmd36},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd37), g_on_cmd37},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd38), g_on_cmd38},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd39), g_on_cmd39},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd40), g_on_cmd40},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd41), g_on_cmd41},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd42), g_on_cmd42},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd43), g_on_cmd43},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd44), g_on_cmd44},
	{DTYPE_DCS_LWRITE, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd45), g_on_cmd45},
	{DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US, sizeof(g_on_cmd46), g_on_cmd46},
	{DTYPE_DCS_LWRITE, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd47), g_on_cmd47},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd48), g_on_cmd48},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd49), g_on_cmd49},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd50), g_on_cmd50},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd51), g_on_cmd51},
	{DTYPE_DCS_LWRITE, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd52), g_on_cmd52},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd53), g_on_cmd53},
	{DTYPE_DCS_LWRITE, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd54), g_on_cmd54},
	{DTYPE_DCS_LWRITE, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd55), g_on_cmd55},
	{DTYPE_DCS_LWRITE, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd56), g_on_cmd56},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd57), g_on_cmd57},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd58), g_on_cmd58},
	{DTYPE_DCS_WRITE, 0, 120, WAIT_TYPE_US, sizeof(g_on_cmd59), g_on_cmd59},
	{DTYPE_DCS_LWRITE, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd60), g_on_cmd60},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd61), g_on_cmd61},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd62), g_on_cmd62},
	{DTYPE_DCS_LWRITE, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd63), g_on_cmd63},
	{DTYPE_DCS_LWRITE, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd64), g_on_cmd64},
	{DTYPE_DCS_WRITE, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd65), g_on_cmd65},
};

/* Power OFF Sequence */
static char g_off_cmd0[] = {
	0x28
};

static char g_off_cmd1[] = {
	0x10
};

static struct dsi_cmd_desc g_lcd_display_off_cmds[] = {
	{ DTYPE_DCS_WRITE, 0, 10, WAIT_TYPE_US, sizeof(g_off_cmd0), g_off_cmd0 },
	{ DTYPE_DCS_WRITE, 0, 100, WAIT_TYPE_MS, sizeof(g_off_cmd1), g_off_cmd1 },
};

/*************************************************************************
 * LCD GPIO for fpga
 */
#define GPIO_AMOLED_RESET_NAME  "gpio_amoled_reset"
#define GPIO_AMOLED_VCC1V8_NAME "gpio_amoled_vcc1v8"
#define GPIO_AMOLED_VCC3V1_NAME "gpio_amoled_vcc3v1"
#define GPIO_AMOLED_TE0_NAME    "gpio_amoled_te0"

static uint32_t g_gpio_amoled_reset;
static uint32_t g_gpio_amoled_vcc1v8;
static uint32_t g_gpio_amoled_vcc3v1;
static uint32_t g_gpio_amoled_te0;

static struct gpio_desc g_fpga_lcd_gpio_request_cmds[] = {
	/* vcc3v1 */
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0, GPIO_AMOLED_VCC3V1_NAME, &g_gpio_amoled_vcc3v1, 0},
	/* vcc1v8 */
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0, GPIO_AMOLED_VCC1V8_NAME, &g_gpio_amoled_vcc1v8, 0},
	/* reset */
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0, GPIO_AMOLED_RESET_NAME, &g_gpio_amoled_reset, 0},
};

/* spec define: vcc1v8=1 --> vcc3v1=1 --> reset=1 to 0 to 1
 * spec not define delay time, so use nt37700p value
 */
static struct gpio_desc g_fpga_lcd_gpio_normal_cmds[] = {
	/* vcc1v8 enable */
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 2, GPIO_AMOLED_VCC1V8_NAME, &g_gpio_amoled_vcc1v8, 1},
	/* vcc3v1 enable */
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 15, GPIO_AMOLED_VCC3V1_NAME, &g_gpio_amoled_vcc3v1, 1},
	/* reset */
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 5, GPIO_AMOLED_RESET_NAME, &g_gpio_amoled_reset, 1},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_US, 50, GPIO_AMOLED_RESET_NAME, &g_gpio_amoled_reset, 0},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 15, GPIO_AMOLED_RESET_NAME, &g_gpio_amoled_reset, 1},
	/* backlight enable */
};

/* follow nt37700p */
static struct gpio_desc g_fpga_lcd_gpio_free_cmds[] = {
	/* backlight enable */
	/* reset */
	{DTYPE_GPIO_FREE, WAIT_TYPE_MS, 0, GPIO_AMOLED_RESET_NAME, &g_gpio_amoled_reset, 0},
	/* vcc3v1 */
	{DTYPE_GPIO_FREE, WAIT_TYPE_MS, 0, GPIO_AMOLED_VCC3V1_NAME, &g_gpio_amoled_vcc3v1, 0},
	/* vcc1v8 */
	{DTYPE_GPIO_FREE, WAIT_TYPE_MS, 0, GPIO_AMOLED_VCC1V8_NAME, &g_gpio_amoled_vcc1v8, 0},
};

/* follow nt37700p */
static struct gpio_desc g_fpga_lcd_gpio_lowpower_cmds[] = {
	/* backlight enable */
	/* reset */
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 5, GPIO_AMOLED_RESET_NAME, &g_gpio_amoled_reset, 0},
	/* vcc3v1 disable */
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 10, GPIO_AMOLED_VCC3V1_NAME, &g_gpio_amoled_vcc3v1, 0},
	/* vcc1v8 disable */
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 10, GPIO_AMOLED_VCC1V8_NAME, &g_gpio_amoled_vcc1v8, 0},

	/* backlight enable input */
	/* reset input */
	{DTYPE_GPIO_INPUT, WAIT_TYPE_US, 100, GPIO_AMOLED_RESET_NAME, &g_gpio_amoled_reset, 0},
	/* vcc3v1 disable */
	{DTYPE_GPIO_INPUT, WAIT_TYPE_US, 100, GPIO_AMOLED_VCC3V1_NAME, &g_gpio_amoled_vcc3v1, 0},
	/* vcc1v8 disable */
	{DTYPE_GPIO_INPUT, WAIT_TYPE_US, 100, GPIO_AMOLED_VCC1V8_NAME, &g_gpio_amoled_vcc1v8, 0},
};

/*************************************************************************
 * LCD GPIO for udp, current follow nt37700p
 */
static struct gpio_desc g_asic_lcd_gpio_request_cmds[] = {
	{ DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0, GPIO_AMOLED_VCC3V1_NAME, &g_gpio_amoled_vcc3v1, 0 },
	{ DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0, GPIO_AMOLED_RESET_NAME, &g_gpio_amoled_reset, 0 },
};

static struct gpio_desc g_asic_lcd_gpio_normal_cmds[] = {
	{ DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 20,
		GPIO_AMOLED_VCC3V1_NAME, &g_gpio_amoled_vcc3v1, 1 },
	{ DTYPE_GPIO_OUTPUT, WAIT_TYPE_US, 50,
		GPIO_AMOLED_RESET_NAME, &g_gpio_amoled_reset, 1 },
	{ DTYPE_GPIO_OUTPUT, WAIT_TYPE_US, 50,
		GPIO_AMOLED_RESET_NAME, &g_gpio_amoled_reset, 0 },
	{ DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 15,
		GPIO_AMOLED_RESET_NAME, &g_gpio_amoled_reset, 1 },
};

static struct gpio_desc g_asic_lcd_gpio_lowpower_cmds[] = {
	{ DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 20,
		GPIO_AMOLED_RESET_NAME, &g_gpio_amoled_reset, 0 },
	{ DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 20,
		GPIO_AMOLED_RESET_NAME, &g_gpio_amoled_vcc3v1, 0 },
};

static struct gpio_desc g_asic_lcd_gpio_free_cmds[] = {
	{ DTYPE_GPIO_FREE, WAIT_TYPE_US, 50,
		GPIO_AMOLED_VCC3V1_NAME, &g_gpio_amoled_vcc3v1, 0 },
	{ DTYPE_GPIO_FREE, WAIT_TYPE_US, 50,
		GPIO_AMOLED_RESET_NAME, &g_gpio_amoled_reset, 0 },
};

/*******************************************************************************
 * LCD VCC for udp, current follow nt37700p
 */
#define VCC_LCDIO_NAME	"lcdio-vcc"
#define VCC_LCDDIG_NAME	"lcddig-vcc"

static struct regulator *g_vcc_lcdio;
static struct regulator *g_vcc_lcddig;

static struct vcc_desc g_lcd_vcc_init_cmds[] = {
	/* vcc get */
	{ DTYPE_VCC_GET, VCC_LCDIO_NAME,
		&g_vcc_lcdio, 0, 0, WAIT_TYPE_MS, 0 },
	{ DTYPE_VCC_GET, VCC_LCDDIG_NAME,
		&g_vcc_lcddig, 0, 0, WAIT_TYPE_MS, 0 },

	/* vcc set voltage */
	{ DTYPE_VCC_SET_VOLTAGE, VCC_LCDDIG_NAME,
		&g_vcc_lcddig, 1250000, 1250000, WAIT_TYPE_MS, 0 },
	/* io set voltage */
	{ DTYPE_VCC_SET_VOLTAGE, VCC_LCDIO_NAME,
		&g_vcc_lcdio, 1850000, 1850000, WAIT_TYPE_MS, 0 },
};

static struct vcc_desc g_lcd_vcc_finit_cmds[] = {
	/* vcc put */
	{ DTYPE_VCC_PUT, VCC_LCDIO_NAME,
		&g_vcc_lcdio, 0, 0, WAIT_TYPE_MS, 0 },
	{ DTYPE_VCC_PUT, VCC_LCDDIG_NAME,
		&g_vcc_lcddig, 0, 0, WAIT_TYPE_MS, 0 },
};

static struct vcc_desc g_lcd_vcc_enable_cmds[] = {
	/* vcc enable */
	{ DTYPE_VCC_ENABLE, VCC_LCDIO_NAME,
		&g_vcc_lcdio, 0, 0, WAIT_TYPE_MS, 3 },
	{ DTYPE_VCC_ENABLE, VCC_LCDDIG_NAME,
		&g_vcc_lcddig, 0, 0, WAIT_TYPE_MS, 3 },
};

static struct vcc_desc g_lcd_vcc_disable_cmds[] = {
	/* vcc enable */
	{ DTYPE_VCC_DISABLE, VCC_LCDDIG_NAME,
		&g_vcc_lcddig, 0, 0, WAIT_TYPE_MS, 3 },
	{ DTYPE_VCC_DISABLE, VCC_LCDIO_NAME,
		&g_vcc_lcdio, 0, 0, WAIT_TYPE_MS, 3 },
};

/*******************************************************************************
 ** LCD IOMUX for udp, current follow nt37700p
 */
static struct pinctrl_data g_pctrl;

static struct pinctrl_cmd_desc g_lcd_pinctrl_init_cmds[] = {
	{ DTYPE_PINCTRL_GET, &g_pctrl, 0 },
	{ DTYPE_PINCTRL_STATE_GET, &g_pctrl, DTYPE_PINCTRL_STATE_DEFAULT },
	{ DTYPE_PINCTRL_STATE_GET, &g_pctrl, DTYPE_PINCTRL_STATE_IDLE },
};

static struct pinctrl_cmd_desc g_lcd_pinctrl_normal_cmds[] = {
	{ DTYPE_PINCTRL_SET, &g_pctrl, DTYPE_PINCTRL_STATE_DEFAULT },
};

static struct pinctrl_cmd_desc g_lcd_pinctrl_lowpower_cmds[] = {
	{DTYPE_PINCTRL_SET, &g_pctrl, DTYPE_PINCTRL_STATE_IDLE},
};

static struct pinctrl_cmd_desc g_lcd_pinctrl_finit_cmds[] = {
	{ DTYPE_PINCTRL_PUT, &g_pctrl, 0 },
};

static void dsc_param_set(struct dkmd_connector_info *pinfo, struct dsc_calc_info *dsc)
{
	struct dsc_algorithm_manager *pt = get_dsc_algorithm_manager_instance();
	struct input_dsc_info input_dsc_info;

	if (!pt) {
		dpu_pr_err("pt is null!\n");
		return;
	}

	dpu_pr_info("+\n");

	pinfo->ifbc_type = IFBC_TYPE_VESA3X_DUAL;
	input_dsc_info.dsc_version = DSC_VERSION_V_1_2;
	input_dsc_info.format = DSC_RGB;
	input_dsc_info.pic_width = pinfo->base.xres;
	input_dsc_info.pic_height = pinfo->base.yres;
	input_dsc_info.slice_width = 671 + 1;
	input_dsc_info.slice_height = 43 + 1;
	input_dsc_info.dsc_bpp = DSC_8BPP;
	input_dsc_info.dsc_bpc = DSC_8BPC;
	input_dsc_info.block_pred_enable = 1;
	input_dsc_info.linebuf_depth = LINEBUF_DEPTH_9;
	input_dsc_info.gen_rc_params = DSC_GENERATE_RC_PARAMETERS;
	pt->vesa_dsc_info_calc(&input_dsc_info, &(dsc->dsc_info), NULL);
	dsc->dsc_en = 1;
	dpu_pr_info("-\n");
}

/* dsi param initialized value from panel spec */
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
		dpu_pr_info("[37290 probe] udp mipi param set\n");
		mipi->hsa = 13;
		mipi->hbp = 5;
		mipi->dpi_hsize = 338;
		mipi->hline_time = 640;
		mipi->vsa = 24;
		mipi->vbp = 58;
		mipi->vfp = 40;

		mipi->dsi_bit_clk = 456;
		mipi->pxl_clk_rate = 192 * 1000000UL;
	}

	mipi->dsi_bit_clk_upt_support = 0;
	mipi->dsi_bit_clk_upt = mipi->dsi_bit_clk;
	mipi->dsi_bit_clk_default = mipi->dsi_bit_clk;
	mipi->pxl_clk_rate_div = 1;

	mipi->clk_post_adjust = 215;
	mipi->lane_nums = DSI_4_LANES;
	mipi->color_mode = DSI_24BITS_1;

	mipi->vc = 0;
	mipi->max_tx_esc_clk = 10 * 1000000;
	mipi->burst_mode = DSI_BURST_SYNC_PULSES_1;
	mipi->non_continue_en = 1;
	mipi->phy_mode = DPHY_MODE;
	mipi->dsi_version = DSI_1_1_VERSION;
}

static void panel_drv_private_data_setup(struct panel_drv_private *priv, struct device_node *np)
{
	if (priv->connector_info.base.fpga_flag == 1) {
		g_gpio_amoled_reset = (uint32_t)of_get_named_gpio(np, "gpios", 0);
		g_gpio_amoled_vcc3v1 = (uint32_t)of_get_named_gpio(np, "gpios", 1);
		g_gpio_amoled_vcc1v8 = (uint32_t)of_get_named_gpio(np, "gpios", 2);

		dpu_pr_info("used gpio:[rst: %u, vcc3v1: %u, tp1v8: %u]\n",
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

		dpu_pr_info("used gpio:[rst: %d, vcc3v1: %d, te0: %d]\n",
			g_gpio_amoled_reset, g_gpio_amoled_vcc3v1, g_gpio_amoled_te0);

		priv->gpio_request_cmds = g_asic_lcd_gpio_request_cmds;
		priv->gpio_request_cmds_len = ARRAY_SIZE(g_asic_lcd_gpio_request_cmds);
		priv->gpio_free_cmds = g_asic_lcd_gpio_free_cmds;
		priv->gpio_free_cmds_len = ARRAY_SIZE(g_asic_lcd_gpio_free_cmds);

		priv->gpio_normal_cmds = g_asic_lcd_gpio_normal_cmds;
		priv->gpio_normal_cmds_len = ARRAY_SIZE(g_asic_lcd_gpio_normal_cmds);
		priv->gpio_lowpower_cmds = g_asic_lcd_gpio_lowpower_cmds;
		priv->gpio_lowpower_cmds_len = (uint32_t)ARRAY_SIZE(g_asic_lcd_gpio_lowpower_cmds);

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

/* dirty region initialized value from panel spec */
static void lcd_init_dirty_region(struct panel_drv_private *priv)
{
	priv->user_pinfo.left_align = -1;
	priv->user_pinfo.right_align = -1;
	priv->user_pinfo.top_align = 44;
	priv->user_pinfo.bottom_align = 44;
	priv->user_pinfo.w_align = -1;
	priv->user_pinfo.h_align = 44;
	priv->user_pinfo.w_min = 1344;
	priv->user_pinfo.h_min = 44;
	priv->user_pinfo.top_start = -1;
	priv->user_pinfo.bottom_start = -1;
	priv->user_pinfo.dirty_region_updt_support = priv->connector_info.dirty_region_updt_support;
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

	/* 1. config base object info
	 * would be used for framebuffer setup
	 */
	pinfo->base.xres = 1344; // FIXME: Modified for new panel device
	pinfo->base.yres = 2772; // FIXME: Modified for new panel device

	/* When calculating DPI needs the following parameters */
	pinfo->base.width = 74; // FIXME: Modified for new panel device
	pinfo->base.height = 154; // FIXME: Modified for new panel device

	/* report to surfaceFlinger, remain: caculate fps by mipi timing para */
	pinfo->base.fps = 60;

	/* 2. config connector info
	 * would be used for dsi & composer setup
	 */
	mipi_lcd_init_dsi_param(pinfo, &connector->mipi);
	dsc_param_set(pinfo, &connector->dsc);

	/* dsi or composer need this param */
	pinfo->dirty_region_updt_support = 1;
	lcd_init_dirty_region(priv);
	pinfo->vsync_ctrl_type = VSYNC_IDLE_MIPI_ULPS | VSYNC_IDLE_CLK_OFF | VSYNC_IDLE_ISR_OFF;

	/* 3. config panel private info
	 * would be used for panel setup
	 */
	pinfo->bl_info.bl_min = 12;
	pinfo->bl_info.bl_max = 2047;
	pinfo->bl_info.bl_default = 2047;
	priv->mipi_brightness_para_type = MIPI_BL_PARA1_DBV8_AND_PARA2_DBV0;

	priv->disp_on_cmds = g_lcd_display_on_cmds;
	priv->disp_on_cmds_len = ARRAY_SIZE(g_lcd_display_on_cmds);
	priv->disp_off_cmds = g_lcd_display_off_cmds;
	priv->disp_off_cmds_len = ARRAY_SIZE(g_lcd_display_off_cmds);

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

panel_device_match_data(nt37290_panel_info, PANEL_NT37290_ID, panel_of_device_setup, panel_of_device_release);

MODULE_LICENSE("GPL");

