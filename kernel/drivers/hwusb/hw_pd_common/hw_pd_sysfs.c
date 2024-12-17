/*
 * hw_pd_sysfs.c
 *
 * Source file of sysfs operation interface of huawei PD driver
 *
 * Copyright (C) 2023 HUAWEI, Inc.
 * Author: HUAWEI, Inc.
 *
 * This package is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/of_device.h>
#include <chipset_common/hwpower/common_module/power_sysfs.h>
#include <chipset_common/hwusb/hw_pd/hw_pd_sysfs.h>
#include <huawei_platform/usb/hw_pd_dev.h>
#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/power/huawei_charger.h>

#ifndef HWLOG_TAG
#define HWLOG_TAG huawei_pd_sys
HWLOG_REGIST();
#endif /* HWLOG_TAG */

#define MMI_PD_TIMES                    3
#define MMI_PD_IBUS_MIN                 300
#define GET_IBUS_INTERVAL               1000

static struct device *g_hw_pd_sysfs_dev;

static ssize_t hw_pd_cc_orientation_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	/* 3 means cc is abnormally grounded */
	if (pd_state(cc_orientation_factory) == CC_ORIENTATION_FACTORY_SET)
		return scnprintf(buf, PAGE_SIZE, "%s\n", "3");

	return scnprintf(buf, PAGE_SIZE, "%s\n",
		pd_state(cc_orientation) ? "2" : "1");
}

static ssize_t hw_pd_pd_state_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int retrys = 0;
	int cur;

	hwlog_info("%s = %d\n", __func__, pd_dpm_get_pd_finish_flag());
	if (pd_dpm_get_pd_finish_flag()) {
		if (!pd_state(ibus_check))
			return scnprintf(buf, PAGE_SIZE, "%s\n",
				pd_dpm_get_pd_finish_flag() ? "0" : "1");
		while (retrys++ < MMI_PD_TIMES) {
			cur = charge_get_ibus();
			hwlog_info("%s, ibus = %d\n", __func__, cur);
			if (cur >= MMI_PD_IBUS_MIN)
				return scnprintf(buf, PAGE_SIZE, "%s\n", "0");

			msleep(GET_IBUS_INTERVAL);
		}
	}
	return scnprintf(buf, PAGE_SIZE, "%s\n", "1");
}

#ifdef CONFIG_TYPEC_CAP_CUSTOM_SRC2
static ssize_t hw_pd_smart_holder_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	hwlog_info("%s = %d\n", __func__, pd_state(smart_holder));
	return scnprintf(buf, PAGE_SIZE, "%s\n",
		pd_state(smart_holder) ? "1" : "0");
}
#endif

static ssize_t hw_pd_cc_state_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret;
	unsigned int cc1 = 0;
	unsigned int cc2 = 0;
	unsigned int cc = 0;

	ret = hw_pd_get_cc_state(&cc1, &cc2);
	if (ret == 0)
		cc = ((cc1 & PD_DPM_CC_STATUS_MASK) |
			(cc2 << PD_DPM_CC2_STATUS_OFFSET)) &
			PD_DPM_BOTH_CC_STATUS_MASK;

	return scnprintf(buf, PAGE_SIZE, "%d\n", cc);
}

static ssize_t hw_pd_vbus_only_status_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	hwlog_info("%s = %d\n", __func__, pd_state(vbus_only_status));
	return scnprintf(buf, PAGE_SIZE, "%s\n",
		pd_state(vbus_only_status) ? "1" : "0");
}

static ssize_t hw_pd_pd_dump_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret = scnprintf(buf, PAGE_SIZE, "version=%s\n", HW_PD_VERSION);
	ret += hw_pd_state_dump((buf + ret), (PAGE_SIZE - ret));
	return ret;
}

static DEVICE_ATTR(cc_orientation, S_IRUGO, hw_pd_cc_orientation_show, NULL);
static DEVICE_ATTR(pd_state, S_IRUGO, hw_pd_pd_state_show, NULL);
#ifdef CONFIG_TYPEC_CAP_CUSTOM_SRC2
static DEVICE_ATTR(smart_holder, S_IRUGO, hw_pd_smart_holder_show, NULL);
#endif /* CONFIG_TYPEC_CAP_CUSTOM_SRC2 */
static DEVICE_ATTR(cc_state, S_IRUGO, hw_pd_cc_state_show, NULL);
static DEVICE_ATTR(vbus_only_status, S_IRUGO, hw_pd_vbus_only_status_show, NULL);
static DEVICE_ATTR(pd_dump, S_IRUGO, hw_pd_pd_dump_show, NULL);

static struct attribute *hw_pd_ctrl_attributes[] = {
	&dev_attr_cc_orientation.attr,
	&dev_attr_pd_state.attr,
#ifdef CONFIG_TYPEC_CAP_CUSTOM_SRC2
	&dev_attr_smart_holder.attr,
#endif /* CONFIG_TYPEC_CAP_CUSTOM_SRC2 */
	&dev_attr_cc_state.attr,
	&dev_attr_vbus_only_status.attr,
	&dev_attr_pd_dump.attr,
	NULL,
};

const struct attribute_group hw_pd_attr_group = {
	.attrs = hw_pd_ctrl_attributes,
};

void hw_pd_sysfs_create(void)
{
	g_hw_pd_sysfs_dev = power_sysfs_create_group("hw_typec", "typec", &hw_pd_attr_group);
	if (!g_hw_pd_sysfs_dev)
		hwlog_info("%s:create fs failed\n", __func__);
}

