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

#include "upgrade_channel_manage.h"
#include "ti981_common.h"
#include "lcd_sysfs_for_dsi.h"

#define HTOTAL_HIGH_BITS 0x11

#define FRAME_CNT_END 100

static struct ti981_serdes *serdes_data = NULL;
bool need_hot_reinit = true;

void set_hot_reinit_flag(bool flag)
{
	need_hot_reinit = flag;
}

bool get_hot_reinit_flag(void)
{
	return need_hot_reinit;
}

static int panel_set_backlight(struct platform_device *pdev, uint32_t bl_level)
{
	return 0;
}

struct i2c_client *get_ti981_client(void)
{
	return serdes_data->client;
}

struct ti981_serdes *get_serdes(void)
{
	return serdes_data;
}

static ssize_t ser_pattern_test_port0(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = NULL;
	client = to_i2c_client(dev);

	cdc_info("enter");
	i2c_write_reg(client->adapter, DS90UB981_I2C_7BIT_ADDR, 0x40, 0x30);
	i2c_write_reg(client->adapter, DS90UB981_I2C_7BIT_ADDR, 0x41, 0x29);
	i2c_write_reg(client->adapter, DS90UB981_I2C_7BIT_ADDR, 0x42, 0x08);
	i2c_write_reg(client->adapter, DS90UB981_I2C_7BIT_ADDR, 0x41, 0x28);
	i2c_write_reg(client->adapter, DS90UB981_I2C_7BIT_ADDR, 0x42, 0x95);
	i2c_write_reg(client->adapter, DS90UB981_I2C_7BIT_ADDR, 0x41, 0x69);
	i2c_write_reg(client->adapter, DS90UB981_I2C_7BIT_ADDR, 0x42, 0x08);
	i2c_write_reg(client->adapter, DS90UB981_I2C_7BIT_ADDR, 0x41, 0x68);
	i2c_write_reg(client->adapter, DS90UB981_I2C_7BIT_ADDR, 0x42, 0x95);
	cdc_info("finish");
	return strlen(buf);
}

static ssize_t deser_pattern_test_port0(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = NULL;
	client = to_i2c_client(dev);
	cdc_info("enter");
	cdc_info("finish");
	return strlen(buf);
}

static ssize_t ser_main_regs_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int i = 0;
	unsigned char reg_value = 0;
	struct i2c_client *client = NULL;

	int ret = 0;

	client = to_i2c_client(dev);
	if (buf == NULL) {
		cdc_err("buff is null\n");
		return -EINVAL;
	}
	ret = memset_s(buf, PAGE_SIZE, 0, PAGE_SIZE);
	if (ret != 0) {
		cdc_err("memset_s error");
		return -ret;
	}

	ret = snprintf_s(buf + strlen(buf), PAGE_SIZE - strlen(buf), PAGE_SIZE - strlen(buf), "[main_reg_list:]\n");
	if (ret < 0)
		cdc_err("ser main_reg_list printf err");

	for (i = 0; i <= SER_MAIN_REG_MAXCNT; i++) {
		i2c_read_reg(client->adapter, DS90UB981_I2C_7BIT_ADDR, i, &reg_value);
		if ((i + 1) % COLUMN_NUM == 0) {
			ret = snprintf_s(buf + strlen(buf), PAGE_SIZE - strlen(buf), PAGE_SIZE - strlen(buf),
				"0x%02x--0x%02x\n", i, reg_value);
			if (ret < 0)
				cdc_err("ser main regs show err for column");
		} else {
			ret = snprintf_s(buf + strlen(buf), PAGE_SIZE - strlen(buf), PAGE_SIZE - strlen(buf),
				"0x%02x--0x%02x", i, reg_value);
			if (ret < 0)
				cdc_err("ser main regs show err");
		}
	}
	ret = snprintf_s(buf + strlen(buf), PAGE_SIZE - strlen(buf), PAGE_SIZE - strlen(buf), "\n");
	if (ret < 0)
		cdc_err("ser main regs show err for last page");
	cdc_info("%s\n", buf);
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

	client = to_i2c_client(dev);
	ret = memset_s(reg_buf, sizeof(reg_buf), 0, REG_BUF_LEN);
	if (ret != 0) {
		cdc_err("memset_s error");
		return -ret;
	}

	p = strchr(buf, ':');
	if (p == NULL) {
		cdc_err("err:can not find '!'\n");
		return -EINVAL;
	}
	ret = memcpy_s(reg_buf, sizeof(reg_buf), buf, p - buf);
	if (ret != 0)
		cdc_err("memcpy_s error");

	ret = kstrtoint(reg_buf, HEXADECIMAL, &reg_addr);
	if (ret != 0) {
		cdc_err("err: kstrtoint failed!\n");
		return -EINVAL;
	}

	if (reg_addr > SER_MAIN_REG_MAXCNT) {
		cdc_err("err:your input [] is too big!\n");
		return -EINVAL;
	}

	ret = kstrtoint(p + 1, HEXADECIMAL, &reg_val);
	if (ret != 0) {
		cdc_err("err: kstrtoint failed!\n");
		return -EINVAL;
	}

	cdc_info("reg[0x%x] = 0x%x\n", reg_addr, reg_val);
	i2c_write_reg(client->adapter, DS90UB981_I2C_7BIT_ADDR, reg_addr, reg_val);
	return count;
}

