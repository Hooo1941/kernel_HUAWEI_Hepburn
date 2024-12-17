/* SPDX-License-Identifier: GPL-2.0 */
/* hc32l110.h
 *
 * hc32l110 header file
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

#ifndef _HC32L110_H_
#define _HC32L110_H_

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

#define HC32L110_FW_VER                         20240327
#define I2C_MAX_WAIT_RETRY                      30
#define BST_VOL_6V                              6000
#define CAM_SENSORMOTOR_HEIGHT_DATA_LEN         20
#define CAM_SENSORMOTOR_CALIB_DATA_MAX_LEN      292
#define CAM_SENSORMOTOR_CALIB_ADD_INFO_MAX_LEN  12
#define CAM_SENSORMOTOR_LOG_NUM_MAX             128
#define READ_BOOTLOADERID_RETRY                 3

/* flash addr */
#define HC32L110_CALI_DATA_TYPE                 0x126
/* I2C reg */
#define HC32L110_SW_MAIN_VER                    0x0C
#define HC32L110_SW_SUB_VER                     0x0D
#define HC32L110_SW_PATCH_VER                   0x0E
#define HC32L110_STAR_STATUS                    0x10
#define HC32L110_STAR_CTL                       0x11
#define HC32L110_STAR_FUNC_CFG                  0x12
#define HC32L110_STAR_CSTATE                    0x13
#define HC32L110_STAR_ISR_EVT                   0x14
#define HC32L110_STAR_ISR_EVT_MASK              0x15
#define HC32L110_STAR_ISR_ERR1                  0x16
#define HC32L110_STAR_ISR_ERR1_MASK             0x17
#define HC32L110_STAR_ISR_ERR2                  0x18
#define HC32L110_STAR_ISR_ERR2_MASK             0x19
#define HC32L110_MOTOR_SPEED_LOW                0x22
#define HC32L110_MOTOR_SPEED_HIGH               0x23
#define HC32L110_MOTOR_TARGET_POS_LOW           0x24
#define HC32L110_MOTOR_TARGET_POS_HIGH          0x25
#define HC32L110_SYS_ERROR_HEIGHT_LOW           0x26
#define HC32L110_SYS_ERROR_HEIGHT_HIGH          0x27
#define HC32L110_SYS_ERR_HEIGHT_AMEND_LOW       0x2C
#define HC32L110_SYS_ERR_HEIGHT_AMEND_HIGH      0x2D
#define HC32L110_MOTOR_CTL                      0x20
#define HC32L110_MOTOR_CSTATE                   0x21
#define HC32L110_START_TMR_VOLT_L               0x70
#define HC32L110_CALI_CTL                       0x90
#define HC32L110_CALI_STEP_CNT_SET_LOW          0x92
#define HC32L110_CALI_STEP_CNT_SET_HIGH         0x93
#define HC32L110_CALI_SPEED_CNT_SET_LOW         0x94
#define HC32L110_CALI_SPEED_CNT_SET_HIGH        0x95
#define HC32L110_CALI_CMD                       0x91
#define HC32L110_CALI_FLASH_DATA_ADDR_L         0x9A
#define HC32L110_CALI_FLASH_DATA_ADDR_H         0x9B
#define HC32L110_CALI_FLASH_DATA_VALUE_L        0x9C
#define HC32L110_CALI_FLASH_DATA_VALUE_H        0x9D
#define HC32L110_CALI_E2PROM_DATA_ADDR_L        0x9E
#define HC32L110_CALI_E2PROM_DATA_ADDR_H        0x9F
#define HC32L110_CALI_E2PROM_DATA_VALUE_L       0xA0
#define HC32L110_CALI_E2PROM_DATA_VALUE_H       0xA1
#define HC32L110_CALI_STATE                     0xA6
#define HC32L110_CALI_CRC_CALC_VALUE_L          0xA2
#define HC32L110_CALI_CRC_CALC_VALUE_H          0xA3
#define HC32L110_CALI_CRC_REC_VALUE_L           0xA4
#define HC32L110_CALI_CRC_REC_VALUE_H           0xA5
/* I2C value */
#define START_STEPPING                          0x01
#define STOP_STEPPING                           0x00
#define HC32L110_CURR_REAL_HEIGHT_LOW           0x28
#define HC32L110_CURR_REAL_HEIGHT_HIGH          0x29
#define HC32L110_CALI_MOVE_FORWARD_STEP         0x23
#define HC32L110_CALI_MOVE_BACKWARD_STEP        0x24
#define HC32L110_CALI_GET_CURRENT_TMR           0x01
#define HC32L110_CALI_GET_CURRENT_HEIGHT        0x02
#define HC32L110_CALI_TMR_DATA_LOW              0x96
#define HC32L110_CALI_TMR_DATA_HIGH             0x97
#define HC32L110_CALI_REAL_HEIGHT_LOW           0x98
#define HC32L110_CALI_REAL_HEIGHT_HIGH          0x99
/* log reg */
#define HC32L110_STAR_LOG_STATUS                0xF0
#define HC32L110_STAR_LOG_GET_ONE_DATA          0xF1
#define HC32L110_STAR_LOG_INFO                  0xF2
#define HC32L110_STAR_LOG_ADDR                  0xF3
#define HC32L110_STAR_LOG_DAT_L                 0xF4
#define HC32L110_STAR_LOG_DAT_H                 0xF5
/* I2C debug reg */
#define HC32L110_STACKED_LR_LL                  0xD0

int hc32l110_power_on(struct cam_star_module_device_info *di);
int hc32l110_set_bootloader(struct cam_star_module_device_info *di);
int hc32l110_calib_write_flash_data(struct cam_star_module_device_info *di, u16 data);
int hc32l110_calib_read_flash(struct cam_star_module_device_info *di,
	u16 offset, u16 *read_value);
int hc32l110_mcu_fw_reg_print(struct cam_star_module_device_info *di);
int hc32l110_read_mcu_i2c_debug_data(struct cam_star_module_device_info *di);
#endif /* _HC32L110_H_ */
