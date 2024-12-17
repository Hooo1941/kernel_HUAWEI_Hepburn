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

#include "ti981_common.h"
#include "dkmd_mipi_panel_info.h"

static int ti981_config_slave_i2c_addr(struct ti981_serdes *serdes)
{
	int ret = -1;
	if (serdes->domain == RDA_DOMAIN)
		ret = i2c_access(serdes->client->adapter, ser_981_init_part1, ARRAY_SIZE(ser_981_init_part1));
	else if (serdes->domain == NON_RDA_DOMAIN)
		ret = i2c_access(serdes->client->adapter, ser_981_init_part1, ARRAY_SIZE(ser_981_init_part1));
	if (ret) {
		cdc_err("serdes 981 slave addr config fail ret %d", ret);
		return ret;
	}
	return ret;
}

static int ti981_check_des_devid(struct ti981_serdes *serdes)
{
	int ret = 0;
	unsigned char des_addr = 0;

	if (serdes->domain == RDA_DOMAIN) {
		ret = i2c_read_reg(serdes->client->adapter, DS90UB981_I2C_7BIT_ADDR, TI981_DES_ID, &des_addr);
		if (ret < 0) {
			cdc_err("read 981 data error!");
			return ret;
		}
	} else {
		unsigned char reg_value = 0;
		ret = i2c_write_reg(serdes->client->adapter, DS90UB981_I2C_7BIT_ADDR, 0x2d, 0x01);
		if (ret < 0)
			cdc_err("write data to 981 error! reg = 0x2d");
		ret = i2c_read_reg(serdes->client->adapter, DS90UB981_I2C_7BIT_ADDR, 0x07, &reg_value);
		if (ret < 0)
			cdc_err("read data to 981 error! reg = 0x%02x", TI981_DES_ID);
		ret = i2c_write_reg(serdes->client->adapter, DS90UB981_I2C_7BIT_ADDR, 0x07, reg_value | 0x08);
		if (ret < 0)
			cdc_err("read data to 981 error! reg = 0x%02x", TI981_DES_ID);
		ret = i2c_read_reg(serdes->client->adapter, DS90UB981_I2C_7BIT_ADDR, TI981_DES_ID, &des_addr);
		if (ret < 0)
			cdc_err("read data to 981 error! reg = 0x%02x", TI981_DES_ID);
		return ret;
	}
	return ret;
}

// rda <--> dsi1 <--> ti981 link port0 <--> default meter
// non_rda <--> dsi0 <--> ti981 link port1 <--> default smartpanel
static int dsi1_lcd_power_info_config(struct ti981_serdes *serdes)
{
	struct device_node *np = NULL;
	int ret = -1;

	cdc_info("enter");

	np = of_find_compatible_node(NULL, NULL, DTS_COMP_DSI1_METER_PANEL);
	if (!np) {
		cdc_err("NOT FOUND device node :%s!", DTS_COMP_DSI1_METER_PANEL);
		return ret;
	}

	serdes->gpio_pdb = of_get_named_gpio(np, "gpios", 0);
	if (!gpio_is_valid(serdes->gpio_pdb)) {
		cdc_err("get gpio_pdb_981 failed");
		return ret;
	}

	serdes->port->gpio_lcd_wake = of_get_named_gpio(np, "gpios", 1);
	if (!gpio_is_valid(serdes->port->gpio_lcd_wake)) {
		cdc_err("get gpio_lcd_wake failed");
		return ret;
	}

	ret = gpio_request(serdes->gpio_pdb, "gpio_pdb_981");
	if (ret) {
		cdc_err("gpio request gpio_pdb_981 fail");
		return ret;
	}

	ret = gpio_request(serdes->port->gpio_lcd_wake, "gpio_lcd_wake");
	if (ret) {
		cdc_err("gpio request gpio_lcd_wake fail");
		return ret;
	}

	return ret;
}

static int dsi0_lcd_power_info_config(struct ti981_serdes *serdes)
{
	struct device_node *np = NULL;
	int ret = 0;

	cdc_info("enter");
	np = of_find_compatible_node(NULL, NULL, DTS_COMP_DSI0_SMART_PANEL);
	if (!np)
		cdc_err("NOT FOUND device node :%s!", DTS_COMP_DSI0_SMART_PANEL);

	serdes->port->gpio_lcd_wake = of_get_named_gpio(np, "gpios", 1);
	if (!gpio_is_valid(serdes->port->gpio_lcd_wake)) {
		cdc_err("get gpio_lcd_wake failed");
		return ret;
	}

	ret = gpio_request(serdes->port->gpio_lcd_wake, "gpio_lcd_wake");
	if (ret) {
		cdc_err("gpio request gpio_lcd_wake fail");
		return ret;
	}

	return ret;
}

