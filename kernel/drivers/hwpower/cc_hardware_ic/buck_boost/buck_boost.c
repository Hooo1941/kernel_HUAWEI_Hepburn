// SPDX-License-Identifier: GPL-2.0
/*
 * buck_boost.c
 *
 * buck_boost driver
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
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

#include <securec.h>
#include <chipset_common/hwpower/hardware_ic/buck_boost.h>
#include <chipset_common/hwpower/common_module/power_algorithm.h>
#include <chipset_common/hwpower/common_module/power_debug.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/common_module/power_gpio.h>
#include <chipset_common/hwpower/common_module/power_dts.h>

#define HWLOG_TAG buck_boost
HWLOG_REGIST();

enum buck_boost_vout_vote_type {
	VOUT_VOTE_TYPE_BEGIN = 0,
	VOUT_VOTE_SET_MAX = VOUT_VOTE_TYPE_BEGIN,
	VOUT_VOTE_SET_MIN,
	VOUT_VOTE_TYPE_END,
};

static struct buck_boost_dev *g_buck_boost_di;

static const struct buck_boost_device_data g_buck_boost_device_data[] = {
	{ BBST_DEVICE_ID_TPS63810_MAIN, "tps63810" },
	{ BBST_DEVICE_ID_TPS63810_AUX, "tps63810_aux" },
	{ BBST_DEVICE_ID_MAX77813_MAIN, "max77813" },
	{ BBST_DEVICE_ID_BBST_GPIO, "bbst_gpio" },
};

static const struct buck_boost_device_data g_buck_boost_user_data[] = {
	{ BBST_USER_LPM, "low_power" },
	{ BBST_USER_CAMERA, "camera" },
	{ BBST_USER_BOOST_5V, "boost_5v" },
	{ BBST_USER_BOOST_RSMC, "rsmc" },
	{ BBST_USER_BOOST_HW_SATE, "hw_sate" },
	{ BBST_USER_BOOST_HX, "hx" },
};

static int buck_boost_get_device_id(const char *str)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(g_buck_boost_device_data); i++) {
		if (!strncmp(str, g_buck_boost_device_data[i].name,
			strlen(str)) && !g_buck_boost_di->t_ops[i])
			return g_buck_boost_device_data[i].id;
	}

	return -EINVAL;
}

static int buck_boost_get_ops_id(unsigned int type)
{
	int i;
	struct buck_boost_ops *ops = NULL;

	for (i = 0; i < BBST_DEVICE_ID_END; i++) {
		ops = g_buck_boost_di->t_ops[i];
		if (!ops || !(ops->bbst_type & type))
			continue;
		break;
	}
	if (i >= BBST_DEVICE_ID_END)
		return -EPERM;

	return i;
}

static struct buck_boost_ops *buck_boost_get_ops(unsigned int type)
{
	int id;

	if (!g_buck_boost_di) {
		hwlog_err("g_buck_boost_di is null\n");
		return NULL;
	}
	if (g_buck_boost_di->ops && (g_buck_boost_di->ops->bbst_type & type))
		return g_buck_boost_di->ops;

	if (g_buck_boost_di->retry_cnt >= BBST_GET_OPS_RETRY_CNT) {
		hwlog_err("retry too many times\n");
		return NULL;
	}

	id = buck_boost_get_ops_id(type);
	if (id < 0) {
		g_buck_boost_di->retry_cnt++;
		return NULL;
	}

	g_buck_boost_di->retry_cnt = 0;
	g_buck_boost_di->ops = g_buck_boost_di->t_ops[id];
	return g_buck_boost_di->ops;
}

int buck_boost_ops_register(struct buck_boost_ops *ops)
{
	int dev_id;

	if (!g_buck_boost_di || !ops || !ops->bbst_name) {
		hwlog_err("g_buck_boost_di or ops or chip_name is null\n");
		return -EPERM;
	}

	dev_id = buck_boost_get_device_id(ops->bbst_name);
	if (dev_id < 0) {
		hwlog_err("%s ops register fail\n", ops->bbst_name);
		return -EINVAL;
	}

	g_buck_boost_di->t_ops[dev_id] = ops;
	hwlog_info("%d:%s ops register ok\n", dev_id, ops->bbst_name);
	return 0;
}

static int buck_boost_gpio_set_vout(unsigned int vout, void *dev_data)
{
	int i;
	unsigned int value;
	struct buck_boost_dev *di = dev_data;

	if (!di) {
		hwlog_err("di is null\n");
		return -ENODEV;
	}

	if (!di->bbst_gpio_vout || !di->vout_table || (di->gpio_vout_num <= 0) ||
		(di->vout_table_size <= 0)) {
		hwlog_info("bbst_gpio_set_vout is ok\n");
		return 0;
	}

	for (i = 0; i < di->vout_table_size; i++) {
		if (vout <= di->vout_table[i].output_voltage) {
			value = di->vout_table[i].gpio_value;
			break;
		}
	}
	if (i >= di->vout_table_size)
		value = di->vout_table[di->vout_table_size - 1].gpio_value;

	for (i = 0; i < di->gpio_vout_num; i++) {
		gpio_set_value(di->bbst_gpio_vout[i], (value & BIT(i)) >> i);
		hwlog_info("set gpio_%d output %u, gpio_val=%d\n", di->bbst_gpio_vout[i],
			(value & BIT(i)) >> i, gpio_get_value(di->bbst_gpio_vout[i]));
	}
	return 0;
}

static bool buck_boost_gpio_power_good(void *dev_data)
{
	struct buck_boost_dev *di = dev_data;

	if (!di) {
		hwlog_err("di is null\n");
		return false;
	}

	hwlog_info("bbst_gpio_power_good is ok\n");
	return true;
}

static bool buck_boost_gpio_set_enable(unsigned int value, void *dev_data)
{
	int gpio_val;
	struct buck_boost_dev *di = dev_data;

	if (!di) {
		hwlog_err("di is null\n");
		return false;
	}

	if (!gpio_is_valid(di->bbst_gpio_en))
		return false;

	gpio_set_value(di->bbst_gpio_en, value);
	gpio_val = gpio_get_value(di->bbst_gpio_en);
	if ((int)value != gpio_val) {
		hwlog_err("set bbst_gpio_enable gpio fail\n");
		return false;
	}

	hwlog_info("bbst_gpio_enable gpio %s now\n", gpio_val ? "high" : "low");
	return true;
}

static struct buck_boost_ops buck_boost_gpio_ops = {
	.bbst_type = BBST_TYPE_MAIN,
	.bbst_name = "bbst_gpio",
	.pwr_good = buck_boost_gpio_power_good,
	.set_vout = buck_boost_gpio_set_vout,
	.set_enable = buck_boost_gpio_set_enable,
};

int buck_boost_set_pwm_enable(unsigned int enable, unsigned int type)
{
	struct buck_boost_ops *l_ops = buck_boost_get_ops(type);

	if (!l_ops)
		return -EPERM;

	if (!l_ops->set_pwm_enable) {
		hwlog_err("set_pwm_enable is null\n");
		return -EPERM;
	}

	return l_ops->set_pwm_enable(enable, l_ops->dev_data);
}

int buck_boost_set_vout(unsigned int vol, unsigned int user)
{
	int i;
	unsigned int temp_vol = 0;
	struct buck_boost_ops *l_ops = buck_boost_get_ops(BBST_TYPE_MAIN);
	struct buck_boost_dev *di = g_buck_boost_di;

	if (!di || !l_ops || (user < BBST_USER_BEGIN) || (user >= BBST_USER_END) ||
		(di->vout_vote_type >= VOUT_VOTE_TYPE_END))
		return -EPERM;

	if (!l_ops->set_vout) {
		hwlog_err("set_vout is null\n");
		return -EPERM;
	}

	hwlog_info("%s vol=%u, user=%u\n", __func__, vol, user);
	di->user_vol[user] = vol;
	for (i = 0; i < BBST_USER_END; i++) {
		if (!test_bit(i, &di->user))
			continue;
		if (di->vout_vote_type == VOUT_VOTE_SET_MIN)
			temp_vol = (unsigned int)power_min_positive(temp_vol, di->user_vol[i]);
		else
			temp_vol = (temp_vol > di->user_vol[i]) ?
				temp_vol : di->user_vol[i];
	}

	hwlog_info("buck_boost set vol = %u\n", temp_vol);
	return l_ops->set_vout(temp_vol, l_ops->dev_data);
}
EXPORT_SYMBOL(buck_boost_set_vout);

bool buck_boost_pwr_good(unsigned int type)
{
	struct buck_boost_ops *l_ops = buck_boost_get_ops(type);

	if (!l_ops)
		return false;

	if (!l_ops->pwr_good) {
		hwlog_err("pwr_good is null\n");
		return false;
	}

	return l_ops->pwr_good(l_ops->dev_data);
}

bool buck_boost_set_enable(unsigned int enable, unsigned int user)
{
	struct buck_boost_ops *l_ops = buck_boost_get_ops(BBST_TYPE_MAIN);
	struct buck_boost_dev *di = g_buck_boost_di;

	if (!di || !l_ops || (user < BBST_USER_BEGIN) || (user >= BBST_USER_END))
		return false;

	if (!l_ops->set_enable) {
		hwlog_err("set_enable is null\n");
		return false;
	}

	if (enable) {
		set_bit(user, &di->user);
		hwlog_info("user:%s open\n", g_buck_boost_user_data[user].name);
		return l_ops->set_enable(enable, l_ops->dev_data);
	} else {
		clear_bit(user, &di->user);
		buck_boost_set_vout(0, user);
		hwlog_info("user:%s close\n", g_buck_boost_user_data[user].name);
		if (di->user == BBST_NO_USER) {
			hwlog_info("buck_boost close\n");
			return l_ops->set_enable(enable, l_ops->dev_data);
		}
	}

	return true;
}
EXPORT_SYMBOL(buck_boost_set_enable);

static int buck_boost_parse_bbst_vout_table(struct device_node *np,
	struct buck_boost_dev *di)
{
	int i, len, size;

	len = power_dts_read_u32_count(power_dts_tag(HWLOG_TAG), np, "gpio_vout_table",
		BBST_VOUT_TABLE_MAX_SIZE, BBST_GPIO_VOUT_END);
	if (len <= 0)
		return -EINVAL;

	size = sizeof(*di->vout_table) * (len / BBST_GPIO_VOUT_END);
	di->vout_table = kzalloc(size, GFP_KERNEL);
	if (!di->vout_table)
		return -ENOMEM;

	if (power_dts_read_u32_array(power_dts_tag(HWLOG_TAG), np,
		"gpio_vout_table", (u32 *)di->vout_table, len)) {
		kfree(di->vout_table);
		return -EINVAL;
	}

	di->vout_table_size = len / BBST_GPIO_VOUT_END;
	for (i = 0; i < di->vout_table_size; i++)
		hwlog_info("vout_table[%d], output_voltage: %d gpio_value: %d\n", i,
			di->vout_table[i].output_voltage, di->vout_table[i].gpio_value);
	return 0;
}

static void buck_boost_parse_bbst_vout_gpios(struct device_node *np,
	struct buck_boost_dev *di)
{
	int i;
	struct power_gpio_group gpio_group[BBST_VOUT_TABLE_MAX_SIZE] = { 0 };

	if (power_gpio_config_gpio_group(np, "bbst_vout_gpios", gpio_group, BBST_VOUT_TABLE_MAX_SIZE, &di->gpio_vout_num)) {
		hwlog_err("%s config gpio group failed\n", __func__);
		return;
	}

	di->bbst_gpio_vout = kzalloc(sizeof(*di->bbst_gpio_vout) * di->gpio_vout_num, GFP_KERNEL);
	if (!di->bbst_gpio_vout)
		return;

	for (i = 0; i < di->gpio_vout_num; i++)
		di->bbst_gpio_vout[i] = gpio_group[i].gpio;
}

static void buck_boost_dts_parse(struct buck_boost_dev *di)
{
	struct device_node *np = NULL;

	if (!di || !di->dev || !di->dev->of_node) {
		hwlog_err("di is null\n");
		return;
	}

	np = di->dev->of_node;
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"boost_use_gpio", &di->boost_use_gpio, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"vout_vote_type", &di->vout_vote_type, VOUT_VOTE_SET_MAX);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"supply_const_vout", &di->supply_const_vout, 0);
	if (di->supply_const_vout)
		(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
			"const_vout", &di->const_vout, BBST_DEFAULT_VOUT);
	buck_boost_parse_bbst_vout_gpios(np, di);
	buck_boost_parse_bbst_vout_table(np, di);
}

static int buck_boost_gpio_init(struct buck_boost_dev *di)
{
	int ret;
	struct device_node *np = NULL;

	if (!di || !di->dev || !di->dev->of_node) {
		hwlog_err("di is null\n");
		return -ENODEV;
	}

	np = di->dev->of_node;
	ret = power_gpio_config_output(np, "bbst_gpio_en",
		"bbst_gpio_en", &di->bbst_gpio_en, 0);
	if (ret) {
		hwlog_err("bbst_gpio_en init fail\n");
		return ret;
	}

	return 0;
}

static int buck_boost_gpio_ops_register(struct buck_boost_dev *di)
{
	int ret;

	buck_boost_gpio_ops.dev_data = (void *)di;
	ret = buck_boost_ops_register(&buck_boost_gpio_ops);
	if (ret) {
		hwlog_err("buck_boost_gpio_ops register fail\n");
		return ret;
	}

	return 0;
}

static int buck_boost_gpio_check(struct buck_boost_dev *di)
{
	int ret;

	ret = buck_boost_gpio_init(di);
	if (ret)
		return ret;

	ret = buck_boost_gpio_ops_register(di);
	if (ret)
		return ret;

	return 0;
}

#ifdef CONFIG_HUAWEI_POWER_DEBUG
static ssize_t buck_boost_dbg_test_flag_show(void *dev_data, char *buf, size_t size)
{
	struct buck_boost_dev *di = dev_data;

	if (!buf || !di) {
		hwlog_err("buf or di is null\n");
		return -EINVAL;
	}

	return scnprintf(buf, size, "buck_boost_test_flag=0x%x\n", di->test_flag);
}

static ssize_t buck_boost_dbg_test_flag_store(void *dev_data, const char *buf, size_t size)
{
	struct buck_boost_dev *di = dev_data;
	int test_flag = 0;

	if (!buf || !di) {
		hwlog_err("buf or di is null\n");
		return -EINVAL;
	}

	if (sscanf_s(buf, "0x%x", &test_flag) != 1) {
		hwlog_err("%s sscanf_s failed\n", __func__);
		return -EINVAL;
	}

	di->test_flag = test_flag;
	hwlog_info("test_flag=0x%x\n", test_flag);
	return size;
}

static ssize_t buck_boost_dbg_set_enable_store(void *dev_data, const char *buf, size_t size)
{
	struct buck_boost_dev *di = dev_data;
	int value = 0;
	int user_id = 0;

	if (!buf || !di) {
		hwlog_err("buf or di is null\n");
		return -EINVAL;
	}

	if (di->test_flag != DBG_BUCK_BOOST_FLAG) {
		hwlog_err("test_flag=0x%x is invalid\n", di->test_flag);
		return -EINVAL;
	}

	if (sscanf_s(buf, "%d %d", &value, &user_id) != 2) {
		hwlog_err("%s sscanf_s failed\n", __func__);
		return -EINVAL;
	}

	if (((value != 0) && (value != 1)) || (user_id < BBST_USER_BEGIN) || (user_id >= BBST_USER_END)) {
		hwlog_err("value or user_id is invalid value=%d user_id=%d\n", value, user_id);
		return -EINVAL;
	}

	hwlog_info("%s, value=%d user_id=%d\n", __func__, value, user_id);
	buck_boost_set_enable(value, user_id);
	return size;
}

static ssize_t buck_boost_dbg_set_vout_store(void *dev_data, const char *buf, size_t size)
{
	struct buck_boost_dev *di = dev_data;
	int user_id = 0;
	int vol = 0;

	if (!buf || !di) {
		hwlog_err("buf or di is null\n");
		return -EINVAL;
	}

	if (di->test_flag != DBG_BUCK_BOOST_FLAG) {
		hwlog_err("test_flag=0x%x is invalid\n", di->test_flag);
		return -EINVAL;
	}

	if (sscanf_s(buf, "%d %d", &vol, &user_id) != 2) {
		hwlog_err("%s sscanf_s failed\n", __func__);
		return -EINVAL;
	}

	if ((vol < 0) || (user_id < BBST_USER_BEGIN) || (user_id >= BBST_USER_END)) {
		hwlog_err("vol or user_id is invalid vol=%d user_id=%d\n", vol, user_id);
		return -EINVAL;
	}

	hwlog_info("%s, vol=%d user_id=%d\n", __func__, vol, user_id);
	buck_boost_set_vout(vol, user_id);
	return size;
}
#endif /* CONFIG_HUAWEI_POWER_DEBUG */

