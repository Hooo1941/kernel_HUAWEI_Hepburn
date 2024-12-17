/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2013-2020. All rights reserved.
 * Description: this file provide interface to get the battery state such as
 *          capacity, voltage, current, temperature
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
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/power_supply.h>
#ifdef CONFIG_HUAWEI_HW_DEV_DCT
#include <huawei_platform/devdetect/hw_dev_dec.h>
#endif
#include <securec.h>

#include <platform_include/basicplatform/linux/power/platform/coul/coul_merge_drv.h>

#define coul_merge_drv_debug(fmt, args...) pr_debug("[coul_merge_drv]" fmt, ## args)
#define coul_merge_drv_info(fmt, args...) pr_info("[coul_merge_drv]" fmt, ## args)
#define coul_merge_drv_warn(fmt, args...) pr_warn("[coul_merge_drv]" fmt, ## args)
#define coul_merge_drv_err(fmt, args...) pr_err("[coul_merge_drv]" fmt, ## args)

static int g_coul_merge_drv_init;
static struct mutex g_coul_merge_drv_lock;
#define coul_merge_drv_lock_f() do { \
	if (!g_coul_merge_drv_init) return; \
	mutex_lock(&g_coul_merge_drv_lock); \
} while (0)
#define coul_merge_drv_unlock_f() mutex_unlock(&g_coul_merge_drv_lock)

struct coulometer_drv {
	struct coul_merge_drv_ops *g_coul_ops;
	void *data;
	int batt_index;
	const char *batt_name;
	int init_flag;
};

struct coulometer_drv g_coul_merge_drv_list[BATT_MAX];
static int g_batt_cnt;

static struct coulometer_drv *coul_merge_drv_get_info(int batt_index)
{
	if (batt_index >= BATT_MAX)
		return NULL;

	if (g_coul_merge_drv_list[batt_index].init_flag == 0)
		return NULL;

	return &g_coul_merge_drv_list[batt_index];
}

int coul_merge_drv_get_batt_name(int batt_index, char *name, int size_max)
{
	int ret = 0;
	struct coulometer_drv *coul_merge_drv = NULL;

	coul_merge_drv = coul_merge_drv_get_info(batt_index);
	if(coul_merge_drv == NULL) {
		coul_merge_drv_err("%s coul_merge_drv is null, index %d\n", __func__, batt_index);
		return -1;
	}
	
	ret = memcpy_s(name, size_max, coul_merge_drv->batt_name, strlen(coul_merge_drv->batt_name) + 1);
	if (ret != EOK) {
		coul_merge_drv_err("[%s] memcpy_s failed\n",__func__);
		return -1;
	}

	return 0;
}

int coul_merge_drv_get_batt_index(const char *name_ptr, int *batt_index)
{
	int i;
	struct coulometer_drv *coul_merge_drv = NULL;

	if (name_ptr == NULL || batt_index == NULL)
		return -1;

	for (i = 0; i < BATT_MAX; i++) {
		coul_merge_drv = coul_merge_drv_get_info(i);
		if (coul_merge_drv == NULL)
			return -1;
		if (strcmp(coul_merge_drv->batt_name, name_ptr) == 0) {
			*batt_index = i;
			return 0;
		}
	}

	return -1;
}


int coul_merge_drv_is_coul_ready(int batt_index)
{
	struct coulometer_drv *coul_merge_drv = NULL;
	struct coul_merge_drv_ops *ops = NULL;

	coul_merge_drv = coul_merge_drv_get_info(batt_index);
	if(coul_merge_drv == NULL)
		return 0;

	ops = coul_merge_drv->g_coul_ops;

	if (ops && ops->is_coul_ready)
		return ops->is_coul_ready(coul_merge_drv->data);

	return 0;
}

void coul_merge_drv_calc(int batt_index)
{
	struct coulometer_drv *coul_merge_drv = NULL;
	struct coul_merge_drv_ops *ops = NULL;

	coul_merge_drv = coul_merge_drv_get_info(batt_index);
	if(coul_merge_drv == NULL)
		return;

	ops = coul_merge_drv->g_coul_ops;

	if (ops && ops->coul_calc_work)
		ops->coul_calc_work(coul_merge_drv->data);

	return;
}


/* check whether fcc is debounce */
int coul_merge_drv_is_fcc_debounce(int batt_index)
{
	struct coulometer_drv *coul_merge_drv = NULL;
	struct coul_merge_drv_ops *ops = NULL;

	coul_merge_drv = coul_merge_drv_get_info(batt_index);
	if(coul_merge_drv == NULL)
		return 0;

	ops = coul_merge_drv->g_coul_ops;

	/* execute the operation of coul module */
	if (ops && ops->coul_is_fcc_debounce)
		return ops->coul_is_fcc_debounce(coul_merge_drv->data);
	return 0;
}

/* get battery polar avg */
int coul_merge_drv_get_polar_avg(int batt_index)
{
	struct coulometer_drv *coul_merge_drv = NULL;
	struct coul_merge_drv_ops *ops = NULL;

	coul_merge_drv = coul_merge_drv_get_info(batt_index);
	if(coul_merge_drv == NULL)
		return 0;

	ops = coul_merge_drv->g_coul_ops;

	/* execute the operation of coul module */
	if (ops && ops->get_polar_avg)
		return ops->get_polar_avg(coul_merge_drv->data);

	return 0;
}

int coul_merge_drv_get_polar_peak(int batt_index)
{
	struct coulometer_drv *coul_merge_drv = NULL;
	struct coul_merge_drv_ops *ops = NULL;

	coul_merge_drv = coul_merge_drv_get_info(batt_index);
	if(coul_merge_drv == NULL)
		return 0;
	ops = coul_merge_drv->g_coul_ops;

	/* execute the operation of coul module */
	if (ops && ops->get_polar_peak)
		return ops->get_polar_peak(coul_merge_drv->data);

	return 0;
}

