/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <linux/types.h>
#include <dpu/soc_dpu_define.h>
#include "dpu_comp_mgr.h"
#include "dpu_hiace_init.h"

#define max_value(x, y) (((x) > (y)) ? (x) : (y))

#define X_PARTITION 6

#define COLOR_SIGMA0 (0x1f & 0x1f) // -1
#define COLOR_SIGMA1 (0x1f & 0x1f) // -1
#define COLOR_SIGMA2 (0x1e & 0x1f) // -2
#define COLOR_SIGMA3 (0x1e & 0x1f) // -2
#define COLOR_SIGMA4 (0x1e & 0x1f) // -2
#define COLOR_SIGMA5 (0x1e & 0x1f) // -2

#define MIN_SIGMA (0x16 & 0x1f) // -10
#define MAX_SIGMA (0x1f & 0x1f) // -1

struct hiace_color_sigma {
	uint32_t s3_green_sigma03;
	uint32_t s3_green_sigma45;
	uint32_t s3_red_sigma03;
	uint32_t s3_red_sigma45;
	uint32_t s3_blue_sigma03;
	uint32_t s3_blue_sigma45;
	uint32_t s3_white_sigma03;
	uint32_t s3_white_sigma45;
};

static uint32_t get_fixed_point_offset(uint32_t half_block_size)
{
	uint32_t num = 2;
	uint32_t len = 2;

	while (len < half_block_size) {
		num++;
		len <<= 1;
	}
	return num;
}

static void dpu_set_lhist_sft(char __iomem *hiace_base, uint32_t half_block_w, uint32_t half_block_h)
{
	uint32_t lhist_sft;
	uint32_t block_pixel_num;
	uint32_t max_lhist_block_pixel_num;
	uint32_t max_lhist_bin_reg_num;

	block_pixel_num = (half_block_w * half_block_h) << 2;
	max_lhist_block_pixel_num = block_pixel_num << 2;
	max_lhist_bin_reg_num = (1 << 16) - 1;
	if (max_lhist_block_pixel_num < (max_lhist_bin_reg_num))
		lhist_sft = 0;
	else if (max_lhist_block_pixel_num < (max_lhist_bin_reg_num << 1))
		lhist_sft = 1;
	else if (max_lhist_block_pixel_num < (max_lhist_bin_reg_num << 2))
		lhist_sft = 2;
	else if (max_lhist_block_pixel_num < (max_lhist_bin_reg_num << 3))
		lhist_sft = 3;
	else
		lhist_sft = 4;

	set_reg(DPU_HIACE_LHIST_SFT_ADDR(hiace_base), lhist_sft, 3, 0);
}

static void dpu_set_xyweight(char __iomem *hiace_base, uint32_t half_block_w, uint32_t half_block_h)
{
	uint32_t xyweight;
	uint32_t fixbit_x;
	uint32_t fixbit_y;
	uint32_t reciprocal_x;
	uint32_t reciprocal_y;

	fixbit_x = get_fixed_point_offset(half_block_w) & 0x1f;
	fixbit_y = get_fixed_point_offset(half_block_h) & 0x1f;
	reciprocal_x = ((1U << (fixbit_x + 8)) / (2 * max_value(half_block_w, 1))) & 0x3ff;
	reciprocal_y = ((1U << (fixbit_y + 8)) / (2 * max_value(half_block_h, 1))) & 0x3ff;
	xyweight = (fixbit_y << 26) | (reciprocal_y << 16) | (fixbit_x << 10) | reciprocal_x;
	set_reg(DPU_HIACE_XYWEIGHT_ADDR(hiace_base), xyweight, 32, 0);
}

