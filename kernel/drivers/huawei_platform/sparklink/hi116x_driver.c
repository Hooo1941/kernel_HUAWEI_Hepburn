/*
 * Huawei hi1162 Driver
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/of.h>
#include <linux/vmalloc.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/pm_wakeup.h>
#include <linux/notifier.h>
#include <linux/fb.h>
#include "hi116x_fwupdate.h"
#include "hi116x_driver.h"

static struct hi1162_device_info *g_hi1162_info = NULL;
static int g_hi1162_state = SPARKLINK_STATE_UPDATING;
static int g_chipid_state = SPARKLINK_CHIPID_FAIL; /* 1-read chipid success; 0-default */
static int g_sf_reset_state = SF_RESET_NO_SUPPORT;

#define arry_size(a) (sizeof(a) / sizeof((a)[0]))

void wakeup_116x_before_i2c_ops(struct hi1162_device_info *di)
{
	if (!di)
		return;

#ifdef CONFIG_FACTORY_MODE
	/* no need trigger wakeup_gpio */
	return;
#endif
	/* GPIO_332_SOC_WAKEUP_HI1162 GPIO */
	gpio_set_value(di->wakeup_gpio, 1);
	msleep(WAKEUP_DELAY_TIME);
	gpio_set_value(di->wakeup_gpio, 0);
	msleep(WAKEUP_DELAY_TIME);
	sparklink_infomsg("trigger wakeup_gpio\n");
}

int hi1162_check_id(struct hi1162_device_info *di)
{
	int ret = 0;

	if (!di)
		return -EINVAL;

	wakeup_116x_before_i2c_ops(di);
	ret = sparklink_i2c_read_byte(di->client, (u8)di->chip_id_reg.addr);
	if (ret < 0 || ret != di->chip_id_reg.value) {
		sparklink_errmsg("read chip id error ret:0x%x\n", ret);
		return -EIO;
	}
	return 0;
}

static void hi1162_get_firmware_update_flag(struct hi1162_device_info *di)
{
	int ret;

	if (!di)
		return;
	sparklink_infomsg("hi1162 read chipid ok and will read app version\n");
	g_chipid_state = SPARKLINK_CHIPID_SUCCESS;

	di->need_update[HI1162_OTA_FW_INFO] = true;
	wakeup_116x_before_i2c_ops(di);
	ret = sparklink_i2c_read_byte(di->client, (u8)di->version_app_reg.addr);
#ifdef CONFIG_FACTORY_MODE
	if (ret == di->version_app_reg.value)
#else
	if (ret >= di->version_app_reg.value)
#endif
	{
		sparklink_errmsg("no need update ota version 0x%x\n", ret);
		di->need_update[HI1162_OTA_FW_INFO] = false;
	}

	sparklink_infomsg("hi116x read app version is 0x%x\n", ret);
}

static int hill6x_get_interrupt(struct hi1162_device_info *di, u16 *intr)
{
	int ret;
	u8 irq[HI1162_IRQ_LEN] = { 0 };

	if (!di || !intr)
		return -EINVAL;

	wakeup_116x_before_i2c_ops(di);
	ret = sparklink_i2c_read_block(di->client, HI1162_IRQ_ADDR, HI1162_IRQ_LEN, irq);
	if (ret < 0) {
		sparklink_errmsg("hi1162 read interrupt reg faild\n");
		return ret;
	}

	*intr = (irq[0] << 8) | irq[1];
	sparklink_infomsg("[get_interrupt] irq=0x%04x\n", *intr);
	return 0;
}

static int hi1162_clear_irq(struct hi1162_device_info *di, u16 irq_val)
{
	int ret;
	u8 irq[HI1162_IRQ_LEN];

	if (!di)
		return -EINVAL;

	irq[0] = (u8)((irq_val >> 8) & 0xff);
	irq[1] = (u8)((irq_val >> 0) & 0xff);

	wakeup_116x_before_i2c_ops(di);
	ret = sparklink_i2c_write_block(di->client, HI1162_CLR_IRQ_ADDR, HI1162_IRQ_LEN, irq);
	if (ret < 0) {
		sparklink_errmsg("hi1162 clear irq: write faild\n");
		return ret;
	}

	return 0;
}

static void hi116x_irq_handler(struct hi1162_device_info *di)
{
	if (!di)
		return;

	if (di->irq_val & HI1162_IRQ_WLTX_REPORT_ACC_INFO) {
		di->irq_val &= ~HI1162_IRQ_WLTX_REPORT_ACC_INFO;
		sparklink_infomsg("wltx irq: report acc info\n");
		hi1162_report_acc_info(di);
	}

	if (di->irq_val & HI1162_IRQ_WLTX_REPORT_HALL_STATUS) {
		di->irq_val &= ~HI1162_IRQ_WLTX_REPORT_HALL_STATUS;
		sparklink_infomsg("wltx irq: report hall status\n");
		hi1162_get_hall_state(di);
	}
}

static void hi1162_irq_work(struct work_struct *work)
{
	int ret;
	struct hi1162_device_info *di = NULL;

	if (!work)
		return;

	di = container_of(work, struct hi1162_device_info, irq_work);
	if (!di)
		return;

	ret = hill6x_get_interrupt(di, &di->irq_val);
	if (!ret) {
		hi1162_clear_irq(di, di->irq_val);
		hi116x_irq_handler(di);
	}

	enable_irq(di->irq);
	di->irq_active = true;
	power_wakeup_unlock(di->wake_lock, false);
}

static irqreturn_t hi1162_interrupt(int irq, void *dev)
{
	struct hi1162_device_info *di;

	di = (struct hi1162_device_info *)dev;
	if (!di)
		return IRQ_HANDLED;

	power_wakeup_lock(di->wake_lock, false);
	sparklink_infomsg("[interrupt] ++\n");
	if (di->irq_active && di->support_wltx) {
		disable_irq_nosync(di->irq);
		di->irq_active = false;
		schedule_work(&di->irq_work);
	} else {
		sparklink_errmsg("[interrupt] irq is not enable\n");
		power_wakeup_unlock(di->wake_lock, false);
	}
	sparklink_infomsg("[interrupt] --\n");

	return IRQ_HANDLED;
}