static ssize_t deser_main_regs_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int i = 0;
	unsigned char reg_value = 0;
	struct i2c_client *client = NULL;
	int ret = 0;

	client = to_i2c_client(dev);
	if (buf == NULL) {
		cdc_err("buff is null\n");
		return -EINVAL;
	}
	ret = memset_s(buf, PAGE_SIZE, 0, PAGE_SIZE);
	if (ret != 0) {
		cdc_err("memset_s error");
		return -ret;
	}

	ret = snprintf_s(buf + strlen(buf), PAGE_SIZE - strlen(buf), PAGE_SIZE - strlen(buf), "[main_reg_list:]\n");
	if (ret < 0)
		cdc_err("deser main_reg_list printf err");

	for (i = 0; i <= SER_MAIN_REG_MAXCNT; i++) {
		i2c_read_reg(client->adapter, DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, i, &reg_value);
		if ((i + 1) % COLUMN_NUM == 0) {
			ret = snprintf_s(buf + strlen(buf), PAGE_SIZE - strlen(buf), PAGE_SIZE - strlen(buf),
				"0x%02x--0x%02x\n", i, reg_value);
			if (ret < 0)
				cdc_err("dser main regs show err for column");
		} else {
			ret = snprintf_s(buf + strlen(buf), PAGE_SIZE - strlen(buf), PAGE_SIZE - strlen(buf),
				"0x%02x--0x%02x", i, reg_value);
			if (ret < 0)
				cdc_err("dser main regs show err");
		}
	}

	ret = snprintf_s(buf + strlen(buf), PAGE_SIZE - strlen(buf), PAGE_SIZE - strlen(buf), "\n");
	if (ret < 0)
		cdc_err("dser main regs show err for last page");
	cdc_info("%s\n", buf);
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

	client = to_i2c_client(dev);
	ret = memset_s(reg_buf, sizeof(reg_buf), 0, REG_BUF_LEN);
	if (ret != 0) {
		cdc_err("memset_s error");
		return -ret;
	}

	p = strchr(buf, ':');
	if (p == NULL) {
		cdc_err("err:can not find '!'\n");
		return -EINVAL;
	}
	ret = memcpy_s(reg_buf, sizeof(reg_buf), buf, p - buf);
	if (ret != 0)
		cdc_err("memcpy_s error");

	ret = kstrtoint(reg_buf, HEXADECIMAL, &reg_addr);
	if (ret != 0) {
		cdc_err("err: kstrtoint failed!\n");
		return -EINVAL;
	}

	if (reg_addr > SER_MAIN_REG_MAXCNT) {
		cdc_err("err:your input [] is too big!\n");
		return -EINVAL;
	}

	ret = kstrtoint(p + 1, HEXADECIMAL, &reg_val);
	if (ret != 0) {
		cdc_err("err: kstrtoint failed!\n");
		return -EINVAL;
	}

	cdc_info("reg[0x%x] = 0x%x\n", reg_addr, reg_val);
	i2c_write_reg(client->adapter, DS90UH988_I2C_7BIT_ADDR_METER_ALIAS, reg_addr, reg_val);
	return count;
}

static ssize_t ser_brightness_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int ret = 0;
	uint32_t brightness = 0;
	struct ti981_serdes *serdes = NULL;

	serdes = i2c_get_clientdata(to_i2c_client(dev));
	ret = kstrtoint(buf, DECIMAL, &brightness);
	if (ret != 0) {
		cdc_err("%s err: kstrtoint failed!", buf);
		return -EINVAL;
	}
	if (brightness < 0)
		brightness = 0;
	else if (brightness > MAX_BACKLIGHT_VALUE)
		brightness = MAX_BACKLIGHT_VALUE;

	ret = set_backlight_to_smart_panel(serdes->client, brightness);
	if (ret < 0)
		cdc_err("meter panel set backlight brightness to %d failed", brightness);
	serdes->ivi_last_backlight = brightness;
	cdc_info("force set backlight brightness to %d", brightness);
	return count;
}