static void dpu_set_size_info(struct dpu_composer *dpu_comp)
{
	uint32_t width;
	uint32_t height;
	uint32_t half_block_w;
	uint32_t half_block_h;
	char __iomem *hiace_base = NULL;

	hiace_base  = dpu_comp->comp_mgr->dpu_base + dpu_comp->hiace_ctrl->dpp_offset + HIACE_OFFSET;
	width = dpu_comp->conn_info->base.xres & 0x1fff;
	height = dpu_comp->conn_info->base.yres & 0x1fff;
	set_reg(DPU_HIACE_IMAGE_INFO_ADDR(hiace_base),  (height << 16) | width, 32, 0);
	set_reg(DPU_HIACE_LHIST_EN_ADDR(hiace_base), 0, 1, 10);

	half_block_w = (width / (2 * X_PARTITION)) & 0x1ff;
	half_block_h = ((height + 11) / 12) & 0x1ff;
	set_reg(DPU_HIACE_HALF_BLOCK_INFO_ADDR(hiace_base), (half_block_h << 16) | half_block_w, 32, 0);

	dpu_set_lhist_sft(hiace_base, half_block_w, half_block_h);
	dpu_set_xyweight(hiace_base, half_block_w, half_block_h);
}

static void dpu_set_skin_protection_info(char __iomem *hiace_base)
{
	uint32_t slop;
	uint32_t th_max;
	uint32_t th_min;
	uint32_t hue;
	uint32_t value;
	uint32_t saturation;

	slop = 68 & 0xff;
	th_min = 0 & 0x1ff;
	th_max = 30 & 0x1ff;
	hue = (slop << 24) | (th_max << 12) | th_min;
	set_reg(DPU_HIACE_HUE_ADDR(hiace_base), hue, 32, 0);

	th_min = 80 & 0xff;
	th_max = 140 & 0xff;
	saturation = (slop << 24) | (th_max << 12) | th_min;
	set_reg(DPU_HIACE_SATURATION_ADDR(hiace_base), saturation, 32, 0);

	th_min = 100 & 0xff;
	th_max = 255 & 0xff;
	value = (slop << 24) | (th_max << 12) | th_min;
	set_reg(DPU_HIACE_VALUE_ADDR(hiace_base), value, 32, 0);

	/* SDR:128;  HDR:0 */
	set_reg(DPU_HIACE_SKIN_GAIN_ADDR(hiace_base), 128, 8, 0);
}

static void dpu_nr_set_somebrightness(char __iomem *nr_base)
{
	uint32_t somebrightness0 = 819 & 0x3ff;
	uint32_t somebrightness1 = 640 & 0x3ff;
	uint32_t somebrightness2 = 384 & 0x3ff;
	uint32_t somebrightness3 = 205 & 0x3ff;
	uint32_t somebrightness4 = 96 & 0x3ff;

	/* disable noisereduction */
	set_reg(DPU_HIACE_BYPASS_NR_ADDR(nr_base), 0x1, 1, 0);
	set_reg(DPU_HIACE_S3_SOME_BRIGHTNESS01_ADDR(nr_base),
		somebrightness0 | (somebrightness1 << 16), 32, 0);
	set_reg(DPU_HIACE_S3_SOME_BRIGHTNESS23_ADDR(nr_base),
		somebrightness2 | (somebrightness3 << 16), 32, 0);
	set_reg(DPU_HIACE_S3_SOME_BRIGHTNESS4_ADDR(nr_base), somebrightness4, 32, 0);
}

