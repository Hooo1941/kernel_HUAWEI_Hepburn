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

#include <dpu_format.h>
#include <dpu/soc_dpu_define.h>
#include "dkmd_dpu.h"
#include "dkmd_comp.h"
#include "dkmd_base_frame.h"
#include "dkmd_base_layer.h"
#include "dkmd_network.h"
#include "dkmd_opr_id.h"
#include "operators/cmd_manager.h"
#include "gfxdev_pandisplay_utils.h"
#include "dpu_comp_mgr.h"
#include "res_mgr.h"
#include "dkmd_timeline.h"


#define PAN_DISPLAY_OPR_NUM 3
#define BUFFER_MAX_COUNT 3

struct screen_format_info {
	uint32_t offset;
	int32_t format;
};

static struct fix_var_screeninfo g_fix_var_screeninfo[GFXDEV_FOARMT_MAX] = {
	 /* GFXDEV_FORMAT_BGRA8888, */
	{ FB_TYPE_PACKED_PIXELS, 1, 1, FB_VMODE_NONINTERLACED, 0, 8, 16, 24, 8, 8, 8, 8, 0, 0, 0, 0, 4 },

	/* GFXDEV_FORMAT_RGB565 */
	{FB_TYPE_PACKED_PIXELS, 1, 1, FB_VMODE_NONINTERLACED, 0, 5, 11, 0, 5, 6, 5, 0, 0, 0, 0, 0, 2},

	/* GFXDEV_FORMAT_YUV_422_I */
	{FB_TYPE_INTERLEAVED_PLANES, 2, 1, FB_VMODE_NONINTERLACED, 0, 5, 11, 0, 5, 6, 5, 0, 0, 0, 0, 0, 2},
};

struct fix_var_screeninfo* get_fix_var_screeninfo(void)
{
	return g_fix_var_screeninfo;
}


void gfxdev_init_fbi_var_info(const struct dkmd_object_info *pinfo,
	struct fix_var_screeninfo *screen_info, gfxdev_var_screeninfo *var)
{
	if (unlikely(!pinfo || !screen_info || !var)) {
		dpu_pr_err("input nullptr");
		return;
	}

	memset_s(var, sizeof(gfxdev_var_screeninfo), 0, sizeof(gfxdev_var_screeninfo));
	var->xoffset = 0;
	var->yoffset = 0;
	var->grayscale = 0;
	var->nonstd = 0;
	var->activate = FB_ACTIVATE_VBL;
	var->accel_flags = 0;
	var->sync = 0;
	var->rotate = 0;

	var->vmode = screen_info->var_vmode;
	var->blue.offset = screen_info->var_blue_offset;
	var->green.offset = screen_info->var_green_offset;
	var->red.offset = screen_info->var_red_offset;
	var->transp.offset = screen_info->var_transp_offset;
	var->blue.length = screen_info->var_blue_length;
	var->green.length = screen_info->var_green_length;
	var->red.length = screen_info->var_red_length;
	var->transp.length = screen_info->var_transp_length;
	var->blue.msb_right = screen_info->var_blue_msb_right;
	var->green.msb_right = screen_info->var_green_msb_right;
	var->red.msb_right = screen_info->var_red_msb_right;
	var->transp.msb_right = screen_info->var_transp_msb_right;
	var->bits_per_pixel = screen_info->bpp * 8;

	var->xres = pinfo->xres;
	var->yres = pinfo->yres;
	var->height = pinfo->height;
	var->width = pinfo->width;
	var->xres_virtual = var->xres;
	var->yres_virtual = var->yres * BUFFER_MAX_COUNT;
	var->reserved[0] = pinfo->fps;
}

