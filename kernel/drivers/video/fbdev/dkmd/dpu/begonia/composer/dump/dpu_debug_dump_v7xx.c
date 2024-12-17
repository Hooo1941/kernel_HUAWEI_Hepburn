/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2025. All rights reserved.
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
#include <linux/delay.h>
#include <dpu/dpu_dm.h>
#include <soc_media1_crg_interface.h>
#include <linux/interrupt.h>
#include "dkmd_listener.h"
#include "dpu_comp_mgr.h"
#include "dpu_opr_config.h"
#include "dpu_config_utils.h"
#include "cmdlist_interface.h"
#include "dksm_debug.h"
#include "dpu_config_utils.h"
#include "dkmd_rect.h"
#include "dkmd_base_layer.h"
#include "opr_mgr/operators/opr_format.h"
#include "dpu_debug_dump.h"
#include "dksm_dmd.h"

static int32_t dpu_parse_rect_info(char __iomem *dm_addr, struct dkmd_dm_info *dm_info, uint32_t layer_id)
{
	DPU_DM_LAYER_HEIGHT_UNION reg_layer_height;
	DPU_DM_CLD0_INPUT_FMT_UNION reg_cld_input_fmt;
	DPU_DM_LAYER_BOT_CROP_UNION reg_layer_bot_crop;
	DPU_DM_CLD0_OUTPUT_IMG_WIDTH_UNION reg_cld_output_img_width;
	DPU_DM_LAYER_OV_DFC_CFG_UNION reg_layer_ov_dfc_cfg;
	DPU_DM_LAYER_OV_CLIP_LEFT_UNION reg_layer_ov_clip_left;
	DPU_DM_LAYER_OV_STARTY_UNION reg_layer_ov_starty;
	DPU_DM_LAYER_OV_ENDY_UNION reg_layer_ov_endy;
	int j;

	reg_layer_height.value = inp32(DPU_DM_LAYER_HEIGHT_ADDR(dm_addr, layer_id));
	reg_layer_bot_crop.value = inp32(DPU_DM_LAYER_BOT_CROP_ADDR(dm_addr, layer_id));
	dpu_pr_debug("reg_layer_height:0x%x", reg_layer_height.value);

	dm_info->dm_layer_info[layer_id].src_aligned_rect.left = 0;
	dm_info->dm_layer_info[layer_id].src_aligned_rect.top = 0;
	dm_info->dm_layer_info[layer_id].src_aligned_rect.right = plus_one(reg_layer_height.reg.layer_width);
	dm_info->dm_layer_info[layer_id].src_aligned_rect.bottom = plus_one(reg_layer_height.reg.layer_height);

	if (dm_info->dm_layer_info[layer_id].src_aligned_rect.bottom == 1 &&
		reg_layer_bot_crop.reg.layer_nxt_order == OPR_CLD) {
		for (j = 0; j < DM_CLD_NUM; ++j) {
			reg_cld_input_fmt.value = inp32(DPU_DM_CLD0_INPUT_FMT_ADDR(dm_addr + j * DM_CLD_REG_LENGTH));
			if (reg_cld_input_fmt.reg.cld0_layer_id == layer_id)  {
				dm_info->dm_layer_info[layer_id].caps |= CAPS_CLD;
				dpu_pr_debug("cld layer:0x%x", layer_id);
				reg_cld_output_img_width.value =
							inp32(DPU_DM_CLD0_OUTPUT_IMG_WIDTH_ADDR(dm_addr + j * DM_CLD_REG_LENGTH));
				dm_info->dm_layer_info[layer_id].src_aligned_rect.right =
							plus_one(reg_cld_output_img_width.reg.cld0_output_img_width);
				dm_info->dm_layer_info[layer_id].src_aligned_rect.bottom =
							plus_one(reg_cld_output_img_width.reg.cld0_output_img_height);
			}
		}
	}

	reg_layer_ov_dfc_cfg.value = inp32(DPU_DM_LAYER_OV_DFC_CFG_ADDR(dm_addr, layer_id));
	reg_layer_ov_clip_left.value = inp32(DPU_DM_LAYER_OV_CLIP_LEFT_ADDR(dm_addr, layer_id));
	if (reg_layer_ov_clip_left.reg.layer_hmirr_en == 1) {
		dm_info->dm_layer_info[layer_id].src_aligned_rect.left +=
			reg_layer_ov_dfc_cfg.reg.layer_ov_clip_right;
		dm_info->dm_layer_info[layer_id].src_aligned_rect.right -=
			reg_layer_ov_clip_left.reg.layer_ov_clip_left;
	} else {
		dm_info->dm_layer_info[layer_id].src_aligned_rect.left +=
			reg_layer_ov_clip_left.reg.layer_ov_clip_left;
		dm_info->dm_layer_info[layer_id].src_aligned_rect.right -=
			reg_layer_ov_dfc_cfg.reg.layer_ov_clip_right;
	}

	reg_layer_ov_starty.value = inp32(DPU_DM_LAYER_OV_STARTY_ADDR(dm_addr, layer_id));
	reg_layer_ov_endy.value = inp32(DPU_DM_LAYER_OV_ENDY_ADDR(dm_addr, layer_id));
	dm_info->dm_layer_info[layer_id].dst_rect.left = reg_layer_ov_starty.reg.layer_ov_startx;
	dm_info->dm_layer_info[layer_id].dst_rect.top = reg_layer_ov_starty.reg.layer_ov_starty;
	dm_info->dm_layer_info[layer_id].dst_rect.right = plus_one(reg_layer_ov_endy.reg.layer_ov_endx);
	dm_info->dm_layer_info[layer_id].dst_rect.bottom = plus_one(reg_layer_ov_endy.reg.layer_ov_endy);

	return 0;
}