static ssize_t ser_brightness_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int ret = -1;
	int brightness = 0;
	struct ti981_serdes *serdes = NULL;

	serdes = i2c_get_clientdata(to_i2c_client(dev));
	if (buf == NULL) {
		cdc_err("buff is null\n");
		return -EINVAL;
	}
	ret = memset_s(buf, PAGE_SIZE, 0, PAGE_SIZE);
	if (ret != 0) {
		cdc_err("memset_s error");
		return -ret;
	}

	brightness = get_backlight_from_smart_panel(serdes->client);
	if (brightness < 0)
		cdc_err("get backlight from mcu failed");
	ret = snprintf_s(buf + strlen(buf), PAGE_SIZE - strlen(buf), PAGE_SIZE - strlen(buf), "ser_backlight: %d\n",
		brightness);
	if (ret < 0)
		cdc_err("ser brightness show failed, errcode %d", ret);
	return strlen(buf);
}

static int dsi0_sys_init(struct ti981_serdes *serdes)
{
	int ret = -1;
	struct attribute_group sysfs_attr_group_upgrade;

	ret = lcd_sysfs_init_for_dsi(serdes->client);
	if (ret)
		cdc_err("primary: dev_name = %s+\n", dev_name(&serdes->client->dev));

	cdc_info("lcd_sysfs primary: dev_name = %s+\n", dev_name(&serdes->client->dev));
	sysfs_attr_group_upgrade = get_sysfs_attr_group_upgrade();
	ret = sysfs_create_group(&(serdes->client->dev.kobj), &sysfs_attr_group_upgrade);
	if (ret)
		cdc_err("sysfs group upgrade creation failed, ret=%d", ret);
	cdc_info("sysfs group upgrade creation succ, ret=%d !", ret);
	return 0;
}

int ti981_config_dsi_disable(struct ti981_serdes *serdes)
{
	/* Disable DSI */
	int ret;
	unsigned char reg_value = 0;
	ret = i2c_read_reg(serdes->client->adapter, DS90UB981_I2C_7BIT_ADDR, 0x02, &reg_value);
	if (ret)
		cdc_err("read data from 981 fail ret %d", ret);
	ret = i2c_write_reg(serdes->client->adapter, DS90UB981_I2C_7BIT_ADDR, 0x02, (reg_value | 0x08));
	if (ret)
		cdc_err("write data to 981 0x02 fail ret %d", ret);
	return 0;
}
int ti981_config_i2c_passthrough(struct ti981_serdes *serdes)
{
	int ret;
	unsigned char reg_value = 0;

	ret = i2c_write_reg(serdes->client->adapter, DS90UB981_I2C_7BIT_ADDR, 0x2d, 0x01);
	if (ret)
		cdc_err("write data to 981 0x07 fail ret %d", ret);
	ret = i2c_read_reg(serdes->client->adapter, DS90UB981_I2C_7BIT_ADDR, 0x07, &reg_value);
	if (ret)
		cdc_err("read data from 981 fail ret %d", ret);
	ret = i2c_write_reg(serdes->client->adapter, DS90UB981_I2C_7BIT_ADDR, 0x07, (reg_value | 0x08));
	if (ret)
		cdc_err("write data to 981 0x07 fail ret %d", ret);

	return 0;
}
int ti981_config_dsi_enable(struct ti981_serdes *serdes)
{
	int ret;
	unsigned char reg_value = 0;
	/* Enable DSI */
	ret = i2c_read_reg(serdes->client->adapter, DS90UB981_I2C_7BIT_ADDR, 0x02, &reg_value);
	if (ret)
		cdc_err("read data from 981 fail ret %d", ret);
	ret = i2c_write_reg(serdes->client->adapter, DS90UB981_I2C_7BIT_ADDR, 0x02, (reg_value & 0xf7));
	if (ret)
		cdc_err("write data to 981 0x02 fail ret %d", ret);
	return 0;
}