static int hi1162_update_time(struct hi1162_device_info *di)
{
	struct timespec64 tv;
	int ret, i;
	u8 time_buf[TIME_LEN] = {0};

	ktime_get_real_ts64(&tv);
	/* tv.tv_sec-int64_t storage to time_buf-u8 */
	for (i = 0; i < sizeof(time_buf); i++) {
		time_buf[i] = (tv.tv_sec >> 8 * i) & 0xff;
		sparklink_infomsg("time_buf[%d]:0x%x\n", i, time_buf[i]);
	}

	wakeup_116x_before_i2c_ops(di);
	ret = sparklink_i2c_write_block(di->client, TIME_ADDR, TIME_LEN, time_buf);
	if (ret < 0) {
		sparklink_errmsg("write TIME_REG_ADDR fail\n");
		return ret;
	}
	sparklink_infomsg("write  TIME_REG_ADDR success\n");
	return 0;
}

static void hi1162_fwupdate_work(struct work_struct *work)
{
	int ret;
	struct hi1162_device_info *di;
	struct sparklink_uart_ops *ops;

	if (!work)
		return;

	di = container_of(work, struct hi1162_device_info, fwupdate_work.work);
	if (!di)
		return;

	ops = spartlink_get_uart_ops();
	if (ops == NULL) {
		sparklink_errmsg("get uart ops error!!!\n");
		g_hi1162_state = SPARKLINK_STATE_UPDATE_FAIL;
		return;
	}

	ret = hi1162_fwupdate(ops, di);
	if (ret < 0) {
		sparklink_errmsg("device update fw error\n");
		if (g_hi1162_state != SPARKLINK_STATE_BADFLASH)
			g_hi1162_state = SPARKLINK_STATE_UPDATE_FAIL;
		return;
	}
	g_hi1162_state = SPARKLINK_STATE_POWERON;

	msleep(1000); /* delay 1s,then do i2c ops */
	hi1162_update_time(di);
}

static int hi1162_gpio_state_init(struct hi1162_device_info *di)
{
	int ret;

	if (!di)
		return -EINVAL;

	di->pctrl = devm_pinctrl_get(di->dev);
	if (IS_ERR(di->pctrl)) {
		sparklink_errmsg("failed to devm_pinctrl_get\n");
		return -EINVAL;
	}
	di->pin_default = pinctrl_lookup_state(di->pctrl, "default");
	if (IS_ERR(di->pin_default)) {
		sparklink_errmsg("failed to pinctrl_lookup_state error\n");
		return -EINVAL;
	}
	ret = pinctrl_select_state(di->pctrl, di->pin_default);
	if (ret < 0) {
		sparklink_errmsg("pinctrl_select_state error\n");
		return ret;
	}

	return 0;
}

static int hi1162_request_wakeup_gpio(struct hi1162_device_info *di)
{
	int ret;

	if (!di)
		return -EINVAL;

	ret = gpio_request(di->wakeup_gpio, "hi1162_wakeup");
	if (ret < 0) {
		sparklink_errmsg("request fail\n");
		return ret;
	}

	ret = gpio_direction_output(di->wakeup_gpio, 0);
	if (ret < 0) {
		sparklink_errmsg("set output fail\n");
		gpio_free(di->wakeup_gpio);
		return ret;
	}

	return 0;
}

static int hi1162_request_reset_gpio(struct hi1162_device_info *di)
{
	int ret;

	if (!di)
		return -EINVAL;

	ret = gpio_request(di->reset_gpio, "hi1162_reset");
	if (ret < 0) {
		sparklink_errmsg("request fail\n");
		return ret;
	}

	/* the reset_gpio set high means power on */
	ret = gpio_direction_output(di->reset_gpio, 0);
	if (ret < 0) {
		sparklink_errmsg("set output fail\n");
		gpio_free(di->reset_gpio);
		return ret;
	}

	return 0;
}

static int hi1162_request_buck_boost_en_gpio(struct hi1162_device_info *di)
{
	int ret;

	if (!di)
		return -EINVAL;

	ret = gpio_request(di->buck_boost_en_gpio, "hi1162_buck_boost_en");
	if (ret < 0) {
		sparklink_errmsg("request hi1162_buck_boost_en fail\n");
		return ret;
	}

	/* the buck_boost_en_gpio set high */
	ret = gpio_direction_output(di->buck_boost_en_gpio, 1);
	if (ret < 0) {
		sparklink_errmsg("set output fail\n");
		gpio_free(di->buck_boost_en_gpio);
		return ret;
	}

	return 0;
}

static int hi1162_request_i2c_switch_gpio(struct hi1162_device_info *di)
{
	int ret;

	if (!di)
		return -EINVAL;

	ret = gpio_request(di->i2c_switch_gpio, "hi1162_i2c_switch_gpio");
	if (ret < 0) {
		sparklink_errmsg("request hi1162_i2c_switch_gpio fail\n");
		return ret;
	}

	/* the i2c_switch_gpio set low */
	ret = gpio_direction_output(di->i2c_switch_gpio, 0);
	if (ret < 0) {
		sparklink_errmsg("set output fail\n");
		return ret;
	}

	return 0;
}

