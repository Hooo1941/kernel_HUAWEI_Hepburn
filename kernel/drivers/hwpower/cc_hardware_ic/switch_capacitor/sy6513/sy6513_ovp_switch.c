// SPDX-License-Identifier: GPL-2.0
/*
 * sy6513_ovp_switch.c
 *
 * sy6513 ovp switch driver
 *
 * Copyright (c) 2023-2023 Huawei Technologies Co., Ltd.
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

#include "sy6513.h"
#include "sy6513_i2c.h"
#include <chipset_common/hwpower/hardware_channel/power_sw.h>
#include <chipset_common/hwpower/common_module/power_delay.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/common_module/power_printk.h>

#define HWLOG_TAG sy6513_ovp_switch
HWLOG_REGIST();

#define SY6513_WAIT_I2C_RESUME_TIMES    10
#define SY6513_SLEEP_TIME               50

#define SY6513_GATE_OFF                 0 /* IC default */
#define SY6513_GATE_ON                  1

static struct sy6513_device_info *g_sy6513_ovp_switch_di;

static void sy6513_wait_for_i2c_working(struct sy6513_device_info *di)
{
	int i;
	for (i = 0; i < SY6513_WAIT_I2C_RESUME_TIMES; i++) {
		if (!di->i2c_is_working) {
			hwlog_info("ic_%u i2c is not working\n", di->ic_role);
			power_msleep(SY6513_SLEEP_TIME, 0, NULL);
			continue;
		}
		break;
	}
}

static int sy6513_write_gate_ctrl(struct sy6513_device_info *di, u8 reg, u8 mask, u8 val)
{
	int ret;

	ret = sy6513_write_multi_mask(di, SY6513_PULL_DOWN_CONFIG_REG, mask, val);
	if (ret) {
		hwlog_err("write gate_ctrl reg failed\n");
		return ret;
	}
	ret = sy6513_read_byte(di, SY6513_PULL_DOWN_CONFIG_REG, &val);
	if (ret) {
		hwlog_err("read gate_ctrl reg failed\n");
		return ret;
	}

	hwlog_info("[gate_ctrl_wr] ic_%u reg[0x%x]=0x%x\n",
		di->ic_role, SY6513_PULL_DOWN_CONFIG_REG, val);
	return 0;
}

static int sy6513_set_wpcgate_sw(struct power_sw_attr *attr, int val)
{
	u8 mask;
	u8 wr = 0;
	struct sy6513_device_info *di = NULL;

	if (!attr || !attr->dev) {
		hwlog_err("set_ovpgate_sw: attr/dev null\n");
		return -ENODEV;
	}

	di = attr->dev;
	if (val == SY6513_GATE_ON) {
		wr = 1 << SY6513_WPC_G_EN_REVERSE_MODE_SHIFT;
		mask = SY6513_OVP_G_EN_REVERSE_MODE_MASK |
			SY6513_WPC_G_EN_REVERSE_MODE_MASK;
	} else if (val == SY6513_GATE_OFF) {
		mask = SY6513_WPC_G_EN_REVERSE_MODE_MASK;
	} else {
		hwlog_err("set_wpcgate_sw: err input=%d\n", val);
		return -EINVAL;
	}

	sy6513_wait_for_i2c_working(di);
	return sy6513_write_gate_ctrl(di, SY6513_PULL_DOWN_CONFIG_REG, mask, wr);
}

static int sy6513_get_wpcgate_sw(struct power_sw_attr *attr)
{
	int ret;
	u8 val = 0;
	int gate_state;
	struct sy6513_device_info *di = NULL;

	if (!attr || !attr->dev) {
		hwlog_err("get_ovpgate_sw: attr/dev null\n");
		return SY6513_GATE_ON;
	}

	di = attr->dev;
	ret = sy6513_read_byte(di, SY6513_PULL_DOWN_CONFIG_REG, &val);
	if (ret)
		return SY6513_GATE_ON;

	hwlog_info("[get_ovpgate_sw] ic_%u reg[0x%x]=0x%x\n",
		di->ic_role, SY6513_PULL_DOWN_CONFIG_REG, val);
	return (val & SY6513_WPC_G_EN_REVERSE_MODE_MASK) ? SY6513_GATE_ON : SY6513_GATE_OFF;
}

static void sy6513_free_wpcgate_sw(struct power_sw_attr *attr)
{
}

