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

#ifndef _TI_DESER_CONFIG_H_
#define _TI_DESER_CONFIG_H_

#include "i2c_common.h"

#define DS90UH988_I2C_7BIT_ADDR 0x2c
#define DS90UH988_I2C_8BIT_ADDR 0x58
#define DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS 0x2b
#define DS90UH988_I2C_8BIT_ADDR_SMART_ALIAS 0x56
#define DS90UH988_I2C_7BIT_ADDR_METER_ALIAS 0x30
#define DS90UH988_I2C_8BIT_ADDR_METER_ALIAS 0x60
#define DS90UH988_I2C_7BIT_ADDR_PASSENGER_ALIAS 0x2e
#define DS90UH988_I2C_8BIT_ADDR_PASSENGER_ALIAS 0x5c
#define DESER_REG_MAXCNT 0xf5
#define GENERAL_PURPOSE_STATUS 0x54
#define GENERAL_PURPOSE_STATUS_LOCK 0x01

static ti_i2c_cmd_entry_t ti988_mediabox_edp0_edp1_pannel_init[] = {
	/* *********************************************
	** i2c
	** ********************************************/
	{ 0x2b, 0x16, 5, REG_OP_W, REG_DEF_MSK },
	{ 0x2c, 0x16, 5, REG_OP_W, REG_DEF_MSK },
	{ 0x44, 0x81, 0, REG_OP_W, REG_DEF_MSK },  // tp irq_en
	/* *********************************************
	** Hold Des DTG in reset
	** ********************************************/
	{ 0x40, 0x50, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Select DTG Page
	{ 0x41, 0x32, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x42, 0x06, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Hold Port 0 DTG in reset
	{ 0x41, 0x62, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x42, 0x06, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Hold Port 1 DTG in reset
	/* *********************************************
	** Disable Stream Mapping
	** ********************************************/
	{ 0x0e, 0x03, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Select both Output Ports
	{ 0xd0, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	// Disable FPD4 video forward to Output Port
	{ 0xd7, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	// Disable FPD3 video forward to Output Port
	/* *********************************************
	** Setup DTG for port 0
	** ********************************************/
	{ 0x40, 0x50, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Select DTG Page
	{ 0x41, 0x20, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x42, 0x53, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	// Set up DTG BPP,Sync Polarities,and Measurement Type
	{ 0x41, 0x29, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Set Hstart
	{ 0x42, 0x80, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Hstart upper byte
	{ 0x41, 0x2a, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x42, 0x38, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Hstart lower byte
	{ 0x41, 0x2f, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Set HSW
	{ 0x42, 0x40, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // HSW upper byte
	{ 0x41, 0x30, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x42, 0x0c, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // HSW lower byte
	/* *********************************************
	** Map video to display output
	** ********************************************/
	{ 0x0e, 0x03, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Select both Output Ports
	{ 0xd0, 0x0c, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	// Enable FPD_RX video forward to Output Port
	{ 0xd1, 0x0f, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	// Every stream forwarded on DC
	{ 0xd6, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	// Send Stream 0 to Output Port 0 and Send Stream 0 to Output Port 1
	{ 0xd7, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // FPD3 mapping disabled
	{ 0x0e, 0x01, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Select Port 0
	/* *********************************************
	** Configure 988 Display
	** ********************************************/
	{ 0x40, 0x2c, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	// Configure OLDI/RGB Port Settings
	{ 0x41, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x42, 0x2f, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x41, 0x01, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x42, 0x2f, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x40, 0x2e, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Configure OLDI/RGB PLL
	{ 0x41, 0x08, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x42, 0x41, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // PLL_NUM23_16
	{ 0x42, 0x0a, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // PLL_NUM15_8
	{ 0x42, 0x3d, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // PLL_NUM7_0
	{ 0x42, 0xff, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // PLL_DEN23_16
	{ 0x42, 0xfd, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // PLL_DEN15_8
	{ 0x42, 0x02, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // PLL_DEN7_0
	{ 0x41, 0x18, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x42, 0x2a, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // PLL_NDIV
	{ 0x41, 0x2d, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x42, 0x11, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // TX_SEL_CLKDIV
	/* *********************************************
	** Release Des DTG reset
	** ********************************************/
	{ 0x40, 0x50, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Select DTG Page
	{ 0x41, 0x32, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x42, 0x04, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Release Port 0 DTG
	{ 0x41, 0x62, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x42, 0x04, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Release Port 1 DTG
	/* *********************************************
	** Enable OLDI Output
	** ********************************************/
	{ 0x01, 0x40, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // OLDI Reset
	{ 0x40, 0x2c, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Enable OLDI/RGB
	{ 0x41, 0x02, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x42, 0x14, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x41, 0x02, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	// Toggle OLDI_SER_EN for Dual OLDI Mode
	{ 0x42, 0x04, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x42, 0x14, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x41, 0x20, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // P0 TX_EN
	{ 0x42, 0x80, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x41, 0x22, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // P1 TX_EN
	{ 0x42, 0x80, NO_DELAY, REG_OP_W, REG_DEF_MSK },
};

static ti_i2c_cmd_entry_t ti988_smart_pannel_init[] = {
	/* *********************************************
	** i2c
	** ********************************************/
	{ 0x2b, 0x16, 5, REG_OP_W, REG_DEF_MSK },
	{ 0x2c, 0x16, 5, REG_OP_W, REG_DEF_MSK },
	{ 0x44, 0x81, 0, REG_OP_W, REG_DEF_MSK },  // tp irq_en
	/* *********************************************
	** Hold Des DTG in reset
	** ********************************************/
	{ 0x40, 0x50, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Select DTG Page
	{ 0x41, 0x32, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x42, 0x06, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Hold Port 0 DTG in reset
	{ 0x41, 0x62, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x42, 0x06, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Hold Port 1 DTG in reset
	/* *********************************************
	** Disable Stream Mapping
	** ********************************************/
	{ 0x0e, 0x03, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Select both Output Ports
	{ 0xd0, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	// Disable FPD4 video forward to Output Port
	{ 0xd7, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	// Disable FPD3 video forward to Output Port
	/* *********************************************
	** Setup DTG for port 0
	** ********************************************/
	{ 0x40, 0x50, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Select DTG Page
	{ 0x41, 0x20, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x42, 0x53, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	// Set up DTG BPP,Sync Polarities,and Measurement Type
	{ 0x41, 0x29, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Set Hstart
	{ 0x42, 0x80, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Hstart upper byte
	{ 0x41, 0x2a, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x42, 0x28, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Hstart lower byte
	{ 0x41, 0x2f, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Set HSW
	{ 0x42, 0x40, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // HSW upper byte
	{ 0x41, 0x30, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x42, 0x0c, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // HSW lower byte
	/* *********************************************
	** Map video to display output
	** ********************************************/
	{ 0x0e, 0x03, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Select both Output Ports
	{ 0xd0, 0x0c, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	// Enable FPD_RX video forward to Output Port
	{ 0xd1, 0x0f, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	// Every stream forwarded on DC
	{ 0xd6, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	// Send Stream 0 to Output Port 0 and Send Stream 0 to Output Port 1
	{ 0xd7, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // FPD3 mapping disabled
	{ 0x0e, 0x01, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Select Port 0
	/* *********************************************
	** Configure 988 Display
	** ********************************************/
	{ 0x40, 0x2c, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	// Configure OLDI/RGB Port Settings
	{ 0x41, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x42, 0x2f, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x41, 0x01, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x42, 0x2f, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x40, 0x2e, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Configure OLDI/RGB PLL
	{ 0x41, 0x08, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x42, 0x7b, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // PLL_NUM23_16
	{ 0x42, 0xe8, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // PLL_NUM15_8
	{ 0x42, 0x47, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // PLL_NUM7_0
	{ 0x42, 0xff, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // PLL_DEN23_16
	{ 0x42, 0xf7, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // PLL_DEN15_8
	{ 0x42, 0xbc, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // PLL_DEN7_0
	{ 0x41, 0x18, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x42, 0x29, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // PLL_NDIV
	{ 0x41, 0x2d, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x42, 0x11, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // TX_SEL_CLKDIV
	/* *********************************************
	** Release Des DTG reset
	** ********************************************/
	{ 0x40, 0x50, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Select DTG Page
	{ 0x41, 0x32, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x42, 0x04, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Release Port 0 DTG
	{ 0x41, 0x62, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x42, 0x04, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Release Port 1 DTG
	/* *********************************************
	** Enable OLDI Output
	** ********************************************/
	{ 0x01, 0x40, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // OLDI Reset
	{ 0x40, 0x2c, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Enable OLDI/RGB
	{ 0x41, 0x02, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x42, 0x14, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x41, 0x02, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	// Toggle OLDI_SER_EN for Dual OLDI Mode
	{ 0x42, 0x04, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x42, 0x14, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x41, 0x20, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // P0 TX_EN
	{ 0x42, 0x80, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x41, 0x22, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // P1 TX_EN
	{ 0x42, 0x80, NO_DELAY, REG_OP_W, REG_DEF_MSK },
};

static ti_i2c_cmd_entry_t ti988_meter_pannel_init[] = {
	/* *********************************************
	** i2c
	** ********************************************/
	{ 0x2b, 0x16, 5, REG_OP_W, REG_DEF_MSK },
	{ 0x2c, 0x16, 5, REG_OP_W, REG_DEF_MSK },
	/* *********************************************
	** Hold Des DTG in reset
	** ********************************************/
	{ 0x40, 0x50, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Select DTG Page
	{ 0x41, 0x32, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x42, 0x06, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Hold Port 0 DTG in reset
	{ 0x41, 0x62, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x42, 0x06, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Hold Port 1 DTG in reset
	/* *********************************************
	** Disable Stream Mapping
	** ********************************************/
	{ 0x0e, 0x03, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Select both Output Ports
	{ 0xd0, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	// Disable FPD4 video forward to Output Port
	{ 0xd7, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	// Disable FPD3 video forward to Output Port
	/* *********************************************
	** Setup DTG for port 0
	** ********************************************/
	{ 0x40, 0x50, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Select DTG Page
	{ 0x41, 0x20, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x42, 0x53, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	// Set up DTG BPP,	Sync Polarities,  and Measurement Type
	{ 0x41, 0x29, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Set Hstart
	{ 0x42, 0x80, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Hstart upper byte
	{ 0x41, 0x2a, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x42, 0x28, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Hstart lower byte
	{ 0x41, 0x2f, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Set HSW
	{ 0x42, 0x40, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // HSW upper byte
	{ 0x41, 0x30, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x42, 0x10, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // HSW lower byte
	/* *********************************************
	** Map video to display output
	** ********************************************/
	{ 0x0e, 0x03, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Select both Output Ports
	{ 0xd0, 0x0c, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	// Enable FPD_RX video forward to Output Port
	{ 0xd1, 0x0f, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	// Every stream forwarded on DC
	{ 0xd6, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	// Send Stream 0 to Output Port 0 and Send Stream 0 to Output Port 1
	{ 0xd7, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // FPD3 mapping disabled
	{ 0x0e, 0x01, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Select Port 0
	/* *********************************************
	** Configure 988 Display
	** ********************************************/
	{ 0x40, 0x2c, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	// Configure OLDI/RGB Port Settings
	{ 0x41, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x42, 0x2f, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x41, 0x01, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x42, 0x2f, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x40, 0x2e, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Configure OLDI/RGB PLL
	{ 0x41, 0x08, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x42, 0xba, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // PLL_NUM23_16
	{ 0x42, 0xc8, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // PLL_NUM15_8
	{ 0x42, 0x5d, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // PLL_NUM7_0
	{ 0x42, 0xff, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // PLL_DEN23_16
	{ 0x42, 0xff, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // PLL_DEN15_8
	{ 0x42, 0x1e, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // PLL_DEN7_0
	{ 0x41, 0x18, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x42, 0x2f, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // PLL_NDIV
	{ 0x41, 0x2d, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x42, 0x12, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // TX_SEL_CLKDIV
	/* *********************************************
	** Release Des DTG reset
	** ********************************************/
	{ 0x40, 0x50, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Select DTG Page
	{ 0x41, 0x32, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x42, 0x04, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Release Port 0 DTG
	{ 0x41, 0x62, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x42, 0x04, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Release Port 1 DTG
	/* *********************************************
	** Enable OLDI Output
	** ********************************************/
	{ 0x01, 0x40, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // OLDI Reset
	{ 0x40, 0x2c, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Enable OLDI/RGB
	{ 0x41, 0x02, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x42, 0x14, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x41, 0x02, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	// Toggle OLDI_SER_EN for Dual OLDI Mode
	{ 0x42, 0x04, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x42, 0x14, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x41, 0x20, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // P0 TX_EN
	{ 0x42, 0x80, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x41, 0x22, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // P1 TX_EN
	{ 0x42, 0x80, NO_DELAY, REG_OP_W, REG_DEF_MSK },
};

static i2c_cmd_entry_t deser_988_smart_pannel_init[] = {
	/* *********************************************
	** i2c
	** ********************************************/
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x2b, 0x19, 5, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x2c, 0x21, 5, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x44, 0x81, 0, REG_OP_W, REG_DEF_MSK},  // tp irq_en
	/* *********************************************
	** Hold Des DTG in reset
	** ********************************************/
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x40, 0x50, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // Select DTG Page
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x41, 0x32, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x06, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // Hold Port 0 DTG in reset
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x41, 0x62, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x06, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // Hold Port 1 DTG in reset
	/* *********************************************
	** Disable Stream Mapping
	** ********************************************/
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x0e, 0x03, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // Select both Output Ports
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0xd0, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	// Disable FPD4 video forward to Output Port
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0xd7, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	// Disable FPD3 video forward to Output Port
	/* *********************************************
	** Setup DTG for port 0
	** ********************************************/
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x40, 0x50, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // Select DTG Page
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x41, 0x20, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x53, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	// Set up DTG BPP,Sync Polarities,and Measurement Type
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x41, 0x29, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // Set Hstart
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x80, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // Hstart upper byte
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x41, 0x2a, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x3c, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // Hstart lower byte
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x41, 0x2f, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // Set HSW
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x40, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // HSW upper byte
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x41, 0x30, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x14, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // HSW lower byte
	/* *********************************************
	** Map video to display output
	** ********************************************/
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x0e, 0x03, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // Select both Output Ports
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0xd0, 0x0c, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	// Enable FPD_RX video forward to Output Port
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0xd1, 0x0f, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	// Every stream forwarded on DC
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0xd6, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	// Send Stream 0 to Output Port 0 and Send Stream 0 to Output Port 1
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0xd7, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // FPD3 mapping disabled
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x0e, 0x01, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // Select Port 0
	/* *********************************************
	** Configure 988 Display
	** ********************************************/
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x40, 0x2c, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	// Configure OLDI/RGB Port Settings
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x41, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x2f, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x41, 0x01, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x2f, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x40, 0x2e, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // Configure OLDI/RGB PLL
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x41, 0x08, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x38, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // PLL_NUM23_16
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0xe3, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // PLL_NUM15_8
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x8c, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // PLL_NUM7_0
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0xff, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // PLL_DEN23_16
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0xff, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // PLL_DEN15_8
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0xf6, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // PLL_DEN7_0
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x41, 0x18, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x22, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // PLL_NDIV
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x41, 0x2d, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x11, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // TX_SEL_CLKDIV
	/* *********************************************
	** Release Des DTG reset
	** ********************************************/
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x40, 0x50, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // Select DTG Page
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x41, 0x32, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x04, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // Release Port 0 DTG
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x41, 0x62, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x04, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // Release Port 1 DTG
	/* *********************************************
	** Enable OLDI Output
	** ********************************************/
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x01, 0x40, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // OLDI Reset
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x40, 0x2c, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // Enable OLDI/RGB
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x41, 0x02, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x14, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x41, 0x02, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	// Toggle OLDI_SER_EN for Dual OLDI Mode
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x04, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x14, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x41, 0x20, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // P0 TX_EN
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x80, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x41, 0x22, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // P1 TX_EN
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x80, NO_DELAY, REG_OP_W, REG_DEF_MSK},
};

static i2c_cmd_entry_t deser_988_smart_pannel_init_dsi1[] = {
	/* *********************************************
	** i2c
	** ********************************************/
	{ DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x2b, 0x16, 5, REG_OP_W, REG_DEF_MSK },
	{ DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x2c, 0x16, 5, REG_OP_W, REG_DEF_MSK },
	{ DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x44, 0x81, 0, REG_OP_W, REG_DEF_MSK }, // tp irq_en
	/* *********************************************
	** Hold Des DTG in reset
	** ********************************************/
	{ DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x40, 0x50, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Select DTG Page
	{ DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x41, 0x32, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x06, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Hold Port 0 DTG in reset
	{ DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x41, 0x62, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x06, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Hold Port 1 DTG in reset
	/* *********************************************
	** Disable Stream Mapping
	** ********************************************/
	{ DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x0e, 0x03, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Select both Output Ports
	{ DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0xd0, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	// Disable FPD4 video forward to Output Port
	{ DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0xd7, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	// Disable FPD3 video forward to Output Port
	/* *********************************************
	** Setup DTG for port 0
	** ********************************************/
	{ DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x40, 0x50, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Select DTG Page
	{ DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x41, 0x20, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x53, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	// Set up DTG BPP,Sync Polarities,and Measurement Type
	{ DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x41, 0x29, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Set Hstart
	{ DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x80, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Hstart upper byte
	{ DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x41, 0x2a, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x3c, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Hstart lower byte
	{ DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x41, 0x2f, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Set HSW
	{ DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x40, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // HSW upper byte
	{ DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x41, 0x30, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x14, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // HSW lower byte
	/* *********************************************
	** Map video to display output
	** ********************************************/
	{ DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x0e, 0x03, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Select both Output Ports
	{ DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0xd0, 0x0c, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	// Enable FPD_RX video forward to Output Port
	{ DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0xd1, 0x0f, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	// Every stream forwarded on DC
	{ DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0xd6, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	// Send Stream 0 to Output Port 0 and Send Stream 0 to Output Port 1
	{ DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0xd7, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // FPD3 mapping disabled
	{ DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x0e, 0x01, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Select Port 0
	/* *********************************************
	** Configure 988 Display
	** ********************************************/
	{ DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x40, 0x2c, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	// Configure OLDI/RGB Port Settings
	{ DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x41, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x2f, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x41, 0x01, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x2f, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x40, 0x2e, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Configure OLDI/RGB PLL
	{ DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x41, 0x08, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x38, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // PLL_NUM23_16
	{ DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0xe3, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // PLL_NUM15_8
	{ DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x8c, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // PLL_NUM7_0
	{ DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0xff, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // PLL_DEN23_16
	{ DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0xff, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // PLL_DEN15_8
	{ DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0xf6, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // PLL_DEN7_0
	{ DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x41, 0x18, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x22, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // PLL_NDIV
	{ DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x41, 0x2d, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x11, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // TX_SEL_CLKDIV
	/* *********************************************
	** Release Des DTG reset
	** ********************************************/
	{ DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x40, 0x50, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Select DTG Page
	{ DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x41, 0x32, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x04, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Release Port 0 DTG
	{ DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x41, 0x62, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x04, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Release Port 1 DTG
	/* *********************************************
	** Enable OLDI Output
	** ********************************************/
	{ DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x01, 0x40, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // OLDI Reset
	{ DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x40, 0x2c, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Enable OLDI/RGB
	{ DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x41, 0x02, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x14, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x41, 0x02, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	// Toggle OLDI_SER_EN for Dual OLDI Mode
	{ DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x04, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x14, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x41, 0x20, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // P0 TX_EN
	{ DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x80, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x41, 0x22, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // P1 TX_EN
	{ DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x80, NO_DELAY, REG_OP_W, REG_DEF_MSK },
};

static i2c_cmd_entry_t deser_988_meter_pannel_init[] = {
	/* *********************************************
	** Hold Des DTG in reset
	** ********************************************/
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x40, 0x50, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // Select DTG Page
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x41, 0x32, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x06, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // Hold Port 0 DTG in reset
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x41, 0x62, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x06, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // Hold Port 1 DTG in reset
	/* *********************************************
	** Disable Stream Mapping
	** ********************************************/
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x0e, 0x03, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // Select both Output Ports
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0xd0, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	// Disable FPD4 video forward to Output Port
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0xd7, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	// Disable FPD3 video forward to Output Port
	/* *********************************************
	** Setup DTG for port 0
	** ********************************************/
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x40, 0x50, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // Select DTG Page
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x41, 0x20, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x53, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	// Set up DTG BPP,Sync Polarities,and Measurement Type
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x41, 0x29, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // Set Hstart
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x80, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // Hstart upper byte
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x41, 0x2a, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x28, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // Hstart lower byte
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x41, 0x2f, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // Set HSW
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x40, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // HSW upper byte
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x41, 0x30, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x10, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // HSW lower byte
	/* *********************************************
	** Map video to display output
	** ********************************************/
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x0e, 0x03, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // Select both Output Ports
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0xd0, 0x0c, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	// Enable FPD_RX video forward to Output Port
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0xd1, 0x0f, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	// Every stream forwarded on DC
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0xd6, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	// Send Stream 0 to Output Port 0 and Send Stream 0 to Output Port 1
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0xd7, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // FPD3 mapping disabled
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x0e, 0x01, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // Select Port 0
	/* *********************************************
	** Configure 988 Display
	** ********************************************/
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x40, 0x2c, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	// Configure OLDI/RGB Port Settings
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x41, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x2f, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x41, 0x01, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x2f, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x40, 0x2e, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // Configure OLDI/RGB PLL
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x41, 0x08, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0xba, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // PLL_NUM23_16
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0xc8, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // PLL_NUM15_8
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x5d, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // PLL_NUM7_0
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0xff, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // PLL_DEN23_16
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0xff, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // PLL_DEN15_8
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x1e, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // PLL_DEN7_0
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x41, 0x18, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x2f, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // PLL_NDIV
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x41, 0x2d, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x12, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // TX_SEL_CLKDIV
	/* *********************************************
	** Release Des DTG reset
	** ********************************************/
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x40, 0x50, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // Select DTG Page
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x41, 0x32, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x04, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // Release Port 0 DTG
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x41, 0x62, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x04, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // Release Port 1 DTG
	/* *********************************************
	** Enable OLDI Output
	** ********************************************/
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x01, 0x40, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // OLDI Reset
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x40, 0x2c, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // Enable OLDI/RGB
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x41, 0x02, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x14, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x41, 0x02, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	// Toggle OLDI_SER_EN for Dual OLDI Mode
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x04, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x14, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x41, 0x20, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // P0 TX_EN
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x80, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x41, 0x22, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // P1 TX_EN
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x80, NO_DELAY, REG_OP_W, REG_DEF_MSK},
};

static i2c_cmd_entry_t deser_988_meter_pannel_init_dsi0[] = {
	/* *********************************************
	** Hold Des DTG in reset
	** ********************************************/
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x40, 0x50, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Select DTG Page
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x41, 0x32, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x06, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Hold Port 0 DTG in reset
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x41, 0x62, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x06, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Hold Port 1 DTG in reset
	/* *********************************************
	** Disable Stream Mapping
	** ********************************************/
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x0e, 0x03, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Select both Output Ports
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0xd0, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	// Disable FPD4 video forward to Output Port
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0xd7, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	// Disable FPD3 video forward to Output Port
	/* *********************************************
	** Setup DTG for port 0
	** ********************************************/
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x40, 0x50, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Select DTG Page
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x41, 0x20, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x53, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	// Set up DTG BPP,Sync Polarities,and Measurement Type
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x41, 0x29, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Set Hstart
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x80, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Hstart upper byte
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x41, 0x2a, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x28, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Hstart lower byte
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x41, 0x2f, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Set HSW
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x40, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // HSW upper byte
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x41, 0x30, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x10, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // HSW lower byte
	/* *********************************************
	** Map video to display output
	** ********************************************/
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x0e, 0x03, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // Select both Output Ports
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0xd0, 0x0c, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	// Enable FPD_RX video forward to Output Port
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0xd1, 0x0f, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	// Every stream forwarded on DC
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0xd6, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	// Send Stream 0 to Output Port 0 and Send Stream 0 to Output Port 1
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0xd7, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // FPD3 mapping disabled
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x0e, 0x01, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // Select Port 0
	/* *********************************************
	** Configure 988 Display
	** ********************************************/
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x40, 0x2c, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	// Configure OLDI/RGB Port Settings
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x41, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x2f, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x41, 0x01, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x2f, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x40, 0x2e, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // Configure OLDI/RGB PLL
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x41, 0x08, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0xba, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // PLL_NUM23_16
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0xc8, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // PLL_NUM15_8
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x5d, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // PLL_NUM7_0
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0xff, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // PLL_DEN23_16
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0xff, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // PLL_DEN15_8
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x1e, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // PLL_DEN7_0
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x41, 0x18, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x2f, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // PLL_NDIV
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x41, 0x2d, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x12, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // TX_SEL_CLKDIV
	/* *********************************************
	** Release Des DTG reset
	** ********************************************/
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x40, 0x50, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // Select DTG Page
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x41, 0x32, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x04, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // Release Port 0 DTG
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x41, 0x62, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x04, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // Release Port 1 DTG
	/* *********************************************
	** Enable OLDI Output
	** ********************************************/
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x01, 0x40, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // OLDI Reset
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x40, 0x2c, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // Enable OLDI/RGB
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x41, 0x02, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x14, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x41, 0x02, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	// Toggle OLDI_SER_EN for Dual OLDI Mode
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x04, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x14, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x41, 0x20, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // P0 TX_EN
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x80, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x41, 0x22, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // P1 TX_EN
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x80, NO_DELAY, REG_OP_W, REG_DEF_MSK},
};

static i2c_cmd_entry_t deser_988_icpic_pannel_init[] = {
	/* *********************************************
	** Hold Des DTG in reset
	** ********************************************/
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x40, 0x50, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // Select DTG Page
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x41, 0x32, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x06, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // Hold Port 0 DTG in reset
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x41, 0x62, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x06, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // Hold Port 1 DTG in reset
	/* *********************************************
	** Disable Stream Mapping
	** ********************************************/
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x0e, 0x03, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // Select both Output Ports
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0xd0, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	// Disable FPD4 video forward to Output Port
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0xd7, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	// Disable FPD3 video forward to Output Port
	/* *********************************************
	** Setup DTG for port 0
	** ********************************************/
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x40, 0x50, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // Select DTG Page
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x41, 0x20, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x53, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	// Set up DTG BPP,	Sync Polarities,  and Measurement Type
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x41, 0x29, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // Set Hstart
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x80, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // Hstart upper byte
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x41, 0x2a, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x28, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // Hstart lower byte
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x41, 0x2f, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // Set HSW
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x40, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // HSW upper byte
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x41, 0x30, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x14, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // HSW lower byte
	/* *********************************************
	** Map video to display output
	** ********************************************/
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x0e, 0x03, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // Select both Output Ports
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0xd0, 0x0c, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	// Enable FPD_RX video forward to Output Port
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0xd1, 0x0f, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	// Every stream forwarded on DC
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0xd6, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	// Send Stream 0 to Output Port 0 and Send Stream 0 to Output Port 1
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0xd7, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // FPD3 mapping disabled
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x0e, 0x01, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // Select Port 0
	/* *********************************************
	** Configure 988 Display
	** ********************************************/
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x40, 0x2c, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	// Configure OLDI/RGB Port Settings
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x41, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x2f, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x41, 0x01, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x2f, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x40, 0x2e, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // Configure OLDI/RGB PLL
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x41, 0x08, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0xa1, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // PLL_NUM23_16
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x2f, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // PLL_NUM15_8
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x62, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // PLL_NUM7_0
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0xff, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // PLL_DEN23_16
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0xff, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // PLL_DEN15_8
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0xf6, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // PLL_DEN7_0
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x41, 0x18, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x2d, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // PLL_NDIV
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x41, 0x2d, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x11, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // TX_SEL_CLKDIV
	/* *********************************************
	** Release Des DTG reset
	** ********************************************/
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x40, 0x50, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // Select DTG Page
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x41, 0x32, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x04, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // Release Port 0 DTG
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x41, 0x62, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x04, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // Release Port 1 DTG
	/* *********************************************
	** Enable OLDI Output
	** ********************************************/
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x01, 0x40, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // OLDI Reset
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x40, 0x2c, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // Enable OLDI/RGB
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x41, 0x02, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x14, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x41, 0x02, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	// Toggle OLDI_SER_EN for Dual OLDI Mode
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x04, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x14, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x41, 0x20, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // P0 TX_EN
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x80, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x41, 0x22, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // P1 TX_EN
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x80, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	/* *********************************************
	** Set up deserializer Patgen
	** *******************************************	*/
	{ DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x40, 0x50, NO_DELAY, REG_OP_W, REG_DEF_MSK },  // SET PATGEN PAGE
	{ DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x41, 0x01, NO_DELAY, REG_OP_W, REG_DEF_MSK },  // SET PATGEN ADDRESS
	{ DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x0c, NO_DELAY, REG_OP_W, REG_DEF_MSK },  // SET BIT PER PIXEL
	{ DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x41, 0x02, NO_DELAY, REG_OP_W, REG_DEF_MSK },  // SET PATGEN ADDRESS
	{ DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x86, NO_DELAY, REG_OP_W, REG_DEF_MSK },  // SET PATGEN ADDRESS
	// AUTO INCREMENT
	{ DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x41, 0x03, NO_DELAY, REG_OP_W, REG_DEF_MSK },  // SET PATGEN ADDRESS
	{ DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0xf8, NO_DELAY, REG_OP_W, REG_DEF_MSK },  // SET PATGEN THW
	{ DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x1f, NO_DELAY, REG_OP_W, REG_DEF_MSK },  // SET PATGEN THW
	{ DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x0c, NO_DELAY, REG_OP_W, REG_DEF_MSK },  // SET PATGEN TVW
	{ DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x03, NO_DELAY, REG_OP_W, REG_DEF_MSK },  // SET PATGEN TVW
	{ DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x80, NO_DELAY, REG_OP_W, REG_DEF_MSK },  // SET PATGEN AHW
	{ DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x1f, NO_DELAY, REG_OP_W, REG_DEF_MSK },  // SET PATGEN AHW
	{ DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0xf4, NO_DELAY, REG_OP_W, REG_DEF_MSK },  // SET PATGEN AVW
	{ DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x02, NO_DELAY, REG_OP_W, REG_DEF_MSK },  // SET PATGEN AVW
	{ DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x28, NO_DELAY, REG_OP_W, REG_DEF_MSK },  // SET PATGEN HSW
	{ DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK },  // SET PATGEN HSW
	{ DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x02, NO_DELAY, REG_OP_W, REG_DEF_MSK },  // SET PATGEN VSW
	{ DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK },  // SET PATGEN VSW
	{ DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x28, NO_DELAY, REG_OP_W, REG_DEF_MSK },  // SET PATGEN HBP
	{ DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK },  // SET PATGEN HBP
	{ DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x0a, NO_DELAY, REG_OP_W, REG_DEF_MSK },  // SET PATGEN VBP
	{ DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK },  // SET PATGEN VBP
	{ DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	// HSYNC Polarity = +, VSYNC Polarity = +
	{ DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x41, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK },  // SET PATGEN ADDRESS
	{ DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x94, NO_DELAY, REG_OP_W, REG_DEF_MSK },  // ENABLE PATGEN COLOR BAR
};

static i2c_cmd_entry_t deser_988_passenger_pannel_init[] = {
	/* *********************************************
	** i2c
	** ********************************************/
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x2b, 0x19, 5, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x2c, 0x21, 5, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x44, 0x81, 0, REG_OP_W, REG_DEF_MSK},  // tp irq_en
	/* *********************************************
	** Hold Des DTG in reset
	** ********************************************/
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x40, 0x50, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // Select DTG Page
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x41, 0x32, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x06, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // Hold Port 0 DTG in reset
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x41, 0x62, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x06, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // Hold Port 1 DTG in reset
	/* *********************************************
	** Disable Stream Mapping
	** ********************************************/
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x0e, 0x03, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // Select both Output Ports
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0xd0, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	// Disable FPD4 video forward to Output Port
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0xd7, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	// Disable FPD3 video forward to Output Port
	/* *********************************************
	** Setup DTG for port 0
	** ********************************************/
	{ DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x40, 0x50, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Select DTG Page
	{ DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x41, 0x20, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x13, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	// Set up DTG BPP,Sync Polarities,and Measurement Type
	{ DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x41, 0x29, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Set Hstart
	{ DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x80, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Hstart upper byte
	{ DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x41, 0x2a, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x34, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Hstart lower byte
	{ DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x41, 0x2f, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Set HSW
	{ DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x40, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // HSW upper byte
	{ DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x41, 0x30, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x14, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // HSW lower byte
	/* *********************************************
	** Setup DTG for port 1
	** ********************************************/
	{ DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x40, 0x50, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Select DTG Page
	{ DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x41, 0x50, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x13, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	// Set up DTG BPP,Sync Polarities,and Measurement Type
	{ DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x41, 0x59, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Set Hstart
	{ DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x80, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Hstart upper byte
	{ DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x41, 0x5a, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x34, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Hstart lower byte
	{ DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x41, 0x5f, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Set HSW
	{ DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x40, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // HSW upper byte
	{ DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x41, 0x60, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x14, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // HSW lower byte
	/* *********************************************
	** Map video to display output
	** ********************************************/
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x0e, 0x03, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // Select both Output Ports
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0xd0, 0x0c, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	// Enable FPD_RX video forward to Output Port
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0xd1, 0x0f, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	// Every stream forwarded on DC
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0xd6, 0x0a, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	// Send Stream 0 to Output Port 0 and Send Stream 0 to Output Port 1
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0xd7, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // FPD3 mapping disabled
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x0e, 0x01, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // Select Port 0
	/* *********************************************
	** Configure 988 Display
	** ********************************************/
	{ DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x40, 0x2c, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	// Configure OLDI/RGB Port Settings
	{ DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x41, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x2f, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x41, 0x01, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x2f, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x40, 0x2e, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Configure OLDI/RGB PLL
	{ DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x41, 0x08, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x12, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // PLL_NUM23_16
	{ DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0xf6, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // PLL_NUM15_8
	{ DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x84, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // PLL_NUM7_0
	{ DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0xff, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // PLL_DEN23_16
	{ DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0xff, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // PLL_DEN15_8
	{ DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0xf6, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // PLL_DEN7_0
	{ DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x41, 0x18, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x1e, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // PLL_NDIV
	{ DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x41, 0x2d, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x10, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // TX_SEL_CLKDIV
	/* *********************************************
	** Release Des DTG reset
	** ********************************************/
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x40, 0x50, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // Select DTG Page
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x41, 0x32, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x04, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // Release Port 0 DTG
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x41, 0x62, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x04, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // Release Port 1 DTG
	/* *********************************************
	** Enable OLDI Output
	** ********************************************/
	{ DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x01, 0x40, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // OLDI Reset
	{ DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x40, 0x2c, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Enable OLDI/RGB
	{ DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x41, 0x02, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x11, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x41, 0x02, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	// Toggle OLDI_SER_EN for Dual OLDI Mode
	{ DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x04, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x11, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x41, 0x20, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // P0 TX_EN
	{ DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x80, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x41, 0x22, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // P1 TX_EN
	{ DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x80, NO_DELAY, REG_OP_W, REG_DEF_MSK },
};

static i2c_cmd_entry_t deser0_988_config_for_dp[] = {

	/* *********************************************
	 ** Hold Des DTG in reset
	 ** ********************************************/
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x40, 0x50, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // Select DTG Page
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x41, 0x32, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x6, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Hold Local Display Output
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x41, 0x62, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x6, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Hold Local Display Output
	/* *********************************************
	** Disable Stream Mapping
	** ********************************************/
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0xe, 0x3, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Select both Output Ports
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0xd0, 0x0, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Disable FPD4 video forward to
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0xd7, 0x0, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Disable FPD3 video forward to

	/* *********************************************
	** Setup DTG for port 0
	** ********************************************/
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x40, 0x50, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Select DTG Page
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x41, 0x20, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	// Set up Local Display DTG BPP, Sync Polarities, and Measurement Type
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x53, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x41, 0x29, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Set Hstart
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x80, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Hstart upper byte
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x41, 0x2a, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x3c, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Hstart lower byte
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x41, 0x2f, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Set HSW
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x40, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // HSW upper byte
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x41, 0x30, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x14, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // HSW lower byte
	/* *********************************************
	** Map video to display output
	** ********************************************/
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0xe, 0x3, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Select both Output Ports
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0xd0, 0xc, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Enable FPD_RX video forward
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0xd1, 0xf, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Every stream forwarded on DC
	// Send Stream 0 to Output Port 0 and Send Stream 0 to Output Port 1
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0xd6, 0x0, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0xd7, 0x0, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // FPD3 to local display output
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0xe, 0x1, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Select Port 0
	/* *********************************************
	 ** Set up deserializer Patgen
	 ** ********************************************/
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x40, 0x50, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Set Patgen page
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x41, 0x1, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Set patgen address
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0xc, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Set bit per pixel
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x41, 0x2, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Set patgen address
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x86, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Set patgen address auto
	// increment
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x41, 0x3, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Set patgen address
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0xa0, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Set patgen THW
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0xf, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Set patgen THW
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x5c, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Set patgen TVW
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x4, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Set patgen TVW
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x0, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Set patgen AHW

	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0xf, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Set patgen AHW
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x38, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Set patgen AVW
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x4, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Set patgen AVW
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x28, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Set patgen HSW

	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x0, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Set patgen HSW
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x2, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Set patgen VSW
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x0, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Set patgen VSW
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x50, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Set patgen HBP

	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x0, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Set patgen HBP
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x1e, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Set patgen VBP
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x0, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Set patgen VBP
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x0, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // HSYNC Polarity = +, VSYNC
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x41, 0x0, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Set patgen address
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x94, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Enable Patgen color bar

	/* *********************************************
	 ** Configure 988 Display
	 ** ********************************************/
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x40, 0x2c, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // Configure OLDI/RGB Port
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x41, 0x0, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x2f, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x41, 0x1, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x2f, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x40, 0x2e, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // Configure OLDI/RGB PLL
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x41, 0x8, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0xbd, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // PLL_NUM23_16
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0xa1, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // PLL_NUM15_8
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x28, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // PLL_NUM7_0
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0xff, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // PLL_DEN23_16
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0xff, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // PLL_DEN15_8
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0xf6, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // PLL_DEN7_0
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x41, 0x18, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x22, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // PLL_NDIV
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x41, 0x2d, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x11, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // TX_SEL_CLKDIV
	/* *********************************************
	 ** Release Des DTG reset
	 ** ********************************************/
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x40, 0x50, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // Select DTG Page
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x41, 0x32, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x4, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Release Local Display Output
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x41, 0x62, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x4, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Release Local Display Output
	/* *********************************************
	 ** Enable OLDI Output
	 ** ********************************************/
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x1, 0x40, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // OLDI Reset
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x40, 0x2c, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // Enable OLDI/RGB
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x41, 0x2, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x14, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x41, 0x2, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Toggle OLDI_SER_EN for Dual
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x4, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x14, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x41, 0x20, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // P0 TX_EN
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x80, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x41, 0x22, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // P1 TX_EN
	{DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, 0x80, NO_DELAY, REG_OP_W, REG_DEF_MSK},
};

static i2c_cmd_entry_t deser1_988_config_for_dp[] = {

	/* *********************************************
	 ** Hold Des DTG in reset
	 ** ********************************************/
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x40, 0x50, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // Select DTG Page
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x41, 0x32, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x6, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Hold Local Display Output
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x41, 0x62, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x6, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Hold Local Display Output

	/* *********************************************
	** Disable Stream Mapping
	** ********************************************/
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0xe, 0x3, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Select both Output Ports
	// Disable FPD4 video forward to Output Port
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0xd0, 0x0, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	// Disable FPD3 video forward to Output Port
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0xd7, 0x0, NO_DELAY, REG_OP_W, REG_DEF_MSK},

	/* *********************************************
	** Setup DTG for port 0
	** ********************************************/
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x40, 0x50, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // Select DTG Page
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x41, 0x20, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x53, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	// Set up Local Display DTG BPP, Sync Polarities, and Measurement Type
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x41, 0x53, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // Set Hstart
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x80, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // Hstart upper byte
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x41, 0x2a, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x3c, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // Hstart lower byte
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x41, 0x2f, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // Set HSW
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x40, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // HSW upper byte
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x41, 0x30, NO_DELAY, REG_OP_W, REG_DEF_MSK},  //
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x14, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // HSW lower byte
	/* *********************************************
	** Map video to display output
	** ********************************************/
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0xe, 0x3, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // Select both Output Ports
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0xd0, 0xc, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Enable FPD_RX video forward
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0xd1, 0xf, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Every stream forwarded on DC
	// Send Stream 1 to Output Port 0 and Send Stream 1 to Output Port 1
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0xd6, 0x0, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	// FPD3 to local display output mapping disabled
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0xd7, 0x0, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0xe, 0x1, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // Select Port 0

	/* *********************************************
	 ** Set up deserializer Patgen
	 ** ********************************************/
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x40, 0x50, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // Set Patgen page
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x41, 0x1, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Set patgen address
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0xc, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Set bit per pixel
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x41, 0x2, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Set patgen address
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x86, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // Set patgen address auto
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x41, 0x3, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Set patgen address
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0xa0, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // Set patgen THW
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x0f, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // Set patgen THW
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x5c, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // Set patgen TVW
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x4, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Set patgen TVW
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x0, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Set patgen AHW

	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x0f, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // Set patgen AHW
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x38, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // Set patgen AVW
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x4, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Set patgen AVW
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x28, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // Set patgen HSW

	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x0, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Set patgen HSW
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x2, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Set patgen VSW
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x0, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Set patgen VSW
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x50, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Set patgen HBP

	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x0, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Set patgen HBP
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x1e, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // Set patgen VBP
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x0, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Set patgen VBP
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x0, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // HSYNC Polarity = +, VSYNC
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x41, 0x0, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Set patgen address
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x94, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Enable Patgen color bar

	/* *********************************************
	 ** Configure 988 Display
	 ** ********************************************/
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x40, 0x2c, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // Configure OLDI/RGB Port
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x41, 0x0, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x2f, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x41, 0x1, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x2f, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x40, 0x2e, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // Configure OLDI/RGB PLL
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x41, 0x8, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0xbd, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // PLL_NUM23_16
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0xa1, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // PLL_NUM15_8
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x28, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // PLL_NUM7_0
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0xff, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // PLL_DEN23_16
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0xff, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // PLL_DEN15_8
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0xf6, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // PLL_DEN7_0
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x41, 0x18, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x22, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // PLL_NDIV
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x41, 0x2d, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x11, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // TX_SEL_CLKDIV
	/* *********************************************
	 ** Release Des DTG reset
	 ** ********************************************/
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x40, 0x50, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // Select DTG Page
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x41, 0x32, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x04, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // Release Local Display Output
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x41, 0x62, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x04, NO_DELAY, REG_OP_W, REG_DEF_MSK},  // Release Local Display Output
	/* *********************************************
	 ** Enable OLDI Output
	 ** ********************************************/
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x01, 0x40, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // OLDI Reset
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x40, 0x2c, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Enable OLDI/RGB
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x41, 0x02, NO_DELAY, REG_OP_W, REG_DEF_MSK},

	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x14, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	// Toggle OLDI_SER_EN for Dual OLDI Mode
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x41, 0x02, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x04, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x14, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x41, 0x20, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // P0 TX_EN
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x80, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x41, 0x22, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // P1 TX_EN
	{DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x42, 0x80, NO_DELAY, REG_OP_W, REG_DEF_MSK},

};

static ti_i2c_cmd_entry_t deser_988_passenger_pannel_init_for_dp[] = {
	/* *********************************************
	** i2c
	** ********************************************/
	{ 0x2b, 0x16, 5, REG_OP_W, REG_DEF_MSK },
	{ 0x2c, 0x16, 5, REG_OP_W, REG_DEF_MSK },
	/* *********************************************
	** Hold Des DTG in reset
	** ********************************************/
	{ 0x40, 0x50, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Select DTG Page
	{ 0x41, 0x32, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x42, 0x06, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Hold Port 0 DTG in reset
	{ 0x41, 0x62, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x42, 0x06, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Hold Port 1 DTG in reset
	/* *********************************************
	** Disable Stream Mapping
	** ********************************************/
	{ 0x0e, 0x03, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Select both Output Ports
	{ 0xd0, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	// Disable FPD4 video forward to Output Port
	{ 0xd7, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	// Disable FPD3 video forward to Output Port
	/* *********************************************
	** Setup DTG for port 0
	** ********************************************/
	{ 0x40, 0x50, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Select DTG Page
	{ 0x41, 0x20, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x42, 0x53, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	// Set up DTG BPP,Sync Polarities,and Measurement Type
	{ 0x41, 0x29, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Set Hstart
	{ 0x42, 0x80, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Hstart upper byte
	{ 0x41, 0x2a, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x42, 0x38, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Hstart lower byte
	{ 0x41, 0x2f, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Set HSW
	{ 0x42, 0x40, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // HSW upper byte
	{ 0x41, 0x30, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x42, 0x0c, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // HSW lower byte
	/* *********************************************
	** Map video to display output
	** ********************************************/
	{ 0x0e, 0x03, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Select both Output Ports
	{ 0xd0, 0x0c, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	// Enable FPD_RX video forward to Output Port
	{ 0xd1, 0x0f, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	// Every stream forwarded on DC
	{ 0xd6, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	// Send Stream 0 to Output Port 0 and Send Stream 0 to Output Port 1
	{ 0xd7, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // FPD3 mapping disabled
	{ 0x0e, 0x01, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Select Port 0
	/* *********************************************
	** Configure 988 Display
	** ********************************************/
	{ 0x40, 0x2c, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	// Configure OLDI/RGB Port Settings
	{ 0x41, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x42, 0x2f, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x41, 0x01, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x42, 0x2f, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x40, 0x2e, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Configure OLDI/RGB PLL
	{ 0x41, 0x08, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x42, 0x41, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // PLL_NUM23_16
	{ 0x42, 0x0a, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // PLL_NUM15_8
	{ 0x42, 0x3d, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // PLL_NUM7_0
	{ 0x42, 0xff, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // PLL_DEN23_16
	{ 0x42, 0xfd, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // PLL_DEN15_8
	{ 0x42, 0x02, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // PLL_DEN7_0
	{ 0x41, 0x18, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x42, 0x2a, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // PLL_NDIV
	{ 0x41, 0x2d, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x42, 0x11, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // TX_SEL_CLKDIV
	/* *********************************************
	** Release Des DTG reset
	** ********************************************/
	{ 0x40, 0x50, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Select DTG Page
	{ 0x41, 0x32, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x42, 0x04, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Release Port 0 DTG
	{ 0x41, 0x62, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x42, 0x04, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Release Port 1 DTG
	/* *********************************************
	** Enable OLDI Output
	** ********************************************/
	{ 0x01, 0x40, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // OLDI Reset
	{ 0x40, 0x2c, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Enable OLDI/RGB
	{ 0x41, 0x02, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x42, 0x14, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x41, 0x02, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	// Toggle OLDI_SER_EN for Dual OLDI Mode
	{ 0x42, 0x04, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x42, 0x14, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x41, 0x20, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // P0 TX_EN
	{ 0x42, 0x80, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x41, 0x22, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // P1 TX_EN
	{ 0x42, 0x80, NO_DELAY, REG_OP_W, REG_DEF_MSK },
};

#endif