static int hi1162_request_irq_gpio(struct hi1162_device_info *di)
{
	int ret;

	if (!di)
		return -EINVAL;

	ret = gpio_request(di->irq_gpio, "hi1162_irq");
	if (ret < 0) {
		sparklink_errmsg("request fail\n");
		return ret;
	}

	ret = gpio_direction_input(di->irq_gpio);
	if (ret < 0) {
		sparklink_errmsg("set input fail\n");
		goto HI1162_IRQ_GPIO_FREE;
	}

	di->irq = gpio_to_irq(di->irq_gpio);
	if (di->irq < 0) {
		sparklink_errmsg("gpio to irq fail\n");
		goto HI1162_IRQ_GPIO_FREE;
	}

	ret = request_irq(di->irq, hi1162_interrupt, IRQF_TRIGGER_FALLING, "hi1162_irq", di);
	if (ret < 0) {
		sparklink_errmsg("request irq fail\n");
		goto HI1162_IRQ_GPIO_FREE;
	}

	ret = enable_irq_wake(di->irq);
	if (ret < 0) {
		sparklink_errmsg("enable irq wake fail\n");
		goto HI1162_IRQ_FREE;
	}

	di->irq_active = true;
	INIT_WORK(&di->irq_work, hi1162_irq_work);

	return 0;
HI1162_IRQ_FREE:
	free_irq(di->irq, di);
HI1162_IRQ_GPIO_FREE:
	gpio_free(di->irq_gpio);
	return -1;
}

static int hi1162_gpio_init(struct hi1162_device_info *di)
{
	int ret;

	if (!di)
		return -EINVAL;

	ret = hi1162_gpio_state_init(di);
	if (ret < 0)
		sparklink_infomsg("pinctrl not config\n");

	ret = hi1162_request_wakeup_gpio(di);
	if (ret < 0)
		return ret;

	ret = hi1162_request_reset_gpio(di);
	if (ret < 0)
		goto HI1162_REQUEST_RESET_GPIO_FAIL;

	ret = hi1162_request_irq_gpio(di);
	if (ret < 0)
		sparklink_errmsg("request irq gpio failed\n");

	ret = hi1162_request_buck_boost_en_gpio(di);
	if (ret < 0)
		sparklink_errmsg("request buck_boost_en_gpio error\n");

	ret = hi1162_request_i2c_switch_gpio(di);
	if (ret < 0)
		sparklink_errmsg("request hi1162 gpio error\n");

	return 0;

HI1162_REQUEST_RESET_GPIO_FAIL:
	gpio_free(di->wakeup_gpio);
	return -1;
}

static void hi1162_gpio_deinit(struct hi1162_device_info *di)
{
	if (!di)
		return;

	gpio_free(di->reset_gpio);
	gpio_free(di->wakeup_gpio);

	if (gpio_is_valid(di->irq_gpio)) {
		gpio_free(di->irq_gpio);
		free_irq(di->irq, di);
	}
	if (gpio_is_valid(di->buck_boost_en_gpio))
		gpio_free(di->buck_boost_en_gpio);
	if (gpio_is_valid(di->i2c_switch_gpio))
		gpio_free(di->i2c_switch_gpio);
}

static int hi1162_property_reg(struct device_node *np, const char *prop, sparklink_reg *reg)
{
	int count = 0;
	int ret;
	u32 buf[REG_MAX] = {0};

	if (!np || !prop || !reg)
		return -EINVAL;

	if (of_property_read_bool(np, prop))
		count = of_property_count_elems_of_size(np, prop, (int)sizeof(u32));

	if (count != REG_MAX) {
		sparklink_errmsg("read property array %s count err\n", prop);
		return -EINVAL;
	}

	ret = of_property_read_u32_array(np, prop, buf, (size_t)(long)count);
	if (ret < 0) {
		sparklink_errmsg("read property array %s fail\n", prop);
		return ret;
	}
	reg->addr = buf[REG_ADDR];
	reg->size = buf[REG_SIZE];
	reg->value = buf[REG_VALUE];
	return 0;
}

static int hi1162_prase_common_gpio(struct hi1162_device_info *di)
{
	int i;
	struct device_node *np;
	const char *gpio_name[] = {"wakeup_gpio", "reset_gpio"};
	unsigned int gpio_value[arry_size(gpio_name)] = {0};

	if (!di)
		return -EINVAL;

	np = di->client->dev.of_node;
	if (!np)
		return -EINVAL;

	for (i = 0; i < arry_size(gpio_name); i++) {
		gpio_value[i] = of_get_named_gpio(np, gpio_name[i], 0);
		if (!gpio_is_valid(gpio_value[i])) {
			sparklink_errmsg("read gpio %s error\n", gpio_name[i]);
			return -EINVAL;
		}
	}

	/* 0:wakeup_gpio, 1:reset_gpio */
	di->wakeup_gpio = gpio_value[0];
	di->reset_gpio = gpio_value[1];

	return 0;
}

static int hi116x_prase_optional_gpio(struct hi1162_device_info *di)
{
	int i;
	struct device_node *np;

	const char *gpio_name[] = {"irq_gpio", "buck_boost_en_gpio", "i2c_switch_gpio"};
	int gpio_value[arry_size(gpio_name)] = {0};

	if (!di)
		return -EINVAL;

	np = di->client->dev.of_node;
	if (!np)
		return -EINVAL;

	for (i = 0; i < arry_size(gpio_name); i++) {
		gpio_value[i] = of_get_named_gpio(np, gpio_name[i], 0);
		if (!gpio_is_valid(gpio_value[i]))
			sparklink_errmsg("read gpio %s error\n", gpio_name[i]);
	}

	di->irq_gpio = gpio_value[0];
	di->buck_boost_en_gpio = gpio_value[1];
	di->i2c_switch_gpio = gpio_value[2];

	return 0;
}