int coul_merge_drv_battery_get_qmax(int batt_index)
{
	struct coulometer_drv *coul_merge_drv = NULL;
	struct coul_merge_drv_ops *ops = NULL;

	coul_merge_drv = coul_merge_drv_get_info(batt_index);
	if(coul_merge_drv == NULL)
		return 0;
	ops = coul_merge_drv->g_coul_ops;

	/* execute the operation of coul module */
	if (ops && ops->get_qmax)
		return ops->get_qmax(coul_merge_drv->data);

	return 0;
}

int coul_merge_drv_battery_update_basp_policy(int batt_index, unsigned int level,
	unsigned int nondc_volt_dec)
{
	struct coulometer_drv *coul_merge_drv = NULL;
	struct coul_merge_drv_ops *ops = NULL;

	coul_merge_drv = coul_merge_drv_get_info(batt_index);
	if(coul_merge_drv == NULL)
		return 0;
	ops = coul_merge_drv->g_coul_ops;

	/* execute the operation of coul module */
	if (ops && ops->update_basp_policy)
		return ops->update_basp_policy(coul_merge_drv->data, level,
					nondc_volt_dec);

	return -EPERM;
}

int coul_merge_drv_battery_get_recrod_fcc(int batt_index, unsigned int size, unsigned int *records)
{
	struct coulometer_drv *coul_merge_drv = NULL;
	struct coul_merge_drv_ops *ops = NULL;

	coul_merge_drv = coul_merge_drv_get_info(batt_index);
	if(coul_merge_drv == NULL)
		return 0;
	ops = coul_merge_drv->g_coul_ops;

	if (ops && ops->get_record_fcc)
		return ops->get_record_fcc(coul_merge_drv->data, size, records);

	return -EPERM;
}

int coul_merge_drv_is_battery_exist(int batt_index)
{
	struct coulometer_drv *coul_merge_drv = NULL;
	struct coul_merge_drv_ops *ops = NULL;

	coul_merge_drv = coul_merge_drv_get_info(batt_index);
	if(coul_merge_drv == NULL)
		return 0;
	ops = coul_merge_drv->g_coul_ops;

	/* execute the operation of coul module */
	if (ops && ops->is_battery_exist)
		return ops->is_battery_exist(coul_merge_drv->data);

	return 0;
}

/* check whether remaining capacity of battery reach the low power threshold */
int coul_merge_drv_is_battery_reach_threshold(int batt_index)
{
	struct coulometer_drv *coul_merge_drv = NULL;
	struct coul_merge_drv_ops *ops = NULL;

	coul_merge_drv = coul_merge_drv_get_info(batt_index);
	if(coul_merge_drv == NULL)
		return 0;
	ops = coul_merge_drv->g_coul_ops;

	if (ops && ops->is_battery_reach_threshold)
		return ops->is_battery_reach_threshold(coul_merge_drv->data);

	return 0;
}

char *coul_merge_drv_battery_brand(int batt_index)
{
	struct coulometer_drv *coul_merge_drv = NULL;
	struct coul_merge_drv_ops *ops = NULL;

	coul_merge_drv = coul_merge_drv_get_info(batt_index);
	if(coul_merge_drv == NULL)
		return 0;
	ops = coul_merge_drv->g_coul_ops;

	if (ops && ops->battery_brand)
		return ops->battery_brand(coul_merge_drv->data);

	return "error";
}

int coul_merge_drv_battery_id_voltage(int batt_index)
{
	struct coulometer_drv *coul_merge_drv = NULL;
	struct coul_merge_drv_ops *ops = NULL;

	coul_merge_drv = coul_merge_drv_get_info(batt_index);
	if(coul_merge_drv == NULL)
		return 0;
	ops = coul_merge_drv->g_coul_ops;

	if (ops && ops->battery_id_voltage)
		return ops->battery_id_voltage(coul_merge_drv->data);

	return -EPERM;
}

int coul_merge_drv_battery_voltage_mv(int batt_index)
{
	struct coulometer_drv *coul_merge_drv = NULL;
	struct coul_merge_drv_ops *ops = NULL;

	coul_merge_drv = coul_merge_drv_get_info(batt_index);
	if(coul_merge_drv == NULL)
		return 0;
	ops = coul_merge_drv->g_coul_ops;

	/* execute the operation of coul module */
	if (ops && ops->battery_voltage)
		return ops->battery_voltage(coul_merge_drv->data);

	return -EPERM;
}

int coul_merge_drv_battery_voltage_uv(int batt_index)
{
	struct coulometer_drv *coul_merge_drv = NULL;
	struct coul_merge_drv_ops *ops = NULL;

	coul_merge_drv = coul_merge_drv_get_info(batt_index);
	if(coul_merge_drv == NULL)
		return 0;
	ops = coul_merge_drv->g_coul_ops;

	if (ops && ops->battery_voltage_uv)
		return ops->battery_voltage_uv(coul_merge_drv->data);

	return -EPERM;
}

/* Charging is Positive value, discharge is negative */
int coul_merge_drv_battery_current(int batt_index)
{
	struct coulometer_drv *coul_merge_drv = NULL;
	struct coul_merge_drv_ops *ops = NULL;

	coul_merge_drv = coul_merge_drv_get_info(batt_index);
	if(coul_merge_drv == NULL)
		return 0;
	ops = coul_merge_drv->g_coul_ops;

	/* execute the operation of coul module */
	if (ops && ops->battery_current)
		return ops->battery_current(coul_merge_drv->data);

	return 0;
}

int coul_merge_drv_battery_resistance(int batt_index)
{
	struct coulometer_drv *coul_merge_drv = NULL;
	struct coul_merge_drv_ops *ops = NULL;

	coul_merge_drv = coul_merge_drv_get_info(batt_index);
	if(coul_merge_drv == NULL)
		return 0;
	ops = coul_merge_drv->g_coul_ops;

	if (ops && ops->battery_resistance)
		return ops->battery_resistance(coul_merge_drv->data);

	return 0;
}

