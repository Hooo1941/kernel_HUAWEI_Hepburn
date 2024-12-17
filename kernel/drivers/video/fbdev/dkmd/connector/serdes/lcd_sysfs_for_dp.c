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
#include "ti983_ser_config.h"
#include "ti_deser_config.h"
#include "log.h"
#include "drv_mcu.h"
#include "lcd_sysfs.h"
#include "lcd_sysfs_for_dp.h"
#include "serdes_common.h"

static uint32_t g_ivi_last_backlight = 0;

static unsigned char g_serdes_addr = DS90UB983_SER2_I2C_7BIT_ADDR;
static unsigned char g_page_num = 0;
static unsigned int g_apb_addr = 0;

static unsigned char get_serdes_addr(void)
{
	return g_serdes_addr;
}

static void set_serdes_addr(unsigned char serdes_addr)
{
	g_serdes_addr = serdes_addr;
}

static unsigned char get_page_num(void)
{
	return g_page_num;
}

static void set_page_num(unsigned char page_num)
{
	g_page_num = page_num;
}

static void set_g_apb_addr(unsigned int apb_addr)
{
	g_apb_addr = apb_addr;
}

static ssize_t ser_pattern_test_vp01(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = NULL;
	struct serdes_data *serdes = NULL;

	sysfs_info("enter ser_pattern");
	client = to_i2c_client(dev);
	serdes = i2c_get_clientdata(client);
	i2c_write_reg(client->adapter, serdes->p_data->ser_addr, 0x40, 0x30);
	i2c_write_reg(client->adapter, serdes->p_data->ser_addr, 0x41, 0x29);
	i2c_write_reg(client->adapter, serdes->p_data->ser_addr, 0x42, 0x08);
	i2c_write_reg(client->adapter, serdes->p_data->ser_addr, 0x41, 0x28);
	i2c_write_reg(client->adapter, serdes->p_data->ser_addr, 0x42, 0x95);
	i2c_write_reg(client->adapter, serdes->p_data->ser_addr, 0x41, 0x69);
	i2c_write_reg(client->adapter, serdes->p_data->ser_addr, 0x42, 0x08);
	i2c_write_reg(client->adapter, serdes->p_data->ser_addr, 0x41, 0x68);
	i2c_write_reg(client->adapter, serdes->p_data->ser_addr, 0x42, 0x95);
	sysfs_info("finish ser_pattern");

	return strlen(buf);
}

static ssize_t deser_pattern_test(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = NULL;
	struct serdes_data *serdes = NULL;

	sysfs_info("enter deser_pattern");
	client = to_i2c_client(dev);
	serdes = i2c_get_clientdata(client);
	if (serdes->p_data->des0_addr != 0x00) {
		i2c_write_reg(client->adapter, serdes->p_data->des0_addr, 0x40, 0x50);
		i2c_write_reg(client->adapter, serdes->p_data->des0_addr, 0x41, 0x00);
		i2c_write_reg(client->adapter, serdes->p_data->des0_addr, 0x42, 0x95);
	}
	if (serdes->p_data->des1_addr != 0x00) {
		i2c_write_reg(client->adapter, serdes->p_data->des1_addr, 0x40, 0x50);
		i2c_write_reg(client->adapter, serdes->p_data->des1_addr, 0x41, 0x00);
		i2c_write_reg(client->adapter, serdes->p_data->des1_addr, 0x42, 0x95);
	}
	sysfs_info("finish deser_pattern");

	return strlen(buf);
}

static ssize_t main_reg_show(struct i2c_client *client, unsigned char dev_addr, char *buf)
{
	int i = 0;
	int ret = 0;
	unsigned char reg_value = 0;

	if (client == NULL) {
		sysfs_err("client is null");
		return -EINVAL;
	}
	if (buf == NULL) {
		sysfs_err("buf is null");
		return -EINVAL;
	}
	for (i = 0; i <= SER_MAIN_REG_MAXCNT; i++) {
		i2c_read_reg(client->adapter, dev_addr, i, &reg_value);
		if ((i + 1) % COLUMN_NUM == 0) {
			ret = snprintf_s(
				buf + strlen(buf), PAGE_SIZE - strlen(buf), PAGE_SIZE - strlen(buf) - 1,\
				 "0x%02x--0x%02x\n", i, reg_value);
			if (ret < 0)
				sysfs_err("ser main regs show err for column");
		} else {
			ret = snprintf_s(buf + strlen(buf),PAGE_SIZE - strlen(buf),
				PAGE_SIZE - strlen(buf) - 1,"0x%02x--0x%02x    ", i, reg_value);
			if (ret < 0)
				sysfs_err("ser main regs show err");
		}
	}

	return 0;
}

