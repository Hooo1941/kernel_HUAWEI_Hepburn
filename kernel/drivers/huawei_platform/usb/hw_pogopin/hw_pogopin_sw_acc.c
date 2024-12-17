/* SPDX-License-Identifier: GPL-2.0 */
/*
 * hw_pogopin_sw_acc.c
 *
 * pogopin driver
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
#include <huawei_platform/usb/hw_pogopin_sw.h>

#define HWLOG_TAG hw_pogopin_acc
HWLOG_REGIST();
static struct hw_pogopin_sw_acc_dev *g_hw_pogopin_sw_acc_dev;
static void pogopin_acc_report_uevent(struct hw_pogopin_sw_acc_kb_info *kb_info);

void hw_pogopin_sw_report_acc_info(struct hw_pogopin_sw_acc_kb_info *kb_info, enum hw_pogopin_sw_status type)
{
	if (type == DISCONNECTED) {
		if (snprintf_s(kb_info[HW_POGOPIN_SW_ACC_INFO_STATE].value, ACC_VALUE_MAX_LEN,
			ACC_VALUE_MAX_LEN - 1, "%s", "DISCONNECTED") < 0)
			hwlog_err("snprintf_s report_acc_info_state_disconnect fail\n");
	} else if (type == CONNECTED) {
		if (snprintf_s(kb_info[HW_POGOPIN_SW_ACC_INFO_STATE].value, ACC_VALUE_MAX_LEN,
			ACC_VALUE_MAX_LEN - 1, "%s", "CONNECTED") < 0)
			hwlog_err("snprintf_s report_acc_info_state_connect fail\n");
	} else if (type == PING_SUCC) {
		if (snprintf_s(kb_info[HW_POGOPIN_SW_ACC_INFO_STATE].value, ACC_VALUE_MAX_LEN,
			ACC_VALUE_MAX_LEN - 1, "%s", "PING_SUCC") < 0)
			hwlog_err("snprintf_s report_acc_info_state_pingsucc fail\n");
	} else {
		return;
	}

	pogopin_acc_report_uevent(kb_info);
}

static void pogopin_acc_report_uevent(struct hw_pogopin_sw_acc_kb_info *kb_info)
{
	char *p_data = NULL;
	int i;
	char *p_uevent_data[ACC_DEV_INFO_NUM_MAX] = { 0 };

	struct hw_pogopin_sw_acc_dev *di = g_hw_pogopin_sw_acc_dev;
	char *p_data_tmp = NULL;
	u8 info_no = HW_POGOPIN_SW_ACC_INFO_END;

	hwlog_info("%s accessory notify uevent begin process\n", __func__);

	if (!di || !di->dev) {
		hwlog_err("di or dev is null\n");
		hw_pogopin_sw_event_notify(HW_POGOPIN_SW_UART_STATUS_FAIL);
		return;
	}
	if (!kb_info) {
		hwlog_err("kb_info is null\n");
		return;
	}

	p_data = kzalloc((info_no * ACC_DEV_INFO_LEN) + 1, GFP_KERNEL);
	if (!p_data) {
		hw_pogopin_sw_event_notify(HW_POGOPIN_SW_UART_STATUS_FAIL);
		return;
	}

	p_data_tmp = p_data;
	for (i = 0; i < info_no; i++) {
		if (snprintf_s(p_data_tmp, ACC_DEV_INFO_LEN, ACC_DEV_INFO_LEN - 1, "%s=%s",
			kb_info[i].name, kb_info[i].value) < 0)
			hwlog_err("snprintf_s fail\n");
		p_uevent_data[i] = p_data_tmp;
		p_data_tmp += ACC_DEV_INFO_LEN;
		if (i != HW_POGOPIN_SW_ACC_INFO_MAC) {
			hwlog_info("acc uevent_data[%d]:%s\n", i, p_uevent_data[i]);
		}
	}

	kobject_uevent_env(&di->dev->kobj, KOBJ_CHANGE, p_uevent_data);

	kfree(p_data);
	hwlog_info("accessory notify uevent end\n");
}

static ssize_t pogopin_acc_dev_info_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int dev_no;
	int info_no;
	char acc_info[ACC_DEV_INFO_MAX] = { 0 };
	char *p_acc_info = acc_info;
	struct hw_pogopin_sw_dev_info *dev_info = NULL;
	struct hw_pogopin_sw_key_info *key_info = NULL;
	struct hw_pogopin_sw_acc_dev *di = g_hw_pogopin_sw_acc_dev;

	if (!di) {
		hwlog_err("di is null\n");
		goto out;
	}

	for (dev_no = ACC_POGO_DEV_NO_BEGIN; dev_no < ACC_POGO_DEV_NO_END; dev_no++) {
		dev_info = &di->dev_info;
		if (!dev_info->key_info) {
			hwlog_info("acc dev[%d] info is null\n", dev_no);
			continue;
		}
		for (info_no = 0; info_no < dev_info->info_no; info_no++) {
			key_info = dev_info->key_info + info_no;
			if (strlen(acc_info) >= (ACC_DEV_INFO_MAX - HW_POGO_SW_DEV_INFO_LEN))
				goto out;

			if (snprintf_s(p_acc_info, HW_POGO_SW_DEV_INFO_LEN,
				HW_POGO_SW_DEV_INFO_LEN - 1, "%s=%s,", key_info->name, key_info->value) < 0)
				hwlog_err("snprintf_s fail\n");
			/* info contain '=' and ',' two bytes */
			p_acc_info += strlen(key_info->name) + strlen(key_info->value) + 2;
		}
		/* two info separated by ','; two devices separated by ';' */
		if (dev_no == (ACC_POGO_DEV_NO_END - 1))
			p_acc_info[strlen(p_acc_info) - 1] = '\0';
		else
			p_acc_info[strlen(p_acc_info) - 1] = ';';
	}

