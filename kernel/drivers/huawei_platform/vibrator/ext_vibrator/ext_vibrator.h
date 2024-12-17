/*
 * ext_vibrator.h
 * ext vibrator driver
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
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

#ifndef _HUAWEI_PMIC_VIBRATOR_H
#define _HUAWEI_PMIC_VIBRATOR_H

#define HUAWEI_PMIC_VIBRATOR_DEFAULT_NAME "vibrator"
#define HUAWEI_PMIC_VIBRATOR_CDEVIE_NAME "haptics"

#define PMIC_VIBRATOR_BRAKE_TIME_OUT 100
#define PMIC_VIBRATOR_HAP_BUF_LENGTH 16
#define PMIC_VIBRATOR_POWER_ON 1
#define PMIC_VIBRATOR_POWER_OFF 0

#define EXT_VIBRATOR_SID             0x01
#define EXT_VIBRATOR_WORK_CID        0x93
#define EXT_VIBRATOR_ON              0x01
#define EXT_VIBRATOR_OFF             0x00

#define EXT_VIBRATOR_COMM_BUF_LEN 10

#define HAPTIC_INVALID  0

enum VIBRATOR_TYPE {
	TYPE_EXT_VIBR_WORK_TIME = 0,
	TYPE_EXT_VIBR_WORK_SCENARIO
};

struct ext_vibrator_haptics_type {
	const unsigned int haptics_num;
	const unsigned int command;
};

#endif
