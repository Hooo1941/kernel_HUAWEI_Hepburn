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
#include "log.h"

// for hisi media box brightness setting
#define DEFAULT_MAX_BRIGHTNESS 1000

#define DPTX_IRQ_HPD_IN 1
#define DPTX_IS_AOD_TYPE 0

static uint32_t port0_lcd_wake;
static uint32_t port1_lcd_wake;

static int ser1_config_slave_i2c_addr(struct serdes_data *serdes)
{
	int ret = -1;
	if (serdes->domain == NON_RDA_DOMAIN) {
		ret = i2c_addr_access(serdes->client->adapter, serdes->p_data->ser_addr,
			ser1_983_init_desalias, ARRAY_SIZE(ser1_983_init_desalias));
		if (ret)
			cdc_err("ser1_983_init_desalias fail ret %d", ret);
	}

	return ret;
}

static int ser2_config_slave_i2c_addr(struct serdes_data *serdes)
{
	int ret = -1;
	if (serdes->domain == NON_RDA_DOMAIN) {
		ret = i2c_addr_access(serdes->client->adapter, serdes->p_data->ser_addr,
			ser2_983_init_desalias, ARRAY_SIZE(ser2_983_init_desalias));
		if (ret)
			cdc_err("ser2_983_init_desalias fail ret %d", ret);
	}

	return ret;
}


static int get_port0_lcd_wake(struct serdes_data *serdes)
{
	int ret = -1;
	if (serdes == NULL) {
		cdc_err("serdes_data is null");
		return ret;
	}

	if (!gpio_is_valid(serdes->dp_port0->gpio_lcd_wake)) {
		cdc_err("get port0 gpio_lcd_wake failed");
		return ret;
	}

	ret = gpio_request(serdes->dp_port0->gpio_lcd_wake, "port0_lcd_wake");
	if (ret) {
		cdc_err("gpio request port0 gpio_lcd_wake fail");
		return ret;
	}

	return ret;
}

static int get_port1_lcd_wake(struct serdes_data *serdes)
{
	int ret = -1;
	if (serdes == NULL) {
		cdc_err("serdes_data is null");
		return ret;
	}

	if (!gpio_is_valid(serdes->dp_port1->gpio_lcd_wake)) {
		cdc_err("get port1 gpio_lcd_wake failed");
		return ret;
	}

	ret = gpio_request(serdes->dp_port1->gpio_lcd_wake, "port1_lcd_wake");
	if (ret) {
		cdc_err("gpio request port1 gpio_lcd_wake fail");
		return ret;
	}

	return ret;
}

static int link_set_wake_power(struct serdes_data *serdes, int index, int value)
{
	int ret = 0;
	int gpio_lcd_wake = -1;

	if (serdes->dp_port0 == NULL)
		return -1;

	if (index == 0)
		gpio_lcd_wake = serdes->dp_port0->gpio_lcd_wake;
	else
		gpio_lcd_wake = serdes->dp_port1->gpio_lcd_wake;

	cdc_info("link_set_wake_power gpio_lcd_wake = %d", gpio_lcd_wake);

	ret = gpio_direction_output(gpio_lcd_wake, value);
	if (ret != 0) {
		cdc_err("gpio_lcd_wake direction out 1 fail");
		return ret;
	}
	cdc_info("gpio_lcd_wake power up finish");

	return 0;
}

int link0_lcd_wake_power_on(struct serdes_data *serdes)
{
	return link_set_wake_power(serdes, 0, 1);
}

int link0_lcd_wake_power_off(struct serdes_data *serdes)
{
	return link_set_wake_power(serdes, 0, 0);
}

int link1_lcd_wake_power_on(struct serdes_data *serdes)
{
	return link_set_wake_power(serdes, 1, 1);
}

int link1_lcd_wake_power_off(struct serdes_data *serdes)
{
	return link_set_wake_power(serdes, 1, 0);
}

static int get_983_pdb(struct serdes_data *serdes)
{
	int ret = 0;

	cdc_info("enter");
	if (serdes == NULL) {
		cdc_err("serdes_data is null in get 983 pdb");
		return ret;
	}

	if (!gpio_is_valid(serdes->p_data->gpio_pdb)) {
		cdc_err("get gpio_pdb_983 failed");
		return ret;
	}
	ret = gpio_request(serdes->p_data->gpio_pdb, SER_PDB);
	if (ret) {
		cdc_err("gpio request gpio_pdb_983 fail");
		return ret;
	}

	return ret;
}

