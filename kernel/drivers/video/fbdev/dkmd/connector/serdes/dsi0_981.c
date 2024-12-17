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

#include <linux/regmap.h>
#include <linux/debugfs.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/init.h>
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
#include <linux/kthread.h>
#include <securec.h>
#include "./include/ti981_ser_config.h"
#include "./include/ti_deser_config.h"
#include "./common/log.h"
#include "./mcu/drv_mcu.h"
#include "./include/lcd_sysfs_for_dsi.h"
#include "./upgrade/upgrade_channel_manage.h"

#define DTS_COMP_DSI0_SMART_PANEL "ti,dsi0_ti981"
#define TI981_I2C_NAME "dsi0_ds981"
#define MIPI_LVDS_CAB_NOT_LINKED 0
#define MIPI_LVDS_NOT_LINKED 1
#define MIPI_LVDS_NO_PCLK 2
#define MIPI_LVDS_LINKED 3

#define MIPI_STATUS_ACTIVE 0
#define MIPI_STATUS_SUSPEND 1
#define MIPI_STATUS_RESUME 2

#define DESER_MAX_TRY_TIMES 100
#define COLUMN_NUM 4
#define REG_BUF_LEN 10
#define HEXADECIMAL 16
#define DECIMAL 10
#define MDELAY_10 10
#define SLEEP_TIME_S 1
#define HTOTAL_HIGH_BITS 0x11

uint32_t g_mipi_on_off = 0;
static struct task_struct *smart_monitor_thread = NULL;
struct i2c_client *g_ti981_client;

struct i2c_client *get_ti981_client(void)
{
	return g_ti981_client;
}

struct panel_power_info {
	u8 gpio_pdb;
	u8 gpio_lcd_wake;
};
struct panel_power_info dsi0_power_info;

unsigned int g_lvds_link_status = MIPI_LVDS_LINKED;
unsigned int g_mipi_resumed = MIPI_STATUS_ACTIVE;

static int dsi1_port1_988_init(struct i2c_client *client);
static int dsi0_port1_988_init(struct i2c_client *client);
static int lcd_wake_power_on(void);

static int dsi0_ti981_988_all_reset(struct i2c_client *client)
{
	BUG_ON(!client);

	int ret = -1;
	int try_cnt = 0;
	unsigned char lock_sts = 0;
	cdc_info("enter");
	dsi0_981_power_on();

	ser_981_init(client);

	while (1) {
		try_cnt++;
		i2c_read_reg(client->adapter,
			DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x54, &lock_sts);
		if (!(lock_sts & 0x1)) { /* bit 0 def lock status */
			msleep(50); /* delay 50ms for each period */
		} else {
			cdc_err("988 lock succ");
			break;
		}
		if (try_cnt > DESER_MAX_TRY_TIMES) {
			cdc_err("988 lock err, ret:%d\n", ret);
			return -1;
		}
	}

	/* clear try_cnt for deser lock sts */
	try_cnt = 0;

	cdc_info("smartpanel on position, init 988");
	mdelay(2); /* delay 2ms for 988 config */

	ret = dsi0_port1_988_init(client);
	if (ret < 0) {
		cdc_err("dsi0_port1_988_init error");
		return ret;
	}
	ret = dsi1_port1_988_init(client);
	/* if in hot_reinit and status is suspend or resume */
	if (ret < 0) {
		cdc_err("dsi1_port1_988_init error");
		return ret;
	}
	cdc_info("smartpanel on position, init 988 end");
	return ret;
}

static int panel_set_backlight(struct platform_device *pdev, uint32_t bl_level)
{
	BUG_ON(!g_ti981_client);
	struct i2c_client *client = g_ti981_client;
	cdc_info("enter");
	i2c_write_reg(client->adapter, BACKLIGHT_COMMAND, 0x02, bl_level);
	cdc_info("finish");
	return 0;
}

