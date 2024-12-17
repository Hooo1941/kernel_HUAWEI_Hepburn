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
#include "ti981_ser_config.h"
#include "ti_deser_config.h"
#include "log.h"
#include "drv_mcu.h"
#include "lcd_sysfs.h"
#include "lcd_sysfs_for_dsi.h"
#include "ti981_common.h"

uint32_t g_ivi_last_backlight = 0;
static struct i2c_client *ti981_client = NULL;

unsigned char g_serdes_addr = DS90UB981_I2C_7BIT_ADDR;
unsigned char g_page_num = 0;

unsigned char get_serdes_addr(void)
{
	return g_serdes_addr;
}

void set_serdes_addr(unsigned char serdes_addr)
{
	g_serdes_addr = serdes_addr;
}

unsigned char get_page_num(void)
{
	return g_page_num;
}

void set_page_num(unsigned char page_num)
{
	g_page_num = page_num;
}

uint32_t get_last_backlight(void)
{
	return g_ivi_last_backlight;
}

void set_last_backlight(uint32_t last_backlight)
{
	g_ivi_last_backlight = last_backlight;
}

static ssize_t ser_pattern_test_port0(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = NULL;

	client = ti981_client;
	sysfs_info("enter");
	i2c_write_reg(client->adapter, DS90UB981_I2C_7BIT_ADDR, 0x40, 0x30);
	i2c_write_reg(client->adapter, DS90UB981_I2C_7BIT_ADDR, 0x41, 0x29);
	i2c_write_reg(client->adapter, DS90UB981_I2C_7BIT_ADDR, 0x42, 0x08);
	i2c_write_reg(client->adapter, DS90UB981_I2C_7BIT_ADDR, 0x41, 0x28);
	i2c_write_reg(client->adapter, DS90UB981_I2C_7BIT_ADDR, 0x42, 0x95);
	i2c_write_reg(client->adapter, DS90UB981_I2C_7BIT_ADDR, 0x41, 0x69);
	i2c_write_reg(client->adapter, DS90UB981_I2C_7BIT_ADDR, 0x42, 0x08);
	i2c_write_reg(client->adapter, DS90UB981_I2C_7BIT_ADDR, 0x41, 0x68);
	i2c_write_reg(client->adapter, DS90UB981_I2C_7BIT_ADDR, 0x42, 0x95);
	sysfs_info("finish");
	return strlen(buf);
}

static ssize_t deser_pattern_test_port0(struct device *dev, struct device_attribute *attr, char *buf)
{
	sysfs_info("enter");
	struct i2c_client *client = NULL;

	client = ti981_client;
	i2c_write_reg(client->adapter, DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x40,0x50); // Set Patgen page
	i2c_write_reg(client->adapter, DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x41,0x1); // Set patgen address
	i2c_write_reg(client->adapter, DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42,0xc); // Set bit per pixel
	i2c_write_reg(client->adapter, DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x41,0x2); // Set patgen address
	i2c_write_reg(client->adapter, DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42,0x86); // Set patgen address auto increment
	i2c_write_reg(client->adapter, DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x41,0x3); // Set patgen address
	i2c_write_reg(client->adapter, DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42,0xa0); // Set patgen THW
	i2c_write_reg(client->adapter, DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42,0x09); // Set patgen THW
	i2c_write_reg(client->adapter, DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42,0xa4); // Set patgen TVW
	i2c_write_reg(client->adapter, DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42,0x08); // Set patgen TVW
	i2c_write_reg(client->adapter, DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42,0x70); // Set patgen AHW
	i2c_write_reg(client->adapter, DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42,0x08); // Set patgen AHW  //dpi hize *2
	i2c_write_reg(client->adapter, DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42,0x70); // Set patgen AVW
	i2c_write_reg(client->adapter, DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42,0x08); // Set patgen AVW
	i2c_write_reg(client->adapter, DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42,0x18); // Set patgen HSW
	i2c_write_reg(client->adapter, DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42,0x00); // Set patgen HSW
	i2c_write_reg(client->adapter, DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42,0x0a); // Set patgen VSW
	i2c_write_reg(client->adapter, DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42,0x0); // Set patgen VSW
	i2c_write_reg(client->adapter, DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42,0xc0); // Set patgen HBP
	i2c_write_reg(client->adapter, DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42,0x0); // Set patgen HBP
	i2c_write_reg(client->adapter, DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42,0x1a); // Set patgen VBP
	i2c_write_reg(client->adapter, DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42,0x0); // Set patgen VBP
	i2c_write_reg(client->adapter, DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42,0x0); // HSYNC Polarity = +, VSYNC Polarity
	i2c_write_reg(client->adapter, DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x41,0x0); // Set patgen address
	i2c_write_reg(client->adapter, DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x42,0x95); // Enable Patgen color bar
	sysfs_info("finish");
	return strlen(buf);
}

