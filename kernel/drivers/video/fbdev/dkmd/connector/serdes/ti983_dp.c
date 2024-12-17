/* Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include "serdes_common.h"

#define DP_FRAME_CNT_END 100

static unsigned int g_lvds_link_status = DP_LVDS_LINKED;
static unsigned int g_dp_resumed = DP_STATUS_ACTIVE;

static int dp_sys_init(struct serdes_data *serdes)
{
	int ret = -1;

	ret = lcd_sysfs_init_for_dp(serdes->client);
	if (ret)
		cdc_err("primary: dev_name = %s+.\n", dev_name(&serdes->client->dev));

	cdc_info("lcd_sysfs primary: dev_name = %s+.\n", dev_name(&serdes->client->dev));

	return 0;
}

static int clear_crc_errors(struct serdes_data *serdes)
{
	int ret = -1;
	unsigned char reg_value = 0;

	ret = i2c_read_reg(serdes->client->adapter, serdes->p_data->ser_addr, 0x02, &reg_value);
	if (ret)
		cdc_err("read data from 983 0x2 fail ret %d", ret);
	// CRC Error Reset
	ret = i2c_write_reg(serdes->client->adapter, serdes->p_data->ser_addr, 0x02, reg_value | 0x20);
	if (ret)
		cdc_err("write CRC Error Reset error, ret %d", ret);

	ret = i2c_read_reg(serdes->client->adapter, serdes->p_data->ser_addr, 0x02, &reg_value);
	if (ret)
		cdc_err("read data from 983 0x2 fail ret %d", ret);

	ret = i2c_write_reg(
		// CRC Error Reset Clear
		serdes->client->adapter, serdes->p_data->ser_addr, 0x02, reg_value & 0xdf);
	if (ret)
		cdc_err("write CRC Error Reset Clear error, ret %d", ret);

	ret = i2c_write_reg(serdes->client->adapter, serdes->p_data->ser_addr, 0x2d, 0x01);
	if (ret)
		cdc_err("write data to 983 0x02 0x01 fail, ret %d", ret);

	return ret;
}

static int set_deserializer_ramp_rb(struct serdes_data *serdes, int des_addr, int ts_code)
{
	int ret = 0;
	unsigned char ramp_rb = 0;
	ret = i2c_write_reg(serdes->client->adapter, des_addr, 0x41, 0xf5);
	if (ret)
		cdc_err("write data to 988 0x41 0xf5 fail when ramp_up_cap_delta > 0, ret %d", ret);

	ret = i2c_read_reg(serdes->client->adapter, des_addr, 0x42, &ramp_rb);
	if (ret)
		cdc_err("read data from 988 temp_ramp_rb fail ret %d", ret);

	ramp_rb &= 0x8f;
	ramp_rb |= (ts_code << DP_RAMP_SHIFT);
	ret = i2c_write_reg(serdes->client->adapter, des_addr, 0x42, ramp_rb);
	if (ret)
		cdc_err("write data to 988 0x42 ramp_rb %d ret %d", ramp_rb, ret);

	ret = i2c_read_reg(serdes->client->adapter, des_addr, 0x42, &ramp_rb);
	if (ret)
		cdc_err("read data from 988 temp_ramp_rb fail again ret %d", ret);

	ramp_rb &= 0xfe;
	ramp_rb |= 0x01;
	ret = i2c_write_reg(serdes->client->adapter, des_addr, 0x42, ramp_rb);
	if (ret)
		cdc_err("write data to 988 0x42 ramp_rb %d ret %d", ramp_rb, ret);

	ret = i2c_write_reg(serdes->client->adapter, des_addr, 0x01, 0x01);
	if (ret)
		cdc_err("write data to 988 0x01 0x01 ret %d", ret);
	msleep(MDELAY_40);

	return ret;
}

static int set_up_ts_efuse_code(struct serdes_data *serdes, int addr, int efuse_code)
{
	int ret = 0;
	ret = i2c_write_reg(serdes->client->adapter, addr, 0x40, 0x3c); // ADFT/MBIST
	if (ret)
		cdc_err("write data to 988 0x40 0x3c fail ret %d", ret);

	ret = i2c_write_reg(serdes->client->adapter, addr, 0x41, 0xf5);
	if (ret)
		cdc_err("write data to 988 0x41 0xf5 fail ret %d", ret);
	// Override TS_CODE Efuse Code
	ret = i2c_write_reg(serdes->client->adapter, addr, 0x42,
		(efuse_code << DP_RAMP_SHIFT) + 1);
	if (ret)
		cdc_err("Override TS_CODE Efuse Code %d", ret);

	return ret;
}

static int set_up_deserializer_ramp_optimizetion(struct serdes_data *serdes, int des_addr, int temp_final_c)
{
	int ret = 0;
	cdc_info("enter");
	int ramp_rb = 0;
	int efuse_ts_code = 2;
	int ramp_up_range_codes = (int)((150 - temp_final_c) / (190 / 11)) + 1;
	int ramp_dn_range_codes = (int)((temp_final_c - 30) / (190 / 11)) + 1;
	int ramp_up_cap_delta = ramp_up_range_codes - 4;
	int ramp_dn_cap_delta = ramp_dn_range_codes - 7;

	ret = set_up_ts_efuse_code(serdes, des_addr, efuse_ts_code);
	if (ret)
		cdc_err("set up ts efuse code ret %d", ret);

	if (ramp_up_cap_delta > 0) {
		int ts_code_up = efuse_ts_code - ramp_up_cap_delta;
		if (ts_code_up < 0)
			ts_code_up = 0;

		ret = set_deserializer_ramp_rb(serdes, des_addr, ts_code_up);
		if (ret)
			cdc_err("set_deserializer_ramp_rb fail for ts_code_up ret %d", ret);
	}

	if (ramp_dn_cap_delta > 0) {
		int ts_code_dn = efuse_ts_code + ramp_dn_cap_delta;
		if (ts_code_dn >= DP_TS_CODE_DN_MAX)
			ts_code_dn = DP_TS_CODE_DN_MAX;

		ret = set_deserializer_ramp_rb(serdes, des_addr, ts_code_dn);
		if (ret)
			cdc_err("set_deserializer_ramp_rb fail for ts_code_dn ret %d", ret);
	}

	return ret;
}

static int read_deserializer_final_temp(struct serdes_data *serdes, int des_addr, int *temp_final_c)
{
	int ret = 0;
	unsigned char temp_final = 0;
	cdc_info("enter");

	ret = i2c_write_reg(serdes->client->adapter, des_addr, 0x40, 0x6c);
	if (ret)
		cdc_err("write data to 983 0x40 0x6c fail ret %d", ret);

	ret = i2c_write_reg(serdes->client->adapter, des_addr, 0x41, 0xd);
	if (ret)
		cdc_err("write data to 983 0x41 0xd fail ret %d", ret);

	ret = i2c_write_reg(serdes->client->adapter, des_addr, 0x42, 0x0);
	if (ret)
		cdc_err("write data to 983 0x42 0x0 fail ret %d", ret);

	ret = i2c_write_reg(serdes->client->adapter, des_addr, 0x41, 0x13);
	if (ret)
		cdc_err("write data to 983 0x41 0x13 fail ret %d", ret);

	ret = i2c_read_reg(serdes->client->adapter, des_addr, 0x42, &temp_final);
	if (ret)
		cdc_err("read data from 983 temp_final fail ret %d", ret);

	*temp_final_c = DP_TEMP_DOUBLE * temp_final - DP_TEMP_FINAL;

	return ret;
}

static int get_vp_status(struct serdes_data *serdes, int reg, unsigned char *vp_status)
{
	int ret = 0;
	// Select VP Page
	ret = i2c_write_reg(serdes->client->adapter, serdes->p_data->ser_addr, 0x40, 0x31);
	if (ret)
		cdc_err("write data to 983 0x40 0x31 fail ret %d", ret);

	ret = i2c_write_reg(serdes->client->adapter, serdes->p_data->ser_addr, 0x41, reg);
	if (ret)
		cdc_err("write data to 983 0x41 reg=0x%0x fail ret %d", reg, ret);

	ret = i2c_read_reg(serdes->client->adapter, serdes->p_data->ser_addr, 0x42, vp_status);
	if (ret)
		cdc_err("read data from 983 vp0_sts fail ret %d", ret);

	return ret;
}

static int ser_check_vp_sync(struct serdes_data *serdes)
{
	int ret = -1;
	unsigned char vp0_sts = 0;
	unsigned char vp1_sts = 0;
	cdc_info("enter");

	ret = get_vp_status(serdes, 0x30, &vp0_sts);
	if (ret)
		cdc_err("read data from 983 vp0_sts fail ret %d", ret);

	ret = get_vp_status(serdes, 0x70, &vp1_sts);
	if (ret)
		cdc_err("read data from 983 vp1_sts fail ret %d", ret);

	if (vp0_sts == 0 || vp1_sts == 0) {
		ret = i2c_addr_access(serdes->client->adapter, serdes->p_data->ser_addr,
			ser_983_video_input_reset, ARRAY_SIZE(ser_983_video_input_reset));
		if (ret)
			cdc_err("ser_983_video_input_reset fail ret %d", ret);
	} else {
		cdc_info("983 vp0_sts or 983 vp1_sts is synchronized!  vp0_sts = %c ,vp1_sts = %c",
			vp0_sts, vp1_sts);
	}

	return ret;
}

static int ser_fpd_pll_setting(struct serdes_data *serdes)
{
	int ret = -1;
	ret = i2c_addr_access(serdes->client->adapter, serdes->p_data->ser_addr,
		ser_983_init_fpdIV_pll_setting, ARRAY_SIZE(ser_983_init_fpdIV_pll_setting));
	if (ret)
		cdc_err("ser_983_init_fpdIV_pll_setting fail ret %d", ret);

	ret = i2c_addr_access(serdes->client->adapter, serdes->p_data->ser_addr,
		ser_983_init_fpdIV_pll_fractional, ARRAY_SIZE(ser_983_init_fpdIV_pll_fractional));
	if (ret)
		cdc_err("ser_983_init_fpdIV_pll_fractional fail ret %d", ret);

	ret = i2c_addr_access(serdes->client->adapter, serdes->p_data->ser_addr,
		ser_983_init_fpdIV_enable_pll, ARRAY_SIZE(ser_983_init_fpdIV_enable_pll));
	if (ret)
		cdc_err("ser_983_init_fpdIV_enable_pll fail ret %d", ret);

	return ret;
}

static int ser_i2c_passthrough_setting(struct serdes_data *serdes)
{
	int ret = 0;
	unsigned char reg_value = 0;

	ret = i2c_addr_access(serdes->client->adapter, serdes->p_data->ser_addr,
		ser_983_init_fpdlink_mode, ARRAY_SIZE(ser_983_init_fpdlink_mode));
	if (ret)
		cdc_err("ser_983_init_fpdlink_mode fail ret %d", ret);

	ret = ser_fpd_pll_setting(serdes);
	if (ret)
		cdc_err("ser_fpd_pll_setting fail ret %d", ret);

	// Enable I2C Passthrough
	ret = i2c_read_reg(serdes->client->adapter, serdes->p_data->ser_addr, 0x07, &reg_value);
	if (ret)
		cdc_err("read data from 983 fail ret %d", ret);

	ret = i2c_write_reg(serdes->client->adapter, serdes->p_data->ser_addr, 0x07, (reg_value | 0x08));
	if (ret)
		cdc_err("write data to 983 0x07 fail ret %d", ret);
	msleep(40);

	return ret;
}

static int ser_vp_setting(struct serdes_data *serdes)
{
	int ret = -1;
	// Select write to port0 reg
	ret = i2c_write_reg(serdes->client->adapter, serdes->p_data->ser_addr, 0x2d, 0x01);
	if (ret)
		cdc_err("write data from 983 0x2d fail ret %d", ret);

	ret = i2c_addr_access(serdes->client->adapter, serdes->p_data->ser_addr,
		ser_983_init_dp_config, ARRAY_SIZE(ser_983_init_dp_config));
	if (ret)
		cdc_err("ser_983_init_dp_config fail ret %d", ret);

	if (serdes->p_data->link_port0_id != LINK_EMPTY)
		ret = serdes->serdes_config_vp0(serdes);

	if (serdes->p_data->link_port1_id != LINK_EMPTY)
		ret = serdes->serdes_config_vp1(serdes);

	if (serdes->p_data->link_port0_id == PASSENGER_PANEL) {
		ret = i2c_addr_access(serdes->client->adapter, serdes->p_data->ser_addr,
			ser_983_config_passenger_tx_layer, ARRAY_SIZE(ser_983_config_passenger_tx_layer));
		if (ret)
			cdc_err("ser_983_config_passenger_tx_layer fail ret %d", ret);
	} else {
		ret = i2c_addr_access(serdes->client->adapter, serdes->p_data->ser_addr,
			ser_983_config_ser_tx_layer, ARRAY_SIZE(ser_983_config_ser_tx_layer));
		if (ret)
			cdc_err("ser_983_config_ser_tx_layer fail ret %d", ret);
	}

	// Enable VPs
	// Set number of VPs used = 2
	ret = i2c_write_reg(serdes->client->adapter, serdes->p_data->ser_addr, 0x43, 0x01);
	if (ret)
		cdc_err("write data to 983 0x43 fail ret %d", ret);

	// Enable video processors
	ret = i2c_write_reg(serdes->client->adapter, serdes->p_data->ser_addr, 0x44, 0x03);
	if (ret)
		cdc_err("write data to 983 0x43 fail ret %d", ret);
	msleep(MDELAY_VP_SYNC);  // Delay for VPs to sync to DP source

	return ret;
}

static int dp_vp_sync(int32_t port_id)
{
	int ret;
	unsigned char ser_addr, reg_value;
	unsigned char deser0_addr;
	struct i2c_adapter *i2c_file = NULL;

	if (port_id == DP_CONNECTOR_EDP0) {
		i2c_file = i2c_get_adapter(0);
		ser_addr = DS90UB983_SER1_I2C_7BIT_ADDR;
		deser0_addr = DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS; // 0x2b
	} else if (port_id == DP_CONNECTOR_EDP1) {
		i2c_file = i2c_get_adapter(5);
		ser_addr = DS90UB983_SER2_I2C_7BIT_ADDR;
		deser0_addr = DS90UH988_I2C_7BIT_ADDR_PASSENGER_ALIAS;
	} else {
		cdc_err("dp port id not support!\n");
	}

	if (!i2c_file) {
		cdc_err("can not get i2c_get_adapter!\n");
		return -1;
	}

	ret = i2c_read_reg(i2c_file, ser_addr, 0x45, &reg_value);
	if (ret < 0) {
		cdc_err("read data from 983 error! reg = 0x45");
		return ret;
	}

	cdc_info("983 reg 0x45:%x!", reg_value);
	if (reg_value & 0x01) {
		i2c_write_reg(i2c_file, deser0_addr, 0x01, 0x01);
		cdc_info("983 vp0_sts have been synchronized!");
		return 0;
	}

	cdc_info("983 vp sync not success!");
	return -1;
}

int notify_ser_dp_unplug(int32_t port_id)
{
	int ret = -1;
	unsigned char ser_addr;
	unsigned char deser0_addr;
	struct i2c_adapter *i2c_file = NULL;

	cdc_info("983 ser_dp_unplug enter!");
	if (port_id == DP_CONNECTOR_EDP0) {
		i2c_file = i2c_get_adapter(0);
		ser_addr = DS90UB983_SER1_I2C_7BIT_ADDR;
	} else if (port_id == DP_CONNECTOR_EDP1) {
		i2c_file = i2c_get_adapter(5);
		ser_addr = DS90UB983_SER2_I2C_7BIT_ADDR;
	} else {
		cdc_err("dp port_id %d is not support", port_id);
		return ret;
	}

	i2c_write_reg(i2c_file, ser_addr, 0x01, 0x01);
	cdc_info("983 ser_dp_unplug success!");
	return ret;
}


int notify_ser_dp_vp_sync(int32_t port_id)
{
	int ret = -1;
	static uint32_t dp_frame_cnt[] = {0, 0, 0, 0, 0, 0};

	dp_frame_cnt[port_id]++;
	cdc_info("port_id:%d frame_cnt:%d\n", port_id, dp_frame_cnt[port_id]);
	if (dp_frame_cnt[port_id] < DP_FRAME_CNT_END) {
		ret = dp_vp_sync(port_id);
		cdc_info("port_id:%d, retval:%d\n", port_id, ret);
	}

	if (ret < 0) {
		if (dp_frame_cnt[port_id] < DP_FRAME_CNT_END) {
			cdc_info("port_id:%d frames cnt:%d, vp sync retry\n", port_id, dp_frame_cnt[port_id]);
			return -1;
		} else {
			cdc_info("port_id:%d vp sync fail, result:%d, frames cnt reached:%d\n",
				port_id, ret, dp_frame_cnt[port_id]);
			dp_frame_cnt[port_id] = 0;
			return 0;
		}
	}

	dp_frame_cnt[port_id] = 0;
	cdc_info("vp sync success\n");
	return ret;
}

static int ser_stability_setting(struct serdes_data *serdes)
{
	int ret = -1;
	int temp_final_c = 0;
	if (serdes->p_data->link_port0_id != LINK_EMPTY) {
		// Read Deserializer 0 Temp
		ret = read_deserializer_final_temp(
			serdes, serdes->p_data->des0_addr, &temp_final_c);
		if (ret)
			cdc_err("read_deserializer_final_temp for \
				DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS fail ret %d", ret);
		// Set up Deserializer 0 Temp Ramp Optimizations
		ret = set_up_deserializer_ramp_optimizetion(
			serdes, serdes->p_data->des0_addr, temp_final_c);
		if (ret)
			cdc_err("set_up_deserializer_ramp_optimizetion for \
				DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS fail ret %d", ret);
	}

	if (serdes->p_data->link_port1_id != LINK_EMPTY) {
		temp_final_c = 0; // reset temp_final_c
		// Read Deserializer 1 Temp
		ret = read_deserializer_final_temp(
			serdes, serdes->p_data->des1_addr, &temp_final_c);
		if (ret)
			cdc_err("read_deserializer_final_temp for \
				DS90UH988_I2C_7BIT_ADDR_METER_ALIAS fail ret %d", ret);

		// Set up Deserializer 1 Temp Ramp Optimizations
		ret = set_up_deserializer_ramp_optimizetion(
			serdes, serdes->p_data->des1_addr, temp_final_c);
		if (ret)
			cdc_err("set_up_deserializer_ramp_optimizetion for \
				DS90UH988_I2C_7BIT_ADDR_METER_ALIAS fail ret %d", ret);
	}
	// Clear CRC errors from initial link process
	ret = clear_crc_errors(serdes);
	if (ret)
		cdc_err("Clear CRC errors from initial link process failed ret %d", ret);

	return ret;
}

static int ser_983_init(struct serdes_data *serdes)
{
	int ret = -1;
	unsigned char lock_sts_deser0 = 0;
	unsigned char reg_value = 0;
	cdc_info("enter");

	if (serdes->serdes_config_slave_addr) {
		ret = serdes->serdes_config_slave_addr(serdes);
		return_value_if_run_error(ret < 0, ret, "ser_983_init_desAlias fail ret %d", ret);
	}

	ret = ser_i2c_passthrough_setting(serdes);
	if (ret)
		cdc_err("ser_i2c_passthrough_setting fail ret %d", ret);

	ret = ser_vp_setting(serdes);
	if (ret)
		cdc_err("ser vp setting ret %d", ret);
	// judge ti988
	if (serdes->p_data->link_port0_id != LINK_EMPTY){
		ret = i2c_read_reg(serdes->client->adapter, serdes->p_data->des0_addr,
			DSER_GENERAL_PURPOSE_STATUS_1, &lock_sts_deser0);
		if (ret < 0) {
			cdc_err("988 not position");
			return ret;
		}
	}

	// config edid
	ret = ti983_config_edid(serdes);
	if (ret < 0)
		cdc_err("ti983_config_edid failed, dp_txlink status is err");

	ret = ser_check_vp_sync(serdes);
	if (ret)
		cdc_err("write data to 983 0x43 fail ret %d", ret);

	ret = ser_stability_setting(serdes);
	if (ret)
		cdc_err("ser_stability_setting fail ret %d", ret);
	cdc_info("end");

	return ret;
}

static int ser_init_setting(struct serdes_data *serdes)
{
	int ret = 0;

	if (serdes->serdes_power_on) {
		ret = serdes->serdes_power_on(serdes);
		return_value_if_run_error(ret < 0, ret, "pull 983 pdb error");
	}

	if (serdes->dp_port0 && serdes->dp_port0->lcd_power_on) {
		ret = serdes->dp_port0->lcd_power_on(serdes);
		return_value_if_run_error(ret < 0, ret, "pull port0 lcd power on error");
	}

	if (serdes->dp_port1 && serdes->dp_port1->lcd_power_on) {
		ret = serdes->dp_port1->lcd_power_on(serdes);
		return_value_if_run_error(ret < 0, ret, "pull port1 lcd power on error");
	}

	mdelay(MDELAY_10); // PDB to I2C reday delay
	/* config 983 register */
	ret = ser_983_init(serdes);
	if (ret) {
		cdc_err("ser_983_init error");
		return ret;
	}

	return ret;
}