/* Charging is Positive value, discharge is negative */
int coul_merge_drv_fifo_avg_current(int batt_index)
{
	struct coulometer_drv *coul_merge_drv = NULL;
	struct coul_merge_drv_ops *ops = NULL;

	coul_merge_drv = coul_merge_drv_get_info(batt_index);
	if(coul_merge_drv == NULL)
		return 0;
	ops = coul_merge_drv->g_coul_ops;

	if (ops && ops->fifo_avg_current)
		return ops->fifo_avg_current(coul_merge_drv->data);

	return 0;
}

int coul_merge_drv_battery_current_avg(int batt_index)
{
	struct coulometer_drv *coul_merge_drv = NULL;
	struct coul_merge_drv_ops *ops = NULL;

	coul_merge_drv = coul_merge_drv_get_info(batt_index);
	if(coul_merge_drv == NULL)
		return 0;
	ops = coul_merge_drv->g_coul_ops;

	if (ops && ops->battery_current_avg)
		return ops->battery_current_avg(coul_merge_drv->data);

	return 0;
}

int coul_merge_drv_battery_unfiltered_capacity(int batt_index)
{
	struct coulometer_drv *coul_merge_drv = NULL;
	struct coul_merge_drv_ops *ops = NULL;

	coul_merge_drv = coul_merge_drv_get_info(batt_index);
	if(coul_merge_drv == NULL)
		return 0;
	ops = coul_merge_drv->g_coul_ops;

	if (ops && ops->battery_unfiltered_capacity)
		return ops->battery_unfiltered_capacity(coul_merge_drv->data);

	return -EPERM;
}

int coul_merge_drv_battery_capacity(int batt_index)
{
	struct coulometer_drv *coul_merge_drv = NULL;
	struct coul_merge_drv_ops *ops = NULL;

	coul_merge_drv = coul_merge_drv_get_info(batt_index);
	if(coul_merge_drv == NULL)
		return 0;
	ops = coul_merge_drv->g_coul_ops;

	if (ops && ops->battery_capacity)
		return ops->battery_capacity(coul_merge_drv->data);

	return -EPERM;
}

int coul_merge_drv_battery_temperature_raw(unsigned int batt_index)
{
	struct coulometer_drv *coul_merge_drv = NULL;
	struct coul_merge_drv_ops *ops = NULL;

	coul_merge_drv = coul_merge_drv_get_info(batt_index);
	if(coul_merge_drv == NULL)
		return 0;
	ops = coul_merge_drv->g_coul_ops;

		/* execute the operation of coul module */
	if (ops && ops->battery_raw_temperature)
		return ops->battery_raw_temperature(coul_merge_drv->data);

	coul_merge_drv_err("%s batt_index %d, ops error\n", __func__);
	return -EPERM;
}

int coul_merge_drv_battery_temperature(int batt_index)
{
	struct coulometer_drv *coul_merge_drv = NULL;
	struct coul_merge_drv_ops *ops = NULL;

	coul_merge_drv = coul_merge_drv_get_info(batt_index);
	if(coul_merge_drv == NULL)
		return 0;
	ops = coul_merge_drv->g_coul_ops;

	/* execute the operation of coul module */
	if (ops && ops->battery_temperature)
		return ops->battery_temperature(coul_merge_drv->data);

	coul_merge_drv_err("%s batt_index %d, ops error\n", __func__);
	return -EPERM;
}

int coul_merge_drv_battery_temperature_for_charger(int batt_index)
{
	struct coulometer_drv *coul_merge_drv = NULL;
	struct coul_merge_drv_ops *ops = NULL;

	coul_merge_drv = coul_merge_drv_get_info(batt_index);
	if(coul_merge_drv == NULL)
		return 0;
	ops = coul_merge_drv->g_coul_ops;

	/* execute the operation of coul module */
	if (ops && ops->battery_temperature_for_charger)
		return ops->battery_temperature_for_charger(coul_merge_drv->data);

	return -EPERM;
}

/* monitor soc if vary too fast */
int coul_merge_drv_battery_soc_vary_flag(int batt_index, int monitor_flag, int *deta_soc)
{
	struct coulometer_drv *coul_merge_drv = NULL;
	struct coul_merge_drv_ops *ops = NULL;

	coul_merge_drv = coul_merge_drv_get_info(batt_index);
	if(coul_merge_drv == NULL)
		return 0;
	ops = coul_merge_drv->g_coul_ops;

	/* execute the operation of coul module */
	if (ops && ops->get_soc_vary_flag)
		return ops->get_soc_vary_flag(coul_merge_drv->data, monitor_flag, deta_soc);

	return -EPERM;
}

int coul_merge_drv_battery_rm(int batt_index)
{
	struct coulometer_drv *coul_merge_drv = NULL;
	struct coul_merge_drv_ops *ops = NULL;

	coul_merge_drv = coul_merge_drv_get_info(batt_index);
	if(coul_merge_drv == NULL)
		return 0;
	ops = coul_merge_drv->g_coul_ops;

	/* execute the operation of coul module */
	if (ops && ops->battery_rm)
		return ops->battery_rm(coul_merge_drv->data);

	return 0;
}

int coul_merge_drv_battery_fcc(int batt_index)
{
	struct coulometer_drv *coul_merge_drv = NULL;
	struct coul_merge_drv_ops *ops = NULL;

	coul_merge_drv = coul_merge_drv_get_info(batt_index);
	if(coul_merge_drv == NULL)
		return 0;
	ops = coul_merge_drv->g_coul_ops;

	/* execute the operation of coul module */
	if (ops && ops->battery_fcc)
		return ops->battery_fcc(coul_merge_drv->data);

	return 0;
}