static ssize_t ser_main_regs_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int i = 0;
	int ret = 0;
	unsigned char reg_value = 0;
	struct i2c_client *client = NULL;

	client = ti981_client;

	if (buf == NULL) {
		sysfs_err("buff is null\n");
		return -EINVAL;
	}
	ret = memset_s(buf, sizeof(buf), 0, PAGE_SIZE);
	if (ret != 0)
		sysfs_err("memset_s error");
	ret = snprintf_s(buf + strlen(buf), PAGE_SIZE - strlen(buf), PAGE_SIZE - strlen(buf), "[main_reg_list:]\n");
	if (ret < 0)
		sysfs_err("ser main_reg_list printf err");

	for (i = 0; i <= SER_MAIN_REG_MAXCNT; i++) {
		i2c_read_reg(client->adapter, DS90UB981_I2C_7BIT_ADDR, i, &reg_value);

		if ((i + 1) % COLUMN_NUM == 0) {
			ret = snprintf_s(buf + strlen(buf), PAGE_SIZE - strlen(buf), PAGE_SIZE - strlen(buf),
				"0x%02x--0x%02x\n", i, reg_value);
			if (ret < 0)
				sysfs_err("ser main regs show err for column");
		} else {
			ret = snprintf_s(buf + strlen(buf), PAGE_SIZE - strlen(buf), PAGE_SIZE - strlen(buf),
				"0x%02x--0x%02x\n", i, reg_value);
			if (ret < 0)
				sysfs_err("ser main regs show err");
		}
	}
	ret = snprintf_s(buf + strlen(buf), PAGE_SIZE - strlen(buf), PAGE_SIZE - strlen(buf), "\n");
	if (ret < 0)
		sysfs_err("ser main regs show err for last page");
	sysfs_info("%s\n", buf);
	return strlen(buf);
}

// [reg_addr]:[reg_val]
static ssize_t ser_main_regs_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	unsigned int reg_addr = 0;
	unsigned int reg_val = 0;
	int ret = -1;
	char *p = NULL;
	char reg_buf[REG_BUF_LEN];
	struct i2c_client *client = NULL;

	client = ti981_client;
	ret = memset_s(reg_buf, sizeof(reg_buf), 0, REG_BUF_LEN);
	if (ret != 0)
		sysfs_err("memset_s error");

	p = strchr(buf, ':');
	if (p == NULL) {
		sysfs_err("err:can not find '!'\n");
		return -EINVAL;
	}
	ret = memcpy_s(reg_buf, sizeof(reg_buf), buf, p - buf);
	if (ret != 0)
		sysfs_err("memcpy_s error");

	ret = kstrtoint(reg_buf, HEXADECIMAL, &reg_addr);
	if (ret != 0) {
		sysfs_err("err: kstrtoint failed!\n");
		return -EINVAL;
	}

	if (reg_addr > SER_MAIN_REG_MAXCNT) {
		sysfs_err("err:your input [] is too big!\n");
		return -EINVAL;
	}

	ret = kstrtoint(p + 1, HEXADECIMAL, &reg_val);
	if (ret != 0) {
		sysfs_err("err: kstrtoint failed!\n");
		return -EINVAL;
	}

	sysfs_err("reg[0x%x] = 0x%x\n", reg_addr, reg_val);
	i2c_write_reg(client->adapter, DS90UB981_I2C_7BIT_ADDR, reg_addr, reg_val);
	return count;
}

