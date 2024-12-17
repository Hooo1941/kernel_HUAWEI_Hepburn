/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: nv operation functions fpr coul module
 *
 * This software is licensed under the terms of the GNU General Public
 * License, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#ifndef _COUL_ALGO_NV_H_
#define _COUL_ALGO_NV_H_

#include <platform_include/basicplatform/linux/power/platform/coul/coul_core.h>

#define DEFAULT_V_OFF_A         1000000
#define DEFAULT_V_OFF_B         0
#define DEFAULT_C_OFF_A         1000000
#define DEFAULT_C_OFF_B         0
#define OFFSET_A_MAX                  1500000
#define OFFSET_A_MIN                   500000
#define OFFSET_B_MAX                  1000000
#define OFFSET_B_MIN                  (-1000000)

void coul_get_cali_params(struct smartstar_coul_device *di, int *v_offset_a, int *v_offset_b,
	int *c_offset_a, int *c_offset_b);
void coul_set_cali_params(struct smartstar_coul_device *di, int *v_offset_a, int *v_offset_b,
	int *c_offset_a, int *c_offset_b);
unsigned long coul_get_nv_info_addr(void);
int coul_get_initial_params(struct smartstar_coul_device *di);
int coul_save_nv_info(struct smartstar_coul_device *di);
int coul_cali_info_set(struct smartstar_coul_device *di, struct coul_batt_cali_info *info);
int coul_cali_info_get(struct smartstar_coul_device *di, struct coul_batt_cali_info *info);

#endif

