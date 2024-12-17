/* SPDX-License-Identifier: GPL-2.0 */
/*
 * rt9426_kirin.h
 *
 * driver for rt9426 kirin platform driver
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

#ifndef _RT9426_KIRIN_H_
#define _RT9426_KIRIN_H_

/* battery capcaity */
#define RT9426_BATT_CAPACITY_ZERO           0
#define RT9426_BATT_CAPACITY_CRITICAL       5
#define RT9426_BATT_CAPACITY_LOW            15
#define RT9426_BATT_CAPACITY_HIGH           95
#define RT9426_BATT_CAPACITY_FULL           100
#define RT9426_BATT_CAPACITY_DEFAULT        50
#define RT9426_BATT_CAPACITY_LOW_LVL        3
#define RT9426_BATT_CAPACITY_WARNING_LVL    10

/* charge state */
#define RT9426_CHARGE_STATE_START_CHARGING  1
#define RT9426_CHARGE_STATE_CHRG_DONE       2
#define RT9426_CHARGE_STATE_STOP_CHARGING   3
#define RT9426_CHARGE_STATE_NOT_CHARGING    4

/* id voltage para */
#define ID_VOLT_PARA_LEVEL                  4

#define POWER_MASK_HIGH_BYTE                0xFF00

enum rt9426_id_volt_table {
	ID_VOLT_INDEX = 0,
	ID_VOLT_ADC_CHANNEL,
	ID_VOLT_MIN,
	ID_VOLT_MAX,
	ID_VOLT_PARA_TOTAL,
};

struct rt9426_id_volt_para {
	int id_index;
	int id_adc_channel;
	int id_volt_min;
	int id_volt_max;
	int batt_id_voltage;
};

#endif /* _RT9426_KIRIN_H_ */
