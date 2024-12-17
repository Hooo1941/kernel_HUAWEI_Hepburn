/*
 * direct_charge_test.h
 *
 * direct charge test driver
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

#ifndef _DIRECT_CHARGE_TEST_H_
#define _DIRECT_CHARGE_TEST_H_
#include <chipset_common/hwpower/direct_charge/direct_charge_ic.h>

struct dc_test_info {
	int succ_flag;
	const char *ic_name[CHARGE_IC_MAX_NUM];
	int channel_num;
	int ibat_max;
	int ibus[CHARGE_IC_MAX_NUM];
	int ibat[CHARGE_IC_MAX_NUM];
	int vbat[CHARGE_IC_MAX_NUM];
	int vout[CHARGE_IC_MAX_NUM];
	int tbat[CHARGE_IC_MAX_NUM];
	int coul_check_flag;
	int coul_ibat_max;
	int coul_vbat_max;
	int coul_ibat[COUL_IC_NUM];
	int coul_vbat[COUL_IC_NUM];
};

#ifdef CONFIG_DIRECT_CHARGER
void dc_test_set_adapter_test_result(int mode, int result);
void dc_update_charging_info(int mode, struct dc_test_info *info, int *vbat_comp);
void dc_show_realtime_charging_info(int mode, unsigned int path);
#else
static inline void dc_test_set_adapter_test_result(int mode, int result)
{
}

static inline void dc_update_charging_info(int mode, struct dc_test_info *info, int *vbat_comp)
{
}

static inline void dc_show_realtime_charging_info(int mode, unsigned int path)
{
}

#endif /* CONFIG_DIRECT_CHARGER */

#endif /* _DIRECT_CHARGE_TEST_H_ */
