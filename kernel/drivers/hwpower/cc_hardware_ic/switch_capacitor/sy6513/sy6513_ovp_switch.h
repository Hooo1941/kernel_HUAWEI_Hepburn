/* SPDX-License-Identifier: GPL-2.0 */
/*
 * sy6513_ovp_switch.h
 *
 * sy6513 ovp switch header file
 *
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

#ifndef _SY6513_OVP_SWITCH_H_
#define _SY6513_OVP_SWITCH_H_

#include "sy6513.h"

void sy6513_parse_sw_attr(struct device_node *np, struct sy6513_device_info *di);
void sy6513_power_sw_register(struct sy6513_device_info *di);

#endif /* _SY6513_OVP_SWITCH_H_ */