out:
	return scnprintf(buf, PAGE_SIZE, "%s\n", acc_info);
}

static DEVICE_ATTR(dev, 0440, pogopin_acc_dev_info_show, NULL);

static struct attribute *pogopin_acc_attributes[] = {
	&dev_attr_dev.attr,
	NULL,
};

static const struct attribute_group pogopin_acc_attr_group = {
	.attrs = pogopin_acc_attributes,
};

static struct device *pogopin_acc_create_group(void)
{
	return power_sysfs_create_group("hw_accessory", "pogopin_monitor",
		&pogopin_acc_attr_group);
}

static void pogopin_acc_remove_group(struct device *dev)
{
	power_sysfs_remove_group(dev, &pogopin_acc_attr_group);
}

static int pogopin_acc_probe(struct platform_device *pdev)
{
	struct hw_pogopin_sw_acc_dev *di = NULL;

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	di = devm_kzalloc(&pdev->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	hwlog_info("%s alloc end \n", __func__);
	g_hw_pogopin_sw_acc_dev = di;
	di->dev = pogopin_acc_create_group();
	platform_set_drvdata(pdev, di);
	hwlog_info("%s create end \n", __func__);

	return 0;
}

static int pogopin_acc_remove(struct platform_device *pdev)
{
	struct hw_pogopin_sw_acc_dev *di = platform_get_drvdata(pdev);

	if (!di)
		return -ENODEV;

	pogopin_acc_remove_group(di->dev);
	platform_set_drvdata(pdev, NULL);
	devm_kfree(&pdev->dev, di);
	g_hw_pogopin_sw_acc_dev = NULL;

	return 0;
}

static const struct of_device_id pogopin_acc_match_table[] = {
	{
		.compatible = "huawei,pogopinaccessory",
		.data = NULL,
	},
	{},
};

static struct platform_driver pogopin_acc_driver = {
	.probe = pogopin_acc_probe,
	.remove = pogopin_acc_remove,
	.driver = {
		.name = "huawei,pogopinaccessory",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(pogopin_acc_match_table),
	},
};

static int __init pogopin_acc_init(void)
{
	hwlog_info("%s create end \n", __func__);
	return platform_driver_register(&pogopin_acc_driver);
}

static void __exit pogopin_acc_exit(void)
{
	platform_driver_unregister(&pogopin_acc_driver);
}

late_initcall(pogopin_acc_init);
module_exit(pogopin_acc_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("pogopin accessory module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
