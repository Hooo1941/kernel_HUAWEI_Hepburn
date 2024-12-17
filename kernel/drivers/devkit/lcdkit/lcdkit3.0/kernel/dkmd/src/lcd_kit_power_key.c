/*
 * lcd_kit_power_key.c
 *
 * lcdkit power key event function for lcd driver
 *
 * Copyright (c) 2022-2024 Huawei Technologies Co., Ltd.
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

#include "lcd_kit_power_key.h"
#include "lcd_kit_disp.h"
#include <platform_include/basicplatform/linux/mfd/pmic_platform.h>
#include <platform_include/basicplatform/linux/powerkey_event.h>
#include <huawei_platform/comb_key/comb_key_event.h>

static int g_restart_type = PRESS_POWER_VOL_DOWN;

/*
 * Forced power-off will cause display error,
 * so need to send 10 command
 * before the long press power off first
 */
static void power_off_work(struct work_struct *work)
{
	int panel_id = lcd_kit_get_active_panel_id();
	struct lcd_kit_adapt_ops *ops = NULL;
	int ret;

	(void *)work;
	if (!disp_info) {
		LCD_KIT_ERR("disp info null!\n");
		return;
	}
	if (disp_info->pwrkey_press.long_press_flag == false) {
		LCD_KIT_INFO("long press flag false!\n");
		return;
	}
	ops = lcd_kit_get_adapt_ops();
	if (!ops) {
		LCD_KIT_ERR("get adapt ops null!\n");
		return;
	}
	if (!lcd_kit_get_panel_on_state(panel_id)) {
		LCD_KIT_INFO("lcd is power off, stop send commands!\n");
		return;
	}
	LCD_KIT_INFO("long press, power off lcd!\n");
	if (disp_info->pwrkey_press.esd_status == PWR_OFF_NEED_DISABLE_ESD) {
		LCD_KIT_INFO("pwr key disabled esd!\n");
		disp_info->esd_enable = 0;
		disp_info->pwrkey_press.esd_status = PWR_OFF_DISABLED_ESD;
	}

	if (ops->mipi_tx) {
		ret = ops->mipi_tx(panel_id, NULL,
			&disp_info->pwrkey_press.cmds);
		LCD_KIT_DEBUG("ret is %d\n", ret);
	}
	if (disp_info->pwrkey_press.power_off_flag) {
		LCD_KIT_INFO("lcd need power off!\n");
		if (common_ops->panel_off_lp)
			common_ops->panel_off_lp(panel_id, NULL);
		if (common_ops->panel_power_off)
			common_ops->panel_power_off(panel_id, NULL);
	}
}

static int __init early_parse_restart_type_cmdline(char *arg)
{
	if (!arg) {
		LCD_KIT_ERR("arg is NULL\n");
		return 0;
	}
	if (strncmp(arg, "0", 1) == 0)
		g_restart_type = PRESS_POWER_ONLY;
	else if (strncmp(arg, "1", 1) == 0)
		g_restart_type = PRESS_POWER_VOL_DOWN;
	else if (strncmp(arg, "2", 1) == 0)
		g_restart_type = PRESS_POWER_VOL_UP;

	LCD_KIT_INFO("g_restart_type = %d\n", g_restart_type);
	return 0;
}
early_param(POWERKEY_RESTART_TYPE_PROP, early_parse_restart_type_cmdline);

static unsigned int get_power_off_timer_val(int panel_id)
{
	unsigned int ret_time;
	unsigned int press_time;
	unsigned int val;
	unsigned int addr;

	if (!disp_info)
		return LONG_PRESS_10S_LCD_TIMER_LEN;
	addr = disp_info->pwrkey_press.rst_addr;
	press_time = pmic_read_reg(addr);
	LCD_KIT_INFO("press_time %u!\n", press_time);
	val = press_time & LONG_PRESS_RST_CONFIG_BIT;
	switch (val) {
	case LONG_PRESS_RST_CONFIG1:
		if (disp_info->pwrkey_press.configtime1)
			ret_time = disp_info->pwrkey_press.configtime1;
		else
			ret_time = LONG_PRESS_8S_LCD_TIMER_LEN;
		break;
	case LONG_PRESS_RST_CONFIG2:
		if (disp_info->pwrkey_press.configtime2)
			ret_time = disp_info->pwrkey_press.configtime2;
		else
			ret_time = LONG_PRESS_10S_LCD_TIMER_LEN;
		break;
	case LONG_PRESS_RST_CONFIG3:
		if (disp_info->pwrkey_press.configtime3)
			ret_time = disp_info->pwrkey_press.configtime3;
		else
			ret_time = LONG_PRESS_30S_LCD_TIMER_LEN;
		break;
	case LONG_PRESS_RST_CONFIG4:
		if (disp_info->pwrkey_press.configtime4)
			ret_time = disp_info->pwrkey_press.configtime4;
		else
			ret_time = LONG_PRESS_60S_LCD_TIMER_LEN;
		break;
	default:
		ret_time = LONG_PRESS_60S_LCD_TIMER_LEN;
		break;
	}
	return ret_time;
}