static int32_t dpu_parse_layers(char __iomem *dm_addr, struct dkmd_dm_info *dm_info)
{
	DPU_DM_LAYER_DMA_SEL_UNION layer_dma_sel;
	DPU_DM_LAYER_OV_CLIP_LEFT_UNION layer_ov_clip_left;
	DPU_DM_LAYER_BOT_CROP_UNION layer_bot_crop;
	DPU_DM_SROT0_INPUT_IMG_WIDTH_UNION sort0_input;
	DPU_DM_SROT0_OUTPUT_IMG_WIDTH_UNION sort0_output;
	DPU_DM_SROT0_INPUT_FMT_UNION srot0_input_fmt;
	DPU_DM_LAYER_SBLK_TYPE_UNION layer_sblk_type;
	DPU_DM_LAYER_OV_MODE_UNION layer_ov_mode;
	uint32_t flip;
	uint32_t i;

	dm_info->dm_layer_info = (struct dkmd_dm_layer_info *)kmalloc(sizeof(struct dkmd_dm_layer_info) *
								dm_info->layer_cnt, GFP_KERNEL);
	if (!dm_info->dm_layer_info) {
		dpu_pr_warn("dm_layer_info alloc failed!");
		return -ENOMEM;
	}

	if (dm_info->layer_cnt <= 0) {
		dpu_pr_warn("layer_cnt is not valid:%u", dm_info->layer_cnt);
		return -1;
	}

	for (i = 0; i < (uint32_t)dm_info->layer_cnt; ++i) {
		dm_info->dm_layer_info[i].layer_id = (uint8_t)i;
		layer_dma_sel.value = inp32(DPU_DM_LAYER_DMA_SEL_ADDR(dm_addr, i));
		layer_sblk_type.value = inp32(DPU_DM_LAYER_SBLK_TYPE_ADDR(dm_addr, i));
		layer_ov_mode.value = inp32(DPU_DM_LAYER_OV_MODE_ADDR(dm_addr, i));
		dpu_pr_debug("layer_dma_sel.value:0x%x", layer_dma_sel.value);

		dm_info->dm_layer_info[i].sdma_idx = layer_dma_sel.reg.layer_dma_sel - 1;
		dm_info->dm_layer_info[i].format = layer_dma_sel.reg.layer_dma_fmt;
		dpu_pr_debug("layer_dma_sel.format:0x%x",  layer_dma_sel.reg.layer_dma_fmt);

		dm_info->dm_layer_info[i].format = (unsigned char)sdma_fmt_to_dpu_fmt(dm_info->dm_layer_info[i].format);
		dm_info->dm_layer_info[i].caps = 0;
		dm_info->dm_layer_info[i].caps |= layer_ov_mode.reg.layer_ov_src_cfg == 1 ? CAPS_DIM : 0;
		dm_info->dm_layer_info[i].caps |= layer_dma_sel.reg.layer_fbc_type == 1 ? CAPS_HEBCE : 0;
		if (layer_dma_sel.reg.layer_fbc_type == 1)
			dpu_pr_warn("layer id[%d] blk type:%u ", i, layer_sblk_type.reg.layer_sblk_type);

		layer_ov_clip_left.value = inp32(DPU_DM_LAYER_OV_CLIP_LEFT_ADDR(dm_addr, i));
		layer_bot_crop.value = inp32(DPU_DM_LAYER_BOT_CROP_ADDR(dm_addr, i));
		sort0_input.value = inp32(DPU_DM_SROT0_INPUT_IMG_WIDTH_ADDR(dm_addr));
		sort0_output.value = inp32(DPU_DM_SROT0_OUTPUT_IMG_WIDTH_ADDR(dm_addr));

		srot0_input_fmt.value = inp32(DPU_DM_SROT0_INPUT_FMT_ADDR(dm_addr));
		flip = layer_ov_clip_left.reg.layer_vmirr_en == 1 ? TRANSFORM_FLIP_V : 0;
		flip |= layer_ov_clip_left.reg.layer_hmirr_en == 1 ? TRANSFORM_FLIP_H : 0;

		if ((layer_bot_crop.reg.layer_nxt_order == OPR_SROT) &&
			((sort0_input.reg.srot0_input_img_height |= sort0_output.reg.srot0_output_img_height) != 0) &&
			((sort0_input.reg.srot0_input_img_width |= sort0_output.reg.srot0_output_img_width) != 0)) {
			switch (flip) {
			case TRANSFORM_FLIP_V:
				dm_info->dm_layer_info[i].trasform = TRANSFORM_ROT_270;
				dm_info->dm_layer_info[i].caps |= CAPS_ROT;
				break;
			case TRANSFORM_FLIP_H:
				dm_info->dm_layer_info[i].trasform = TRANSFORM_ROT_90;
				dm_info->dm_layer_info[i].caps |= CAPS_ROT;
				break;
			case (TRANSFORM_FLIP_V | TRANSFORM_FLIP_H):
				dm_info->dm_layer_info[i].trasform = TRANSFORM_FLIP_V_ROT_90;
				dm_info->dm_layer_info[i].caps |= CAPS_ROT;
					break;
			default:
				dpu_pr_warn("wrong flip info");
				break;
			}
		} else {
			dm_info->dm_layer_info[i].trasform = (uint8_t)((flip == (TRANSFORM_FLIP_V | TRANSFORM_FLIP_H)) ?
												TRANSFORM_ROT_180 : flip);
		}

		dpu_parse_rect_info(dm_addr, dm_info, i);
	}

	return 0;
}

