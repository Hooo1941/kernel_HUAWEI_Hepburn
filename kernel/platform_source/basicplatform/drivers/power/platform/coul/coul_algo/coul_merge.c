/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: Converged calculation of two batteries
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

#include <linux/module.h>
#include <linux/power_supply.h>
#include <linux/kernel.h>
#include <linux/of.h>
#include <linux/reboot.h>
#include <linux/suspend.h>
#include <linux/pm_wakeup.h>
#include <linux/workqueue.h>
#include <pmic_interface.h>
#include <linux/mfd/pmic_platform.h>

#include <platform_include/basicplatform/linux/power/platform/coul/coul_merge_drv.h>
#include <platform_include/basicplatform/linux/power/platform/coul/coul_drv.h>
#include <platform_include/basicplatform/linux/power/platform/bci_battery.h>
#ifdef CONFIG_DIRECT_CHARGER
#include <chipset_common/hwpower/direct_charge/direct_charger.h>
#endif
#include "coul_nv.h"
#include <huawei_platform/power/batt_info_pub.h>

#define coul_merge_debug(fmt, args...) pr_debug("[coul_merge]" fmt, ## args)
#define coul_merge_info(fmt, args...) pr_info("[coul_merge]" fmt, ## args)
#define coul_merge_warn(fmt, args...) pr_warn("[coul_merge]" fmt, ## args)
#define coul_merge_err(fmt, args...) pr_err("[coul_merge]" fmt, ## args)

#define CALCULATE_SOC_MS                (20 * 1000)
#define BATTRY_SN_LEN                   16
#define CATHODE_INDEX                   10
#define TYPE_SILICON_CHARS_LEN          6
#define BATTRY_SN_CHAR_SIZE              (BATTRY_SN_LEN * 3 + 6)

#define SWITCH_ON_VOLT_BASE             2500
#define SWITCH_ON_VOLT_STEP             50
#define PMIC_SWITCH_ON_VOLT_ADDR_H      PMIC_HRST_REG15_ADDR(0)
#define PMIC_SWITCH_ON_VOLT_MASK_H      0xF8
#define PMIC_SWITCH_ON_VOLT_OFFSET      3

#define COUL_MERGE_BATTERY_COUNT        3

static struct coul_merge_device *g_coul_merge = NULL;
static char g_type_silicon_chars[] = { 'K', 'L', 'N', 'U', 'V', 'Y' };
static int g_coul_merge_battery_count;

/* cap_ratio_x == 0 indicate this battery is not exist */
static int coul_merge_battery_count(void)
{
	struct coul_merge_device *di = NULL;
	int battery_count = 0;

	if(!g_coul_merge) {
		coul_merge_err("%s g_coul_merge is null.\n", __func__);
		return battery_count;
	}
	di = g_coul_merge;

	if(di->cap_ratio_0)
		battery_count++;

	if(di->cap_ratio_1)
		battery_count++;

	if(di->cap_ratio_2)
		battery_count++;

	return battery_count;
}

int coul_merge_is_battery_exist(void)
{
	int batt_index;
	int is_exist = 1;

	for (batt_index = 0; batt_index < g_coul_merge_battery_count; batt_index++) {
		is_exist = (int)((unsigned int)coul_merge_drv_is_battery_exist(batt_index));
		if (is_exist == 0)
			return is_exist;
	}

	return is_exist;
}

int coul_merge_is_battery_reach_threshold(void)
{
	return coul_merge_drv_is_battery_reach_threshold(BATT_0);
}

int coul_merge_battery_voltage(void)
{
	int vol, batt_index;
	int vol_max = INT_MIN;

	for (batt_index = 0; batt_index < g_coul_merge_battery_count; batt_index++) {
		vol = coul_merge_drv_battery_voltage_mv(batt_index);
		vol_max = max(vol, vol_max);
	}

	return vol_max;
}

char *coul_merge_battery_brand(void)
{
	return coul_merge_drv_battery_brand(BATT_0);
}

int coul_merge_battery_id_voltage(void)
{
	return coul_merge_drv_battery_id_voltage(BATT_0);
}

int coul_merge_battery_voltage_uv(void)
{
	int vol, batt_index;
	int vol_max = INT_MIN;

	for (batt_index = 0; batt_index < g_coul_merge_battery_count; batt_index++) {
		vol = coul_merge_drv_battery_voltage_uv(batt_index);
		vol_max = max(vol, vol_max);
	}

	return vol_max;
}