int ti981_power_on(struct ti981_serdes *serdes)
{
	int ret = -1;
	int gpio_pdb_981 = 0;

	gpio_pdb_981 = serdes->gpio_pdb;
	cdc_info("gpio_pdb_981 = %d", gpio_pdb_981);
	ret = gpio_direction_output(gpio_pdb_981, 0);
	if (ret != 0) {
		cdc_err("gpio_pdb_981 direction out 0 fail");
		return ret;
	}

	mdelay(MDELAY_10);
	ret = gpio_direction_output(gpio_pdb_981, 1);
	if (ret != 0) {
		cdc_err("gpio_pdb_981 direction out 1 fail");
		return ret;
	}

	cdc_info("gpio_pdb_981 being pulled end");
	mdelay(MDELAY_10);
	return ret;
}

static int get_dsi_config(struct ti981_serdes *serdes)
{
	int ret = -1;
	uint32_t value[DSI_NUM] = {0};
	serdes->dsi_config = kzalloc(sizeof(struct ti981_dsi_config), GFP_KERNEL);
	if (!serdes->dsi_config) {
		cdc_err("serdes->dsi_config kzalloc failed");
		return -ENOMEM;
	}

	ret = of_property_read_u32_array(serdes->client->dev.of_node, DSI_LANE_NUM, value, DSI_NUM);
	if (ret != 0) {
		cdc_err("get dsi_lane_num failed");
		kfree(serdes->dsi_config);
		serdes->dsi_config = NULL;
		return ret;
	}
	serdes->dsi_config->dsi0_lane_num = value[0];
	serdes->dsi_config->dsi1_lane_num = value[1];
	cdc_info("dsi0_lane[%d] dsi1_lane[%d]", serdes->dsi_config->dsi0_lane_num, serdes->dsi_config->dsi1_lane_num);
	ret = of_property_read_u32_array(serdes->client->dev.of_node, DSI_TSKIP, value, DSI_NUM);
	if (ret != 0) {
		cdc_err("get dsi_tskip failed");
		kfree(serdes->dsi_config);
		serdes->dsi_config = NULL;
		return ret;
	}

	serdes->dsi_config->dsi0_tskip = value[0];
	serdes->dsi_config->dsi1_tskip = value[1];
	cdc_info("dsi0_tskip[0x%x] dsi1_tskip[0x%x]", serdes->dsi_config->dsi0_tskip, serdes->dsi_config->dsi1_tskip);
	return 0;
}

int ti981_serdes_init(struct ti981_serdes *serdes)
{
	int ret = 0;
	uint32_t ti981_link_port0 = 0;
	uint32_t ti981_link_port1 = 0;

	// ti981 link port0 <--> default meter，
	// ti981 link port1 <--> default smartpanel
	ret = of_property_read_u32(serdes->client->dev.of_node, TI981_LINK_PORT0, &ti981_link_port0);
	if (ret) {
		cdc_err("ti981_link_port0 read error");
		return ret;
	}
	ret = of_property_read_u32(serdes->client->dev.of_node, TI981_LINK_PORT1, &ti981_link_port1);
	if (ret) {
		cdc_err("ti981_link_port1 read error");
		return ret;
	}

	if (serdes->domain == RDA_DOMAIN) {
		ret = get_dsi_config(serdes);
		if (ret < 0) {
			cdc_err("get_dsi_config error");
			return ret;
		}
	}

	serdes->serdes_config_slave_addr = ti981_config_slave_i2c_addr;
	serdes->serdes_check_des_devid = ti981_check_des_devid;

	if (serdes->domain == RDA_DOMAIN) {
		serdes->serdes_config_power_gpio = dsi1_lcd_power_info_config;
		serdes->serdes_power_on = ti981_power_on;
	} else {
		serdes->serdes_config_power_gpio = dsi0_lcd_power_info_config;
	}

	if (ti981_link_port0 != LINK_EMPTY)
		serdes->link_port0_panel_id = ti981_link_port0;
	else
		serdes->link_port0_panel_id = LINK_EMPTY;

	if (ti981_link_port1 != LINK_EMPTY)
		serdes->link_port1_panel_id = ti981_link_port1;
	else
		serdes->link_port1_panel_id = LINK_EMPTY;

	serdes->ivi_last_backlight = 0;
	serdes->mipi_on_off = 0;
	serdes->monitor_thread = NULL;
	serdes->lvds_link_status = MIPI_LVDS_LINKED;
	serdes->mipi_resumed = MIPI_STATUS_ACTIVE;
	return ret;
}

