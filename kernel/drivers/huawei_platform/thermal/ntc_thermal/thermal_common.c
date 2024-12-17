/*
 * thermal_common.c
 *
 * thermal for ntc module
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
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
#include <linux/slab.h>

#include <huawei_platform/log/hw_log.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include "ext_sensorhub_api.h"
#include "thermal_common.h"

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif

#define HWLOG_TAG watch_ntc
HWLOG_REGIST();

#define THERMAL_SERVICE_ID     0x01
#define THERMAL_COMMAND_ID     0x82
#define BYTES_RSP_LEN  5
#define RSP_TYPE_POS   0
#define RSP_DATA_POS   1
#define DATA_LEN       4
#define THERMAL_CONST         100

struct hw_thermal_class {
	struct class *thermal_class;
	struct device *temperature_device;
};

struct hw_thermal_class hw_thermal_info;

int watch_query_thermal_info(enum watch_thermal_cmd_to_mcu command, int *temp)
{
	int ret_val = -1;
	int ret;
	unsigned char send_cmd = (unsigned char)command; /* command will not be greater than 0xff */
	struct command cmd = {
		.service_id = THERMAL_SERVICE_ID,
		.command_id = THERMAL_COMMAND_ID,
		.send_buffer = &send_cmd,
		.send_buffer_len = sizeof(send_cmd),
	};
	struct cmd_resp mcu_rsp;

	mcu_rsp.receive_buffer_len = BYTES_RSP_LEN;
	mcu_rsp.receive_buffer = kmalloc(mcu_rsp.receive_buffer_len, GFP_KERNEL);

	if (!mcu_rsp.receive_buffer)
		return -ENOMEM;
	ret = send_command(CHARGE_CHANNEL, &cmd, true, &mcu_rsp);
	if (ret < 0) {
		hwlog_err("thermal send command failed, ret: %d, cmd: %d\n", ret, command);
		goto err;
	}

	if (mcu_rsp.receive_buffer[RSP_TYPE_POS] != command) {
		hwlog_err("thermal receive buff is %02x\n", mcu_rsp.receive_buffer[RSP_TYPE_POS]);
		goto err;
	}
	ret = memcpy_s(temp, DATA_LEN, &mcu_rsp.receive_buffer[RSP_DATA_POS], DATA_LEN);
	if (ret) {
		hwlog_err("parse thermal failed, temp %d\n", *temp);
		goto err;
	}
	*temp *= THERMAL_CONST;
	ret_val = 0;
err:
	kfree(mcu_rsp.receive_buffer);
	return ret_val;
}

int thermal_parse_dts(struct device_node *dev_node,
		      struct thermal_priv *info)
{
	int ret;
	const char *sensor_name = NULL;
	const char *ops_name = NULL;

	if (power_dts_read_string(power_dts_tag(HWLOG_TAG), dev_node,
				  "sensor_name", &sensor_name))
		return -EINVAL;

	if (power_dts_read_string(power_dts_tag(HWLOG_TAG), dev_node,
				  "ops_name", &ops_name))
		return -EINVAL;

	ret = strncpy_s(info->sensor.sensor_name, sizeof(info->sensor.sensor_name),
		  sensor_name, (POWER_TZ_STR_MAX_LEN - 1));
	if (ret < 0)
		hwlog_err("strncpy_s sensor_name failed, ret %d\n", ret);

	ret = strncpy_s(info->sensor.ops_name, sizeof(info->sensor.ops_name),
		  ops_name, (POWER_TZ_STR_MAX_LEN - 1));
	if (ret < 0)
		hwlog_err("strncpy_s ops_name failed, ret %d\n", ret);

	return 0;
}

static int create_file_node(struct platform_device *pdev, struct thermal_priv *priv,
			    struct attribute_group *attr, const char *name)
{
	struct device *dev = &pdev->dev;
	struct device_node *dev_node = dev->of_node;
	int ret;