static int dp_983_power_on(struct serdes_data *serdes)
{
	int ret = 0;
	int gpio_pdb_983 = serdes->p_data->gpio_pdb;

	ret = gpio_direction_output(gpio_pdb_983, 0);
	return_value_if_run_error(ret != 0, ret, "gpio_pdb_983 direction out 0 fail");
	// PDB reset pulse width+ Delay between PDB pulses + PDB glitch filtering
	mdelay(MDELAY_10);
	ret = gpio_direction_output(gpio_pdb_983, 1);
	return_value_if_run_error(ret != 0, ret, "gpio_pdb_983 direction out 1 fail");
	cdc_info("gpio_pdb_983 being pulled end");

	return ret;
}

static int serdes_config_dp_vp0_smartpanel(struct serdes_data *serdes)
{
	int ret = -1;
	ret = i2c_addr_access(serdes->client->adapter, serdes->p_data->ser_addr,
		ser_983_init_smartpanel_vp0_config, ARRAY_SIZE(ser_983_init_smartpanel_vp0_config));
	if (ret)
		cdc_err("ser_983_init_smartpanel_vp0_config fail ret %d", ret);

	return ret;
}
static int serdes_config_dp_vp1_smartpanel(struct serdes_data *serdes)
{
	int ret = -1;
	ret = i2c_addr_access(serdes->client->adapter, serdes->p_data->ser_addr,
		ser_983_init_smartpanel_vp1_config, ARRAY_SIZE(ser_983_init_smartpanel_vp1_config));
	if (ret)
		cdc_err("ser_983_init_smartpanel_vp1_config fail ret %d", ret);

	return ret;
}

static int serdes_config_dp_passengerpanel(struct serdes_data *serdes)
{
	int ret = -1;
	ret = i2c_addr_access(serdes->client->adapter, serdes->p_data->ser_addr,
		ser_983_init_passengerpanel_config, ARRAY_SIZE(ser_983_init_passengerpanel_config));
	if (ret)
		cdc_err("serdes_config_dp_passengerpanel fail ret %d", ret);

	return ret;
}

int serdes_parse_dt(struct serdes_data *serdes, struct serder_dts_data *pdata)
{
	int ret = 0;
	uint32_t ser_i2c_reg = 0;
	uint32_t ser_pdb = 0;
	uint32_t link_port0[LINK_NUM] = {0};
	uint32_t link_port1[LINK_NUM] = {0};
	struct device_node *np = NULL;

	cdc_info("enter");
	ret = of_property_read_u32(serdes->client->dev.of_node, SER_I2C_ADDR, &ser_i2c_reg);
	return_value_if_run_error(ret < 0, ret, "ser_i2c_reg read error");
	pdata->ser_addr =(unsigned char)(ser_i2c_reg & 0xFF);

	ret = of_property_read_u32(serdes->client->dev.of_node, SER_DP_ID, &pdata->dp_id);
	return_value_if_run_error(ret < 0, ret, "ser_dp_id read error");

	ret = of_property_read_u32(serdes->client->dev.of_node, SER_PDB, &ser_pdb);
	return_value_if_run_error(ret < 0, ret, "gpio_pdb_983 read error");
	pdata->gpio_pdb = ser_pdb;

	ret = of_property_read_u32_array(serdes->client->dev.of_node, SERDES_LINK_PORT0, link_port0, LINK_NUM);
	return_value_if_run_error(ret < 0, ret, "ti983_dp_port0 read error");

	ret = of_property_read_u32_array(serdes->client->dev.of_node, SERDES_LINK_PORT1, link_port1, LINK_NUM);
	return_value_if_run_error(ret < 0, ret, "ti983_dp_port1 read error");

	if (link_port0[0] != LINK_EMPTY && link_port1[0] != LINK_EMPTY)
		serdes->need_cropx = true;

	if (link_port0[0] != LINK_EMPTY) {
		pdata->link_port0_id = link_port0[0];
		pdata->gpio_lcd_wake0 = link_port0[1];
		pdata->des0_addr = link_port0[2];
	} else {
		pdata->link_port0_id = LINK_EMPTY;
		pdata->gpio_lcd_wake0 = -1;
		pdata->des0_addr = 0;
	}

	if (link_port1[0] != LINK_EMPTY) {
		pdata->link_port1_id = link_port1[0];
		pdata->gpio_lcd_wake1 = link_port1[1];
		pdata->des1_addr = link_port1[2];
	} else {
		pdata->link_port1_id = LINK_EMPTY;
		pdata->gpio_lcd_wake1 = -1;
		pdata->des1_addr = 0;
	}

	cdc_info(" DT:reg=0x%x", pdata->ser_addr);
	cdc_info(" DT:gpio_pdb_983=%d", pdata->gpio_pdb);
	cdc_info(" DT:link_port0_id=%d, gpio_lcd_wake0=%d, reg=0x%x",
		pdata->link_port0_id, pdata->gpio_lcd_wake0, pdata->des0_addr);
	cdc_info(" DT:link_port1_id=%d, gpio_lcd_wake1=%d, reg=0x%x",
		pdata->link_port1_id, pdata->gpio_lcd_wake1, pdata->des1_addr);

	return 0;
}