int coul_merge_battery_current(void)
{
	int curr[COUL_MERGE_BATTERY_COUNT], batt_index;

	for (batt_index = 0; batt_index < g_coul_merge_battery_count; batt_index++)
		curr[batt_index] = coul_merge_drv_battery_current(batt_index);

	switch (g_coul_merge_battery_count) {
	case BATTERY_COUNT_ONE:
		return curr[BATT_0];
	case BATTERY_COUNT_TWO:
		if (g_coul_merge && g_coul_merge->is_series_bat)
			return (curr[BATT_0] + curr[BATT_1]) / HALF;
		return curr[BATT_0] + curr[BATT_1];
	case BATTERY_COUNT_THREE:
		if (g_coul_merge && g_coul_merge->is_series_bat)
			return (curr[BATT_0] + (curr[BATT_1] + curr[BATT_2])) / HALF;
		return curr[BATT_0] + curr[BATT_1] + curr[BATT_2];
	default:
		coul_merge_err("%s g_coul_merge_battery_count is %d.\n", __func__, g_coul_merge_battery_count);
		break;
	}

	return 0;
}
int coul_merge_battery_resistance(void)
{
	int res[COUL_MERGE_BATTERY_COUNT], batt_index;

	for (batt_index = 0; batt_index < g_coul_merge_battery_count; batt_index++)
		res[batt_index] = coul_merge_drv_battery_resistance(batt_index);

	switch (g_coul_merge_battery_count) {
	case BATTERY_COUNT_ONE:
		return res[BATT_0];
	case BATTERY_COUNT_TWO:
		if (g_coul_merge && g_coul_merge->is_series_bat)
			return res[BATT_0] + res[BATT_1];
		return (res[BATT_0] * res[BATT_1]) / (res[BATT_0] + res[BATT_1]);
	case BATTERY_COUNT_THREE:
		if (g_coul_merge && g_coul_merge->is_series_bat)
			return res[BATT_0] + ((res[BATT_0] * res[BATT_1]) / (res[BATT_0] + res[BATT_1]));
		return (res[BATT_0] * res[BATT_1] * res[BATT_2]) /
		((res[BATT_0] * res[BATT_1]) + (res[BATT_0] * res[BATT_2]) + (res[BATT_1] * res[BATT_2]));
	default:
		coul_merge_err("%s g_coul_merge_battery_count is %d.\n", __func__, g_coul_merge_battery_count);
		break;
	}

	return 0;
}

int coul_merge_fifo_avg_current(void)
{
	int curr[COUL_MERGE_BATTERY_COUNT], batt_index;

	for (batt_index = 0; batt_index < g_coul_merge_battery_count; batt_index++)
		curr[batt_index] = coul_merge_drv_fifo_avg_current(batt_index);

	switch (g_coul_merge_battery_count) {
	case BATTERY_COUNT_ONE:
		return curr[BATT_0];
	case BATTERY_COUNT_TWO:
		if (g_coul_merge && g_coul_merge->is_series_bat)
			return (curr[BATT_0] + curr[BATT_1]) / HALF;
		return curr[BATT_0] + curr[BATT_1];
	case BATTERY_COUNT_THREE:
		if (g_coul_merge && g_coul_merge->is_series_bat)
			return (curr[BATT_0] + (curr[BATT_1] + curr[BATT_2])) / HALF;
		return curr[BATT_0] + curr[BATT_1] + curr[BATT_2];
	default:
		coul_merge_err("%s g_coul_merge_battery_count is %d.\n", __func__, g_coul_merge_battery_count);
		break;
	}

	return 0;
}

int coul_merge_battery_current_avg(void)
{
	int curr[COUL_MERGE_BATTERY_COUNT], batt_index;

	for (batt_index = 0; batt_index < g_coul_merge_battery_count; batt_index++)
		curr[batt_index] = coul_merge_drv_battery_current_avg(batt_index);

	switch (g_coul_merge_battery_count) {
	case BATTERY_COUNT_ONE:
		return curr[BATT_0];
	case BATTERY_COUNT_TWO:
		if (g_coul_merge && g_coul_merge->is_series_bat)
			return (curr[BATT_0] + curr[BATT_1]) / HALF;
		return curr[BATT_0] + curr[BATT_1];
	case BATTERY_COUNT_THREE:
		if (g_coul_merge && g_coul_merge->is_series_bat)
			return (curr[BATT_0] + (curr[BATT_1] + curr[BATT_2])) / HALF;
		return curr[BATT_0] + curr[BATT_1] + curr[BATT_2];
	default:
		coul_merge_err("%s g_coul_merge_battery_count is %d.\n", __func__, g_coul_merge_battery_count);
		break;
	}

	return 0;
}

