/* Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
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

#ifndef _TI983_SER_CONFIG_H_
#define _TI983_SER_CONFIG_H_

#include "../common/i2c_common.h"
#include "ti_deser_config.h"

#define DS90UB983_SER1_I2C_7BIT_ADDR 0x18
#define DS90UB983_SER1_I2C_8BIT_ADDR 0x30

#define DS90UB983_SER2_I2C_7BIT_ADDR 0x14
#define DS90UB983_SER2_I2C_8BIT_ADDR 0x28

#define TI983_DES_ID 0x08
#define TI983_GENERAL_STS 0x0c
#define EDID_LEN 128
static uint8_t edid_for_arhud_maxview[EDID_LEN] = {
	0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x4D, 0x79, 0x01, 0x00, 0x01, 0x01, 0x01, 0x01,
	0x33, 0x18, 0x01, 0x03, 0x81, 0x00, 0x00, 0x78, 0x0A, 0xEE, 0x9D, 0xA3, 0x54, 0x4C, 0x99, 0x26,
	0x0F, 0x47, 0x4A, 0xBF, 0xEF, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0xDF, 0x3F, 0x38, 0x98, 0x40, 0x70, 0x34, 0x80, 0x60, 0x0C,
	0x0A, 0x04, 0x44, 0x87, 0x00, 0x00, 0x00, 0x1E, 0x66, 0x21, 0x50, 0xB0, 0x51, 0x00, 0x1B, 0x30,
	0x40, 0x70, 0x36, 0x00, 0xC4, 0x8E, 0x21, 0x00, 0x00, 0x1E, 0x00, 0x00, 0x00, 0xFC, 0x00, 0x53,
	0x6B, 0x79, 0x77, 0x6F, 0x72, 0x74, 0x68, 0x20, 0x54, 0x56, 0x0A, 0x20, 0x00, 0x00, 0x00, 0xFD,
	0x00, 0x30, 0x4C, 0x0F, 0x55, 0x3C, 0x00, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x01, 0x70
};

static uint8_t edid_for_passanger[EDID_LEN] = {
	0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x4D, 0x79, 0x01, 0x00, 0x01, 0x01, 0x01, 0x01,
	0x33, 0x18, 0x01, 0x03, 0x81, 0x00, 0x00, 0x78, 0x0A, 0xEE, 0x9D, 0xA3, 0x54, 0x4C, 0x99, 0x26,
	0x0F, 0x47, 0x4A, 0xBF, 0xEF, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0xDF, 0x3F, 0x38, 0x98, 0x40, 0x70, 0x34, 0x80, 0x60, 0x0C,
	0x0A, 0x04, 0x44, 0x87, 0x00, 0x00, 0x00, 0x1E, 0x66, 0x21, 0x50, 0xB0, 0x51, 0x00, 0x1B, 0x30,
	0x40, 0x70, 0x36, 0x00, 0xC4, 0x8E, 0x21, 0x00, 0x00, 0x1E, 0x00, 0x00, 0x00, 0xFC, 0x00, 0x53,
	0x6B, 0x79, 0x77, 0x6F, 0x72, 0x74, 0x68, 0x20, 0x54, 0x56, 0x0A, 0x20, 0x00, 0x00, 0x00, 0xFD,
	0x00, 0x30, 0x4C, 0x0F, 0x55, 0x3C, 0x00, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x01, 0x70
};

static ti_i2c_cmd_entry_t ser1_983_init_desalias[] = {
	/* *********************************************
	** Set up Variables
	** ********************************************/
	{ 0x70, DS90UH988_I2C_8BIT_ADDR, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Passenger panel
	{ 0x78, DS90UH988_I2C_8BIT_ADDR_SMART_ALIAS, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x88, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK },

	{ 0x71, DS90UH988_I2C_8BIT_ADDR, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Smart panel
	{ 0x79, DS90UH988_I2C_8BIT_ADDR_METER_ALIAS + 1, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x89, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK },

	{ 0x72, 0x84, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // MCU
	{ 0x7a, 0x84, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x8a, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x73, 0x90, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // TP
	{ 0x7b, 0x94, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x8b, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x74, 0xa8, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // NFC
	{ 0x7c, 0xa8, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x8c, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x75, 0x88, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // light sensor
	{ 0x7d, 0x88, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x8d, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x76, 0xd8, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // LM
	{ 0x7e, 0xd8, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x8e, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK },

	{ 0x1b, 0x98, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // gpio4 port1
};

static ti_i2c_cmd_entry_t ser2_983_init_desalias[] = {
	/* *********************************************
	** Set up Variables
	** ********************************************/
	{ 0x70, DS90UH988_I2C_8BIT_ADDR, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Passenger panel
	{ 0x78, DS90UH988_I2C_8BIT_ADDR_PASSENGER_ALIAS, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x88, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK },

	{ 0x71, 0x84, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // MCU
	{ 0x79, 0x84, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x89, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK },

	{ 0x72, 0xa0, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // TP
	{ 0x7a, 0xa0, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x8a, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK },

	{ 0x73, 0x88, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // light sensor
	{ 0x7b, 0x88, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x8b, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK },

	{ 0x1b, 0x88, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // gpio4 port0
};

static ti_i2c_cmd_entry_t ser_983_init_fpdlink_mode[] = {
	/* *********************************************
	** Program SER to FPD-Link IV mode
	** ********************************************/
	{ 0x5b, 0x23, NO_DELAY, REG_OP_W, REG_DEF_MSK },  // Disable FPD3 FIFO pass through
	{ 0x05, 0x3c, NO_DELAY, REG_OP_W, REG_DEF_MSK },  // Force FPD4_TX independent mode
};

static ti_i2c_cmd_entry_t ser_983_init_fpdIV_pll_setting[] = {
	/* *********************************************
	** Set up FPD IV PLL Settings - This section can be commented out to improve bringup time
	** if 983/981 MODE_SEL0 and MODE_SEL2 are strapped to the correct FPD IV speed
	** ********************************************/
	{ 0x40, 0x08, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Select PLL reg page
	{ 0x41, 0x1b, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x42, 0x08, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Disable PLL0
	{ 0x41, 0x5b, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x42, 0x08, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Disable PLL1
	{ 0x02, 0xd1, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Enable mode overwrite
	{ 0x2d, 0x01, NO_DELAY, REG_OP_W, REG_DEF_MSK },

	{ 0x40, 0x08, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Select PLL page
	{ 0x41, 0x05, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Select Ncount Reg
	{ 0x42, 0x7d, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Set Ncount
	{ 0x41, 0x13, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Select post div reg
	{ 0x42, 0x90, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Set post div for 6.75 Gbps
	{ 0x2d, 0x01, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Select write reg to port 0
	{ 0x6a, 0x0a, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // set BC sampling rate
	{ 0x6e, 0x80, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // set BC fractional sampling
	{ 0x40, 0x04, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Select FPD page and set BC settings for FPD IV port 0
	{ 0x41, 0x06, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x42, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x41, 0x0d, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x42, 0x34, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x41, 0x0e, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x42, 0x53, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x40, 0x08, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Select PLL page
	{ 0x41, 0x45, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Select Ncount Reg
	{ 0x42, 0x7d, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Set Ncount
	{ 0x41, 0x53, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Select post div reg
	{ 0x42, 0x90, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Set post div for 6.75 Gbps
	{ 0x2d, 0x12, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Select write reg to port 1
	{ 0x6a, 0x0a, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // set BC sampling rate
	{ 0x6e, 0x80, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // set BC fractional sampling
	{ 0x40, 0x04, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Select FPD page and set BC settings for FPD IV port 1

	{ 0x41, 0x26, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x42, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x41, 0x2d, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x42, 0x34, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x41, 0x2e, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x42, 0x53, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x02, 0xd1, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Set HALFRATE_MODE
};

static ti_i2c_cmd_entry_t ser_983_init_fpdIV_pll_fractional[] = {
	/* *********************************************
	** Zero out PLL fractional - This section can be commented out to improve bringup time
	** if 983/981 MODE_SEL0 and MODE_SEL2 are strapped to the correct FPD IV speed
	** ********************************************/
	{ 0x40, 0x08, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Select PLL page
	{ 0x41, 0x04, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x42, 0x01, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x41, 0x1e, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x42, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x41, 0x1f, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x42, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x41, 0x20, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x42, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x40, 0x08, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Select PLL page
	{ 0x41, 0x44, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x42, 0x01, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x41, 0x5e, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x42, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x41, 0x5f, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x42, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x41, 0x60, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x42, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK },
};

static ti_i2c_cmd_entry_t ser_983_init_fpdIV_enable_pll[] = {
	/* *********************************************
	** Configure and Enable PLLs - This section can be commented out to improve bringup time
	** if 983/981 MODE_SEL0 and MODE_SEL2 are strapped to the correct FPD IV speed
	** ********************************************/
	{ 0x41, 0x0e, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Select VCO reg
	{ 0x42, 0xc7, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Set VCO
	{ 0x41, 0x4e, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Select VCO reg
	{ 0x42, 0xc7, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Set VCO
	{ 0x01, 0x30, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // soft reset PLL
	{ 0x41, 0x1b, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x42, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Enable PLL0
	{ 0x41, 0x5b, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x42, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Enable PLL1
	{ 0x01, 0x01, 0x28, REG_OP_W, REG_DEF_MSK }, // soft reset Ser 40ms
};

static ti_i2c_cmd_entry_t ser_983_init_dp_config[] = {
	/* *********************************************
	 ** Set DP Config
	 ** ********************************************/
	{ 0x48, 0x1, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Enable APB Interface

	{ 0x49, 0x0, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Force HPD low to configure 983 DP settings
	{ 0x4a, 0x0, NO_DELAY, REG_OP_W, REG_DEF_MSK},

	{ 0x4b, 0x0, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{ 0x4c, 0x0, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{ 0x4d, 0x0, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{ 0x4e, 0x0, NO_DELAY, REG_OP_W, REG_DEF_MSK},

	{ 0x49, 0x74, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Set max advertised link rate = 5.4 Gbps
	{ 0x4a, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{ 0x4b, 0x14, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{ 0x4c, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{ 0x4d, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{ 0x4e, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK},

	{ 0x49, 0x70, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Set max advertised lane count = 4
	{ 0x4a, 0x0, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{ 0x4b, 0x4, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{ 0x4c, 0x0, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{ 0x4d, 0x0, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{ 0x4e, 0x0, NO_DELAY, REG_OP_W, REG_DEF_MSK},

	{ 0x49, 0x14, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Request min VOD swing of 0x02
	{ 0x4a, 0x2, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{ 0x4b, 0x2, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{ 0x4c, 0x0, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{ 0x4d, 0x0, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{ 0x4e, 0x0, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	// Lane Rate Optimizations for CS2.0 with 8155 at 5.4Gbps (No SSC)
	{ 0x40, 0x10, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{ 0x41, 0x4b, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{ 0x42, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{ 0x41, 0x4f, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{ 0x42, 0x80, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{ 0x41, 0x30, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{ 0x42, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{ 0x41, 0x56, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{ 0x42, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{ 0x40, 0x10, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{ 0x41, 0xcb, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{ 0x42, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{ 0x41, 0xcf, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{ 0x42, 0x80, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{ 0x41, 0xb0, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{ 0x42, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{ 0x41, 0xd6, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{ 0x42, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{ 0x40, 0x14, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{ 0x41, 0x4b, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{ 0x42, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{ 0x41, 0x4f, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{ 0x42, 0x80, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{ 0x41, 0x30, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{ 0x42, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{ 0x41, 0x56, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{ 0x42, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{ 0x40, 0x14, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{ 0x41, 0xcb, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{ 0x42, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{ 0x41, 0xcf, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{ 0x42, 0x80, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{ 0x41, 0xb0, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{ 0x42, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{ 0x41, 0xd6, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{ 0x42, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK},

	{ 0x49, 0x18, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Set SST/MST mode and DP/eDP Mode
	{ 0x4a, 0x0, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{ 0x4b, 0x14, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{ 0x4c, 0x0, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{ 0x4d, 0x0, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{ 0x4e, 0x0, NO_DELAY, REG_OP_W, REG_DEF_MSK},

	{ 0x49, 0x0, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Force HPD high to trigger link training
	{ 0x4a, 0x0, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{ 0x4b, 0x1, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{ 0x4c, 0x0, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{ 0x4d, 0x0, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{ 0x4e, 0x0, 0xc8, REG_OP_W, REG_DEF_MSK},
	// 200ms,Allow time after HPD is pulled high for the source to train and provide video (may
	// need to adjust based on source properties)
};

static ti_i2c_cmd_entry_t ser_983_init_passengerpanel_config[] = {
	/* *********************************************
	** Program VP Configs
	** ********************************************/
	// Configure VP 0 smart panel
	{ 0x40, 0x32, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{ 0x41, 0x01, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Set VP_SRC_SELECT to Stream 0 for SST
	{ 0x42, 0xa8, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Mode
	{ 0x41, 0x02, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{ 0x42, 0x38, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // VID H Active
	{ 0x42, 0x04, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // VID H Active
	{ 0x41, 0x10, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{ 0x42, 0x38, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Horizontal Active
	{ 0x42, 0x04, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Horizontal Active
	{ 0x42, 0x2c, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Horizontal Back Porch
	{ 0x42, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Horizontal Back Porch
	{ 0x42, 0x0c, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Horizontal Sync
	{ 0x42, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Horizontal Sync
	{ 0x42, 0xd0, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Horizontal Total
	{ 0x42, 0x04, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Horizontal Total
	{ 0x42, 0x70, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Vertical Active
	{ 0x42, 0x08, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Vertical Active
	{ 0x42, 0x1a, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Vertical Back Porch
	{ 0x42, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Vertical Back Porch
	{ 0x42, 0x0a, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Vertical Sync
	{ 0x42, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Vertical Sync
	{ 0x42, 0x10, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Vertical Front Porch
	{ 0x42, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Vertical Front Porch
	{ 0x41, 0x27, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{ 0x42, 0x04, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // HSYNC Polarity = +, VSYNC Polarity = +
	{ 0x41, 0x23, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // M/N Register
	{ 0x42, 0x02, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // M value
	{ 0x42, 0x1f, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // M value
	{ 0x42, 0x0f, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // N value
};

static ti_i2c_cmd_entry_t ser_983_init_smartpanel_vp0_config[] = {
	/* *********************************************
	** Program VP Configs
	** ********************************************/
	// Configure VP 0 smart panel
	{ 0x40, 0x32, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{ 0x41, 0x01, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Set VP_SRC_SELECT to Stream 0 for SST
	{ 0x42, 0xa8, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Mode
	{ 0x41, 0x02, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{ 0x42, 0x38, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // VID H Active
	{ 0x42, 0x04, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // VID H Active
	{ 0x41, 0x10, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{ 0x42, 0x38, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Horizontal Active
	{ 0x42, 0x04, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Horizontal Active
	{ 0x42, 0x2c, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Horizontal Back Porch
	{ 0x42, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Horizontal Back Porch
	{ 0x42, 0x0c, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Horizontal Sync
	{ 0x42, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Horizontal Sync
	{ 0x42, 0xd0, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Horizontal Total
	{ 0x42, 0x04, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Horizontal Total
	{ 0x42, 0x70, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Vertical Active
	{ 0x42, 0x08, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Vertical Active
	{ 0x42, 0x1a, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Vertical Back Porch
	{ 0x42, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Vertical Back Porch
	{ 0x42, 0x0a, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Vertical Sync
	{ 0x42, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Vertical Sync
	{ 0x42, 0x10, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Vertical Front Porch
	{ 0x42, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Vertical Front Porch
	{ 0x41, 0x27, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{ 0x42, 0x04, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // HSYNC Polarity = +, VSYNC Polarity = +
	{ 0x41, 0x23, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // M/N Register
	{ 0x42, 0x02, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // M value
	{ 0x42, 0x1f, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // M value
	{ 0x42, 0x0f, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // N value
};

static ti_i2c_cmd_entry_t ser_983_init_smartpanel_vp1_config[] = {
	// Configure VP 1
	{ 0x40, 0x32, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x41, 0x41, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Set VP_SRC_SELECT to Stream 0 for SST
	{ 0x42, 0xa8, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Mode
	{ 0x41, 0x42, NO_DELAY, REG_OP_W, REG_DEF_MSK },

	{ 0x42, 0x80, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // VID H Active
	{ 0x42, 0x07, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // VID H Active

	{ 0x41, 0x48, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x42, 0x80, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Crop Start X
	{ 0x42, 0x07, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Crop Start X
	{ 0x42, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Crop Start Y
	{ 0x42, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Crop Start Y
	{ 0x42, 0xff, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Crop Stop X
	{ 0x42, 0x0e, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Crop Stop X
	{ 0x42, 0x37, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Crop Stop Y
	{ 0x42, 0x04, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Crop Stop Y

	{ 0x41, 0x50, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x42, 0x80, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Horizontal Active
	{ 0x42, 0x07, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Horizontal Active
	{ 0x42, 0x28, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Horizontal Back Porch
	{ 0x42, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Horizontal Back Porch
	{ 0x42, 0x28, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Horizontal Sync
	{ 0x42, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Horizontal Sync
	{ 0x42, 0xf8, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Horizontal Total
	{ 0x42, 0x07, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Horizontal Total

	{ 0x42, 0x38, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Vertical Active
	{ 0x42, 0x04, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Vertical Active
	{ 0x42, 0x4a, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Vertical Back Porch
	{ 0x42, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Vertical Back Porch
	{ 0x42, 0x02, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Vertical Sync
	{ 0x42, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Vertical Sync
	{ 0x42, 0x0c, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Vertical Front Porch
	{ 0x42, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Vertical Front Porch
	{ 0x41, 0x67, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x42, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // HSYNC Polarity = +, VSYNC Polarity = +

	{ 0x41, 0x40, NO_DELAY, REG_OP_W, REG_DEF_MSK },
	{ 0x42, 0x04, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // Enable Cropping

	{ 0x41, 0x63, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // M/N Register
	{ 0x42, 0x1c, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // M value
	{ 0x42, 0x1b, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // M value
	{ 0x42, 0x0f, NO_DELAY, REG_OP_W, REG_DEF_MSK }, // N value
};

static ti_i2c_cmd_entry_t ser_983_video_input_reset[] = {
	// Video Input Reset if VP is not syncronized
	{ 0x49, 0x54, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{ 0x4a, 0x0, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{ 0x4b, 0x1, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{ 0x4c, 0x0, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{ 0x4d, 0x0, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{ 0x4e, 0x0, NO_DELAY, REG_OP_W, REG_DEF_MSK},
};

static ti_i2c_cmd_entry_t ser_983_enable_patgen[] = {

	/* *********************************************
	 ** Enable PATGEN
	 ** ********************************************/
	{ 0x40, 0x30, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{ 0x41, 0x29, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{ 0x42, 0x08, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Set PATGEN Color Depth to 24bpp for VP0
	{ 0x41, 0x28, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{ 0x42, 0x95, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	// Enable PATGEN on VP0 - Comment out this line to disable PATGEN and enable end to end video
	{ 0x41, 0x69, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{ 0x42, 0x08, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Set PATGEN Color Depth to 24bpp for VP1
	{ 0x41, 0x68, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	{ 0x42, 0x95, NO_DELAY, REG_OP_W, REG_DEF_MSK},
	// Enable PATGEN on VP1 - Comment out this line to disable PATGEN and enable end to end video
};

static ti_i2c_cmd_entry_t ser_983_config_ser_tx_layer[] = {
	/* *********************************************
	 ** Configure Serializer TX Link Layer
	 ** ********************************************/
	{ 0x40, 0x2e, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Link layer Reg page
	{ 0x41, 0x01, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Select LINK0_STREAM_EN
	{ 0x42, 0x01, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Enable Link Layer 0 Streams
	{ 0x41, 0x06, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Select LINK0_SLOT_REQ1
	{ 0x42, 0x41, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Set number of time slots

	{ 0x41, 0x11, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Select LINK1_STREAM_EN
	{ 0x42, 0x01, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Enable Link Layer 1 Streams
	{ 0x41, 0x16, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Select LINK1_SLOT_REQ0
	{ 0x42, 0x41, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Set number of time slots
	{ 0x41, 0x12, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Select LINK1_MAP_REG0
	{ 0x42, 0x01, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Assign link layer stream 0 map
	{ 0x41, 0x02, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Select LINK1_MAP_REG1
	{ 0x42, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Assign link layer stream 0 map

	{ 0x41, 0x20, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Set Link layer vp bpp
	{ 0x42, 0x55, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Set Link layer vp bpp according to VP Bit per pixel
	{ 0x41, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Link layer enable
	{ 0x42, 0x0f, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Link layer enable
};

static ti_i2c_cmd_entry_t ser_983_config_passenger_tx_layer[] = {
	/* *********************************************
	 ** Configure Serializer TX Link Layer
	 ** ********************************************/
	{ 0x40, 0x2e, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Link layer Reg page
	{ 0x41, 0x01, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Select LINK0_STREAM_EN
	{ 0x42, 0x01, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Enable Link Layer 0 Streams
	{ 0x41, 0x06, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Select LINK0_SLOT_REQ1
	{ 0x42, 0x41, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Set number of time slots

	{ 0x41, 0x11, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Select LINK1_STREAM_EN
	{ 0x42, 0x01, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Enable Link Layer 1 Streams
	{ 0x41, 0x16, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Select LINK1_SLOT_REQ0
	{ 0x42, 0x41, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Set number of time slots
	{ 0x41, 0x12, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Select LINK1_MAP_REG0
	{ 0x42, 0x01, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Assign link layer stream 0 map
	{ 0x41, 0x02, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Select LINK1_MAP_REG1
	{ 0x42, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Assign link layer stream 0 map

	{ 0x41, 0x20, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Set Link layer vp bpp
	{ 0x42, 0x55, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Set Link layer vp bpp according to VP Bit per pixel
	{ 0x41, 0x00, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Link layer enable
	{ 0x42, 0x0f, NO_DELAY, REG_OP_W, REG_DEF_MSK}, // Link layer enable
};

#endif