int ti983_serdes_init(struct serdes_data *serdes, struct serder_dts_data *pdata)
{
	serdes->serdes_config_power_gpio = get_983_pdb;
	serdes->serdes_power_on = dp_983_power_on;

	if (serdes->domain == NON_RDA_DOMAIN) {
		if (pdata->link_port0_id == SMART_PANEL) {
			serdes->serdes_config_vp0 = serdes_config_dp_vp0_smartpanel;
			serdes->serdes_config_slave_addr = ser1_config_slave_i2c_addr;
		} else if (pdata->link_port0_id == PASSENGER_PANEL) {
			serdes->serdes_config_vp0 = serdes_config_dp_passengerpanel;
			serdes->serdes_config_slave_addr = ser2_config_slave_i2c_addr;
		}

		if (pdata->link_port1_id == SMART_PANEL) {
			serdes->serdes_config_vp1 = serdes_config_dp_vp1_smartpanel;
			serdes->serdes_config_slave_addr = ser1_config_slave_i2c_addr;
		}
	}

	return 0;
}

static int smartpanel_deser0_config(struct serdes_data *serdes)
{
	int ret = 0;
	ret = i2c_addr_access(serdes->client->adapter, serdes->p_data->des0_addr,
		ti988_mediabox_edp0_edp1_pannel_init, ARRAY_SIZE(ti988_mediabox_edp0_edp1_pannel_init));
	if (ret)
		cdc_err("smartpanel_deser0_config error");

	return ret;
}

static int smartpanel_deser1_config(struct serdes_data *serdes)
{
	int ret = 0;
	ret = i2c_addr_access(serdes->client->adapter,  serdes->p_data->des1_addr,
		ti988_mediabox_edp0_edp1_pannel_init, ARRAY_SIZE(ti988_mediabox_edp0_edp1_pannel_init));
	if (ret)
		cdc_err("smartpanel_deser1_config error");

	return ret;
}

static int passengerpanel_deser0_config(struct serdes_data *serdes)
{
	int ret = 0;
	ret = i2c_addr_access(serdes->client->adapter, serdes->p_data->des0_addr,
		deser_988_passenger_pannel_init_for_dp, ARRAY_SIZE(deser_988_passenger_pannel_init_for_dp));
	if (ret)
		cdc_err("passengerpanel_deser0_config error");

	return ret;
}

static int ti983_link_port_init(struct serdes_data *serdes)
{
	if (serdes->dp_port0 == NULL && serdes->dp_port1 == NULL) {
		cdc_err("(serdes->dp_port0 and serdes->dp_port1 is NULL");
		return -1;
	}

	if (serdes->dp_port0 != NULL) {
		struct ti983_dp_port *port_info = serdes->dp_port0; // port0 port1
		if (port_info->panel_id == SMART_PANEL)
			port_info->lcd_deser_config = smartpanel_deser0_config;
		else if (port_info->panel_id == PASSENGER_PANEL)
			port_info->lcd_deser_config = passengerpanel_deser0_config;
	}

	if (serdes->dp_port1 != NULL) {
		struct ti983_dp_port *port_info = serdes->dp_port1; // port0 port1
		if (port_info->panel_id == SMART_PANEL)
			port_info->lcd_deser_config = smartpanel_deser1_config;
	}

	return 0;
}

