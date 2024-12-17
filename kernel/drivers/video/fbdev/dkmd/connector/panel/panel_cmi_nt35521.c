/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include "dpu_conn_mgr.h"
#include "panel_mgr.h"
#include "panel_drv.h"

/*******************************************************************************
** Display ON/OFF Sequence begin
*/

/* ************************* cmi power on initial code ********************* */
static char cmi_power_on_param1[] =
{
	0xff,
	0xAA, 0x55, 0xA5, 0x80,
};

static char cmi_power_on_param2[] =
{
	0x6F,
	0x11, 0x00,
};

static char cmi_power_on_param3[] =
{
	0xF7,
	0x20, 0x00,
};

static char cmi_power_on_param4[] =
{
	0x6F,
	0x06,
};

static char cmi_power_on_param5[] =
{
	0xF7,
	0xA0,
};

static char cmi_power_on_param6[] =
{
	0x6F,
	0x19,
};

static char cmi_power_on_param7[] =
{
	0xF7,
	0x12,
};

static char cmi_power_on_param_f4[] =
{
	0xF4,
	0x03,
};

static char cmi_power_on_param8[] =
{
	0xF0,
	0x55, 0xAA, 0x52, 0x08, 0x00,
};

static char cmi_power_on_param9[] =
{
	0xC8,
	/* resolve esd result in lcd blank screen */
	0x80,
};

static char cmi_power_on_param10[] =
{
	0xB1,
	0x68, 0x21,
};

static char cmi_power_on_param11[] =
{
	0xB6,
	0x08,
};

static char cmi_power_on_param12[] =
{
	0x6F,
	0x02,
};

static char cmi_power_on_param13[] =
{
	0xB8,
	0x08,
};

static char cmi_power_on_param14[] =
{
	0xBB,
	0x74, 0x44,
};

static char cmi_power_on_param15[] =
{
	0xBC,
	0x00, 0x00,
};

static char cmi_power_on_param16[] =
{
	0xBD,
	0x02, 0xB0, 0x1E, 0x1E, 0x00,
};

static char cmi_power_on_param_c5[] =
{
	0xC5,
	0x01, 0x07,
};

static char cmi_power_on_param17[] =
{
	0xF0,
	0x55, 0xAA, 0x52, 0x08, 0x01,
};

static char cmi_power_on_param18[] =
{
	0xB0,
	0x05, 0x05,
};

static char cmi_power_on_param19[] =
{
	0xB1,
	0x05, 0x05,
};

static char cmi_power_on_param20[] =
{
	0xBC,
	0xA0, 0x01,
};

static char cmi_power_on_param21[] =
{
	0xBD,
	0xA0, 0x01,
};

static char cmi_power_on_param22[] =
{
	0xCA,
	0x00,
};

static char cmi_power_on_param23[] =
{
	0xC0,
	0x04,
};

static char cmi_power_on_param24[] =
{
	0xB3,
	0x37, 0x37,
};

static char cmi_power_on_param25[] =
{
	0xB4,
	0x0F, 0x0F,
};

static char cmi_power_on_param_b6[] =
{
	0xB6,
	0x05, 0x05,
};

static char cmi_power_on_param26[] =
{
	0xB9,
	/* resolve esd result in lcd blank screen */
	0x36, 0x36,
};

static char cmi_power_on_param27[] =
{
	0xBA,
	0x25, 0x25,
};

static char cmi_power_on_param28[] =
{
	0xF0,
	0x55, 0xAA, 0x52, 0x08, 0x02,
};

static char cmi_power_on_param29[] =
{
	0xEE,
	0x01,
};

static char cmi_power_on_param30[] =
{
	0xB0,
	0x00,0x00,0x00,0x42,0x00,0x88,0x00,0xA6,0x00,0xBF,0x00,0xE6,0x01,0x05,0x01,0x32,
};

static char cmi_power_on_param31[] =
{
	0xB1,
	0x01,0x58,0x01,0x94,0x01,0xC1,0x02,0x08,0x02,0x40,0x02,0x42,0x02,0x78,0x02,0xB4,
};

static char cmi_power_on_param32[] =
{
	0xB2,
	0x02,0xD9,0x03,0x0C,0x03,0x30,0x03,0x5F,0x03,0x7F,0x03,0xA5,0x03,0xC0,0x03,0xE0,
};

