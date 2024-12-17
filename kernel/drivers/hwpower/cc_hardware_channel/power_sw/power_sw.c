// SPDX-License-Identifier: GPL-2.0
/*
 * power_sw.c
 *
 * power_switch module, support gipo and other implements
 *
 * Copyright (c) 2022-2022 Huawei Technologies Co., Ltd.
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

#include <chipset_common/hwpower/hardware_channel/power_sw.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <linux/of_gpio.h>

#define HWLOG_TAG power_sw
HWLOG_REGIST();

#define POWER_SW_NUMS       8

static struct power_sw g_pwr_sw[POWER_SW_NUMS];

bool power_sw_ready(int idx)
{
	if ((idx < 0) || (idx >= POWER_SW_NUMS))
		return false;

	return g_pwr_sw[idx].ready;
}

int power_sw_set_output(int idx, int status)
{
	if ((idx < 0) || (idx >= POWER_SW_NUMS) ||
		!g_pwr_sw[idx].ready || !g_pwr_sw[idx].set)
		return -EINVAL;

	hwlog_info("[set_output] %s set %s\n", g_pwr_sw[idx].attr.label,
		status == POWER_SW_ON ? "on" : "off");
	return g_pwr_sw[idx].set(&g_pwr_sw[idx].attr, status);
}

int power_sw_get_output(int idx)
{
	if ((idx < 0) || (idx >= POWER_SW_NUMS) ||
		!g_pwr_sw[idx].ready || !g_pwr_sw[idx].get)
		return POWER_SW_ON;

	return g_pwr_sw[idx].get(&g_pwr_sw[idx].attr);
}

void power_sw_free(int idx)
{
	if ((idx < 0) || (idx >= POWER_SW_NUMS) ||
		!g_pwr_sw[idx].ready || !g_pwr_sw[idx].free)
		return;

	g_pwr_sw[idx].free(&g_pwr_sw[idx].attr);
	g_pwr_sw[idx].ready = false;
}

int power_sw_register(int idx, struct power_sw *sw)
{
	if (!sw || !sw->set || !sw->get || !sw->free) {
		hwlog_err("register: sw/attr/ops null\n");
		return -ENODEV;
	}
	if ((idx < 0) || (idx >= POWER_SW_NUMS) || g_pwr_sw[idx].ready) {
		hwlog_err("register: idx=%d out-of-range/occupied\n", idx);
		return -EINVAL;
	}

	memcpy(&g_pwr_sw[idx].attr, &sw->attr, sizeof(g_pwr_sw[idx].attr));
	g_pwr_sw[idx].set = sw->set;
	g_pwr_sw[idx].get = sw->get;
	g_pwr_sw[idx].free = sw->free;
	g_pwr_sw[idx].ready = true;
	g_pwr_sw[idx].set(&g_pwr_sw[idx].attr, g_pwr_sw[idx].attr.dflt ==
		g_pwr_sw[idx].attr.en ? POWER_SW_ON : POWER_SW_OFF);

	hwlog_info("sw[%d] label:%s registered\n", idx, sw->attr.label);
	return 0;
}

static int power_sw_get_idx_by_label(const char *label)
{
	int idx;

	for (idx = 0; idx < POWER_SW_NUMS; idx++) {
		if (!g_pwr_sw[idx].ready)
			continue;
		if (strstr(g_pwr_sw[idx].attr.label, label))
			break;
	}

	return idx;
}

int power_sw_set_output_by_label(const char *label, int status, bool force)
{
	int idx;
	int cur_status;

	if (!label)
		return -EINVAL;

	idx = power_sw_get_idx_by_label(label);
	cur_status = power_sw_get_output(idx);
	if (!force && (status == cur_status))
		return 0;
	return power_sw_set_output(idx, status);
}

int power_sw_get_output_by_label(const char *label)
{
	if (!label)
		return POWER_SW_OFF;

	return power_sw_get_output(power_sw_get_idx_by_label(label));
}

static int power_sw_set_by_gpio(struct power_sw_attr *attr, int status)
{
	int ret;

	if (!attr) {
		hwlog_err("set_by_gpio: attr null\n");
		return -EINVAL;
	}

	if (!attr->num) {
		attr->status_set = status;
		return 0;
	}

	ret = gpio_direction_output(attr->num,
		status == POWER_SW_ON ? attr->en : !attr->en);
	hwlog_info("[set_by_gpio] gpio_%d %s now\n", attr->num,
		gpio_get_value(attr->num) ? "high" : "low");

	return ret;
}

static int power_sw_get_by_gpio(struct power_sw_attr *attr)
{
	int gpio_val;

	if (!attr) {
		hwlog_err("get_by_gpio: attr null\n");
		return POWER_SW_OFF;
	}

	if (!attr->num)
		return attr->status_set;

	gpio_val = gpio_get_value(attr->num);
	return gpio_val == attr->en ? POWER_SW_ON : POWER_SW_OFF;
}

bool power_sw_exist_by_label(const char *label)
{
	if (!label)
		return false;

	return power_sw_ready(power_sw_get_idx_by_label(label));
}

static void power_sw_free_by_gpio(struct power_sw_attr *attr)
{
	if (!attr) {
		hwlog_err("free_by_gpio: attr null\n");
		return;
	}

	gpio_free(attr->num);
}

static int power_sw_init_gpio(struct device_node *np)
{
	int i;
	int sw_idx;
	int gpio_count;
	struct power_sw sw;

	gpio_count = of_gpio_count(np);
	if (gpio_count <= 0)
		return 0;

	for (i = 0; i < gpio_count; i++) {
		if (power_dts_read_string_index(power_dts_tag(HWLOG_TAG),
			np, "labels", i, &sw.attr.label)) {
			hwlog_err("init_gpio: parse label failed\n");
			continue;
		}
		sw.attr.num = of_get_gpio(np, i);
		hwlog_info("[init_gpio] num=%d\n", sw.attr.num);
		if (!gpio_is_valid(sw.attr.num)) {
			hwlog_err("init_gpio: gpio%d invalid\n", sw.attr.num);
			continue;
		}
		if (gpio_request(sw.attr.num, sw.attr.label)) {
			hwlog_err("init_gpio: request gpio%d failed\n", sw.attr.num);
			continue;
		}
		if (power_dts_read_u32_index(power_dts_tag(HWLOG_TAG),
			np, "dlfts", i, &sw.attr.dflt) ||
			power_dts_read_u32_index(power_dts_tag(HWLOG_TAG),
				np, "en", i, &sw.attr.en) ||
			power_dts_read_u32_index(power_dts_tag(HWLOG_TAG),
				np, "indexs", i, &sw_idx)) {
			hwlog_err("init_gpio: get dflt_val/idx failed\n");
			gpio_free(sw.attr.num);
			continue;
		}
		sw.set = power_sw_set_by_gpio;
		sw.get = power_sw_get_by_gpio;
		sw.free = power_sw_free_by_gpio;
		if (power_sw_register(sw_idx, &sw))
			gpio_free(sw.attr.num);
	}

	return 0;
}

static int power_sw_probe(struct platform_device *pdev)
{
	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	return power_sw_init_gpio(pdev->dev.of_node);
}

static const struct of_device_id power_sw_match_table[] = {
	{
		.compatible = "huawei,power_sw",
		.data = NULL,
	},
	{},
};

static struct platform_driver power_sw_driver = {
	.probe = power_sw_probe,
	.driver = {
		.name = "huawei,power_sw",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(power_sw_match_table),
	},
};

static int __init power_sw_init(void)
{
	return platform_driver_register(&power_sw_driver);
}

static void __exit power_sw_exit(void)
{
	platform_driver_unregister(&power_sw_driver);
}

fs_initcall_sync(power_sw_init);
module_exit(power_sw_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("power switch module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
