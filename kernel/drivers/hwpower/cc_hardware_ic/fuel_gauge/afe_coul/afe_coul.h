 // SPDX-License-Identifier: GPL-2.0
 /*
  * afe_coul.c
  *
  * driver for AFE(analog front ending) coul gauge
  *
  * Copyright (c) 2023-2023 Huawei Technologies Co., Ltd.
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

#ifndef _AFE_COUL_H_
#define _AFE_COUL_H_

#include <linux/device.h>
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/mutex.h>
#include <linux/workqueue.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/bitops.h>
#include <chipset_common/hwpower/common_module/power_time.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/common_module/power_i2c.h>

#define AFE_COUL_ID_CELLWISE			0x0001
#define AFE_COUL_ID_SGM				0x7104
#define AFE_COUL_NUM_CC_REG			4
#define AFE_COUL_NUM_FIFO			10
#define AFE_COUL_VOLT_REG_BASE			32000
#define AFE_COUL_VOLT_REG_UNIT			625 /* unit 10uv */
#define AFE_COUL_VOLT_DEFAULT			3800
#define AFE_COUL_UNIT_CHIP_TEMP			4 /* unit 0.25 celsius */
#define AFE_COUL_UNIT_CUR_REG			916 /* (30000 * 1000 / 32768) */
#define AFE_COUL_CUR_REG_A			30000
#define AFE_COUL_CUR_REG_B			32768
#define AFE_COUL_COE_C_OFFSET			2048
#define AFE_COUL_LOW_VOLT_INT_A			8
#define AFE_COUL_LOW_VOLT_INT_B			2300
#define AFE_COUL_DELAY_10MS			10
#define AFE_COUL_DELAY_500MS			500
#define AFE_COUL_RESISTANCE_SENSE		20 /* 2mohm */
#define AFE_COUL_MAX_CC_MAH			20000
#define AFE_COUL_DEFAULT_CC			0
#define AFE_COUL_INDEX_AUX_COUL			1

#define AFE_COUL_CMD_LEN			1
#define AFE_COUL_CMD_LEN_MAX			32
#define AFE_COUL_BITS_PER_BYTE			8
#define AFE_COUL_ONE_BYTE			1
#define AFE_COUL_TWO_BYTES			2
#define AFE_COUL_THREE_BYTES			3
#define AFE_COUL_FOUR_BYTES			4
#define AFE_COUL_CC_REG0			0
#define AFE_COUL_CC_REG1			1
#define AFE_COUL_CC_REG2			2
#define AFE_COUL_CC_REG3			3
#define AFE_COUL_THREE_BYTE_BITS		24
#define AFE_COUL_TWO_BYTE_BITS			16

/* Register Address Definition For AFE */
#define AFE_COUL_REG_VERSION			0x01
#define AFE_COUL_REG_CURRENT			0x03
#define AFE_COUL_REG_VOLTAGE			0x05
#define AFE_COUL_REG_BAT_TEMP			0x07
#define AFE_COUL_REG_CC_IN			0x09
#define AFE_COUL_REG_CC_OUT			0x0D
#define AFE_COUL_REG_VOLT_FIFO_BASE		0x11
#define AFE_COUL_REG_CURR_FIFO_BASE		0x25
#define AFE_COUL_REG_TEMP_FIFO_BASE		0x39
#define AFE_COUL_REG_CHIP_TEMP			0x4D
#define AFE_COUL_REG_VOLT_GAIN			0x93
#define AFE_COUL_REG_CURR_GAIN			0x97
#define AFE_COUL_REG_CURR_OFFSET		0x99
#define AFE_COUL_REG_UNDER_VOLT			0xA0
#define AFE_COUL_REG_INT_TRI			0xA6
#define AFE_COUL_REG_INT_FLAG			0xAB
#define AFE_COUL_REG_CHIP_STATUS		0xAD
#define AFE_COUL_REG_OPCG1			0xAF
#define AFE_COUL_REG_OPCG2			0xB1
#define AFE_COUL_REG_OPCG3			0xB3
#define AFE_COUL_REG_OPCG4			0xB5
#define AFE_COUL_REG_INT_PIN_CLT		0xB6
#define AFE_COUL_REG_ALGO_FLAG1			0xD0
#define AFE_COUL_REG_ALGO_FLAG2			0xD1
#define AFE_COUL_REG_OCV			0xD2
#define AFE_COUL_REG_OCV_TEMP			0xD4
#define AFE_COUL_REG_LAST_SOC			0xD6
#define AFE_COUL_REG_BAT_MOVE			0xD7
#define AFE_COUL_REG_CC_OVERFLOW		0xD8
#define AFE_COUL_REG_LAST_TIME			0xDC

/* Bit Definition of AFE Status Register */
#define AFE_COUL_ACTIVE_MODE_FLAG		BIT(15)
#define AFE_COUL_LP_MODE_FLAG			BIT(14)
#define AFE_COUL_ULP_MODE_FLAG			BIT(13)
#define AFE_COUL_SLEEP_MODE_FLAG		BIT(12)
#define AFE_COUL_CALI_MODE_FLAG			BIT(11)
#define AFE_COUL_BAT_PRES_FLAG			BIT(10)
#define AFE_COUL_BSA_FLAG			BIT(7)

/* Bit Definition of INT flag Register */
#define AFE_COUL_LOW_VOLT_MASK			BIT(12)
#define AFE_COUL_CC_OVERFLOW_MASK		BIT(8)