int coul_merge_drv_get_charge_state(int batt_index)
{
	struct coulometer_drv *coul_merge_drv = NULL;
	struct coul_merge_drv_ops *ops = NULL;

	coul_merge_drv = coul_merge_drv_get_info(batt_index);
	if(coul_merge_drv == NULL)
		return 0;
	ops = coul_merge_drv->g_coul_ops;

	/* execute the operation of coul module */
	if (ops && ops->get_charge_state)
		return ops->get_charge_state(coul_merge_drv->data);

	return 0;
}

int coul_merge_drv_battery_fcc_design(int batt_index)
{
	struct coulometer_drv *coul_merge_drv = NULL;
	struct coul_merge_drv_ops *ops = NULL;

	coul_merge_drv = coul_merge_drv_get_info(batt_index);
	if(coul_merge_drv == NULL)
		return 0;
	ops = coul_merge_drv->g_coul_ops;

	/* execute the operation of coul module */
	if (ops && ops->battery_fcc_design)
		return ops->battery_fcc_design(coul_merge_drv->data);

	return 0;
}

int coul_merge_drv_battery_tte(int batt_index)
{
	struct coulometer_drv *coul_merge_drv = NULL;
	struct coul_merge_drv_ops *ops = NULL;

	coul_merge_drv = coul_merge_drv_get_info(batt_index);
	if(coul_merge_drv == NULL)
		return 0;
	ops = coul_merge_drv->g_coul_ops;

	/* execute the operation of coul module */
	if (ops && ops->battery_tte)
		return ops->battery_tte(coul_merge_drv->data);

	return -EPERM;
}

int coul_merge_drv_battery_ttf(int batt_index)
{
	struct coulometer_drv *coul_merge_drv = NULL;
	struct coul_merge_drv_ops *ops = NULL;

	coul_merge_drv = coul_merge_drv_get_info(batt_index);
	if(coul_merge_drv == NULL)
		return 0;
	ops = coul_merge_drv->g_coul_ops;

	/* execute the operation of coul module */
	if (ops && ops->battery_ttf)
		return ops->battery_ttf(coul_merge_drv->data);

	return -EPERM;
}

int coul_merge_drv_battery_health(int batt_index)
{
	struct coulometer_drv *coul_merge_drv = NULL;
	struct coul_merge_drv_ops *ops = NULL;

	coul_merge_drv = coul_merge_drv_get_info(batt_index);
	if(coul_merge_drv == NULL)
		return 0;
	ops = coul_merge_drv->g_coul_ops;

	/* execute the operation of coul module */
	if (ops && ops->battery_health)
		return ops->battery_health(coul_merge_drv->data);

	return 0;
}

int coul_merge_drv_battery_technology(int batt_index)
{
	struct coulometer_drv *coul_merge_drv = NULL;
	struct coul_merge_drv_ops *ops = NULL;

	coul_merge_drv = coul_merge_drv_get_info(batt_index);
	if(coul_merge_drv == NULL)
		return 0;
	ops = coul_merge_drv->g_coul_ops;

	/* execute the operation of coul module */
	if (ops && ops->battery_technology)
		return ops->battery_technology(coul_merge_drv->data);

	return POWER_SUPPLY_TECHNOLOGY_LIPO;
}

struct chrg_para_lut *coul_merge_drv_battery_charge_params(int batt_index)
{
	struct coulometer_drv *coul_merge_drv = NULL;
	struct coul_merge_drv_ops *ops = NULL;

	coul_merge_drv = coul_merge_drv_get_info(batt_index);
	if(coul_merge_drv == NULL)
		return 0;
	ops = coul_merge_drv->g_coul_ops;

	/* execute the operation of coul module */
	if (ops && ops->battery_charge_params)
		return ops->battery_charge_params(coul_merge_drv->data);

	return NULL;
}

int coul_merge_drv_battery_ifull(int batt_index)
{
	struct coulometer_drv *coul_merge_drv = NULL;
	struct coul_merge_drv_ops *ops = NULL;

	coul_merge_drv = coul_merge_drv_get_info(batt_index);
	if(coul_merge_drv == NULL)
		return 0;
	ops = coul_merge_drv->g_coul_ops;

	if (ops && ops->battery_ifull)
		return ops->battery_ifull(coul_merge_drv->data);

	return -EPERM;
}

int coul_merge_drv_battery_vbat_max(int batt_index)
{
	struct coulometer_drv *coul_merge_drv = NULL;
	struct coul_merge_drv_ops *ops = NULL;

	coul_merge_drv = coul_merge_drv_get_info(batt_index);
	if(coul_merge_drv == NULL)
		return 0;
	ops = coul_merge_drv->g_coul_ops;

	if (ops && ops->battery_vbat_max)
		return ops->battery_vbat_max(coul_merge_drv->data);

	return 0;
}

int coul_merge_drv_battery_get_limit_fcc(int batt_index)
{
	struct coulometer_drv *coul_merge_drv = NULL;
	struct coul_merge_drv_ops *ops = NULL;

	coul_merge_drv = coul_merge_drv_get_info(batt_index);
	if(coul_merge_drv == NULL)
		return 0;
	ops = coul_merge_drv->g_coul_ops;

	if (ops && ops->get_battery_limit_fcc)
		return ops->get_battery_limit_fcc(coul_merge_drv->data);

	return -EPERM;
}

int coul_merge_drv_battery_cycle_count(int batt_index)
{
	struct coulometer_drv *coul_merge_drv = NULL;
	struct coul_merge_drv_ops *ops = NULL;

	coul_merge_drv = coul_merge_drv_get_info(batt_index);
	if(coul_merge_drv == NULL)
		return 0;
	ops = coul_merge_drv->g_coul_ops;

	if (ops && ops->battery_cycle_count)
		return ops->battery_cycle_count(coul_merge_drv->data);

	return 0;
}

int coul_merge_drv_battery_cc(int batt_index)
{
	struct coulometer_drv *coul_merge_drv = NULL;
	struct coul_merge_drv_ops *ops = NULL;

	coul_merge_drv = coul_merge_drv_get_info(batt_index);
	if(coul_merge_drv == NULL)
		return 0;

	ops = coul_merge_drv->g_coul_ops;

	if (ops && ops->battery_cc)
		return ops->battery_cc(coul_merge_drv->data) / PERMILLAGE;

	return -EPERM;
}

