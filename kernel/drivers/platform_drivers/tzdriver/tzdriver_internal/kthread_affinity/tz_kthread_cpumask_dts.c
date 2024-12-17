/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 * Description: function for get kthread cpumask
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */
#include "tz_kthread_cpumask.h"
#include <linux/of.h>
#include <linux/cpumask.h>
#include "tc_ns_log.h"

/* on mdc, kthread should be bind to ctrlcpu, which is read from dts */
int get_kthread_cpumask(struct cpumask *cpumask)
{
	int ret;
	uint32_t i;
	uint32_t ctrl_cpu_num;
	struct device_node *ctrl_cpu_node = NULL;

	if (!cpumask)
		return -1;

	ctrl_cpu_node = of_find_node_by_name(NULL, "ascend_ctl");
	if (ctrl_cpu_node == NULL) {
		tloge("get ctrl cpu node failed\n");
		return -1;
	}

	ret = of_property_read_u32_index(ctrl_cpu_node, "ctrl_cpu_num", 0, &ctrl_cpu_num);
	if (ret < 0) {
		tloge("get ctrl cpu num failed, err=%d\n", ret);
		return ret;
	}
	if (ctrl_cpu_num > num_online_cpus()) {
		tloge("ctrl cpu num is invalid\n");
		return -1;
	}

	for (i = 0; i < ctrl_cpu_num; i++)
		cpumask_set_cpu(i, cpumask);

	return 0;
}