static char cmi_power_on_param33[] =
{
	0xB3,
	0x03,0xF6,0x03,0xFF,
};

static char cmi_power_on_param46[] =
{
	0xF0,
	0x55, 0xAA, 0x52, 0x08, 0x06,
};

static char cmi_power_on_param47[] =
{
	0xB0,
	0x29, 0x2A,
};

static char cmi_power_on_param48[] =
{
	0xB1,
	0x10, 0x12,
};

static char cmi_power_on_param49[] =
{
	0xB2,
	0x14, 0x16,
};

static char cmi_power_on_param50[] =
{
	0xB3,
	0x18, 0x1A,
};

static char cmi_power_on_param51[] =
{
	0xB4,
	0x08, 0x0A,
};

static char cmi_power_on_param52[] =
{
	0xB5,
	0x2E, 0x2E,
};

static char cmi_power_on_param53[] =
{
	0xB6,
	0x2E, 0x2E,
};

static char cmi_power_on_param54[] =
{
	0xB7,
	0x2E, 0x2E,
};

static char cmi_power_on_param55[] =
{
	0xB8,
	0x2E, 0x00,
};

static char cmi_power_on_param56[] =
{
	0xB9,
	0x2E, 0x2E,
};

static char cmi_power_on_param57[] =
{
	0xBA,
	0x2E, 0x2E,
};

static char cmi_power_on_param58[] =
{
	0xBB,
	0x01, 0x2E,
};

static char cmi_power_on_param59[] =
{
	0xBC,
	0x2E, 0x2E,
};

static char cmi_power_on_param60[] =
{
	0xBD,
	0x2E, 0x2E,
};

static char cmi_power_on_param61[] =
{
	0xBE,
	0x2E, 0x2E,
};

static char cmi_power_on_param62[] =
{
	0xBF,
	0x0B, 0x09,
};

static char cmi_power_on_param63[] =
{
	0xC0,
	0x1B, 0x19,
};

static char cmi_power_on_param64[] =
{
	0xC1,
	0x17, 0x15,
};

static char cmi_power_on_param65[] =
{
	0xC2,
	0x13, 0x11,
};

static char cmi_power_on_param66[] =
{
	0xC3,
	0x2A, 0x29,
};

static char cmi_power_on_param67[] =
{
	0xE5,
	0x2E, 0x2E,
};

static char cmi_power_on_param68[] =
{
	0xC4,
	0x29, 0x2A,
};

static char cmi_power_on_param69[] =
{
	0xC5,
	0x1B, 0x19,
};

static char cmi_power_on_param70[] =
{
	0xC6,
	0x17, 0x15,
};

static char cmi_power_on_param71[] =
{
	0xC7,
	0x13, 0x11,
};

static char cmi_power_on_param72[] =
{
	0xC8,
	0x01, 0x0B,
};

static char cmi_power_on_param73[] =
{
	0xC9,
	0x2E, 0x2E,
};

static char cmi_power_on_param74[] =
{
	0xCA,
	0x2E, 0x2E,
};

static char cmi_power_on_param75[] =
{
	0xCB,
	0x2E, 0x2E,
};

static char cmi_power_on_param76[] =
{
	0xCC,
	0x2E, 0x09,
};

static char cmi_power_on_param77[] =
{
	0xCD,
	0x2E, 0x2E,
};

static char cmi_power_on_param78[] =
{
	0xCE,
	0x2E, 0x2E,
};

static char cmi_power_on_param79[] =
{
	0xCF,
	0x08, 0x2E,
};

static char cmi_power_on_param80[] =
{
	0xD0,
	0x2E, 0x2E,
};

static char cmi_power_on_param81[] =
{
	0xD1,
	0x2E, 0x2E,
};

static char cmi_power_on_param82[] =
{
	0xD2,
	0x2E, 0x2E,
};

static char cmi_power_on_param83[] =
{
	0xD3,
	0x0A, 0x00,
};

static char cmi_power_on_param84[] =
{
	0xD4,
	0x10, 0x12,
};

static char cmi_power_on_param85[] =
{
	0xD5,
	0x14, 0x16,
};

static char cmi_power_on_param86[] =
{
	0xD6,
	0x18, 0x1A,
};

static char cmi_power_on_param87[] =
{
	0xD7,
	0x2A, 0x29,
};