static int hi1162_prase_reg_dts(struct hi1162_device_info *di)
{
	int ret, i;
	struct device_node *np;
	const char *reg_name[] = {"chip_id_reg", "version_ssb_reg",
		"version_bt_reg", "version_app_reg",
		// fw_info [index ramsize flashsize]
		"ssbefc_fw_info", "ckff_fw_info", "ssb_fw_info", "nv_fw_info",
		"bt_fw_info", "app_fw_info", "ota_fw_info"};
	sparklink_reg reg_value[arry_size(reg_name)] = {{0}};

	if (!di)
		return -EINVAL;

	np = di->client->dev.of_node;
	if (!np)
		return -EINVAL;

	for (i = 0; i < arry_size(reg_name); i++) {
		ret = hi1162_property_reg(np, reg_name[i], &reg_value[i]);
		if (ret < 0) {
			sparklink_errmsg("read reg %s error\n", reg_name[i]);
			return ret;
		}
	}

	/* 0:chip id reg, 1:ssb ver reg, 2:bt ver reg, 3:app ver reg */
	di->chip_id_reg = reg_value[0];
	di->version_ssb_reg = reg_value[1];
	di->version_bt_reg = reg_value[2];
	di->version_app_reg = reg_value[3];

	/* 0\4:ssbefc info, 1\5:ckff info */
	/* 2\6:ssb info, 3\7:nv info, 4\8:bt info, 5\9:app info 6\10:ota */
	di->fw_info[0] = reg_value[4];
	di->fw_info[1] = reg_value[5];
	di->fw_info[2] = reg_value[6];
	di->fw_info[3] = reg_value[7];
	di->fw_info[4] = reg_value[8];
	di->fw_info[5] = reg_value[9];
	di->fw_info[6] = reg_value[10];
	return 0;
}

static int hi1162_prase_fw_path(struct hi1162_device_info *di)
{
	int ret, i;
	struct device_node *np;
	const char *path_name[] = {"ssbefc_fw_path", "ckff_fw_path", "ssb_fw_path", "nv_fw_path",
		"bt_fw_path", "app_fw_path", "ota_fw_path"};
	if (!di)
		return -EINVAL;

	np = di->client->dev.of_node;
	if (!np)
		return -EINVAL;

	for (i = 0; i < arry_size(path_name); i++) {
		ret = of_property_read_string(np, path_name[i], &di->fw_path[i]);
		if (ret < 0) {
			sparklink_errmsg("read fw_path= %s error\n", path_name[i]);
			return -EINVAL;
		}
	}
	return 0;
}

static int hi1162_prase_uart_info(struct hi1162_device_info *di)
{
	int ret;
	struct device_node *np;

	if (!di)
		return -EINVAL;

	np = di->client->dev.of_node;
	if (!np)
		return -EINVAL;

	ret = of_property_read_string(np, "uart_name", &di->tty_name);
	if (ret < 0) {
		sparklink_errmsg("read uart_name error\n");
		return ret;
	}
	ret = of_property_read_u32(np, "default_baud", &di->default_baud);
	if (ret < 0) {
		sparklink_errmsg("read default_baud error\n");
		return ret;
	}
	ret = of_property_read_u32(np, "max_baud", &di->max_baud);
	if (ret < 0) {
		sparklink_errmsg("read max_baud error\n");
		return ret;
	}
	return 0;
}

static int hi1162_prase_wltx_info(struct hi1162_device_info *di)
{
	int ret;
	struct device_node *np;

	if (!di || !di->client)
		return -EINVAL;

	np = di->client->dev.of_node;
	if (!np)
		return -EINVAL;

	ret = of_property_read_u32(np, "support_wltx", &di->support_wltx);
	if (ret < 0) {
		sparklink_errmsg("support_wltx is not set, config 0\n");
		di->support_wltx = 0;
	}

	return 0;
}

static int hi116x_prase_attach_event_info(struct hi1162_device_info *di)
{
	int ret;
	struct device_node *np = NULL;

	if (!di || !di->client)
		return -EINVAL;

	np = di->client->dev.of_node;
	if (!np)
		return -EINVAL;

	ret = of_property_read_u32(np, "support_pen_attach_q_detect", &di->support_pen_attach_q_detect);
	if (ret < 0) {
		sparklink_errmsg("support_pen_attach_q_detect is not set, config 0\n");
		di->support_pen_attach_q_detect = 0;
	}

	ret = of_property_read_u32(np, "support_kb_attach_pogopin", &di->support_kb_attach_pogopin);
	if (ret < 0) {
		sparklink_errmsg("support_kb_attach_pogopin is not set, config 0\n");
		di->support_kb_attach_pogopin = 0;
	}

	return 0;
}

static int hi1162_prase_switch_channel_info(struct hi1162_device_info *di)
{
	int ret;
	struct device_node *np;

	if (!di || !di->client)
		return -EINVAL;

	np = di->client->dev.of_node;
	if (!np)
		return -EINVAL;

	ret = of_property_read_u32(np, "support_switch_channel", &di->support_switch_channel);
	if (ret < 0) {
		sparklink_errmsg("support_switch_channel is not set, config 0\n");
		di->support_switch_channel = 0;
	}

	return 0;
}

static int hi1162_prase_dts(struct hi1162_device_info *di)
{
	int ret;

	if (!di)
		return -EINVAL;

	/* necessary config, must be here, start */
	ret = hi1162_prase_common_gpio(di);
	if (ret < 0)
		return ret;

	ret = hi1162_prase_reg_dts(di);
	if (ret < 0)
		return ret;

	ret = hi1162_prase_fw_path(di);
	if (ret < 0)
		return ret;

	ret = hi1162_prase_uart_info(di);
	if (ret < 0)
		return ret;
	/* necessary config, must be here, end */

	/* customize config, must be here, start */
	ret = hi1162_prase_wltx_info(di);
	if (ret < 0)
		return ret;

	ret = hi116x_prase_attach_event_info(di);
	if (ret < 0)
		return ret;

	ret = hi1162_prase_switch_channel_info(di);
	if (ret < 0)
		return ret;

	ret = hi116x_prase_optional_gpio(di);
	if (ret < 0)
		return ret;
	/* customize config, must be here, end */

	return 0;
}

static void hi1162_connect_i2c_work(struct work_struct *work)
{
	if (!g_hi1162_info)
		return;

	gpio_set_value(g_hi1162_info->i2c_switch_gpio, 0);
	sparklink_errmsg("setting i2c switch gpio low\n");

	return;
}

