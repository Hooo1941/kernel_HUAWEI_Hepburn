// SPDX-License-Identifier: GPL-2.0
/* Copyright (c) Huawei Technologies Co., Ltd. 2022-2022.
 * All rights reserved.
 * Description: Noise Calculation and Denoising Algorithm Task
 * Author: yangxiaopeng@huawei.com
 * Create: 2022-11-24
 */
#include <linux/semaphore.h>
#include <securec.h>
#include "image_compositing_algo.h"
#include "matting_algo_task.h"
#include "als_matting_algo_info.h"
#include <huawei_platform/log/hw_log.h>
#include "math_calculation.h"

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif

#define HWLOG_TAG matting_algo_task
HWLOG_REGIST();

#define MAX_PRINT_BUFFER_LEN (512)
#define OFFSET_ONE (1)
#define OFFSET_TWO (2)

struct display_framebuffer_info_st g_display_frame_buf_info = {0};

/*
 * priority[0]->layer64
 * priority[1]->layer67
 * priority[2]->layer49
 * priority[3]->layer71
 */
const struct frame_lyer_priority_st g_const_frame_priority = {
	.layerinfo = {{64, 0}, {67, 1}, {49, 2}, {71, 3}},
};

const struct frame_lyer_priority_st *matting_algo_get_const_frame_priority(void)
{
	return &g_const_frame_priority;
}

struct display_framebuffer_info_st *matting_algo_get_display_frame_buf_info(void)
{
	return &g_display_frame_buf_info;
}

int matting_algo_get_fb_info(struct display_framebuffer_info_st *fb_info_input)
{
	int i;
	char strlog[MAX_PRINT_BUFFER_LEN];
	const struct frame_lyer_priority_st *const_frame_priority = matting_algo_get_const_frame_priority();

	/* If the parameter transferred from the MCU is invalid, cropping is not started. */
	if (!get_alg_matting_data_param_valid()) {
		// Recovering crop state
		set_frame_crop_state(true);
		return EC_FAILURE;
	}

	g_display_frame_buf_info.fb_num = fb_info_input->fb_num;

	// before change: index0->layer64, 1->layer67, 2->layer49, 3->layer71
	// after change:  index0->layer64, 1->layer67, 2->layer49, 3->layer71
	for (i = 0; i < IMAGE_LAYER_MAX_NUM; i++)
		g_display_frame_buf_info.fb_start_addr[const_frame_priority->layerinfo[i].priority] = \
			fb_info_input->fb_start_addr[i];

	g_display_frame_buf_info.fb_length = fb_info_input->fb_length;
	g_display_frame_buf_info.fb_bit_per_pixel = fb_info_input->fb_bit_per_pixel;
	g_display_frame_buf_info.fb_byte_width = fb_info_input->fb_byte_width;
	g_display_frame_buf_info.fb_width = fb_info_input->fb_width;
	g_display_frame_buf_info.fb_height = fb_info_input->fb_height;
	g_display_frame_buf_info.pixel_format = fb_info_input->pixel_format;

	if (matting_algo_debug_enable()) {
		hwlog_info("[%s] fb_num=%d, fb_start_addr=0x%p, 0x%p, 0x%p, 0x%p\n", __func__,
			g_display_frame_buf_info.fb_num,
			g_display_frame_buf_info.fb_start_addr[INDEX_ZERO],
			g_display_frame_buf_info.fb_start_addr[INDEX_ONE],
			g_display_frame_buf_info.fb_start_addr[INDEX_TWO],
			g_display_frame_buf_info.fb_start_addr[INDEX_THREE]);

		memset_s(strlog, sizeof(strlog), '\0', sizeof(strlog));
		snprintf_s(strlog, sizeof(strlog), sizeof(strlog) - 1,
			"fb_length=%d, %d, %d, %d, %d, %d",
			g_display_frame_buf_info.fb_length,
			g_display_frame_buf_info.fb_bit_per_pixel,
			g_display_frame_buf_info.fb_byte_width,
			g_display_frame_buf_info.fb_width,
			g_display_frame_buf_info.fb_height,
			g_display_frame_buf_info.pixel_format);
		hwlog_info("[%s] %s\n", __func__, strlog);
	}

	up(matting_algo_get_semaphore_frame_crop());

	return EC_SUCCESS;
}
EXPORT_SYMBOL(matting_algo_get_fb_info);

uint8_t g_composited_rgb_frame_buffer[COMPOSITED_FRAME_RGB_BUF_LEN] = {0};