static char cmi_power_on_param88[] =
{
	0xE6,
	0x2E, 0x2E,
};

static char cmi_power_on_param89[] =
{
	0xD8,
	0x00, 0x00, 0x00, 0x00, 0x00,
};

static char cmi_power_on_param90[] =
{
	0xD9,
	0x00, 0x00, 0x00, 0x00, 0x00,
};

static char cmi_power_on_param91[] =
{
	0xE7,
	0x00,
};

static char cmi_power_on_param92[] =
{
	0xF0,
	0x55, 0xAA, 0x52, 0x08, 0x03,
};

static char cmi_power_on_param93[] =
{
	0xB0,
	0x00, 0x00,
};

static char cmi_power_on_param94[] =
{
	0xB1,
	0x00, 0x00,
};

static char cmi_power_on_param95[] =
{
	0xB2,
	0x05, 0x00, 0x00, 0x00, 0x00,
};

static char cmi_power_on_param96[] =
{
	0xB6,
	0x05, 0x00, 0x00, 0x00, 0x00,
};

static char cmi_power_on_param97[] =
{
	0xB7,
	0x05, 0x00, 0x00, 0x00, 0x00,
};

static char cmi_power_on_param98[] =
{
	0xBA,
	0x57, 0x00, 0x00, 0x00, 0x00,
};

static char cmi_power_on_param99[] =
{
	0xBB,
	0x57, 0x00, 0x00, 0x00, 0x00,
};

static char cmi_power_on_param100[] =
{
	0xC0,
	0x00, 0x00, 0x00, 0x00,
};

static char cmi_power_on_param101[] =
{
	0xC1,
	0x00, 0x00, 0x00, 0x00,
};

static char cmi_power_on_param102[] =
{
	0xC4,
	0x60,
};

static char cmi_power_on_param103[] =
{
	0xC5,
	0x40,
};

static char cmi_power_on_param104[] =
{
	0xF0,
	0x55, 0xAA, 0x52, 0x08, 0x05,
};

static char cmi_power_on_param105[] =
{
	0xBD,
	0x03, 0x01, 0x03, 0x03, 0x03,
};

static char cmi_power_on_param106[] =
{
	0xB0,
	0x17, 0x06,
};

static char cmi_power_on_param107[] =
{
	0xB1,
	0x17, 0x06,
};

static char cmi_power_on_param108[] =
{
	0xB2,
	0x17, 0x06,
};

static char cmi_power_on_param109[] =
{
	0xB3,
	0x17, 0x06,
};

static char cmi_power_on_param110[] =
{
	0xB4,
	0x17, 0x06,
};

static char cmi_power_on_param111[] =
{
	0xB5,
	0x17, 0x06,
};

static char cmi_power_on_param112[] =
{
	0xB8,
	0x00,
};

static char cmi_power_on_param113[] =
{
	0xB9,
	0x00,
};

static char cmi_power_on_param114[] =
{
	0xBA,
	0x00,
};

static char cmi_power_on_param115[] =
{
	0xBB,
	0x02,
};

static char cmi_power_on_param116[] =
{
	0xBC,
	0x00,
};

static char cmi_power_on_param117[] =
{
	0xC0,
	0x07,
};

static char cmi_power_on_param118[] =
{
	0xC4,
	0x80,
};

static char cmi_power_on_param119[] =
{
	0xC5,
	0xA4,
};

static char cmi_power_on_param120[] =
{
	0xC8,
	0x05, 0x30,
};

static char cmi_power_on_param121[] =
{
	0xC9,
	0x01, 0x31,
};

static char cmi_power_on_param122[] =
{
	0xCC,
	0x00, 0x00, 0x3C,
};

static char cmi_power_on_param123[] =
{
	0xCD,
	0x00, 0x00, 0x3C,
};

static char cmi_power_on_param124[] =
{
	0xD1,
	0x00, 0x05, 0x09, 0x07, 0x10,
};

static char cmi_power_on_param125[] =
{
	0xD2,
	0x00, 0x05, 0x0E, 0x07, 0x10,
};

static char cmi_power_on_param126[] =
{
	0xE5,
	0x06,
};

static char cmi_power_on_param127[] =
{
	0xE6,
	0x06,
};

static char cmi_power_on_param128[] =
{
	0xE7,
	0x06,
};

