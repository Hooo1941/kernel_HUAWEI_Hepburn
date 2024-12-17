/*
 * echub_thermal.c
 *
 * thermal driver for echub
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
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/thermal.h>
#include <securec.h>
#include "../echub.h"
#include "../echub_i2c.h"

#define SENSOR_NAME_STR_MAX_LEN		(30)
#define SENSOR_NUMBER_MAX_LEN		(20)
#define ECHUB_I2C_NTC_DTAT_MAXLEN	(5)
#define NTC_TEMP_INVALID			(-23)
#define ECHUB_NTC_REG				(0x0261)
#define ECHUB_FAN_SPEED				(0x027c)

// sensor type---process function---reg---regoffset---outputbufLen
typedef int (*thermal_data_process_function)(int regoffset, char regdata, int bytes, int *temp);

struct echub_channel_sensor_info {
	char *sensor_type;
	thermal_data_process_function process;
	int regoffset;
	int regdata;
	int bytes;
};

struct echub_ntc_sensor {
	char sensor_type[SENSOR_NAME_STR_MAX_LEN];
	unsigned int sensor_id;
	struct thermal_zone_device *tz_dev;
};

struct echub_thermal_priv {
	struct device *dev;
	struct echub_ntc_sensor sensor[SENSOR_NUMBER_MAX_LEN];
	struct echub_i2c_dev *echub_dev;
	unsigned int sensor_num;
};

static int g_temp_last[SENSOR_NUMBER_MAX_LEN];
static struct echub_thermal_priv *g_echub_tz_info;

static int get_fan_speed_from_ec(int regoffset, char regdata, int bytes, int *temp);
static int get_ntc_data_from_ec(int regoffset, char regdata, int bytes, int *temp);

static const struct echub_channel_sensor_info echub_sensor_info[] = {
	{"soc_side",     get_ntc_data_from_ec,  ECHUB_NTC_REG,   0x5,  5},
	{"type_c",       get_ntc_data_from_ec,  ECHUB_NTC_REG,   0x7,  5},
	{"charger_mos",  get_ntc_data_from_ec,  ECHUB_NTC_REG,   0x8,  5},
	{"ambience",     get_ntc_data_from_ec,  ECHUB_NTC_REG,   0xf,  5},
	{"type_c_conn2", get_ntc_data_from_ec,  ECHUB_NTC_REG,   0x2f, 5},
	{"fan_speed0",   get_fan_speed_from_ec, ECHUB_FAN_SPEED, 0x0,  5},
};

static int get_fan_speed_from_ec(int regoffset, char regdata, int bytes, int *temp)
{
	char value[ECHUB_I2C_NTC_DTAT_MAXLEN] = {0};
	// fan_speed
	int ret = g_echub_tz_info->echub_dev->read_func(g_echub_tz_info->echub_dev,
		regoffset, regdata,
		value, bytes);
	if (ret) {
		echub_err("%s failed %d ret = %d\n", __func__, __LINE__, ret);
		return ret;
	}
	*temp = (value[3]  << 8) + value[2];
	return 0;
}

static int get_ntc_data_from_ec(int regoffset, char regdata, int bytes, int *temp)
{
	char value[ECHUB_I2C_NTC_DTAT_MAXLEN] = {0};
	// sensor_temp
	int ret = g_echub_tz_info->echub_dev->read_func(g_echub_tz_info->echub_dev,
		regoffset, regdata,
		value, bytes);
	if (ret) {
		echub_err("%s failed %d ret = %d\n", __func__, __LINE__, ret);
		return ret;
	}
	*temp = ((value[3]  << 8) + value[2]) * 100;
	return 0;
}

static void get_thermal_data_from_ec(struct thermal_zone_device *thermal, int *temp)
{
	int i, ret;
	int raw_temp = NTC_TEMP_INVALID;

	if ((thermal == NULL) || (temp == NULL))
		echub_err("%s failed %d\n", __func__, __LINE__);

	for (i = 0; i < ARRAY_SIZE(echub_sensor_info); i++) {
		if (strncmp(echub_sensor_info[i].sensor_type, thermal->type,
			strlen(thermal->type)) == 0) {
			ret = echub_sensor_info[i].process(echub_sensor_info[i].regoffset,
				echub_sensor_info[i].regdata,
				echub_sensor_info[i].bytes,
				&raw_temp);
			if (ret) {
				echub_err("%s failed %d ret = %d sensor:%s\n",
					__func__, __LINE__, ret, thermal->type);
				*temp = g_temp_last[i];
				return;
			}
			g_temp_last[i] = raw_temp;
			break;
		}
	}
	*temp = raw_temp;
}

static int echub_thermal_get_temp(struct thermal_zone_device *thermal, int *temp)
{
	if (!thermal || !temp) {
		echub_err("%s failed %d\n", __func__, __LINE__);
		return -EINVAL;
	}

	get_thermal_data_from_ec(thermal, temp);
	return 0;
}

static struct thermal_zone_device_ops echub_thermal_ops = {
	.get_temp = echub_thermal_get_temp,
};

static int echub_thermal_parse_dts(struct device_node *dev_node,
	struct echub_thermal_priv *info)
{
	int i, ret;
	const char *sensor_type = NULL;
	unsigned int sensor_id;
	struct device_node *echub_sensor_np = NULL;
	struct device_node *echub_sensor_regnp = NULL;
	char node_name[SENSOR_NAME_STR_MAX_LEN] = {0};

	echub_sensor_np = of_get_child_by_name(dev_node, "sensors");
	if (!echub_sensor_np) {
		echub_err("%s failed %d not find sensors sub-node\n", __func__, __LINE__);
		return -EINVAL;
	}
	info->sensor_num = of_get_child_count(echub_sensor_np);
	echub_info("%s huawei,sensor_num %d\n", __func__, info->sensor_num);

	if (info->sensor_num != ARRAY_SIZE(echub_sensor_info))
		echub_err("%s failed %d sensor num error!\n", __func__, __LINE__);

	for (i = 0; i < info->sensor_num; i++) {
		ret = snprintf_s(node_name, sizeof(node_name),
			sizeof(node_name) - 1,
			"sensor%d",
			i);
		if (ret < 0)
			echub_err("%s failed %d\n", __func__, __LINE__);
		echub_sensor_regnp = of_get_child_by_name(echub_sensor_np, node_name);
		ret = of_property_read_string(echub_sensor_regnp, "sensor_name", &sensor_type);
		echub_info("%s sensor_type %s, ret %d i %d\n", node_name, sensor_type, ret, i);
		if (ret < 0)
			echub_err("%s failed %d\n", __func__, __LINE__);

		strncpy_s(info->sensor[i].sensor_type, sizeof(info->sensor[i].sensor_type),
			sensor_type, SENSOR_NAME_STR_MAX_LEN - 1);
		ret = of_property_read_u32(echub_sensor_regnp, "sensor_id",
			&sensor_id);
		if (ret < 0)
			echub_err("%s failed %d\n", __func__, __LINE__);

		info->sensor[i].sensor_id = sensor_id;
		g_temp_last[i] = NTC_TEMP_INVALID;
	}

	of_node_put(echub_sensor_regnp);
	of_node_put(echub_sensor_np);
	return 0;
}

static int echub_thermal_probe(struct platform_device *pdev)
{
	int i, ret;
	int flag = 0;
	struct echub_i2c_dev *echub_dev = NULL;
	struct echub_thermal_priv *priv = NULL;

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	echub_dev = dev_get_drvdata(pdev->dev.parent);
	if (echub_dev == NULL) {
		echub_err("echub_dev is null, get echub_dev form tmp\n");
		echub_dev = get_echub_dev();
		if (echub_dev == NULL) {
			echub_err("echub_dev is NULL return\n");
			return -ENODEV;
		}
	}

	priv = devm_kzalloc(&pdev->dev, sizeof(struct echub_thermal_priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	g_echub_tz_info = priv;
	priv->dev = &pdev->dev;
	priv->echub_dev = echub_dev;

	ret = echub_thermal_parse_dts(priv->dev->of_node, priv);
	if (ret < 0) {
		echub_err("echub_thermal_parse_dts failed\n");
		goto fail_parse_dts;
	}
	for (i = 0; i < priv->sensor_num; i++) {
		priv->sensor[i].tz_dev = thermal_zone_device_register(
			priv->sensor[i].sensor_type, 0, 0, priv,
			&echub_thermal_ops, NULL, 0, 0);
		flag++;
		if (IS_ERR(priv->sensor[i].tz_dev)) {
			echub_err("ntc thermal zone register fail\n");
			ret = -ENODEV;
			goto fail_register_tz;
		}
	}

	platform_set_drvdata(pdev, priv);
	return 0;

fail_register_tz:
	for (i = 0; i < flag; i++)
		thermal_zone_device_unregister(priv->sensor[i].tz_dev);

fail_parse_dts:
	of_node_put(priv->dev->of_node);

fail_free_mem:
	devm_kfree(&pdev->dev, priv);
	g_echub_tz_info = NULL;

	return ret;
}

static int echub_thermal_remove(struct platform_device *pdev)
{
	int i;
	struct echub_thermal_priv *priv = platform_get_drvdata(pdev);

	echub_info("%s enter\n", __func__);
	if (!priv) {
		echub_err("%s failed %d\n", __func__, __LINE__);
		return -ENODEV;
	}

	for (i = 0; i < priv->sensor_num; i++)
		thermal_zone_device_unregister(priv->sensor[i].tz_dev);

	platform_set_drvdata(pdev, NULL);
	devm_kfree(&pdev->dev, priv);
	g_echub_tz_info = NULL;
	return 0;
}

static const struct of_device_id echub_thermal_match[] = {
	{
		.compatible = "huawei,echub_thermal",
		.data = NULL,
	},
	{},
};

static struct platform_driver echub_thermal_driver = {
	.probe = echub_thermal_probe,
	.remove = echub_thermal_remove,
	.driver = {
		.name = "huawei,echub_thermal",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(echub_thermal_match),
	},
};

static int __init echub_thermal_init(void)
{
	return platform_driver_register(&echub_thermal_driver);
}

static void __exit echub_thermal_exit(void)
{
	platform_driver_unregister(&echub_thermal_driver);
}

module_init(echub_thermal_init);
module_exit(echub_thermal_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("huawei pc echub thermal driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