static void hi116x_switch_i2c_hw_reset(void)
{
	int ret;

	if (!g_hi1162_info)
		return;

	if(gpio_is_valid(g_hi1162_info->i2c_switch_gpio)) {
		gpio_set_value(g_hi1162_info->i2c_switch_gpio, 1);
		msleep(HI1162_SWITCH_GPIO_UP_WAIT);  /* delay 15ms, wait switch function done */
	}

	gpio_set_value(g_hi1162_info->reset_gpio, 1);
	/* delay 10ms for reset */
	msleep(HW_RESET_DELAY_TIME);
	gpio_set_value(g_hi1162_info->reset_gpio, 0);

	if(gpio_is_valid(g_hi1162_info->i2c_switch_gpio)) {
		schedule_delayed_work(&g_hi1162_info->connect_i2c_work,
	                          msecs_to_jiffies(HI1162_SWITCH_GPIO_DELAY));
		ret = gpio_get_value(g_hi1162_info->i2c_switch_gpio);
		sparklink_errmsg("setting reset gpio low level and i2c switch gpio value:%d\n", ret);
	}
}

int hi1162_device_set_mode(int powermode)
{
	int ret;

	if (!g_hi1162_info || powermode > SPARKLINK_STATE_MAX)
		return -1;

	if (powermode == SPARKLINK_STATE_POWERON || powermode == SPARKLINK_STATE_RESET) {
		if (g_sf_reset_state == SF_RESET_SUPPORT) {
			wakeup_116x_before_i2c_ops(g_hi1162_info);
			ret = sparklink_i2c_write_byte(g_hi1162_info->client, SF_RESET_ADDR, SF_RESET);
			if (ret < 0) {
				sparklink_errmsg("write rst reg ret:%d rst_state:%d and do hw-reset\n",
					ret, g_sf_reset_state);
				hi116x_switch_i2c_hw_reset();
			}
		} else {
			sparklink_errmsg("rst_state:%d and do hw-reset direct\n", g_sf_reset_state);
			hi116x_switch_i2c_hw_reset();
		}
	} else {
		sparklink_errmsg("mode err\n");
	}
	return 0;
}

int hi1162_device_get_chipid_state(void)
{
	return g_chipid_state;
}

void hi1162_device_set_state(int state)
{
	g_hi1162_state = state;
}

int hi1162_device_get_state(void)
{
	return g_hi1162_state;
}

static int hi1162_device_version(void)
{
	int ret;

	if (!g_hi1162_info)
		return -1;

	wakeup_116x_before_i2c_ops(g_hi1162_info);
	ret = sparklink_i2c_read_byte(g_hi1162_info->client, (u8)g_hi1162_info->version_app_reg.addr);
	if (ret < 0)
		sparklink_errmsg("read app version fail\n");

	return ret;
}

static int hi1162_i2c_log(int addr, int rw_flage, char *buf, size_t len)
{
	return -1;
}

#ifdef CONFIG_SYSFS
static ssize_t hi1162_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf);

static ssize_t hi1162_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);

static struct power_sysfs_attr_info hi1162_sysfs_field_tbl[] = {
	power_sysfs_attr_rw(hi1162, 0644, WL_TX_SYSFS_TX_OPEN, tx_open),
	power_sysfs_attr_ro(hi1162, 0444, WL_TX_SYSFS_RX_PRODUCT_TYPE, rx_product_type),
	power_sysfs_attr_ro(hi1162, 0444, WL_TX_SYSFS_TX_STATUS, tx_status),
	power_sysfs_attr_ro(hi1162, 0444, WL_TX_SYSFS_TX_VIN, tx_vin),
	power_sysfs_attr_ro(hi1162, 0444, WL_TX_SYSFS_TX_IIN, tx_iin),
	power_sysfs_attr_ro(hi1162, 0444, WL_TX_SYSFS_TX_IIN_AVG, tx_iin_avg),
	power_sysfs_attr_ro(hi1162, 0444, WL_TX_SYSFS_HALL_STATE, hall_state),
};

static struct attribute *hi1162_sysfs_attrs[ARRAY_SIZE(hi1162_sysfs_field_tbl) + 1];

static const struct attribute_group hi1162_sysfs_attr_group = {
	.attrs = hi1162_sysfs_attrs,
};

static void hi1162_sysfs_create_group(struct device *dev)
{
	power_sysfs_init_attrs(hi1162_sysfs_attrs,
		hi1162_sysfs_field_tbl, ARRAY_SIZE(hi1162_sysfs_field_tbl));
	power_sysfs_create_link_group("hw_power", "charger", "wireless_aux_tx",
		dev, &hi1162_sysfs_attr_group);
}

static void hi1162_sysfs_remove_group(struct device *dev)
{
	power_sysfs_remove_link_group("hw_power", "charger", "wireless_aux_tx",
		dev, &hi1162_sysfs_attr_group);
}
#else
static inline void hi1162_sysfs_create_group(struct device *dev)
{
}

static inline void hi1162_sysfs_remove_group(struct device *dev)
{
}
#endif /* CONFIG_SYSFS */

