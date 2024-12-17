/* SPDX-License-Identifier: GPL-2.0
 *
 * thermal_common.h
 *
 * thermal common head file
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
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

#ifndef _THERMAL_COMMON_H_
#define _THERMAL_COMMON_H_

#include <linux/thermal.h>

#define POWER_TZ_STR_MAX_LEN      16

struct watch_tz_sensor {
	char sensor_name[POWER_TZ_STR_MAX_LEN];
	char ops_name[POWER_TZ_STR_MAX_LEN];
	struct thermal_zone_device *tz_dev;
};

struct thermal_priv {
	struct device *dev;
	struct watch_tz_sensor sensor;
};

enum watch_thermal_cmd_to_mcu {
	SHELL_FRAME = 1,
	SHELL_FRONT = 2,
	SHELL_AMBIENT = 3,
};

/* query_thermal_info by command */
int watch_query_thermal_info(enum watch_thermal_cmd_to_mcu command, int *temp);
/* thermal probe */
int thermal_probe(struct platform_device *pdev, struct thermal_priv **priv,
		  struct thermal_zone_device_ops *ops,
		  struct attribute_group *attr, const char *name);
/* create exp node */
int thermal_node_probe(struct platform_device *pdev, struct attribute *attr);

#endif /* _THERMAL_COMMON_H_ */
