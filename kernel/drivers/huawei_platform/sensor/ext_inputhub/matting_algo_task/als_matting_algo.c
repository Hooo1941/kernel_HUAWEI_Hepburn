// SPDX-License-Identifier: GPL-2.0
/* Copyright (c) Huawei Technologies Co., Ltd. 2022-2022.
 * All rights reserved.
 * Description: ALS image matting algorithm source file
 * Author: zhoujie09@huawei.com
 * Create: 2022-11-24
 */
#include <linux/kernel.h>
#include <linux/slab.h>
#include <stdint.h>
#include "securec.h"
#include "als_matting_algo.h"
#include "als_matting_algo_info.h"
#include "image_compositing_algo.h"
#include "matting_algo_task.h"
#include <huawei_platform/log/hw_log.h>
#include "math_calculation.h"
#include "lcd_kit_drm_panel.h"
#include "formula_als.h"

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif

#define HWLOG_TAG matting_algo_task
HWLOG_REGIST();

#define LUX_VALUE_INDOOR (500)
#define LUX_VALUE_DIFF (30)
#define LUX_VALUE_ALLOWABLE_PERCENT (10)
#define LUX_VALUE_FULL_PERCENT (100)
#define BRIGHTNESS_VALUE_MAX_LEN  (92)
#define DEC_BASE (10)
#define BACKLIGHT_PATH "/sys/class/backlight/panel0-backlight/brightness"

#define GAMMA_TABLE_SIZE 256
#define R_CHN 0
#define G_CHN 1
#define B_CHN 2
#define COEF0 0
#define COEF1 1
#define COEF2 2
#define FIRST_BOUNDARY 0
#define FRAME_NEXT 1
#define MAX_PLOLYFIT_ORDER 3
#define ORDER_TO_COEF_OFFSET 1
#define POWY_TO_POWX_FACTOR 2
#define MAX_PLOLYFIT_Y_COUNT (MAX_PLOLYFIT_ORDER + ORDER_TO_COEF_OFFSET)
#define MAX_PLOLYFIT_MATRIX_COUNT (MAX_PLOLYFIT_Y_COUNT * \
	POWY_TO_POWX_FACTOR * MAX_PLOLYFIT_Y_COUNT)
#define POW_LEVEL_COUNT (((MAX_PLOLYFIT_ORDER + ORDER_TO_COEF_OFFSET) * \
	POWY_TO_POWX_FACTOR) + ORDER_TO_COEF_OFFSET)