static int deser_init_setting(struct serdes_data *serdes)
{
	int ret = 0;
	int try_cnt = 0;
	unsigned char lock_sts_deser0 = 0;
	unsigned char lock_sts_deser1 = 0;

	/* config 988 register */
	while (1) {
		try_cnt++;
		if (try_cnt > DESER_MAX_TRY_TIMES) {
			cdc_err("988 lock err when try_cnt is %d, ret:%d\n", try_cnt, ret);
			return -1;
		}
		if (serdes->p_data->link_port0_id != LINK_EMPTY) {
			ret = i2c_read_reg(serdes->client->adapter, serdes->p_data->des0_addr,
				DSER_GENERAL_PURPOSE_STATUS_1, &lock_sts_deser0);
			if (ret < 0) {
				msleep(MDELAY_40);
				continue;
			}
		} else {
			lock_sts_deser0 = 1;
		}

		if (serdes->p_data->link_port1_id != LINK_EMPTY) {
			ret = i2c_read_reg(serdes->client->adapter, serdes->p_data->des1_addr,
				DSER_GENERAL_PURPOSE_STATUS_1, &lock_sts_deser1);
			if (ret < 0) {
				msleep(MDELAY_40);
				continue;
			}
		} else {
			lock_sts_deser1 = 1;
		}

		if (lock_sts_deser0 & lock_sts_deser1 & 0x1) {
			mdelay(2); // delay 2ms for 988 config
			if (serdes->dp_port0 && serdes->dp_port0->lcd_deser_config) {
				ret = serdes->dp_port0->lcd_deser_config(serdes);
				return_value_if_run_error(ret < 0, ret, "dp_port0_988_init failed");
			}

			if (serdes->dp_port1 && serdes->dp_port1->lcd_deser_config) {
				ret = serdes->dp_port1->lcd_deser_config(serdes);
				return_value_if_run_error(ret < 0, ret, "dp_port1_988_init failed");
			}
			// if in hot_reinit and status is suspend or resume
			break;
		}
		// delay 40ms if any deser is unlocked status
		msleep(MDELAY_40);
	}

	return 0;
}