static ssize_t hi1162_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct power_sysfs_attr_info *info = NULL;
	struct hi1162_device_info *di = g_hi1162_info;
	u16 temp_data = 0;

	info = power_sysfs_lookup_attr(attr->attr.name,
		hi1162_sysfs_field_tbl, ARRAY_SIZE(hi1162_sysfs_field_tbl));
	if (!info || !di)
		return -EINVAL;

	switch (info->name) {
	case WL_TX_SYSFS_RX_PRODUCT_TYPE:
		sparklink_infomsg("%s: WL_TX_SYSFS_RX_PRODUCT_TYPE\n", __func__);
		hi1162_wltx_get_rx_product_type(di);
		return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE -1, "%s\n", di->rx_product_type);
	case WL_TX_SYSFS_TX_OPEN:
		sparklink_infomsg("%s: WL_TX_SYSFS_TX_OPEN\n", __func__);
		hi1162_wltx_get_tx_info_byte(di, &temp_data, HI1162_TX_OPEN_REG);
		return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE -1, "%d\n", temp_data);
	case WL_TX_SYSFS_TX_STATUS:
		sparklink_infomsg("%s: WL_TX_SYSFS_TX_STATUS\n", __func__);
		hi1162_wltx_get_tx_info_byte(di, &temp_data, HI1162_TX_STATUS_REG);
		return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE -1, "%d\n", temp_data);
	case WL_TX_SYSFS_TX_VIN:
		sparklink_infomsg("%s: WL_TX_SYSFS_TX_VIN\n", __func__);
		hi1162_wltx_get_tx_info_block(di, &temp_data, HI1162_TX_VIN_REG);
		return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE -1, "%d\n", temp_data);
	case WL_TX_SYSFS_TX_IIN:
		sparklink_infomsg("%s: WL_TX_SYSFS_TX_IIN\n", __func__);
		hi1162_wltx_get_tx_info_block(di, &temp_data, HI1162_TX_IIN_REG);
		return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE -1, "%d\n", temp_data);
	case WL_TX_SYSFS_TX_IIN_AVG:
		sparklink_infomsg("%s: WL_TX_SYSFS_TX_IIN_AVG\n", __func__);
		hi1162_wltx_get_tx_info_block(di, &temp_data, HI1162_TX_IIN_AVG_REG);
		return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE -1, "%d\n", temp_data);
	case WL_TX_SYSFS_HALL_STATE:
		sparklink_infomsg("%s: WL_TX_SYSFS_HALL_STATE\n", __func__);
		return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE -1, "%d\n", di->hall_state);
	default:
		sparklink_errmsg("%s: NO THIS NODE:%d\n", __func__, info->name);
		break;
	}
	return 0;
}

static ssize_t hi1162_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct power_sysfs_attr_info *info = NULL;
	long val = 0;

	info = power_sysfs_lookup_attr(attr->attr.name,
		hi1162_sysfs_field_tbl, ARRAY_SIZE(hi1162_sysfs_field_tbl));
	if (!info || !g_hi1162_info)
		return -EINVAL;

	switch (info->name) {
	case WL_TX_SYSFS_TX_OPEN:
		if (kstrtol(buf, POWER_BASE_DEC, &val) < 0) {
			sparklink_errmsg("%s: val is not valid\n", __func__);
			return -EINVAL;
		}
		hi1162_wltx_open_tx(g_hi1162_info, val);
		break;
	default:
		sparklink_errmsg("%s: NO THIS NODE:%d\n", __func__, info->name);
		break;
	}
	return count;
}

int hi1162_uart_switch_channel(int uart_channel)
{
	int ret;

	if (!g_hi1162_info || uart_channel >= SPARKLINK_UART_CHANNEL_MAX)
		return -EINVAL;

	if (!g_hi1162_info->support_switch_channel) {
		sparklink_errmsg("not support switch channel\n");
		return -EINVAL;
	}

	switch (uart_channel) {
	case SPARKLINK_UART_L0:
		/* L0 -- 2 line soft flow */
		wakeup_116x_before_i2c_ops(g_hi1162_info);
		ret = sparklink_i2c_write_byte(g_hi1162_info->client, UART_CHANNEL_REG_ADDR, uart_channel);
		if (ret < 0) {
			sparklink_errmsg("soft flow write reg fail\n");
			return ret;
		}
		sparklink_infomsg("soft flow write reg sucess\n");
		break;

	case SPARKLINK_UART_H0:
		/* H0 -- 4 line hw flow */
		wakeup_116x_before_i2c_ops(g_hi1162_info);
		ret = sparklink_i2c_write_byte(g_hi1162_info->client, UART_CHANNEL_REG_ADDR, uart_channel);
		if (ret < 0) {
			sparklink_errmsg("hw flow write reg fail\n");
			return ret;
		}
		sparklink_infomsg("hw flow write reg sucess\n");
		break;

	default:
		/* 1162 uart_channel is neither L0 nor H0 */
		sparklink_errmsg("uart_channel is unknow\n");
		return -EINVAL;
	}
	return 0;
}

struct sparklink_driver_ops hi1162_ops = {
	.device_powermode = hi1162_device_set_mode,
	.device_get_state = hi1162_device_get_state,
	.device_version = hi1162_device_version,
	.i2c_log = hi1162_i2c_log,
	.device_uart_switch_channel = hi1162_uart_switch_channel,
};

static int hi1162_dev_info_setup(struct i2c_client *client, struct hi1162_device_info *di)
{
	int ret;

	if (!client || !di)
		return -EINVAL;

	di->dev = &client->dev;
	di->client = client;
	i2c_set_clientdata(client, di);
	g_hi1162_info = di;
	di->wake_lock = wakeup_source_register(NULL, "hi1162");

	ret = hi1162_prase_dts(di);
	if (ret < 0)
		return ret;

	ret = hi1162_gpio_init(di);
	if (ret < 0)
		return ret;

	return 0;
}