int coul_merge_battery_capacity(void)
{
	int soc[COUL_MERGE_BATTERY_COUNT] = {0};
	int batt_index;
	struct coul_merge_device *di = NULL;

	if(!g_coul_merge) {
		coul_merge_err("%s g_coul_merge is null.\n", __func__);
		return 0;
	}
	di = g_coul_merge;

	for (batt_index = 0; batt_index < g_coul_merge_battery_count; batt_index++)
		soc[batt_index] = coul_merge_drv_battery_capacity(batt_index);

	di->soc = soc[BATT_0] * di->cap_ratio_0 + soc[BATT_1] * di->cap_ratio_1 + soc[BATT_2] * di->cap_ratio_2;
	di->soc /= PERMILLAGE;
	coul_merge_info("%s soc_0 %d, soc_1 %d, soc_2 %d, mSoc %d\n",
	 		__func__, soc[BATT_0], soc[BATT_1], soc[BATT_2], di->soc);

	return di->soc;
}

static int coul_merge_get_capacity(void)
{
	struct coul_merge_device *di = NULL;

	if(!g_coul_merge) {
		coul_merge_err("%s g_coul_merge is null.\n", __func__);
		return 0;
	}
	di = g_coul_merge;

	coul_merge_info("%s soc %d\n", __func__, di->soc);
	return di->soc;
}

static int coul_merge_get_unfiltered_capacity(void)
{
	int soc[COUL_MERGE_BATTERY_COUNT] = {0};
	int unfiltered_soc, batt_index;
	struct coul_merge_device *di = NULL;

	if(!g_coul_merge) {
		coul_merge_err("%s g_coul_merge is null.\n", __func__);
		return 0;
	}
	di = g_coul_merge;

	for (batt_index = 0; batt_index < g_coul_merge_battery_count; batt_index++)
		soc[batt_index] = coul_merge_drv_battery_unfiltered_capacity(batt_index);

	unfiltered_soc = (soc[BATT_0] * di->cap_ratio_0 + soc[BATT_1] * di->cap_ratio_1
	+ soc[BATT_2] * di->cap_ratio_2) / PERMILLAGE;

	coul_merge_info("%s unfiltered soc %d\n", __func__, unfiltered_soc);

	return unfiltered_soc;
}

int coul_merge_battery_temperature(void)
{
	return coul_merge_drv_battery_temperature(BATT_0);
}

static int coul_merge_battery_removed_before_boot(void)
{
	return coul_merge_drv_battery_removed_before_boot(BATT_0);
}

#ifdef CONFIG_BATT_SOH
int coul_merge_convert_mv2regval(int mv)
{
	return coul_merge_drv_convert_mv2regval(BATT_0,mv);
}

int coul_merge_convert_regval2ua(unsigned int reg_val)
{
	return coul_merge_drv_convert_regval2ua(BATT_0,reg_val);
}

int coul_merge_convert_regval2uv(unsigned int reg_val)
{
	return coul_merge_drv_convert_regval2uv(BATT_0,reg_val);
}
#endif

int coul_merge_battery_rm(void)
{
	int batt_index;
	int rm_sum = 0;

	for (batt_index = 0; batt_index < g_coul_merge_battery_count; batt_index++)
		rm_sum += coul_merge_drv_battery_rm(batt_index);

	return rm_sum;
}

int coul_merge_battery_fcc(void)
{
	int batt_index;
	int fcc_sum = 0;

	for (batt_index = 0; batt_index < g_coul_merge_battery_count; batt_index++)
		fcc_sum += coul_merge_drv_battery_fcc(batt_index);

	return fcc_sum;
}

int coul_merge_battery_fcc_design(void)
{
	int batt_index;
	int fcc_sum = 0;

	for (batt_index = 0; batt_index < g_coul_merge_battery_count; batt_index++)
		fcc_sum += coul_merge_drv_battery_fcc_design(batt_index);

	return fcc_sum;
}

int coul_merge_battery_health(void)
{
	int health[COUL_MERGE_BATTERY_COUNT], batt_index;

	for (batt_index = 0; batt_index < g_coul_merge_battery_count; batt_index++) {
		health[batt_index] = coul_merge_drv_battery_health(batt_index);

		if(health[batt_index] != POWER_SUPPLY_HEALTH_GOOD) {
			coul_merge_err("%s: The health status of the batt %d is 0x%x\n",
					__func__, batt_index, health[batt_index]);
			return health[batt_index];
		}
	}

	return POWER_SUPPLY_HEALTH_GOOD;
}

