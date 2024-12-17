/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: private interface for coul module
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

#ifndef _COUL_ALGO_PRIVATE_INTERFACE_H_
#define _COUL_ALGO_PRIVATE_INTERFACE_H_

#include <platform_include/basicplatform/linux/power/platform/coul/coul_event.h>
#include "coul_core.h"

struct soc_param_data {
	int fcc_uah;
	int unusable_charge_uah;
	int remaining_charge_uah;
	int cc_uah;
	int delta_rc_uah;
	int rbatt;
};

struct coul_device_ops {
	int (*calculate_cc_uah)(int batt);
	void (*save_cc_uah)(int batt, int cc_uah);
	int (*convert_ocv_regval2ua)(int batt, short reg_val);
	int (*convert_ocv_regval2uv)(int batt, short reg_val);
	int (*is_battery_moved)(int batt);
	void (*set_battery_moved_magic_num)(int batt, int move_flag);
	void (*get_fifo_avg_data)(int batt, struct vcdata *vc);
	int (*get_fifo_depth)(void);
	int (*get_delta_rc_ignore_flag)(int batt);
	int (*get_nv_read_flag)(int batt);
	void (*set_nv_save_flag)(int batt, int nv_flag);
	int (*get_use_saved_ocv_flag)(int batt);
	int (*get_fcc_invalid_up_flag)(int batt);
	void (*set_low_low_int_val)(int vol_mv);
	void (*save_ocv)(int batt, int ocv, int invalid_fcc_up_flag);
	short (*get_ocv)(int batt);
	void (*clear_ocv)(int batt);
	void (*save_ocv_temp)(int batt, short ocv_temp);
	short (*get_ocv_temp)(int batt);
	void (*clear_ocv_temp)(int batt);
	int (*get_abs_cc)(int batt);
	unsigned int (*get_coul_time)(int batt);
	void (*clear_coul_time)(int batt);
	void (*clear_cc_register)(int batt);
	void (*cali_adc)(int batt);
	int (*get_battery_voltage_uv)(int batt);
	int (*get_battery_current_ua)(int batt);
	int (*get_battery_vol_uv_from_fifo)(int batt, short fifo_order);
	int (*get_battery_cur_ua_from_fifo)(int batt, short fifo_order);
	int (*get_eco_vol_uv_from_fifo)(int batt, short fifo_order);
	int (*get_eco_cur_ua_from_fifo)(int batt, short fifo_order);
	int (*get_eco_temp_from_fifo)(int batt, short fifo_order);
	short (*get_offset_current_mod)(void);
	short (*get_offset_vol_mod)(void);
	void (*set_offset_vol_mod)(void);
	int (*get_ate_a)(void);
	int (*get_ate_b)(void);
	void (*irq_enable)(int batt);
	void (*irq_disable)(int batt);
	void (*show_key_reg)(void);
	void (*enter_eco)(int batt);
	void (*exit_eco)(int batt);
	int (*calculate_eco_leak_uah)(int batt);
	void (*save_last_soc)(short soc);
	void (*get_last_soc)(short *soc);
	void (*clear_last_soc_flag)(void);
	void (*get_last_soc_flag)(bool *valid);
	void (*cali_auto_off)(void);
	void (*save_ocv_level)(int batt, u8 level);
	void (*get_ocv_level)(int batt, u8 *level);
	void (*set_i_in_event_gate)(int batt, int ma);
	void (*set_i_out_event_gate)(int batt, int ma);
	int (*get_chip_temp)(void);
	int (*get_bat_temp)(int batt);
	int (*convert_regval2uv)(int batt, unsigned int reg_val);
	int (*convert_regval2ua)(int batt, unsigned int reg_val);
	int (*convert_regval2temp)(unsigned int reg_val);
	unsigned int (*convert_uv2regval)(int batt, int uv_val);
	s64 (*convert_regval2uah)(int batt, u64 reg_val);

	int (*get_drained_battery_flag)(int batt);
	void (*clear_drained_battery_flag)(int batt);
	void (*set_bootocv_sample)(int batt, u8 set_val);
	void (*set_dev_cali_param)(int batt, struct coul_cali_params *params);
	int (*coul_dev_check)(void);
	int (*set_batt_mode)(int mode);
};

