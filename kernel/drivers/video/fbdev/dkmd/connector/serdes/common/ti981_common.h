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

#ifndef _TI981_COMMON_H_
#define _TI981_COMMON_H_

#include <linux/regmap.h>
#include <linux/debugfs.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/init.h>
#include <linux/kthread.h>
#include <securec.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/pm.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/of_graph.h>
#include "../include/ti981_ser_config.h"
#include "../include/ti_deser_config.h"
#include "log.h"
#include "../mcu/drv_mcu.h"

#define DTS_COMP_DSI1_METER_PANEL "ti,dsi1_ti981"
#define DTS_COMP_DSI0_SMART_PANEL "ti,dsi0_ti981"
#define TI981_I2C_NAME "dsi1_ti981"
#define TI981_I2C_NAME_DSI0 "dsi0_ti981"
#define MIPI_LVDS_CAB_NOT_LINKED 0
#define MIPI_LVDS_NOT_LINKED 1
#define MIPI_LVDS_NO_PCLK 2
#define MIPI_LVDS_LINKED 3

#define MIPI_STATUS_ACTIVE 0
#define MIPI_STATUS_SUSPEND 1
#define MIPI_STATUS_RESUME 2

#define DESER_MAX_TRY_TIMES 10
#define VP_SYNC_MAX_TRY_TIMES 3
#define COLUMN_NUM 4
#define REG_BUF_LEN 10
#define HEXADECIMAL 16
#define DECIMAL 10
#define MDELAY_10 10
#define MAX_BACKLIGHT_VALUE 1000

#define SLEEP_TIME_S 1
#define LCD_WAKE_DELAY_TIME 500

/* ti981 link port status */
#define LINK_EMPTY 0xFF
#define SMART_PANEL 0x00
#define METER_PANEL 0x01
#define PASSENGER_PANEL 0x02
#define ICPIC_PANEL 0x03 // E12 cluster

#define DSI_NUM_0 0x00
#define DSI_NUM_1 0x01
#define DSI_NUM 2

#define RDA_DOMAIN 0x00
#define NON_RDA_DOMAIN 0x01

#define TI981_LINK_PORT0 "link_port0"
#define TI981_LINK_PORT1 "link_port1"

#define DSI_LANE_NUM "dsi_lane"
#define DSI_TSKIP "tskip"

struct ti981_serdes;
struct ti981_link_port {
	unsigned int panel_id;
	int gpio_lcd_wake;
	struct lcd_sysfs_ops *port_ops;
	int (*port_lock_check)(struct ti981_serdes *serdes);
	int (*lcd_power_on)(struct ti981_serdes *serdes);
	int (*lcd_power_off)(struct ti981_serdes *serdes);
	int (*lcd_deser_config)(struct ti981_serdes *serdes);
	int (*port_extern_check)(struct ti981_serdes *serdes);
	int (*port_resume)(struct ti981_serdes *serdes);
};

struct ti981_dsi_config {
	unsigned int dsi0_lane_num;
	unsigned int dsi1_lane_num;
	unsigned int dsi0_tskip;
	unsigned int dsi1_tskip;
};

/*
 * rda <--> dsi1 <--> ti981 link port0 <--> default meter
 * non_rda <--> dsi0 <--> ti981 link port1 <--> default smartpanel
 */
struct ti981_serdes {
	int domain;
	struct i2c_client *client;
	int gpio_pdb; // ti981 power gpio

	// I2C slave addr
	int (*serdes_config_slave_addr)(struct ti981_serdes *serdes);

	// dsi0/1
	int (*serdes_config_dsi0)(struct ti981_serdes *serdes);
	int (*serdes_config_dsi1)(struct ti981_serdes *serdes);

	unsigned int ivi_last_backlight;
	unsigned int mipi_on_off;
	unsigned int lvds_link_status;
	unsigned int mipi_resumed;
	struct task_struct *monitor_thread;

	unsigned int link_port0_panel_id;
	unsigned int link_port1_panel_id;

	struct ti981_link_port *port;
	struct ti981_link_port *port0;
	struct ti981_link_port *port1;
	struct ti981_dsi_config *dsi_config;

	int (*serdes_config_power_gpio)(struct ti981_serdes *serdes);
	int (*serdes_power_on)(struct ti981_serdes *serdes);
	int (*serdes_check_des_devid)(struct ti981_serdes *serdes);
	int (*serdes_extern_check)(struct ti981_serdes *serdes);
};

int ti981_serdes_init(struct ti981_serdes *serdes);
int ti981_config_dsi(struct ti981_serdes *serdes);
int ti981_config_link_port(struct ti981_serdes *serdes);
int ti981_power_on(struct ti981_serdes *serdes);

int lcd_wake_power_on(struct ti981_serdes *serdes);
int lcd_wake_power_off(struct ti981_serdes *serdes);

void set_hot_reinit_flag(bool flag);
bool get_hot_reinit_flag(void);
int notify_ser_dsi_vp_sync(uint32_t idx);

#endif /* _TI981_COMMON_H_ */
