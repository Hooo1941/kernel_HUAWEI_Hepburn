/* SPDX-License-Identifier: GPL-2.0 */
/*
 * cps2023_ovp_switch.h
 *
 * cps2023 ovp switch header file
 *
 * Copyright (c) 2022-2023 Huawei Technologies Co., Ltd.
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

#ifndef _CPS2023_OVP_SWITCH_H_
#define _CPS2023_OVP_SWITCH_H_

#include "cps2023.h"

void cps2023_parse_sw_attr(struct device_node *np,
	struct cps2023_device_info *di);
void cps2023_power_sw_register(struct cps2023_device_info *di);

#endif /* _CPS2023_OVP_SWITCH_H_ */