int32_t gfxdev_init_fscreen_info(struct composer *comp, struct fix_var_screeninfo *screen_info,
	gfxdev_fix_screeninfo *fix)
{
	if (unlikely(!fix || !screen_info))
		return -EINVAL;

	if (unlikely(!comp))
		return -ENODEV;

	memset_s(fix, sizeof(gfxdev_fix_screeninfo), 0, sizeof(gfxdev_fix_screeninfo));
	fix->type_aux = 0;
	fix->visual = FB_VISUAL_TRUECOLOR;
	fix->ywrapstep = 0;
	fix->mmio_start = 0;
	fix->mmio_len = 0;
	fix->accel = FB_ACCEL_NONE;

	fix->type = screen_info->fix_type;
	fix->xpanstep = (uint16_t)screen_info->fix_xpanstep;
	fix->ypanstep = (uint16_t)screen_info->fix_ypanstep;

	(void)snprintf(fix->id, sizeof(fix->id), "dpufb%u", comp->index);

	fix->line_length = roundup(comp->base.xres * screen_info->bpp, DMA_STRIDE_ALIGN);
	fix->smem_len = roundup(fix->line_length * comp->base.yres * BUFFER_MAX_COUNT, PAGE_SIZE);
	fix->smem_start = 0;
	fix->reserved[0] = (uint16_t)is_mipi_cmd_panel(&comp->base);

	dpu_res_register_screen_info(comp->base.xres, comp->base.yres);
	return 0;
}

static int32_t pan_display_get_screen_format(const struct screen_format_info *format_info,
	uint32_t len, uint32_t offset)
{
	uint32_t i;

	for (i = 0; i < len; ++i) {
		if (offset == format_info[i].offset)
			return format_info[i].format;
	}

	dpu_pr_err("offset=%u not support", offset);
	return -1;
}

static int32_t pan_display_get_format(const gfxdev_var_screeninfo *var)
{
	struct screen_format_info blue_info[] = {
		{8, DPU_FMT_RGBX_4444},
		{10, DPU_FMT_RGBX_5551},
		{11, DPU_FMT_BGR_565}
	};

	struct screen_format_info red_info[] = {
		{8, DPU_FMT_BGRX_4444},
		{10, DPU_FMT_BGRX_5551},
		{11, DPU_FMT_RGB_565}
	};

	if (var->bits_per_pixel == 32) {
		if (var->blue.offset == 0)
			return (var->transp.length == 8) ? DPU_FMT_BGRA_8888 : DPU_FMT_BGRX_8888;

		return (var->transp.length == 8) ? DPU_FMT_RGBA_8888 : DPU_FMT_RGBX_8888;
	} else if (var->bits_per_pixel == 16) {
		if (var->transp.offset == 12) {
			blue_info[0].format = DPU_FMT_RGBA_4444;
			blue_info[1].format = DPU_FMT_RGBA_5551;
			red_info[0].format = DPU_FMT_BGRA_4444;
			red_info[1].format = DPU_FMT_BGRA_5551;
		}

		if (var->blue.offset == 0)
			return pan_display_get_screen_format(red_info, ARRAY_SIZE(red_info), var->red.offset);

		return pan_display_get_screen_format(blue_info, ARRAY_SIZE(blue_info), var->blue.offset);
	}

	dpu_pr_err("bits_per_pixel=%u not support", var->bits_per_pixel);
	return -1;
}

static int32_t build_post_pipeline(struct composer *comp, const struct dkmd_base_frame *frame,
	struct dkmd_network *network)
{
	struct dkmd_pipeline *pipeline = NULL;

	network->post_pipeline_num = 1;
	network->post_pipelines = kzalloc(network->post_pipeline_num * sizeof(*network->post_pipelines), GFP_KERNEL);
	if (!network->post_pipelines) {
		dpu_pr_err("alloc post pipeline mem fail");
		return -ENOMEM;
	}