static int dsi0_sys_init(struct i2c_client *client)
{
	int ret = -1;
	ret = lcd_sysfs_init_for_dsi(client);
	if (ret)
		cdc_err("primary: dev_name = %s+.\n", dev_name(&client->dev));

	cdc_info("lcd_sysfs primary: dev_name = %s+.\n", dev_name(&client->dev));
	ret = sysfs_create_group(&(client->dev.kobj), &sysfs_attr_group_upgrade);
	if (ret)
		cdc_err("sysfs group upgrade creation failed, ret=%d", ret);
	cdc_info("sysfs group upgrade creation succ, ret=%d !", ret);
	return 0;
}

int dsi0_981_power_on(void)
{
	int ret = -1;
	int gpio_pdb_981 = dsi0_power_info.gpio_pdb;

	cdc_info("gpio_pdb_981 = %d", gpio_pdb_981);

	ret = gpio_direction_output(gpio_pdb_981, 0);
	if (ret != 0)
		cdc_err("gpio_pdb_981 direction out 0 fail");
	mdelay(MDELAY_10);
	ret = gpio_direction_output(gpio_pdb_981, 1);
	if (ret != 0)
		cdc_err("gpio_pdb_981 direction out 1 fail");
	cdc_info("gpio_pdb_981 being pulled end");

	mdelay(MDELAY_10);
	return ret;
}

static int lcd_wake_power_on(void)
{
	int ret = -1;
	int gpio_lcd_wake = dsi0_power_info.gpio_lcd_wake;

	cdc_info("gpio_lcd_wake = %d", gpio_lcd_wake);

	ret = gpio_direction_output(gpio_lcd_wake, 1);
	if (ret != 0)
		cdc_err("gpio_lcd_wake direction out 1 fail");
	cdc_info("gpio_lcd_wake power up finish");

	return ret;
}

int ser_981_init(struct i2c_client *client)
{
	int ret;
	unsigned char reg_value = 0;
	cdc_info("enter");

	ret = i2c_access(client->adapter, ser_981_init_part1, ARRAY_SIZE(ser_981_init_part1));
	if (ret)
		cdc_err("smart_ser_981_init_part1 fail ret %d", ret);

	/* Disable DSI */
	ret = i2c_read_reg(client->adapter, DS90UB981_I2C_7BIT_ADDR, 0x02, &reg_value);
	if (ret)
		cdc_err("read data from 981 fail ret %d", ret);
	ret  = i2c_write_reg(client->adapter, DS90UB981_I2C_7BIT_ADDR, 0x02, (reg_value | 0x08));
	if (ret)
		cdc_err("write data to 981 0x02 fail ret %d", ret);

	ret = i2c_access(client->adapter, ser_981_init_part2, ARRAY_SIZE(ser_981_init_part2));
	if (ret)
		cdc_err("smart_ser_981_init_part1 fail ret %d", ret);

	/* dsi0 lane */
	ret = i2c_read_reg(client->adapter, DS90UB981_I2C_7BIT_ADDR, 0x4f, &reg_value);
	if (ret)
		cdc_err("read data from 981 fail ret %d", ret);
	ret  = i2c_write_reg(client->adapter, DS90UB981_I2C_7BIT_ADDR, 0x4f, ((reg_value & 0x73) | 0x8c));
	if (ret)
		cdc_err("write data to 981 0x4f fail ret %d", ret);

	ret = i2c_access(client->adapter, ser_981_init_part3, ARRAY_SIZE(ser_981_init_part3));
	if (ret)
		cdc_err("smart_ser_981_init_part1 fail ret %d", ret);

	/* dsi0 lane */
	ret = i2c_read_reg(client->adapter, DS90UB981_I2C_7BIT_ADDR, 0x4f, &reg_value);
	if (ret)
		cdc_err("read data from 981 fail ret %d", ret);
	ret  = i2c_write_reg(client->adapter, DS90UB981_I2C_7BIT_ADDR, 0x4f, ((reg_value & 0x73) | 0x84));
	if (ret)
		cdc_err("write data to 981 0x4f fail ret %d", ret);

	ret = i2c_access(client->adapter, ser_981_init_part4, ARRAY_SIZE(ser_981_init_part4));
	if (ret)
		cdc_err("smart_ser_981_init_part1 fail ret %d", ret);

	/* Enable I2C Passthrough */
	ret = i2c_read_reg(client->adapter, DS90UB981_I2C_7BIT_ADDR, 0x07, &reg_value);
	if (ret)
		cdc_err("read data from 981 fail ret %d", ret);
	ret  = i2c_write_reg(client->adapter, DS90UB981_I2C_7BIT_ADDR, 0x07, reg_value | 0x08);
	if (ret)
		cdc_err("write data to 981 0x07 fail ret %d", ret);

	ret = i2c_access(client->adapter, ser_981_init_part5, ARRAY_SIZE(ser_981_init_part5));
	if (ret)
		cdc_err("smart_ser_981_init_part1 fail ret %d", ret);

	/* Enable DSI */
	ret = i2c_read_reg(client->adapter, DS90UB981_I2C_7BIT_ADDR, 0x02, &reg_value);
	if (ret)
		cdc_err("read data from 981 fail ret %d", ret);
	ret  = i2c_write_reg(client->adapter, DS90UB981_I2C_7BIT_ADDR, 0x02, (reg_value & 0xf7));
	if (ret)
		cdc_err("write data to 981 0x02 fail ret %d", ret);

	msleep(1);
	cdc_info("end");
	return ret;
}