static const uint32_t g_pow_table[GAMMA_TABLE_SIZE] = {
	0,        508,      2333,     5692,     10719,    17512,    26154,    36714,
	49250,    63818,    80466,    99237,    120174,   143313,   168692,   196342,
	226295,   258583,   293232,   330270,   369724,   411618,   455975,   502820,
	552174,   604059,   658496,   715504,   775103,   837312,   902149,   969633,
	1039780,  1112608,  1188133,  1266372,  1347340,  1431052,  1517524,  1606770,
	1698805,  1793643,  1891298,  1991784,  2095113,  2201299,  2310356,  2422294,
	2537128,  2654868,  2775528,  2899119,  3025652,  3155139,  3287592,  3423021,
	3561437,  3702851,  3847275,  3994717,  4145189,  4298701,  4455263,  4614884,
	4777575,  4943346,  5112205,  5284163,  5459228,  5637410,  5818718,  6003161,
	6190748,  6381487,  6575388,  6772459,  6972708,  7176145,  7382777,  7592612,
	7805659,  8021926,  8241421,  8464151,  8690125,  8919351,  9151835,  9387587,
	9626612,  9868920,  10114516, 10363410, 10615607, 10871115, 11129941, 11392093,
	11657578, 11926401, 12198571, 12474095, 12752978, 13035228, 13320851, 13609855,
	13902245, 14198029, 14497213, 14799802, 15105805, 15415226, 15728073, 16044351,
	16364067, 16687227, 17013837, 17343904, 17677432, 18014429, 18354900, 18698851,
	19046288, 19397217, 19751643, 20109573, 20471012, 20835966, 21204440, 21576440,
	21951972, 22331041, 22713653, 23099812, 23489526, 23882798, 24279635, 24680042,
	25084024, 25491586, 25902733, 26317472, 26735806, 27157742, 27583283, 28012437,
	28445206, 28881597, 29321615, 29765264, 30212550, 30663477, 31118050, 31576274,
	32038155, 32503696, 32972903, 33445781, 33922333, 34402566, 34886483, 35374090,
	35865391, 36360390, 36859092, 37361502, 37867625, 38377465, 38891026, 39408313,
	39929330, 40454082, 40982574, 41514809, 42050793, 42590529, 43134021, 43681275,
	44232295, 44787084, 45345648, 45907989, 46474113, 47044025, 47617727, 48195224,
	48776520, 49361620, 49950528, 50543247, 51139782, 51740137, 52344316, 52952322,
	53564161, 54179836, 54799350, 55422709, 56049915, 56680973, 57315888, 57954661,
	58597298, 59243803, 59894179, 60548430, 61206560, 61868573, 62534472, 63204262,
	63877946, 64555527, 65237011, 65922399, 66611697, 67304907, 68002034, 68703081,
	69408052, 70116950, 70829779, 71546543, 72267245, 72991889, 73720479, 74453017,
	75189508, 75929955, 76674362, 77422731, 78175068, 78931374, 79691654, 80455911,
	81224149, 81996371, 82772579, 83552779, 84336973, 85125165, 85917357, 86713554,
	87513758, 88317974, 89126204, 89938451, 90754720, 91575013, 92399334, 93227685,
	94060071, 94896494, 95736958, 96581465, 97430020, 98282626, 99139284, 100000000,
};

static void alg_mat_oneframe_rgb_gamma_calc(struct display_data_info_st *display_data,
	uint64_t *all_pixel_rgb_value, uint8_t rgb_ch_count)
{
	uint32_t pixel_idx;
	uint32_t red;
	uint32_t green;
	uint32_t blue;
	uint8_t i;
	struct rgb_pixel_info_st *cur_line_start_addr = NULL;
	struct alg_matting_data_param_info_st *data_param_info = matting_algo_get_data_param_info();

	if (display_data->pixel_data == NULL) {
		hwlog_err("[%s] cur_frame_start_addr null err\n", __func__);
		return;
	}

	if (rgb_ch_count != RGB_CURVE) {
		hwlog_err("[%s] rgb_ch_count=%d err\n", __func__, rgb_ch_count);
		return;
	}

	for (i = 0; i < data_param_info->fov_height; i++) {
		cur_line_start_addr = (struct rgb_pixel_info_st *)((uint8_t *)display_data->pixel_data +
			display_data->pixel_line_offset * i);
		for (pixel_idx = 0; pixel_idx < data_param_info->fov_width; pixel_idx++) {
			red = (cur_line_start_addr + pixel_idx)->red;
			green = (cur_line_start_addr + pixel_idx)->green;
			blue = (cur_line_start_addr + pixel_idx)->blue;

			all_pixel_rgb_value[R_CHN] = all_pixel_rgb_value[R_CHN] + g_pow_table[red];
			all_pixel_rgb_value[G_CHN] = all_pixel_rgb_value[G_CHN] + g_pow_table[green];
			all_pixel_rgb_value[B_CHN] = all_pixel_rgb_value[B_CHN] + g_pow_table[blue];
		}
	}

	/* The gamma parameter retains eight decimal places.
	 * In this case, only two decimal places are required.
	 */
	for (i = 0; i < RGB_CURVE; i++)
		all_pixel_rgb_value[i] = math_div64_32(all_pixel_rgb_value[i], POWTABLE_MULTIPLES, NULL);
}