int ti983_config_link_port(struct serdes_data *serdes, struct serder_dts_data *pdata)
{
	int ret = 0;

	if (serdes == NULL || pdata == NULL) {
		cdc_err("serdes_data is null");
		return -1;
	}

	cdc_info("enter");
	if (serdes->domain == NON_RDA_DOMAIN) {
		if (pdata->link_port0_id != LINK_EMPTY) {
			serdes->dp_port0 = kzalloc(sizeof(struct ti983_dp_port), GFP_KERNEL);
			if (!serdes->dp_port0) {
				cdc_err("ti983_link_port0 kzalloc failed");
				return -ENOMEM;
			}
			serdes->dp_port0->panel_id = pdata->link_port0_id;
			serdes->dp_port0->gpio_lcd_wake = pdata->gpio_lcd_wake0;
			serdes->dp_port0->port_config_power_gpio = get_port0_lcd_wake;
			serdes->dp_port0->lcd_power_on = link0_lcd_wake_power_on;
			serdes->dp_port0->lcd_power_off = link0_lcd_wake_power_off;
		}

		if (pdata->link_port1_id != LINK_EMPTY) {
			serdes->dp_port1 = kzalloc(sizeof(struct ti983_dp_port), GFP_KERNEL);
			if (!serdes->dp_port1) {
				cdc_err("ti983_link_port1 kzalloc failed");
				return -ENOMEM;
			}
			serdes->dp_port1->panel_id = pdata->link_port1_id;
			serdes->dp_port1->gpio_lcd_wake = pdata->gpio_lcd_wake1;
			serdes->dp_port1->port_config_power_gpio = get_port1_lcd_wake;
			serdes->dp_port1->lcd_power_on = link1_lcd_wake_power_on;
			serdes->dp_port1->lcd_power_off = link1_lcd_wake_power_off;
		}

		ret = ti983_link_port_init(serdes);
		if (ret) {
			cdc_err("ti983 init failed");
			return ret;
		}
	}

	return 0;
}

static int dp_video_input_reset(int dp_id)
{
	int ret = -1;
	struct i2c_adapter *adapter = NULL;
	cdc_info("enter dp_video_input_reset");

	if (dp_id == DP_CONNECTOR_EDP0) {
		cdc_info("enter DP0 dp_video_input_reset");
		adapter = i2c_get_adapter(DP0_I2C_DEVICE_NUM);
		/* soft reset 988 */
		i2c_write_reg(adapter, DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x01, 0x01);
		i2c_write_reg(adapter, DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x01, 0x01);
		/* select page APB */
		i2c_write_reg(adapter, DS90UB983_SER1_I2C_7BIT_ADDR, 0x48, 0x01);
		ret = i2c_addr_access(adapter, DS90UB983_SER1_I2C_7BIT_ADDR,
			ser_983_video_input_reset, ARRAY_SIZE(ser_983_video_input_reset));
		if (ret)
			cdc_err("ser_983_video_input_reset fail ret %d", ret);

		cdc_info("enter ti983_video_DPRX_RESET");
		i2c_write_reg(adapter, DS90UB983_SER1_I2C_7BIT_ADDR, 0x1, 0x40);
		ret = i2c_addr_access(adapter, DS90UB983_SER1_I2C_7BIT_ADDR,
			ser_983_init_dp_config, ARRAY_SIZE(ser_983_init_dp_config));
		if (ret)
			cdc_err("ser_983_init_dp_config fail ret %d", ret);
		/* soft reset 988 */
		i2c_write_reg(adapter, DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x01, 0x01);
		i2c_write_reg(adapter, DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x01, 0x01);
	}
	if (dp_id == DP_CONNECTOR_EDP1) {
		cdc_info("enter DP1 dp_video_input_reset");
		adapter = i2c_get_adapter(DP1_I2C_DEVICE_NUM);
		/* soft reset 988 */
		i2c_write_reg(adapter, DS90UH988_I2C_7BIT_ADDR_PASSENGER_ALIAS, 0x01, 0x01);
		/* select page APB */
		i2c_write_reg(adapter, DS90UB983_SER2_I2C_7BIT_ADDR, 0x48, 0x01);
		ret = i2c_addr_access(adapter, DS90UB983_SER2_I2C_7BIT_ADDR,
			ser_983_video_input_reset, ARRAY_SIZE(ser_983_video_input_reset));
		if (ret)
			cdc_err("ser_983_video_input_reset fail ret %d", ret);

		cdc_info("enter ti983_video_DPRX_RESET");
		i2c_write_reg(adapter, DS90UB983_SER2_I2C_7BIT_ADDR, 0x1, 0x40);
		ret = i2c_addr_access(adapter, DS90UB983_SER2_I2C_7BIT_ADDR,
			ser_983_init_dp_config, ARRAY_SIZE(ser_983_init_dp_config));
		if (ret)
			cdc_err("ser_983_init_dp_config fail ret %d", ret);
		/* soft reset 988 */
		i2c_write_reg(adapter, DS90UH988_I2C_7BIT_ADDR_PASSENGER_ALIAS, 0x01, 0x01);
	}

	cdc_info("exit dp_video_input_reset");
	return ret;
}

