/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <linux/device.h>
#include <linux/platform_drivers/platform_qos.h>
#include <securec.h>
#include "dkmd_log.h"
#include "ddr_dvfs.h"

struct semaphore g_ddr_sem;
static int32_t g_ddr_bandwidth;
static struct platform_qos_request g_qos_req_perf;

static ssize_t dpu_ddr_bandwidth_show(struct device *dev, struct device_attribute *attr, char* buf)
{
	int32_t ret;
	dpu_check_and_return((!dev || !buf), -1, err, "input is null pointer");

	down(&g_ddr_sem);

	ret = snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%d\n", g_ddr_bandwidth);
	if (ret < 0) {
		dpu_pr_err("snprintf_s failed, ret value is %d\n", ret);
		up(&g_ddr_sem);
		return -1;
	}
	buf[strlen(buf) + 1] = '\0';

	up(&g_ddr_sem);

	return ret;
}

static ssize_t dpu_ddr_bandwidth_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int32_t ret;

	dpu_check_and_return((!dev || !buf), -1, err, "input is null pointer");

	down(&g_ddr_sem);

	ret = sscanf_s(buf, "%d", &g_ddr_bandwidth);
	if (ret == 0) {
		dpu_pr_err("get buf (%s) g_ddr_bandwidth fail\n", buf);
		up(&g_ddr_sem);
		return -1;
	}

	dpu_pr_debug("g_ddr_bandwidth=%d", g_ddr_bandwidth);

#ifdef CONFIG_DEVFREQ_GOV_PLATFORM_QOS
	platform_qos_update_request(&g_qos_req_perf, g_ddr_bandwidth);
#endif

	up(&g_ddr_sem);

	return (ssize_t)count;
}

static DEVICE_ATTR(ddr_freq, S_IRUSR | S_IRGRP | S_IWUSR, dpu_ddr_bandwidth_show, dpu_ddr_bandwidth_store);

void dpu_ddr_bandwidth_release(bool need_vote)
{
	if(!need_vote)
		return;

	dpu_pr_debug("ddr vote release bandwidth 0");

#ifdef CONFIG_DEVFREQ_GOV_PLATFORM_QOS
	platform_qos_update_request(&g_qos_req_perf, 0);
#endif
}

void dpu_ddr_bandwidth_recovery(bool need_vote)
{
	if(!need_vote)
		return;

	down(&g_ddr_sem);

	dpu_pr_debug("ddr vote recovery bandwidth %d", g_ddr_bandwidth);

#ifdef CONFIG_DEVFREQ_GOV_PLATFORM_QOS
	platform_qos_update_request(&g_qos_req_perf, g_ddr_bandwidth);
#endif
	up(&g_ddr_sem);
}

void dpu_ddr_dvfs_init(struct dkmd_attr *attrs)
{
	dpu_pr_info("ddr dvfs init");

	if (unlikely(!attrs)) {
		dpu_pr_err("attrs is null\n");
		return;
	}

	sema_init(&g_ddr_sem, 1);

#ifdef CONFIG_DEVFREQ_GOV_PLATFORM_QOS
	platform_qos_add_request(&g_qos_req_perf, PLATFORM_QOS_MEMORY_THROUGHPUT, 0);
#endif

	dkmd_sysfs_attrs_append(attrs, &dev_attr_ddr_freq.attr);
}

void dpu_ddr_dvfs_deinit(void)
{
#ifdef CONFIG_DEVFREQ_GOV_PLATFORM_QOS
	platform_qos_remove_request(&g_qos_req_perf);
#endif
}