static int dp_ti983_988_reset(struct serdes_data *serdes)
{
	int ret = 0;
	unsigned char des_addr = 0;

	cdc_info("enter");
	ret = ser_init_setting(serdes);
	if (ret) {
		cdc_err("ser_init_setting error! ret val is %d", ret);
		return ret;
	}

	ret = i2c_read_reg(serdes->client->adapter, serdes->p_data->ser_addr, TI983_DES_ID, &des_addr);
	if (ret)
		cdc_err("read 983 des id failed des_addr is 0x%x", des_addr);

	cdc_info("read 983 des id des_addr is 0x%x", des_addr);

	ret = deser_init_setting(serdes);
	if (ret) {
		cdc_err("deser_init_setting error! ret = %d", ret);
		return ret;
	}
	cdc_info("deserializer init success");

	return 0;
}

static int dp_serdes_init_setting(struct serdes_data *serdes)
{
	int ret = -1;
	if (serdes == NULL || serdes->client == NULL) {
		cdc_err("serdes or serdes->client is null");
		return ret;
	}
	cdc_info("enter");

	ret = ser_init_setting(serdes);
	if (ret) {
		cdc_err("ser_init_befor is %d", ret);
		return ret;
	}
	cdc_info("serialize init success");

	ret = deser_init_setting(serdes);
	if (ret) {
		cdc_err("deser_init_setting error! ret = %d", ret);
		return ret;
	}
	cdc_info("deserializer init success");

	return 0;
}