static void alg_mat_dbv_curve_calc(struct display_data_info_st *display_data,
	uint64_t max_noise[][RGB_CURVE])
{
	int64_t brightness = display_data->blightness;
	uint8_t chn_idx;
	uint8_t rgb_idx;
	uint32_t retain_multiples;
	struct alg_matting_data_param_info_st *data_param_info = matting_algo_get_data_param_info();

	for (chn_idx = 0; chn_idx < DISPLAY_ALS_CHANNEL_MAX; chn_idx++) {
		for (rgb_idx = 0; rgb_idx < RGB_CURVE; rgb_idx++) {
			max_noise[chn_idx][rgb_idx] = brightness * brightness *
				data_param_info->dbv_coef[chn_idx][rgb_idx][COEF0] +
				brightness * data_param_info->dbv_coef[chn_idx][rgb_idx][COEF1] +
				data_param_info->dbv_coef[chn_idx][rgb_idx][COEF2];
			/* The noise parameter retains multiples (currently 10^5) decimal places.
			 * Currently, only two decimal places are required.
			 */
			retain_multiples = data_param_info->multiples;
			if (data_param_info->multiples > DBV_NOISE_RETAIN_MULTIPLES)
				retain_multiples = data_param_info->multiples / DBV_NOISE_RETAIN_MULTIPLES;

			max_noise[chn_idx][rgb_idx] = math_div64_32(max_noise[chn_idx][rgb_idx],
				retain_multiples, NULL);
		}
	}
}

static void alg_mat_noise_calc_func(const uint64_t *all_pixel_rgb_value,
	uint64_t max_noise[][RGB_CURVE], uint8_t cur_frame_idx,
	uint32_t als_noise[][DISPLAY_ALS_CHANNEL_MAX])
{
	uint8_t chn_idx;
	uint8_t rgb_idx;
	struct alg_matting_data_param_info_st *data_param_info = matting_algo_get_data_param_info();
	uint16_t fov_size = data_param_info->fov_height * data_param_info->fov_width;
	uint64_t noise[DISPLAY_ALS_CHANNEL_MAX] = {0};
	uint64_t value;
	uint32_t remaining_multiples;

	for (chn_idx = 0; chn_idx < DISPLAY_ALS_CHANNEL_MAX; chn_idx++) {
		for (rgb_idx = 0; rgb_idx < RGB_CURVE; rgb_idx++) {
			value = all_pixel_rgb_value[rgb_idx] * max_noise[chn_idx][rgb_idx];
			noise[chn_idx] += value;
		}

		value = math_div64_32(noise[chn_idx], fov_size, NULL);

		remaining_multiples = DEFAULT_POWTABLE_MULTIPLES / POWTABLE_MULTIPLES;
		if (data_param_info->multiples > DBV_NOISE_RETAIN_MULTIPLES)
			remaining_multiples *= DBV_NOISE_RETAIN_MULTIPLES;
		else
			remaining_multiples *= data_param_info->multiples;

		als_noise[cur_frame_idx][chn_idx] =  math_div64_32(value, remaining_multiples, NULL);
	}
}

static uint64_t alg_mat_math_minus_without_overflow(uint64_t minuend, uint64_t subtrahend, uint64_t default_value)
{
	if (minuend >= subtrahend)
		return minuend - subtrahend;
	else
		return default_value;
}