static int dsi1_port1_988_init(struct i2c_client *client)
{
	int ret = -1;
	ret = i2c_access(client->adapter, deser_988_meter_pannel_init, ARRAY_SIZE(deser_988_meter_pannel_init));
	return ret;
}

static int dsi0_port1_988_init(struct i2c_client *client)
{
	int ret = -1;
	ret = i2c_access(client->adapter, deser_988_smart_pannel_init, ARRAY_SIZE(deser_988_smart_pannel_init));
	return ret;
}

static int dsi0_ti981_988_reset(struct i2c_client *client)
{
	BUG_ON(!client);

	int ret = -1;
	int try_cnt = 0;
	unsigned char des_addr = 0;
	unsigned char lock_sts = 0;
	unsigned char reg_value = 0;
	uint32_t init_value = 0;
	cdc_info("enter");

	ret = i2c_read_reg(client->adapter, DS90UB981_I2C_7BIT_ADDR, 0x07, &reg_value);
	if (ret < 0)
		cdc_err("read data to 981 error! reg = 0x%02x", TI981_DES_ID);
	ret = i2c_write_reg(client->adapter, DS90UB981_I2C_7BIT_ADDR, 0x07, reg_value | 0x08);
	if (ret < 0)
		cdc_err("read data to 981 error! reg = 0x%02x", TI981_DES_ID);
	ret = i2c_write_reg(client->adapter, DS90UB981_I2C_7BIT_ADDR, 0x2d, 0x01);
	if (ret < 0)
		cdc_err("write data to 981 error! reg = 0x2d");
	ret = i2c_read_reg(client->adapter, DS90UB981_I2C_7BIT_ADDR, TI981_DES_ID, &des_addr);
	if (ret < 0)
		cdc_err("read data to 981 error! reg = 0x%02x", TI981_DES_ID);

	ret = i2c_access(client->adapter, ser_981_init_part1, ARRAY_SIZE(ser_981_init_part1));
	if (ret)
		cdc_err("smart_ser_981_init_part1 fail ret %d", ret);

	while (1) {
		try_cnt++;
		i2c_read_reg(client->adapter,
			DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x54, &lock_sts);
		if (!(lock_sts & 0x1)) { /* bit 0 def lock status */
			msleep(50); /* delay 50ms for each period */
		} else {
			cdc_err("988 lock succ");
			break;
		}
		if (try_cnt > DESER_MAX_TRY_TIMES) {
			cdc_err("988 lock err, ret:%d\n", ret);
			return -1;
		}
	}

	/* clear try_cnt for deser lock sts */
	try_cnt = 0;

	cdc_info("smartpanel on position, init 988");
	mdelay(2); /* delay 2ms for 988 config */
	ret = dsi0_port1_988_init(client);
	/* if in hot_reinit and status is suspend or resume */
	if (ret < 0) {
		cdc_err("dsi0_port1_988_init error");
		return ret;
	}
	cdc_info("smartpanel on position, init 988 end");
	return ret;
}

