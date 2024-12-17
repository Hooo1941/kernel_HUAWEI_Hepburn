/* SPDX-License-Identifier: GPL-2.0 */
/*
 * bq40z50.h
 *
 * bq40z50 driver
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
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

#ifndef _BQ40Z50_H_
#define _BQ40Z50_H_

#include <asm/unaligned.h>
#include <linux/bitops.h>
#include <linux/i2c.h>
#include <linux/module.h>
#include <linux/notifier.h>
#include <linux/platform_device.h>
#include <linux/power_supply.h>
#include <linux/slab.h>
#include <asm/unaligned.h>
#include <linux/bitops.h>
#include <linux/workqueue.h>

#include "bq40z50_soc_cali.h"

#define BATTERY_FUEL_INVALID                 0
#define BATTERY_FUEL_BQ40Z50                 1
#define BATTERY_FUEL_SH366006                2
#define BATTERY_FUEL_CBM8580                 3

#define BQ40Z50_CMD_LEN                      1
#define BQ40Z50_MANUFACTURER_CMD_LEN         2
#define BQ40Z50_RETRY_COUNT                  3
#define BQ40Z50_STATUS_BLOCK_LEN             5
#define BQ40Z50_DASTATUS_BLOCK_LEN           17
#define BQ40Z50_BLOCK_LEN_MAX                32
#define conver2u32(x)                        (((x)[4] << 24) + ((x)[3] << 16) + ((x)[2] << 8) + (x)[1])

/* battery temperature */
#define BQ40Z50_BATT_TEMP_ABNORMAL_LOW       (-20)
#define BQ40Z50_BATT_TEMP_ABNORMAL_HIGH      60
#define BQ40Z50_BATT_TEMP_ZERO               273
#define BQ40Z50_BATT_TEMP_ABNORMAL_MAX_CNT   2
#define BQ40Z50_BATT_CELL1_TEMP_OFFSET       3
#define BQ40Z50_BATT_CELL2_TEMP_OFFSET       5

#define BQ40Z50_BATT_RESISTANCE              3
#define BQ40Z50_RESISTANCE_SCALE             10
#if defined(CONFIG_COUL_DRV)
#define BQ40Z50_LOW_CAPACITY_CELL_VOL        3660
#define BQ40Z50_LOW_CAPACITY_MONITOR_VOL     11130
#elif defined(CONFIG_HUAWEI_COUL)
#define BQ40Z50_LOW_CAPACITY_CELL_VOL        3660
#define BQ40Z50_LOW_CAPACITY_MONITOR_VOL     7320
#endif /* CONFIG_COUL_DRV */
#define BQ40Z50_ABNORMAL_LOW_CAPACITY        4
#define BQ40Z50_LOW_CAPACITY_MAP_SOC         20
#define BQ40Z50_CUTOFF_VOL_DEFAULT           3000
#define BQ40Z50_CUTOFF_RETRY_COUNT           3

/* battery capcaity */
#define BQ40Z50_BATT_CAPACITY_ZERO           0
#define BQ40Z50_BATT_CAPACITY_CRITICAL       5
#define BQ40Z50_BATT_CAPACITY_LOW            15
#define BQ40Z50_BATT_CAPACITY_HIGH           95
#define BQ40Z50_BATT_CAPACITY_FULL           100
#define BQ40Z50_BATT_CAPACITY_DEFAULT        50
#define BQ40Z50_BATT_CAPACITY_LOW_LVL        3
#define BQ40Z50_BATT_CAPACITY_WARNING_LVL    10

/* battery voltage */
#define BQ40Z50_BATT_MAX_VOLTAGE_DEFAULT     13200
#define BQ40Z50_BATT_VOLTAGE_DEFAULT         11460

/* battery status */
#define BQ40Z50_BATT_STATUS_FC               BIT(5)

/* battery brand max len */
#define BQ40Z50_BATT_BRAND_LEN               30

/* charge state */
#define BQ40Z50_CHARGE_STATE_START_CHARGING  1
#define BQ40Z50_CHARGE_STATE_CHRG_DONE       2
#define BQ40Z50_CHARGE_STATE_STOP_CHARGING   3
#define BQ40Z50_CHARGE_STATE_NOT_CHARGING    4

