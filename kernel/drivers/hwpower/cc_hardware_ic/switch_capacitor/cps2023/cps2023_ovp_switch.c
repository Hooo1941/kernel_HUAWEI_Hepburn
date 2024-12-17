// SPDX-License-Identifier: GPL-2.0
/*
 * cps2023_ovp_switch.c
 *
 * cps2023 ovp switch driver
 *
 * Copyright (c) 2022-2023 Huawei Technologies Co., Ltd.
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

#include "cps2023.h"
#include "cps2023_i2c.h"
#include <chipset_common/hwpower/hardware_channel/power_sw.h>
#include <chipset_common/hwpower/common_module/power_delay.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/common_module/power_printk.h>

#define HWLOG_TAG cps2023_ovp_switch
HWLOG_REGIST();

#define CPS2023_WAIT_I2C_RESUME_TIMES     10
#define CPS2023_SLEEP_TIME                50

static void cps2023_wait_for_i2c_working(struct cps2023_device_info *di)
{
	int i;

	for (i = 0; i < CPS2023_WAIT_I2C_RESUME_TIMES; i++) {
		if (!di->i2c_is_working) {
			hwlog_info("ic_%u i2c is not working\n", di->ic_role);
			power_msleep(CPS2023_SLEEP_TIME, 0, NULL);
			continue;
		}
		break;
	}
}

static int cps2023_write_gate_ctrl(struct cps2023_device_info *di, u8 reg, u8 mask, u8 val)
{
	int ret;

	ret = cps2023_write_multi_mask(di, CPS2023_CTRL1_REG, mask, val);
	if (ret) {
		hwlog_err("write gate_ctrl reg failed\n");
		return ret;
	}
	ret = cps2023_read_byte(di, CPS2023_CTRL1_REG, &val);
	if (ret) {
		hwlog_err("read gate_ctrl reg failed\n");
		return ret;
	}

	hwlog_info("[gate_ctrl_wr] ic_%u reg[0x%x]=0x%x\n",
		di->ic_role, CPS2023_CTRL1_REG, val);
	return 0;
}

static int cps2023_set_wpcgate_sw(struct power_sw_attr *attr, int status)
{
	u8 mask;
	u8 wr;
	struct cps2023_device_info *di = NULL;

	if (!attr || !attr->dev) {
		hwlog_err("set_wpcgate_sw: attr/dev null\n");
		return -ENODEV;
	}

	di = attr->dev;
	if (status == POWER_SW_ON) {
		wr = CPS2023_CTRL1_ACDRV_MANUAL_EN_MASK |
			CPS2023_CTRL1_WPCGATE_EN_MASK;
		mask = CPS2023_CTRL1_ACDRV_MANUAL_EN_MASK |
			CPS2023_CTRL1_WPCGATE_EN_MASK |
			CPS2023_CTRL1_OVPGATE_EN_MASK;
	} else if (status == POWER_SW_OFF) {
		wr = 1 << CPS2023_CTRL1_ACDRV_MANUAL_EN_SHIFT;
		mask = CPS2023_CTRL1_ACDRV_MANUAL_EN_MASK |
			CPS2023_CTRL1_WPCGATE_EN_MASK;
	} else {
		hwlog_err("set_wpcgate_sw: err input=%d\n", status);
		return -EINVAL;
	}

	cps2023_wait_for_i2c_working(di);
	return cps2023_write_gate_ctrl(di, CPS2023_CTRL1_REG, mask, wr);
}

static int cps2023_get_wpcgate_sw(struct power_sw_attr *attr)
{
	int ret;
	u8 val = 0;

	struct cps2023_device_info *di = NULL;

	if (!attr || !attr->dev) {
		hwlog_err("get_wpcgate_sw: attr/dev null\n");
		return POWER_SW_ON;
	}

	di = attr->dev;
	ret = cps2023_read_byte(di, CPS2023_CTRL5_REG, &val);
	if (ret)
		return POWER_SW_ON;

	hwlog_info("[get_wpcgate_sw] ic_%u reg[0x%x]=0x%x\n",
		di->ic_role, CPS2023_CTRL5_REG, val);
	return (val & CPS2023_CTRL5_WPCGATE_STAT_MASK) ?
		POWER_SW_ON : POWER_SW_OFF;
}

static void cps2023_free_wpcgate_sw(struct power_sw_attr *attr)
{
}

static int cps2023_set_ovpgate_sw(struct power_sw_attr *attr, int status)
{
	u8 mask;
	u8 wr;
	struct cps2023_device_info *di = NULL;

	if (!attr || !attr->dev) {
		hwlog_err("set_ovpgate_sw: attr/dev null\n");
		return -ENODEV;
	}

	di = attr->dev;
	if (status == POWER_SW_ON) {
		wr = CPS2023_CTRL1_ACDRV_MANUAL_EN_MASK |
			CPS2023_CTRL1_OVPGATE_EN_MASK;
		mask = CPS2023_CTRL1_ACDRV_MANUAL_EN_MASK |
			CPS2023_CTRL1_WPCGATE_EN_MASK |
			CPS2023_CTRL1_OVPGATE_EN_MASK;
	} else if (status == POWER_SW_OFF) {
		wr = 1 << CPS2023_CTRL1_ACDRV_MANUAL_EN_SHIFT;
		mask = CPS2023_CTRL1_ACDRV_MANUAL_EN_MASK |
			CPS2023_CTRL1_OVPGATE_EN_MASK;
	} else {
		hwlog_err("set_ovpgate_sw: err input=%d\n", status);
		return -EINVAL;
	}

	cps2023_wait_for_i2c_working(di);
	return cps2023_write_gate_ctrl(di, CPS2023_CTRL1_REG, mask, wr);
}

static int cps2023_get_ovpgate_sw(struct power_sw_attr *attr)
{
	int ret;
	u8 val = 0;
	struct cps2023_device_info *di = NULL;

	if (!attr || !attr->dev) {
		hwlog_err("get_ovpgate_sw: attr/dev null\n");
		return POWER_SW_ON;
	}

	di = attr->dev;
	ret = cps2023_read_byte(di, CPS2023_CTRL5_REG, &val);
	if (ret)
		return POWER_SW_ON;

	hwlog_info("[get_ovpgate_sw] ic_%u reg[0x%x]=0x%x\n",
		di->ic_role, CPS2023_CTRL5_REG, val);
	return (val & CPS2023_CTRL5_OVPGATE_STAT_MASK) ?
		POWER_SW_ON : POWER_SW_OFF;
}

static void cps2023_free_ovpgate_sw(struct power_sw_attr *attr)
{
}

static void cps2023_wpcgate_sw_register(struct cps2023_device_info *di)
{
	struct power_sw sw;

	if (di->wpcsw_attr.num < 0)
		return;

	sw.attr.dev = (void *)di;
	sw.attr.num = di->wpcsw_attr.num;
	sw.attr.dflt = di->wpcsw_attr.dflt;
	sw.attr.en = di->wpcsw_attr.en;
	sw.attr.label = di->wpcsw_attr.label;
	sw.set = cps2023_set_wpcgate_sw;
	sw.get = cps2023_get_wpcgate_sw;
	sw.free = cps2023_free_wpcgate_sw;

	(void)power_sw_register(sw.attr.num, &sw);
}

static void cps2023_ovpgate_sw_register(struct cps2023_device_info *di)
{
	struct power_sw sw;

	if (di->ovpsw_attr.num < 0)
		return;

	sw.attr.dev = (void *)di;
	sw.attr.num = di->ovpsw_attr.num;
	sw.attr.dflt = di->ovpsw_attr.dflt;
	sw.attr.en = di->ovpsw_attr.en;
	sw.attr.label = di->ovpsw_attr.label;
	sw.set = cps2023_set_ovpgate_sw;
	sw.get = cps2023_get_ovpgate_sw;
	sw.free = cps2023_free_ovpgate_sw;

	(void)power_sw_register(sw.attr.num, &sw);
}

void cps2023_power_sw_register(struct cps2023_device_info *di)
{
	cps2023_wpcgate_sw_register(di);
	cps2023_ovpgate_sw_register(di);
}

static void __cps2023_parse_sw_attr(struct device_node *np,
	const char *prop, struct power_sw_attr *sw_attr)
{
	int i, len, col;
	const char *str = NULL;

	len = power_dts_read_count_strings(power_dts_tag(HWLOG_TAG), np,
		prop, 1, POWER_SW_ATTR_TOTAL);
	if (len <= 0) {
		sw_attr->num = -1;
		return;
	}

	for (i = 0; i < len; i++) {
		if (power_dts_read_string_index(power_dts_tag(HWLOG_TAG), np,
			prop, i, &str))
			return;
		col = i % POWER_SW_ATTR_TOTAL;
		switch (col) {
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

void cps2023_parse_sw_attr(struct device_node *np,
	struct cps2023_device_info *di)
{
	 __cps2023_parse_sw_attr(np, "ovpgate_sw", &di->ovpsw_attr);
	 __cps2023_parse_sw_attr(np, "wpcgate_sw", &di->wpcsw_attr);
}