/* Bit Definition of OPCG1 Register */
#define AFE_COUL_IRQ_EN_MASK			(0xF << 12)
#define AFE_COUL_VOL_IRQ_EN			BIT(14)
#define AFE_COUL_CC_EN				BIT(6)
#define AFE_COUL_BD_PRES_EN			BIT(5)

/* Bit Definition of OPCG2 Register */
#define AFE_COUL_CC_MAX_MASK			(0x3FF << 5)
#define AFE_COUL_CC_MAX_BEGIN			5

/* Bit Definition of OPCG3 Register */
#define AFE_COUL_CURR_CALI_CMD			BIT(15)
#define AFE_COUL_VBAT_CALI_CMD			BIT(14)
#define AFE_COUL_ENTR_ACT_CMD			BIT(13)
#define AFE_COUL_ENTR_LP_CMD			BIT(11)
#define AFE_COUL_ENTR_ULP_CMD			BIT(9)
#define AFE_COUL_ENTR_CALI_CMD			BIT(7)
#define AFE_COUL_ENTR_SP_CMD			BIT(5)
#define AFE_COUL_CC_RESET			BIT(3)
#define AFE_COUL_AFE_RESET			BIT(2)
#define AFE_COUL_FIFO_RESET			BIT(1)

/* Bit Definition of OPCG4 Register */
#define AFE_COUL_ENTR_SD_CMD			BIT(3)

/* Bit Definition of Pin Ctr Register */
#define AFE_COUL_VOLT_LO_IRQ			BIT(12)
#define AFE_COUL_CC_HI_IRQ			BIT(8)

/* Bit Definition of Algorithm Flag1 Register */
#define AFE_COUL_USE_SAVED_OCV_FLAG 		BIT(5)
#define AFE_COUL_DRC_SCENE_FLAG			BIT(3) /* delta rc ignore flag */
#define AFE_COUL_NV_READ_FLAG			BIT(1)
#define AFE_COUL_NV_SAVE_FLAG			BIT(0)
#define AFE_COUL_NV_SUCCESS			1
#define AFE_COUL_NV_FAIL			0

/* Bit Definition of Algorithm Flag2 Register */
#define AFE_COUL_BAT_EXIST_STATE_FLAG   	BIT(1)
#define AFE_COUL_LOW_BAT_POWER_ON_FLAG  	BIT(0)
#define AFE_COUL_SAVE_OCV_LEVEL         	(BIT(5) | BIT(4) | BIT(3) | BIT(2))
#define AFE_COUL_OCV_LEVEL_SHIFT        	2
#define AFE_COUL_OCV_LEVEL_MAX          	8

/* Bit Definition of OCV Reg */
#define AFE_COUL_INVALID_TO_UPDATE_FCC  	0x8000
#define AFE_COUL_VALID_TO_UPDATE_FCC    	0
#define AFE_COUL_USE_CLAC_OCV           	0xABCD
#define AFE_COUL_OCV_REG_UNIT           	1000 /* 10uv */
#define AFE_COUL_OCV_REG_BASE           	20000

/* Bit Definition of Bat Move Reg */
#define AFE_COUL_BATTERY_MOVE_MAGIC_NUM		0xc3
#define AFE_COUL_BATTERY_PLUGOUT_SHUTDOWN_MAGIC_NUM 0x18

/* Bit Definition of Last_SOC Reg */
#define AFE_COUL_SAVE_LAST_SOC_FLAG     	BIT(7)
#define AFE_COUL_SAVE_LAST_SOC			0x7F

/* Calibration Para */
#define AFE_COUL_VOLT_GAIN_DEFAULT		1000000
#define AFE_COUL_VOLT_GAIN_MIN			937500
#define AFE_COUL_VOLT_GAIN_MAX			1062500
#define AFE_COUL_VOLT_GAIN_BASE			32678
#define AFE_COUL_VOLT_GAIN_OFFSET		34726
#define AFE_COUL_CURR_GAIN_DEFAULT		1000000
#define AFE_COUL_CURR_GAIN_MAX			1200000
#define AFE_COUL_CURR_OFFSET_MAX		200000
#define AFE_COUL_CURR_OFFSET_MIN		(-200000)
#define AFE_COUL_CURR_GAIN_MIN			800000
#define AFE_COUL_CURR_GAIN_BASE			2048
#define AFE_COUL_CURR_GAIN_OFFSET		4096

/* AFE ntc parameter */
#define AFE_COUL_TEMP_UNIT			10 /* 0.1 celsius */
#define AFE_COUL_DEFAULT_TEMP			(25 * AFE_COUL_TEMP_UNIT)
#define AFE_COUL_NTC_MIN_TEMP			((-30) * AFE_COUL_TEMP_UNIT)
#define AFE_COUL_NTC_MAX_TEMP			(80 * AFE_COUL_TEMP_UNIT)
#define AFE_COUL_NTC_SERIES_R			1800
#define AFE_COUL_DEFAULT_NTC_R			1000
#define AFE_COUL_TS_PULL_UP_VOLT		1800

#define AFE_COUL_MAX_TEMPS			10
#define AFE_COUL_MAX_RECORDS_CNT		5

struct afe_coul_device {
	struct i2c_client *client;
	struct device *dev;
	struct workqueue_struct *afe_workqueue;
	struct delayed_work irq_work;
	int irq;
	int abs_cc_mah;
	int coul_index;
	int coul_time_offset;
	int batt_moved_flag;
	int force_offset_cali;
	int c_offset_ua;
	unsigned int r_sense;
	unsigned int int_gpio;
};

#endif