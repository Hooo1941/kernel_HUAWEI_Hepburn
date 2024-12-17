/* SPDX-License-Identifier: GPL-2.0
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022.
 * All rights reserved.
 * Description: head file of ALS image matting algorithm
 * Author: yangxiaopeng@huawei.com
 * Create: 2022-11-24
 */

#ifndef ALS_MATTING_ALGO_INFO_H
#define ALS_MATTING_ALGO_INFO_H
#include <stdint.h>

#define RGB_CURVE 3
#define DBV_COEF_CURVE 3

#define SENSORMGR_ALS_CHANNEL_MAX (5)

#define SY399_GAIN_COUNT 4
#define SY3133_CHANNEL_NUMBER 4

#define DEFAULT_POWTABLE_MULTIPLES (100000000)
#define POWTABLE_MULTIPLES (1000000)
#define DBV_NOISE_RETAIN_MULTIPLES (100)
#define DEFAULT_MULTIPLES  (1000)

#define MILLI (1000)
#define SENSORMGR_ALS_CALIB_COEF (10000)

enum matting_algo_index {
	INDEX_ZERO = 0,
	INDEX_ONE,
	INDEX_TWO,
	INDEX_THREE,
	INDEX_FOUR,
	INDEX_FIVE,
	INDEX_SIX,
	INDEX_SEVEN,
	INDEX_EIGHT,
	INDEX_NINE,
	INDEX_TEN,
	INDEX_MAX
};

enum matting_algo_ctr_cmd_type {
	GET_ALG_MAT_PARAM_CMD = 0x01,
	SEND_ALG_MAT_PARAM_CMD = 0x02,
	SEND_ALG_MAT_PARAM_RESPONSE = 0x03,
};

enum matting_algo_ctr_sub_cmd_type {
	ALG_MAT_SUBCMD,
	ALG_MAT_CMD_VALUES,
};

#pragma pack(push, 4)
enum matting_algo_als_type_en {
	ALS_AUTO = 0x00,
	ALS_TSL2584TSV = 0x01,
	ALS_VD6281 = 0x02,
	ALS_TCS3701 = 0x03,
	ALS_STK32670 = 0x04,
	ALS_SYH399AS23 = 0x05,
	ALS_TSL25913 = 0x06,
	ALS_LTR311 = 0x07,
	ALS_TSL2585 = 0x08,
	ALS_TSL2540 = 0x09,
	ALS_SY3133 = 0x0A,
	ALS_SIP1225 = 0x0B,
	ALS_ID_MAX = 0xFF,
};

enum matting_algo_als_lux_type_en {
	ALS_LUX_MIN = 0,
	ALS_LUX_MAX = 1,
};

/* ambient optical sensor offset configuration parameters */
struct als_offset_st {
	/* Offset for lux calibration */
	uint32_t als_offset;
	/* Channel calibration offset */
	uint32_t channel_offset[SENSORMGR_ALS_CHANNEL_MAX];
};

struct als_lcd_coef_st {
	enum matting_algo_als_lux_type_en lux_type;
	float dg_f2_ch;
	union {
		float coef_a2_ch;
		float coef_b2_ch;
	};
	float coef_c2_ch;
	float coef_d2_ch;
};

struct coefficient_st {
	int32_t photopic;
	int32_t ir;
	int32_t dgf;
};

struct tsl2585_als_coef {
	int32_t ir_pho_ratio1;
	int32_t ir_pho_ratio2;
	int32_t ir_data_percent;
	uint32_t multiples;
	struct coefficient_st coef_a;
	struct coefficient_st coef_c;
};

struct sip1225_coef_part_st {
	int32_t coef1;
	int32_t coef2;
	uint32_t multiples;
};

struct sip1225_als_coef_st {
	int32_t ratio;
	uint32_t multiples;
	struct sip1225_coef_part_st coef_h_s;
	struct sip1225_coef_part_st coef_l_s;
};

struct tsl2540_als_coef_st {
	enum matting_algo_als_lux_type_en lux_type;
	float ir_ratio;
	float dg_f2_ch;
	float coef_b2_ch;
	float coef_c2_ch;
	float coef_d2_ch;
	float coef_e2_ch;
	float coef_f2_ch;
};

struct sy3133_als_coef_st {
	float green_coef;
	float green_offset;
	float lux_offset;
	float ratio_h;
	float lux_coef_h;
	float ratio_xe;
	float lux_coef_xe;
	float ratio_d;
	float lux_coef_d;
	float lux_coef_u;
};

struct ltr311_lcd_info_st {
	uint32_t lcd_type;
	uint32_t c_limit;
	uint32_t d_limit;
};

struct ltr311_lux_param_st {
	struct ltr311_lcd_info_st ltr311_lcd_info_s;
	uint32_t ltr311_lux_ratio;
};

struct syh399_lcd_para_st {
	float als_coef;
	float clr_coef;
	float a_thres;
	float d_thres;
	float c_thres;
	float c_ratio;
	float d_ratio;
	float a_ratio;
	uint32_t als_middle;
	uint32_t clr_middle;
};

struct syh399_as23_drv_info_st {
	uint8_t als_dark_cnt;
	uint8_t clr_dark_cnt;
};

struct sy3133_lux_param_st {
	float gain_normal;
	float time_normal;
	uint16_t data_avg[SY3133_CHANNEL_NUMBER];
};

struct syh399_lux_param_st {
	struct syh399_lcd_para_st syh399_lcd_para_s;
	struct syh399_as23_drv_info_st syh399_as23_drv_data_s;
	uint8_t syh399_als_gain_factor[SY399_GAIN_COUNT];
	uint8_t syh399_clr_gain_factor[SY399_GAIN_COUNT];
	uint16_t shy399_as23_als_gain[SY399_GAIN_COUNT];
	uint32_t als_dark_noise;
	uint32_t clr_dark_noise;
	uint32_t clr_threshold;
	uint32_t clr_diffclr;
	uint32_t average_cnt;
	uint32_t mth_root;
	uint32_t gold_offset;
	uint32_t lux_factor;
	uint32_t ratio_low;
	uint32_t ratio_offset;
};

union als_lux_param_un {
	struct ltr311_lux_param_st sip1225_param_s;
	struct sy3133_lux_param_st tsl2540_param_s;
	struct syh399_lux_param_st sy3133_param_s;
};

union als_lcd_coef_info_un {
	struct als_lcd_coef_st normal_coef_s;
	struct tsl2585_als_coef tsl2585_coef_s;
	struct sip1225_als_coef_st sip1225_coef_s;
	struct tsl2540_als_coef_st tsl2540_coef_s;
	struct sy3133_als_coef_st sy3133_coef_s;
};

struct alg_matting_data_param_info_st {
	int32_t dbv_coef[SENSORMGR_ALS_CHANNEL_MAX][RGB_CURVE][DBV_COEF_CURVE];
	/* Multiple of 100000 */
	uint32_t multiples;
	uint16_t offset_x;
	uint16_t offset_y;
	/* Width of the cutout FOV area */
	uint16_t fov_width;
	/* Height of the cutout FOV area */
	uint16_t fov_height;
	uint32_t default_gain[SENSORMGR_ALS_CHANNEL_MAX];
	uint8_t als_type_e;
	union als_lcd_coef_info_un als_coef_u;
	union als_lux_param_un lux_param_u;
	struct als_offset_st als_offset_s;
};

#pragma pack(pop)

#endif /* ALS_MATTING_ALGO_INFO_H */