/* battery health level 0: Unknown, 1:CRITICAL, 2:LOW, 3:NORMAL, 4:HIGH, 5:FULL */
static int coul_merge_battery_capacity_level(void)
{
	struct coul_merge_device *di = g_coul_merge;
	int capacity, status;

	if (di == NULL)
		return POWER_SUPPLY_CAPACITY_LEVEL_UNKNOWN;

	if (!coul_merge_is_battery_exist())
		return POWER_SUPPLY_CAPACITY_LEVEL_UNKNOWN;

	capacity = di->soc;
	if ((capacity > CAPACITY_FULL) || (capacity < CAPACITY_MIN))
		status = POWER_SUPPLY_CAPACITY_LEVEL_UNKNOWN;
	else if ((capacity >= CAPACITY_MIN) && (capacity <= CAPACITY_CRITICAL))
		status = POWER_SUPPLY_CAPACITY_LEVEL_CRITICAL;
	else if ((capacity > CAPACITY_CRITICAL) && (capacity <= CAPACITY_LOW))
		status = POWER_SUPPLY_CAPACITY_LEVEL_LOW;
	else if ((capacity >= CAPACITY_HIGH) && (capacity < CAPACITY_FULL))
		status = POWER_SUPPLY_CAPACITY_LEVEL_HIGH;
	else if (capacity == CAPACITY_FULL)
		status = POWER_SUPPLY_CAPACITY_LEVEL_FULL;
	else
		status = POWER_SUPPLY_CAPACITY_LEVEL_NORMAL;

	return status;
}

int coul_merge_battery_technology(void)
{
	return coul_merge_drv_battery_technology(BATT_0);
}

int coul_merge_battery_vbat_max(void)
{
	int vbat, batt_index;
	int vbat_max = INT_MIN;

	for (batt_index = 0; batt_index < g_coul_merge_battery_count; batt_index++) {
		vbat = coul_merge_drv_battery_vbat_max(batt_index);
		vbat_max = max(vbat, vbat_max);
	}

	return vbat_max;
}

int coul_merge_battery_cycle_count(void)
{
	int cycle, batt_index;
	int cycle_max = INT_MIN;

	for (batt_index = 0; batt_index < g_coul_merge_battery_count; batt_index++) {
		cycle = coul_merge_drv_battery_cycle_count(batt_index);
		cycle_max = max(cycle, cycle_max);
	}

	return cycle_max;
}

int coul_merge_battery_get_limit_fcc(void)
{
	int batt_index;
	int fcc_sum = 0;

	for (batt_index = 0; batt_index < g_coul_merge_battery_count; batt_index++)
		fcc_sum += coul_merge_drv_battery_get_limit_fcc(batt_index);

	return fcc_sum;
}

int coul_merge_low_temp_opt(void)
{
	return coul_merge_drv_low_temp_opt(BATT_0);
}

int coul_merge_battery_cc(void)
{
	int batt_index;
	int cc_sum = 0;

	for (batt_index = 0; batt_index < g_coul_merge_battery_count; batt_index++)
		cc_sum += coul_merge_drv_battery_cc_uah(batt_index);

	return cc_sum;
}

int coul_merge_battery_cc_cache(void)
{
	int batt_index;
	int cc_sum = 0;

	for (batt_index = 0; batt_index < g_coul_merge_battery_count; batt_index++)
		cc_sum += coul_merge_drv_battery_cc_cache(batt_index);

	return cc_sum;
}

int coul_merge_battery_get_qmax(void)
{
	int batt_index;
	int qmax_sum = 0;

	for (batt_index = 0; batt_index < g_coul_merge_battery_count; batt_index++)
		qmax_sum += coul_merge_drv_battery_get_qmax(batt_index);

	return qmax_sum;
}

static int coul_merge_get_soc_by_ocv(int temp, int ocv)
{
	return coul_merge_drv_get_soc_by_ocv(BATT_0, temp, ocv);
}

/*
 * put the new event en queue
 * if the event_queue is full, return -ENOSPC
 */
static int coul_event_enqueue(struct coul_merge_device *di,
		  unsigned int *event)
{
	unsigned long flags;
	int ret = 0;

	spin_lock_irqsave(&(di->event_lock), flags);
	if (kfifo_in(&di->event_fifo, event, 1) == 0) {
		coul_merge_err("drop event %d\n", *event);
		ret = -ENOSPC;
	}
	spin_unlock_irqrestore(&(di->event_lock), flags);

	return ret;
}

/*
 * get event frome event_queue
 * return the numbers of event dequeued, currently it is 1
 */
static int coul_event_dequeue(struct coul_merge_device *di,
		  unsigned int *event)
{
	return kfifo_out_spinlocked(&di->event_fifo, event,
				    1, &di->event_lock);
}

