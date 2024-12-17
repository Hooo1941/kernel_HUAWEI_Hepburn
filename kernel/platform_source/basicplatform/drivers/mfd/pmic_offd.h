/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 *
 * pmic_offd.h
 *
 * pmic off find funtion

 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef _PMIC_OFFD_H_
#define _PMIC_OFFD_H_

#if defined(CONFIG_PMIC_21BV100_PMU)

#include <pmic_interface.h>

#ifndef BIT
#define BIT(x) (1 << x)
#endif

#define PMIC_OFFD_EN 1

#define PMU_BTPWR_EN_ADDR PMIC_BTPWR_EN_ONOFF_CTRL_ADDR(0)
#define BTPWR_EN_MASK BIT(PMIC_BTPWR_EN_ONOFF_CTRL_reg_pmu_btpwr_en_START)

#define PMU_BT_EN_ADDR PMIC_BT_EN_ONOFF_CTRL_ADDR(0)
#define BT_EN_MASK BIT(PMIC_BT_EN_ONOFF_CTRL_reg_pmu_bt_en_START)

#define PMU_UWB_EN_ADDR PMIC_UWB_EN_ONOFF_CTRL_ADDR(0)
#define UWB_EN_MASK BIT(PMIC_UWB_EN_ONOFF_CTRL_reg_pmu_uwb_en_START)

#define PMU_OFFD_FLAG_ADDR PMIC_FSR_FLAG_ADDR(0)
#define OFFD_FLAG_MASK BIT(PMIC_FSR_FLAG_sc_offd_flag_START)

#define PMU_OFFD_TIME_ADDR PMIC_OFFD_TIMER0_ADDR(0)

#endif

#endif