static int ti983_serdes_enable(struct serdes_data *serdes)
{
	int ret = 0;

	cdc_info("enter");
	ret = ser_init_setting(serdes);
	if (ret) {
		cdc_err("ser_init_befor is %d", ret);
		return ret;
	}
	cdc_info("serialize init success");

	ret = deser_init_setting(serdes);
	if (ret) {
		cdc_err("deser_init_setting error! ret = %d", ret);
		return ret;
	}
	cdc_info("deserializer init success");

	return 0;
}

static int serdes_config_gpio_request(struct serdes_data *serdes, struct serder_dts_data *pdata)
{
	int ret = -1;

	if (serdes->serdes_config_power_gpio) {
		ret = serdes->serdes_config_power_gpio(serdes);
		return_value_if_run_error(ret < 0, ret, "serdes_config_power_gpio failed");
	}

	if (serdes->dp_port0 && serdes->dp_port0->port_config_power_gpio && serdes->dp_port0->panel_id != ARHUD_PANEL) {
		ret = serdes->dp_port0->port_config_power_gpio(serdes);
		return_value_if_run_error(ret < 0, ret, "port0_config_power_gpio failed");
	}

	if (serdes->dp_port1 && serdes->dp_port1->port_config_power_gpio) {
		ret = serdes->dp_port1->port_config_power_gpio(serdes);
		return_value_if_run_error(ret < 0, ret, "port1_config_power_gpio failed");
	}

	return ret;
}

