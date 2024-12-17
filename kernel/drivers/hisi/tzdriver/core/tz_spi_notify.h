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
#ifndef TZ_SPI_NOTIFY_H
#define TZ_SPI_NOTIFY_H
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include "teek_ns_client.h"

int tz_spi_init(struct device *class_dev, struct device_node *np);
void free_tz_spi(struct device *class_dev);
int send_notify_cmd(unsigned int cmd_id);

#endif