static ssize_t ser_main_regs_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int i = 0;
	int ret = 0;
	unsigned char dev_addr = 0;
	struct i2c_client *client = NULL;
	struct serdes_data *serdes = NULL;

	client = to_i2c_client(dev);
	serdes = i2c_get_clientdata(client);
	dev_addr = serdes->p_data->ser_addr;
	if (buf == NULL) {
		sysfs_err("buff is null");
		return -EINVAL;
	}
	ret = memset_s(buf, PAGE_SIZE, 0, PAGE_SIZE);
	if (ret != 0) {
		sysfs_err("memset_s error");
		return -EINVAL;
	}
	ret = snprintf_s(buf + strlen(buf), PAGE_SIZE - strlen(buf), PAGE_SIZE - strlen(buf) - 1, "[main_reg_list:]\n");
	if (ret < 0) {
		sysfs_err("ser main_reg_list printf err");
		return -EINVAL;
	}
	ret = main_reg_show(client, dev_addr, buf);
	if (ret < 0) {
		sysfs_err("ser main_reg_show err");
		return -EINVAL;
	}
	ret = snprintf_s(buf + strlen(buf), PAGE_SIZE - strlen(buf), PAGE_SIZE - strlen(buf) - 1, "\n");
	if (ret < 0) {
		sysfs_err("ser main regs show err for last page");
		return -EINVAL;
	}
	sysfs_info("%s\n", buf);

	return strlen(buf);
}

// [reg_addr]:[reg_val]
static ssize_t ser_main_regs_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int ret = 0;
	unsigned int reg_addr = 0;
	unsigned int reg_val = 0;
	char *p = NULL;
	char reg_buf[REG_BUF_LEN];
	struct i2c_client *client = NULL;
	struct serdes_data *serdes = NULL;

	client = to_i2c_client(dev);
	serdes = i2c_get_clientdata(client);
	ret = memset_s(reg_buf, sizeof(reg_buf), 0, REG_BUF_LEN);
	if (ret != 0) {
		sysfs_err("memset_s error");
		return -EINVAL;
	}
	p = strchr(buf, ':');
	if (p == NULL) {
		sysfs_err("err:can not find '!'");
		return -EINVAL;
	}
	ret = memcpy_s(reg_buf, sizeof(reg_buf), buf, p - buf);
	if (ret != 0) {
		sysfs_err("memcpy_s error");
		return -EINVAL;
	}
	ret = kstrtoint(reg_buf, HEXADECIMAL, &reg_addr);
	if (ret != 0) {
		sysfs_err("err: kstrtoint failed!");
		return -EINVAL;
	}

	if (reg_addr > SER_MAIN_REG_MAXCNT) {
		sysfs_err("err:your input [] is too big!");
		return -EINVAL;
	}
	ret = kstrtoint(p + 1, HEXADECIMAL, &reg_val);
	if (ret != 0) {
		sysfs_err("err: kstrtoint failed!");
		return -EINVAL;
	}
	sysfs_info("reg[0x%x] = 0x%x\n", reg_addr, reg_val);
	i2c_write_reg(client->adapter, serdes->p_data->ser_addr, reg_addr, reg_val);

	return count;
}