static int sy6513_set_ovpgate_sw(struct power_sw_attr *attr, int val)
{
	u8 mask;
	u8 wr = 0;
	struct sy6513_device_info *di = NULL;

	if (!attr || !attr->dev) {
		hwlog_err("set_ovpgate_sw: attr/dev null\n");
		return -ENODEV;
	}

	di = attr->dev;
	if (val == SY6513_GATE_ON) {
		wr = 1 << SY6513_OVP_G_EN_REVERSE_MODE_SHIFT;
		mask = SY6513_OVP_G_EN_REVERSE_MODE_MASK | SY6513_WPC_G_EN_REVERSE_MODE_MASK;
	} else if (val == SY6513_GATE_OFF) {
		mask = SY6513_OVP_G_EN_REVERSE_MODE_MASK;
	} else {
		hwlog_err("set_ovpgate_sw: err input=%d\n", val);
		return -EINVAL;
	}

	sy6513_wait_for_i2c_working(di);
	return sy6513_write_gate_ctrl(di, SY6513_PULL_DOWN_CONFIG_REG, mask, wr);
}

static int sy6513_get_ovpgate_sw(struct power_sw_attr *attr)
{
	int ret;
	u8 val = 0;
	int gate_state;
	struct sy6513_device_info *di = NULL;

	if (!attr || !attr->dev) {
		hwlog_err("get_ovpgate_sw: attr/dev null\n");
		return SY6513_GATE_ON;
	}

	di = attr->dev;
	ret = sy6513_read_byte(di, SY6513_PULL_DOWN_CONFIG_REG, &val);
	if (ret)
		return SY6513_GATE_ON;

	hwlog_info("[get_ovpgate_sw] ic_%u reg[0x%x]=0x%x\n",
		di->ic_role, SY6513_PULL_DOWN_CONFIG_REG, val);
	return (val & SY6513_OVP_G_EN_REVERSE_MODE_MASK) ?
		SY6513_GATE_ON : SY6513_GATE_OFF;
}

static void sy6513_free_ovpgate_sw(struct power_sw_attr *attr)
{
}

void sy6513_wpcgate_sw_register(struct sy6513_device_info *di)
{
	struct power_sw sw;

	if (di->wpcsw_attr.num < 0)
		return;

	sw.attr.dev = (void *)di;
	sw.attr.num = di->wpcsw_attr.num;
	sw.attr.dflt = di->wpcsw_attr.dflt;
	sw.attr.label = di->wpcsw_attr.label;
	sw.set = sy6513_set_wpcgate_sw;
	sw.get = sy6513_get_wpcgate_sw;
	sw.free = sy6513_free_wpcgate_sw;

	(void)power_sw_register(sw.attr.num, &sw);
}

void sy6513_ovpgate_sw_register(struct sy6513_device_info *di)
{
	struct power_sw sw;

	if (di->ovpsw_attr.num < 0)
		return;

	sw.attr.dev = (void *)di;
	sw.attr.num = di->ovpsw_attr.num;
	sw.attr.dflt = di->ovpsw_attr.dflt;
	sw.attr.label = di->ovpsw_attr.label;
	sw.set = sy6513_set_ovpgate_sw;
	sw.get = sy6513_get_ovpgate_sw;
	sw.free = sy6513_free_ovpgate_sw;

	(void)power_sw_register(sw.attr.num, &sw);
}

void sy6513_power_sw_register(struct sy6513_device_info *di)
{
	sy6513_wpcgate_sw_register(di);
	sy6513_ovpgate_sw_register(di);
}

static void __sy6513_parse_sw_attr(struct device_node *np, const char *prop, struct power_sw_attr *sw_attr)
{
	int i;
	int len;
	const char *str = NULL;

	len = power_dts_read_count_strings(power_dts_tag(HWLOG_TAG), np, prop, 1, POWER_SW_ATTR_TOTAL);
	if (len <= 0) {
		sw_attr->num = -1;
		return;
	}

	for (i = 0; i < len; i++) {
		if (power_dts_read_string_index(power_dts_tag(HWLOG_TAG), np, prop, i, &str))
			goto err;
		switch (i %  POWER_SW_ATTR_TOTAL) {
		case POWER_SW_ATTR_LABEL:
			sw_attr->label = str;
			break;
		case POWER_SW_ATTR_INDEX:
			if (kstrtoint(str, 0, &sw_attr->num))
				goto err;
			break;
		case POWER_SW_ATTR_DFLT:
			if (kstrtoint(str, 0, &sw_attr->dflt))
				goto err;
			break;
		case POWER_SW_ATTR_EN:
			if (kstrtoint(str, 0, &sw_attr->en))
				goto err;
			break;
		default:
			goto err;
		}
	}

	return;

err:
	hwlog_err("parse_sw_attr: synx err\n");
	sw_attr->num = -1;
}

void sy6513_parse_sw_attr(struct device_node *np, struct sy6513_device_info *di)
{
	__sy6513_parse_sw_attr(np, "ovpgate_sw", &di->ovpsw_attr);
	__sy6513_parse_sw_attr(np, "wpcgate_sw", &di->wpcsw_attr);
}
