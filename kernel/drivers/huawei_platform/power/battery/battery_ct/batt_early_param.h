/* SPDX-License-Identifier: GPL-2.0 */
/*
 * battery_early_param.h
 *
 * battery early param.
 *
 * Copyright (c) 2019-2020 Huawei Technologies Co., Ltd.
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

#ifndef _BATTERY_EARLY_PARAM_H_
#define _BATTERY_EARLY_PARAM_H_

#include <huawei_platform/power/batt_info_pub.h>
#include "batt_aut_checker.h"

#define BATT_MAX_UID_LEN 13
#define BATT_HEX_STR_LEN 3
#define BATT_MAX_NAME_LEN 20

struct batt_ic_para {
	int ic_type;
	int uid_len;
	char uid[BATT_MAX_UID_LEN];
};

struct batt_info_para {
	enum batt_source source;
	char sn[MAX_SN_LEN];
	char type[BATTERY_TYPE_BUFF_SIZE];
};

struct batt_cert_para {
	int valid_sign;
	enum key_cr key_result;
	int sign_len;
	char signature[BATT_MAX_SIGN_LEN];
};

struct batt_lock_para {
	uint8_t cyclk;
	uint8_t pglk;
};

struct batt_ic_para *batt_get_ic_para(void);
struct batt_info_para *batt_get_info_para(void);
struct batt_cert_para *batt_get_cert_para(void);
struct batt_lock_para *batt_get_lock_para(void);
char *batt_get_nv_sn(void);
char *batt_get_batt_name(void);

#endif /* _BATTERY_EARLY_PARAM_H_ */