static ssize_t deser_main_regs_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int i = 0;
	int ret = 0;
	unsigned char reg_value = 0;
	unsigned char dev_addr = 0;
	struct i2c_client *client = NULL;
	struct serdes_data *serdes = NULL;

	client = to_i2c_client(dev);
	serdes = i2c_get_clientdata(client);
	dev_addr = serdes->p_data->des0_addr;
	if (buf == NULL) {
		sysfs_err("buff is null");
		return -EINVAL;
	}
	ret = memset_s(buf, PAGE_SIZE, 0, PAGE_SIZE);
	if (ret != 0) {
		sysfs_err("memset_s error");
		return -EINVAL;
	}
	ret = snprintf_s(buf + strlen(buf), PAGE_SIZE - strlen(buf), PAGE_SIZE - strlen(buf) - 1,
		"[dev_addr 0x%x main_reg_list:]\n", dev_addr);
	if (ret < 0) {
		sysfs_err("deser main_reg_list printf err");
		return -EINVAL;
	}
	ret = main_reg_show(client, dev_addr, buf);
	if (ret < 0) {
		sysfs_err("ser main_reg_show err");
		return -EINVAL;
	}
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
	struct serdes_data *serdes = NULL;

	client = to_i2c_client(dev);
	serdes = i2c_get_clientdata(client);
	ret = memset_s(reg_buf, sizeof(reg_buf), 0, REG_BUF_LEN);
	if (ret != 0) {
		sysfs_err("memset_s error");
		return -EINVAL;
	}
	p = strchr(buf, ':');
	if (p == NULL) {
		sysfs_err("err:can not find '!'");
		return -EINVAL;
	}
	ret = memcpy_s(reg_buf, sizeof(reg_buf), buf, p - buf);
	if (ret != 0)
		sysfs_err("memcpy_s error");
	ret = kstrtoint(reg_buf, HEXADECIMAL, &reg_addr);
	if (ret != 0) {
		sysfs_err("err: kstrtoint failed!");
		return -EINVAL;
	}
	if (reg_addr > SER_MAIN_REG_MAXCNT) {
		sysfs_err("err:your input [] is too big!");
		return -EINVAL;
	}
	ret = kstrtoint(p + 1, HEXADECIMAL, &reg_val);
	if (ret != 0) {
		sysfs_err("err: kstrtoint failed!");
		return -EINVAL;
	}
	sysfs_info("reg[0x%x] = 0x%x", reg_addr, reg_val);
	if (serdes->p_data->des0_addr != 0)
		i2c_write_reg(client->adapter, serdes->p_data->des0_addr, reg_addr, reg_val);
	if (serdes->p_data->des1_addr != 0)
		i2c_write_reg(client->adapter, serdes->p_data->des1_addr, reg_addr, reg_val);

	return count;
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

	client = to_i2c_client(dev);
	serdes_addr = get_serdes_addr();
	page_num = get_page_num();
	page_num_reg = (page_num << 0x02) | 0x03;
	sysfs_info("enter get_serdes_page_register, page[%d]", page_num);
	/* config serdes register */
	i2c_write_reg(client->adapter, serdes_addr, 0x40, page_num_reg);
	i2c_write_reg(client->adapter, serdes_addr, 0x41, 0x00);
	ret = memset_s(buf, PAGE_SIZE, 0, PAGE_SIZE);
	if (ret != 0) {
		sysfs_err("memset_s error");
		return -EINVAL;
	}
	ret = snprintf_s(buf + strlen(buf), PAGE_SIZE - strlen(buf), PAGE_SIZE - strlen(buf) - 1,
		"addr[0x%x]\n[page%d_reg_list:]\n", serdes_addr, page_num);
	if (ret < 0) {
		sysfs_err("get_serdes_page_register page[%d]_reg_list printf err", page_num);
		return -EINVAL;
	}
	for (i = 0; i <= SER_MAIN_REG_MAXCNT; i++) {
		i2c_read_reg(client->adapter, serdes_addr, 0x42, &reg_value);
		if ((i + 1) % COLUMN_NUM == 0) {
			ret = snprintf_s(buf + strlen(buf), PAGE_SIZE - strlen(buf),
				PAGE_SIZE - strlen(buf) - 1, "0x%02x--0x%02x\n", i, reg_value);
			if (ret < 0)
				sysfs_err("get_serdes_page_register err for column");
		} else {
			ret = snprintf_s(buf + strlen(buf), PAGE_SIZE - strlen(buf),
				PAGE_SIZE - strlen(buf) - 1, "0x%02x--0x%02x  ", i, reg_value);
			if (ret < 0)
				sysfs_err("get_serdes_page_register err");
		}
	}
	ret = snprintf_s(buf + strlen(buf), PAGE_SIZE - strlen(buf), PAGE_SIZE - strlen(buf) - 1, "\n");
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
	if (ret != 0) {
		sysfs_err("memset_s error");
		return -EINVAL;
	}
	p = strchr(buf, ':');
	if (p == NULL) {
		sysfs_err("err:can not find '!'");
		return -EINVAL;
	}
	ret = memcpy_s(reg_buf, sizeof(reg_buf), buf, p - buf);
	if (ret != 0) {
		sysfs_err("memcpy_s error");
		return -EINVAL;
	}
	ret = kstrtoint(reg_buf, HEXADECIMAL, &serdes_addr);
	if (ret != 0) {
		sysfs_err("err: kstrtoint failed!");
		return -EINVAL;
	}
	ret = kstrtoint(p + 1, HEXADECIMAL, &page_num);
	if (ret != 0) {
		sysfs_err("err: kstrtoint failed!");
		return -EINVAL;
	}
	set_serdes_addr((unsigned char)(serdes_addr & 0xff));
	set_page_num((unsigned char)(page_num & 0xff));
	sysfs_info("page_num[0x%x]", page_num);

	return count;
}

