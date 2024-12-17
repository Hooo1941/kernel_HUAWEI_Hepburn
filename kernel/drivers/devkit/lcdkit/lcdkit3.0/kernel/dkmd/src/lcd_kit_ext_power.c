/*
 * lcd_kit_ext_power.c
 *
 * lcdkit power function for lcd driver
 *
 * Copyright (c) 2019-2020 Huawei Technologies Co., Ltd.
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

#include "lcd_kit_disp.h"
#include "lcd_kit_power.h"
#include "lcd_kit_common.h"
#include "lcd_kit_ext_power.h"

#define MAX_WAIT_POWER_OFF_TIME 500

/* global gpio */
uint32_t g_lcd_kit_gpio_plugin;
struct completion power_off_done;
struct lcd_pwr_off_optimize_config g_pwr_off_optimize_cfg;

/* gpio power */
static struct gpio_desc gpio_req_cmds[] = {
	{
		DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0,
		GPIO_NAME, &g_lcd_kit_gpio_plugin, 0
	},
};

static struct gpio_desc gpio_low_cmds[] = {
	{
		DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 0,
		GPIO_NAME, &g_lcd_kit_gpio_plugin, 0
	},
};

static struct gpio_desc gpio_high_cmds[] = {
	{
		DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 0,
		GPIO_NAME, &g_lcd_kit_gpio_plugin, 1
	},
};

static struct gpio_desc gpio_free_cmds[] = {
	{
		DTYPE_GPIO_FREE, WAIT_TYPE_US, 0,
		GPIO_NAME, &g_lcd_kit_gpio_plugin, 0
	},
};

static struct gpio_power_arra gpio_power_plugin[] = {
	{ GPIO_REQ, ARRAY_SIZE(gpio_req_cmds), gpio_req_cmds },
	{ GPIO_HIGH, ARRAY_SIZE(gpio_high_cmds), gpio_high_cmds },
	{ GPIO_LOW, ARRAY_SIZE(gpio_low_cmds), gpio_low_cmds },
	{ GPIO_FREE, ARRAY_SIZE(gpio_free_cmds), gpio_free_cmds },
};

void wait_power_off_done(int panel_id)
{
	int working_flag = g_pwr_off_optimize_cfg.working_flag;

	if (!disp_info->pwr_off_optimize_info.support)
		return;

	if (working_flag == panel_id + true) {
		LCD_KIT_INFO("power off completion\n");
		reinit_completion(&power_off_done);
		if (!wait_for_completion_timeout(&power_off_done,
			msecs_to_jiffies(MAX_WAIT_POWER_OFF_TIME)))
			LCD_KIT_ERR("wait power off done time out!\n");
	}
}

static void lcd_kit_get_gpio_num(uint32_t type)
{
	switch (type) {
	case LCD_KIT_VCI:
		g_lcd_kit_gpio_plugin = g_pwr_off_optimize_cfg.lcd_vci->buf[POWER_NUM];
		break;
	case LCD_KIT_IOVCC:
		g_lcd_kit_gpio_plugin = g_pwr_off_optimize_cfg.lcd_iovcc->buf[POWER_NUM];
		break;
	case LCD_KIT_RST:
		g_lcd_kit_gpio_plugin = g_pwr_off_optimize_cfg.lcd_rst->buf[POWER_NUM];
		break;
	case LCD_KIT_VDD:
		g_lcd_kit_gpio_plugin = g_pwr_off_optimize_cfg.lcd_vdd->buf[POWER_NUM];
		break;
	default:
		LCD_KIT_ERR("not support type:%d\n", type);
		break;
	}
}

int lcd_kit_gpio_tx_plugin(uint32_t type, uint32_t op)
{
	uint32_t i;
	int ret;
	struct gpio_power_arra *gpio_cm = NULL;

	lcd_kit_get_gpio_num(type);

	for (i = 0; i < ARRAY_SIZE(gpio_power_plugin); i++) {
		if (gpio_power_plugin[i].oper == op) {
			gpio_cm = &gpio_power_plugin[i];
			break;
		}
	}
	if (i >= ARRAY_SIZE(gpio_power_plugin)) {
		LCD_KIT_ERR("not found cm from gpio_power\n");
		return LCD_KIT_FAIL;
	}
	if (!gpio_cm) {
		LCD_KIT_ERR("gpio_cm is null!\n");
		return LCD_KIT_FAIL;
	}
	if (gpio_cm->num > 0) {
		ret = lcd_kit_gpio_cmds_tx(gpio_cm->cm, gpio_cm->num);
		LCD_KIT_INFO("gpio:%d ,op:%d\n", *gpio_cm->cm->gpio, op);
		return ret;
	} else {
		LCD_KIT_INFO("gpio is not bigger than 0\n");
		return LCD_KIT_FAIL;
	}
}

static int lcd_kit_power_ctrl(uint32_t data,
	uint32_t gpio_type, uint32_t power_type)
{
	int ret = LCD_KIT_OK;
	struct lcd_kit_adapt_ops *adapt_ops = lcd_kit_get_adapt_ops();

	if (!adapt_ops) {
		LCD_KIT_ERR("can not register adapt_ops!\n");
		return LCD_KIT_FAIL;
	}
	if (data > 0) {
		LCD_KIT_ERR("do not handle power on event!\n");
		return LCD_KIT_FAIL;
	}
	switch (power_type) {
	case GPIO_MODE:
		if (adapt_ops->gpio_disable_plugin)
			ret = adapt_ops->gpio_disable_plugin(gpio_type);
		break;
	default:
		LCD_KIT_ERR("power mode is not support\n");
		return LCD_KIT_FAIL;
	}
	return ret;
}