static int ti981_config_dsi_lanes(struct ti981_serdes *serdes, bool need_set_4lane)
{
	int ret = 0;
	unsigned char reg_value = 0;
	ret = i2c_read_reg(serdes->client->adapter, DS90UB981_I2C_7BIT_ADDR, 0x4f, &reg_value);
	if (ret < 0)
		cdc_err("read data from 981 fail ret %d", ret);
	if (need_set_4lane)
		ret = i2c_write_reg(serdes->client->adapter, DS90UB981_I2C_7BIT_ADDR, 0x4f, ((reg_value & 0x73) | 0x8c));
	else
		ret = i2c_write_reg(serdes->client->adapter, DS90UB981_I2C_7BIT_ADDR, 0x4f, ((reg_value & 0x73) | 0x84));

	if (ret < 0)
		cdc_err("write data to 981 0x4f fail ret %d", ret);
	return 0;
}
static int serdes_config_dsi0_meter(struct ti981_serdes *serdes)
{
	int ret = 0;

	ret = i2c_access(serdes->client->adapter, ser_981_meterpanel_dsi0_config,
		ARRAY_SIZE(ser_981_meterpanel_dsi0_config));
	if (ret)
		cdc_err("ser_981_meterpanel_dsi0_config fail ret %d", ret);
	if (serdes->dsi_config->dsi0_lane_num == DSI_4_LANES)
		ret = ti981_config_dsi_lanes(serdes, true);
	else
		ret = ti981_config_dsi_lanes(serdes, false);
	return 0;
}
static int serdes_config_dsi0_smartpanel(struct ti981_serdes *serdes)
{
	int ret = 0;

	ret = i2c_access(serdes->client->adapter, ser_981_smartpanel_dsi0_config,
		ARRAY_SIZE(ser_981_smartpanel_dsi0_config));
	if (ret)
		cdc_err("ser_981_smartpanel_dsi0_config fail ret %d", ret);
	if (serdes->dsi_config->dsi0_lane_num == DSI_4_LANES)
		ret = ti981_config_dsi_lanes(serdes, true);
	else
		ret = ti981_config_dsi_lanes(serdes, false);
	return 0;
}

static int serdes_config_dsi1_meter(struct ti981_serdes *serdes)
{
	int ret = 0;

	ret = i2c_access(serdes->client->adapter, ser_981_meterpanel_dsi1_config,
		ARRAY_SIZE(ser_981_meterpanel_dsi1_config));
	if (ret)
		cdc_err("ser_981_meterpanel_dsi1_config fail ret %d", ret);
	if (serdes->dsi_config->dsi1_lane_num == DSI_4_LANES)
		ret = ti981_config_dsi_lanes(serdes, true);
	else
		ret = ti981_config_dsi_lanes(serdes, false);
	return 0;
}

int serdes_config_dsi1_smartpanel(struct ti981_serdes *serdes)
{
	int ret = 0;
	ret = i2c_access(serdes->client->adapter, ser_981_smartpanel_dsi1_config,
		ARRAY_SIZE(ser_981_smartpanel_dsi1_config));
	if (ret)
		cdc_err("ser_981_smartpanel_dsi1_config fail ret %d", ret);
	if (serdes->dsi_config->dsi1_lane_num == DSI_4_LANES)
		ret = ti981_config_dsi_lanes(serdes, true);
	else
		ret = ti981_config_dsi_lanes(serdes, false);
	return 0;
}
int serdes_config_dsi0_passengerpanel(struct ti981_serdes *serdes)
{
	int ret = 0;
	ret = i2c_access(serdes->client->adapter, ser_981_passenger_config,
		ARRAY_SIZE(ser_981_passenger_config));
	if (ret)
		cdc_err("ser_981_passenger_config fail ret %d", ret);
	if (serdes->dsi_config->dsi0_lane_num == DSI_4_LANES)
		ret = ti981_config_dsi_lanes(serdes, true);
	else
		ret = ti981_config_dsi_lanes(serdes, false);
	return 0;
}
int serdes_config_dsi1_icpic_panel(struct ti981_serdes *serdes)
{
	int ret = 0;

	ret = i2c_access(serdes->client->adapter, ser_981_icpic_dsi1_config,
		ARRAY_SIZE(ser_981_icpic_dsi1_config));
	if (ret)
		cdc_err("serdes_config_dsi1_icpic_panel fail ret %d", ret);
	if (serdes->dsi_config->dsi1_lane_num == DSI_4_LANES)
		ret = ti981_config_dsi_lanes(serdes, true);
	else
		ret = ti981_config_dsi_lanes(serdes, false);
	return 0;
}