static int image_pixel_order_change_to_rgba_index(uint8_t *index)
{
	struct display_framebuffer_info_st *display_frame_buf_info = matting_algo_get_display_frame_buf_info();
	switch (display_frame_buf_info->pixel_format) {
	case PIX_FMT_BGRA8888:
		index[INDEX_ZERO] = INDEX_TWO;
		index[INDEX_ONE] = INDEX_ONE;
		index[INDEX_TWO] = INDEX_ZERO;
		index[INDEX_THREE] = INDEX_THREE;
		break;
	case PIX_FMT_RGBA8888:
		index[INDEX_ZERO] = INDEX_ZERO;
		index[INDEX_ONE] = INDEX_ONE;
		index[INDEX_TWO] = INDEX_TWO;
		index[INDEX_THREE] = INDEX_THREE;
		break;
	case PIX_FMT_ABGR8888:
		index[INDEX_ZERO] = INDEX_THREE;
		index[INDEX_ONE] = INDEX_TWO;
		index[INDEX_TWO] = INDEX_ONE;
		index[INDEX_THREE] = INDEX_ZERO;
		break;
	case PIX_FMT_BGR888:
		index[INDEX_ZERO] = INDEX_TWO;
		index[INDEX_ONE] = INDEX_ONE;
		index[INDEX_TWO] = INDEX_ZERO;
		index[INDEX_THREE] = INDEX_THREE;
		break;
	case PIX_FMT_RGB888:
		index[INDEX_ZERO] = INDEX_ZERO;
		index[INDEX_ONE] = INDEX_ONE;
		index[INDEX_TWO] = INDEX_TWO;
		index[INDEX_THREE] = INDEX_THREE;
		break;
	default:
		hwlog_err("[%s] pixel_format=%d not support, please check!\n",
			__func__, display_frame_buf_info->pixel_format);
		return EC_FAILURE;
	}

	return EC_SUCCESS;
}

static uint8_t *image_compositing_one_layer(uint8_t *input_index,
	struct alg_matting_data_param_info_st *data_param_info_ptr,
	struct display_framebuffer_info_st *display_frame_buf_info_ptr,
	struct image_compositing_param_info *img_comp_param_info_ptr)
{
	uint32_t comp_line_pixel_cnt;
	uint32_t comp_line_offset;
	uint32_t current_line_offset;
	uint8_t *pixel_start_addr = NULL;
	uint32_t i;
	uint32_t j;
	uint8_t value;

	for (i = 0; i < IMAGE_LAYER_MAX_LENGTH; i++) {
		comp_line_pixel_cnt = 0;
		comp_line_offset = i * img_comp_param_info_ptr->com_rgb_frm_buf_offset;
		current_line_offset = img_comp_param_info_ptr->orig_line_offset +
			display_frame_buf_info_ptr->fb_byte_width * i;
		pixel_start_addr = (uint8_t *)(display_frame_buf_info_ptr->fb_start_addr[0] + current_line_offset);
		for (j = 0; j < img_comp_param_info_ptr->line_pixel_max_width; j += IMAGE_RGB_PIXEL_MAX_WIDTH) {
			value = *((uint8_t *)(pixel_start_addr + j + input_index[INDEX_ZERO]));
			g_composited_rgb_frame_buffer[comp_line_offset + comp_line_pixel_cnt] = value;
			value = *((uint8_t *)(pixel_start_addr + j + input_index[INDEX_ONE]));
			g_composited_rgb_frame_buffer[comp_line_offset + comp_line_pixel_cnt + OFFSET_ONE] = value;
			value = *((uint8_t *)(pixel_start_addr + j + input_index[INDEX_TWO]));
			g_composited_rgb_frame_buffer[comp_line_offset + comp_line_pixel_cnt + OFFSET_TWO] = value;
			comp_line_pixel_cnt += IMAGE_RGB_PIXEL_MAX_WIDTH;
		}
	}

	return g_composited_rgb_frame_buffer;
}