static char cmi_power_on_param129[] =
{
	0xE8,
	0x06,
};

static char cmi_power_on_param130[] =
{
	0xE9,
	0x06,
};

static char cmi_power_on_param131[] =
{
	0xEA,
	0x06,
};

static char cmi_power_on_param132[] =
{
	0xED,
	0x30,
};

static char cmi_power_on_param133[] =
{
	0x6F,
	0x11,
};

static char cmi_power_on_param134[] =
{
	0xF3,
	0x01,
};

static char cmi_power_on_param135[] =
{
	0xF0,
	0x55, 0xAA, 0x52, 0x08, 0x00,
};

static char cmi_power_on_param136[] =
{
	0xD9,
	0x01, 0x01,
};

static char cmi_power_on_param_d1[] = {
	0xD1,
	0x00, 0x02, 0x06, 0x0A, 0x0E, 0x12, 0x15, 0x18, 0x18, 0x18, 0x16, 0x14, 0x12, 0x0A, 0x04, 0x00,
};

// Smart Color parameters
static char cmi_power_on_param137[] = {
	0xD7,
	0x24, 0x23, 0x22, 0x21, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static char cmi_power_on_param138[] = {
	0xD8,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x23, 0x22, 0x00,
};

// Disable edge enhancement and close CE dimming function
static char cmi_power_on_param139[] =
{
	0xCC,
	0x40, 0x36, 0xBA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x50, 0xFF, 0x00,
};

// set cabc PWM level in UI mode
static char cmi_power_on_param140[] = {
	0xE6,
	0xFF, 0xFF, 0xFA, 0xFA,
};

// write cabc minimum brightness
static char cmi_power_on_param141[] = {
	0x5E,
	0x28,
};

/* *************** Power ON Sequence(sleep mode to Normal mode) ************ */
static char bl_level_0[] =
{
	0x51,
	0x00,
};

static char bl_value[] =
{
	0x51,
	0xFF,
};

static char bl_enable[] =
{
	0x53,
	0x24,
};

// CABC
#if  CMI_CABC_ENABLED
// Init cabc to UI mode when panel on
static char cabc_mode[] =
{
	0x55,
	0x81,
};
#endif

static char exit_sleep[] =
{
	0x11,
};

static char display_on[] =
{
	0x29,
};

/*
 * Power OFF Sequence(Normal to power off)
 */
static char display_off[] =
{
	0x28,
};

static char enter_sleep[] =
{
	0x10,
};

static struct dsi_cmd_desc lcd_display_on_cmds[] =
{
	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param1), cmi_power_on_param1
	},
	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param2), cmi_power_on_param2
	},
	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param3), cmi_power_on_param3
	},
	{
		DTYPE_DCS_WRITE1, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param4), cmi_power_on_param4
	},
	{
		DTYPE_DCS_WRITE1, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param5), cmi_power_on_param5
	},
	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param6), cmi_power_on_param6
	},
	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param7), cmi_power_on_param7
	},

	{
		DTYPE_DCS_WRITE1, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param_f4), cmi_power_on_param_f4
	},

	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param8), cmi_power_on_param8
	},
	{
		DTYPE_DCS_WRITE1, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param9), cmi_power_on_param9
	},

	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param10), cmi_power_on_param10
	},
	{
		DTYPE_DCS_WRITE1, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param11), cmi_power_on_param11
	},
	{
		DTYPE_DCS_WRITE1, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param12), cmi_power_on_param12
	},
	{
		DTYPE_DCS_WRITE1, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param13), cmi_power_on_param13
	},
	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param14), cmi_power_on_param14
	},
	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param15), cmi_power_on_param15
	},
	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param16), cmi_power_on_param16
	},

	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param_c5), cmi_power_on_param_c5
	},

	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param17), cmi_power_on_param17
	},

	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param18), cmi_power_on_param18
	},
	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param19), cmi_power_on_param19
	},
	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param20), cmi_power_on_param20
	},
	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param21), cmi_power_on_param21
	},
	{
		DTYPE_DCS_WRITE1, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param22), cmi_power_on_param22
	},
	{
		DTYPE_DCS_WRITE1, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param23), cmi_power_on_param23
	},

	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param24), cmi_power_on_param24
	},
	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param25), cmi_power_on_param25
	},

	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param_b6), cmi_power_on_param_b6
	},

	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param26), cmi_power_on_param26
	},
	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param27), cmi_power_on_param27
	},
	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param28), cmi_power_on_param28
	},

	{
		DTYPE_DCS_WRITE1, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param29), cmi_power_on_param29
	},
	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param30), cmi_power_on_param30
	},
	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param31), cmi_power_on_param31
	},

	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param32), cmi_power_on_param32
	},
	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param33), cmi_power_on_param33
	},

	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param46), cmi_power_on_param46
	},
	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param47), cmi_power_on_param47
	},
	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param48), cmi_power_on_param48
	},
	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param49), cmi_power_on_param49
	},
	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param50), cmi_power_on_param50
	},
	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param51), cmi_power_on_param51
	},
	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param52), cmi_power_on_param52
	},
	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param53), cmi_power_on_param53
	},
	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param54), cmi_power_on_param54
	},
	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param55), cmi_power_on_param55
	},
	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param56), cmi_power_on_param56
	},
	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param57), cmi_power_on_param57
	},
	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param58), cmi_power_on_param58
	},
	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param59), cmi_power_on_param59
	},
	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param60), cmi_power_on_param60
	},
	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param61), cmi_power_on_param61
	},
	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param62), cmi_power_on_param62
	},
	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param63), cmi_power_on_param63
	},
	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param64), cmi_power_on_param64
	},
	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param65), cmi_power_on_param65
	},
	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param66), cmi_power_on_param66
	},
	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param67), cmi_power_on_param67
	},
	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param68), cmi_power_on_param68
	},
	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param69), cmi_power_on_param69
	},
	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param70), cmi_power_on_param70
	},
	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param71), cmi_power_on_param71
	},
	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param72), cmi_power_on_param72
	},
	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param73), cmi_power_on_param73
	},
	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param74), cmi_power_on_param74
	},
	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param75), cmi_power_on_param75
	},
	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param76), cmi_power_on_param76
	},
	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param77), cmi_power_on_param77
	},
	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param78), cmi_power_on_param78
	},
	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param79), cmi_power_on_param79
	},
	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param80), cmi_power_on_param80
	},
	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param81), cmi_power_on_param81
	},
	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param82), cmi_power_on_param82
	},
	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param83), cmi_power_on_param83
	},
	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param84), cmi_power_on_param84
	},
	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param85), cmi_power_on_param85
	},
	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param86), cmi_power_on_param86
	},
	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param87), cmi_power_on_param87
	},
	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param88), cmi_power_on_param88
	},
	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param89), cmi_power_on_param89
	},
	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param90), cmi_power_on_param90
	},
	{
		DTYPE_DCS_WRITE1, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param91), cmi_power_on_param91
	},
	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param92), cmi_power_on_param92
	},
	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param93), cmi_power_on_param93
	},
	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param94), cmi_power_on_param94
	},
	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param95), cmi_power_on_param95
	},
	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param96), cmi_power_on_param96
	},
	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param97), cmi_power_on_param97
	},
	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param98), cmi_power_on_param98
	},
	{
		DTYPE_DCS_LWRITE, 0, 0, WAIT_TYPE_US,
		sizeof(cmi_power_on_param99), cmi_power_on_param99
	},
	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param100), cmi_power_on_param100
	},
	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param101), cmi_power_on_param101
	},
	{
		DTYPE_DCS_WRITE1, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param102), cmi_power_on_param102
	},

	{
		DTYPE_DCS_WRITE1, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param103), cmi_power_on_param103
	},
	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param104), cmi_power_on_param104
	},
	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param105), cmi_power_on_param105
	},
	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param106), cmi_power_on_param106
	},
	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param107), cmi_power_on_param107
	},
	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param108), cmi_power_on_param108
	},
	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param109), cmi_power_on_param109
	},
	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param110), cmi_power_on_param110
	},
	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param111), cmi_power_on_param111
	},
	{
		DTYPE_DCS_WRITE1, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param112), cmi_power_on_param112
	},
	{
		DTYPE_DCS_WRITE1, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param113), cmi_power_on_param113
	},
	{
		DTYPE_DCS_WRITE1, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param114), cmi_power_on_param114
	},
	{
		DTYPE_DCS_WRITE1, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param115), cmi_power_on_param115
	},
	{
		DTYPE_DCS_WRITE1, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param116), cmi_power_on_param116
	},
	{
		DTYPE_DCS_WRITE1, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param117), cmi_power_on_param117
	},
	{
		DTYPE_DCS_WRITE1, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param118), cmi_power_on_param118
	},
	{
		DTYPE_DCS_WRITE1, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param119), cmi_power_on_param119
	},
	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param120), cmi_power_on_param120
	},
	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param121), cmi_power_on_param121
	},
	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param122), cmi_power_on_param122
	},
	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param123), cmi_power_on_param123
	},
	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param124), cmi_power_on_param124
	},
	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param125), cmi_power_on_param125
	},
	{
		DTYPE_DCS_WRITE1, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param126), cmi_power_on_param126
	},
	{
		DTYPE_DCS_WRITE1, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param127), cmi_power_on_param127
	},

	{
		DTYPE_DCS_WRITE1, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param128), cmi_power_on_param128
	},
	{
		DTYPE_DCS_WRITE1, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param129), cmi_power_on_param129
	},
	{
		DTYPE_DCS_WRITE1, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param130), cmi_power_on_param130
	},
	{
		DTYPE_DCS_WRITE1, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param131), cmi_power_on_param131
	},
	{
		DTYPE_DCS_WRITE1, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param132), cmi_power_on_param132
	},
	{
		DTYPE_DCS_WRITE1, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param133), cmi_power_on_param133
	},
	{
		DTYPE_DCS_WRITE1, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param134), cmi_power_on_param134
	},

	/* set pwm freq to 39.06KHz */
	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param135), cmi_power_on_param135
	},
	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param136), cmi_power_on_param136
	},

	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param_d1), cmi_power_on_param_d1
	},
	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param137), cmi_power_on_param137
	},

	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param138), cmi_power_on_param138
	},

	// Disable edge enhancement
	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param139), cmi_power_on_param139
	},
	{
		DTYPE_DCS_LWRITE, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param140), cmi_power_on_param140
	},
	{
		DTYPE_DCS_WRITE1, 0, 100, WAIT_TYPE_US,
		sizeof(cmi_power_on_param141), cmi_power_on_param141
	},

	{
		DTYPE_DCS_WRITE1, 0, 200, WAIT_TYPE_US,
		sizeof(bl_value), bl_value
	},
	{
		DTYPE_DCS_WRITE1, 0, 200, WAIT_TYPE_US,
		sizeof(bl_enable), bl_enable
	},
