/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: ocv operation functions for coul module
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

#ifndef _COUL_ALGO_OCV_OPS_H_
#define _COUL_ALGO_OCV_OPS_H_

#include <platform_include/basicplatform/linux/power/platform/coul/coul_core.h>

#define DELTA_SLEEP_TIME_OFFSET 30 /* sleep time offset, in s */
#define DELTA_SLEEP_TIME        (10 * 60) /* sleep time bigger could update ocv, in s */
#define DELTA_SLEEP_CURRENT     50 /* sleep current less could updat eocv, in mA */
#define VOL_FIFO_MAX            10
#define VOL_MAX_DIFF_UV         5000
#define CURR2UPDATE_OCV_TIME    (10 * 60)
#define SR_MAX_RESUME_TIME      90         /* 90 s */
#define SR_DELTA_SLEEP_TIME     (4 * 60)   /* 4 min */
#define SR_DELTA_WAKEUP_TIME    30         /* 30 s */
#define SR_TOTAL_TIME           (30 * 60)  /* 30 min */
#define SR_DUTY_RATIO           95
#define DEFAULT_BATTERY_OHMIC_RESISTANCE        100 /* mohm */
#define FLAG_USE_CLAC_OCV       0xABCD
#define CAPACITY_INVALID_AREA_4500      4500000
#define CAPACITY_INVALID_AREA_2500      2500000
#define PC_UPPER_LIMIT          1000
#define MIN_VALID_VOL_DATA_NUM  3
#define WAKEUP_AVG_CUR_500MA    500
#define WAKEUP_AVG_CUR_250MA    250
#define SEC_PER_HOUR_DIV_200    18
#define PERMILLAGE_DIV_200      5

#define OCV_UPDATE_DELTA_TIME   30

#define SEC_PER_MIN             60
#define SEC_PER_HOUR            3600
#define CALI_RBATT_CC_MAX       20 /* mAh */
#define CALI_RBATT_CURR_MIN     500 /* mA */

void coul_judge_eco_leak_uah(struct smartstar_coul_device *di, int soc);
void coul_set_ocv_cali_level(struct smartstar_coul_device *di, unsigned char last_ocv_level);
void coul_update_ocv_cali_rbatt(struct smartstar_coul_device *di, int avg_c,
	int rbatt_calc);
void coul_record_ocv_cali_info(struct smartstar_coul_device *di);
int coul_core_dsm_report_ocv_cali_info(struct smartstar_coul_device *di,
	int err_num, const char *buff);
void coul_core_clear_cc_register(struct smartstar_coul_device *di);
void coul_core_clear_coul_time(struct smartstar_coul_device *di);
void coul_core_get_initial_ocv(struct smartstar_coul_device *di);
void coul_get_ocv_by_fcc(struct smartstar_coul_device *di);
int coul_get_ocv_vol_from_fifo(struct smartstar_coul_device *di);

#ifdef CONFIG_PM
void coul_record_wakeup_info(struct smartstar_coul_device *di, int wakeup_time);
void coul_record_sleep_info(struct smartstar_coul_device *di, int sr_sleep_time);
void coul_update_ocv_after_resume(struct smartstar_coul_device *di);
#endif
#endif

