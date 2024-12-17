/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
 * Description: This module is an independent thread that receives
 * and sends netlink messages and interrupts message forwarding.
 * Each module needs to register messages with this module.
 * This module accepts external messages and forwards them through
 * a mapping table to the registered module.
 * Author: linlixin2@huawei.com
 * Create: 2020-10-22
 */

#include <securec.h>

#include <huawei_platform/log/hw_log.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/netlink.h>
#include <linux/of.h>
#include <linux/printk.h>
#include <linux/semaphore.h>
#include <linux/skbuff.h>
#include <linux/time.h>
#include <linux/timer.h>
#include <net/sock.h>
#include <net/netlink.h>
#include <uapi/linux/netlink.h>

#ifdef CONFIG_HW_RSMC_X800
#include "x800/rsmc_start_x800.h"
#endif

#ifdef CONFIG_HW_RSMC_X801
#include "x801/rsmc_start_x801.h"
#endif

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif
#define HWLOG_TAG RSMC_CHIPTYPE_SELECT
HWLOG_REGIST();
MODULE_LICENSE("GPL");

#ifdef NETLINK_RSMC
#undef NETLINK_RSMC
#endif

#define DTS_NODE_HUAWEI_RSMC "huawei_rsmc"
#define DTS_PROP_RSMC_ENABLE_X801 "rsmc_enable_x801"
#define DTS_PROP_RSMC_ENABLE_X800 "rsmc_enable"

bool rsmc_enable_detect(void)
{
	int ret;
	struct device_node *node = of_find_compatible_node(NULL, NULL, DTS_NODE_HUAWEI_RSMC);
	if (node == NULL) {
		hwlog_err("%s: no huawei_rsmc", __func__);
		return false;
	}
	if (of_property_read_bool(node, DTS_PROP_RSMC_ENABLE_X801)) {
		hwlog_info("%s: rsmc support x801", __func__);
		return true;
	}
	if (of_property_read_bool(node, DTS_PROP_RSMC_ENABLE_X800)) {
		hwlog_info("%s: rsmc support x800", __func__);
		return true;
	}
	return false;
}

int rsmc_init_thread(void *data)
{
	int ret;
	struct device_node *node = of_find_compatible_node(NULL, NULL, DTS_NODE_HUAWEI_RSMC);
	hwlog_info("%s: enter", __func__);
	if (node == NULL) {
		hwlog_err("%s: no huawei_rsmc", __func__);
		return -EINVAL;
	}
#ifdef CONFIG_HW_RSMC_X801
	if (of_property_read_bool(node, DTS_PROP_RSMC_ENABLE_X801)) {
		hwlog_info("%s: rsmc_enable_x801", __func__);
		ret = rsmc_start_x801();
		if (ret == 0) {
			hwlog_info("%s: rsmc_enable_x801 success", __func__);
			return 0;
		} else {
			return -EINVAL;
		}
	}
#endif

#ifdef CONFIG_HW_RSMC_X800
	if (of_property_read_bool(node, DTS_PROP_RSMC_ENABLE_X800)) {
		hwlog_info("%s: rsmc_enable_x800", __func__);
		ret = rsmc_start_x800();
		if (ret == 0) {
			hwlog_info("%s: rsmc_enable_x800 success", __func__);
			return 0;
		} else {
			return -EINVAL;
		}
	}
#endif
	hwlog_info("%s: rsmc_disable", __func__);
	return -EINVAL;
}

int __init rsmc_init(void)
{
	struct task_struct *task = NULL;
	if (!rsmc_enable_detect()) {
		hwlog_err("%s: not support rsmc", __func__);
		return -EINVAL;
	}
	task = kthread_run(rsmc_init_thread, NULL, "rsmc_init_thread");
	if (IS_ERR(task)) {
		hwlog_err("%s: failed to create thread", __func__);
		task = NULL;
		return -EINVAL;
	}
	return 0;
}

void __exit rsmc_exit(void)
{
	struct device_node *node = of_find_compatible_node(NULL, NULL, DTS_NODE_HUAWEI_RSMC);
	hwlog_info("%s: enter", __func__);
	if (node == NULL) {
		hwlog_info("%s: not support rsmc", __func__);
		return;
	}
#ifdef CONFIG_HW_RSMC_X801
	if (of_property_read_bool(node, DTS_PROP_RSMC_ENABLE_X801)) {
		hwlog_info("%s: x801 exit", __func__);
		rsmc_stop_x801();
		return;
	}
#endif

#ifdef CONFIG_HW_RSMC_X800
	if (of_property_read_bool(node, DTS_PROP_RSMC_ENABLE_X800)) {
		hwlog_info("%s: x800 exit", __func__);
		rsmc_stop_x800();
	}
#endif
}

late_initcall(rsmc_init);
module_exit(rsmc_exit);