static int32_t dpu_parse_scl_layers(char __iomem *dm_addr, struct dkmd_dm_info *dm_info)
{
	DPU_DM_SCL0_INPUT_IMG_WIDTH_UNION reg_scl_input_img_width;
	DPU_DM_SCL0_OUTPUT_IMG_WIDTH_UNION reg_scl_output_img_width;
	DPU_DM_SCL0_TYPE_UNION reg_scl_type;
	DPU_DM_SCL0_THRESHOLD_UNION reg_scl_threshold;
	DPU_DM_SCL0_OUTPUT_IMG_WIDTH_UNION reg_scl_output;
	DPU_DM_SCL0_INPUT_IMG_WIDTH_UNION reg_scl_input;
	uint32_t layer_id;
	uint32_t crop_height;
	uint32_t i;
	uint32_t j;

	for (i = 0; i < DM_SCL_NUM; ++i) {
		reg_scl_input_img_width.value = inp32(DPU_DM_SCL0_INPUT_IMG_WIDTH_ADDR(dm_addr + i * DM_SCL_REG_LENGTH));
		reg_scl_output_img_width.value = inp32(DPU_DM_SCL0_OUTPUT_IMG_WIDTH_ADDR(dm_addr + i * DM_SCL_REG_LENGTH));
		reg_scl_type.value = inp32(DPU_DM_SCL0_TYPE_ADDR(dm_addr + i * DM_SCL_REG_LENGTH));
		dpu_pr_debug("reg_scl_type:0x%x", reg_scl_type.value);

		reg_scl_threshold.value = inp32(DPU_DM_SCL0_THRESHOLD_ADDR(dm_addr + i * DM_SCL_REG_LENGTH));
		if ((reg_scl_input_img_width.reg.scl0_input_img_width != reg_scl_output_img_width.reg.scl0_output_img_width) ||
			(reg_scl_input_img_width.reg.scl0_input_img_height !=
			reg_scl_output_img_width.reg.scl0_output_img_height)) {
				layer_id = reg_scl_type.reg.scl0_layer_id;
				dpu_pr_debug("layer_id:%d", layer_id);

				if (layer_id != DM_INVALID_LAYER_ID && layer_id >= dm_info->layer_cnt) {
					dpu_pr_warn("layer_id:%d is invalid", layer_id);
					continue;
				}

				if (reg_scl_type.reg.scl0_layer_id != DM_INVALID_LAYER_ID &&
					reg_scl_threshold.reg.scl0_pre_post_sel == DM_USE_SCL_BEFORE_OV) {
					if (layer_id >= dm_info->layer_cnt) {
						dpu_pr_warn("scl layer id(%u) is out of range(%u), need check.", layer_id, dm_info->layer_cnt);
					}
					dm_info->dm_layer_info[layer_id].caps |= CAPS_SCL;
				} else if (reg_scl_type.reg.scl0_layer_id == DM_INVALID_LAYER_ID &&
							reg_scl_threshold.reg.scl0_pre_post_sel == DM_USE_SCL_AFTER_OV) {
					dpu_pr_debug("post ov scl layer_id:%d", layer_id);
					reg_scl_input.value = inp32(DPU_DM_SCL0_INPUT_IMG_WIDTH_ADDR(dm_addr + i * DM_SCL_REG_LENGTH));
					dpu_pr_debug("reg_scl_input:0x%x", reg_scl_input.value);

					reg_scl_output.value = inp32(DPU_DM_SCL0_OUTPUT_IMG_WIDTH_ADDR(dm_addr + i * DM_SCL_REG_LENGTH));
					dpu_pr_debug("reg_scl_output:0x%x", reg_scl_output.value);

					if (reg_scl_input.reg.scl0_input_img_width != reg_scl_output.reg.scl0_output_img_width) {
						for (j = 0; j < dm_info->layer_cnt; ++j)
							dm_info->dm_layer_info[j].caps |= CAPS_ROG;
					} else {
						crop_height = reg_scl_input.reg.scl0_input_img_height -
												reg_scl_output.reg.scl0_output_img_height;
						dpu_pr_debug("crop_height:%d", crop_height);
						for (j = 0; j < dm_info->layer_cnt; ++j) {
							dm_info->dm_layer_info[j].dst_rect.top -= (crop_height / 2);
							dm_info->dm_layer_info[j].dst_rect.bottom -= (crop_height / 2);
						}
					}
				} else {
					dpu_pr_warn("invalid scl reg, layer_id(%u) scl0_pre_post_sel(%u), need check.",
									reg_scl_type.reg.scl0_layer_id, reg_scl_threshold.reg.scl0_pre_post_sel);
				}
			}
	}

	return 0;
}