int coul_merge_charger_event_rcv(unsigned int event)
{
	struct coul_merge_device *di = g_coul_merge;

	if (di == NULL) {
		coul_merge_err("%s di is NULL\n");
		return -1;
	}

	if (!coul_event_enqueue(di, &event)) {
		queue_delayed_work(system_power_efficient_wq,
			&di->coul_rcv_event_work, 0);
	} else {
		coul_merge_err("can't enqueue event:%d\n", event);
		return -EBUSY;
	}

	return 0;
}

static void coul_merge_rcv_event_work(struct work_struct *work)
{
	unsigned int event = 0;
	int batt_index;
	struct coul_merge_device *di = container_of(work,
		struct coul_merge_device, coul_rcv_event_work.work);

	coul_merge_info("%s +\n", __func__);
	mutex_lock(&di->lock);
	while (coul_event_dequeue(di, &event)) {
		for (batt_index = 0; batt_index < g_coul_merge_battery_count; batt_index++)
			coul_merge_drv_charger_event_rcv(batt_index, event);
	}
	mutex_unlock(&di->lock);
	coul_merge_info("%s -\n", __func__);
}

static int coul_merge_update_basp_policy(unsigned int level,
	unsigned int nondc_volt_dec)
{
	int batt_index;
	int ret = 0;

	for (batt_index = 0; batt_index < g_coul_merge_battery_count; batt_index++)
		ret += coul_merge_drv_battery_update_basp_policy(batt_index, level, nondc_volt_dec);

	return ret;
}

static int coul_merge_is_smart_battery(void)
{
	return coul_merge_drv_is_smart_battery(BATT_0);
}

static int coul_merge_get_desired_charging_current(void)
{
	int curr, batt_index;
	int curr_min = INT_MAX;

	for (batt_index = 0; batt_index < g_coul_merge_battery_count; batt_index++) {
		curr = coul_merge_drv_get_desired_charging_current(batt_index);
		curr_min = min(curr, curr_min);
	}

	return curr_min;
}

static int coul_merge_get_desired_charging_voltage(void)
{
	int vol, batt_index;
	int vol_min = INT_MAX;

	for (batt_index = 0; batt_index < g_coul_merge_battery_count; batt_index++) {
		vol = coul_merge_drv_get_desired_charging_voltage(batt_index);
		vol_min = min(vol, vol_min);
	}

	return vol_min;
}

static void coul_merge_set_work_interval(struct coul_merge_device *di)
{
	if (di->soc > LOW_SOC)
		di->soc_work_interval = CALCULATE_SOC_MS;
	else
		di->soc_work_interval = CALCULATE_SOC_MS / HALF;

#ifdef CONFIG_DIRECT_CHARGER
	if (direct_charge_in_charging_stage() == DC_IN_CHARGING_STAGE &&
		(coul_merge_drv_get_charge_state(BATT_0) == CHARGING_STATE_CHARGE_START) &&
		(di->soc_work_interval > (CALCULATE_SOC_MS / QUARTER)))
		di->soc_work_interval = CALCULATE_SOC_MS / QUARTER;
#endif
}

static void batt_soc_algo_work(void)
{
	int batt_index;

	for (batt_index = 0; batt_index < g_coul_merge_battery_count; batt_index++)
		coul_merge_drv_calc(batt_index);
}

static void coul_merge_monitor_work(struct work_struct *work)
{
	struct coul_merge_device *di = container_of(work,
		struct coul_merge_device, calculate_soc_delayed_work.work);

	mutex_lock(&di->lock);
	coul_merge_info("%s start\n", __func__);
	batt_soc_algo_work();
	coul_merge_battery_capacity();
	coul_merge_set_work_interval(di);
	coul_merge_info("%s end\n", __func__);
	mutex_unlock(&di->lock);

	if (!coul_merge_is_battery_exist()) {
		coul_merge_info("battery not exist, do not calc soc any more\n");
		return;
	}

	queue_delayed_work(system_power_efficient_wq,
		&di->calculate_soc_delayed_work, msecs_to_jiffies(di->soc_work_interval));
}

static void coul_merge_resume_work(struct work_struct *work)
{
	int batt_index;
	struct coul_merge_device *di = container_of(work,
		struct coul_merge_device, merge_resume_work);

	coul_merge_info("%s: +\n", __func__);

	if (di == NULL) {
		coul_merge_err("%s: failed to get coul_merge_device\n", __func__);
		return;
	}

	if (!di->resume_wake_lock) {
		coul_merge_err("%s: failed to get resume_wake_lock\n", __func__);
		return;
	}

	for (batt_index = 0; batt_index < g_coul_merge_battery_count; batt_index++)
		coul_merge_drv_resume(batt_index);

	coul_merge_battery_capacity();

	__pm_relax(di->resume_wake_lock);

	coul_merge_info("%s: -\n", __func__);
}