static void alg_mat_multi_frame_noise_calc(struct display_data_info_st *display_data,
	uint64_t als_start, uint64_t als_end, uint32_t *als_chn_noise,
	uint8_t als_chn_noise_count)
{
	uint64_t *frame_time_stamp = display_data->frame_timestamp;
	uint8_t frame_num = display_data->frame_num;
	uint8_t fram_idx;
	uint8_t chn_idx;
	uint64_t duration[FRAME_MAX] = {0};
	uint32_t duration_scope;

	if (als_end <= als_start) {
		hwlog_info("[%s]: frame_num=%d, als_end=%d, als_start=%d\n",
			__func__, frame_num, als_end, als_start);
		return;
	}

	duration_scope = als_end - als_start;
	for (fram_idx = 0;  fram_idx < frame_num; fram_idx++) {
		// first boundary frame timestamp revise
		if (fram_idx == FIRST_BOUNDARY) {
			if (frame_num == 1)
				duration[fram_idx] = duration_scope;
			else
				duration[fram_idx] = alg_mat_math_minus_without_overflow(frame_time_stamp[fram_idx + FRAME_NEXT],
					als_start, 0);
		} else if (frame_num - 1 == fram_idx) {
			// last boundary frame timestamp revise
			duration[fram_idx] = alg_mat_math_minus_without_overflow(als_end, frame_time_stamp[fram_idx], 0);
		} else {
			duration[fram_idx] = alg_mat_math_minus_without_overflow(frame_time_stamp[fram_idx + FRAME_NEXT],
				frame_time_stamp[fram_idx], 0);
		}
	}

	for (fram_idx = 0; fram_idx < frame_num; fram_idx++) {
		// percentage calculation
		for (chn_idx = 0; chn_idx < als_chn_noise_count; chn_idx++)
			als_chn_noise[chn_idx] += (uint32_t)(math_div64_32(display_data->als_channel_noise[fram_idx][chn_idx]
				* duration[fram_idx], duration_scope, NULL));
	}
}

int32_t alg_mat_oneframe_noise_calc(struct display_data_info_st *display_data)
{
	uint8_t cur_frame_idx = display_data->frame_num - 1;
	uint64_t all_pixel_rgb_value[RGB_CURVE] = {0};
	uint64_t max_noise[DISPLAY_ALS_CHANNEL_MAX][RGB_CURVE];

	if (display_data == NULL) {
		hwlog_err("[%s]: display_data null err\n", __func__);
		return EC_FAILURE;
	}

	if (cur_frame_idx >= FRAME_MAX) {
		hwlog_err("[%s]: frame_index=%d err, FRAME_MAX=%d\n",
			__func__, cur_frame_idx, FRAME_MAX);
		return EC_FAILURE;
	}

	alg_mat_dbv_curve_calc(display_data, max_noise);
	alg_mat_oneframe_rgb_gamma_calc(display_data, all_pixel_rgb_value, RGB_CURVE);
	alg_mat_noise_calc_func(all_pixel_rgb_value, max_noise, cur_frame_idx,
		display_data->als_channel_noise);

	return EC_SUCCESS;
}

int32_t alg_mat_all_frame_noise_calc(struct display_data_info_st *display_data,
	uint64_t als_start, uint64_t als_end, uint32_t *als_chn_noise,
	uint8_t als_chn_noise_count)
{
	uint8_t frame_num;

	if (display_data == NULL) {
		hwlog_err("[%s]: display_data null err\n", __func__);
		return EC_FAILURE;
	}

	if (als_chn_noise == NULL) {
		hwlog_err("[%s]: als_chn_noise null err\n", __func__);
		return EC_FAILURE;
	}

	frame_num = display_data->frame_num;
	if ((frame_num == 0) || (frame_num > FRAME_MAX)) {
		hwlog_err("[%s]: frame_num=%d error, FRAME_MAX=%d.\n",
			__func__, frame_num, FRAME_MAX);
		return EC_FAILURE;
	}

	alg_mat_multi_frame_noise_calc(display_data, als_start, als_end,
		als_chn_noise, als_chn_noise_count);

	return EC_SUCCESS;
}

bool lcd_is_screen_nosie_zero(uint32_t *als_channel_noise, uint8_t noise_count)
{
	uint32_t noise_sum = 0;
	uint8_t channel;

	for (channel = 0; channel < noise_count; channel++)
		noise_sum += als_channel_noise[channel];

	if (noise_sum == 0)
		return true;

	return false;
}