static int ti981_serdes_enable(struct ti981_serdes *serdes)
{
	int ret = -1;
	cdc_info("enter");

	if (serdes->serdes_power_on) {
		ret = serdes->serdes_power_on(serdes);
		if (ret) {
			cdc_err("serdes powr on error");
			return ret;
		}
	}
	if (serdes->port && serdes->port->lcd_power_on) {
		ret = serdes->port->lcd_power_on(serdes);
		if (ret) {
			cdc_err("lcd powr on error");
			return ret;
		}
	}

	/* config 981 register */
	ret = serdes->serdes_config_slave_addr(serdes);
	if (ret) {
		cdc_err("serdes_config_slave_addr error!");
		return -1;
	}
	if (serdes->domain == RDA_DOMAIN) {
		/* dsi config */
		// disable DSI
		ret = ti981_config_dsi_disable(serdes);
		if (serdes->serdes_config_dsi0)
			serdes->serdes_config_dsi0(serdes);
		if (serdes->serdes_config_dsi1)
			serdes->serdes_config_dsi1(serdes);
	}
	// i2c pass through
	ret = ti981_config_i2c_passthrough(serdes);
	if (serdes->domain == RDA_DOMAIN)
		ret = ti981_config_dsi_enable(serdes);

	serdes->serdes_check_des_devid(serdes);

	if (serdes->port && serdes->port->port_lock_check)
		serdes->port->port_lock_check(serdes);

	mdelay(2);
	if (serdes->port && serdes->port->lcd_deser_config) {
		ret = serdes->port->lcd_deser_config(serdes);
		if (ret < 0) {
			cdc_err("lcd_deser_config error");
			return ret;
		}
	}
	return ret;
}