static void coul_merge_set_switch_on_voltage(unsigned int voltage)
{
	unsigned int regval;

	if (voltage < SWITCH_ON_VOLT_BASE) {
		coul_merge_err("%s error volt=0x%x\n", __func__, voltage);
		return;
	}

	regval = (voltage - SWITCH_ON_VOLT_BASE) / SWITCH_ON_VOLT_STEP;
	regval = (regval << PMIC_SWITCH_ON_VOLT_OFFSET) & PMIC_SWITCH_ON_VOLT_MASK_H;
	pmic_write_reg(PMIC_SWITCH_ON_VOLT_ADDR_H, (unsigned char)regval);
	coul_merge_info("%s voltage=%d, write val =0x%x\n", __func__, voltage, regval);
}

static void get_switch_on_voltage_dts(const struct device_node *np)
{
	int ret;
	unsigned int switch_on_voltage = 0;

	ret = of_property_read_u32(np, "switch_on_voltage", &switch_on_voltage);
	if (ret) {
		coul_merge_info("%s can not get switch_on_voltage.\n", __func__);
		return;
	}
	coul_merge_info("%s switch_on_voltage %d\n", __func__, switch_on_voltage);
	coul_merge_set_switch_on_voltage(switch_on_voltage);
}

static void coul_merge_parse_dts(struct coul_merge_device *di)
{
	struct device_node *np = di->dev->of_node;
	int ret;

	ret = of_property_read_u32(np, "is_series_bat", &di->is_series_bat);
	if (ret) {
		coul_merge_info("dts:can not get is_series_bat,use default 0\n");
		di->is_series_bat = 0;
	} else {
		coul_merge_info("dts:get is_series_bat = %u\n", di->is_series_bat);
	}

	ret = of_property_read_u32(np, "cap_ratio_0", &di->cap_ratio_0);
	if (ret) {
		coul_merge_info("dts:can not get cap_ratio_0,use default 1000\n");
		di->cap_ratio_0 = 1000;
	} else {
		coul_merge_info("dts:get cap_ratio_0 = %u\n", di->cap_ratio_0);
	}

	ret = of_property_read_u32(np, "cap_ratio_1", &di->cap_ratio_1);
	if (ret) {
		coul_merge_info("dts:can not get cap_ratio_1,use default 0\n");
		di->cap_ratio_1 = 0;
	} else {
		coul_merge_info("dts:get cap_ratio_1 = %u\n", di->cap_ratio_1);
	}

	ret = of_property_read_u32(np, "cap_ratio_2", &di->cap_ratio_2);
	if (ret) {
		coul_merge_info("dts:can not get cap_ratio_2,use default 0\n");
		di->cap_ratio_2 = 0;
	} else {
		coul_merge_info("dts:get cap_ratio_2 = %u\n", di->cap_ratio_2);
	}

	get_switch_on_voltage_dts(np);
}

static int coul_merge_shutdown_prepare(struct notifier_block *nb,
	unsigned long event, void *data)
{
	struct coul_merge_device *di =
		container_of(nb, struct coul_merge_device, reboot_nb);
	int batt_index;

	switch (event) {
	case SYS_DOWN:
	case SYS_HALT:
	case SYS_POWER_OFF:
		coul_merge_info("coul prepare to shutdown, event = %lu\n", event);
		cancel_delayed_work_sync(&di->calculate_soc_delayed_work);
		for (batt_index = g_coul_merge_battery_count - 1; batt_index >= 0; batt_index--)
			coul_merge_drv_shutdown(batt_index);
		break;
	default:
		coul_merge_err("error event, coul ignore, event = %lu\n", event);
		break;
	}
	return 0;
}

#ifdef CONFIG_PM
/* suspend function, called when coul enter sleep, v9 no sleep */
static int coul_merge_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct coul_merge_device *di = platform_get_drvdata(pdev);
	int batt_index;

	coul_merge_info("%s: +\n", __func__);

	if (coul_merge_is_battery_exist())
		cancel_delayed_work(&di->calculate_soc_delayed_work);

	/* The sequence of BATT_1 and BATT_0 cannot be changed.
	    because only BATT_0 controls the chip enter or out the ECO.
	*/
	for (batt_index = g_coul_merge_battery_count - 1; batt_index >= 0; batt_index--)
		coul_merge_drv_suspend(batt_index);

	coul_merge_info("%s: -\n", __func__);

	return 0;
}