static int32_t dpu_print_layer_info(struct dkmd_dm_info *dm_info)
{
	uint32_t i;
	dpu_pr_err("  Z   |  tr  | caps | sdma idx |  format  | src crop (l,t,r,b) | frame(l,t,r,b) \n"
	"                                                                ------+------+------+----------+----------+"
		"--------------------+------------------------\n");

	for (i = 0; i < dm_info->layer_cnt; ++i)
		dpu_pr_err(" %4u | %04x | %04x |  %06x  | %08u |"
			"%4d,%4d,%4d,%4d |%5d,%5d,%5d,%5d \n",
			dm_info->dm_layer_info[i].layer_id, dm_info->dm_layer_info[i].trasform, dm_info->dm_layer_info[i].caps,
			dm_info->dm_layer_info[i].sdma_idx, dm_info->dm_layer_info[i].format,
			dm_info->dm_layer_info[i].src_aligned_rect.left, dm_info->dm_layer_info[i].src_aligned_rect.top,
			dm_info->dm_layer_info[i].src_aligned_rect.right, dm_info->dm_layer_info[i].src_aligned_rect.bottom,
			dm_info->dm_layer_info[i].dst_rect.left, dm_info->dm_layer_info[i].dst_rect.top,
			dm_info->dm_layer_info[i].dst_rect.right, dm_info->dm_layer_info[i].dst_rect.bottom);

	dpu_pr_err("================================================================================");

	return 0;
}

