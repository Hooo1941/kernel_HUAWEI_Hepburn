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

#include "rsmc_msg_loop.h"

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif
#define HWLOG_TAG RSMC_START_X800
HWLOG_REGIST();

int rsmc_start_x800(void)
{
	int ret;
	hwlog_info("%s: enter", __func__);
	ret = rsmc_init_x800();
	if (ret != 0) {
		hwlog_info("%s: rsmc_enable_x801 success", __func__);
		return -EINVAL;
	}
	return 0;
}

void rsmc_stop_x800(void)
{
	hwlog_info("%s: enter", __func__);
	rsmc_exit_x800();
}