int coul_merge_drv_battery_cc_cache(int batt_index)
{
	struct coulometer_drv *coul_merge_drv = NULL;
	struct coul_merge_drv_ops *ops = NULL;

	coul_merge_drv = coul_merge_drv_get_info(batt_index);
	if (coul_merge_drv == NULL)
		return 0;

	ops = coul_merge_drv->g_coul_ops;
	if (ops && ops->battery_cc_cache)
		return ops->battery_cc_cache(coul_merge_drv->data) / PERMILLAGE;

	return -EPERM;
}

int coul_merge_drv_battery_cc_uah(int batt_index)
{
	struct coulometer_drv *coul_merge_drv = NULL;
	struct coul_merge_drv_ops *ops = NULL;

	coul_merge_drv = coul_merge_drv_get_info(batt_index);
	if(coul_merge_drv == NULL)
		return 0;
	ops = coul_merge_drv->g_coul_ops;

	if (ops && ops->battery_cc)
		return ops->battery_cc(coul_merge_drv->data);

	return -EPERM;
}

int coul_merge_drv_power_supply_voltage(int batt_index)
{
	return DEFAULT_POWER_SUPPLY_VOLTAGE;
}

void coul_merge_drv_charger_event_rcv(int batt_index, unsigned int event)
{
	struct coulometer_drv *coul_merge_drv = NULL;
	struct coul_merge_drv_ops *ops = NULL;

	coul_merge_drv = coul_merge_drv_get_info(batt_index);
	if(coul_merge_drv == NULL)
		return;
	ops = coul_merge_drv->g_coul_ops;

	/* execute the operation of coul module */
	if (ops && ops->charger_event_rcv) {
		coul_merge_drv_info("batt_index %d, charger event = 0x%x\n", batt_index, (int)event);
		ops->charger_event_rcv(coul_merge_drv->data, event);
	}
}

int coul_merge_drv_low_temp_opt(int batt_index)
{
	struct coulometer_drv *coul_merge_drv = NULL;
	struct coul_merge_drv_ops *ops = NULL;

	coul_merge_drv = coul_merge_drv_get_info(batt_index);
	if(coul_merge_drv == NULL)
		return 0;
	ops = coul_merge_drv->g_coul_ops;

	if (ops && ops->coul_low_temp_opt)
		return ops->coul_low_temp_opt(coul_merge_drv->data);

	return 0;
}

int coul_merge_drv_battery_fifo_curr(int batt_index, unsigned int index)
{
	struct coulometer_drv *coul_merge_drv = NULL;
	struct coul_merge_drv_ops *ops = NULL;

	coul_merge_drv = coul_merge_drv_get_info(batt_index);
	if(coul_merge_drv == NULL)
		return 0;
	ops = coul_merge_drv->g_coul_ops;

	if (ops && ops->battery_fifo_curr)
		return ops->battery_fifo_curr(coul_merge_drv->data, index);

	return 0;
}

int coul_merge_drv_battery_fifo_vol(int batt_index, unsigned int index)
{
	struct coulometer_drv *coul_merge_drv = NULL;
	struct coul_merge_drv_ops *ops = NULL;

	coul_merge_drv = coul_merge_drv_get_info(batt_index);
	if(coul_merge_drv == NULL)
		return 0;
	ops = coul_merge_drv->g_coul_ops;

	if (ops && ops->battery_fifo_vol)
		return ops->battery_fifo_vol(coul_merge_drv->data, index);

	return 0;
}

int coul_merge_drv_battery_rpcb(int batt_index)
{
	struct coulometer_drv *coul_merge_drv = NULL;
	struct coul_merge_drv_ops *ops = NULL;

	coul_merge_drv = coul_merge_drv_get_info(batt_index);
	if(coul_merge_drv == NULL)
		return 0;
	ops = coul_merge_drv->g_coul_ops;

	if (ops && ops->battery_rpcb)
		return ops->battery_rpcb(coul_merge_drv->data);

	return 0;
}

/*
 * users need to check ocv update conditions
 * can not be called on resume and charge done
 */
void coul_merge_drv_force_ocv(int batt_index)
{
	struct coulometer_drv *coul_merge_drv = NULL;
	struct coul_merge_drv_ops *ops = NULL;

	coul_merge_drv = coul_merge_drv_get_info(batt_index);
	if(coul_merge_drv == NULL)
		return;
	ops = coul_merge_drv->g_coul_ops;

	coul_merge_drv_lock_f();
	if (ops && ops->force_ocv)
		ops->force_ocv(coul_merge_drv->data);

	coul_merge_drv_unlock_f();
}

int coul_merge_drv_battery_fifo_depth(int batt_index)
{
	struct coulometer_drv *coul_merge_drv = NULL;
	struct coul_merge_drv_ops *ops = NULL;

	coul_merge_drv = coul_merge_drv_get_info(batt_index);
	if(coul_merge_drv == NULL)
		return 0;
	ops = coul_merge_drv->g_coul_ops;

	if (ops && ops->battery_fifo_depth)
		return ops->battery_fifo_depth(coul_merge_drv->data);

	return 0;
}

int coul_merge_drv_battery_ufcapacity_tenth(int batt_index)
{
	struct coulometer_drv *coul_merge_drv = NULL;
	struct coul_merge_drv_ops *ops = NULL;

	coul_merge_drv = coul_merge_drv_get_info(batt_index);
	if(coul_merge_drv == NULL)
		return 0;
	ops = coul_merge_drv->g_coul_ops;

	if (ops && ops->battery_ufcapacity_tenth)
		return ops->battery_ufcapacity_tenth(coul_merge_drv->data);

	return 0;
}