static void buck_boost_register_power_debug(struct buck_boost_dev *di)
{
#ifdef CONFIG_HUAWEI_POWER_DEBUG
	power_dbg_ops_register("buck_boost", "test_flag", di,
		buck_boost_dbg_test_flag_show, buck_boost_dbg_test_flag_store);
	power_dbg_ops_register("buck_boost", "set_enable", di,
		NULL, buck_boost_dbg_set_enable_store);
	power_dbg_ops_register("buck_boost", "set_vout", di,
		NULL, buck_boost_dbg_set_vout_store);
#endif /* CONFIG_HUAWEI_POWER_DEBUG */
}

static int buck_boost_init_vout(struct buck_boost_dev *di)
{
	struct buck_boost_ops *l_ops = buck_boost_get_ops(BBST_TYPE_MAIN);

	if (!di || !l_ops || !l_ops->set_vout) {
		hwlog_err("di or ops or set_vout is null\n");
		return -EPERM;
	}

	if (!di->supply_const_vout)
		return 0;

	hwlog_info("buck_boost set vol = %u\n", di->const_vout);
	return l_ops->set_vout(di->const_vout, l_ops->dev_data);
}

static int buck_boost_probe(struct platform_device *pdev)
{
	int ret;
	struct buck_boost_dev *di = NULL;

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	di = devm_kzalloc(&pdev->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	g_buck_boost_di = di;
	di->dev = &pdev->dev;
	buck_boost_dts_parse(di);
	if (di->boost_use_gpio) {
		ret = buck_boost_gpio_check(di);
		if (ret) {
			hwlog_err("buck boost gpio check fail\n");
			kfree(di->bbst_gpio_vout);
			kfree(di->vout_table);
			devm_kfree(&pdev->dev, di);
			g_buck_boost_di = NULL;
			return ret;
		}
	}
	buck_boost_register_power_debug(di);
	buck_boost_init_vout(di);
	platform_set_drvdata(pdev, di);
	return 0;
}

static int buck_boost_remove(struct platform_device *pdev)
{
	struct buck_boost_dev *di = platform_get_drvdata(pdev);

	if (!di)
		return -ENODEV;

	kfree(di->bbst_gpio_vout);
	kfree(di->vout_table);
	platform_set_drvdata(pdev, NULL);
	g_buck_boost_di = NULL;
	return 0;
}

static const struct of_device_id buck_boost_match_table[] = {
	{
		.compatible = "huawei, buck_boost",
		.data = NULL,
	},
	{},
};

static struct platform_driver buck_boost_driver = {
	.probe = buck_boost_probe,
	.remove = buck_boost_remove,
	.driver = {
		.name = "huawei, buck_boost",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(buck_boost_match_table),
	},
};

static int __init buck_boost_init(void)
{
	return platform_driver_register(&buck_boost_driver);
}

static void __exit buck_boost_exit(void)
{
	platform_driver_unregister(&buck_boost_driver);
}

fs_initcall_sync(buck_boost_init);
module_exit(buck_boost_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("buck_boost module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