	pipeline = &network->post_pipelines[0];
	pipeline->block_layer = kzalloc(sizeof(*pipeline->block_layer), GFP_KERNEL);
	if (!pipeline->block_layer) {
		dpu_pr_err("alloc post pipeline's block_layer mem fail");
		kfree(network->post_pipelines);
		network->post_pipelines = NULL;
		return -ENOMEM;
	}
	pipeline->block_layer->layer_type = POST_LAYER;
	pipeline->block_layer->src_rect = frame->layers[0].src_rect;
	pipeline->block_layer->dst_rect = frame->layers[0].src_rect;
	pipeline->block_layer->dsc_en = comp->base.dsc_en;
	pipeline->block_layer->spr_en = comp->base.spr_en;
	pipeline->block_layer->dsc_out_width = comp->base.dsc_out_width;
	pipeline->block_layer->dsc_out_height = comp->base.dsc_out_height;

	pipeline->opr_num = (pipeline->block_layer->dsc_en == 1) ? 3 : 2; // use dpp/dsc/itfsw as pre-pipeline
	pipeline->opr_ids = kzalloc(pipeline->opr_num * sizeof(*pipeline->opr_ids), GFP_KERNEL);
	if (!pipeline->opr_ids) {
		dpu_pr_err("alloc post pipeline's opr_ids mem fail");
		kfree(network->post_pipelines);
		kfree(pipeline->block_layer);
		network->post_pipelines = NULL;
		pipeline->block_layer = NULL;
		return -ENOMEM;
	}

	dpu_pr_debug("pan display:dsc_en[%u], spr_en[%u], dsc_out_w[%u], dsc_out_h[%u]",
		pipeline->block_layer->dsc_en, pipeline->block_layer->spr_en,
		pipeline->block_layer->dsc_out_width, pipeline->block_layer->dsc_out_height);

	pipeline->opr_ids[0].info.type = OPERATOR_DPP;
	pipeline->opr_ids[0].info.ins = frame->scene_id;
	if (pipeline->block_layer->dsc_en == 1) {
		pipeline->opr_ids[1].info.type = OPERATOR_DSC;
		pipeline->opr_ids[1].info.ins = 0;
		pipeline->opr_ids[2].info.type = OPERATOR_ITFSW;
		pipeline->opr_ids[2].info.ins = frame->scene_id;
	} else {
		pipeline->opr_ids[1].info.type = OPERATOR_ITFSW;
		pipeline->opr_ids[1].info.ins = frame->scene_id;
	}

	return 0;
}

static int32_t build_pre_pipeline(const struct dkmd_base_frame *frame, struct dkmd_network *network)
{
	struct dkmd_pipeline *pipeline = NULL;

	network->pre_pipeline_num = 1;
	network->pre_pipelines = kzalloc(network->pre_pipeline_num * sizeof(*network->pre_pipelines), GFP_KERNEL);
	if (!network->pre_pipelines) {
		dpu_pr_err("alloc pre pipeline mem fail");
		return -ENOMEM;
	}

	pipeline = &network->pre_pipelines[0];
	pipeline->block_layer = &frame->layers[0];
	pipeline->opr_num = 2; // use sdma/ov as pre pipeline
	pipeline->opr_ids = kzalloc(pipeline->opr_num * sizeof(*pipeline->opr_ids), GFP_KERNEL);
	if (!pipeline->opr_ids) {
		dpu_pr_err("alloc pre pipeline's opr_ids mem fail");
		kfree(network->pre_pipelines);
		network->pre_pipelines = NULL;
		return -ENOMEM;
	}
	pipeline->opr_ids[0].info.type = OPERATOR_SDMA;
	pipeline->opr_ids[0].info.ins = frame->scene_id;
	pipeline->opr_ids[1].info.type = OPERATOR_OV;
	pipeline->opr_ids[1].info.ins = frame->scene_id;

	return 0;
}

static int32_t build_network_info(struct composer *comp, const struct dkmd_base_frame *frame,
	struct dkmd_network *network)
{
	struct dkmd_pipeline *pipeline = NULL;

	// pan display has no block segmentation, network is not only first block but last block
	network->is_first_block = true;
	network->is_last_block = true;

	if (build_pre_pipeline(frame, network))
		return -1;