static int lcd_kit_event_handler_plugin(int panel_id, uint32_t event,
	uint32_t data, uint32_t delay)
{
	int ret = LCD_KIT_OK;
	uint32_t power_type;
	uint32_t gpio_type;

	LCD_KIT_INFO("event = %u, data = %u, delay = %u\n", event, data, delay);
	switch (event) {
	case EVENT_VCI:
		gpio_type = LCD_KIT_VCI;
		power_type = g_pwr_off_optimize_cfg.lcd_vci->buf[POWER_MODE];
		ret = lcd_kit_power_ctrl(data, gpio_type, power_type);
		break;
	case EVENT_IOVCC:
		gpio_type = LCD_KIT_IOVCC;
		power_type = g_pwr_off_optimize_cfg.lcd_iovcc->buf[POWER_MODE];
		ret = lcd_kit_power_ctrl(data, gpio_type, power_type);
		break;
	case EVENT_RESET:
		gpio_type = LCD_KIT_RST;
		power_type = g_pwr_off_optimize_cfg.lcd_rst->buf[POWER_MODE];
		ret = lcd_kit_power_ctrl(data, gpio_type, power_type);
		break;
	case EVENT_VDD:
		gpio_type = LCD_KIT_VDD;
		power_type = g_pwr_off_optimize_cfg.lcd_vdd->buf[POWER_MODE];
		ret = lcd_kit_power_ctrl(data, gpio_type, power_type);
		break;
	case EVENT_NONE:
		LCD_KIT_INFO("none event\n");
		break;
	default:
		LCD_KIT_INFO("event not exist\n");
		break;
	}
	lcd_kit_delay(delay, LCD_KIT_WAIT_MS, true);
	return ret;
}

static void lcd_kit_power_off_optimize_work(struct work_struct *work)
{
	int ret;
	unsigned int i;
	struct lcd_kit_array_data *pevent = NULL;
	int panel_id;

	if (!g_pwr_off_optimize_cfg.working_flag) {
		LCD_KIT_INFO("work state is err!\n");
		return;
	}

	LCD_KIT_INFO("power off work start!\n");
	panel_id = g_pwr_off_optimize_cfg.working_flag - true;
	pevent = g_pwr_off_optimize_cfg.panel_power_off_event;
	for (i = 0; i < g_pwr_off_optimize_cfg.panel_power_off_seq_cnt; i++) {
		if (!pevent || !pevent->buf) {
			LCD_KIT_ERR("pevent is null!\n");
			g_pwr_off_optimize_cfg.working_flag = false;
			return;
		}
		ret = lcd_kit_event_handler_plugin(panel_id,
			pevent->buf[EVENT_NUM],
			pevent->buf[EVENT_DATA],
			pevent->buf[EVENT_DELAY]);
		if (ret) {
			LCD_KIT_ERR("send event 0x%x error!\n", pevent->buf[EVENT_NUM]);
			g_pwr_off_optimize_cfg.working_flag = false;
			break;
		}
		pevent++;
	}
	g_pwr_off_optimize_cfg.working_flag = false;
	lcd_kit_set_skip_power_on_off(false);
	complete_all(&power_off_done);
	LCD_KIT_INFO("power off work end!\n");
	return;
}

void lcd_kit_power_off_optimize_handle(int panel_id)
{
	if (!disp_info->pwr_off_optimize_info.support)
		return;

	if (g_pwr_off_optimize_cfg.working_flag) {
		LCD_KIT_ERR("last power off work still running!\n");
		return;
	}

	if (!lcd_kit_get_skip_power_on_off()) {
		lcd_kit_delay(disp_info->pwr_off_optimize_info.delay_time,
			LCD_KIT_WAIT_MS, true);
		LCD_KIT_ERR("is not power off work\n");
		return;
	}

	LCD_KIT_INFO("power off work init\n");
	g_pwr_off_optimize_cfg.working_flag = panel_id + true;
	g_pwr_off_optimize_cfg.panel_power_off_seq_cnt = power_seq->power_off_seq.cnt;
	g_pwr_off_optimize_cfg.panel_power_off_event = power_seq->power_off_seq.arry_data;
	g_pwr_off_optimize_cfg.lcd_iovcc = &power_hdl->lcd_iovcc;
	g_pwr_off_optimize_cfg.lcd_vci = &power_hdl->lcd_vci;
	g_pwr_off_optimize_cfg.lcd_vdd = &power_hdl->lcd_vdd;
	g_pwr_off_optimize_cfg.lcd_rst = &power_hdl->lcd_rst;
	schedule_delayed_work(&g_pwr_off_optimize_cfg.pwr_off_optimize_work,
		msecs_to_jiffies(disp_info->pwr_off_optimize_info.delay_time));
}

void lcd_kit_register_power_off_optimize(void)
{
	struct delayed_work *pwr_off_optimize_work = NULL;

	pwr_off_optimize_work = &g_pwr_off_optimize_cfg.pwr_off_optimize_work;
	INIT_DELAYED_WORK(pwr_off_optimize_work, lcd_kit_power_off_optimize_work);
	init_completion(&power_off_done);
	LCD_KIT_INFO("power off work regist!\n");
}