static ssize_t get_apb_value(struct device *dev, struct device_attribute *attr, char *buf)
{
	int i = 0;
	int ret = -1;
	unsigned char reg_value = 0;
	unsigned char addr16b_lsb = 0;
	unsigned char addr16b_msb = 0;
	struct i2c_client *client = NULL;
	struct serdes_data *serdes = NULL;
	client = to_i2c_client(dev);
	serdes = i2c_get_clientdata(client);
	addr16b_lsb = g_apb_addr & 0xFF;
	addr16b_msb = (g_apb_addr & 0xFF00) >> 8;
	sysfs_info("enter get_apb_value, g_apb_addr 0x%x, addr16b_msb 0x%x, addr16b_lsb 0x%x",
		g_apb_addr, addr16b_msb, addr16b_lsb);
	/* config serdes register */
	i2c_write_reg(client->adapter, serdes->p_data->ser_addr, APB_ADR0, addr16b_lsb);
	i2c_write_reg(client->adapter, serdes->p_data->ser_addr, APB_ADR1, addr16b_msb);
	i2c_write_reg(client->adapter, serdes->p_data->ser_addr, APB_CTL, 0x03);
	ret = memset_s(buf, PAGE_SIZE, 0, PAGE_SIZE);
	if (ret != 0) {
		sysfs_err("memset_s error");
		return -EINVAL;
	}
	ret = snprintf_s(buf + strlen(buf), PAGE_SIZE - strlen(buf), PAGE_SIZE - strlen(buf) - 1,
		"i2c addr[0x%x]\n apd_addr[0x%x %x]\n", serdes->p_data->ser_addr, addr16b_msb, addr16b_lsb);
	if (ret < 0) {
		sysfs_err("get_apb_value apd_addr[0x%x%x] printf err", addr16b_msb, addr16b_lsb);
		return -EINVAL;
	}
	i2c_read_reg(client->adapter, serdes->p_data->ser_addr, APB_DATA0, &reg_value);
	ret = snprintf_s(buf + strlen(buf), PAGE_SIZE - strlen(buf),
		PAGE_SIZE - strlen(buf) - 1, "reg 0x%x value byte0--0x%02x\n", APB_DATA0, reg_value);
	if (ret < 0) {
		sysfs_err("get_apb_value err");
		return -EINVAL;
	}
	i2c_read_reg(client->adapter, serdes->p_data->ser_addr, APB_DATA1, &reg_value);
	ret = snprintf_s(buf + strlen(buf), PAGE_SIZE - strlen(buf),
		PAGE_SIZE - strlen(buf) - 1, "reg 0x%x value byte1--0x%02x\n", APB_DATA1, reg_value);
	if (ret < 0) {
		sysfs_err("get_apb_value err");
		return -EINVAL;
	}
	i2c_read_reg(client->adapter, serdes->p_data->ser_addr, APB_DATA2, &reg_value);
	ret = snprintf_s(buf + strlen(buf), PAGE_SIZE - strlen(buf),
		PAGE_SIZE - strlen(buf) - 1, "reg 0x%x value byte2--0x%02x\n", APB_DATA2, reg_value);
	if (ret < 0) {
		sysfs_err("get_apb_value err");
		return -EINVAL;
	}
	i2c_read_reg(client->adapter, serdes->p_data->ser_addr, APB_DATA3, &reg_value);
	ret = snprintf_s(buf + strlen(buf), PAGE_SIZE - strlen(buf),
		PAGE_SIZE - strlen(buf), "reg 0x%x value byte2--0x%02x\n", APB_DATA3, reg_value);
	if (ret < 0) {
		sysfs_err("get_apb_value err");
		return -EINVAL;
	}
	ret = snprintf_s(buf + strlen(buf), PAGE_SIZE - strlen(buf), PAGE_SIZE - strlen(buf) - 1, "\n");
	if (ret < 0)
		sysfs_err("get_apb_value err for last page");
	sysfs_info("%s\n", buf);
	sysfs_info("end get_apb_value ret = %d", ret);

	return strlen(buf);
}

