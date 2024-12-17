/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 *
 * pmic_offd.c
 *
 * pmic off find funtion
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "pmic_offd.h"
#include <platform_include/basicplatform/linux/mfd/pmic_platform.h>

#if defined(PMIC_OFFD_EN)

void pmu_set_bt_power_enable(int enable)
{
	unsigned int tmp;
	unsigned int orig;

	orig = pmic_read_reg(PMU_BTPWR_EN_ADDR);
	tmp = orig & (~BTPWR_EN_MASK);
	tmp |= enable & BTPWR_EN_MASK;
	pmic_write_reg(PMU_BTPWR_EN_ADDR, (int)tmp);
}
EXPORT_SYMBOL_GPL(pmu_set_bt_power_enable);

void pmu_set_bt_enable(int enable)
{
	unsigned int tmp;
	unsigned int orig;

	orig = pmic_read_reg(PMU_BT_EN_ADDR);
	tmp = orig & (~BT_EN_MASK);
	tmp |= enable & BT_EN_MASK;
	pmic_write_reg(PMU_BT_EN_ADDR, (int)tmp);
}
EXPORT_SYMBOL_GPL(pmu_set_bt_enable);

void pmu_set_uwb_enable(int enable)
{
	unsigned int tmp;
	unsigned int orig;

	orig = pmic_read_reg(PMU_UWB_EN_ADDR);
	tmp = orig & (~UWB_EN_MASK);
	tmp |= enable & UWB_EN_MASK;
	pmic_write_reg(PMU_UWB_EN_ADDR, (int)tmp);
}
EXPORT_SYMBOL_GPL(pmu_set_uwb_enable);

int pmu_get_bt_power_status(void)
{
	unsigned int orig;

	orig = pmic_read_reg(PMU_BTPWR_EN_ADDR);
	if ((orig & BTPWR_EN_MASK) == BTPWR_EN_MASK)
		return 1;

	return 0;
}
EXPORT_SYMBOL_GPL(pmu_get_bt_power_status);

int pmu_get_bt_status(void)
{
	unsigned int orig;

	orig = pmic_read_reg(PMU_BT_EN_ADDR);
	if ((orig & BT_EN_MASK) == BT_EN_MASK)
		return 1;

	return 0;
}
EXPORT_SYMBOL_GPL(pmu_get_bt_status);

int pmu_get_uwb_status(void)
{
	unsigned int orig;

	orig = pmic_read_reg(PMU_UWB_EN_ADDR);
	if ((orig & UWB_EN_MASK) == UWB_EN_MASK)
		return 1;

	return 0;
}
EXPORT_SYMBOL_GPL(pmu_get_uwb_status);

void pmu_set_offd_flag(int flag)
{	
	unsigned int tmp;
	unsigned int orig;

	orig = pmic_read_reg(PMU_OFFD_FLAG_ADDR);
	tmp = orig & (~OFFD_FLAG_MASK);
	tmp |= flag & OFFD_FLAG_MASK;
	pmic_write_reg(PMU_OFFD_FLAG_ADDR, (int)tmp);
}
EXPORT_SYMBOL_GPL(pmu_set_offd_flag);

int pmu_get_offd_flag(void)
{
	unsigned int orig;

	orig = pmic_read_reg(PMU_OFFD_FLAG_ADDR);
	if ((orig & OFFD_FLAG_MASK) == OFFD_FLAG_MASK)
		return 1;

	return 0;
}
EXPORT_SYMBOL_GPL(pmu_get_offd_flag);

void pmu_set_offd_time(int time)
{
	if (time < 0 || time > 0xFF) {
		pr_err("time %d is  out of range", time);
		return;
	}

	pmic_write_reg(PMU_OFFD_TIME_ADDR, time);
}
EXPORT_SYMBOL_GPL(pmu_set_offd_time);

#endif