static int ti983_pm_suspend(struct device *dev)
{
	int ret = 0;
	struct serdes_data *serdes = NULL;
	g_lvds_link_status = DP_LVDS_CAB_NOT_LINKED;
	g_dp_resumed = DP_STATUS_SUSPEND;

	cdc_info("enter");
	serdes = i2c_get_clientdata(to_i2c_client(dev));
	ret = gpio_direction_output(serdes->p_data->gpio_pdb, 0);
	return_value_if_run_error(ret != 0, ret, "gpio_pdb_983 direction out 0 fail");

	if (serdes->dp_port0) {
		ret = gpio_direction_output(serdes->dp_port0->gpio_lcd_wake, 0);
		gpio_free(serdes->dp_port0->gpio_lcd_wake);
	}

	if (serdes->dp_port1) {
		ret = gpio_direction_output(serdes->dp_port1->gpio_lcd_wake, 0);
		gpio_free(serdes->dp_port1->gpio_lcd_wake);
	}
	gpio_free(serdes->p_data->gpio_pdb);

	return ret;
}

static void serdes_resume(struct work_struct *work)
{
	int ret = -1;
	struct serdes_data *serdes = NULL;

	cdc_info("enter serdes_resume work");
	serdes = container_of(work, struct serdes_data, work_serdes_resume);
	if (serdes == NULL) {
		cdc_err("resume struct serdes check failed!");
		return;
	}
	serdes->lvds_link_status = DP_LVDS_CAB_NOT_LINKED;
	serdes->dp_resumed = DP_STATUS_RESUME;
	ret = serdes_config_gpio_request(serdes, serdes->p_data);
	if (ret < 0) {
		cdc_err("serdes_config_gpio_request failed");
		return;
	}
	ret = dp_serdes_init_setting(serdes);
	if (ret < 0) {
		cdc_err("panel init failed, link status is err");
		serdes->lvds_link_status = DP_LVDS_NOT_LINKED;
		return;
	}
	serdes->lvds_link_status = DP_LVDS_LINKED;
	serdes->dp_resumed = DP_STATUS_ACTIVE;
	return;
}