static void dsi0_lcd_gpio_config(void)
{
	struct device_node *np = NULL;
	int ret;

	cdc_info("enter");

	np = of_find_compatible_node(NULL, NULL, DTS_COMP_DSI0_SMART_PANEL);
	if (!np)
		cdc_err("NOT FOUND device node :%s!", DTS_COMP_DSI0_SMART_PANEL);

	dsi0_power_info.gpio_pdb = of_get_named_gpio(np, "gpios", 0);
	if (!gpio_is_valid(dsi0_power_info.gpio_pdb)) {
		cdc_err("get gpio_pdb_981 failed");
		return;
	}
	ret = gpio_request(dsi0_power_info.gpio_pdb, "gpio_pdb_981");
	if (ret) {
		cdc_err("gpio request gpio_pdb_981 fail");
		return;
	}
}

static int dsi0_resume_handle(void)
{
	BUG_ON(!g_ti981_client);
	dsi0_lcd_gpio_config();
#ifdef CONFIG_PRODUCT_CDC_ACE
	return dsi0_ti981_988_reset(g_ti981_client);
#else
	return dsi0_ti981_988_all_reset(g_ti981_client);
#endif
}

static int enable_vp1_sync(struct i2c_client *client)
{
	int ret = -1;
	unsigned char reg_value = 0;
	ret = i2c_read_reg(client->adapter, DS90UB981_I2C_7BIT_ADDR, 0x44, &reg_value);
	if (ret < 0) {
		cdc_err("read data from 981 error! reg = 0x44");
		return ret;
	}

	ret = i2c_write_reg(client->adapter, DS90UB981_I2C_7BIT_ADDR, 0x44, reg_value & 0xfd);
	if (ret < 0) {
		cdc_err("write data to 981 error! reg = 0x44");
		return ret;
	}

	ret = i2c_write_reg(client->adapter, DS90UB981_I2C_7BIT_ADDR, 0x44, reg_value | 0x02);
	if (ret < 0) {
		cdc_err("write data to 981 error! reg = 0x44");
		return ret;
	}
	return 0;
}

static int thread_smartpanel_monitor(void* thread_data)
{
	BUG_ON(!g_ti981_client);
	int ret = -1;
	unsigned char reg_value = 0;
	struct i2c_client *client = g_ti981_client;
	bool enable_vp1_flag = false;
	cdc_info("enter smart monitor thread");
	while (1) {
		if (g_mipi_resumed == MIPI_STATUS_RESUME) {
			ret = dsi0_resume_handle();
			if (ret < 0) {
				cdc_err("resume failed, lvds_init_status not linked, need hot_reinit");
			} else {
				cdc_err("resume success");
				g_lvds_link_status = MIPI_LVDS_LINKED;
				g_mipi_resumed = MIPI_STATUS_ACTIVE;
			}
		}

		ret = i2c_read_reg(client->adapter, DS90UB981_I2C_7BIT_ADDR, 0x45, &reg_value);
		if (ret < 0) {
			cdc_err("read data from 981 error! reg = 0x45");
			ssleep(SLEEP_TIME_S);
			continue;
		}
		/* enable vp1 until synchronization is on */
		if (!(reg_value & 0x02) && !enable_vp1_flag) {
			ret = enable_vp1_sync(client);
			if (ret < 0)
				cdc_err("enable vp1 error!");
		} else {
			enable_vp1_flag = true;
		}
		ssleep(SLEEP_TIME_S);
		/* data from 988 */
		(void)i2c_write_reg(client->adapter, DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x40, 0x53);
		(void)i2c_write_reg(client->adapter, DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x41, 0x40);
		(void)i2c_read_reg(client->adapter, DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, &reg_value);
		(void)i2c_read_reg(client->adapter, DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42, &reg_value);
	}
	return 0;
}