#if  CMI_CABC_ENABLED
	{
		DTYPE_DCS_WRITE1, 0, 200, WAIT_TYPE_US,
	 	sizeof(cabc_mode), cabc_mode
	},
#endif
	{DTYPE_DCS_WRITE, 0, 120, WAIT_TYPE_MS, sizeof(exit_sleep), exit_sleep},
	{DTYPE_DCS_WRITE, 0, 20, WAIT_TYPE_MS, sizeof(display_on), display_on},
};

static struct dsi_cmd_desc lcd_display_off_cmds[] =
{
	{
		DTYPE_DCS_WRITE1, 0, 200, WAIT_TYPE_US,
		sizeof(bl_level_0), bl_level_0
	},
	{
		DTYPE_DCS_WRITE, 0, 20, WAIT_TYPE_MS,
		sizeof(display_off), display_off
	},
	{
		DTYPE_DCS_WRITE, 0, 120, WAIT_TYPE_MS,
		sizeof(enter_sleep), enter_sleep
	}
};

/*******************************************************************************
** LCD GPIO
*/

#define GPIO_LCD_VCC1V8_NAME	"gpio_lcd_vcc1v8"
#define GPIO_LCD_P5V5_ENABLE_NAME	"gpio_lcd_p5v5_enable"
#define GPIO_LCD_N5V5_ENABLE_NAME "gpio_lcd_n5v5_enable"
#define GPIO_LCD_RESET_NAME	"gpio_lcd_reset"
#define GPIO_LCD_BL_ENABLE_NAME	"gpio_lcd_bl_enable"

