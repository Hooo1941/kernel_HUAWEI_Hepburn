/* SPDX-License-Identifier: GPL-2.0 */
/*
 * charge_mode_test.h
 *
 * common interface, variables, definition etc for wireless charge test
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

#ifndef _CHARGE_MODE_TEST_H_
#define _CHARGE_MODE_TEST_H_

#define CHARGE_MODE_LEN_MAX                20
#define PROTOCOL_LEN_MAX                   20
#define CHARGE_MODE_NUM_MAX                50
#define PROTOCOL_NUM_MAX                   3
#define RESULT_BUF_LEN_MAX                 256
#define CHARGE_MODE_SUCCESS                0
#define CHARGE_MODE_FAILURE                1
#define INVALID_RESULT                     (-1)
#define INVALID_INDEX                      (-1)
#define DELAY_TIME_READY_OK                1
#define ENABLE                             1
#define DISABLE                            0
#define CHARGE_MODE_WORK_TIME              5000

enum charge_mode_sysfs_type {
	CHARGE_MODE_SYSFS_START = 0,
	CHARGE_MODE_SYSFS_RESULT,
	CHARGE_MODE_SYSFS_END,
};

enum charge_mode_protocol_type {
	CHARGE_MODE_PROTOCOL_BEGIN = 0,
	CHARGE_MODE_PROTOCOL_SCP,
	CHARGE_MODE_PROTOCOL_UFCS,
	CHARGE_MODE_PROTOCOL_HVC,
	CHARGE_MODE_PROTOCOL_TOTAL,
};

enum charge_mode_type {
	CHARGE_MODE_TYPE_DCP = 0,
	CHARGE_MODE_TYPE_LVC,
	CHARGE_MODE_TYPE_SC,
	CHARGE_MODE_TYPE_MAIN_SC,
	CHARGE_MODE_TYPE_AUX_SC,
	CHARGE_MODE_TYPE_SC4,
	CHARGE_MODE_TYPE_MAIN_SC4,
	CHARGE_MODE_TYPE_AUX_SC4,
	CHARGE_MODE_TYPE_HVC,
	CHARGE_MODE_TYPE_END,
};

enum charge_mode_para_type {
	CHARGE_MODE_PROTOCOL = 0,
	CHARGE_MODE_MODE,
	CHARGE_MODE_IBAT_TH,
	CHARGE_MODE_TIME,
	CHARGE_MODE_TYPE,
	CHARGE_MODE_EXT,
	CHARGE_MODE_PARA_TOTAL,
};

enum charge_mode_result {
	CHARGE_MODE_RESULT_INIT = 0,
	CHARGE_MODE_RESULT_SUCC,
	CHARGE_MODE_RESULT_FAIL,
};

enum charge_mode_sub_result {
	CHARGE_MODE_SUB_INIT = 0,
	CHARGE_MODE_SUB_SUCC,
	CHARGE_MODE_IBAT_FAIL,
	CHARGE_MODE_UE_PROTOCOL_FAIL,
	CHARGE_MODE_ADP_PROTOCOL_FAIL,
	CHARGE_MODE_CC_MOISTURE,
	CHARGE_MODE_TEMP_ERR,
	CHARGE_MODE_VOL_INVALID,
	CHARGE_MODE_ADP_UNSUPPORT,
};

struct charge_mode_map {
	char name[PROTOCOL_LEN_MAX];
	int index;
};

struct charge_mode_action {
	int first;
	int second;
};

/*
 * The address offset is calculated when resolving parameters,
 * and the order of structure members cannot be changed.
 */
struct charge_mode_para {
	char protocol[PROTOCOL_LEN_MAX];
	char mode[CHARGE_MODE_LEN_MAX];
	int ibat_th;
	int time;
	int ext;
	int force;
	enum charge_mode_result result;
	enum charge_mode_sub_result sub_result;
};

struct charge_mode_dev {
	struct device *dev;
	struct notifier_block charge_mode_nb;
	struct delayed_work test_work;
	struct charge_mode_para mode_para[CHARGE_MODE_NUM_MAX];
	long long start_time;
	long long curr_time;
	int mode_idx;
	int mode_num;
	int adp_mode;
	int ping_result;
	int temp_err_flag;
	int voltage_invalid_flag;
	int delay_time;
	char result[RESULT_BUF_LEN_MAX];
};

#endif /* _CHARGE_MODE_TEST_H_ */