static void init_work_queue(struct serdes_data *serdes)
{
	serdes->serdes_wq = create_singlethread_workqueue("serdes_983_DP");
	INIT_WORK(&serdes->work_serdes_resume, serdes_resume);
}

static int ti983_pm_resume(struct device *dev)
{
	int ret = -1;
	cdc_info("enter");
	struct serdes_data *serdes = i2c_get_clientdata(to_i2c_client(dev));
	if (serdes == NULL) {
		cdc_err("can not find serdes_data!");
		return ret;
	}
	queue_work(serdes->serdes_wq, &serdes->work_serdes_resume);

	return 0;
}

static int dp_serdes_config_gpio(struct serdes_data *serdes, struct serder_dts_data *pdata)
{
	int ret = 0;

	if (serdes->serdes_config_power_gpio) {
		ret = serdes->serdes_config_power_gpio(serdes);
		return_value_if_run_error(ret < 0, ret, "serdes_config_power_gpio failed");
	}

	if (serdes->dp_port0 && serdes->dp_port0->port_config_power_gpio) {
		ret = serdes->dp_port0->port_config_power_gpio(serdes);
		return_value_if_run_error(ret < 0, ret, "port0_config_power_gpio failed");
	}

	if (serdes->dp_port1 && serdes->dp_port1->port_config_power_gpio) {
		ret = serdes->dp_port1->port_config_power_gpio(serdes);
		return_value_if_run_error(ret < 0, ret, "port1_config_power_gpio failed");
	}

	return ret;
}