static void lcd_als_channel_minus_noise(struct display_als_data_info_st *als_info,
	uint32_t *als_ch_noise, uint8_t noise_count, struct sensor_hub_als_data_st *sensor_als_data_ptr)
{
	uint8_t channel;
	uint32_t als_gain;
	struct alg_matting_data_param_info_st *data_param_info = matting_algo_get_data_param_info();

	for (channel = 0; channel < noise_count; channel++) {
		if (als_info->als_gain[channel] != 0) {
			als_gain = math_div64_32(als_info->als_gain[channel] * DEFAULT_MULTIPLES,
				data_param_info->default_gain[channel], NULL);
			als_ch_noise[channel] *= math_div64_32(als_gain, DEFAULT_MULTIPLES, NULL);
			if (sensor_als_data_ptr->als_channel[channel] > als_ch_noise[channel])
				sensor_als_data_ptr->als_channel[channel] -= als_ch_noise[channel];
			else
				sensor_als_data_ptr->als_channel[channel] = 0;
		}
	}
}

static void get_sensor_hub_als_data(struct display_als_data_info_st *display_als_data_ptr,
	struct sensor_hub_als_data_st *sensor_als_data_ptr)
{
	memcpy_s(sensor_als_data_ptr->als_again, sizeof(sensor_als_data_ptr->als_again),
		display_als_data_ptr->als_gain, sizeof(display_als_data_ptr->als_gain));

	sensor_als_data_ptr->als_atime = display_als_data_ptr->als_atime;

	memcpy_s(sensor_als_data_ptr->als_channel, sizeof(sensor_als_data_ptr->als_channel),
		display_als_data_ptr->als_channel, sizeof(display_als_data_ptr->als_channel));
}

static uint32_t  g_last_brightness = 0;

void matting_algo_clear_last_print_brightness(void)
{
	g_last_brightness = 0;
}

static void print_als_data(struct display_als_data_info_st *als_info,
	struct sensor_hub_als_data_st *sensor_als_data, uint32_t *als_ch_noise,
	uint8_t noise_count, uint32_t brightness)
{
	if (brightness != g_last_brightness) {
		hwlog_info("before-ch[%u %u %u %u %u] last-bl=%d before-lux=%d als-gain[%u %u %u %u %u]\n",
			als_info->als_channel[INDEX_ZERO], als_info->als_channel[INDEX_ONE], als_info->als_channel[INDEX_TWO],
			als_info->als_channel[INDEX_THREE], als_info->als_channel[INDEX_FOUR], g_last_brightness, als_info->als_lux,
			als_info->als_gain[INDEX_ZERO], als_info->als_gain[INDEX_ONE], als_info->als_gain[INDEX_TWO],
			als_info->als_gain[INDEX_THREE], als_info->als_gain[INDEX_FOUR]);

		hwlog_info("after-ch[%u %u %u %u %u] noise[%u %u %u %u %u] after-lux=%u brightness=%d\n",
			sensor_als_data->als_channel[INDEX_ZERO], sensor_als_data->als_channel[INDEX_ONE],
			sensor_als_data->als_channel[INDEX_TWO], sensor_als_data->als_channel[INDEX_THREE],
			sensor_als_data->als_channel[INDEX_FOUR], als_ch_noise[INDEX_ZERO],
			als_ch_noise[INDEX_ONE], als_ch_noise[INDEX_TWO], als_ch_noise[INDEX_THREE],
			als_ch_noise[INDEX_FOUR], sensor_als_data->als_lux, brightness);
	}
}

