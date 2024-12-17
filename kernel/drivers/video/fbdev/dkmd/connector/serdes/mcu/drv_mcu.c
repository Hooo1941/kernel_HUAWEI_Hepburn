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

#include "drv_mcu.h"
#include "../upgrade/upgrade_channel_manage.h"

int set_backlight_to_smart_panel(struct i2c_client *client, int brightness)
{
	int ret = -1;
	if (get_ota_update_status()) {
		cdc_debug("error:OTA updating, not allow set backlight to smart_panel\n");
		return ret;
	}

	if (brightness < 0)
		brightness = 0;
	else if (brightness > MAX_BACKLIGHT_VALUE)
		brightness = MAX_BACKLIGHT_VALUE;

	ret = i2c_write_reg_len2(client->adapter, SMART_MCU_I2C_7BIT_ADDR, BACKLIGHT_COMMAND, brightness);
	if (ret < 0)
		cdc_err("smartpanel set backlight to %d failed!", brightness);
	else
		cdc_info("smartpanel set backlight to %d success", brightness);
	return ret;
}

int get_backlight_from_smart_panel(struct i2c_client *client)
{
	int ret = -1;
	int brightness = 0;

	if (get_ota_update_status()) {
		cdc_debug("error:OTA updating, not allow get backlight from smart_panel\n");
		return ret;
	}

	ret = i2c_read_reg_len2(client->adapter, SMART_MCU_I2C_7BIT_ADDR, BACKLIGHT_COMMAND, &brightness);
	if (ret < 0) {
		cdc_err("smartpanel get backlight from mcu failed!");
		return ret;
	}
	cdc_info("smartpanel get backlight[%d] from mcu success", brightness);
	return brightness;
}

int set_vibration_motor_mode(int vibration_waveform, int vibration_mode)
{
	int ret = -1;
	struct i2c_adapter *adapter = NULL;
	int vibration_value = 0;

	adapter = i2c_get_adapter(0x4);
	if (get_ota_update_status()) {
		cdc_debug("error:OTA updating, not allow set vibration motor mode\n");
		return ret;
	}

	vibration_value = (vibration_waveform << LSH_BYTE_NUM) | vibration_mode;
	ret = i2c_write_reg_len2(adapter, SMART_MCU_I2C_7BIT_ADDR,
		VIBRATION_MOTOR_CONTROL_COMMAND, vibration_value);
	if (ret < 0)
		cdc_err("set vibration motor mode 0x%x failed!", vibration_value);
	else
		cdc_info("set vibration motor mode 0x%x success", vibration_value);
	return ret;
}

int get_vibration_motor_mode(void)
{
	int ret = -1;
	struct i2c_adapter *adapter = NULL;
	int vibration_value = 0;

	adapter = i2c_get_adapter(0x4);
	if (get_ota_update_status()) {
		cdc_debug("error:OTA updating, not allow get vibration motor mode\n");
		return ret;
	}

	ret = i2c_read_reg_len2(adapter, SMART_MCU_I2C_7BIT_ADDR,
		VIBRATION_MOTOR_CONTROL_COMMAND, &vibration_value);
	if (ret < 0) {
		cdc_err("get vibration motor failed!");
		return ret;
	}
	cdc_info("get vibration motor mode 0x%x success", vibration_value);
	return vibration_value;
}

int get_temperature_from_smart_panel(int *thermal)
{
	int ret = -1;
	int i = 0;
	int temp_h = 0;
	int temp_l = 0;
	char temp_vol_data[I2C_MSG_LEN_TEMPVOL] = {0};
	struct i2c_adapter *adapter = NULL;

	adapter = i2c_get_adapter(0x4);
	if (get_ota_update_status()) {
		cdc_debug("error:OTA updating, not allow get temperature\n");
		return ret;
	}

	cdc_debug("lcd_thermal_read_temp addr = 0x%x, reg = 0x%x",
		SMART_MCU_I2C_7BIT_ADDR, TEMPERATURE_AND_VOLTAGE_COMMAND);
	ret = i2c_read_addr_len1(adapter, SMART_MCU_I2C_7BIT_ADDR,
		TEMPERATURE_AND_VOLTAGE_COMMAND, temp_vol_data, I2C_MSG_LEN_TEMPVOL);
	if (ret < 0) {
		cdc_err("i2c read error lcd_thermal_read_temp addr = 0x%x, reg = 0x%x, err=0x%x",
			SMART_MCU_I2C_7BIT_ADDR, TEMPERATURE_AND_VOLTAGE_COMMAND, ret);
		*thermal = DEFAULT_TEMP_VALUE;
	} else {
		for (i = 0; i < I2C_MSG_LEN_TEMPVOL; i++)
			cdc_info("temp_vol_data[%d]=0x%x\n", i, temp_vol_data[i]);
		temp_h = temp_vol_data[LCD_TEMPH];
		temp_l = temp_vol_data[LCD_TEMPL];
		*thermal = (temp_h << LSH_BYTE_NUM) + temp_l;
	}
	return ret;
}

