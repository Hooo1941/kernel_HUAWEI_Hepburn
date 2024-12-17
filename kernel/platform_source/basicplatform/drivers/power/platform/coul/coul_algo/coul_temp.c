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
 *
 */
#include <platform_include/basicplatform/linux/power/platform/coul/coul_private_interface.h>
#include "coul_temp.h"

#define TEMP_IPC_GET_ABNORMAL   100
#define TEMPERATURE_CHANGE_LIMIT        30
#define ABNORMAL_BATT_TEMPERATURE_POWEROFF 670
#define DELTA_TEMP 150
#define LOW_BATT_TEMP_CHECK_THRESHOLD (-100)
#define MAX_ADC_OFFSET          4096
#define TEMP_RETRY_TIMES        3
#define STABLY_TEMP_RETRY_TIMES 5

int coul_battery_retry_temp_permille(struct smartstar_coul_device *di, enum battery_temp_user user)
{
	if (di == NULL)
		goto error;

	/* default is no battery in sft and udp, so here temp is fixed 25 */
	if (di->is_board_type != BAT_BOARD_ASIC || di->temp_shielded) {
		coul_core_err("not support ntc, use default temp\n");
		return DEFAULT_TEMP * PERMILLAGE;
	}

	if (di->coul_dev_ops && di->coul_dev_ops->get_bat_temp)
		return di->coul_dev_ops->get_bat_temp(di->batt_index) * PERMILLAGE;

error:
	coul_core_err("%s error\n", __func__);
	if (user == USER_CHARGER)
		return TEMP_IPC_GET_ABNORMAL * PERMILLAGE;
	return DEFAULT_TEMP * PERMILLAGE;
}

int coul_battery_get_temperature_tenth_degree(struct smartstar_coul_device *di,
	enum battery_temp_user user)
{
	return coul_battery_retry_temp_permille(di, user) / PERCENT;
}

/* BATTERY_TEMP_USER:0-coul;1-charger, Return the battery temperature */
static int coul_ntc_temperature_compensation(
	struct smartstar_coul_device *di, enum battery_temp_user user)
{
	int i, temp_without_compensation, temp_with_compensation;
	int ichg = 0;
	bool batt_temp_fitted = false;

	/*
	 * modify the temperature obtained by sampling,
	 * according to the temperature compensation value corresponding to the different current
	 */
	temp_without_compensation = coul_battery_get_temperature_tenth_degree(di, user);
	temp_with_compensation = temp_without_compensation;
#ifdef CONFIG_HUAWEI_BATTERY_TEMP_FITTING
	batt_temp_fitted = btf_temp_fitted_eqr();
#endif
	if ((di->ntc_compensation_is == 1) && (!batt_temp_fitted) &&
			(temp_without_compensation >= COMPENSATION_THRESHOLD)) {
		ichg = di->coul_dev_ops->get_battery_current_ua(di->batt_index) / PERMILLAGE;
		ichg = abs(ichg);
		for (i = 0; i < COMPENSATION_PARA_LEVEL; i++) {
			if (ichg >= di->ntc_temp_compensation_para[i].ntc_compensation_ichg) {
				temp_with_compensation = temp_without_compensation -
					di->ntc_temp_compensation_para[i].ntc_compensation_value;
				break;
			}
		}
	}

	coul_core_info("coul_ntc_temperature_compensation: current = %d,"
			" temp_without_compensation = %d, temp_with_compensation = %d\n",
		ichg, temp_without_compensation, temp_with_compensation);
	return temp_with_compensation;
}

/*
 * the fun for adc get some err, we can avoid
 * BATTERY_TEMP_USER:0-coul;1-charger, return value battery temperature
 */
int coul_get_temperature_stably(struct smartstar_coul_device *di,
	enum battery_temp_user user)
{
	int cnt = 0;
	int temperature = 0;
	int delta;

	if (di == NULL) {
		coul_core_err("error, di is NULL, return default temp\n");
		return DEFAULT_TEMP * TENTH;
	}

	while (cnt++ < STABLY_TEMP_RETRY_TIMES) {
		temperature = coul_ntc_temperature_compensation(di, user);
		delta = abs(di->batt_temp - temperature);
		if ((delta > DELTA_TEMP) ||
			(temperature > ABNORMAL_BATT_TEMPERATURE_POWEROFF) ||
			(temperature <= LOW_BATT_TEMP_CHECK_THRESHOLD))
			continue;
		coul_core_info("%s, stably temp!,old_temp =%d, cnt =%d, temp = %d\n",
			__func__, di->batt_temp, cnt, temperature);
		return temperature;
	}
	return temperature;
}

void coul_update_battery_temperature(struct smartstar_coul_device *di,
	int status)
{
	int temp;

	if (di == NULL) {
		coul_core_err("%s, di is NULL\n", __func__);
		return;
	}
	temp = coul_get_temperature_stably(di, USER_COUL);
	if (status == TEMPERATURE_INIT_STATUS) {
		coul_core_info("init temp = %d\n", temp);
		di->batt_temp = temp;
	} else {
		if (temp - di->batt_temp > TEMPERATURE_CHANGE_LIMIT) {
			coul_core_err("temperature change too fast, pre = %d, current = %d\n",
				di->batt_temp, temp);
			di->batt_temp = di->batt_temp + TEMPERATURE_CHANGE_LIMIT;
		} else if (di->batt_temp - temp > TEMPERATURE_CHANGE_LIMIT) {
			coul_core_err("temperature change too fast, pre = %d, current = %d\n",
				di->batt_temp, temp);
			di->batt_temp = di->batt_temp - TEMPERATURE_CHANGE_LIMIT;
		} else if (di->batt_temp != temp) {
			coul_core_info("temperature changed, pre = %d, current = %d\n",
				di->batt_temp, temp);
			di->batt_temp = temp;
		}
	}
}