static void get_pixel_rgba_value_info(struct compositing_layer_info_st *comp_layer_info_ptr,
	uint32_t current_line_offset, uint32_t offset_j, uint8_t *input_index,
	int32_t *current_value_a, int32_t *previout_value_a)
{
	/* Obtain a pixel at a corresponding position of each frame of image */
	uint32_t layer_num;
	uint32_t n;
	uint8_t *pixel_start_addr = NULL;
	struct display_framebuffer_info_st *display_frame_buf_info = matting_algo_get_display_frame_buf_info();

	for (layer_num = 0; layer_num < display_frame_buf_info->fb_num; layer_num++) {
		pixel_start_addr = (uint8_t *)(display_frame_buf_info->fb_start_addr[layer_num] + current_line_offset);
		comp_layer_info_ptr[layer_num].value_r = *((uint8_t *)(pixel_start_addr + offset_j +
			input_index[INDEX_ZERO]));
		comp_layer_info_ptr[layer_num].value_g = *((uint8_t *)(pixel_start_addr + offset_j +
			input_index[INDEX_ONE]));
		comp_layer_info_ptr[layer_num].value_b = *((uint8_t *)(pixel_start_addr + offset_j +
			input_index[INDEX_TWO]));
		comp_layer_info_ptr[layer_num].value_a = *((uint8_t *)(pixel_start_addr + offset_j +
			input_index[INDEX_THREE]));

		current_value_a[layer_num] = comp_layer_info_ptr[layer_num].value_a;
		if (current_value_a[layer_num] != previout_value_a[layer_num]) {
			// alpha in [0, 255]
			comp_layer_info_ptr[layer_num].alpha = math_div64_32(
				comp_layer_info_ptr[layer_num].value_a * DEFAULT_MULTIPLES,
				IMAGE_PIXEL_MAX_VALUE, NULL);
			comp_layer_info_ptr[layer_num].delta = DEFAULT_MULTIPLES - comp_layer_info_ptr[layer_num].alpha;
		}
		previout_value_a[layer_num] = current_value_a[layer_num];
	}

	/* Calculate the proportion of pixels in each frame of image */
	for (layer_num = 0; layer_num < display_frame_buf_info->fb_num; layer_num++) {
		if (layer_num == 0)
			comp_layer_info_ptr[layer_num].percentage = DEFAULT_MULTIPLES;
		else
			comp_layer_info_ptr[layer_num].percentage = comp_layer_info_ptr[layer_num].alpha;

		for (n = layer_num + 1; n < display_frame_buf_info->fb_num; n++) {
			comp_layer_info_ptr[layer_num].percentage *= comp_layer_info_ptr[n].delta;
			comp_layer_info_ptr[layer_num].percentage = math_div64_32(comp_layer_info_ptr[layer_num].percentage,
				DEFAULT_MULTIPLES, NULL);
		}
	}
}

static void composited_every_pixel(struct compositing_layer_info_st *comp_layer_info_ptr,
	struct display_framebuffer_info_st *display_frame_buf_info,
	uint32_t comp_line_offset, uint32_t comp_line_pixel_cnt)
{
	uint32_t r = 0;
	uint32_t g = 0;
	uint32_t b = 0;
	uint32_t layer_num;

	/* image synthesis */
	for (layer_num = 0; layer_num < display_frame_buf_info->fb_num; layer_num++) {
		r += comp_layer_info_ptr[layer_num].value_r * comp_layer_info_ptr[layer_num].percentage;
		g += comp_layer_info_ptr[layer_num].value_g * comp_layer_info_ptr[layer_num].percentage;
		b += comp_layer_info_ptr[layer_num].value_b * comp_layer_info_ptr[layer_num].percentage;
	}
	r = math_div64_32(r, DEFAULT_MULTIPLES, NULL);
	g = math_div64_32(g, DEFAULT_MULTIPLES, NULL);
	b = math_div64_32(b, DEFAULT_MULTIPLES, NULL);
	g_composited_rgb_frame_buffer[comp_line_offset + comp_line_pixel_cnt] = r;
	g_composited_rgb_frame_buffer[comp_line_offset + comp_line_pixel_cnt + OFFSET_ONE] = g;
	g_composited_rgb_frame_buffer[comp_line_offset + comp_line_pixel_cnt + OFFSET_TWO] = b;

	if (r > IMAGE_PIXEL_MAX_VALUE || g > IMAGE_PIXEL_MAX_VALUE || b > IMAGE_PIXEL_MAX_VALUE)
		hwlog_err("[%s] r=%d g=%d b=%d error, please check!\n", __func__, r, g, b);
}

static uint8_t *image_compositing_all_layer(uint8_t *input_index,
	struct alg_matting_data_param_info_st *data_param_info_ptr,
	struct display_framebuffer_info_st *display_frame_buf_info_ptr,
	struct image_compositing_param_info *img_comp_param_info_ptr)
{
	int32_t  current_value_a[IMAGE_LAYER_MAX_NUM] = {-1, -1, -1, -1};
	int32_t  previout_value_a[IMAGE_LAYER_MAX_NUM] = {-1, -1, -1, -1};
	uint32_t i;
	uint32_t j;
	uint32_t comp_line_pixel_cnt;
	uint32_t comp_line_offset;
	uint32_t current_line_offset;
	struct compositing_layer_info_st comp_layer_info[IMAGE_LAYER_MAX_NUM];

	memset_s(comp_layer_info, sizeof(comp_layer_info), 0, sizeof(comp_layer_info));