static void dpu_nr_set_color_sigma(char __iomem *hiace_base)
{
	struct hiace_color_sigma color_sigma;

	set_reg(DPU_HIACE_S3_MIN_MAX_SIGMA_ADDR(hiace_base), MIN_SIGMA | (MAX_SIGMA << 16), 32, 0);

	color_sigma.s3_green_sigma03 = COLOR_SIGMA0 | (COLOR_SIGMA0 << 8) | (COLOR_SIGMA2 << 16) | (COLOR_SIGMA3 << 24);
	color_sigma.s3_red_sigma03 =  color_sigma.s3_green_sigma03;
	color_sigma.s3_blue_sigma03 = color_sigma.s3_green_sigma03;
	color_sigma.s3_white_sigma03 = color_sigma.s3_green_sigma03;

	color_sigma.s3_green_sigma45 = COLOR_SIGMA4 | (COLOR_SIGMA5 << 8);
	color_sigma.s3_red_sigma45 = color_sigma.s3_green_sigma45;
	color_sigma.s3_blue_sigma45 = color_sigma.s3_red_sigma45;
	color_sigma.s3_white_sigma45 = color_sigma.s3_red_sigma45;

	set_reg(DPU_HIACE_S3_GREEN_SIGMA03_ADDR(hiace_base), color_sigma.s3_green_sigma03, 32, 0);
	set_reg(DPU_HIACE_S3_GREEN_SIGMA45_ADDR(hiace_base), color_sigma.s3_green_sigma45, 32, 0);

	set_reg(DPU_HIACE_S3_RED_SIGMA03_ADDR(hiace_base), color_sigma.s3_red_sigma03, 32, 0);
	set_reg(DPU_HIACE_S3_RED_SIGMA45_ADDR(hiace_base), color_sigma.s3_red_sigma45, 32, 0);

	set_reg(DPU_HIACE_S3_BLUE_SIGMA03_ADDR(hiace_base), color_sigma.s3_blue_sigma03, 32, 0);
	set_reg(DPU_HIACE_S3_BLUE_SIGMA45_ADDR(hiace_base), color_sigma.s3_blue_sigma45, 32, 0);

	set_reg(DPU_HIACE_S3_WHITE_SIGMA03_ADDR(hiace_base), color_sigma.s3_white_sigma03, 32, 0);
	set_reg(DPU_HIACE_S3_WHITE_SIGMA45_ADDR(hiace_base), color_sigma.s3_white_sigma45, 32, 0);

	set_reg(DPU_HIACE_S3_FILTER_LEVEL_ADDR(hiace_base), 20, 5, 0);
	set_reg(DPU_HIACE_S3_SIMILARITY_COEFF_ADDR(hiace_base), 296, 10, 0);
	set_reg(DPU_HIACE_S3_V_FILTER_WEIGHT_ADJ_ADDR(hiace_base), 0x1, 2, 0);
}

static void dpu_nr_set_skin_threshold(char __iomem *hiace_base)
{
	uint32_t s3_hue;
	uint32_t s3_saturation;
	uint32_t s3_value;
	uint32_t slop;
	uint32_t th_max, th_min;

	slop = 68 & 0xff;
	th_min = 0x0 & 0x1ff;
	th_max = 25 & 0x1ff;
	s3_hue = (slop << 24) | (th_max << 12) | th_min;
	set_reg(DPU_HIACE_S3_HUE_ADDR(hiace_base), s3_hue, 32, 0);

	th_min = 80 & 0xff;
	th_max = 120 & 0xff;
	s3_saturation = (slop << 24) | (th_max << 12) | th_min;
	set_reg(DPU_HIACE_S3_SATURATION_ADDR(hiace_base), s3_saturation, 32, 0);

	th_min = 120 & 0xff;
	th_max = 255 & 0xff;
	s3_value = (slop << 24) | (th_max << 12) | th_min;
	set_reg(DPU_HIACE_S3_VALUE_ADDR(hiace_base), s3_value, 32, 0);

	set_reg(DPU_HIACE_S3_SKIN_GAIN_ADDR(hiace_base), 0x80, 8, 0);
}

static void dpu_noisereduction_init(char __iomem *hiace_base)
{
	dpu_nr_set_somebrightness(hiace_base);
	dpu_nr_set_color_sigma(hiace_base);
	dpu_nr_set_skin_threshold(hiace_base);
}

static void dpu_hiace_init_ab_status(char __iomem *dpp_base)
{
	uint32_t gamma_ab_work;
	uint32_t gamma_ab_shadow;
	uint32_t global_hist_ab_work;
	uint32_t global_hist_ab_shadow;
	char __iomem *hiace_base = dpp_base + HIACE_OFFSET;

	global_hist_ab_work = inp32(DPU_HIACE_GLOBAL_HIST_AB_WORK_ADDR(hiace_base));
	global_hist_ab_shadow = !global_hist_ab_work;
	gamma_ab_work = inp32(DPU_HIACE_GAMMA_AB_WORK_ADDR(hiace_base));
	gamma_ab_shadow = !gamma_ab_work;

	set_reg(DPU_HIACE_GAMMA_AB_SHADOW_ADDR(hiace_base), gamma_ab_shadow, 1, 0);
	set_reg(DPU_HIACE_GLOBAL_HIST_AB_SHADOW_ADDR(hiace_base), global_hist_ab_shadow, 1, 0);
}

