/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2013-2020. All rights reserved.
 * Description: this file provide interface to get the battery state such as
 *          capacity, voltage, current, temperature
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

#ifndef _COUL_MERGE_DRV_H
#define _COUL_MERGE_DRV_H

#include <linux/kernel.h>
#include <linux/suspend.h>
#include <linux/kfifo.h>

#define BATT_0                                 0
#define BATT_1                                 1
#define BATT_2                                 2
#define BATT_MAX                               3

#define BATT_NAME_SIZE_MAX  20

#define CAPACITY_MIN                    0
#define CAPACITY_CRITICAL               5
#define CAPACITY_LOW                    15
#define CAPACITY_HIGH                   95
#define CAPACITY_FULL                   100

#define SOC_VARY_LOW                    10
#define SOC_VARY_HIGH                   90

#define LOW_SOC         30   // 3%
#define INVALID_TEMP                            (-99)

#define CHARGING_STATE_CHARGE_START             1
#define CHARGING_STATE_CHARGE_DONE              2
#define CHARGING_STATE_CHARGE_STOP              3
#define CHARGING_STATE_CHARGE_RECHARGE          4
#define CHARGING_STATE_CHARGE_NOT_CHARGE        5
#define CHARGING_STATE_CHARGE_UNKNOW            0

#define HALF    2
#define QUARTER 4
#define TENTH                      10

#ifndef TRUE
#define TRUE                          1
#endif
#ifndef FALSE
#define FALSE                         0
#endif
#define DEFAULT_SOC_MONITOR_LIMIT       100


#define BAT_VOL_3500    3500
#define BAT_VOL_3600    3600
#define BAT_VOL_3700    3700
#define BAT_VOL_3800    3800
#define BAT_VOL_3900    3900

#define BASP_TAG "[BASP] " /* Battery Aging Safe Policy LOG TAG */

#define VTERM_MAX_DEFAULT_MV            4400
#define MAX_BATT_CHARGE_CUR_RATIO       70 /* 0.7C */
#define COUL_IC_GOOD    0
#define COUL_IC_BAD     1

#define COMPENSATION_PARA_LEVEL         20
#define COMPENSATION_THRESHOLD          200 /* 20c */

#define INVALID_TEMP_VAL                (-999)
#define INVALID_VOLTAGE_VAL             (-999)
#define BATTERY_NORMAL_CUTOFF_VOL       3150
#define DEFAULT_RPCB            10000 /* uohm */
#define POLAR_CALC_INTERVAL     200 /* ms */
#define TEMP_POLAR_REPORT       5
#define CURR_AVG_DEFAULT        19999 /* ma */
/* ma = ua/1000 = uas/s/1000 = uah*3600/s/1000 = uah*18/(s*5) */
#define cc_uas2ma(cc, time) (((cc) * 18) / ((time) * 5))

#define PERMILLAGE 1000
#define DEFAULT_POWER_SUPPLY_VOLTAGE 4200

struct coul_batt_cali_info{
	int batt_index;
	int charge_mode;
	int data_type;
	int a;
	int b;
};

enum {
	PARALLEL_MODE = 0,
	SERIES_MODE,
	BATT_MODE_END
};

enum {
	BATTERY_COUNT_ZERO = 0,
	BATTERY_COUNT_ONE,
	BATTERY_COUNT_TWO,
	BATTERY_COUNT_THREE,
	BATTERY_COUNT_END
};

enum {
	BATT_DATA_VOLT_CALI = 0,
	BATT_DATA_CUR_CALI,
	BATT_DATA_VOLT,
	BATT_DATA_CUR,
	BATT_DATA_TYPE_END
};

#define MAX_COUL_EVENT_COUNT 16

struct coul_merge_device {
	struct device *dev;
	struct delayed_work calculate_soc_delayed_work;
	struct delayed_work coul_rcv_event_work;
	struct work_struct merge_resume_work;
	DECLARE_KFIFO(event_fifo, unsigned int,
		      MAX_COUL_EVENT_COUNT);
	spinlock_t event_lock;
	struct notifier_block pm_notify;
	struct notifier_block reboot_nb;
	struct mutex lock;
	struct wakeup_source *resume_wake_lock;
	unsigned int soc_work_interval;
	int soc;
	int cap_ratio_0;
	int cap_ratio_1;
	int cap_ratio_2;
	int last_soc_enable;
	int startup_delta_soc;
	int is_series_bat;
	struct device *sysfs_device;
};