static ssize_t deser_main_regs_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int i = 0;
	unsigned char reg_value = 0;
	struct i2c_client *client = NULL;
	int ret = 0;

	client = ti981_client;
	if (buf == NULL) {
		sysfs_err("buff is null\n");
		return -EINVAL;
	}
	ret = memset_s(buf, sizeof(buf), 0, PAGE_SIZE);
	if (ret != 0)
		sysfs_err("memset_s error");
	ret = snprintf_s(buf + strlen(buf), PAGE_SIZE - strlen(buf), PAGE_SIZE - strlen(buf), "[main_reg_list:]\n");
	if (ret < 0)
		sysfs_err("deser main_reg_list printf err");
	for (i = 0; i <= SER_MAIN_REG_MAXCNT; i++) {
		i2c_read_reg(client->adapter, DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, i, &reg_value);

		if ((i + 1) % COLUMN_NUM == 0) {
			ret = snprintf_s(buf + strlen(buf), PAGE_SIZE - strlen(buf), PAGE_SIZE - strlen(buf), "0x%02x--0x%02x\n", i,
				reg_value);
			if (ret < 0)
				sysfs_err("dser main regs show err for column");
		} else {
			ret = snprintf_s(buf + strlen(buf), PAGE_SIZE - strlen(buf), PAGE_SIZE - strlen(buf), "0x%02x--0x%02x	 ",
				i, reg_value);
			if (ret < 0)
				sysfs_err("dser main regs show err");
		}
	}
	ret = snprintf_s(buf + strlen(buf), PAGE_SIZE - strlen(buf), PAGE_SIZE - strlen(buf), "\n");
	if (ret < 0)
		sysfs_err("dser main regs show err for last page");
	sysfs_info("%s\n", buf);
	return strlen(buf);
}

// [reg_addr]:[reg_val]
static ssize_t deser_main_regs_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	unsigned int reg_addr = 0;
	unsigned int reg_val = 0;
	int ret = -1;
	char *p = NULL;
	char reg_buf[REG_BUF_LEN];
	struct i2c_client *client = NULL;

	client = ti981_client;
	ret = memset_s(reg_buf, sizeof(reg_buf), 0, REG_BUF_LEN);
	if (ret != 0)
		sysfs_err("memset_s error");

	p = strchr(buf, ':');
	if (p == NULL) {
		sysfs_err("err:can not find '!'\n");
		return -EINVAL;
	}
	ret = memcpy_s(reg_buf, sizeof(reg_buf), buf, p - buf);
	if (ret != 0)
		sysfs_err("memcpy_s error");

	ret = kstrtoint(reg_buf, HEXADECIMAL, &reg_addr);
	if (ret != 0) {
		sysfs_err("err: kstrtoint failed!\n");
		return -EINVAL;
	}

	if (reg_addr > SER_MAIN_REG_MAXCNT) {
		sysfs_err("err:your input [] is too big!\n");
		return -EINVAL;
	}

	ret = kstrtoint(p + 1, HEXADECIMAL, &reg_val);
	if (ret != 0) {
		sysfs_err("err: kstrtoint failed!\n");
		return -EINVAL;
	}

	sysfs_info("reg[0x%x] = 0x%x\n", reg_addr, reg_val);
	i2c_write_reg(client->adapter, DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, reg_addr, reg_val);
	return count;
}