int get_millimeter_wave_radar_state(unsigned char *reg_value)
{
	int ret = -1;
	int val;
	struct i2c_adapter *adapter = NULL;

	adapter = i2c_get_adapter(0x4);
	if (get_ota_update_status()) {
		cdc_debug("error:OTA updating, not allow get millimeter wave radar state\n");
		return ret;
	}

	ret = i2c_read_addr_len1(adapter, SMART_MCU_I2C_7BIT_ADDR,
		MILLIMETER_WAVE_RADAR_CONFIG_COMMAND, reg_value, RADAR_CONFIG_LENGTH);
	if (ret < 0) {
		cdc_err("get millimeter wave radar state failed!");
		return ret;
	}
	val = ((reg_value[RADAR_CONFIG_LENGTH_BIT] << LSH_BYTE_NUM) | reg_value[RADAR_CONFIG_MODE_BIT]) <<
		LSH_BYTE_NUM | reg_value[RADAR_CONFIG_RESV_BIT];
	cdc_info("get millimeter wave radar state 0x%x success", val);
	return ret;
}

int set_millimeter_wave_radar_mode(unsigned char *reg_value)
{
	int ret = -1;
	int val = 0;
	unsigned char radar_buf[RADAR_CONFIG_LENGTH + 1] = {0};
	struct i2c_adapter *adapter = NULL;

	adapter = i2c_get_adapter(0x4);
	if (get_ota_update_status()) {
		cdc_debug("error:OTA updating, not allow get millimeter wave radar mode\n");
		return ret;
	}

	radar_buf[0] = MILLIMETER_WAVE_RADAR_CONFIG_COMMAND;
	for (int i = 0; i < RADAR_CONFIG_LENGTH; i++)
		radar_buf[i + 1] = reg_value[i];

	ret = i2c_write_addr_len1(adapter, SMART_MCU_I2C_7BIT_ADDR,
		MILLIMETER_WAVE_RADAR_CONFIG_COMMAND, radar_buf, RADAR_CONFIG_LENGTH);
	if (ret < 0) {
		cdc_err("set millimeter wave radar mode failed!");
		return ret;
	}
	val = (radar_buf[RADAR_CONFIG_MODE_BIT + 1] << LSH_BYTE_NUM) | radar_buf[RADAR_CONFIG_RESV_BIT + 1];
	cdc_info("set millimeter wave radar mode 0x%x success", val);
	return ret;
}

int get_millimeter_wave_radar_data1(unsigned char *reg_value)
{
	int ret = -1;
	struct i2c_adapter *adapter = NULL;

	adapter = i2c_get_adapter(0x4);
	if (get_ota_update_status()) {
		cdc_debug("error:OTA updating, not allow get millimeter wave radar data1\n");
		return ret;
	}

	ret = i2c_read_addr_len1(adapter, SMART_MCU_I2C_7BIT_ADDR,
		MILLIMETER_WAVE_RADAR_DATA1_COMMAND, reg_value, RADAR_DATA1_LENGTH);
	if (ret < 0) {
		cdc_err("get millimeter wave radar data1 failed!");
		return ret;
	}
	cdc_info("get millimeter wave radar data1 success");
	return ret;
}

int get_millimeter_wave_radar_data2(unsigned char *reg_value)
{
	int ret = -1;
	struct i2c_adapter *adapter = NULL;

	adapter = i2c_get_adapter(0x4);
	if (get_ota_update_status()) {
		cdc_debug("error:OTA updating, not allow get millimeter wave radar data2\n");
		return ret;
	}

	ret = i2c_read_addr_len1(adapter, SMART_MCU_I2C_7BIT_ADDR,
		MILLIMETER_WAVE_RADAR_DATA2_COMMAND, reg_value, RADAR_DATA2_LENGTH);
	if (ret < 0) {
		cdc_err("get millimeter wave radar data2 failed!");
		return ret;
	}
	cdc_info("get millimeter wave radar data2 success");
	return ret;
}

int get_millimeter_wave_radar_result(unsigned char *reg_value)
{
	int ret = -1;
	struct i2c_adapter *adapter = NULL;

	adapter = i2c_get_adapter(0x4);
	if (get_ota_update_status()) {
		cdc_debug("error:OTA updating, not allow get millimeter wave radar result\n");
		return ret;
	}

	ret = i2c_read_addr_len1(adapter, SMART_MCU_I2C_7BIT_ADDR,
		MILLIMETER_WAVE_RADAR_RESULT_COMMAND, reg_value, RADAR_RESULT_LENGTH);
	if (ret < 0) {
		cdc_err("get millimeter wave radar result failed!");
		return ret;
	}
	cdc_info("get millimeter wave radar result success");
	return ret;
}

int get_millimeter_wave_radar_config(unsigned char *reg_value)
{
	int ret = -1;
	struct i2c_adapter *adapter = NULL;

	adapter = i2c_get_adapter(0x4);
	const unsigned char id = (MILLIMETER_WAVE_RADAR_CONFIG << 1) | 0x1;

	if (get_ota_update_status()) {
		cdc_debug("error:OTA updating, not allow get millimeter wave radar config\n");
		return ret;
	}
	ret = i2c_write_reg(adapter, SMART_MCU_I2C_7BIT_ADDR, GENERAL_COMMAND, id);
	return_value_if_run_error(ret < 0, ret, "get millimeter wave radar config failed!");

	ret = i2c_read_addr_len1(adapter, SMART_MCU_I2C_7BIT_ADDR, GENERAL_COMMAND, reg_value, 1);
	return_value_if_run_error(ret < 0, ret, "get millimeter wave radar config failed!");

	cdc_info("get millimeter wave radar config success");
	return ret;
}