struct coul_merge_drv_ops {
	int (*is_coul_ready)(void *data);
	int (*is_battery_exist)(void *data);
	int (*is_battery_reach_threshold)(void *data);
	char *(*battery_brand)(void *data);
	int (*battery_id_voltage)(void *data);
	int (*battery_voltage)(void *data);
	int (*battery_voltage_uv)(void *data);
	int (*battery_current)(void *data);
	int (*battery_resistance)(void *data);
	int (*fifo_avg_current)(void *data);
	int (*battery_current_avg)(void *data);
	int (*battery_unfiltered_capacity)(void *data);
	int (*battery_capacity)(void *data);
	int (*battery_temperature)(void *data);
	int (*chip_temperature)(void *data);
	int (*battery_temperature_for_charger)(void *data);
	int (*battery_raw_temperature)(void *data);
	int (*battery_rm)(void *data);
	int (*battery_fcc)(void *data);
	int (*battery_fcc_design)(void *data);
	int (*battery_uuc)(void *data);
	int (*battery_delta_rc)(void *data);
	int (*battery_tte)(void *data);
	int (*battery_ttf)(void *data);
	int (*battery_health)(void *data);
	int (*battery_technology)(void *data);
	struct chrg_para_lut *(*battery_charge_params)(void *data);
	int (*battery_vbat_max)(void *data);
	int (*battery_ifull)(void *data);
	int (*charger_event_rcv)(void *data, unsigned int);
	int (*battery_cycle_count)(void *data);
	int (*get_battery_limit_fcc)(void *data);
	int (*coul_is_fcc_debounce)(void *data);
	int (*set_hltherm_flag)(void *data, int flag);
	int (*get_hltherm_flag)(void *data);
	int (*dev_check)(void *data);
	int (*get_soc_vary_flag)(void *data, int monitor_flag, int *deta_soc);
	int (*coul_low_temp_opt)(void *data);
	int (*battery_cc)(void *data);
	int (*battery_cc_cache)(void *data);
	int (*battery_fifo_curr)(void *data, unsigned int);
	int (*battery_fifo_vol)(void *data, unsigned int);
	int (*battery_rpcb)(void *data);
	int (*battery_fifo_depth)(void *data);
	int (*battery_ufcapacity_tenth)(void *data);
	int (*convert_regval2ua)(void *data, unsigned int reg_val);
	int (*convert_regval2uv)(void *data, unsigned int reg_val);
	int (*convert_regval2temp)(void *data, unsigned int reg_val);
	int (*convert_mv2regval)(void *data, int vol_mv);
	int (*cal_uah_by_ocv)(void *data, int ocv_uv, int *ocv_soc_uAh);
	int (*battery_removed_before_boot)(void *data);
	int (*get_qmax)(void *data);
	int (*get_ocv_by_soc)(void *data, int temp, int soc);
	int (*get_soc_by_ocv)(void *data, int temp, int ocv);
	int (*get_ocv)(void *data);
	u8 (*get_ocv_level)(void *data);
	int (*ocv_valid_refresh_fcc)(void *data);
	int (*get_polar_avg)(void *data);
	int (*get_polar_peak)(void *data);
	int (*update_basp_policy)(void *data, unsigned int level,
		unsigned int nondc_volt_dec);
	int (*get_record_fcc)(void *data, unsigned int size, unsigned int *records);
	void (*force_ocv)(void *data);
	int (*coul_coul_interpolate_pc)(void *data, int ocv);
	void (*coul_calc_work)(void *data);
	int (*get_charge_state)(void *data);
	int (*get_desired_charging_current)(void *data);
	int (*get_desired_charging_voltage)(void *data);
	int (*is_smart_battery)(void *data);
	int (*is_battery_full_charged)(void *data);
	int (*get_last_soc)(void *data);
	void (*set_last_soc)(void *data, int last_soc);
	int (*coul_suspend)(void *data);
	int (*coul_resume)(void *data);
	void (*coul_shutdown)(void *data);
	int (*coul_set_cali)(void *data, struct coul_batt_cali_info *info);
	int (*coul_get_cali)(void *data, struct coul_batt_cali_info *info);
	int (*set_batt_mode)(void *data, int mode);
	int (*set_cali_adc_disable)(void *data, int state);
	int (*get_cali_adc_disable)(void *data);
};

int coul_merge_drv_ocv_valid_refresh_fcc(int batt);
void coul_merge_drv_charger_event_rcv(int batt, unsigned int event);
int coul_merge_drv_is_coul_ready(int batt);
int coul_merge_drv_is_battery_exist(int batt);
int coul_merge_drv_is_battery_reach_threshold(int batt);
int coul_merge_drv_battery_voltage_mv(int batt);
char *coul_merge_drv_battery_brand(int batt);
int coul_merge_drv_battery_id_voltage(int batt);
int coul_merge_drv_battery_voltage_uv(int batt);
int coul_merge_drv_battery_current(int batt);
int coul_merge_drv_battery_resistance(int batt);
int coul_merge_drv_fifo_avg_current(int batt);
int coul_merge_drv_battery_current_avg(int batt);
int coul_merge_drv_battery_unfiltered_capacity(int batt);
int coul_merge_drv_battery_capacity(int batt);
int coul_merge_drv_battery_temperature_raw(unsigned int batt_index);
int coul_merge_drv_battery_temperature(int batt);
int coul_merge_drv_battery_rm(int batt);
int coul_merge_drv_battery_fcc(int batt);
int coul_merge_drv_battery_fcc_design(int batt);
int coul_merge_drv_battery_uuc(int batt);
int coul_merge_drv_battery_delta_rc(int batt);