static uint32_t gpio_lcd_vcc1v8;
static uint32_t gpio_lcd_p5v5_enable;
static uint32_t gpio_lcd_n5v5_enable;
static uint32_t gpio_lcd_reset;
static uint32_t gpio_lcd_bl_enable;

static struct gpio_desc lcd_gpio_request_cmds[] = {
	/* backlight enable */
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0,
		GPIO_LCD_BL_ENABLE_NAME, &gpio_lcd_bl_enable, 0},
	/* IOVCC _1.8V */
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0,
		GPIO_LCD_VCC1V8_NAME, &gpio_lcd_vcc1v8, 0},
	/* AVDD_5.5V */
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0,
		GPIO_LCD_P5V5_ENABLE_NAME, &gpio_lcd_p5v5_enable, 0},
	/* AVEE_-5.5V */
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0,
		GPIO_LCD_N5V5_ENABLE_NAME, &gpio_lcd_n5v5_enable, 0},
	/* reset */
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0,
		GPIO_LCD_RESET_NAME, &gpio_lcd_reset, 0},
};

static struct gpio_desc lcd_gpio_free_cmds[] = {
	/* backlight enable */
	{DTYPE_GPIO_FREE, WAIT_TYPE_US, 0,
		GPIO_LCD_BL_ENABLE_NAME, &gpio_lcd_bl_enable, 0},
	/* AVEE_-5.5V */
	{DTYPE_GPIO_FREE, WAIT_TYPE_US, 100,
		GPIO_LCD_N5V5_ENABLE_NAME, &gpio_lcd_n5v5_enable, 0},
	/* AVDD_5.5V */
	{DTYPE_GPIO_FREE, WAIT_TYPE_US, 100,
		GPIO_LCD_P5V5_ENABLE_NAME, &gpio_lcd_p5v5_enable, 0},
	/* IOVCC _1.8V */
	{DTYPE_GPIO_FREE, WAIT_TYPE_US, 0,
		GPIO_LCD_VCC1V8_NAME, &gpio_lcd_vcc1v8, 0},
	/* reset */
	{DTYPE_GPIO_FREE, WAIT_TYPE_US, 100,
		GPIO_LCD_RESET_NAME, &gpio_lcd_reset, 0},
};