static ssize_t set_apb_addr(struct device *dev, struct device_attribute *attr,
	const char *buf, size_t count)
{
	unsigned int page_num = 0;
	int ret = -1;

	ret = kstrtoint(buf, HEXADECIMAL, &page_num);
	if (ret != 0) {
		sysfs_err("err: kstrtoint failed!");
		return -EINVAL;
	}
	set_g_apb_addr((page_num & 0xffff));
	sysfs_info("apb_addr[0x%x]", page_num);

	return count;
}

static ssize_t ti983_video_reset_test(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = NULL;
	struct serdes_data *serdes = NULL;

	sysfs_info("enter ti983_video_reset_test");
	client = to_i2c_client(dev);
	serdes = i2c_get_clientdata(client);
	i2c_write_reg(client->adapter, serdes->p_data->ser_addr, 0x48, 0x01);
	i2c_write_reg(client->adapter, serdes->p_data->ser_addr, 0x49, 0x54);
	i2c_write_reg(client->adapter, serdes->p_data->ser_addr, 0x4a, 0x00);
	i2c_write_reg(client->adapter, serdes->p_data->ser_addr, 0x4b, 0x01);
	i2c_write_reg(client->adapter, serdes->p_data->ser_addr, 0x4c, 0x00);
	i2c_write_reg(client->adapter, serdes->p_data->ser_addr, 0x4d, 0x00);
	i2c_write_reg(client->adapter, serdes->p_data->ser_addr, 0x4e, 0x00);
	sysfs_info("exit ti983_video_reset_test");

	return strlen(buf);
}

static DEVICE_ATTR(ser_pattern_port, 0640, ser_pattern_test_vp01, NULL);
static DEVICE_ATTR(deser_pattern_port, 0640, deser_pattern_test, NULL);
static DEVICE_ATTR(ser_main_regs, 0640, ser_main_regs_show, ser_main_regs_store);
static DEVICE_ATTR(deser_main_regs, 0640, deser_main_regs_show, deser_main_regs_store);
static DEVICE_ATTR(dump_page_reg_value, 0640, get_serdes_page_register, set_serdes_page);
static DEVICE_ATTR(read_apb_reg_value, 0640, get_apb_value, set_apb_addr);
static DEVICE_ATTR(video_reset_test, 0640, ti983_video_reset_test, NULL);

static struct attribute *g_dp_sysfs_fs_ops[] = {
	&dev_attr_ser_pattern_port.attr,
	&dev_attr_deser_pattern_port.attr,
	&dev_attr_ser_main_regs.attr,
	&dev_attr_deser_main_regs.attr,
	&dev_attr_dump_page_reg_value.attr,
	&dev_attr_read_apb_reg_value.attr,
	&dev_attr_video_reset_test.attr,
	NULL,
};

struct attribute_group sysfs_attr_group_dp = {
	.attrs = g_dp_sysfs_fs_ops,
};

int lcd_sysfs_init_for_dp(struct i2c_client *client)
{
	int ret = LCD_OK;
	ret = sysfs_create_group(&(client->dev.kobj), &sysfs_attr_group_dp);
	if (ret)
		sysfs_err("create fs node fail");
	return ret;
}