int coul_merge_drv_battery_tte(int batt);
int coul_merge_drv_battery_ttf(int batt);
int coul_merge_drv_battery_health(int batt);
int coul_merge_drv_battery_capacity_level(int batt);
int coul_merge_drv_battery_technology(int batt);
struct chrg_para_lut *coul_merge_drv_battery_charge_params(int batt);
int coul_merge_drv_battery_ifull(int batt);
int coul_merge_drv_battery_vbat_max(int batt);
int coul_merge_drv_battery_cycle_count(int batt);
int coul_merge_drv_battery_get_limit_fcc(int batt);

#ifdef CONFIG_COUL_ALGORITHM_CORE
int coul_merge_drv_coul_ops_register(struct coul_merge_drv_ops *coul_ops,
	void *data, int batt, const char *batt_name);
#else
static inline int coul_merge_drv_coul_ops_register(struct coul_merge_drv_ops *coul_ops,
	void *data, int batt, const char *batt_name)
{
	return 0;
}
#endif

int coul_merge_drv_power_supply_voltage(int batt);
int coul_merge_drv_is_fcc_debounce(int batt);
int coul_merge_drv_set_hltherm_flag(int batt, int temp_flag);
int coul_merge_drv_get_hltherm_flag(int batt);
int coul_merge_drv_battery_soc_vary_flag(int batt, int monitor_flag, int *deta_soc);
int coul_merge_drv_battery_temperature_for_charger(int batt);
int coul_merge_drv_low_temp_opt(int batt);
int coul_merge_drv_battery_cc(int batt);
int coul_merge_drv_battery_cc_cache(int batt);
int coul_merge_drv_battery_fifo_depth(int batt);
int coul_merge_drv_battery_ufcapacity_tenth(int batt);
int coul_merge_drv_battery_fifo_curr(int batt, unsigned int index);
int coul_merge_drv_convert_regval2ua(int batt, unsigned int reg_val);
int coul_merge_drv_convert_regval2uv(int batt, unsigned int reg_val);
int coul_merge_drv_convert_regval2temp(int batt, unsigned int reg_val);
int coul_merge_drv_convert_mv2regval(int batt, int vol_mv);
int coul_merge_drv_cal_uah_by_ocv(int batt, int ocv_uv, int *ocv_soc_uah);
int coul_merge_drv_coul_chip_temperature(int batt);
int coul_merge_drv_battery_cc_uah(int batt);
int coul_merge_drv_battery_removed_before_boot(int batt);
int coul_merge_drv_battery_get_qmax(int batt);
int coul_merge_drv_get_ocv_by_soc(int batt, int temp, int soc);
int coul_merge_drv_get_soc_by_ocv(int batt, int temp, int ocv);
int coul_merge_drv_get_ocv(int batt);
u8 coul_merge_drv_get_ocv_level(int batt);

int coul_merge_drv_battery_update_basp_policy(int batt, unsigned int level,
	unsigned int nondc_volt_dec);
int coul_merge_drv_battery_get_recrod_fcc(int batt, unsigned int size, unsigned int *records);
int coul_merge_drv_battery_fifo_vol(int batt, unsigned int index);
int coul_merge_drv_battery_rpcb(int batt);
void coul_merge_drv_force_ocv(int bat);
int coul_merge_drv_coul_interpolate_pc(int batt, int ocv);
int coul_merge_drv_get_desired_charging_current(int batt);
int coul_merge_drv_get_desired_charging_voltage(int batt);
int coul_merge_drv_is_smart_battery(int batt);
int coul_merge_drv_is_battery_full_charged(int batt);
int coul_merge_drv_get_charge_state(int batt);
void coul_merge_drv_calc(int batt);
int coul_merge_drv_get_last_soc(void);
void coul_merge_drv_save_last_soc(int last_soc);
int coul_merge_create_sysfs(struct coul_merge_device *di);

int coul_merge_drv_suspend(int batt);
int coul_merge_drv_resume(int batt);
void coul_merge_drv_shutdown(int batt_index);

int coul_merge_drv_set_cali(struct coul_batt_cali_info *info);
int coul_merge_drv_get_cali(struct coul_batt_cali_info *info);
int coul_merge_drv_get_batt_name(int batt_index, char *name, int size_max);
int coul_merge_drv_get_batt_index(const char *name_ptr, int *batt_index);
int coul_merge_drv_set_batt_mode(int batt, int mode);
int coul_merge_drv_get_batt_count(void);
int coul_merge_drv_get_adc_cali_state(void);
int coul_merge_drv_set_adc_cali_disable(int batt, int state);

#endif /* _COUL_DRV_H */
