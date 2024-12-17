/* SPDX-License-Identifier: GPL-2.0 */
/*
 * buck_charge_state.h
 *
 * buck charge state module
 *
 * Copyright (c) 2021-2021 Huawei Technologies Co., Ltd.
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

#ifndef _BUCK_CHARGE_STATE_H_
#define _BUCK_CHARGE_STATE_H_

#define CHARGE_STATE_COUL_FAULT_VOLT              60
#define CHARGE_STATE_INCR_TERM_VOL_FORBIDDEN      0x2
#define CHARGE_STATE_INCR_TERM_VOL_ENABLE         0x1
#define CHARGE_STATE_INCR_TERM_VOL_DISABLE        0x0
#define CHARGE_STATE_INCRESE_TERM_VOLT            16
#define CHARGE_STATE_INCRESE_DETLA_VOLT           30
#define CHARGE_STATE_INCRESE_MAX_CYCLE            200
#define CHARGE_STATE_WEAKSOURCE_CNT               10
#define CHARGE_STATE_RX_HIGH_VOUT                 7000

/* define charging current gears of charger */
#define CHARGE_CURRENT_0000_MA                    0
#define CHARGE_CURRENT_0100_MA                    100
#define CHARGE_CURRENT_0150_MA                    150
#define CHARGE_CURRENT_0500_MA                    500
#define CHARGE_CURRENT_0800_MA                    800
#define CHARGE_CURRENT_1000_MA                    1000
#define CHARGE_CURRENT_1200_MA                    1200
#define CHARGE_CURRENT_1900_MA                    1900
#define CHARGE_CURRENT_2000_MA                    2000
#define CHARGE_CURRENT_4000_MA                    4000
#define CHARGE_CURRENT_MAX_MA                     32767

struct charge_state_dev {
	struct notifier_block fault_nb;
	struct work_struct fault_work;
	unsigned int charge_fault;
	int weaksource_cnt;
	bool ntc_fault;
};

#if (defined(CONFIG_HUAWEI_CHARGER_AP) || defined(CONFIG_HUAWEI_BUCK_CHARGER))
void charge_state_dsm_report(int err_no);
void charge_state_check(unsigned int state, unsigned int *events);
void charge_state_check_power_good(unsigned int state, unsigned int *events);
bool charge_state_is_ntc_fault(void);
void charge_state_stop_charging_handler(void);
int charge_state_init(struct device *dev);
void charge_state_remove(struct device *dev);
#else
static inline void charge_state_dsm_report(int err_no)
{
}

static inline void charge_state_check(unsigned int state, unsigned int *events)
{
}

static inline void charge_state_check_power_good(unsigned int state, unsigned int *events)
{
}

static inline void charge_state_stop_charging_handler(void)
{
}

static inline bool charge_state_is_ntc_fault(void)
{
	return false;
}

static inline int charge_state_init(struct device *dev)
{
	return 0;
}

static inline void charge_state_remove(struct device *dev)
{
}
#endif /* CONFIG_HUAWEI_CHARGER_AP || CONFIG_HUAWEI_BUCK_CHARGER */

#endif /* _BUCK_CHARGE_STATE_H_ */