static int ti981_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int ret;
	cdc_err("enter ti981_i2c_probe nonrda");
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C))
		cdc_err("get i2c adapter fail");
	g_ti981_client = client;
	dsi0_sys_init(g_ti981_client);
	cdc_info("primary: dev_name = %s+.\n", dev_name(&client->dev));
	dsi0_lcd_gpio_config();
#ifdef CONFIG_PRODUCT_CDC_ACE
	cdc_info("enter ti981_reset");
	ret = dsi0_ti981_988_reset(client);
#endif

#ifdef CONFIG_PRODUCT_CDC
	/* single os version */
	cdc_info("enter ti981_all_reset");
	ret = dsi0_ti981_988_all_reset(client);
#endif
	if (ret < 0) {
		cdc_err("panel init failed, link status is err");
		g_lvds_link_status = MIPI_LVDS_NOT_LINKED;
	}

	smart_monitor_thread = kthread_create(thread_smartpanel_monitor, NULL, "smartpanel_monitor");
	if (!IS_ERR(smart_monitor_thread))
		wake_up_process(smart_monitor_thread);
	cdc_info("create smart monitor thread success");
	return 0;
}

static int ti981_pm_suspend(struct device *dev)
{
	int ret = 0;

	cdc_info("enter");
	g_lvds_link_status = MIPI_LVDS_CAB_NOT_LINKED;
	g_mipi_resumed = MIPI_STATUS_SUSPEND;

	ret = gpio_direction_output(dsi0_power_info.gpio_lcd_wake, 0);
	if (ret != 0)
		cdc_err("gpio_pdb_981 direction out 0 fail\n");
	gpio_free(dsi0_power_info.gpio_pdb);
	gpio_free(dsi0_power_info.gpio_lcd_wake);

	return ret;
}

static int ti981_pm_resume(struct device *dev)
{
	cdc_info("enter");
	g_lvds_link_status = MIPI_LVDS_CAB_NOT_LINKED;
	g_mipi_resumed = MIPI_STATUS_RESUME;
	return 0;
}


static const struct i2c_device_id ti981_id[] = {
	{ TI981_I2C_NAME, 0 },
	{}
};

static const struct dev_pm_ops ti981_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(ti981_pm_suspend, ti981_pm_resume)
};


static const struct of_device_id ti981_match_table[] = {
	{
	.compatible = DTS_COMP_DSI0_SMART_PANEL,
	.data = NULL,
	},
	{},
};

static struct i2c_driver ti981_i2c_driver = {
	.probe		  = ti981_i2c_probe,
	.remove		   = NULL,
	.id_table	 = ti981_id,
	.driver = {
		.name	 = "TI981_I2C_NAME",
		.owner	  = THIS_MODULE,
		.of_match_table = ti981_match_table,
		.pm = &ti981_pm_ops,
	},
};

static int __init dsi0_981_init(void)
{
	int ret = 0;
	cdc_info("enter dsi0_981_init");
	ret = i2c_add_driver(&ti981_i2c_driver);
	if (ret) {
		cdc_err("dsi1 i2c_add_driver fail, error=%d\n", ret);
		return ret;
	}
	return ret;
}

late_initcall(dsi0_981_init);