static ssize_t ser_brightness_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct i2c_client *client = NULL;
	client = ti981_client;
	int ret = 0;
	int brightness = 0;

	ret = kstrtoint(buf, DECIMAL, &brightness);
	if (ret != 0) {
		sysfs_err("%s err: kstrtoint failed!", buf);
		return -EINVAL;
	}

	ret = set_backlight_to_smart_panel(ti981_client, brightness);
	if (ret < 0)
		sysfs_err("set backlight brightness to %d failed", brightness);
	g_ivi_last_backlight = brightness;
	sysfs_info("set backlight brightness to %d", brightness);
	return count;
}

static ssize_t ser_brightness_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = NULL;
	int ret = 0;
	int brightness = 0;

	client = ti981_client;
	if (buf == NULL) {
		sysfs_err("buff is null\n");
		return -EINVAL;
	}
	ret = memset_s(buf, PAGE_SIZE, 0, PAGE_SIZE);
	if (ret != 0)
		sysfs_err("memset_s error");
	brightness = get_backlight_from_smart_panel(ti981_client);
	if (brightness < 0)
		sysfs_err("get backlight from mcu failed");
	ret = snprintf_s(buf, PAGE_SIZE, PAGE_SIZE, "ser_backlight:%d\n", brightness);
	if (ret < 0)
		sysfs_err("ser brightness show page");
	return strlen(buf);
}

static ssize_t get_serdes_page_register(struct device *dev, struct device_attribute *attr, char *buf)
{
	int i = 0;
	int ret = -1;
	unsigned char reg_value = 0;
	unsigned char serdes_addr = 0;
	unsigned char page_num = 0;
	unsigned char page_num_reg = 0;
	struct i2c_client *client = NULL;
	struct ti981_serdes *serdes = NULL;

	client = ti981_client;
	serdes = i2c_get_clientdata(to_i2c_client(dev));
	if (buf == NULL) {
		sysfs_err("buff is null\n");
		return -EINVAL;
	}

	serdes_addr = get_serdes_addr();
	page_num= get_page_num();
	page_num_reg = (page_num << 0x02) | 0x03;
	sysfs_info("enter get_serdes_page_register, page[%d]", page_num);

	/* config serdes register */
	i2c_write_reg(client->adapter, serdes_addr, 0x40, page_num_reg);
	i2c_write_reg(client->adapter, serdes_addr, 0x41, 0x00);

	ret = memset_s(buf, sizeof(buf), 0, PAGE_SIZE);
	if (ret != 0)
		sysfs_err("memset_s error");

	ret = snprintf_s(buf + strlen(buf), PAGE_SIZE - strlen(buf), PAGE_SIZE - strlen(buf),
		"addr[0x%x]\n[page%d_reg_list:]\n", serdes_addr, page_num);
	if (ret < 0)
		sysfs_err("get_serdes_page_register page[%d]_reg_list printf err", page_num);

	for (i = 0; i <= SER_MAIN_REG_MAXCNT; i++) {
		i2c_read_reg(client->adapter, serdes_addr, 0x42, &reg_value);

		if ((i + 1) % COLUMN_NUM == 0) {
			ret = snprintf_s(buf + strlen(buf), PAGE_SIZE - strlen(buf),
				PAGE_SIZE - strlen(buf), "0x%02x--0x%02x\n", i, reg_value);
			if (ret < 0)
				sysfs_err("get_serdes_page_register err for column");
		} else {
			ret = snprintf_s(buf + strlen(buf), PAGE_SIZE - strlen(buf),
				PAGE_SIZE - strlen(buf), "0x%02x--0x%02x", i, reg_value);
			if (ret < 0)
				sysfs_err("get_serdes_page_register err");
		}
	}

	ret = snprintf_s(buf + strlen(buf), PAGE_SIZE - strlen(buf), PAGE_SIZE - strlen(buf), "\n");
	if (ret < 0)
		sysfs_err("get_serdes_page_register err for last page");
	sysfs_info("%s\n", buf);
	sysfs_info("end get_serdes_page_register ret = %d", ret);

	return strlen(buf);
}