static int dsi0_resume_handle(struct ti981_serdes *serdes)
{
	serdes->serdes_config_power_gpio(serdes);
	return ti981_serdes_enable(serdes);
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

static int dsi_vp1_sync(uint32_t idx)
{
	int ret = -1;
	struct i2c_client* client = NULL;
	unsigned char reg_value = 0;

	client = get_ti981_client();
	if (client == NULL) {
		cdc_err("i2c client null");
		return ret;
	}
	ret = i2c_read_reg(client->adapter, DS90UB981_I2C_7BIT_ADDR, 0x45, &reg_value);
	if (ret < 0) {
		cdc_err("read data from 981 error! reg = 0x45");
		return ret;
	}

	if (reg_value & 0x02) {
		cdc_info("the vp1 has been synchronized");
		i2c_write_reg(client->adapter, DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, 0x01, 0x01);
		return ret;
	}

	cdc_info("vp1 not sync");
	return -1;
}

int notify_ser_dsi_vp_sync(uint32_t idx)
{
	int ret = -1;
	static uint32_t frame_cnt = 0;
	struct i2c_client* client = NULL;

	client = get_ti981_client();
	if (client == NULL) {
		cdc_err("i2c client null");
		return ret;
	}
	if(frame_cnt == 0)
		enable_vp1_sync(client);
	frame_cnt++;
	cdc_info("frame_cnt:%d\n", frame_cnt);
	if (frame_cnt < FRAME_CNT_END) {
		ret = dsi_vp1_sync(idx);
		cdc_info("retval:%d\n", ret);
	}

	if (ret < 0) {
		if (frame_cnt < FRAME_CNT_END) {
			cdc_info("frames cnt:%d, vp sync retry\n", frame_cnt);
			return -1;
		} else {
			cdc_info("vp sync fail result:%d,frames cnt reached:%d\n", ret, frame_cnt);
			frame_cnt = 0;
			return 0;
		}
	}

	frame_cnt = 0;
	cdc_info("vp sync success\n");
	return ret;
}

static int get_lock_state_from_deser(struct i2c_client * client, unsigned char *lock_sts)
{
	int ret = -1;
	unsigned char general_sts;
	ret = i2c_read_reg(client->adapter, DS90UH988_I2C_7BIT_ADDR_SMART_ALIAS, GENERAL_PURPOSE_STATUS, lock_sts);
	if (ret < 0) {
		cdc_err("read data from 988 error! reg = 0x54");
		i2c_write_reg(client->adapter, DS90UB981_I2C_7BIT_ADDR, 0x2d, 0x12);
		i2c_read_reg(client->adapter, DS90UB981_I2C_7BIT_ADDR, TI981_GENERAL_STS, &general_sts);
		return ret;
	}
	cdc_info("read data from 988 success! reg = 0x54 data = 0x%x", *lock_sts);
	return ret;
}

static void clear_link_lost(struct ti981_serdes *serdes)
{
	int ret = -1;
	unsigned char reg_value = 0;
	struct i2c_client *client = NULL;

	client = serdes->client;
	ret = i2c_read_reg(client->adapter, DS90UB981_I2C_7BIT_ADDR, 0x02, &reg_value);
	if (ret < 0) {
		cdc_err("read data from 981 error! reg = 0x02");
		return;
	}

	ret = i2c_write_reg(client->adapter, DS90UB981_I2C_7BIT_ADDR, 0x02, reg_value | 0x20);
	if (ret < 0)
		cdc_err("write data to 981 error! reg = 0x02");

	ret = i2c_write_reg(client->adapter, DS90UB981_I2C_7BIT_ADDR, 0x02, reg_value);
	if (ret < 0)
		cdc_err("write data to 981 error! reg = 0x02");
	return;
}

static void smart_panel_hot_reinit(struct ti981_serdes *serdes)
{
	int ret = -1;
	if (serdes->port == NULL) {
		cdc_err("serdes port is null, smart_panel_hot_reinit failed");
		return;
	}

	lcd_wake_power_off(serdes);
	serdes->lvds_link_status = MIPI_LVDS_NOT_LINKED;
	msleep(LCD_WAKE_DELAY_TIME);
	if (serdes->port->lcd_power_on) {
		ret = serdes->port->lcd_power_on(serdes);
		if (ret < 0) {
			cdc_err("lcd powr on error, smart_panel_hot_reinit failed");
			return;
		}
	}

	if (serdes->port->port_lock_check) {
		ret = serdes->port->port_lock_check(serdes);
		if (ret < 0) {
			cdc_err("port_lock_check error, smart_panel_hot_reinit failed");
			return;
		}
	}

	mdelay(2);
	if (serdes->port->lcd_deser_config) {
		ret = serdes->port->lcd_deser_config(serdes);
		if (ret < 0) {
			cdc_err("lcd_deser_config error, smart_panel_hot_reinit failed");
			return;
		}
	}
	serdes->lvds_link_status = MIPI_LVDS_LINKED;
	clear_link_lost(serdes);
	cdc_info("smart_panel_hot_reinit success");
	return;
}

static void check_hot_reinit(struct ti981_serdes *serdes)
{
	if (!get_hot_reinit_flag()) {
		cdc_info("do not need hot reinit");
		return;
	}
	int ret = -1;
	unsigned char lock_sts = 0;
	struct i2c_client *client = NULL;

	client = serdes->client;
	if (serdes->lvds_link_status != MIPI_LVDS_LINKED)
		smart_panel_hot_reinit(serdes);

	ret = get_lock_state_from_deser(client, &lock_sts);
	if (ret < 0) {
		smart_panel_hot_reinit(serdes);
		return;
	}

	if (!(lock_sts & GENERAL_PURPOSE_STATUS_LOCK))
		smart_panel_hot_reinit(serdes);
	return;
}

static void check_vp_sync(struct ti981_serdes *serdes)
{
	int ret = -1;
	unsigned char reg_value = 0;
	struct i2c_client *client = serdes->client;
	bool enable_vp1_flag = false;

	ret = i2c_read_reg(client->adapter, DS90UB981_I2C_7BIT_ADDR, 0x45, &reg_value);
	if (ret < 0)
		cdc_err("read data from 981 error! reg = 0x45");

	/* enable vp0 vp1 until synchronization is on */
	if (!(reg_value & 0x02) && !enable_vp1_flag) {
		ret = enable_vp1_sync(client);
		if (ret < 0)
			cdc_err("enable vp1 error!");
	} else {
		enable_vp1_flag = true;
	}

	return;
}

static int thread_smartpanel_monitor(void *thread_data)
{
	struct ti981_serdes *serdes = NULL;
	int ret = -1;
	unsigned char reg_value = 0;
	struct i2c_client *client = NULL;

	serdes = (struct ti981_serdes *)thread_data;
	client = serdes->client;
	cdc_info("enter smart monitor thread");
	while (1) {
		if (serdes->mipi_resumed == MIPI_STATUS_RESUME) {
			ret = dsi0_resume_handle(serdes);
			if (ret < 0) {
				cdc_err("resume failed, lvds_init_status not linked, need hot_reinit");
			} else {
				cdc_err("resume success");
				serdes->lvds_link_status = MIPI_LVDS_LINKED;
				serdes->mipi_resumed = MIPI_STATUS_ACTIVE;
			}
		}
		check_vp_sync(serdes);
		check_hot_reinit(serdes);
		if (kthread_should_stop()) {
			cdc_err("smartpanel monitor has been stoped!");
			break;
		}
		ssleep(SLEEP_TIME_S);
	}

	return 0;
}

static int ti981_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int ret;
	cdc_info("enter ti981_i2c_probe");
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C))
		cdc_err("get i2c adapter fail");

	serdes_data = devm_kzalloc(&client->dev, sizeof(struct ti981_serdes), GFP_KERNEL);
	if (!serdes_data) {
		cdc_err("ti981_serdes kzalloc failed");
		return -ENOMEM;
	}
	struct ti981_serdes *serdes = NULL;
	serdes = serdes_data;
	serdes->client = client;
	serdes->domain = NON_RDA_DOMAIN;
	i2c_set_clientdata(client, serdes);

	ret = ti981_serdes_init(serdes);
	return_value_if_run_error(ret < 0, ret, "ti981_serdes_init failed");

	ret = ti981_config_dsi(serdes);
	return_value_if_run_error(ret < 0, ret, "ti981_config_dsi failed");

	ret = ti981_config_link_port(serdes);
	return_value_if_run_error(ret < 0, ret, "ti981_config_link_port failed");

	dsi0_sys_init(serdes);
	pr_info("primary: dev_name = %s+.\n", dev_name(&client->dev));
	ret = serdes->serdes_config_power_gpio(serdes);
	if (ret) {
		cdc_err("serdes config power gpio error");
		kfree(serdes->port);
		serdes->port = NULL;
		return ret;
	}
	ret = ti981_serdes_enable(serdes);
	if (ret < 0) {
		cdc_err("panel init failed, link status is err");
		serdes->lvds_link_status = MIPI_LVDS_NOT_LINKED;
	} else {
		cdc_info("panel init success, link status is ok");
		serdes->lvds_link_status = MIPI_LVDS_LINKED;
	}

	return 0;
}