	for (i = 0; i < data_param_info_ptr->fov_height; i++) {
		comp_line_pixel_cnt = 0;
		comp_line_offset = i * img_comp_param_info_ptr->com_rgb_frm_buf_offset;
		current_line_offset = img_comp_param_info_ptr->orig_line_offset +
			display_frame_buf_info_ptr->fb_byte_width * i;
		for (j = 0; j < img_comp_param_info_ptr->line_pixel_max_width; j += IMAGE_PIXEL_MAX_WIDTH) {
			get_pixel_rgba_value_info(comp_layer_info, current_line_offset, j, input_index,
				current_value_a, previout_value_a);

			composited_every_pixel(comp_layer_info, display_frame_buf_info_ptr,
				comp_line_offset, comp_line_pixel_cnt);

			comp_line_pixel_cnt += IMAGE_RGB_PIXEL_MAX_WIDTH;
		}
	}

	return g_composited_rgb_frame_buffer;
}

static int32_t check_frame_buf_info(void)
{
	uint32_t i;
	struct alg_matting_data_param_info_st *data_param_info = matting_algo_get_data_param_info();
	struct display_framebuffer_info_st *display_frame_buf_info = matting_algo_get_display_frame_buf_info();

	if (display_frame_buf_info->fb_num <= 0 || display_frame_buf_info->fb_num > IMAGE_LAYER_MAX_NUM) {
		hwlog_err("[%s] fb_num=%d error!\n", __func__, display_frame_buf_info->fb_num);
		return EC_FAILURE;
	}

	for (i = 0; i < display_frame_buf_info->fb_num; i++) {
		if (display_frame_buf_info->fb_start_addr[i] == NULL) {
			hwlog_err("[%s] fb_start_addr[%d] is NULL!\n", __func__, i);
			return EC_FAILURE;
		}
	}

	if ((display_frame_buf_info->pixel_format == PIX_FMT_BGR888 ||
		 display_frame_buf_info->pixel_format == PIX_FMT_RGB888) &&
		(display_frame_buf_info->fb_num != 1)) {
		hwlog_err("[%s] fb_num=%d, pixel_format=%d error!\n", __func__, display_frame_buf_info->fb_num,
			display_frame_buf_info->pixel_format);
		return EC_FAILURE;
	}

	if ((display_frame_buf_info->fb_width < (data_param_info->offset_x + data_param_info->fov_width)) || \
		(display_frame_buf_info->fb_height < (data_param_info->offset_y + data_param_info->fov_height))) {
		hwlog_err("[%s] fb_width=%d, fb_height=%d error, expected witdh>=%d, height>=%d\n", __func__,
			display_frame_buf_info->fb_width, display_frame_buf_info->fb_height,
			(data_param_info->offset_x + data_param_info->fov_width),
			(data_param_info->offset_y + data_param_info->fov_height));
		return EC_FAILURE;
	}

	return EC_SUCCESS;
}

uint8_t *image_compositing(void)
{
	int32_t ret;
	uint8_t rgba_index[IMAGE_PIXEL_MAX_WIDTH] = {0};
	uint8_t *buffer = NULL;
	struct image_compositing_param_info img_comp_param_info = {0};
	struct alg_matting_data_param_info_st *data_param_info = matting_algo_get_data_param_info();
	struct display_framebuffer_info_st *display_frame_buf_info = matting_algo_get_display_frame_buf_info();

	ret = check_frame_buf_info();
	if (ret < 0) {
		hwlog_err("[%s] call check_frame_buf_info failed!\n", __func__);
		return NULL;
	}

	ret = image_pixel_order_change_to_rgba_index(rgba_index);
	if (ret < 0) {
		hwlog_err("[%s] call image_pixel_order_change_to_rgba_index failed!\n", __func__);
		return NULL;
	}

	img_comp_param_info.orig_line_offset =  display_frame_buf_info->fb_byte_width *
		data_param_info->offset_y +
		display_frame_buf_info->fb_bit_per_pixel / BIT_NUM_PER_BYTE *
		data_param_info->offset_x;
	img_comp_param_info.com_rgb_frm_buf_offset = data_param_info->fov_width * IMAGE_RGB_PIXEL_MAX_WIDTH;
	img_comp_param_info.line_pixel_max_width = display_frame_buf_info->fb_bit_per_pixel / BIT_NUM_PER_BYTE *
		data_param_info->fov_width;

	switch (display_frame_buf_info->pixel_format) {
	case PIX_FMT_BGRA8888:
	// fall-through
	case PIX_FMT_RGBA8888:
	// fall-through
	case PIX_FMT_ABGR8888:
		buffer = image_compositing_all_layer(rgba_index, data_param_info,
			display_frame_buf_info, &img_comp_param_info);
		break;
	case PIX_FMT_BGR888:
	// fall-through
	case PIX_FMT_RGB888:
		buffer = image_compositing_one_layer(rgba_index, data_param_info,
			display_frame_buf_info, &img_comp_param_info);
		break;
	default:
		hwlog_err("[%s] not support pixel_format=%d\n", __func__, display_frame_buf_info->pixel_format);
		break;
	}

	return buffer;
}