static ssize_t set_serdes_page(struct device *dev, struct device_attribute *attr,
	const char *buf, size_t count)
{
	unsigned int serdes_addr = 0;
	unsigned int page_num = 0;
	int ret = -1;
	char *p = NULL;
	char reg_buf[REG_BUF_LEN];

	ret = memset_s(reg_buf, sizeof(reg_buf), 0, REG_BUF_LEN);
	if (ret != 0)
		sysfs_err("memset_s error");

	p = strchr(buf, ':');
	if (p == NULL) {
		sysfs_err("err:can not find '!'\n");
		return -EINVAL;
	}
	ret = memcpy_s(reg_buf, sizeof(reg_buf), buf, p - buf);
	if (ret != 0)
		sysfs_err("memcpy_s error");

	ret = kstrtoint(reg_buf, HEXADECIMAL, &serdes_addr);
	if (ret != 0) {
		sysfs_err("err: kstrtoint failed!\n");
		return -EINVAL;
	}

	ret = kstrtoint(p + 1, HEXADECIMAL, &page_num);
	if (ret != 0) {
		sysfs_err("err: kstrtoint failed!\n");
		return -EINVAL;
	}
	set_serdes_addr((unsigned char)(serdes_addr & 0xff));
	set_page_num((unsigned char)(page_num & 0xff));
	sysfs_info("page_num[0x%x]\n", page_num);

	return count;
}

static int lcd_check_support(int index)
{
	struct lcd_sysfs_ops *sysfs_ops = NULL;

	switch (index) {
	case SER_PATTERN_INDEX:
	case DESER_PATTERN_INDEX:
		return SYSFS_SUPPORT;
	case PANEL_INFO_INDEX:
		return SYSFS_NOT_SUPPORT;
	case SER_MAIN_REGS:
	case DESER_MAIN_REGS:
		return SYSFS_SUPPORT;
	case SER_PANEL_ON:
	case SER_PANEL_OFF:
		return SYSFS_NOT_SUPPORT;
	case SER_BRIGHTNESS:
		return SYSFS_SUPPORT;
	case LCD_OTA_VERSION:
		return SYSFS_NOT_SUPPORT;
	case LIGHT_SENSOR:
	case LCD_SN_CODE:
	case LCD_FACTORY_OPS:
	case LCD_FAIL_DET:
		return SYSFS_NOT_SUPPORT;
	case LCD_DEFAULT:
		return SYSFS_SUPPORT;
	default:
		return SYSFS_NOT_SUPPORT;
	}
}

static struct lcd_sysfs_ops g_lcd_sysfs_fs_ops = {
	.check_support = lcd_check_support,
	.ser_pattern_port0_show = ser_pattern_test_port0,
	.deser_pattern_port0_show = deser_pattern_test_port0,
	.ser_main_regs_show = ser_main_regs_show,
	.ser_main_regs_store = ser_main_regs_store,
	.deser_main_regs_show = deser_main_regs_show,
	.deser_main_regs_store = deser_main_regs_store,
	.ser_brightness_show = ser_brightness_show,
	.ser_brightness_store = ser_brightness_store,
	.lcd_default_test_show = get_serdes_page_register,
	.lcd_default_test_store = set_serdes_page,
};

static struct lcd_sysfs_lock sysfs_lock_dsi;

int lcd_sysfs_init_for_dsi(struct i2c_client *client)
{
	int ret = 0;
	ret = LCD_OK;
	ti981_client = client;
	lcd_lock_init(&sysfs_lock_dsi, LCD_DSI_INDEX);
	lcd_sysfs_ops_register(&g_lcd_sysfs_fs_ops, LCD_DSI_INDEX);
	ret = lcd_create_sysfs(client, LCD_DSI_INDEX);
	if (ret)
		sysfs_err("create fs node fail\n");
	return ret;
}