int coul_merge_drv_battery_removed_before_boot(int batt_index)
{
	struct coulometer_drv *coul_merge_drv = NULL;
	struct coul_merge_drv_ops *ops = NULL;

	coul_merge_drv = coul_merge_drv_get_info(batt_index);
	if(coul_merge_drv == NULL)
		return 0;
	ops = coul_merge_drv->g_coul_ops;

	if (ops && ops->battery_removed_before_boot)
		return ops->battery_removed_before_boot(coul_merge_drv->data);

	return -1;
}

int coul_merge_drv_coul_ops_register(struct coul_merge_drv_ops *coul_ops,
	void *data, int batt_index, const char *batt_name)
{
	if(batt_index >= BATT_MAX) {
		coul_merge_drv_info("%s error, batt_index %d, batt_name %s\n",
			__func__, batt_index, batt_name);
		return -1;
	}

	g_coul_merge_drv_list[batt_index].g_coul_ops = coul_ops;
	g_coul_merge_drv_list[batt_index].data = data;
	g_coul_merge_drv_list[batt_index].batt_index = batt_index;
	g_coul_merge_drv_list[batt_index].batt_name = batt_name;
	g_coul_merge_drv_list[batt_index].init_flag = 1;

	g_batt_cnt++;
	coul_merge_drv_info("%s batt_index %d, batt_name %s, g_batt_cnt %d\n",
		__func__, batt_index, batt_name, g_batt_cnt);
	return 0;
}

int coul_merge_drv_get_batt_count(void)
{
	return g_batt_cnt;
}

int coul_merge_drv_convert_regval2ua(int batt_index, unsigned int reg_val)
{
	struct coulometer_drv *coul_merge_drv = NULL;
	struct coul_merge_drv_ops *ops = NULL;

	coul_merge_drv = coul_merge_drv_get_info(batt_index);
	if(coul_merge_drv == NULL)
		return 0;
	ops = coul_merge_drv->g_coul_ops;

	if (ops && ops->convert_regval2ua)
		return ops->convert_regval2ua(coul_merge_drv->data, reg_val);

	return -EPERM;
}

int coul_merge_drv_convert_regval2uv(int batt_index, unsigned int reg_val)
{
	struct coulometer_drv *coul_merge_drv = NULL;
	struct coul_merge_drv_ops *ops = NULL;

	coul_merge_drv = coul_merge_drv_get_info(batt_index);
	if(coul_merge_drv == NULL)
		return 0;
	ops = coul_merge_drv->g_coul_ops;

	if (ops && ops->convert_regval2uv)
		return ops->convert_regval2uv(coul_merge_drv->data, reg_val);

	return 0;
}

int coul_merge_drv_convert_regval2temp(int batt_index, unsigned int reg_val)
{
	struct coulometer_drv *coul_merge_drv = NULL;
	struct coul_merge_drv_ops *ops = NULL;

	coul_merge_drv = coul_merge_drv_get_info(batt_index);
	if(coul_merge_drv == NULL)
		return 0;
	ops = coul_merge_drv->g_coul_ops;

	if (ops && ops->convert_regval2temp)
		return ops->convert_regval2temp(coul_merge_drv->data, reg_val);

	return -EPERM;
}

int coul_merge_drv_convert_mv2regval(int batt_index, int vol_mv)
{
	struct coulometer_drv *coul_merge_drv = NULL;
	struct coul_merge_drv_ops *ops = NULL;

	coul_merge_drv = coul_merge_drv_get_info(batt_index);
	if(coul_merge_drv == NULL)
		return 0;
	ops = coul_merge_drv->g_coul_ops;

	if (ops && ops->convert_mv2regval)
		return ops->convert_mv2regval(coul_merge_drv->data, vol_mv);

	return -EPERM;
}

int coul_merge_drv_cal_uah_by_ocv(int batt_index, int ocv_uv, int *ocv_soc_uah)
{
	struct coulometer_drv *coul_merge_drv = NULL;
	struct coul_merge_drv_ops *ops = NULL;

	coul_merge_drv = coul_merge_drv_get_info(batt_index);
	if(coul_merge_drv == NULL)
		return 0;
	ops = coul_merge_drv->g_coul_ops;

	if (ops && ops->cal_uah_by_ocv)
		return ops->cal_uah_by_ocv(coul_merge_drv->data, ocv_uv, ocv_soc_uah);

	return -EPERM;
}

int coul_merge_drv_get_ocv_by_soc(int batt_index, int temp, int soc)
{
	struct coulometer_drv *coul_merge_drv = NULL;
	struct coul_merge_drv_ops *ops = NULL;

	coul_merge_drv = coul_merge_drv_get_info(batt_index);
	if(coul_merge_drv == NULL)
		return 0;
	ops = coul_merge_drv->g_coul_ops;

	if (ops && ops->get_ocv_by_soc)
		return ops->get_ocv_by_soc(coul_merge_drv->data, temp, soc);

	return -EPERM;
}

int coul_merge_drv_get_soc_by_ocv(int batt_index, int temp, int ocv)
{
	struct coulometer_drv *coul_merge_drv = NULL;
	struct coul_merge_drv_ops *ops = NULL;

	coul_merge_drv = coul_merge_drv_get_info(batt_index);
	if(coul_merge_drv == NULL)
		return 0;
	ops = coul_merge_drv->g_coul_ops;

	if (ops && ops->get_soc_by_ocv)
		return ops->get_soc_by_ocv(coul_merge_drv->data, temp, ocv);

	return -EPERM;
}

int coul_merge_drv_get_ocv(int batt_index)
{
	struct coulometer_drv *coul_merge_drv = NULL;
	struct coul_merge_drv_ops *ops = NULL;

	coul_merge_drv = coul_merge_drv_get_info(batt_index);
	if(coul_merge_drv == NULL)
		return 0;
	ops = coul_merge_drv->g_coul_ops;

	if (ops && ops->get_ocv)
		return ops->get_ocv(coul_merge_drv->data);

	return 0;
}