// rda <--> dsi1 <--> ti981 link port0 <--> default meter
// non_rda <--> dsi0 <--> ti981 link port1 <--> default smartpanel
int ti981_config_dsi(struct ti981_serdes *serdes)
{
	if (serdes->domain == RDA_DOMAIN) {
		if (serdes->link_port1_panel_id == METER_PANEL)
			serdes->serdes_config_dsi0 = serdes_config_dsi0_meter;
		else if (serdes->link_port1_panel_id == PASSENGER_PANEL)
			serdes->serdes_config_dsi0 = serdes_config_dsi0_passengerpanel;
		else
			serdes->serdes_config_dsi0 = serdes_config_dsi0_smartpanel;

		if (serdes->link_port0_panel_id == METER_PANEL)
			serdes->serdes_config_dsi1 = serdes_config_dsi1_meter;
		else if (serdes->link_port0_panel_id == SMART_PANEL)
			serdes->serdes_config_dsi1 = serdes_config_dsi1_smartpanel;
		else if (serdes->link_port0_panel_id == ICPIC_PANEL)
			serdes->serdes_config_dsi1 = serdes_config_dsi1_icpic_panel;
	}
	return 0;
}

static int meter_lock_check(struct ti981_serdes *serdes)
{
	unsigned char lock_sts = 0;
	int try_cnt = 0;
	while (1) {
		try_cnt++;
		if (serdes->domain == RDA_DOMAIN)
			i2c_read_reg(serdes->client->adapter, DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x54, &lock_sts);
		else
			i2c_read_reg(serdes->client->adapter, DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x54, &lock_sts);

		if (!(lock_sts & 0x1)) {
			msleep(50);
		} else {
			cdc_err("988 lock succ");
			break;
		}
		if (try_cnt > DESER_MAX_TRY_TIMES) {
			cdc_err("988 lock err");
			return -1;
		}
	}
	return 0;
}

static int meter_deser_config(struct ti981_serdes *serdes)
{
	int ret = 0;
	if (serdes->domain == RDA_DOMAIN)
		ret = i2c_addr_access(serdes->client->adapter, DS90UH988_I2C_7BIT_ADDR_METER_ALIAS,
			ti988_meter_pannel_init, ARRAY_SIZE(ti988_meter_pannel_init));
	else
		ret = i2c_addr_access(serdes->client->adapter, DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS,
			ti988_meter_pannel_init, ARRAY_SIZE(ti988_meter_pannel_init));

	if (ret)
		cdc_err("meter_deser_config error");
	return ret;
}

static int meter_power_resume(struct ti981_serdes *serdes)
{
	return 0;
}

/*
 * rda <--> dsi1 <--> ti981 link port0 <--> default meter
 * non_rda <--> dsi0 <--> ti981 link port1 <--> default smartpanel
 */
static int samrtpanel_lock_check(struct ti981_serdes *serdes)
{
	unsigned char lock_sts = 0;
	int try_cnt = 0;
	while (1) {
		try_cnt++;
		if (serdes->domain == NON_RDA_DOMAIN)
			i2c_read_reg(serdes->client->adapter, DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x54, &lock_sts);
		else
			i2c_read_reg(serdes->client->adapter, DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, 0x54, &lock_sts);

		if (!(lock_sts & 0x1)) {
			msleep(50);
		} else {
			cdc_err("988 lock succ");
			break;
		}
		if (try_cnt > DESER_MAX_TRY_TIMES) {
			cdc_err("988 lock err,\n");
			return -1;
		}
	}
	return 0;
}

static int smartpanel_deser_config(struct ti981_serdes *serdes)
{
	int ret = 0;
	if (serdes->domain == NON_RDA_DOMAIN)
		ret = i2c_addr_access(serdes->client->adapter, DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS,
			ti988_smart_pannel_init, ARRAY_SIZE(ti988_smart_pannel_init));
	else
		ret = i2c_addr_access(serdes->client->adapter, DS90UH988_I2C_7BIT_ADDR_METER_ALIAS,
			ti988_smart_pannel_init, ARRAY_SIZE(ti988_smart_pannel_init));
	if (ret)
		cdc_err("meter_deser_config error");
	return ret;
}