static int ti983_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int ret = -1;
	struct serdes_data *serdes = NULL;
	struct serder_dts_data *pdata = NULL;

	cdc_info("enter ti983_i2c_probe");
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C))
		cdc_err("get i2c adapter fail");

	serdes = devm_kzalloc(&client->dev, sizeof(struct serdes_data), GFP_KERNEL);
	return_value_if_run_error(!serdes, -ENOMEM, "ti983_serdes kzalloc failed");

	pdata = devm_kzalloc(&client->dev, sizeof(struct serder_dts_data), GFP_KERNEL);
	return_value_if_run_error(!serdes, -ENOMEM, "serder_dts_data kzalloc failed");

	serdes->client = client;
	serdes->domain = NON_RDA_DOMAIN;
	serdes->p_data = pdata;
	i2c_set_clientdata(client, serdes);

	ret = serdes_parse_dt(serdes, pdata);
	return_value_if_run_error(ret, -ENOMEM, "pdata is NULL for DT");

	ret = ti983_serdes_init(serdes, pdata);
	return_value_if_run_error(ret < 0, ret, "ti983_serdes_init failed");

	ret = ti983_config_link_port(serdes, pdata);
	return_value_if_run_error(ret < 0, ret, "ti983_config_link_port failed");

	ret = dp_sys_init(serdes);
	if (ret)
		cdc_err("dp0_sys_init failed");

	cdc_info("primary: dev_name = %s", dev_name(&client->dev));
	ret = dp_serdes_config_gpio(serdes, pdata);
	return_value_if_run_error(ret < 0, ret, "dp_serdes_config_gpio failed");

	ret = ti983_serdes_enable(serdes);
	if (ret < 0) {
		cdc_err("panel init failed, link status is err");
		g_lvds_link_status = DP_LVDS_NOT_LINKED;
	}
	init_work_queue(serdes);

	return 0;
}

static const struct i2c_device_id ti983_id[] = {{TI983_I2C_NAME, 0}, {}};

static const struct dev_pm_ops ti983_pm_ops = {SET_SYSTEM_SLEEP_PM_OPS(ti983_pm_suspend, ti983_pm_resume)};

static const struct of_device_id ti983_match_table[] = {
	{
		.compatible = DTS_COMP_DP_PANEL,
		.data = NULL,
	},
	{},
};

static struct i2c_driver ti983_i2c_driver = {
	.probe = ti983_i2c_probe,
	.remove = NULL,
	.id_table = ti983_id,
	.driver =
		{
			.name = TI983_I2C_NAME,
			.owner = THIS_MODULE,
			.of_match_table = ti983_match_table,
			.pm = &ti983_pm_ops,
		},
};

static int __init ti983_dp0_init(void)
{
	int ret = 0;
	cdc_info("enter dp_983_init");
	ret = i2c_add_driver(&ti983_i2c_driver);
	if (ret) {
		cdc_err("dp0 i2c_add_driver fail, error=%d\n", ret);
		return ret;
	}
	return ret;
}

late_initcall(ti983_dp0_init);