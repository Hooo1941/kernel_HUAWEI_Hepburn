/* SPDX-License-Identifier: GPL-2.0
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022.
 * All rights reserved.
 * Description: head file of ALS image matting algorithm
 * Author: zhoujie09@huawei.com
 * Create: 2022-11-24
 */

#ifndef ALS_MATTING_ALGO_H
#define ALS_MATTING_ALGO_H

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#define DIS_ALS_CUTOUT_VALUE 45

#define ALS_CUTOUT_STOP_STATE_INDEX (-1)

enum matting_algo_error_code {
	EC_SUCCESS = 0,  /* OK or No error */
	EC_FAILURE = -1, /* Execution failed */
	EC_INVALID = -9, /* Invalid argument */
};

#define DISPLAY_ALS_CHANNEL_MAX 5
#define FRAME_MAX 9
#define RGB_CURVE 3
#define DBV_COEF_CURVE 3
/* Default ALS average transmission duration transferred from the MCU */
#define ALS_INTEG_TIME 125

struct bgr_pixel_info_st {
	uint8_t blue;
	uint8_t green;
	uint8_t red;
};

struct rgb_pixel_info_st {
	uint8_t red;
	uint8_t green;
	uint8_t blue;
};

struct display_data_info_st {
	uint8_t blightness;
	uint8_t frame_num;
	uint64_t frame_timestamp[FRAME_MAX];
	uint32_t als_channel_noise[FRAME_MAX][DISPLAY_ALS_CHANNEL_MAX];
	uint32_t *pixel_data;
	uint32_t pixel_line_offset;
};

struct display_als_data_info_st {
	/* Ambient light intensity */
	uint32_t als_lux;
	/* Ambient optical component gain value */
	uint16_t als_gain[DISPLAY_ALS_CHANNEL_MAX];
	/* Integration time of ambient light components */
	uint16_t als_atime;
	/* Ambient Optical Component Type */
	uint32_t als_type;
	/* Ambient Optical Channel Value */
	uint32_t als_channel[DISPLAY_ALS_CHANNEL_MAX];
	/* Ambient light collection start time */
	uint64_t timestamp_start;
	/* End time of ambient light collection */
	uint64_t timestamp_end;
};

int32_t alg_mat_oneframe_noise_calc(struct display_data_info_st *display_data);
int32_t alg_mat_all_frame_noise_calc(struct display_data_info_st *display_data,
	uint64_t als_start, uint64_t als_end, uint32_t *als_chn_noise,
	uint8_t als_chn_noise_count);
void lcd_calc_screen_noise(uint8_t *fb_start_addr);
uint32_t lcd_dest_als_value_calc(struct display_als_data_info_st *als_data);
void matting_algo_clear_calc_cut_out_info(void);
void matting_algo_clear_last_print_brightness(void);
#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */
#endif /* ALS_MATTING_ALGO_H */