static int passenger_deser_config(struct ti981_serdes *serdes)
{
	int ret = -1;
	ret = i2c_access(serdes->client->adapter, deser_988_passenger_pannel_init,
		ARRAY_SIZE(deser_988_passenger_pannel_init));
	return ret;
}

static int icpic_deser_config(struct ti981_serdes *serdes)
{
	int ret = -1;
	ret = i2c_access(serdes->client->adapter, deser_988_icpic_pannel_init,
		ARRAY_SIZE(deser_988_icpic_pannel_init));
	return ret;
}

static int smartpanel_power_resume(struct ti981_serdes *serdes)
{
	return 0;
}

static int set_wake_power(struct ti981_serdes *serdes, int value)
{
	int ret = -1;
	int gpio_lcd_wake = serdes->port->gpio_lcd_wake;

	cdc_info("gpio_lcd_wake = %d", gpio_lcd_wake);

	ret = gpio_direction_output(gpio_lcd_wake, value);
	if (ret != 0) {
		cdc_err("gpio_lcd_wake direction out 1 fail");
		return ret;
	}

	cdc_info("gpio_lcd_wake power up finish");
	return ret;
}

int lcd_wake_power_on(struct ti981_serdes *serdes)
{
	return set_wake_power(serdes, 1);
}

int lcd_wake_power_off(struct ti981_serdes *serdes)
{
	return set_wake_power(serdes, 0);
}

static int ti981_link_port_init(struct ti981_serdes *serdes)
{
	if (serdes->port == NULL) {
		cdc_err("serdes port is null");
		return -1;
	}
	struct ti981_link_port *port_info = serdes->port;
	if (port_info->panel_id == METER_PANEL) {
		port_info->port_lock_check = meter_lock_check;
		port_info->lcd_deser_config = meter_deser_config;
		port_info->port_resume = meter_power_resume;
	} else if (port_info->panel_id == SMART_PANEL) {
		port_info->port_lock_check = samrtpanel_lock_check;
		port_info->lcd_deser_config = smartpanel_deser_config;
		port_info->port_resume = smartpanel_power_resume;
	} else if (port_info->panel_id == PASSENGER_PANEL) {
		port_info->port_lock_check = samrtpanel_lock_check;
		port_info->lcd_deser_config = passenger_deser_config;
		port_info->port_resume = smartpanel_power_resume;
	} else if (port_info->panel_id == ICPIC_PANEL) {
		port_info->port_lock_check = meter_lock_check;
		port_info->lcd_deser_config = icpic_deser_config;
		port_info->port_resume = meter_power_resume;
	}

	port_info->lcd_power_on = lcd_wake_power_on;
	port_info->lcd_power_off = lcd_wake_power_off;
	return 0;
}

// rda <--> dsi1 <--> ti981 link port0 <--> default meter
// non_rda <--> dsi0 <--> ti981 link port1 <--> default smartpanel
int ti981_config_link_port(struct ti981_serdes *serdes)
{
	int ret = 0;
	if (serdes->domain == RDA_DOMAIN) {
		serdes->port0 = kzalloc(sizeof(struct ti981_link_port), GFP_KERNEL);
		if (!serdes->port0) {
			cdc_err("ti981_link_port0 kzalloc failed");
			return -ENOMEM;
		}
		serdes->port0->panel_id = serdes->link_port0_panel_id;
		serdes->port = serdes->port0;
	}

	if (serdes->domain == NON_RDA_DOMAIN && serdes->link_port1_panel_id != LINK_EMPTY) {
		serdes->port1 = kzalloc(sizeof(struct ti981_link_port), GFP_KERNEL);
		if (!serdes->port1) {
			cdc_err("ti981_link_port1 kzalloc failed");
			return -ENOMEM;
		}
		serdes->port1->panel_id = serdes->link_port1_panel_id;
		serdes->port = serdes->port1;
	}

	ret = ti981_link_port_init(serdes);
	if (ret) {
		cdc_err("ti981 link port0 init failed");
		if (serdes->port0) {
			kfree(serdes->port0);
			serdes->port0 = NULL;
		}
		else if (serdes->port1) {
			kfree(serdes->port1);
			serdes->port1 = NULL;
		}
		return ret;
	}

	return ret;
}

