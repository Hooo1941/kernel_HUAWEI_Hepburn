/*
 * freq_qos_debug.h
 *
 * create debug freq qos sysfs head file
 *
 * Copyright (c) 2012-2022 Huawei Technologies Co., Ltd.
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

#ifndef __FREQ_QOS_DEBUG_H__
#define __FREQ_QOS_DEBUG_H__

#include <linux/pm_qos.h>
#ifdef CONFIG_FREQ_QOS_DEBUG
void freq_qos_add_request_name(struct freq_qos_request *req);
#else
static inline void freq_qos_add_request_name(struct freq_qos_request *req) {}
#endif
#endif