/* called when coul wakeup from deep sleep */
static int coul_merge_resume(struct platform_device *pdev)
{
	struct coul_merge_device *di = platform_get_drvdata(pdev);

	coul_merge_info("%s: +\n", __func__);

	di->soc_work_interval = CALCULATE_SOC_MS / HALF;
	queue_delayed_work(system_power_efficient_wq,
			&di->calculate_soc_delayed_work,
			round_jiffies_relative(msecs_to_jiffies(
				(unsigned int)di->soc_work_interval)));

	/* The sequence of BATT_1 and BATT_0 cannot be changed.
	    because only BATT_0 controls the chip enter or out the ECO.
	*/
	__pm_stay_awake(di->resume_wake_lock);
	queue_work(system_power_efficient_wq, &di->merge_resume_work);

	coul_merge_info("%s: -\n", __func__);
	return 0;
}
#endif

static int coul_merge_coul_is_ready(void)
{
	if(g_coul_merge)
		return 1;
	else
		return 0;
}

/* 0: invalid battery, 1: successed */
static struct chrg_para_lut *coul_merge_battery_charge_params(void)
{
	return coul_merge_drv_battery_charge_params(BATT_0);
}

static void get_cur_calibration(int *cur_offset_a, int *cur_offset_b)
{
	struct coul_batt_cali_info info = { 0 };
	info.batt_index = 0;
	info.charge_mode = PARALLEL_MODE;
	info.data_type = BATT_DATA_CUR_CALI;
	info.a = DEFAULT_C_OFF_A;
	info.b = DEFAULT_C_OFF_B;

	if (cur_offset_a == NULL || cur_offset_b == NULL) {
		coul_merge_err("%s input para is null\n", __func__);
		return;
	}

	if (coul_merge_drv_get_cali(&info))
		coul_merge_err("%s failed, use DEFAULT_C_OFF_A and DEFAULT_C_OFF_B!\n", __func__);

	*cur_offset_a = info.a;
	*cur_offset_b = info.b;
}

static int is_cathode_type_silicon(char type)
{
	int i;

	for (i = 0; i < TYPE_SILICON_CHARS_LEN; i++) {
		if (type == g_type_silicon_chars[i])
			return 1;
	}
	return 0;
}

static enum bat_model_bat_type coul_merge_get_cathode_type(void)
{
	char buf[BATTRY_SN_CHAR_SIZE] = {0};
	enum bat_model_bat_type ret_type = BAT_CATHODE_TYPE_GRAPHITE;
	char type;
	int ret;

	ret = get_battery_identifier(buf, BATTRY_SN_CHAR_SIZE, 0, BATTRY_SN_LEN);
	if (ret) {
		coul_merge_err("%s error get_battery_identifier, ret=%d\n", __func__, ret);
		return ret_type;
	}

	type = buf[CATHODE_INDEX];
	if (is_cathode_type_silicon(type))
		ret_type = BAT_CATHODE_TYPE_SILICON;
#ifdef CONFIG_DFX_DEBUG_FS
	coul_merge_info("%s get_battery_identifier buf=%s, ret_type=%d\n",
		__func__, buf, ret_type);
#endif
	return ret_type;
}

struct coulometer_ops coul_merge_ops = {
	.is_coul_ready = coul_merge_coul_is_ready,
	.is_battery_exist = coul_merge_is_battery_exist,
	.is_battery_reach_threshold = coul_merge_is_battery_reach_threshold,
	.battery_brand = coul_merge_battery_brand,
	.battery_id_voltage = coul_merge_battery_id_voltage,
	.battery_voltage = coul_merge_battery_voltage,
	.battery_voltage_uv = coul_merge_battery_voltage_uv,
	.battery_current = coul_merge_battery_current,
	.battery_resistance = coul_merge_battery_resistance,
	.fifo_avg_current = coul_merge_fifo_avg_current,
	.battery_current_avg = coul_merge_battery_current_avg,
	/* The coul_merge_battery_capacity function cannot be used.
	   Otherwise, the startup SOC will be overwritten. */
	.battery_capacity = coul_merge_get_capacity,
	.battery_unfiltered_capacity = coul_merge_get_unfiltered_capacity,
	.battery_temperature = coul_merge_battery_temperature,
	.battery_temperature_for_charger =
		coul_merge_battery_temperature,
	.battery_rm = coul_merge_battery_rm,
	.battery_fcc = coul_merge_battery_fcc,
	.battery_fcc_design = coul_merge_battery_fcc_design,
	.battery_health = coul_merge_battery_health,
	.battery_capacity_level = coul_merge_battery_capacity_level,
	.battery_technology = coul_merge_battery_technology,
	.battery_vbat_max = coul_merge_battery_vbat_max,
	.charger_event_rcv = coul_merge_charger_event_rcv,
	.battery_cycle_count = coul_merge_battery_cycle_count,
	.get_battery_limit_fcc = coul_merge_battery_get_limit_fcc,
	.coul_low_temp_opt = coul_merge_low_temp_opt,
	.battery_cc = coul_merge_battery_cc,
	.battery_cc_cache = coul_merge_battery_cc_cache,
	.get_qmax = coul_merge_battery_get_qmax,
	.get_soc_by_ocv = coul_merge_get_soc_by_ocv,
	.battery_charge_params = coul_merge_battery_charge_params,
	.update_basp_policy = coul_merge_update_basp_policy,
	.is_smart_battery = coul_merge_is_smart_battery,
	.get_desired_charging_current = coul_merge_get_desired_charging_current,
	.get_desired_charging_voltage = coul_merge_get_desired_charging_voltage,
	.get_c_calibration = get_cur_calibration,
	.get_cathode_type = coul_merge_get_cathode_type,
	.chip_temperature = coul_merge_battery_temperature,
	.battery_removed_before_boot = coul_merge_battery_removed_before_boot,
#ifdef CONFIG_BATT_SOH
	.convert_mv2regval = coul_merge_convert_mv2regval,
	.convert_regval2ua = coul_merge_convert_regval2ua,
	.convert_regval2uv = coul_merge_convert_regval2uv,
#endif
};