static void hi1162_set_firmware_update_work(struct work_struct *work)
{
	int ret, i;
	struct hi1162_device_info *di;
	u8 value_nv[NV_LEN] = {0};

	if (!work)
		return;

	di = container_of(work, struct hi1162_device_info, check_update_work.work);
	if (!g_hi1162_info || !di)
		return;

	sparklink_infomsg("hi1162_set_firmware_update_work enter\n");
	ret = hi1162_check_id(di);
	if (ret < 0) {
		sparklink_errmsg("chipid check error,1162 is empty flash, need update\n");
		for (i = HI1162_SSBEFC_FW_INFO; i <= HI1162_OTA_FW_INFO; i++)
			di->need_update[i] = true;
		/* NV magic number 0x254d */
		wakeup_116x_before_i2c_ops(di);
		ret = sparklink_i2c_read_block(di->client, NV_ADDR, NV_LEN, value_nv);
		/* value_nv[1] == 0x25 value_nv[0] == 0x4d */
		if ((ret == 0) && (((value_nv[1] << 8) | value_nv[0]) == NV_MAGIC)) {
			sparklink_errmsg("no need update nv;version:0x%x-%x\n", value_nv[0], value_nv[1]);
			di->need_update[HI1162_SSB_FW_INFO] = false;
			di->need_update[HI1162_NV_FW_INFO] = false;
			di->need_update[HI1162_SSBEFC_FW_INFO] = false;
			di->need_update[HI1162_CKFF_FW_INFO] = false;
		}
	} else {
		for (i = HI1162_SSBEFC_FW_INFO; i <= HI1162_APP_FW_INFO; i++)
			di->need_update[i] = false;

		hi1162_get_firmware_update_flag(di);
	}

	wakeup_116x_before_i2c_ops(di);
	g_sf_reset_state = sparklink_i2c_read_byte(g_hi1162_info->client, SF_RESET_SUPPORT_ADDR);
	sparklink_infomsg("g_sf_reset_state %d\n", g_sf_reset_state);
	/* if it not factory version,it donot flash-check */
	#ifndef CONFIG_FACTORY_MODE
		sparklink_infomsg("not factory mode; do not flash-check\n");
		di->need_update[HI1162_SSBEFC_FW_INFO] = false;
		di->need_update[HI1162_CKFF_FW_INFO] = false;
		di->need_update[HI1162_NV_FW_INFO] = false;
	#endif

	return;
}

static int hi1162_reboot_notify(struct notifier_block *nb,
				unsigned long code, void *unused)
{
	int ret;

	if (!g_hi1162_info || !gpio_is_valid(g_hi1162_info->reset_gpio)
		|| !gpio_is_valid(g_hi1162_info->buck_boost_en_gpio)) {
		sparklink_errmsg("reset_gpio or buck_boost_en_gpio is invalid\n");
		return 0;
	}

	wakeup_116x_before_i2c_ops(g_hi1162_info);
	ret = sparklink_i2c_write_byte(g_hi1162_info->client, DISABLE_IQR_BT_ADDR, DISABLE_IQR_BT);
	if (ret < 0)
		sparklink_errmsg("write DISABLE_IQR_BT_ADDR fail ret:%d \n", ret);

	/* delay 50ms make sure write reg:0x1a 0x1 and 1162 lock irq and close bt */
	msleep(DISABLE_IQR_BT_DELAY_TIME);
	gpio_set_value(g_hi1162_info->reset_gpio, 1);
	msleep(SHUTDOWN_DELAY_TIME); /* delay 5ms */
	gpio_set_value(g_hi1162_info->buck_boost_en_gpio, 0);
	msleep(SHUTDOWN_DELAY_TIME); /* delay 5ms */
	gpio_set_value(g_hi1162_info->reset_gpio, 0);

	return 0;
}

static int sparklink_fb_notify(struct notifier_block *nb, unsigned long action, void *data)
{
	int ret;
	struct fb_event *event = (struct fb_event *)data;
	int *blank;

	if (!g_hi1162_info || !event || !event->data || action != FB_EVENT_BLANK)
		return NOTIFY_OK;

	if (g_hi1162_state == SPARKLINK_STATE_UPDATING) {
		sparklink_errmsg("no fb event when 116x is updating\n");
		return NOTIFY_OK;
	}

	blank = event->data;

	switch (*blank) {
	case FB_BLANK_UNBLANK:
		/* screen on reg0x13 write 0x1 */
		sparklink_infomsg("it will screen on\n");
		wakeup_116x_before_i2c_ops(g_hi1162_info);
		ret = sparklink_i2c_write_byte(g_hi1162_info->client, LCD_ON_OFF_ADDR, LCD_STATE_ON);
		if (ret < 0)
			sparklink_errmsg("screen on write reg fail\n");
		break;
	case FB_BLANK_POWERDOWN:
		/* screen off  reg0x13 write 0x2 */
		sparklink_infomsg("it will screen off\n");
		wakeup_116x_before_i2c_ops(g_hi1162_info);
		ret = sparklink_i2c_write_byte(g_hi1162_info->client, LCD_ON_OFF_ADDR, LCD_STATE_OFF);
		if (ret < 0)
			sparklink_errmsg("screen off write reg fail\n");
		break;
	default:
		break;
	}

	return NOTIFY_DONE;
}

static int sparklink_pen_attach_notifier_call(struct notifier_block *nb,
	unsigned long action, void *data)
{
	if (!g_hi1162_info || g_hi1162_state == SPARKLINK_STATE_UPDATING) {
		sparklink_errmsg("no pen_attach event when 116x is updating\n");
		return NOTIFY_OK;
	}

	switch (action) {
	case POWER_NE_WLTX_HALL_APPROACH:
		sparklink_infomsg("pen attach,write reg\n");
		wakeup_116x_before_i2c_ops(g_hi1162_info);
		sparklink_i2c_write_byte(g_hi1162_info->client, ATTACH_PEN_ADDR, PEN_KB_ATTACH_EVENT);
		break;
	case POWER_NE_WLTX_HALL_AWAY_FROM:
		sparklink_infomsg("pen away,write reg\n");
		wakeup_116x_before_i2c_ops(g_hi1162_info);
		sparklink_i2c_write_byte(g_hi1162_info->client, ATTACH_PEN_ADDR, PEN_KB_AWAY_EVENT);
		break;
	default:
		return NOTIFY_OK;
	}

	return NOTIFY_DONE;
}

static int sparklink_kb_attach_notifier_call(struct notifier_block *nb,
	unsigned long action, void *data)
{
	if (!g_hi1162_info || g_hi1162_state == SPARKLINK_STATE_UPDATING) {
		sparklink_errmsg("no kb_attach event when 116x is updating\n");
		return NOTIFY_OK;
	}

	switch (action) {
	case HW_POGOPIN_PING_SUCC_SPARKLINK:
		sparklink_infomsg("keyboard attach,write reg\n");
		wakeup_116x_before_i2c_ops(g_hi1162_info);
		sparklink_i2c_write_byte(g_hi1162_info->client, ATTACH_KEYBOARD_ADDR, PEN_KB_ATTACH_EVENT);
		break;
	case HW_POGOPIN_DISCONNECT_SPARKLINK:
		sparklink_infomsg("keyboard away,write reg\n");
		wakeup_116x_before_i2c_ops(g_hi1162_info);
		sparklink_i2c_write_byte(g_hi1162_info->client, ATTACH_KEYBOARD_ADDR, PEN_KB_AWAY_EVENT);
		break;
	default:
		return NOTIFY_OK;
	}

	return NOTIFY_DONE;
}