/* bq40z50 reg map */
#define BQ40Z50_REG_MANUFACTURER_SEND        0x00 /* manufacturer access send cmd */
#define BQ40Z50_REG_MANUFACTURER_READ        0x23 /* manufacturer access read cmd */
#define BQ40Z50_REG_TEMP                     0x08 /* temperature */
#define BQ40Z50_REG_VOLT                     0x09 /* voltage */
#define BQ40Z50_REG_CELL1_VOLT               0x3F /* cell1 voltage */
#define BQ40Z50_REG_CELL2_VOLT               0x3E /* cell2 voltage */
#define BQ40Z50_REG_CELL3_VOLT               0x3D /* cell3 voltage */
#define BQ40Z50_REG_CURR                     0x0A /* current */
#define BQ40Z50_REG_RM                       0x0F /* remaining capacity */
#define BQ40Z50_REG_FCC                      0x10 /* full charge capacity */
#define BQ40Z50_REG_AVRGCURR                 0x0B /* average current */
#define BQ40Z50_REG_TTE                      0x12 /* average time to empty */
#define BQ40Z50_REG_TTF                      0x13 /* average time to full */
#define BQ40Z50_REG_CHARGING_CURRENT         0x14 /* charging current */
#define BQ40Z50_REG_CHARGING_VOLTAGE         0x15 /* charging voltage */
#define BQ40Z50_REG_BATTERY_STATUS           0x16 /* battery status */
#define BQ40Z50_REG_CYCLE                    0x17 /* cycle count */
#define BQ40Z50_REG_SOC                      0x0D /* relative state of charge */
#define BQ40Z50_REG_DC                       0x18 /* design capacity */
#define BQ40Z50_REG_DEVICE_NAME              0x21 /* device name */
#define BQ40Z50_REG_SOH                      0x4F /* state of health */
#define BQ40Z50_REG_FLAGS                    0x16 /* battery status */
#define BQ40Z50_REG_SAFETY_STATUS            0x51 /* safety status */
#define BQ40Z50_REG_PF_STATUS                0x53 /* PF status */
#define BQ40Z50_REG_OP_STATUS                0x54 /* Operation status */
#define BQ40Z50_REG_DASTATUS2                0x72 /* temperature data */
#define BQ40Z50_REG_LAST_SOC                 0x01 /* last soc before shut down */
#define BQ40Z50_REG_CTL                      0x44 /* manufacturer block access send or read cmd */

#define BQ40Z50_ABNORMAL_SOC_VALUE           (-1)
#define BQ40Z50_REG_LAST_SOC_DEFAULT         500
#define BQ40Z50_REG_LAST_SOC_VALID_FLAG      BIT(15)
#define BQ40Z50_BATT_CELL_COUNT_DEFAULT      2  /* 1:1cell, 2:2cells 3:3cells, 4:4cells */
#define BQ40Z50_BATT_3CELL                   3

#define BQ40Z50_REG_MAX_VAL                  0xFFFF /* reg max val */

/* bq40z50 gauge update param */
#define BQ40Z50_SUB_CMD_READ_STATUS_BLOCK_LEN           7
#define BQ40Z50_SUB_CMD_WIRTE_SIZE                      3
#define BQ40Z50_SUB_CMD_WIRTE_LEN                       0x02

#define BQ40Z50_SUB_CMD_SOH_FCC                         0x77 /* state of health full charge capacity mAh */
#define BQ40Z50_SOH_FCC_DATA_LOW_BITS_INDEX             3
#define BQ40Z50_SOH_FCC_DATA_HIGH_BITS_INDEX            4
#define BQ40Z50_SOH_FCC_BASE                            256

/* coul device_name judge factory */
#define BQ40Z50_FW_DEVICE_NAME_FW_VERSION_INDEX 15

struct bq40z50_reg_cache {
	s16 temp;
	s16 cell1_temp;
	s16 cell2_temp;
	u16 vol;
	u16 cell1_vol;
	u16 cell2_vol;
	s16 curr;
	s16 avg_curr;
	u16 rm;
	u16 tte;
	u16 ttf;
	u16 fcc;
	u16 dc;
	u16 cycle;
	u16 soc;
	u8 soh;
	u16 flags;
	u16 charge_current;
	u16 charge_voltage;
	u16 status;
};