// dp get_ext_disp_info for dptxlink
int get_dp_ext_disp_info(struct dp_ext_disp_info *info, int dp_id)
{
	int ret = -1;
	return_value_if_run_error(!info, -ENOMEM, "dp_ext_disp_info kzalloc failed");
	info->port_id = dp_id;

	if (!info->edid || !info->sinfo) {
		cdc_err("info->edid or info->sinfo is null");
		return -1;
	}

	if (dp_id == DP_CONNECTOR_EDP0) {
		// ARHUD+MAXVIEW
		info->screen_num = 1;
		info->edid_blocks = 0;

		ret = memcpy_s(info->edid, sizeof(uint8_t) * EDID_LEN, edid_for_arhud_maxview, sizeof(uint8_t) * EDID_LEN);
		if (ret != 0) {
			cdc_err("[ti983]info->edid memcpy failed");
			return ret;
		}

		info->sinfo->h_active_pixels = 1080;
		info->sinfo->v_active_pixels = 2160;
	} else if (dp_id == DP_CONNECTOR_EDP1) {
		// passanger
		info->screen_num = 1;
		info->edid_blocks = 0;

		ret = memcpy_s(info->edid, sizeof(uint8_t) * EDID_LEN, edid_for_passanger, sizeof(uint8_t) * EDID_LEN);
		if (ret != 0) {
			cdc_err("[ti983]info->edid memcpy failed");
			return ret;
		}

		info->sinfo->h_active_pixels = 1080;
		info->sinfo->v_active_pixels = 2160;
	}

	return ret;
}

void set_backlight(int port_id, uint32_t bl_level)
{
	// screen side implement set_backlight
	cdc_info("enter set_backlight, port_id:%d, bl_level:%u", port_id, bl_level);
}

int ti983_config_edid(struct serdes_data *serdes)
{
	int ret = 0;
	int max_retry = 5;

	struct dp_connect_ctrl_info dp_info;

	dp_info.port_id = serdes->p_data->dp_id;
	dp_info.edid_len = 1;
	dp_info.screen_num = 1;
	dp_info.is_dprx_ready = true;
	dp_info.get_ext_disp_info = get_dp_ext_disp_info;
	dp_info.notify_ser_dp_vp_sync = notify_ser_dp_vp_sync;
	dp_info.notify_ser_dp_unplug = notify_ser_dp_unplug;
	dp_info.set_backlight = set_backlight;
	dp_info.max_brightness = DEFAULT_MAX_BRIGHTNESS;
	dp_info.dp_connect_ctrl_reset_dprx = dp_video_input_reset;

	while (max_retry > 0) {
		ret = dpu_dptx_register_ext_disp_callback(&dp_info, DPTX_IRQ_HPD_IN, DPTX_IS_AOD_TYPE);
		if (ret < 0) {
			cdc_err("dpu_dptx_register_ext_disp_callback err! wait to retry!");
			mdelay(MDELAY_EDID);
			max_retry--;
			continue;
		}
		break;
	}
	if (ret < 0)
		cdc_err("dpu_dptx_register_ext_disp_callback err! txlink err");

	return ret;
}