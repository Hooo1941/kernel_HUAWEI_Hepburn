/*
 * cloud_cellular_algorithm.h
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
#ifndef CLOUD_CELLULAR_ALGORITHM_H
#define CLOUD_CELLULAR_ALGORITHM_H
#include <linux/types.h>
#include <linux/version.h>
#include <linux/spinlock.h>
#include "cloud_network.h"

typedef void (*status_change_callback) (algorithm_path status);
void cellular_algorithm_start(algorithm_path paths_in_use, status_change_callback status_change);
void cellular_algorithm_stop(void);
int cellular_usage_notification(algorithm_path paths_in_use);
algorithm_path cellular_expect_status(void);
#endif /* CLOUD_CELLULAR_ALGORITHM_H */