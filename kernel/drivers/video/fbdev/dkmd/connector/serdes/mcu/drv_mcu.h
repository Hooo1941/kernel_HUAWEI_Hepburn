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

#ifndef _DRV_MCU_H_
#define _DRV_MCU_H_

#include "../common/i2c_common.h"
#include "../common/log.h"
extern struct i2c_client *g_ti981_client;
#define SMART_MCU_I2C_7BIT_ADDR 0x42
#define LIGHT_SENSOR_I2C_7BIT_ADDR 0x44
#define METER_MCU_I2C_ADDR_IN_NON_RDA 0x46

#define HARDWARE_ID_COMMAND 0x21
#define TEMPERATURE_AND_VOLTAGE_COMMAND 0x23
#define BACKLIGHT_COMMAND 0x24
#define LCD_GPIO_COMMAND 0x25
#define LCD_FAULT_DETAILS_COMMAND 0x26
#define VIBRATION_MOTOR_CONTROL_COMMAND 0x28
#define MILLIMETER_WAVE_RADAR_CONFIG_COMMAND 0x29
#define MILLIMETER_WAVE_RADAR_DATA1_COMMAND 0x2A
#define MILLIMETER_WAVE_RADAR_DATA2_COMMAND 0x2B
#define MILLIMETER_WAVE_RADAR_RESULT_COMMAND 0x2C
#define ELECTRONIC_LABEL_COMMAND 0x30
#define GENERAL_COMMAND 0x31
#define INTERRUPT_COMMAND 0x32
#define EQUIPMENT_COMMAND 0x36

// 0x30 Field id
#define SN_FIELD 0x02
#define PRECISE_TRACING_FIELD 0x03
#define CUSTOMER_HARDWARE_FIELD 0x04
#define CUSTOMER_SOFTWARE_FIELD 0x05
#define SCREEN_PARAMETER_FIELD 0x07

// 0x36 Field id
#define READ_WRITE_CONFIG_FIELD 0x11
#define WRITE_FLASH_FIELD 0x13
#define BURN_IN_FIELD 0x17

// 0x31 Field id
#define HUAWEI_HARDWARE_VERSION_FIELD 0x01
#define HUAWEI_SOFTWARE_VERSION_FIELD 0x05
#define MCU_IN_POSITION_FIELD 0x07
#define TEMPERATURE_SENSOR_FIELD 0x09
#define LIGHT_SENSOR_STATUS_FIELD 0x19
#define LIGHT_SENSOR_VALUE_FIELD 0x1A
#define BACK_LIGHT_FIELD 0x22
#define LCD_UPDN_FIELD 0x33
#define ADC_VOLTAGE_FIELD 0x37
#define MOTOR_EXIST_FIELD 0x40
#define MOTOR_CONTROL_FIELD 0x41
#define MOTOR_VIBRATION_SUPPORT_FIELD 0x42
#define MOTOR_VIBRATION_FIELD 0x43
#define MILLIMETER_WAVE_RADAR_CONFIG 0x46

// 0x32 Field id
#define FAULT_STATUS_FIELD 0x00

#define MAX_BACKLIGHT_VALUE 1000
#define LSH_BYTE_NUM 8
#define REG_BUF_LEN 10

#define DEFAULT_TEMP_VALUE (-5555)
#define I2C_MSG_LEN_TEMPVOL 6
#define LCD_TEMPH 2
#define LCD_TEMPL 3
#define RADAR_CONFIG_LENGTH 3
#define RADAR_DATA1_LENGTH 196
#define RADAR_DATA2_LENGTH 52
#define RADAR_RESULT_LENGTH 7

#define RADAR_CONFIG_LENGTH_BIT 0
#define RADAR_CONFIG_MODE_BIT 1
#define RADAR_CONFIG_RESV_BIT 2

int set_backlight_to_smart_panel(struct i2c_client *client, int brightness);
int get_backlight_from_smart_panel(struct i2c_client *client);
int set_vibration_motor_mode(int vibration_waveform, int vibration_mode);
int get_vibration_motor_mode(void);
int get_temperature_from_smart_panel(int *thermal);
int get_millimeter_wave_radar_state(unsigned char *reg_value);
int set_millimeter_wave_radar_mode(unsigned char *reg_value);
int get_millimeter_wave_radar_data1(unsigned char *reg_value);
int get_millimeter_wave_radar_data2(unsigned char *reg_value);
int get_millimeter_wave_radar_result(unsigned char *reg_value);
int get_millimeter_wave_radar_config(unsigned char *reg_value);

#endif
