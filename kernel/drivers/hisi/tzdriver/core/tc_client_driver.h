/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
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
#ifndef TC_CLIENT_DRIVER_H
#define TC_CLIENT_DRIVER_H

#include <linux/list.h>
#include <linux/cdev.h>
#include "teek_ns_client.h"

struct dev_node {
	struct class *driver_class;
	struct cdev char_dev;
	dev_t devt;
	struct device *class_dev;
	const struct file_operations *fops;
	char *node_name;
};

bool get_tz_init_flag(void);
struct tc_ns_dev_list *get_dev_list(void);
struct tc_ns_dev_file *tc_find_dev_file(unsigned int dev_file_id);
int tc_ns_client_open(struct tc_ns_dev_file **dev_file, uint8_t kernel_api);
int tc_ns_client_close(struct tc_ns_dev_file *dev);
int is_agent_alive(unsigned int agent_id);

#ifdef CONFIG_ACPI
int get_acpi_tz_irq(void);
#endif

#endif