static int hi1162_register_notifier(struct hi1162_device_info *di)
{
	int ret;

	if (!di)
	return -EINVAL;

	di->reboot_notifier.notifier_call = hi1162_reboot_notify;
	ret = register_reboot_notifier(&di->reboot_notifier);
	if (ret) {
		sparklink_errmsg("failed to register reboot notifier\n");
		return ret;
	}

	di->fb_notifier.notifier_call = sparklink_fb_notify;
	ret = fb_register_client(&di->fb_notifier);
	if (ret) {
		sparklink_errmsg("failed to register fb notifier\n");
		return ret;
	}

	if (di->support_pen_attach_q_detect) {
		di->pen_attach_notifier.notifier_call = sparklink_pen_attach_notifier_call;
		ret = power_event_bnc_register(POWER_BNT_WLTX_AUX, &di->pen_attach_notifier);
		if (ret) {
			sparklink_errmsg("failed to register pen attach notifier\n");
			return ret;
		}
	}

#ifdef CONFIG_POGO_PIN
	if (di->support_kb_attach_pogopin) {
		di->kb_attach_notifier.notifier_call = sparklink_kb_attach_notifier_call;
		ret = hw_pogopin_sw_event_notifier_register(&di->kb_attach_notifier);
		if (ret) {
			sparklink_errmsg("failed to register kb attach notifier\n");
			return ret;
		}
	}
#endif

	return 0;
}

static int hi1162_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int ret;
	struct hi1162_device_info *di;

	if (!client)
		return -EINVAL;

	sparklink_infomsg("hi116x probe enter\n");
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_BYTE_DATA)) {
		sparklink_errmsg("i2c_check failed\n");
		return -EIO;
	}

	di = devm_kzalloc(&client->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	ret = hi1162_dev_info_setup(client, di);
	if (ret < 0)
		goto FERR_DEVICE_INFO;

	/* need init i2c work befor set_mode */
	INIT_DELAYED_WORK(&di->connect_i2c_work, hi1162_connect_i2c_work);
	ret = hi1162_device_set_mode(SPARKLINK_STATE_POWERON);
	if (ret < 0)
		goto FREE_GPIO;

	ret = hi1162_register_notifier(di);
	if (ret < 0)
		return ret;

	spartlink_driver_ops_register(&hi1162_ops);
	INIT_DELAYED_WORK(&di->check_update_work, hi1162_set_firmware_update_work);
	/* after reset 1162,it need wait 1s to read chipid */
	schedule_delayed_work(&di->check_update_work, msecs_to_jiffies(1000));

	INIT_DELAYED_WORK(&di->fwupdate_work, hi1162_fwupdate_work);
	schedule_delayed_work(&di->fwupdate_work, msecs_to_jiffies(10000)); /* 10s */
	if (di->support_wltx)
		hi1162_sysfs_create_group(di->dev);
	sparklink_infomsg("hi1162 probe ok\n");
	return 0;

FREE_GPIO:
	sparklink_errmsg("gpio deinit, probe error\n");
	hi1162_gpio_deinit(di);
FERR_DEVICE_INFO:
	sparklink_errmsg("device check ok probe error\n");
	wakeup_source_unregister(di->wake_lock);
	devm_kfree(&client->dev, di);
	di = NULL;
	g_hi1162_info = NULL;
	return -1;
}

static int hi1162_remove(struct i2c_client *client)
{
	struct hi1162_device_info *di;

	if (!client)
		return -EINVAL;

	di = i2c_get_clientdata(client);
	if (!di)
		return -EIO;

	sparklink_infomsg("%s: called\n", __func__);

	if (di->support_wltx)
		hi1162_sysfs_remove_group(di->dev);
	if (g_hi1162_info) {
		wakeup_source_unregister(di->wake_lock);
		hi1162_gpio_deinit(di);
		devm_kfree(&client->dev, g_hi1162_info);
		unregister_reboot_notifier(&di->reboot_notifier);
		fb_unregister_client(&di->fb_notifier);
		if (di->support_pen_attach_q_detect)
			power_event_bnc_unregister(POWER_BNT_WLTX_AUX, &di->pen_attach_notifier);
#ifdef CONFIG_POGO_PIN
		if (di->support_kb_attach_pogopin)
			hw_pogopin_sw_event_notifier_unregister(&di->kb_attach_notifier);
#endif
		g_hi1162_info = NULL;
		di = NULL;
	}

	return 0;
}

static const struct of_device_id hi1162_match_table[] = {
	{.compatible = "huawei,hi1162", },
	{ },
};

static const struct i2c_device_id hi1162_device_id[] = {
	{"hi1162_device", 0},
	{ }
};
MODULE_DEVICE_TABLE(i2c, hi1162_device_id);

static struct i2c_driver hi1162_i2c_driver = {
	.probe = hi1162_probe,
	.remove = hi1162_remove,
	.id_table = hi1162_device_id,
	.driver = {
		.name = "hi1162_device",
		.owner = THIS_MODULE,
		.bus = &i2c_bus_type,
		.of_match_table = hi1162_match_table,
	},
};

static int __init hi1162_module_init(void)
{
	return i2c_add_driver(&hi1162_i2c_driver);
}

static void __exit hi1162_module_exit(void)
{
	return i2c_del_driver(&hi1162_i2c_driver);
}

module_init(hi1162_module_init);
module_exit(hi1162_module_exit);
MODULE_AUTHOR("Huawei Device Company");
MODULE_DESCRIPTION("Huawei hi1162 Driver");
MODULE_LICENSE("GPL");