void dpu_hiace_init_params(struct dpu_composer *dpu_comp)
{
	char __iomem *hiace_base = NULL;
	char __iomem *dpu_base = NULL;
	char __iomem *dpp_base = NULL;
	unsigned long dw_jiffies = 0;
	uint32_t tmp = 0;
	bool is_ready = false;
	uint32_t up_thres;
	uint32_t low_thres;

	dpu_pr_info("[hiace] init param");

	dpu_check_and_no_retval(!dpu_comp, err, "[hiace] dpu_comp is null!");
	dpu_check_and_no_retval(!dpu_comp->comp_mgr, err, "[hiace] comp_mgr is null!");
	dpu_check_and_no_retval(!dpu_comp->conn_info, err, "[hiace] conn_info is null!");

	dpu_base = dpu_comp->comp_mgr->dpu_base;
	dpp_base = dpu_base + dpu_comp->hiace_ctrl->dpp_offset;
	hiace_base = dpp_base + HIACE_OFFSET;

	dpu_comp_active_vsync(dpu_comp);

	// only for cmd panel
	if (!is_mipi_video_panel(&dpu_comp->conn_info->base))
		set_reg(DPU_HIACE_BYPASS_ACE_ADDR(hiace_base), 0x0, 1, 0);

	set_reg(DPU_HIACE_INIT_GAMMA_ADDR(hiace_base), 0x1, 1, 0);
	set_reg(DPU_HIACE_UPDATE_LOCAL_ADDR(hiace_base), 0x1, 1, 0);
	set_reg(DPU_HIACE_UPDATE_FNA_ADDR(hiace_base), 0x1, 1, 0);

	/* PIPE_CFG default value */
	/* init ace param 0x30~ 0x60 */
	/* size init must move to hal cause ov clip change size */
	dpu_set_size_info(dpu_comp);
	dpu_set_skin_protection_info(hiace_base);

	up_thres = 248 & 0xff;
	low_thres = 8 & 0xff;
	set_reg(DPU_HIACE_UP_LOW_TH_ADDR(hiace_base), (up_thres << 8) | low_thres, 32, 0);

	/* wait for gamma init finishing */
	dw_jiffies = jiffies + HZ / 2;
	do {
		tmp = inp32(DPU_HIACE_INIT_GAMMA_ADDR(hiace_base));
		if ((tmp & 0x1) != 0x1) {
			is_ready = true;
			break;
		}
	} while (time_after(dw_jiffies, jiffies));

	dpu_pr_info("[hiace]init gamma status %d", is_ready);

	dpu_hiace_init_ab_status(dpp_base);

	/* clear hiace interrupt */
	set_reg(DPU_HIACE_ACE_INT_STAT_ADDR(hiace_base),  0x1, 1, 0);

	/* unmask hiace interrupt */
	set_reg(DPU_HIACE_ACE_INT_UNMASK_ADDR(hiace_base),  0x1, 1, 0);

	set_reg(DPU_HIACE_RGB_BLEND_WEIGHT_ADDR(hiace_base), 0, 32, 0);
	set_reg(DPU_DPP_INT_MSK_ADDR(dpp_base), 0x0, 1, 0);
	if (dpu_comp->hiace_ctrl->dpp_offset == DPU_DPP0_OFFSET) {
		set_reg(DPU_GLB_NS_SDP_TO_GIC_MSK_ADDR(dpu_base + DPU_GLB0_OFFSET), 0, 1, 4);
	} else {
		set_reg(DPU_GLB_NS_SDP_TO_GIC_MSK_ADDR(dpu_base + DPU_GLB0_OFFSET), 0, 1, 1);
	}
	dpu_noisereduction_init(hiace_base);
	// hiace v3 has default value no need init
	set_reg(DPU_HIACE_BYPASS_ACE_ADDR(hiace_base), 0x1, 1, 0);

	dpu_comp_deactive_vsync(dpu_comp);
}
