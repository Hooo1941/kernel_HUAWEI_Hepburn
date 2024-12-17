/* SPDX-License-Identifier: GPL-2.0
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022.
 * All rights reserved.
 * Description: Noise Calculation and Denoising Algorithm Task
 * Author: yangxiaopeng@huawei.com
 * Create: 2022-11-24
 */

#ifndef IMAGE_COMPOSITING_ALGO_H
#define IMAGE_COMPOSITING_ALGO_H
#include "als_matting_algo_info.h"
#include "als_matting_algo.h"

#define IMAGE_LAYER_MAX_NUM 4
#define IMAGE_LAYER_MAX_LENGTH DIS_ALS_CUTOUT_VALUE
#define IMAGE_LAYER_MAX_WIDTH DIS_ALS_CUTOUT_VALUE
#define IMAGE_PIXEL_MAX_WIDTH 4
#define IMAGE_RGB_PIXEL_MAX_WIDTH RGB_CURVE
#define IMAGE_PIXEL_MAX_VALUE (255)

#define COMPOSITED_FRAME_RGB_BUF_LEN (IMAGE_LAYER_MAX_LENGTH * \
	IMAGE_LAYER_MAX_WIDTH * IMAGE_RGB_PIXEL_MAX_WIDTH)

#define BIT_NUM_PER_BYTE 8

// Frame Layer Priority
struct frame_layer_info_st {
	const uint8_t plane_id;
	const uint8_t priority;
};

struct frame_lyer_priority_st {
	struct frame_layer_info_st layerinfo[IMAGE_LAYER_MAX_NUM];
};

// pixel format
enum matting_algo_pixel_format {
	PIX_FMT_GRAY8,
	PIX_FMT_BGR888,
	PIX_FMT_GRAY16,
	PIX_FMT_FLOAT32,
	PIX_FMT_IRDEPTH,
	/* YUV420P */
	PIX_FMT_I420,
	PIX_FMT_NV12,
	PIX_FMT_NV21,
	PIX_FMT_BGRA8888,
	PIX_FMT_RGBA8888,
	PIX_FMT_RGB888,
	PIX_FMT_RGB565,
	PIX_FMT_ABGR8888,
};

struct display_framebuffer_info_st {
	/* Total number of layers in the current frame */
	uint8_t  fb_num;
	/* 0->layer49, 1->layer64, 2->layer67, 3->layer71 */
	uint8_t  *fb_start_addr[IMAGE_LAYER_MAX_NUM];
	/* Total length of a single framebuffer in bytes */
	uint32_t fb_length;
	/* Number of bits occupied by each pixel */
	uint32_t fb_bit_per_pixel;
	/* The number of bytes in a row of pixels may be 32-bit or 64-bit
	 * aligned. Therefore, the number of bytes may be several more than
	 * the actualpixel bytes. For example, if the number of wide pixels
	 * is 768,the number of bytes corresponding to the row of pixels is
	 * greater thanor equal to 768 x 4, because 32- or 64-byte alignment
	 * needs to be considered.
	 */
	uint32_t fb_byte_width;
	/* Number of Wide Pixels */
	uint32_t fb_width;
	/* High pixel count */
	uint32_t fb_height;
	/* Image Format */
	enum matting_algo_pixel_format pixel_format;
};

struct compositing_layer_info_st {
	uint8_t   value_r;
	uint8_t   value_g;
	uint8_t   value_b;
	uint8_t   value_a;
	uint32_t  alpha;
	uint32_t  delta;
	uint32_t  percentage;
};

struct sensor_hub_als_data_st {
	uint32_t als_lux;
	float uv_index;
	uint32_t als_channel[SENSORMGR_ALS_CHANNEL_MAX];
	uint16_t als_again[SENSORMGR_ALS_CHANNEL_MAX];
	uint16_t als_atime;
};

struct image_compositing_param_info {
	uint32_t orig_line_offset;
	uint32_t com_rgb_frm_buf_offset;
	uint32_t line_pixel_max_width;
};

int matting_algo_get_fb_info(struct display_framebuffer_info_st *fb_info_input);
uint8_t *image_compositing(void);
struct display_framebuffer_info_st *matting_algo_get_display_frame_buf_info(void);
const struct frame_lyer_priority_st *matting_algo_get_const_frame_priority(void);
#endif