	if (build_post_pipeline(comp, frame, network)) {
		pipeline = &network->pre_pipelines[0];
		kfree(pipeline->opr_ids);
		kfree(network->pre_pipelines);
		pipeline->opr_ids = NULL;
		network->pre_pipelines = NULL;
		return -1;
	}

	return 0;
}

static void destory_network_info(struct dkmd_network *network)
{
	uint32_t i;
	struct dkmd_pipeline *pipeline = NULL;

	for (i = 0; i < network->pre_pipeline_num; i++) {
		pipeline = &network->pre_pipelines[i];
		kfree(pipeline->opr_ids);
		pipeline->opr_ids = NULL;
	}
	kfree(network->pre_pipelines);
	network->pre_pipelines = NULL;

	for (i = 0; i < network->post_pipeline_num; i++) {
		pipeline = &network->post_pipelines[i];
		kfree(pipeline->block_layer);
		kfree(pipeline->opr_ids);
		pipeline->block_layer = NULL;
		pipeline->opr_ids = NULL;
	}
	kfree(network->post_pipelines);
	network->post_pipelines = NULL;
}

static int32_t build_frame_info(const struct dkmd_object_info *object_info, gfxdev_fix_screeninfo *fix,
	gfxdev_var_screeninfo *var, struct dkmd_base_frame *frame)
{
	struct dkmd_base_layer *layer = NULL;

	frame->layers_num = 1;
	frame->layers = kzalloc(frame->layers_num * sizeof(*frame->layers), GFP_KERNEL);
	if (!frame->layers) {
		dpu_pr_err("alloc layer mem fail");
		return -ENOMEM;
	}

	layer = &frame->layers[0];
	layer->layer_type = SOURCE_LAYER;
	layer->format = pan_display_get_format(var);
	if (unlikely(layer->format == -1)) {
		kfree(frame->layers);
		frame->layers = NULL;
		return -1;
	}
	layer->transform = TRANSFORM_NONE;
	layer->compress_type = COMPRESS_NONE;
	layer->iova = fix->smem_start +
		var->xoffset * (var->bits_per_pixel >> 3) + var->yoffset * fix->line_length;
	layer->phyaddr = 0;
	layer->stride = fix->line_length;
	layer->acquire_fence = -1;
	layer->dm_index = 0;
	layer->csc_mode = CSC_709_NARROW;
	layer->color_space = COLOR_SPACE_SRGB;
	layer->src_rect.right = var->xres;
	layer->src_rect.bottom = var->yres;
	layer->dst_rect = layer->src_rect;

	frame->fps = object_info->fps;
	frame->scene_id = object_info->pipe_sw_itfch_idx;
	frame->scene_mode = SCENE_MODE_NORMAL;

	return 0;
}

int32_t build_display_info(const struct dkmd_object_info *object_info, gfxdev_fix_screeninfo *fix,
	gfxdev_var_screeninfo *var, struct composer *comp,
	struct dkmd_base_frame *frame, struct dkmd_network *network)
{
	if (unlikely(!object_info || !fix || !var || !comp || !frame || !network))
		return -1;

	if (build_frame_info(object_info, fix, var, frame))
		return -1;

	if (build_network_info(comp, frame, network)) {
		kfree(frame->layers);
		frame->layers = NULL;
		return -1;
	}

	return 0;
}

int32_t execute_compose(struct composer *comp, gfxdev_var_screeninfo *var,
	struct dkmd_base_frame *frame, struct dkmd_network *network)
{
	uint32_t i;
	int32_t ret;
	struct disp_frame present_frame = { 0 };
	struct dpu_composer *dpu_comp = NULL;

	g_pan_display_frame_index++;
	if (unlikely(generate_network_cmd(frame, network) != 0)) {
		dpu_pr_err("generator cmd fail");
		return -1;
	}

