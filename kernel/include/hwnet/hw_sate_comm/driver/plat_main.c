/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 * Description: This module is used to start the driver peripheral
 * and identify the peripheral chip type.
 * Author: lizhenzhou1@huawei.com
 * Create: 2022-11-26
 */

#include "plat_main.h"

#include <securec.h>

#include <log/hw_log.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/jiffies.h>
#include <linux/kernel.h>
#include <linux/ktime.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/poll.h>
#include <linux/version.h>

#include "dev/bbic_dev.h"
#include "vcom/vcom.h"

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif
#define HWLOG_TAG HW_SATE_MAIN
HWLOG_REGIST();

#define SUCCESS 0
#define FAILURE 1

#define BBIC_CHIPTYPE_HLCT "hlct"
#define BBIC_CHIPTYPE_WS "ws"
#define DTS_NODE_HUAWEI_SATE "huawei_sate"

int hw_sate_init_thread(void *data)
{
	int ret;
	int bbic_chip_type = -1;
	const char *dts_bbic_chip_type = NULL;
	struct device_node *node;

	node = of_find_compatible_node(NULL, NULL, DTS_NODE_HUAWEI_SATE);
	if (node == NULL) {
		hwlog_err("%s: no huawei_sate node", __func__);
		return -1;
	}
	ret = of_property_read_string(node, "bbic_chip_type", &dts_bbic_chip_type);
	if (ret != 0) {
		hwlog_err("%s, read bbic chip type failed\n", __func__);
		return -1;
	}

	if (strcmp(dts_bbic_chip_type, BBIC_CHIPTYPE_HLCT) == 0) {
		bbic_chip_type = BBIC_CHIP_TYPE_HLCT;
	} else if (strcmp(dts_bbic_chip_type, BBIC_CHIPTYPE_WS) == 0) {
		bbic_chip_type = BBIC_CHIP_TYPE_WS;
	} else {
		hwlog_err("%s, bbic chip type failed: %d\n", __func__, bbic_chip_type);
		return -1;
	}
	hwlog_info("%s, bbic chip type: %d\n", __func__, bbic_chip_type);
	set_bbic_chip_type(bbic_chip_type);

	ret = hw_stavcom_init();
	if (ret != 0) {
		hwlog_err("%s, task init failed\n", __func__);
		return -1;
	}

	ret = hw_bbic_dev_init();
	if (ret != 0) {
		hwlog_err("%s, hlct dev init\n", __func__);
		return -1;
	}

	return ret;
}

int __init hw_sate_plat_init(void)
{
	struct task_struct *task = kthread_run(hw_sate_init_thread, NULL, "hw_sate_init_thread");
	if (IS_ERR(task)) {
		hwlog_err("%s: failed to create thread", __func__);
		task = NULL;
		return -1;
	}
	return 0;
}

void __exit hw_sate_plat_exit(void)
{
	hw_stavcom_exit();
	hw_bbic_dev_exit();
}

module_init(hw_sate_plat_init);
module_exit(hw_sate_plat_exit);