unsigned int coul_core_get_pd_charge_flag(void);
int coul_core_get_battery_temperature(void *data);
int coul_core_is_battery_exist(void *data);
int coul_core_get_battery_capacity(void *data);
void coul_core_get_battery_voltage_and_current(
	struct smartstar_coul_device *di, int *ibat_ua, int *vbat_uv);
int coul_core_get_battery_voltage_mv(void *data);
int coul_core_get_battery_current_ma(void *data);
int coul_get_battery_unfiltered_capacity(void *data);
int coul_core_get_battery_rm(void *data);
int coul_core_get_battery_fcc(void *data);
int coul_core_get_battery_uuc(void *data);
int coul_core_get_battery_cc(void *data);
int coul_core_get_battery_cc_cache(void *data);
int coul_core_get_battery_delta_rc(void *data);
int coul_core_get_battery_ocv(void *data);
int coul_core_get_battery_resistance(void *data);
int coul_core_get_chip_temp(void *data);
void coul_calibration_adc(struct smartstar_coul_device *di, int temp_care);
void coul_get_last_cali_info(struct smartstar_coul_device *di,
	int *last_cali_temp, int *last_cali_time);
int coul_core_get_fifo_avg_current_ma(void *data);
int coul_is_between(int left, int right, int value);
int coul_linear_interpolate(int y0, int x0, int y1, int x1, int x);
int coul_interpolate_single_lut(const struct single_row_lut *lut, int x);
int coul_interpolate_single_y_lut(const struct single_row_lut *lut, int y);
int coul_interpolate_scalingfactor(const struct sf_lut *sf_lut,
	int row_entry, int pc);
int coul_interpolate_fcc(struct smartstar_coul_device *di, int batt_temp);
int coul_calculate_fcc_uah(struct smartstar_coul_device *di);
int coul_interpolate_ocv(const struct pc_temp_ocv_lut *lut,
	int batt_temp_degc, int pc);
int coul_coul_interpolate_pc_high_precision(const struct pc_temp_ocv_lut *lut,
	int batt_temp, int ocv);
int coul_interpolate_pc(const struct pc_temp_ocv_lut *lut, int batt_temp, int ocv);
int coul_calculate_pc(struct smartstar_coul_device *di, int ocv_uv,
	int batt_temp, int chargecycles);
int coul_get_rbatt(struct smartstar_coul_device *di, int soc_rbatt, int batt_temp);
int coul_basp_full_pc_by_voltage(struct smartstar_coul_device *di);
int coul_core_get_qmax(struct smartstar_coul_device *di);
int coul_core_get_battery_qmax(void *data);
int coul_battery_para_changed(struct smartstar_coul_device *di);
void coul_force_ocv_update(struct smartstar_coul_device *di);
bool coul_could_cc_update_ocv(struct smartstar_coul_device *di);
bool coul_is_in_capacity_dense_area(struct smartstar_coul_device *di, int ocv_uv);
int coul_get_timestamp(char *str, int len);
void coul_batt_notify(struct smartstar_coul_device *di,
	enum batt_coul_event event);
void coul_core_set_low_vol_int(struct smartstar_coul_device *di, int state);
void coul_get_battery_id_voltage(struct smartstar_coul_device *di);
int coul_bound_soc(int soc);
void coul_clear_moved_battery_data(struct smartstar_coul_device *di, const struct device_node *cfg);
int coul_probe_check_coul_dev_ops(struct smartstar_coul_device *di);
int coul_check_batt_data(struct smartstar_coul_device *di);
int coul_get_zero_cap_vol(struct smartstar_coul_device *di);
void coul_calculate_iavg_ma(struct smartstar_coul_device *di, int iavg_ua);
void coul_set_work_interval(struct smartstar_coul_device *di);
int coul_core_register(struct coul_device_ops *dev_ops, const struct device_node *cfg);
void check_chg_done_max_avg_cur_flag(struct smartstar_coul_device *di);
void coul_start_soh_check(struct smartstar_coul_device *di);

#endif