	present_frame.cmdlist_id = frame->scene_cmdlist_id;
	present_frame.layer_count = frame->layers_num;
	present_frame.scene_id = frame->scene_id;
	if (frame->layers_num > DISP_LAYER_MAX_COUNT) {
		dpu_pr_err("layers_num out of range!\n");
		return -1;
	}
	for (i = 0; i < frame->layers_num; ++i) {
		present_frame.layer[i].acquired_fence = -1;
		present_frame.layer[i].share_fd = -1;
	}
	present_frame.frame_index = g_pan_display_frame_index;
	present_frame.disp_rect.x = 0;
	present_frame.disp_rect.y = 0;
	present_frame.disp_rect.w = var->xres;
	present_frame.disp_rect.h = var->yres;
	present_frame.dvfs_info.vote_freq_info.current_total_freq = DPU_CORE_FREQ3;
	present_frame.dvfs_info.vote_freq_info.sdma_freq = 0;
	/* when pan display, is not supported intra dvfs */
	present_frame.dvfs_info.is_supported_intra_dvfs = false;
	present_frame.effect_params.effect_num = 0;
	present_frame.present_fence_pt = dkmd_timeline_get_next_value(get_online_timeline(comp));

	dpu_comp = to_dpu_composer(comp);
	if (is_ppc_support(&dpu_comp->conn_info->base))
		present_frame.ppc_config_id = PPC_CONFIG_ID_G_MODE;
	
	present_frame.active_frame_rate = 60;

	ret = comp->present(comp, (void *)&present_frame);
	if (ret < 0) {
		dpu_pr_err("pan display fail");
		dkmd_timeline_dec_next_value(get_online_timeline(comp));
	}

	return ret;
}

static void destory_display_info(struct dkmd_base_frame *frame, struct dkmd_network *network)
{
	if (unlikely(!frame || !network)) {
		dpu_pr_err("input nullptr");
		return;
	}

	kfree(frame->layers);
	frame->layers = NULL;
	destory_network_info(network);
}

int32_t gfxdev_pan_display(gfxdev_fix_screeninfo *fix, gfxdev_var_screeninfo *var,
	const struct dkmd_object_info *pinfo, struct composer *comp)
{
	int ret = -1;
	struct dkmd_base_frame frame;
	struct dkmd_network network;
	struct dpu_composer *dpu_comp = NULL;

	if (unlikely(!fix || !var)) {
		dpu_pr_err("fix or var is null");
		return -EINVAL;
	}

	if (unlikely(!pinfo)) {
		dpu_pr_err("pinfo is null");
		return -EINVAL;
	}

	if (unlikely(!comp)) {
		dpu_pr_err("comp is null");
		return -EINVAL;
	}

	if (unlikely(fix->smem_start == 0)) {
		dpu_pr_err("smem_start is 0");
		return -EINVAL;
	}

	if (fix->xpanstep)
		var->xoffset = (var->xoffset / fix->xpanstep) * fix->xpanstep;

	if (fix->ypanstep)
		var->yoffset = (var->yoffset / fix->ypanstep) * fix->ypanstep;

	if (build_display_info(pinfo, fix, var, comp, &frame, &network))
		return -EINVAL;

	down(&comp->blank_sem);
	if (!comp->power_on) {
		dpu_pr_warn("%s is power off", comp->base.name);
		destory_display_info(&frame, &network);
		up(&comp->blank_sem);
		return -1;
	}

	dpu_pr_debug("%s pan display, scene_id:%d, smem_start:%#x, xpanstep = %u, ypanstep = %u",
		comp->base.name, frame.scene_id, fix->smem_start, fix->xpanstep, fix->ypanstep);

	dpu_comp = to_dpu_composer(comp);
	composer_active_vsync(dpu_comp->conn_info, true);

	ret = execute_compose(comp, var, &frame, &network);
	if (likely(ret == 0))
		dpu_pr_debug("%s pan display success", comp->base.name);
	else
		dpu_pr_err("%s pan display fail", comp->base.name);

	composer_active_vsync(dpu_comp->conn_info, false);

	destory_display_info(&frame, &network);
	up(&comp->blank_sem);

	return 0;
}