static int ti981_pm_suspend(struct device *dev)
{
	int ret = 0;
	cdc_info("enter");
	struct ti981_serdes *serdes = NULL;

	serdes = i2c_get_clientdata(to_i2c_client(dev));
	serdes->lvds_link_status = MIPI_LVDS_CAB_NOT_LINKED;
	serdes->mipi_resumed = MIPI_STATUS_SUSPEND;

	ret = gpio_direction_output(serdes->port->gpio_lcd_wake, 0);
	if (ret != 0)
		cdc_err("gpio_lcd_wake direction out 0 fail\n");
	gpio_free(serdes->port->gpio_lcd_wake);
	cdc_info("finish");
	return ret;
}

static int ti981_pm_resume(struct device *dev)
{
	int ret = -1;
	struct ti981_serdes *serdes = NULL;

	cdc_info("enter");
	serdes = i2c_get_clientdata(to_i2c_client(dev));

	ret = ti981_serdes_enable(serdes);
	if (ret < 0) {
		cdc_err("panel init failed, link status is err");
		serdes->lvds_link_status = MIPI_LVDS_NOT_LINKED;
	} else {
		cdc_info("panel init success, link status is ok");
		serdes->mipi_resumed = MIPI_STATUS_ACTIVE;
		serdes->lvds_link_status = MIPI_LVDS_LINKED;
	}
	cdc_info("finish");
	return 0;
}

static const struct i2c_device_id ti981_id[] = {
	{ TI981_I2C_NAME_DSI0, 0 },
	{}
};

static const struct dev_pm_ops ti981_pm_ops = { SET_SYSTEM_SLEEP_PM_OPS(ti981_pm_suspend, ti981_pm_resume) };

static const struct of_device_id ti981_match_table[] = {
	{
		.compatible = DTS_COMP_DSI0_SMART_PANEL,
		.data = NULL,
	},
	{},
};

static struct i2c_driver ti981_i2c_driver = {
	.probe = ti981_i2c_probe,
	.remove = NULL,
	.id_table = ti981_id,
	.driver = {
		.name = TI981_I2C_NAME_DSI0,
		.owner = THIS_MODULE,
		.of_match_table = ti981_match_table,
		.pm = &ti981_pm_ops,
	},
};

static int __init ti981_dsi0_init(void)
{
	int ret = 0;
	cdc_info("enter ti981_dsi0_init");
	ret = i2c_add_driver(&ti981_i2c_driver);
	if (ret) {
		cdc_err("dsi0 i2c_add_driver fail, error=%d\n", ret);
		return ret;
	}
	return ret;
}

late_initcall(ti981_dsi0_init);