u8 coul_merge_drv_get_ocv_level(int batt_index)
{
	struct coulometer_drv *coul_merge_drv = NULL;
	struct coul_merge_drv_ops *ops = NULL;

	coul_merge_drv = coul_merge_drv_get_info(batt_index);
	if(coul_merge_drv == NULL)
		return 0;
	ops = coul_merge_drv->g_coul_ops;

	if (ops && ops->get_ocv_level)
		return ops->get_ocv_level(coul_merge_drv->data);

	return 0;
}

int coul_merge_drv_ocv_valid_refresh_fcc(int batt_index)
{
	struct coulometer_drv *coul_merge_drv = NULL;
	struct coul_merge_drv_ops *ops = NULL;

	coul_merge_drv = coul_merge_drv_get_info(batt_index);
	if(coul_merge_drv == NULL)
		return 0;
	ops = coul_merge_drv->g_coul_ops;

	if (ops && ops->ocv_valid_refresh_fcc)
		return ops->ocv_valid_refresh_fcc(coul_merge_drv->data);

	return 0;
}

int coul_merge_drv_battery_uuc(int batt_index)
{
	struct coulometer_drv *coul_merge_drv = NULL;
	struct coul_merge_drv_ops *ops = NULL;

	coul_merge_drv = coul_merge_drv_get_info(batt_index);
	if(coul_merge_drv == NULL)
		return 0;
	ops = coul_merge_drv->g_coul_ops;

	if (ops && ops->battery_uuc)
		return ops->battery_uuc(coul_merge_drv->data);

	return 0;
}

int coul_merge_drv_battery_delta_rc(int batt_index)
{
	struct coulometer_drv *coul_merge_drv = NULL;
	struct coul_merge_drv_ops *ops = NULL;

	coul_merge_drv = coul_merge_drv_get_info(batt_index);
	if(coul_merge_drv == NULL)
		return 0;
	ops = coul_merge_drv->g_coul_ops;

	if (ops && ops->battery_delta_rc)
		return ops->battery_delta_rc(coul_merge_drv->data);

	return 0;
}

int coul_merge_drv_coul_chip_temperature(int batt_index)
{
	struct coulometer_drv *coul_merge_drv = NULL;
	struct coul_merge_drv_ops *ops = NULL;

	coul_merge_drv = coul_merge_drv_get_info(batt_index);
	if(coul_merge_drv == NULL)
		return 0;
	ops = coul_merge_drv->g_coul_ops;

	if (ops && ops->chip_temperature)
		return ops->chip_temperature(coul_merge_drv->data);

	return INVALID_TEMP;
}

int coul_merge_drv_coul_interpolate_pc(int batt_index, int ocv)
{
	struct coulometer_drv *coul_merge_drv = NULL;
	struct coul_merge_drv_ops *ops = NULL;

	coul_merge_drv = coul_merge_drv_get_info(batt_index);
	if(coul_merge_drv == NULL)
		return 0;
	ops = coul_merge_drv->g_coul_ops;

	if (ops && ops->coul_coul_interpolate_pc)
		return ops->coul_coul_interpolate_pc(coul_merge_drv->data, ocv);

	return 0;
}

int coul_merge_drv_get_desired_charging_current(int batt_index)
{
	struct coulometer_drv *coul_merge_drv = NULL;
	struct coul_merge_drv_ops *ops = NULL;

	coul_merge_drv = coul_merge_drv_get_info(batt_index);
	if(coul_merge_drv == NULL)
		return 0;
	ops = coul_merge_drv->g_coul_ops;

	if (ops && ops->get_desired_charging_current)
		return ops->get_desired_charging_current(coul_merge_drv->data);

	return 0;
}

int coul_merge_drv_get_desired_charging_voltage(int batt_index)
{
	struct coulometer_drv *coul_merge_drv = NULL;
	struct coul_merge_drv_ops *ops = NULL;

	coul_merge_drv = coul_merge_drv_get_info(batt_index);
	if(coul_merge_drv == NULL)
		return 0;
	ops = coul_merge_drv->g_coul_ops;

	if (ops && ops->get_desired_charging_voltage)
		return ops->get_desired_charging_voltage(coul_merge_drv->data);

	return 0;
}

int coul_merge_drv_is_smart_battery(int batt_index)
{
	struct coulometer_drv *coul_merge_drv = NULL;
	struct coul_merge_drv_ops *ops = NULL;

	coul_merge_drv = coul_merge_drv_get_info(batt_index);
	if(coul_merge_drv == NULL)
		return 0;
	ops = coul_merge_drv->g_coul_ops;

	if (ops && ops->is_smart_battery)
		return ops->is_smart_battery(coul_merge_drv->data);

	return 0;
}

int coul_merge_drv_is_battery_full_charged(int batt_index)
{
	struct coulometer_drv *coul_merge_drv = NULL;
	struct coul_merge_drv_ops *ops = NULL;

	coul_merge_drv = coul_merge_drv_get_info(batt_index);
	if(coul_merge_drv == NULL)
		return 0;
	ops = coul_merge_drv->g_coul_ops;

	if (ops && ops->is_battery_full_charged)
		return ops->is_battery_full_charged(coul_merge_drv->data);

	return 0;
}

int coul_merge_drv_suspend(int batt_index)
{
	struct coulometer_drv *coul_merge_drv = NULL;
	struct coul_merge_drv_ops *ops = NULL;

	coul_merge_drv = coul_merge_drv_get_info(batt_index);
	if(coul_merge_drv == NULL)
		return 0;
	ops = coul_merge_drv->g_coul_ops;

	if (ops && ops->coul_suspend)
		return ops->coul_suspend(coul_merge_drv->data);

	return 0;
}

