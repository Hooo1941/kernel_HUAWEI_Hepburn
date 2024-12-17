/* SPDX-License-Identifier: GPL-2.0
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022.
 * All rights reserved.
 * Description: Noise Calculation and Denoising Algorithm Task
 * Author: yangxiaopeng@huawei.com
 * Create: 2022-11-1
 */

#ifndef MATTING_ALGO_TASK_H
#define MATTING_ALGO_TASK_H

#include "als_matting_algo_info.h"

enum matting_algo_lcd_state {
	MATTING_ALGO_AP_LCD_CLOSE = 0,
	MATTING_ALGO_AP_LCD_OPEN = 1,
	MATTING_ALGO_AP_LCD_INVALID,
};

struct matting_algo_lock_st {
	struct mutex set_frame_crop_state_lock;
	struct mutex set_start_time_lock;
	struct mutex set_cut_out_index_lock;
	struct mutex matting_algo_lux_value_lock;
	struct mutex matting_algo_brightness_lock;
};

bool matting_algo_debug_enable(void);
bool matting_algo_crop_debug_enable(void);
void set_alg_matting_data_param_valid(bool valid);
bool get_alg_matting_data_param_valid(void);
void set_alg_matting_data_param_info(void *param);
struct alg_matting_data_param_info_st *get_alg_matting_data_param_info(void);
uint64_t get_millisec_timestamp(void);
int read_file_to_buffer(uint8_t *data, int data_len, uint8_t *path);
void write_file_to_device(uint8_t *data, int data_len, uint8_t *path);
uint64_t matting_algo_get_start_time(void);
void matting_algo_set_start_time(uint64_t start_time);
bool get_frame_crop_state(void);
void set_frame_crop_state(bool state);
struct semaphore *matting_algo_get_semaphore_frame_crop(void);
struct semaphore *matting_algo_get_semaphore_als_report(void);
struct alg_matting_data_param_info_st *matting_algo_get_data_param_info(void);
int32_t matting_algo_get_cut_out_index(void);
void matting_algo_set_cut_out_index(int32_t cut_out_index);
struct matting_algo_lock_st *matting_algo_get_lock_st(void);
void matting_algo_lock_st_init(void);
uint32_t matting_algo_get_current_brightness_callback(uint32_t brightness);
uint32_t matting_algo_get_current_brightness(void);
void matting_algo_set_current_brightness(uint32_t brightness);
#endif