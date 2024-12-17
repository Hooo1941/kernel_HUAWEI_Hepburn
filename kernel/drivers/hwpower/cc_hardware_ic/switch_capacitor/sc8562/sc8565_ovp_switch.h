/* SPDX-License-Identifier: GPL-2.0 */
/*
 * sc8565_ovp_switch.h
 *
 * sc8565 ovp switch header file
 *
 * Copyright (c) 2022-2022 Huawei Technologies Co., Ltd.
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

#ifndef _SC8565_OVP_SWITCH_H_
#define _SC8565_OVP_SWITCH_H_

#include "sc8562.h"

void sc8565_parse_sw_attr(struct device_node *np,
	struct sc8562_device_info *di);
void sc8565_power_sw_register(struct sc8562_device_info *di);

#endif /* _SC8565_OVP_SWITCH_H_ */