int coul_merge_drv_resume(int batt_index)
{
	struct coulometer_drv *coul_merge_drv = NULL;
	struct coul_merge_drv_ops *ops = NULL;

	coul_merge_drv = coul_merge_drv_get_info(batt_index);
	if(coul_merge_drv == NULL)
		return 0;
	ops = coul_merge_drv->g_coul_ops;

	if (ops && ops->coul_resume)
		return ops->coul_resume(coul_merge_drv->data);

	return 0;
}

void coul_merge_drv_shutdown(int batt_index)
{
	struct coulometer_drv *coul_merge_drv = NULL;
	struct coul_merge_drv_ops *ops = NULL;

	coul_merge_drv = coul_merge_drv_get_info(batt_index);
	if(coul_merge_drv == NULL)
		return;
	ops = coul_merge_drv->g_coul_ops;

	if (ops && ops->coul_shutdown)
		return ops->coul_shutdown(coul_merge_drv->data);

	return;
}

int coul_merge_drv_get_last_soc(void)
{
	struct coulometer_drv *coul_merge_drv = NULL;
	struct coul_merge_drv_ops *ops = NULL;

	coul_merge_drv = coul_merge_drv_get_info(BATT_0);
	if(coul_merge_drv == NULL)
		return 0;
	ops = coul_merge_drv->g_coul_ops;

	if (ops && ops->get_last_soc)
		return ops->get_last_soc(coul_merge_drv->data);

	return -1;
}

void coul_merge_drv_save_last_soc(int last_soc)
{
	struct coulometer_drv *coul_merge_drv = NULL;
	struct coul_merge_drv_ops *ops = NULL;

	coul_merge_drv = coul_merge_drv_get_info(BATT_0);
	if(coul_merge_drv == NULL)
		return;
	ops = coul_merge_drv->g_coul_ops;

	if (ops && ops->set_last_soc)
		ops->set_last_soc(coul_merge_drv->data, last_soc);
}

int coul_merge_drv_set_cali(struct coul_batt_cali_info *info)
{
	struct coulometer_drv *coul_merge_drv = NULL;
	struct coul_merge_drv_ops *ops = NULL;
	int batt_index = info->batt_index;

	coul_merge_drv = coul_merge_drv_get_info(batt_index);
	if(coul_merge_drv == NULL)
		return 0;
	ops = coul_merge_drv->g_coul_ops;

	if (ops && ops->coul_set_cali)
		return ops->coul_set_cali(coul_merge_drv->data, info);

	return 0;
}

int coul_merge_drv_get_cali(struct coul_batt_cali_info *info)
{
	struct coulometer_drv *coul_merge_drv = NULL;
	struct coul_merge_drv_ops *ops = NULL;
	int batt_index = info->batt_index;

	coul_merge_drv = coul_merge_drv_get_info(batt_index);
	if(coul_merge_drv == NULL)
		return -1;
	ops = coul_merge_drv->g_coul_ops;

	if (ops && ops->coul_get_cali)
		return ops->coul_get_cali(coul_merge_drv->data, info);

	return -1;
}

int coul_merge_drv_set_batt_mode(int batt, int mode)
{
	struct coulometer_drv *coul_merge_drv = NULL;
	struct coul_merge_drv_ops *ops = NULL;

	coul_merge_drv = coul_merge_drv_get_info(batt);
	if(coul_merge_drv == NULL)
		return -1;
	ops = coul_merge_drv->g_coul_ops;

	if (ops && ops->set_batt_mode)
		return ops->set_batt_mode(coul_merge_drv->data, mode);

	return -1;
}
int coul_merge_drv_get_adc_cali_state(void)
{
	struct coulometer_drv *coul_merge_drv = NULL;
	struct coul_merge_drv_ops *ops = NULL;

	coul_merge_drv = coul_merge_drv_get_info(0);
	if(coul_merge_drv == NULL)
		return -1;

	ops = coul_merge_drv->g_coul_ops;
	if (ops && ops->get_cali_adc_disable)
		return ops->get_cali_adc_disable(coul_merge_drv->data);

	return -1;
}

int coul_merge_drv_set_adc_cali_disable(int batt, int state)
{
	struct coulometer_drv *coul_merge_drv = NULL;
	struct coul_merge_drv_ops *ops = NULL;

	coul_merge_drv = coul_merge_drv_get_info(batt);
	if(coul_merge_drv == NULL)
		return -1;
	ops = coul_merge_drv->g_coul_ops;

	if (ops && ops->set_cali_adc_disable)
		return ops->set_cali_adc_disable(coul_merge_drv->data, state);

	return -1;
}

int __init coul_merge_drv_init(void)
{
#ifdef CONFIG_HUAWEI_HW_DEV_DCT
	struct coulometer_drv *coul_merge_drv = NULL;
	int i;
#endif

	mutex_init(&g_coul_merge_drv_lock);

#ifdef CONFIG_HUAWEI_HW_DEV_DCT
	/* detect coul device successful, set the flag as present */

		for (i = 0; i < BATT_MAX; i++) {
			coul_merge_drv = coul_merge_drv_get_info(i);
			if (coul_merge_drv && coul_merge_drv->g_coul_ops && coul_merge_drv->g_coul_ops->dev_check) {
				if (coul_merge_drv->g_coul_ops->dev_check(coul_merge_drv->data) == COUL_IC_GOOD) {
					coul_merge_drv_info("coul ic is good\n");
					set_hw_dev_flag(DEV_I2C_COUL);
				} else {
					coul_merge_drv_err("coul ic is bad\n");
				}
			} else {
				coul_merge_drv_err("ops dev_check is null\n");
			}
		}


#endif

	g_coul_merge_drv_init = 1;
	coul_merge_drv_info("%s succ\n", __func__);
	return 0;
}

/* call after coul_merge_drv_coul_ops_register */
fs_initcall_sync(coul_merge_drv_init);

void __exit coul_merge_drv_exit(void)
{
	mutex_destroy(&g_coul_merge_drv_lock);
}

module_exit(coul_merge_drv_exit);

MODULE_DESCRIPTION("coul module driver");
MODULE_LICENSE("GPL");