static struct gpio_desc lcd_gpio_normal_cmds[] = {
	/* IOVCC _1.8V */
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 12,
		GPIO_LCD_VCC1V8_NAME, &gpio_lcd_vcc1v8, 0},
	/* AVDD_5.5V  */
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 10,
		GPIO_LCD_P5V5_ENABLE_NAME, &gpio_lcd_p5v5_enable, 1},
	/* AVEE_-5.5V */
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 50,
		GPIO_LCD_N5V5_ENABLE_NAME, &gpio_lcd_n5v5_enable, 1},
	/* reset */
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 20,
		GPIO_LCD_RESET_NAME, &gpio_lcd_reset, 1},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_US, 20,
		GPIO_LCD_RESET_NAME, &gpio_lcd_reset, 0},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 20,
		GPIO_LCD_RESET_NAME, &gpio_lcd_reset, 1},
	/* backlight enable */
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 5,
		GPIO_LCD_BL_ENABLE_NAME, &gpio_lcd_bl_enable, 1},
};

static struct gpio_desc lcd_gpio_lowpower_cmds[] = {
	/* backlight enable */
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 5,
		GPIO_LCD_BL_ENABLE_NAME, &gpio_lcd_bl_enable, 0},
	/* AVEE_-5.5V */
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 5,
		GPIO_LCD_N5V5_ENABLE_NAME, &gpio_lcd_n5v5_enable, 0},
	/* AVDD_5.5V */
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 5,
		GPIO_LCD_P5V5_ENABLE_NAME, &gpio_lcd_p5v5_enable, 0},
	/* IOVCC _1.8V */
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 5,
		GPIO_LCD_VCC1V8_NAME, &gpio_lcd_vcc1v8, 0},
	/* reset */
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 5,
		GPIO_LCD_RESET_NAME, &gpio_lcd_reset, 0},
	/* backlight enable input */
	{DTYPE_GPIO_INPUT, WAIT_TYPE_US, 100,
		GPIO_LCD_BL_ENABLE_NAME, &gpio_lcd_bl_enable, 0},
	/* AVEE_-5.5V input */
	{DTYPE_GPIO_INPUT, WAIT_TYPE_MS, 5,
		GPIO_LCD_N5V5_ENABLE_NAME, &gpio_lcd_n5v5_enable, 0},
	/* AVDD_5.5V input */
	{DTYPE_GPIO_INPUT, WAIT_TYPE_MS, 5,
		GPIO_LCD_P5V5_ENABLE_NAME, &gpio_lcd_p5v5_enable, 0},
	/* IOVCC _1.8V input */
	{DTYPE_GPIO_INPUT, WAIT_TYPE_MS, 5,
		GPIO_LCD_VCC1V8_NAME, &gpio_lcd_vcc1v8, 0},
	/* reset input */
	{DTYPE_GPIO_INPUT, WAIT_TYPE_US, 100,
		GPIO_LCD_RESET_NAME, &gpio_lcd_reset, 0},
};