struct bq40z50_device_info {
	struct i2c_client *client;
	struct device *dev;
	struct bq40z50_reg_cache cache;
	struct mutex rw_lock;
	struct rtc_device *rtc_dev;
	struct delayed_work soc_sync_rtc;
	struct workqueue_struct *sync_wq;
#if defined(CONFIG_COUL_DRV)
	struct chrg_para_lut *para_batt_data;
	struct bq40z50_cali_para cali_para;
	u32 is_smart_battery;
	bool use_converter;
	bool soc_cali_enable;
	int prevent_low_cap;
	int cutoff_vol;
	int low_vol_cnt;
#elif defined(CONFIG_HUAWEI_COUL)
	bool is_smart_battery;
#endif /* CONFIG_COUL_DRV */
	int charge_status;
	int vbat_max;
	int cell_count;
	u32 abnormal_temp_count;
	int device_id;
	int power_saving_enable;
};

struct bq40z50_log_data {
	int temp;
	int vbat;
	int ibat;
	int avg_ibat;
	int rm;
	int soc_unfilter;
	int soc_est;
	int soc;
	int fcc;
	int ibat_th;
	int vbat_th;
	int cycle_count;
	int cell1_vol;
	int cell2_vol;
	int bat_status;
	int safe_status;
	int pf_status;
	int op_status;
	int sohfcc;
};

int bq40z50_read_byte(struct bq40z50_device_info *di, u8 reg, u8 *value);
int bq40z50_write_byte(struct bq40z50_device_info *di, u8 reg, u8 value);
int bq40z50_read_word(struct bq40z50_device_info *di, u8 reg, u16 *data);
int bq40z50_write_word(struct bq40z50_device_info *di, u8 reg, u16 data);
int bq40z50_read_block(struct bq40z50_device_info *di, u8 reg, u8 *buf, u8 buf_len);
int bq40z50_write_block(struct bq40z50_device_info *di, u8 reg, u8 *buf, u8 buf_len);

#ifdef CONFIG_COUL_DRV
void bq40z50_set_di(struct bq40z50_device_info *di);
int bq40z50_ops_register_hisi(struct bq40z50_device_info *di);
int bq40z50_get_para_data(struct bq40z50_device_info *di);
#endif /* CONFIG_COUL_DRV */

int bq40z50_ops_register_base(struct bq40z50_device_info *di);
int bq40z50_is_ready_base(void *dev_data);
int bq40z50_get_battery_temp_base(void *dev_data);
int bq40z50_get_cell_temp_base(struct bq40z50_device_info *di, u8 reg_offset, s16 *temp_catche);
int bq40z50_is_battery_exist_base(void *dev_data);
int bq40z50_get_battery_soc_base(void *dev_data);
int bq40z50_get_battery_vol_base(void *dev_data);
int bq40z50_get_cell_vbat_mv_base(struct bq40z50_device_info *di, u8 reg, u16 *vol_catche);
int bq40z50_get_battery_curr_base(void *dev_data);
int bq40z50_get_battery_avgcurr_base(void *dev_data);
int bq40z50_get_battery_rm_base(void *dev_data);
int bq40z50_get_battery_dc_base(void *dev_data);
int bq40z50_get_battery_fcc_base(void *dev_data);
int bq40z50_get_battery_cycle_base(void *dev_data);
int bq40z50_set_battery_low_voltage_base(int val, void *dev_data);
int bq40z50_get_desired_charging_current_base(void *dev_data);
int bq40z50_get_desired_charging_voltage_base(void *dev_data);
bool bq40z50_is_smart_battery_base(void *dev_data);
const char *bq40z50_get_battery_brand_base(void *dev_data);
int bq40z50_get_battery_vbat_max_base(void *dev_data);
const char *bq40z50_get_coul_model_base(void *dev_data);
int bq40z50_get_battery_status_base(struct bq40z50_device_info *di);
int bq40z50_set_last_capacity_base(int capacity, void *dev_data);
int bq40z50_get_last_capacity_base(void *dev_data);
int bq40z50_get_safety_status_base(struct bq40z50_device_info *di);
int bq40z50_get_pf_status_base(struct bq40z50_device_info *di);
int bq40z50_get_operation_status_base(struct bq40z50_device_info *di);

#endif /* _BQ40Z50_H_ */