	ret = sysfs_create_link(&hw_thermal_info.temperature_device->kobj,
				&pdev->dev.kobj, name);
	if (ret != 0) {
		hwlog_err("%s: create hw_thermal device file error: %d\n", dev_node->name, ret);
		return -EINVAL;
	}
	ret = sysfs_create_group(&pdev->dev.kobj, attr);
	if (ret != 0) {
		hwlog_err("%s: create thermal file error: %d\n", dev_node->name, ret);
		sysfs_remove_link(&hw_thermal_info.temperature_device->kobj, name);
		return -EINVAL;
	}
	return 0;
}

int thermal_probe(struct platform_device *pdev, struct thermal_priv **priv,
		  struct thermal_zone_device_ops *ops,
		  struct attribute_group *attr, const char *name)
{
	int ret;

	if (!pdev || !pdev->dev.of_node || !priv)
		return -ENODEV;

	*priv = devm_kzalloc(&pdev->dev, sizeof(struct thermal_priv), GFP_KERNEL);
	if (!(*priv))
		return -ENOMEM;

	(*priv)->dev = &pdev->dev;

	ret = thermal_parse_dts((*priv)->dev->of_node, *priv);
	if (ret < 0) {
		hwlog_err("thermal_parse_dts failed\n");
		goto fail_parse_dts;
	}

	(*priv)->sensor.tz_dev = thermal_zone_device_register((*priv)->sensor.sensor_name, 0,
							   0, *priv, ops, NULL, 0, 0);
	if (IS_ERR((*priv)->sensor.tz_dev)) {
		hwlog_err("ntc thermal zone register fail\n");
		ret = -ENODEV;
		goto fail_register_tz;
	}

	platform_set_drvdata(pdev, *priv);
	ret = create_file_node(pdev, *priv, attr, name);
	if (ret != 0)
		goto fail_register_tz;

	return 0;

fail_register_tz:
	thermal_zone_device_unregister((*priv)->sensor.tz_dev);

fail_parse_dts:
	of_node_put((*priv)->dev->of_node);
	devm_kfree(&pdev->dev, *priv);
	*priv = NULL;
	return ret;
}

static int create_thermal_node(struct platform_device *pdev, struct attribute *attr)
{
	struct device *dev = &pdev->dev;
	struct device_node *dev_node = dev->of_node;
	int ret;
	ret = sysfs_create_link(&hw_thermal_info.temperature_device->kobj, &pdev->dev.kobj, dev_node->name);
	if (ret != 0) {
		pr_err("%s: create hw_thermal device file error: %d\n", dev_node->name, ret);
		return -EINVAL;
	}
	ret = sysfs_create_file(&pdev->dev.kobj, attr);
	if (ret != 0) {
		pr_err("%s: create exp_node file error: %d\n", dev_node->name, ret);
		sysfs_remove_link(&hw_thermal_info.temperature_device->kobj, dev_node->name);
		return -EINVAL;
	}
	return 0;
}

int thermal_node_probe(struct platform_device *pdev, struct attribute *attr)
{
	int ret;
	ret = create_thermal_node(pdev, attr);
	return ret;
}

static int __init thermal_common_init(void)
{
	/* create huawei thermal class */
	hw_thermal_info.thermal_class = class_create(THIS_MODULE, "hw_thermal");
	if (IS_ERR(hw_thermal_info.thermal_class)) {
		hwlog_err("Huawei thermal class create error\n");
		return PTR_ERR(hw_thermal_info.thermal_class);
	}

	/* create device "temp" */
	hw_thermal_info.temperature_device =
		device_create(hw_thermal_info.thermal_class, NULL, 0, NULL, "temp");
	if (IS_ERR(hw_thermal_info.temperature_device)) {
		hwlog_err("hw_thermal:temperature device create error\n");
		class_destroy(hw_thermal_info.thermal_class);
		hw_thermal_info.thermal_class = NULL;
		return PTR_ERR(hw_thermal_info.temperature_device);
	}
	return 0;
}

static void __exit thermal_common_exit(void)
{
	if (hw_thermal_info.thermal_class) {
		device_destroy(hw_thermal_info.thermal_class, 0);
		class_destroy(hw_thermal_info.thermal_class);
	}
}

module_init(thermal_common_init);
module_exit(thermal_common_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("huawei thermal common driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");