/* dsi param initialized value from panel spec */
static void mipi_lcd_init_dsi_param(struct mipi_panel_info *mipi)
{
	mipi->hsa = 3;
	mipi->hbp = 48;
	mipi->dpi_hsize = 542;
	mipi->hline_time = 1385;
	mipi->vsa = 2;
	mipi->vbp = 25;
	mipi->vfp = 25;

	mipi->dsi_bit_clk = 120;
	mipi->dsi_bit_clk_upt = mipi->dsi_bit_clk;
	mipi->dsi_bit_clk_default = mipi->dsi_bit_clk;

	mipi->pxl_clk_rate = 20 * 1000000UL;
	mipi->pxl_clk_rate_div = 1;
	mipi->dsi_bit_clk_upt_support = 0;

	mipi->clk_post_adjust = 215;
	mipi->lane_nums = DSI_4_LANES;
	mipi->color_mode = DSI_24BITS_1;

	mipi->vc = 0;
	mipi->max_tx_esc_clk = 10 * 1000000;
	mipi->burst_mode = DSI_BURST_SYNC_PULSES_1;
	mipi->non_continue_en = 1;
}

static int32_t panel_of_device_setup(struct panel_drv_private *priv)
{
	int32_t ret;
	struct dkmd_connector_info *pinfo = &priv->connector_info;

	dpu_pr_info("enter!\n");

	/* Inheritance based processing */
	panel_base_of_device_setup(priv);

	pinfo->base.type = PANEL_MIPI_VIDEO;
	/* 1. config base object info
	 * would be used for framebuffer setup
	 */
	pinfo->base.xres = 720; // Modified for new panel device
	pinfo->base.yres = 1280; // Modified for new panel device

	/* When calculating DPI needs the following parameters */
	pinfo->base.width = 68; // Modified for new panel device
	pinfo->base.height = 121; // Modified for new panel device

	// caculate fps by mipi timing para
	pinfo->base.fps = 60;

	/* 2. config connector info
	 * would be used for dsi & composer setup
	 */
	mipi_lcd_init_dsi_param(&get_primary_connector(pinfo)->mipi);

	/* dsi or composer need this param */
	pinfo->dirty_region_updt_support = 0;

	/* 3. config panel private info
	 * would be used for panel setup
	 */
	pinfo->bl_info.bl_min = 1;
	pinfo->bl_info.bl_max = 255;
	pinfo->bl_info.bl_default = 102;
	priv->mipi_brightness_para_type = MIPI_BL_PARA1_DBV0;

	priv->disp_on_cmds = lcd_display_on_cmds;
	priv->disp_on_cmds_len = (uint32_t)ARRAY_SIZE(lcd_display_on_cmds);
	priv->disp_off_cmds = lcd_display_off_cmds;
	priv->disp_off_cmds_len = (uint32_t)ARRAY_SIZE(lcd_display_off_cmds);

	/* Don't need to operate vcc and pinctrl for fpga */
	if (pinfo->base.fpga_flag == 1) {
		priv->vcc_enable_cmds_len = 0;
		priv->vcc_disable_cmds_len = 0;
		priv->pinctrl_normal_cmds_len = 0;
		priv->pinctrl_lowpower_cmds_len = 0;
		priv->pinctrl_init_cmds_len = 0;
		priv->pinctrl_finit_cmds_len = 0;
		priv->vcc_init_cmds_len = 0;
		priv->vcc_finit_cmds_len = 0;
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

panel_device_match_data(nt35521_panel_info, PANEL_NT35521_ID, panel_of_device_setup, panel_of_device_release);

MODULE_LICENSE("GPL");
