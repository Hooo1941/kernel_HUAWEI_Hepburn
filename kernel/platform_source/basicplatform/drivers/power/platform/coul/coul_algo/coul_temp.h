/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: temp functions for coul module
 *
 * This software is licensed under the terms of the GNU General Public
 * License, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef _COUL_ALGO_TEMP_H_
#define _COUL_ALGO_TEMP_H_

#include <platform_include/basicplatform/linux/power/platform/coul/coul_core.h>

int coul_battery_retry_temp_permille(struct smartstar_coul_device *di, enum battery_temp_user user);
int coul_battery_get_temperature_tenth_degree(struct smartstar_coul_device *di,
	enum battery_temp_user user);
int coul_get_temperature_stably(struct smartstar_coul_device *di,
	enum battery_temp_user user);
void coul_update_battery_temperature(struct smartstar_coul_device *di,
	int status);

#endif
