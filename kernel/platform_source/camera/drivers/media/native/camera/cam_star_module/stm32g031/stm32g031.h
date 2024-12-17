/* SPDX-License-Identifier: GPL-2.0 */
/* stm32g031.h
 *
 * stm32g031 header file
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

#ifndef _STM32G031_H_
#define _STM32G031_H_

#include "../cam_star_module_adapter.h"
#include <huawei_platform/log/hw_log.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/gpio.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/workqueue.h>
#include <linux/bitops.h>
#include <linux/mutex.h>
#include <linux/notifier.h>
#include <linux/delay.h>

/* I2C reg */
#define STM32G031_STAR_CSTATE             0x13
#define STM32G031_MOTOR_SPEED_LOW         0x22
#define STM32G031_MOTOR_SPEED_HIGH        0x23
#define STM32G031_MOTOR_TARGET_POS_LOW    0x24
#define STM32G031_MOTOR_TARGET_POS_HIGH   0x25
#define STM32G031_MOTOR_CTL               0x20
#define STM32G031_MOTOR_CSTATE            0x21

#define STM32G031_CALI_CTL                0x90
#define STM32G031_CALI_STEP_CNT_SET_LOW   0x92
#define STM32G031_CALI_STEP_CNT_SET_HIGH  0x93
#define STM32G031_CALI_SPEED_CNT_SET_LOW  0x94
#define STM32G031_CALI_SPEED_CNT_SET_HIGH 0x95
#define STM32G031_CALI_CMD                0x91
#define STM32G031_CALI_FLASH_DATA_ADDR_L  0x9A
#define STM32G031_CALI_FLASH_DATA_ADDR_H  0x9B
#define STM32G031_CALI_FLASH_DATA_VALUE_L 0x9C
#define STM32G031_CALI_FLASH_DATA_VALUE_H 0x9D
#define STM32G031_CALI_STATE              0xA6
#define STM32G031_CALI_CRC_CALC_VALUE_L   0xA2
#define STM32G031_CALI_CRC_CALC_VALUE_H   0xA3
#define STM32G031_CALI_CRC_REC_VALUE_L    0xA4
#define STM32G031_CALI_CRC_REC_VALUE_H    0xA5
/* I2C value */
#define START_STEPPING                    0x01
#define STOP_STEPPING                     0x00
#define STM32G031_CURR_REAL_HEIGHT_LOW    0x28
#define STM32G031_CURR_REAL_HEIGHT_HIGH   0x29
#define STM32G031_CALI_MOVE_FORWARD_STEP  0x23
#define STM32G031_CALI_MOVE_BACKWARD_STEP 0x24
#define STM32G031_CALI_GET_CURRENT_TMR    0x01
#define STM32G031_CALI_GET_CURRENT_HEIGHT 0x02
#define STM32G031_CALI_TMR_DATA_LOW       0x96
#define STM32G031_CALI_TMR_DATA_HIGH      0x97
#define STM32G031_CALI_REAL_HEIGHT_LOW    0x98
#define STM32G031_CALI_REAL_HEIGHT_HIGH   0x99

int stm32g031_power_on(struct cam_star_module_device_info *di);
int stm32g031_set_bootloader(struct cam_star_module_device_info *di);
int stm32g031_calib_write_flash(struct cam_star_module_device_info *di, u16 data);
int stm32g031_calib_read_flash(struct cam_star_module_device_info *di,
	u16 offset, u16 *read_value);
#endif /* _STM32G031_H_ */