static void coul_merge_init_soc(struct coul_merge_device *di)
{
	batt_soc_algo_work();
	coul_merge_battery_capacity();
}

static int coul_merge_probe(struct platform_device *pdev)
{
	struct coul_merge_device *di = NULL;
	int ret;

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	di = devm_kzalloc(&pdev->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	di->dev = &pdev->dev;
	coul_merge_parse_dts(di);
	mutex_init(&di->lock);
	spin_lock_init(&di->event_lock);
	INIT_KFIFO(di->event_fifo);
	di->soc_work_interval = CALCULATE_SOC_MS;
	INIT_DELAYED_WORK(&di->calculate_soc_delayed_work, coul_merge_monitor_work);
	INIT_DELAYED_WORK(&di->coul_rcv_event_work, coul_merge_rcv_event_work);
	INIT_WORK(&di->merge_resume_work, coul_merge_resume_work);
	di->reboot_nb.notifier_call = coul_merge_shutdown_prepare;
	register_reboot_notifier(&(di->reboot_nb));

	platform_set_drvdata(pdev, di);
	g_coul_merge = di;
	g_coul_merge_battery_count = coul_merge_battery_count();
	coul_merge_init_soc(di);

	/* create sysfs */
	ret = coul_merge_create_sysfs(di);
	if (ret) {
		coul_merge_err("%s failed to create sysfs\n", __func__);
		return -1;
	}

	queue_delayed_work(system_power_efficient_wq,
		&di->calculate_soc_delayed_work, msecs_to_jiffies(di->soc_work_interval));

	ret = coul_drv_coul_ops_register(&coul_merge_ops, KCOUL);
	if (ret)
		coul_merge_err("%s: coul_merge_ops fail\n", __func__);

	di->resume_wake_lock = wakeup_source_register(di->dev, "coulresume_wakelock");
	if (!di->resume_wake_lock) {
		coul_merge_err("%s resume_wake_lock register failed\n", __func__);
		return -1;
	}

	coul_merge_info("%s: succ, soc %d\n", __func__, di->soc);
	return 0;
}

static int coul_merge_remove(struct platform_device *pdev)
{
	struct coul_merge_device *di = platform_get_drvdata(pdev);

	if (!di)
		return -ENODEV;
	mutex_destroy(&di->lock);
	g_coul_merge = NULL;
	return 0;
}

static const struct of_device_id coul_merge_match_table[] = {
	{
		.compatible = "hisilicon,coul_merge",
		.data = NULL,
	},
	{},
};

static struct platform_driver coul_merge_driver = {
	.probe = coul_merge_probe,
	.remove = coul_merge_remove,
#ifdef CONFIG_PM
	.suspend = coul_merge_suspend,
	.resume = coul_merge_resume,
#endif
	.driver = {
		.name = "hisilicon,coul_merge",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(coul_merge_match_table),
	},
};

static int __init coul_merge_init(void)
{
	return  platform_driver_register(&coul_merge_driver);
}

static void __exit coul_merge_exit(void)
{
	platform_driver_unregister(&coul_merge_driver);
}

rootfs_initcall(coul_merge_init);
module_exit(coul_merge_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("battery driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
