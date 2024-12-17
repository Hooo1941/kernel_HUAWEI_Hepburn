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

#ifndef _SERDES_COMMON_H_
#define _SERDES_COMMON_H_

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
#include <linux/workqueue.h>
#include "ti981_ser_config.h"
#include "ti_deser_config.h"
#include "log.h"
#include "drv_mcu.h"
#include "ti983_ser_config.h"
#include "lcd_sysfs_for_dp.h"
#include "dpu_link_dp.h"

#define DP0_I2C_DEVICE_NUM 0
#define DP1_I2C_DEVICE_NUM 5
#define DTS_COMP_DP_PANEL "ti,dp_ti983"
#define TI983_I2C_NAME "dp_ti983"
#define DP_LVDS_CAB_NOT_LINKED 0
#define DP_LVDS_NOT_LINKED 1
#define DP_LVDS_NO_PCLK 2
#define DP_LVDS_LINKED 3
#define DSER_GENERAL_PURPOSE_STATUS_1 0x54

#define DP_RAMP_SHIFT 4
#define DP_TS_CODE_DN_MAX 7
#define DP_TEMP_FINAL 273
#define DP_TEMP_DOUBLE 2

#define DP_STATUS_ACTIVE 0
#define DP_STATUS_SUSPEND 1
#define DP_STATUS_RESUME 2

#define DESER_MAX_TRY_TIMES 10
#define COLUMN_NUM 4
#define REG_BUF_LEN 10
#define HEXADECIMAL 16
#define DECIMAL 10
#define MDELAY_10 10
#define MDELAY_40 40
#define MDELAY_VP_SYNC 100
#define MDELAY_EDID 600
#define LINK_NUM 3

// link panel status
enum panel_type {
	SMART_PANEL = 0x00,
	METER_PANEL = 0x01,
	PASSENGER_PANEL = 0x02,
	ICPIC_PANEL = 0x03, // E12 cluster
	ARHUD_PANEL = 0x04, // F2 ARHUD
	DLP_PANEL = 0x05,
	LINK_EMPTY = 0xFF,
};

#define DP_CONNECTOR_EDP0 4
#define DP_CONNECTOR_EDP1 5

#define RDA_DOMAIN 0x00
#define NON_RDA_DOMAIN 0x01

#define SER_I2C_ADDR "reg"
#define SER_DP_ID "dp_id"
#define SER_PDB "gpio_pdb_983"
#define SERDES_LINK_PORT0 "link_port0"
#define SERDES_LINK_PORT1 "link_port1"

struct serder_dts_data {
	unsigned char ser_addr;
	unsigned char des0_addr;
	unsigned char des1_addr;
	unsigned int gpio_pdb;
	int link_port0_id;
	int link_port1_id;
	unsigned int gpio_lcd_wake0;
	unsigned int gpio_lcd_wake1;
	int dp_id;
	int reserved1;
};

struct serdes_data;

struct ti983_dp_port {
	unsigned int panel_id;
	unsigned int gpio_lcd_wake;
	struct lcd_sysfs_ops *port_ops;
	int (*port_config_power_gpio)(struct serdes_data *serdes);
	int (*port_lock_check)(struct serdes_data *serdes);
	int (*lcd_power_on)(struct serdes_data *serdes);
	int (*lcd_power_off)(struct serdes_data *serdes);
	int (*lcd_deser_config)(struct serdes_data *serdes);
	int (*port_extern_check)(struct serdes_data *serdes);
	int (*port_resume)(struct serdes_data *serdes);
};


struct serdes_data {
	int domain;
	struct i2c_client *client;
	// I2C slave addr
	int (*serdes_config_slave_addr)(struct serdes_data *serdes);
	uint8_t* dp_edid;
	struct task_struct *monitor_thread;

	// workqueue
	struct workqueue_struct *serdes_wq;
	struct work_struct work_serdes_resume;
	struct work_struct initialize_wk;
	unsigned int lvds_link_status;
	unsigned int dp_resumed;

	// DTS Node
	struct serder_dts_data *p_data;
	unsigned short mcu_last_heartbeat;

	// ti983 port
	struct ti983_dp_port *dp_port0;
	struct ti983_dp_port *dp_port1;
	bool need_cropx;
	bool serdes_init_flag;
	// 983 vp 0/1
	int (*serdes_config_vp0)(struct serdes_data *serdes);
	int (*serdes_config_vp1)(struct serdes_data *serdes);

	int (*serdes_config_power_gpio)(struct serdes_data *serdes);
	int (*serdes_power_on)(struct serdes_data *serdes);
};

// ti983
int serdes_parse_dt(struct serdes_data *serdes, struct serder_dts_data *pdata);
int ti983_serdes_init(struct serdes_data *serdes, struct serder_dts_data *pdata);
int ti983_config_link_port(struct serdes_data *serdes, struct serder_dts_data *pdata);
int ti983_config_edid(struct serdes_data *serdes);
int notify_ser_dp_vp_sync(int32_t port_id);
int notify_ser_dp_unplug(int32_t port_id);

#endif /* _SERDES_COMMON_H_ */