static void check_print_als_data(struct display_als_data_info_st *als_info,
	struct sensor_hub_als_data_st *sensor_als_data, uint32_t *als_ch_noise, uint8_t noise_count)
{
	uint32_t lux_diff = abs(als_info->als_lux - sensor_als_data->als_lux);
	uint32_t lux_diff_outdoor_max = als_info->als_lux * LUX_VALUE_ALLOWABLE_PERCENT / LUX_VALUE_FULL_PERCENT;
	uint32_t brightness = matting_algo_get_current_brightness();
	if (als_info->als_lux <= LUX_VALUE_INDOOR) {
		if (lux_diff > LUX_VALUE_DIFF)
			print_als_data(als_info, sensor_als_data, als_ch_noise, noise_count, brightness);
	} else {
		if (lux_diff > lux_diff_outdoor_max)
			print_als_data(als_info, sensor_als_data, als_ch_noise, noise_count, brightness);
	}
	g_last_brightness = brightness;
}

static int32_t lcd_als_value_minus_noise(struct display_als_data_info_st *als_info,
	uint32_t *als_ch_noise, uint8_t noise_count, uint32_t *lux)
{
	int32_t ret;
	bool is_noise_zero;
	struct sensor_hub_als_data_st sensor_hub_als_data = { 0 };
	struct alg_matting_data_param_info_st *data_param_info = matting_algo_get_data_param_info();

	is_noise_zero = lcd_is_screen_nosie_zero(als_ch_noise, noise_count);
	if (is_noise_zero == false) {
		get_sensor_hub_als_data(als_info, &sensor_hub_als_data);
		lcd_als_channel_minus_noise(als_info, als_ch_noise, noise_count, &sensor_hub_als_data);
		ret = lcd_get_alx_lux_value(data_param_info->als_type_e, &sensor_hub_als_data);
		if (ret == EC_FAILURE) {
			hwlog_err("[%s] als_type=%d\n", __func__, data_param_info->als_type_e);
			return EC_FAILURE;
		}

		*lux = sensor_hub_als_data.als_lux;
		check_print_als_data(als_info, &sensor_hub_als_data, als_ch_noise, noise_count);
	}

	return EC_SUCCESS;
}

struct display_data_info_st g_display_cut_out_info = {0};
struct display_data_info_st g_calc_cut_out_info = {0};
bool g_need_get_display_cut_out_info = true;

void matting_algo_clear_calc_cut_out_info(void)
{
	memset_s(&g_calc_cut_out_info, sizeof(g_calc_cut_out_info), 0, sizeof(g_calc_cut_out_info));
}

void lcd_calc_screen_noise(uint8_t *fb_start_addr)
{
	uint32_t brightness = 0;
	int32_t ret;
	int32_t num;
	struct alg_matting_data_param_info_st *data_param_info = matting_algo_get_data_param_info();
	int32_t cut_out_index;

	cut_out_index = matting_algo_get_cut_out_index();
	if (cut_out_index < 0) {
		hwlog_debug("[%s] cut_out_index < 0\n", __func__);
		return;
	}

	if (fb_start_addr == NULL) {
		hwlog_err("[%s] fb_start_addr param is NULL, no need calc noise\n", __func__);
		return;
	}

	brightness = matting_algo_get_current_brightness();
	if (brightness == 0) {
		hwlog_err("[%s] brightness=0 no nedd process\n", __func__);
		return;
	}

	g_display_cut_out_info.frame_timestamp[cut_out_index % FRAME_MAX] = get_millisec_timestamp();
	g_display_cut_out_info.frame_num = cut_out_index % FRAME_MAX + 1;
	g_display_cut_out_info.blightness = brightness;
	g_display_cut_out_info.pixel_line_offset = data_param_info->fov_width * RGB_CURVE;
	g_display_cut_out_info.pixel_data = (uint32_t *)fb_start_addr;

	ret = alg_mat_oneframe_noise_calc(&g_display_cut_out_info);
	if (ret != EC_SUCCESS) {
		hwlog_err("[%s] call alg_mat_oneframe_noise_calc failed, need check!\n", __func__);
		return;
	}

	cut_out_index++;
	matting_algo_set_cut_out_index(cut_out_index);

	if ((cut_out_index > FRAME_MAX) && (cut_out_index % FRAME_MAX == 1)) {
		// when frame num bigger than FRAME_MAX, need discards the previous, around again
		num = cut_out_index;
		matting_algo_set_start_time(get_millisec_timestamp());
		matting_algo_set_cut_out_index(1);

		if (matting_algo_debug_enable())
			hwlog_info("[%s] frame_num=%d, FRAME_MAX=%d\n", __func__, num, FRAME_MAX);
	}

	if (g_need_get_display_cut_out_info)
		memcpy_s(&g_calc_cut_out_info, sizeof(struct display_data_info_st),
			&g_display_cut_out_info, sizeof(struct display_data_info_st));
}