static int32_t dpu_dsm_print_layer_info(struct dkmd_dm_info *dm_info)
{
	uint32_t cnt = 0;
	uint32_t i = 0;

	dpu_check_and_return(!dsm_lcd_client, -EINVAL, info, "dsm_lcd_client is null!");

	while((dsm_client_ocuppy(dsm_lcd_client) != 0) && (cnt < DSM_OCCUPY_RETRY_TIMES)) {
		dpu_pr_warn("dsm_client_ocuppy failed, retry %d times", ++cnt);
		usleep_range(500, 600);
	}

	if (cnt == DSM_OCCUPY_RETRY_TIMES) {
		dpu_pr_warn("dsm_client_ocuppy failed");
		return -1;
	}

	dsm_client_record(dsm_lcd_client, "  Z   |  tr  | caps | sdma idx |  format  | src crop (l,t,r,b) | frame(l,t,r,b) \n"
	"                                                                ------+------+------+----------+----------+"
		"--------------------+------------------------\n");

	for (i = 0; i < dm_info->layer_cnt; ++i)
		dsm_client_record(dsm_lcd_client, " %4u | %04x | %04x |  %06x  | %08u |"
			"%4d,%4d,%4d,%4d |%5d,%5d,%5d,%5d \n",
			dm_info->dm_layer_info[i].layer_id, dm_info->dm_layer_info[i].trasform, dm_info->dm_layer_info[i].caps,
			dm_info->dm_layer_info[i].sdma_idx, dm_info->dm_layer_info[i].format,
			dm_info->dm_layer_info[i].src_aligned_rect.left, dm_info->dm_layer_info[i].src_aligned_rect.top,
			dm_info->dm_layer_info[i].src_aligned_rect.right, dm_info->dm_layer_info[i].src_aligned_rect.bottom,
			dm_info->dm_layer_info[i].dst_rect.left, dm_info->dm_layer_info[i].dst_rect.top,
			dm_info->dm_layer_info[i].dst_rect.right, dm_info->dm_layer_info[i].dst_rect.bottom);

	dsm_client_notify(dsm_lcd_client, DSM_LCD_VACTIVE_TIMEOUT_ERROR_NO);

	return 0;
}

int32_t dpu_parse_layer_info(char __iomem *dpu_base, uint32_t scene_id)
{
	struct dkmd_dm_info dm_info;
	char __iomem *dm_addr = NULL;
	DPU_DM_LAYER_NUMBER_UNION reg_layer_number;

	dpu_check_and_return(!dpu_base, -1, err, "dpu_base is null!");

	dm_addr = (char __iomem *)(dpu_base + g_dm_tlb_info[scene_id].dm_data_addr);
	dpu_pr_err("DM layer info parse begin! scene_id:%d:", scene_id);

	reg_layer_number.value = inp32(DPU_DM_LAYER_NUMBER_ADDR(dm_addr));
	dm_info.layer_cnt = reg_layer_number.reg.layer_number;
	dpu_pr_err("layer_cnt:%d", dm_info.layer_cnt);

	dm_info.dm_layer_info = NULL;
	if (dpu_parse_layers(dm_addr, &dm_info)) {
		dpu_pr_warn("DM parse layer failed");

		if (dm_info.dm_layer_info) {
			kfree(dm_info.dm_layer_info);
			dm_info.dm_layer_info = NULL;
		}

		return -1;
	}

	dpu_parse_scl_layers(dm_addr, &dm_info);

	dpu_print_layer_info(&dm_info);

	if (dm_info.dm_layer_info) {
		kfree(dm_info.dm_layer_info);
		dm_info.dm_layer_info = NULL;
	}

	return 0;
}