static int pwrkey_and_comb_key_press_event_notifier(struct notifier_block *pwrkey_event_nb,
	unsigned long event, void *data)
{
	struct notifier_block *p = pwrkey_event_nb;
	void *pd = data;
	unsigned int time;
	int panel_id = lcd_kit_get_active_panel_id();

	if (!disp_info)
		return LCD_KIT_OK;
	if (disp_info->pwrkey_press.support == false) {
		LCD_KIT_INFO("not support this func!\n");
		return LCD_KIT_OK;
	}

	if (g_restart_type == PRESS_POWER_ONLY)
		time = disp_info->pwrkey_press.timer_val;
	else
		time = disp_info->pwrkey_press.timer_val + 6000; /* 6s */

	if ((g_restart_type == PRESS_POWER_ONLY && event == PRESS_KEY_6S) ||
		(g_restart_type != PRESS_POWER_ONLY && event == COMB_KEY_PRESS_DOWN)) {
		disp_info->pwrkey_press.long_press_flag = true;
		schedule_delayed_work(&disp_info->pwrkey_press.pf_work,
			msecs_to_jiffies(time));
	} else if ((g_restart_type == PRESS_POWER_ONLY && event == PRESS_KEY_UP) ||
		(g_restart_type != PRESS_POWER_ONLY && event == COMB_KEY_PRESS_RELEASE)) {
		if (disp_info->pwrkey_press.long_press_flag == false)
			return LCD_KIT_OK;
		if (disp_info->pwrkey_press.esd_status == PWR_OFF_DISABLED_ESD) {
			disp_info->pwrkey_press.esd_status = PWR_OFF_NEED_DISABLE_ESD;
			disp_info->esd_enable = 1;
			LCD_KIT_INFO("pwr key enable esd!\n");
		}
		disp_info->pwrkey_press.long_press_flag = false;
		cancel_delayed_work_sync(&disp_info->pwrkey_press.pf_work);
	} else {
		LCD_KIT_INFO("event %d is not support!\n", event);
	}
	return LCD_KIT_OK;
}

void lcd_kit_register_power_key_notify(int panel_id)
{
	int ret;
	struct delayed_work *p_work = NULL;
	static bool pwroff_config_flag = false;

	if (!disp_info)
		return;
	if (disp_info->pwrkey_press.support == false)
		return;

	p_work = &disp_info->pwrkey_press.pf_work;
	INIT_DELAYED_WORK(p_work, power_off_work);
	disp_info->pwrkey_press.timer_val = get_power_off_timer_val(panel_id);
	if (pwroff_config_flag) {
		LCD_KIT_ERR("power_key notifier has been registed!\n");
		return;
	}
	if (g_restart_type == PRESS_POWER_ONLY) {
		disp_info->pwrkey_press.nb.notifier_call = pwrkey_and_comb_key_press_event_notifier;
		ret = powerkey_register_notifier(&disp_info->pwrkey_press.nb);
		if (ret < 0)
			LCD_KIT_ERR("register power_key notifier failed!\n");
	} else {
#ifdef CONFIG_POWERKEY_SPMI
#ifdef CONFIG_KEYBOARD_PMIC_VOLUME_KEY
		disp_info->pwrkey_press.nb.notifier_call = pwrkey_and_comb_key_press_event_notifier;
		ret = comb_key_register_notifier(&disp_info->pwrkey_press.nb);
		if (ret < 0)
			LCD_KIT_ERR("register comb_key notifier failed!\n");
#endif
#endif
	}
	pwroff_config_flag = true;
}

