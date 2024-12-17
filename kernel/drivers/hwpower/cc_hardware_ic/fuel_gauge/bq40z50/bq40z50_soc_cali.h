// SPDX-License-Identifier: GPL-2.0
/*
 * bq40z50_jump_cali.h
 *
 * bq40z50 soc calibration
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

#ifndef _BQ40Z50_JUMP_CALI_H_
#define _BQ40Z50_JUMP_CALI_H_

#include <linux/atomic.h>
#include <chipset_common/hwpower/common_module/power_time.h>

#define BQ40Z50_JUMP_CAL_UNLOCK            1
#define BQ40Z50_DMD_REPORT_ID              930001021
#define BQ40Z50_PERCENT                    100
#define BQ40Z50_DOUBLE                     2
#define BQ40Z50_BATTERY_COUNT              2
#define BQ40Z50_ONE_SECOND                 1
#define BQ40Z50_SOC_FULL                   100
#define BQ40Z50_SOC_EMPTY                  0
#define BQ40Z50_SOC_95                     95
#define BQ40Z50_SOC_CHANGE_MAX             1
#define BQ40Z50_CALI_INTERVAL              10
#define BQ40Z50_CALI_VOLT_LIMIT            3650
#define BQ40Z50_BATT_VOLT_MIN              2800
#define BQ40Z50_CALI_JUDGE_MAX_NUM         3
#define BQ40Z50_START_CAL_SOC_EST          1
#define BQ40Z50_END_CAL_SOC_EST            0
#define BQ40Z50_START_DMD_REPORT           1
#define BQ40Z50_END_DMD_REPORT             0
#define BQ40Z50_BAT_TBL_LEN_MAX            6 /* temp + rate + 4 sample point */
#define BQ40Z50_BAT_TBL_MAX_NUS            6
#define BQ40Z50_BAT_TBL_MIN_TEMP           10
#define BQ40Z50_BAT_TBL_MAX_TEMP           45
#define BQ40Z50_SOC_EST_AVG_CNT            10
#define BQ40Z50_CALI_STAGE_MAX             2
#define BQ40Z50_SMOOTH_SOC_0_DEFAULT       500
#define BQ40Z50_SMOOTH_SOC_1_DEFAULT       1000
#define BQ40Z50_SMOOTH_TIMES_0_DEFAULT     15
#define BQ40Z50_SMOOTH_TIMES_1_DEFAULT     25
#define BQ40Z50_START_DECREASE_SOC_DEFAULT 10
#define BQ40Z50_START_INCREASE_SOC_DEFAULT 35
#define BQ40Z50_TBL_VOLT_OFFSET            2
#define BQ40Z50_SEC_PER_HOUR               3600
#define bq40z50_check_func_ops(ops, name) \
	do { if (((ops) == NULL) || ((ops)->name == NULL)) return (-EPERM);} while (0)

struct volt_soc_tbl {
	int temp;
	int rate;
	int soc15_volt;
	int soc10_volt;
	int soc5_volt;
	int soc0_volt;
};

struct volt_soc_data {
	struct volt_soc_tbl tbl0; /* 10 celsius, 0.2C */
	struct volt_soc_tbl tbl1; /* 25 celsius, 0.2C */
	struct volt_soc_tbl tbl2; /* 25 celsius, 0.5C */
	struct volt_soc_tbl tbl3; /* Reserved */
	struct volt_soc_tbl tbl4; /* Reserved */
	struct volt_soc_tbl tbl5; /* Reserved */
};

/* stage depend on delta_soc */
struct smooth_stage {
	int soc0;
	int soc1;
};

/* smooth complete times */
struct smooth_speed {
	int stage0_times;
	int stage1_times;
};

struct volt_soc_loop_fifo {
	int fifo[BQ40Z50_SOC_EST_AVG_CNT];
	int start;
	int len;
};

struct bq40z50_jump_cali_ops {
	void *dev_data;
	int (*get_volt1)(void *dev_data);
	int (*get_volt2)(void *dev_data);
	int (*get_temp1)(void *dev_data);
	int (*get_temp2)(void *dev_data);
	int (*get_curr)(void *dev_data);
	int (*get_avg_curr)(void *dev_data);
	int (*get_soc)(void *dev_data);
	int (*get_rm)(void *dev_data);
	int (*get_cycle)(void *dev_data);
	int (*get_fcc)(void *dev_data);
	int (*get_dc)(void *dev_data); /* fcc design */
	int (*get_last_soc)(void *dev_data);
	void (*set_last_soc)(void *dev_data, int capacity);
};

struct bq40z50_cali_para {
	struct volt_soc_data volt_soc_tbl;
	struct smooth_stage smooth_soc;
	struct smooth_speed smooth_times;
	int last_soc_enable;
	int startup_decrease_soc;
	int startup_increase_soc;
	int volt_cali_limit;
};

struct bq40z50_soc_cali_info {
	atomic_t jump_cali_lock;
	int volt1; /* mV */
	int volt2;
	int temp1; /* degree */
	int temp2;
	int rm; /* mAh */
	int rm_prev;
	int cycle;
	int fcc;
	int fcc_design;
	int curr; /* mA */
	int avg_curr;
	int soc_coul; /* unit 0.01% */
	int soc_est;
	int soc_cali;
	int soc_cali_prev;
	int cnt;
	int discharge_cali_flag;
	int report_flag;
	int discharge_smooth_flag;
	int smooth_soc0;
	int smooth_soc1;
	int last_soc_enable;
	int startup_decrease_soc;
	int startup_increase_soc;
	int volt_cali_limit;
	time64_t last_cali_time;
	struct volt_soc_data bat_volt_soc;
	struct volt_soc_loop_fifo soc_est_fifo;
	struct bq40z50_cali_para *dts_info;
	struct bq40z50_jump_cali_ops *ops;
};

/*
 * return the estimated soc which calculated by battery volt, curr and temp
 */
int bq40z50_get_soc_est(void);

/*
 * soc calibration entrypoint function, which can calibrate the capacity when the gauge value high
 */
int bq40z50_soc_jump_cali(unsigned short soc_coul);
int bq40z50_soc_cali_init(struct bq40z50_jump_cali_ops *ops, struct bq40z50_cali_para *info);

#endif /* _BQ40Z50_JUMP_CALI_H_ */