uint32_t lcd_dest_als_value_calc(struct display_als_data_info_st *als_data)
{
	int32_t ret;
	uint32_t als_chn_noise[SENSORMGR_ALS_CHANNEL_MAX] = {0};
	uint32_t lux = als_data->als_lux;
	uint32_t *final_channel = NULL;
	uint8_t als_chn_noise_count = SENSORMGR_ALS_CHANNEL_MAX;
	struct display_data_info_st calc_cut_out_info = {0};

	g_need_get_display_cut_out_info = false;
	if (matting_algo_debug_enable())
		memcpy_s(&calc_cut_out_info, sizeof(struct display_data_info_st),
			&g_calc_cut_out_info, sizeof(struct display_data_info_st));

	ret = alg_mat_all_frame_noise_calc(&g_calc_cut_out_info, als_data->timestamp_start,
		als_data->timestamp_end, als_chn_noise, als_chn_noise_count);
	if (ret != EC_SUCCESS)
		hwlog_err("[%s] alg_mat_all_frame_noise_calc return err\n", __func__);

	if (g_calc_cut_out_info.frame_num > 1) {
		final_channel = (uint32_t *)(g_calc_cut_out_info.als_channel_noise +
			g_calc_cut_out_info.frame_num - 1);
		if (memcpy_s(g_calc_cut_out_info.als_channel_noise,
			sizeof(g_calc_cut_out_info.als_channel_noise), final_channel,
			SENSORMGR_ALS_CHANNEL_MAX * sizeof(uint32_t)) != EOK)
			hwlog_err("[%s] memcpy_s err\n", __func__);

		g_calc_cut_out_info.frame_num = 1;
		matting_algo_set_cut_out_index(1);
	}

	ret = lcd_als_value_minus_noise(als_data, als_chn_noise, als_chn_noise_count, &lux);
	if (ret != EC_SUCCESS)
		hwlog_err("[%s] lcd_als_value_minus_noise err\n", __func__);

	if (matting_algo_debug_enable()) {
		hwlog_info("[statics] scope=%lld, %llu, %llu, %d, [0]=%llu\n",
			(als_data->timestamp_end - als_data->timestamp_start), als_data->timestamp_start,
			als_data->timestamp_end, calc_cut_out_info.frame_num,
			calc_cut_out_info.frame_timestamp[INDEX_ZERO]);
		hwlog_info("[statics] [1-4]=%llu, %llu, %llu, %llu\n",
			calc_cut_out_info.frame_timestamp[INDEX_ONE], calc_cut_out_info.frame_timestamp[INDEX_TWO],
			calc_cut_out_info.frame_timestamp[INDEX_THREE], calc_cut_out_info.frame_timestamp[INDEX_FOUR]);
		hwlog_info("[statics] [5-8]=%llu, %llu, %llu, %llu\n",
			calc_cut_out_info.frame_timestamp[INDEX_FIVE], calc_cut_out_info.frame_timestamp[INDEX_SIX],
			calc_cut_out_info.frame_timestamp[INDEX_SEVEN], calc_cut_out_info.frame_timestamp[INDEX_EIGHT]);
	}
	g_need_get_display_cut_out_info = true;

	return lux;
}