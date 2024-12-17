/******************************************************************
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2018. All rights reserved.
 * File name    segment_klt.c
 * Description:
 *
 * Date         2020-04-17 16:28:10
 ********************************************************************/
#include <linux/string.h>
#include <linux/printk.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/io.h>
#include "ipp.h"
#include "memory.h"
#include "segment_klt.h"
#include "cmdlst_manager.h"
#include "cvdr_drv.h"
#include "cvdr_opt.h"
#include "klt_drv.h"
#include "klt_reg_offset.h"
#include "ipp_top_reg_offset.h"
#include "cfg_table_ipp_path.h"

#define LOG_TAG LOG_MODULE_KLT
#define CFG_KLT_DS_FOR_PREV      (1)
#define CFG_KLT_DS_FOR_NEXT      (1)
#define CFG_KLT_FOR_KLT          (1)
#define KLT_DS_BLK_H_SIZE        (8)

static int check_klt_layer_req_param(msg_req_klt_t *klt_req)
{
	unsigned int align_val = 0, i = 0, tmp = 0;
	unsigned int width, height;
	cfg_tab_klt_t *klt_reg_param = &klt_req->reg_cfg;

	if (klt_reg_param->klt_klt_cfg.fwd_search.bits.fwd_pyr_num > KLT_MAX_FWD_PYR_NUM ||
		klt_reg_param->klt_klt_cfg.fwd_search.bits.fwd_pyr_num < KLT_MIN_FWD_PYR_NUM) {
		loge("Failed: fwd_pyr_num = %d, out of range", klt_reg_param->klt_klt_cfg.fwd_search.bits.fwd_pyr_num);
		return ISP_IPP_ERR;
	}

	if (klt_reg_param->klt_klt_cfg.klt_start.bits.start_klt == 0) {
		loge("Failed: klt_work_mode.%d, but start_klt is 0", klt_req->work_mode);
		return ISP_IPP_ERR;
	}

	width = klt_reg_param->klt_klt_cfg.klt_start.bits.frame_width + 1;
	height = klt_reg_param->klt_klt_cfg.klt_start.bits.frame_height + 1;
	if ((width % 2) != 0) {
		loge("Failed: input image width must be even: width.%d", width);
		return ISP_IPP_ERR;
	}

	align_val = 1;
	for (i = 0; i < (klt_reg_param->klt_klt_cfg.fwd_search.bits.fwd_pyr_num - 1); i++)
		align_val *= 2;

	logd("align_val.%d, fwd_pyr_num.%d", align_val, klt_reg_param->klt_klt_cfg.fwd_search.bits.fwd_pyr_num);
	if ((width % align_val) != 0 || (height % align_val) != 0) {
		loge("Failed: input image width/height must be multiple of 2^%d: width.%d, height.%d",
			klt_reg_param->klt_klt_cfg.fwd_search.bits.fwd_pyr_num - 1, width, height);
		return ISP_IPP_ERR;
	}

	tmp = (klt_reg_param->klt_klt_cfg.fwd_search.bits.fwd_patch_size + 3);
	tmp = (tmp << (klt_reg_param->klt_klt_cfg.fwd_search.bits.fwd_pyr_num - 1));
	logd("tmp.%d, fwd_patch_size.%d, fwd_pyr_num.%d", tmp,
		klt_reg_param->klt_klt_cfg.fwd_search.bits.fwd_patch_size,
		klt_reg_param->klt_klt_cfg.fwd_search.bits.fwd_pyr_num);
	if ((width < tmp) || (height < tmp)) {
		loge("Failed: input image width/height must be >= (fwd_patch_size+3)<<(fwd_pyr_num-1): width.%d, height.%d",
			width, height);
		return ISP_IPP_ERR;
	}

	return ISP_IPP_OK;
}

static int check_ds_layer_req_param(msg_req_klt_t *klt_req)
{
	unsigned int align_val = 0, i = 0;
	unsigned int width, height;
	cfg_tab_klt_t *klt_reg_param = &klt_req->reg_cfg;

	if (klt_reg_param->klt_ds_cfg.bits.ds_width == 0 || klt_reg_param->klt_ds_cfg.bits.ds_height == 0 ||
		klt_reg_param->klt_ds_cfg.bits.ds_nb_layer < KLT_MIN_FWD_PYR_NUM ||
		klt_reg_param->klt_ds_cfg.bits.ds_nb_layer > KLT_MAX_FWD_PYR_NUM) {
		loge("Failed: error ds para: but ds_width.%d, ds_height.%d, ds_nb_layer.%d",
			klt_reg_param->klt_ds_cfg.bits.ds_width, klt_reg_param->klt_ds_cfg.bits.ds_height,
			klt_reg_param->klt_ds_cfg.bits.ds_nb_layer);
		return ISP_IPP_ERR;
	}

	width = klt_reg_param->klt_ds_cfg.bits.ds_width + 1;
	height = klt_reg_param->klt_ds_cfg.bits.ds_height + 1;
	if ((width % 2) != 0) {
		loge("Failed: input image width must be even: width.%d", width);
		return ISP_IPP_ERR;
	}

	align_val = 1;
	for (i = 0; i < (klt_reg_param->klt_ds_cfg.bits.ds_nb_layer - 1); i++)
		align_val *= 2;

	logd("align_val.%d, ds_nb_layer.%d", align_val, klt_reg_param->klt_ds_cfg.bits.ds_nb_layer);
	if ((width % align_val) != 0 || (height % align_val) != 0) {
		loge("Failed: input image width/height must be multiple of 2^%d: width.%d, height.%d",
			klt_reg_param->klt_ds_cfg.bits.ds_nb_layer - 1, width, height);
		return ISP_IPP_ERR;
	}

	return ISP_IPP_OK;
}

/*****
 * @Name : seg_klt_check_req_param
 *	KLT layer's size constraints:
 *		[Note] input image width must be even and multiple of 2^(fwd_pyr_num-1);
 *			   input image width must be >= (fwd_patch_size+3)<<(fwd_pyr_num-1)[end].
 *		[Note] input image height must be multiple of 2^(fwd_pyr_num-1);
 *			   input image height must be >= (fwd_patch_size+3)<<(fwd_pyr_num-1)[end].
 *	DS layer's size constraints:
 *		[Note] input image height must be multiple of 2^(ds_nb_layer-1) [end].
 *		[Note] input image width must be even and multiple of 2^(ds_nb_layer-1) [end].
 * @Desc:
 *	check the reg para.
 ******/
static int seg_klt_check_req_param(msg_req_klt_t *klt_req)
{
	int ret = 0;

	if (klt_req->work_mode < KLT_OFF || klt_req->work_mode >= KLT_WORK_MODE_MAX) {
		loge("klt's work_mode.%d is invalid", klt_req->work_mode);
		return ISP_IPP_ERR;
	}

	if (klt_req->work_mode == KLT_DS_AND_KLT || klt_req->work_mode == KLT_ONLY_KLT) { // KLT layer
		loge_if_set_ret(check_klt_layer_req_param(klt_req), ret);
		if (ret) return ISP_IPP_ERR;
	}

	if (klt_req->work_mode == KLT_DS_AND_KLT || klt_req->work_mode == KLT_ONLY_DS) { // Ds_layer
		loge_if_set_ret(check_ds_layer_req_param(klt_req), ret);
		if (ret) return ISP_IPP_ERR;
	}

	return ISP_IPP_OK;
}

#ifdef IPP_UT_DEBUG
static void seg_klt_dump_req(msg_req_klt_t *klt_req)
{
	int i = 0;
	cfg_tab_klt_t *reg_cfg = &klt_req->reg_cfg;
	logi("sizeof(msg_req_klt_t) = 0x%x", (int)(sizeof(msg_req_klt_t)));
	logi("work_mode = %d", klt_req->work_mode);

	logi("Dump steam info:");
	for (i = 0; i < KLT_STREAM_MAX; i++)
		logi("stream.%d: width.%d, height.%d, stride.%d, fd.%d, prot.%d, offset.%d, format.%d", i,
			klt_req->streams[i].width, klt_req->streams[i].height, klt_req->streams[i].stride,
			klt_req->streams[i].fd, klt_req->streams[i].prot, klt_req->streams[i].offset, klt_req->streams[i].format);

	for (i = 0; i < KLT_STAT_BUFF_MAX; i++)
		logi("klt_stat_buff: i=%d, width.%d, height.%d, stride.%d, fd.%d, prot.%d, offset.0x%08x, format.%d", i,
			klt_req->klt_stat_buff[i].width, klt_req->klt_stat_buff[i].height, klt_req->klt_stat_buff[i].stride,
			klt_req->klt_stat_buff[i].fd, klt_req->klt_stat_buff[i].prot, klt_req->klt_stat_buff[i].offset,
			klt_req->klt_stat_buff[i].format);

	logi("Dump reg_cfg:");
	logi("klt_ds_cfg: ds_nb_layer: %d, ds_height:%d, ds_width:%d",
		reg_cfg->klt_ds_cfg.bits.ds_nb_layer, reg_cfg->klt_ds_cfg.bits.ds_height, reg_cfg->klt_ds_cfg.bits.ds_width);
	logi("klt_klt_cfg:");
	logi("start_klt: %d, frame_height: %d, frame_width: %d, kp_numbers: %d",
		reg_cfg->klt_klt_cfg.klt_start.bits.start_klt, reg_cfg->klt_klt_cfg.klt_start.bits.frame_height,
		reg_cfg->klt_klt_cfg.klt_start.bits.frame_width, reg_cfg->klt_klt_cfg.klt_start.bits.kp_numbers);
	logi("fwd_iter_accuracy: %d, fwd_patch_size: %d, fwd_pyr_num: %d",
		reg_cfg->klt_klt_cfg.fwd_search.bits.fwd_iter_accuracy, reg_cfg->klt_klt_cfg.fwd_search.bits.fwd_patch_size,
		reg_cfg->klt_klt_cfg.fwd_search.bits.fwd_pyr_num);
	logi("bwd_iter_accuracy: %d, bwd_patch_size: %d, bwd_pyr_num: %d",
		reg_cfg->klt_klt_cfg.bwd_search.bits.bwd_iter_accuracy, reg_cfg->klt_klt_cfg.bwd_search.bits.bwd_patch_size,
		reg_cfg->klt_klt_cfg.bwd_search.bits.bwd_pyr_num);
	logi("bwd_iter_guaranteed_nr_0: %d, fwd_iter_guaranteed_nr_0-4: %d, %d, %d, %d, %d",
		reg_cfg->klt_klt_cfg.iter_param.iter_cfg1.bits.bwd_iter_guaranteed_nr_0,
		reg_cfg->klt_klt_cfg.iter_param.iter_cfg1.bits.fwd_iter_guaranteed_nr_0,
		reg_cfg->klt_klt_cfg.iter_param.iter_cfg1.bits.fwd_iter_guaranteed_nr_1,
		reg_cfg->klt_klt_cfg.iter_param.iter_cfg1.bits.fwd_iter_guaranteed_nr_2,
		reg_cfg->klt_klt_cfg.iter_param.iter_cfg1.bits.fwd_iter_guaranteed_nr_3,
		reg_cfg->klt_klt_cfg.iter_param.iter_cfg1.bits.fwd_iter_guaranteed_nr_4);
	logi("bwd_iter_max_nr_0: %d, fwd_iter_max_nr_0-4: %d, %d, %d, %d, %d",
		reg_cfg->klt_klt_cfg.iter_param.iter_cfg2.bits.bwd_iter_max_nr_0,
		reg_cfg->klt_klt_cfg.iter_param.iter_cfg2.bits.fwd_iter_max_nr_0,
		reg_cfg->klt_klt_cfg.iter_param.iter_cfg2.bits.fwd_iter_max_nr_1,
		reg_cfg->klt_klt_cfg.iter_param.iter_cfg2.bits.fwd_iter_max_nr_2,
		reg_cfg->klt_klt_cfg.iter_param.iter_cfg2.bits.fwd_iter_max_nr_3,
		reg_cfg->klt_klt_cfg.iter_param.iter_cfg2.bits.fwd_iter_max_nr_4);
	logi("motion_epsilon: %d, min_eig_thresh: %d",
		reg_cfg->klt_klt_cfg.eval_param.bits.motion_epsilon, reg_cfg->klt_klt_cfg.eval_param.bits.min_eig_thresh);
	for (i = 0; i < 16; ++i)
		logi("kp_prev_x_y: %d", reg_cfg->klt_klt_cfg.prev_kp_coord.kp_prev_xy[i].bits.kp_prev_x_y);

	return;
}
#endif

static int seg_klt_cfg_cvdr_klt_ds_wr_vp(
	msg_req_klt_t *klt_req,
	seg_klt_cfg_t *seg_cfg_tab,
	klt_layers_e current_layer,
	unsigned int wr_stream_id)
{
	unsigned int i = 0, tmp = 0;
	pix_format_e format;
	unsigned int ds_nb_layer = klt_req->reg_cfg.klt_ds_cfg.bits.ds_nb_layer;
	cvdr_opt_fmt_t cfmt;
	if (memset_s(&cfmt, sizeof(cvdr_opt_fmt_t), 0, sizeof(cvdr_opt_fmt_t))) {
		loge("Failed : memset_s");
		return ISP_IPP_ERR;
	}

	for (i = 0; i < ds_nb_layer; i++) {
		switch (i) {
		case TAG_FOR_KLT_LAYER_0_VP_WR:
			cfmt.id = IPP_VP_WR_KLT_NEXT_LAYER0;
			break;
		case TAG_FOR_KLT_LAYER_1_VP_WR:
			cfmt.id = IPP_VP_WR_KLT_NEXT_LAYER1;
			break;
		case TAG_FOR_KLT_LAYER_2_VP_WR:
			cfmt.id = IPP_VP_WR_KLT_NEXT_LAYER2;
			break;
		case TAG_FOR_KLT_LAYER_3_VP_WR:
			cfmt.id = IPP_VP_WR_KLT_NEXT_LAYER3;
			break;
		case TAG_FOR_KLT_LAYER_4_VP_WR:
			cfmt.id = IPP_VP_WR_KLT_NEXT_LAYER4;
			break;
		default:
			loge("error: buf index = %d", i);
			return ISP_IPP_ERR;
		}

		if (klt_req->streams[wr_stream_id + i].buffer) {
			tmp = klt_req->streams[wr_stream_id + i].width / KLT_DS_BLK_H_SIZE;
			if ((klt_req->streams[wr_stream_id + i].width % KLT_DS_BLK_H_SIZE) != 0)
				tmp += 1;
			cfmt.width  = tmp * 64; // DS block8*8
			cfmt.height = align_up(klt_req->streams[wr_stream_id + i].height, 8) / 8; // DS block8*8
			cfmt.addr   = klt_req->streams[wr_stream_id + i].buffer;
			cfmt.expand = STD_PIX;
			cfmt.full_width = (i == 0) ? (cfmt.width / 2) : cfmt.width; // i = 0 is 2pf8
			format          = (i == 0) ? PIXEL_FMT_IPP_2PF8 : PIXEL_FMT_IPP_1PF8;

			loge_if(dataflow_cvdr_wr_cfg_vp(&seg_cfg_tab->klt_cvdr_cfg_tab[current_layer], &cfmt, 0, ISP_IPP_CLK, format));
		}
	}

	return ISP_IPP_OK;
}

static int seg_klt_cfg_ds_layer_vp_port(msg_req_klt_t *klt_req,
	seg_klt_cfg_t *seg_cfg_tab, unsigned int is_ds_pre)
{
	cvdr_opt_fmt_t cfmt;
	unsigned int rd_stream_id;
	unsigned int wr_stream_id;
	klt_layers_e current_layer;
	if (memset_s(&cfmt, sizeof(cvdr_opt_fmt_t), 0, sizeof(cvdr_opt_fmt_t))) {
		loge("Failed : memset_s");
		return ISP_IPP_ERR;
	}
	rd_stream_id = (is_ds_pre == 1) ? KLT_DS_PREV_IMG_BUFF : KLT_DS_NEXT_IMG_BUFF;
	wr_stream_id = (is_ds_pre == 1) ? KLT_PREV_LAYER0_BUFF : KLT_NEXT_LAYER0_BUFF;
	current_layer = (is_ds_pre == 1) ? LAYER_KLT_DS_FOR_PRE : LAYER_KLT_DS_FOR_NXT;

	if (klt_req->streams[rd_stream_id].buffer) {
		cfmt.id      = IPP_VP_RD_KLT_SRC;
		cfmt.width   = klt_req->streams[rd_stream_id].width;
		cfmt.height  = klt_req->streams[rd_stream_id].height;
		cfmt.addr    = klt_req->streams[rd_stream_id].buffer;
		cfmt.full_width = cfmt.width / 2;
		cfmt.line_size  = cfmt.width / 2; // 2pf8
		cfmt.expand     = STD_PIX;
		loge_if(dataflow_cvdr_rd_cfg_vp(&seg_cfg_tab->klt_cvdr_cfg_tab[current_layer], &cfmt, 0,
				ISP_IPP_CLK, PIXEL_FMT_IPP_2PF8));
	}
	loge_if(seg_klt_cfg_cvdr_klt_ds_wr_vp(klt_req, seg_cfg_tab, current_layer, wr_stream_id));

	return ISP_IPP_OK;
}

static int seg_klt_set_cvdr_cfg_tbl(
	msg_req_klt_t *klt_req,
	seg_klt_cfg_t *seg_cfg_tab)
{
	if (klt_req->work_mode == KLT_ONLY_DS) {
		seg_klt_cfg_ds_layer_vp_port(klt_req, seg_cfg_tab, 1);
	} else if (klt_req->work_mode == KLT_ONLY_KLT) {
		if (klt_req->streams[KLT_PREV_LAYER0_BUFF].buffer && klt_req->streams[KLT_NEXT_LAYER0_BUFF].buffer) {
			cfg_tbl_cvdr_nr_rd_cfg(&seg_cfg_tab->klt_cvdr_cfg_tab[LAYER_KLT_DO_KLT], IPP_NR_RD_KLT_PREV_FRAME);
			cfg_tbl_cvdr_nr_rd_cfg(&seg_cfg_tab->klt_cvdr_cfg_tab[LAYER_KLT_DO_KLT], IPP_NR_RD_KLT_NEXT_FRAME);
		}
	} else if (klt_req->work_mode == KLT_DS_AND_KLT) {
		seg_klt_cfg_ds_layer_vp_port(klt_req, seg_cfg_tab, 1);
		seg_klt_cfg_ds_layer_vp_port(klt_req, seg_cfg_tab, 0);
		if (klt_req->streams[KLT_PREV_LAYER0_BUFF].buffer && klt_req->streams[KLT_NEXT_LAYER0_BUFF].buffer) {
			cfg_tbl_cvdr_nr_rd_cfg(&seg_cfg_tab->klt_cvdr_cfg_tab[LAYER_KLT_DO_KLT], IPP_NR_RD_KLT_PREV_FRAME);
			cfg_tbl_cvdr_nr_rd_cfg(&seg_cfg_tab->klt_cvdr_cfg_tab[LAYER_KLT_DO_KLT], IPP_NR_RD_KLT_NEXT_FRAME);
		}
	}

	return ISP_IPP_OK;
}

static void seg_klt_set_rd_cfg_para(msg_req_klt_t *klt_req, klt_prev_and_next_rd_cfg_t *rd_cfg){
	if(klt_req->streams[KLT_PREV_LAYER0_BUFF].buffer != 0)
		rd_cfg->rd_prev_base_addr[0].bits.rd_prev_base_addr = klt_req->streams[KLT_PREV_LAYER0_BUFF].buffer >> 7;
	rd_cfg->rd_prev_line_stride[0].u32 = 0;

	if(klt_req->streams[KLT_PREV_LAYER1_BUFF].buffer != 0)
		rd_cfg->rd_prev_base_addr[1].bits.rd_prev_base_addr = klt_req->streams[KLT_PREV_LAYER1_BUFF].buffer >> 7;
	rd_cfg->rd_prev_line_stride[1].u32 = 0;

	if(klt_req->streams[KLT_PREV_LAYER2_BUFF].buffer != 0)
		rd_cfg->rd_prev_base_addr[2].bits.rd_prev_base_addr = klt_req->streams[KLT_PREV_LAYER2_BUFF].buffer >> 7;
	rd_cfg->rd_prev_line_stride[2].u32 = 0;

	if(klt_req->streams[KLT_PREV_LAYER3_BUFF].buffer != 0)
		rd_cfg->rd_prev_base_addr[3].bits.rd_prev_base_addr = klt_req->streams[KLT_PREV_LAYER3_BUFF].buffer >> 7;
	rd_cfg->rd_prev_line_stride[3].u32 = 0;

	if(klt_req->streams[KLT_PREV_LAYER4_BUFF].buffer != 0)
		rd_cfg->rd_prev_base_addr[4].bits.rd_prev_base_addr = klt_req->streams[KLT_PREV_LAYER4_BUFF].buffer >> 7;
	rd_cfg->rd_prev_line_stride[4].u32 = 0;

	if(klt_req->streams[KLT_NEXT_LAYER0_BUFF].buffer != 0)
		rd_cfg->rd_next_base_addr[0].bits.rd_next_base_addr = klt_req->streams[KLT_NEXT_LAYER0_BUFF].buffer >> 7;
	rd_cfg->rd_next_line_stride[0].u32 = 0;

	if(klt_req->streams[KLT_NEXT_LAYER1_BUFF].buffer != 0)
		rd_cfg->rd_next_base_addr[1].bits.rd_next_base_addr = klt_req->streams[KLT_NEXT_LAYER1_BUFF].buffer >> 7;
	rd_cfg->rd_next_line_stride[1].u32 = 0;

	if(klt_req->streams[KLT_NEXT_LAYER2_BUFF].buffer != 0)
		rd_cfg->rd_next_base_addr[2].bits.rd_next_base_addr = klt_req->streams[KLT_NEXT_LAYER2_BUFF].buffer >> 7;
	rd_cfg->rd_next_line_stride[2].u32 = 0;

	if(klt_req->streams[KLT_NEXT_LAYER3_BUFF].buffer != 0)
		rd_cfg->rd_next_base_addr[3].bits.rd_next_base_addr = klt_req->streams[KLT_NEXT_LAYER3_BUFF].buffer >> 7;
	rd_cfg->rd_next_line_stride[3].u32 = 0;

	if(klt_req->streams[KLT_NEXT_LAYER4_BUFF].buffer != 0)
		rd_cfg->rd_next_base_addr[4].bits.rd_next_base_addr = klt_req->streams[KLT_NEXT_LAYER4_BUFF].buffer >> 7;
	rd_cfg->rd_next_line_stride[4].u32 = 0;

	return;
}

static int seg_klt_set_klt_cfg_tbl(msg_req_klt_t *klt_req, seg_klt_cfg_t *seg_cfg_tab)
{
	switch (klt_req->work_mode) {
	case KLT_ONLY_DS:
		if (memcpy_s(&seg_cfg_tab->klt_cfg_tab[LAYER_KLT_DS_FOR_PRE].klt_ds_cfg, sizeof(klt_ds_cfg_t),
					&klt_req->reg_cfg.klt_ds_cfg, sizeof(klt_ds_cfg_t))) {
			loge("Failed : LAYER_KLT_DS_FOR_PRE");
			return ISP_IPP_ERR;
		}

		break;
	case KLT_ONLY_KLT:
		if (memcpy_s(&seg_cfg_tab->klt_cfg_tab[LAYER_KLT_DO_KLT].klt_klt_cfg, sizeof(klt_klt_cfg_t),
					&klt_req->reg_cfg.klt_klt_cfg, sizeof(klt_klt_cfg_t))) {
			loge("Failed : LAYER_KLT_DO_KLT");
			return ISP_IPP_ERR;
		}
		seg_klt_set_rd_cfg_para(klt_req, &seg_cfg_tab->klt_cfg_tab[LAYER_KLT_DO_KLT].klt_klt_cfg.rd_cfg);

		break;
	case KLT_DS_AND_KLT:
		if (memcpy_s(&seg_cfg_tab->klt_cfg_tab[LAYER_KLT_DS_FOR_PRE].klt_ds_cfg, sizeof(klt_ds_cfg_t),
					&klt_req->reg_cfg.klt_ds_cfg, sizeof(klt_ds_cfg_t))) {
			loge("Failed : LAYER_KLT_DS_FOR_PRE");
			return ISP_IPP_ERR;
		}
		if (memcpy_s(&seg_cfg_tab->klt_cfg_tab[LAYER_KLT_DS_FOR_NXT].klt_ds_cfg, sizeof(klt_ds_cfg_t),
					&klt_req->reg_cfg.klt_ds_cfg, sizeof(klt_ds_cfg_t))) {
			loge("Failed : LAYER_KLT_DS_FOR_NXT");
			return ISP_IPP_ERR;
		}
		if (memcpy_s(&seg_cfg_tab->klt_cfg_tab[LAYER_KLT_DO_KLT].klt_klt_cfg, sizeof(klt_klt_cfg_t),
					&klt_req->reg_cfg.klt_klt_cfg, sizeof(klt_klt_cfg_t))) {
			loge("Failed : LAYER_KLT_DO_KLT");
			return ISP_IPP_ERR;
		}
		seg_klt_set_rd_cfg_para(klt_req, &seg_cfg_tab->klt_cfg_tab[LAYER_KLT_DO_KLT].klt_klt_cfg.rd_cfg);

		break;
	default:
		loge("error klt mode.%d", klt_req->work_mode);
		return ISP_IPP_ERR;
	}
	return ISP_IPP_OK;
}

static int seg_klt_set_cfg_tbl(
	msg_req_klt_t *klt_req,
	seg_klt_cfg_t *seg_cfg_tab,
	global_info_ipp_t *global_info)
{
	int ret = 0;
	ret = seg_klt_set_klt_cfg_tbl(klt_req, seg_cfg_tab);
	if (ret != 0) {
		loge("Failed : seg_klt_set_klt_cfg_tbl");
		return ISP_IPP_ERR;
	}

	ret = seg_klt_set_cvdr_cfg_tbl(klt_req, seg_cfg_tab);
	if (ret != 0) {
		loge("Failed : seg_klt_set_cvdr_cfg_tbl");
		return ISP_IPP_ERR;
	}

	return ISP_IPP_OK;
}

static void seg_klt_cfg_cmdlst_stripe_common_para(cmdlst_stripe_info_t *cmdlst_stripe, unsigned int i)
{
	cmdlst_stripe[i].resource_share  = 1 << IPP_CMD_RES_SHARE_KLT;
	cmdlst_stripe[i].irq_module  = 1 << IPP_CMD_IRQ_MODULE_KLT;
	cmdlst_stripe[i].en_link		 = 0;
	cmdlst_stripe[i].ch_link		 = 0;
	cmdlst_stripe[i].ch_link_act_nbr = 0;
	cmdlst_stripe[i].is_last_stripe  = 0;
	cmdlst_stripe[i].is_need_set_sop = 0;
	cmdlst_stripe[i].hw_priority	 = CMD_PRIO_HIGH;

	return;
}

static int seg_klt_get_ds_function_irq_info(
	msg_req_klt_t *klt_req,
	cmdlst_stripe_info_t *cmdlst_stripe,
	unsigned int stripe_idx)
{
	unsigned int irq = 0;
	// ds layer no IPP_KLT_DONE_IRQ irq;
	irq = irq | (((unsigned int)(1)) << IPP_KLT_CVDR_VP_RD_EOF_CMDLST);
	irq = irq | (((unsigned int)(1)) << IPP_KLT_CVDR_VP_RD_EOF_KLT_SRC);

	switch (klt_req->reg_cfg.klt_ds_cfg.bits.ds_nb_layer - 1) {
	case TAG_FOR_KLT_LAYER_4_VP_WR:
		irq = irq | (((unsigned int)(1)) << IPP_KLT_CVDR_VP_WR_EOF_NEXT_LAYER_4);
	case TAG_FOR_KLT_LAYER_3_VP_WR:
		irq = irq | (((unsigned int)(1)) << IPP_KLT_CVDR_VP_WR_EOF_NEXT_LAYER_3);
	case TAG_FOR_KLT_LAYER_2_VP_WR:
		irq = irq | (((unsigned int)(1)) << IPP_KLT_CVDR_VP_WR_EOF_NEXT_LAYER_2);
	case TAG_FOR_KLT_LAYER_1_VP_WR:
		irq = irq | (((unsigned int)(1)) << IPP_KLT_CVDR_VP_WR_EOF_NEXT_LAYER_1);
	case TAG_FOR_KLT_LAYER_0_VP_WR:
		irq = irq | (((unsigned int)(1)) << IPP_KLT_CVDR_VP_WR_EOF_NEXT_LAYER_0);
		break;
	default:
		loge("error: ds_nb_layer = %d", klt_req->reg_cfg.klt_ds_cfg.bits.ds_nb_layer);
		return ISP_IPP_ERR;
	}

	cmdlst_stripe[stripe_idx].irq_mode = irq;
	logd("klt_stripe_idx=%d, irq=0x%x", stripe_idx, irq);

	return ISP_IPP_OK;
}

static int seg_klt_get_klt_function_irq_info(
	msg_req_klt_t *klt_req,
	cmdlst_stripe_info_t *cmdlst_stripe,
	unsigned int stripe_idx)
{
	unsigned int irq = 0;
	irq = irq | (((unsigned int)(1)) << IPP_KLT_CVDR_VP_RD_EOF_CMDLST);
	irq = irq | (((unsigned int)(1)) << IPP_KLT_DONE_IRQ);
	cmdlst_stripe[stripe_idx].irq_mode = irq;
	logd("klt_stripe_idx=%d, irq=0x%x", stripe_idx, irq);

	return ISP_IPP_OK;
}

static int seg_klt_set_cmdlst_stripe(
	msg_req_klt_t *klt_req,
	cmdlst_para_t *cmdlst_para,
	global_info_ipp_t *global_info)
{
	unsigned int i = 0;
	unsigned int irq = 0;
	cmdlst_stripe_info_t *cmdlst_stripe = cmdlst_para->cmd_stripe_info;
	klt_work_mode_e work_mode = klt_req->work_mode;
	unsigned int stripe_cnt = (work_mode == KLT_DS_AND_KLT) ?
		(CFG_KLT_DS_FOR_PREV + CFG_KLT_DS_FOR_NEXT + CFG_KLT_FOR_KLT) : 1;

	if (work_mode == KLT_ONLY_KLT || work_mode == KLT_DS_AND_KLT) {
		if (klt_req->klt_stat_buff[KLT_KP_PREV_BUFF].buffer) stripe_cnt++;
		if (klt_req->klt_stat_buff[KLT_KP_NEXT_BUFF].buffer) stripe_cnt++;
	}

	logd("KLT, total_stripe_cnt = %d", stripe_cnt);
	global_info->klt_stripe_cnt = stripe_cnt;
	switch (work_mode) {
	case KLT_ONLY_DS:
		seg_klt_cfg_cmdlst_stripe_common_para(cmdlst_stripe, 0);
		loge_if(seg_klt_get_ds_function_irq_info(klt_req, cmdlst_stripe, 0));
		break;
	case KLT_ONLY_KLT:
		for (i = 0; i < stripe_cnt; i++) {
			seg_klt_cfg_cmdlst_stripe_common_para(cmdlst_stripe, i);
			if (i == 0) { // cfg klt_klt part
				loge_if(seg_klt_get_klt_function_irq_info(klt_req, cmdlst_stripe, i));
			} else { // get stat data
				irq = irq | (((unsigned int)(1)) << IPP_KLT_CVDR_VP_RD_EOF_CMDLST);
				irq = irq | (((unsigned int)(1)) << IPP_KLT_CVDR_VP_WR_EOF_CMDLST);
				cmdlst_stripe[i].irq_mode = irq;
				logd("klt_stripe_idx=%d, irq=0x%x", i, irq);
			}
		}
		break;
	case KLT_DS_AND_KLT:
		for (i = 0; i < stripe_cnt; i++) {
			seg_klt_cfg_cmdlst_stripe_common_para(cmdlst_stripe, i);
			if (i == LAYER_KLT_DS_FOR_PRE || i == LAYER_KLT_DS_FOR_NXT) { // cfg klt_DS part
				loge_if(seg_klt_get_ds_function_irq_info(klt_req, cmdlst_stripe, i));
			} else if (i == LAYER_KLT_DO_KLT) { // cfg klt_KLT part
				loge_if(seg_klt_get_klt_function_irq_info(klt_req, cmdlst_stripe, i));
			} else { // get stat data
				irq = irq | (((unsigned int)(1)) << IPP_KLT_CVDR_VP_RD_EOF_CMDLST);
				irq = irq | (((unsigned int)(1)) << IPP_KLT_CVDR_VP_WR_EOF_CMDLST);
				cmdlst_stripe[i].irq_mode = irq;
				logd("klt_stripe_idx=%d, irq=0x%x", i, irq);
			}
		}
		break;
	default:
		loge("Failed: error klt work_mode.%d", work_mode);
		return ISP_IPP_ERR;
	}

	cmdlst_stripe[stripe_cnt - 1].is_last_stripe = 1;
	cmdlst_stripe[0].hw_priority = CMD_PRIO_LOW;

	cmdlst_para->channel_id = global_info->klt_channel;
	cmdlst_para->stripe_cnt = stripe_cnt;
	cmdlst_para->cmd_entry_mem_id = MEM_ID_CMDLST_ENTRY_KLT;
	cmdlst_para->cmd_buf_mem_id = MEM_ID_CMDLST_BUF_KLT;
	cmdlst_para->work_module = global_info->work_module;

	return ISP_IPP_OK;
}

static int seg_klt_set_stat_data_cmdlst_buf(
	msg_req_klt_t *klt_req,
	schedule_cmdlst_link_t *cmd_link_entry,
	int *index)
{
	int ret = 0;
	cmdlst_wr_cfg_t cmdlst_wr_cfg;
	if (memset_s(&cmdlst_wr_cfg, sizeof(cmdlst_wr_cfg_t), 0, sizeof(cmdlst_wr_cfg_t))) {
		loge("Failed : memset_s");
		return ISP_IPP_ERR;
	}
	cmdlst_wr_cfg.is_incr = 1;
	cmdlst_wr_cfg.read_mode = 0;

	if (klt_req->klt_stat_buff[KLT_KP_PREV_BUFF].buffer) {
		cmdlst_wr_cfg.data_size = KLT_KP_NUM_MAX + KLT_KP_NUM_MAX; // kp_prev_x + kp_prev_y
		cmdlst_wr_cfg.is_wstrb  = 0;
		cmdlst_wr_cfg.reg_rd_addr = IPP_BASE_ADDR_KLT + KLT_KP_PREV_BUFF_X_0_REG;
		cmdlst_wr_cfg.buff_wr_addr = klt_req->klt_stat_buff[KLT_KP_PREV_BUFF].buffer;
		loge_if_set_ret(seg_ipp_set_cmdlst_wr_buf(&cmd_link_entry[(*index)++].cmd_buf, &cmdlst_wr_cfg), ret);
		if (ret) return ISP_IPP_ERR;
	}

	if (klt_req->klt_stat_buff[KLT_KP_NEXT_BUFF].buffer) {
		cmdlst_wr_cfg.data_size = KLT_KP_NUM_MAX + KLT_KP_NUM_MAX; // kp_next_x + kp_next_y
		cmdlst_wr_cfg.is_wstrb  = 0;
		cmdlst_wr_cfg.reg_rd_addr = IPP_BASE_ADDR_KLT + KLT_KP_NEXT_BUFF_X_0_REG;
		cmdlst_wr_cfg.buff_wr_addr = klt_req->klt_stat_buff[KLT_KP_NEXT_BUFF].buffer;
		loge_if_set_ret(seg_ipp_set_cmdlst_wr_buf(&cmd_link_entry[(*index)++].cmd_buf, &cmdlst_wr_cfg), ret);
		if (ret) return ISP_IPP_ERR;
	}

	logd("total of %d stripes are configured", *index);
	return ISP_IPP_OK;
}

static int seg_klt_set_cmdlst_buf(
	msg_req_klt_t *klt_req,
	cmdlst_para_t *cmdlst_para,
	seg_klt_cfg_t *seg_cfg_tab,
	global_info_ipp_t *global_info)
{
	unsigned int i = 0;
	int ret = 0;
	klt_work_mode_e work_mode = klt_req->work_mode;
	schedule_cmdlst_link_t *cmd_link_entry = cmdlst_para->cmd_entry;

	switch (work_mode) {
	case KLT_ONLY_DS:
		loge_if_set_ret(klt_prepare_ds_cmd(&g_klt_devs[0], &cmd_link_entry[i].cmd_buf,
								&(seg_cfg_tab->klt_cfg_tab[LAYER_KLT_DS_FOR_PRE].klt_ds_cfg)), ret);
		if (ret) return ISP_IPP_ERR;

		loge_if_set_ret(cvdr_prepare_cmd(&g_cvdr_devs[0], &cmd_link_entry[i++].cmd_buf,
								&(seg_cfg_tab->klt_cvdr_cfg_tab[LAYER_KLT_DS_FOR_PRE])), ret);
		if (ret) return ISP_IPP_ERR;

		break;
	case KLT_ONLY_KLT:
		loge_if_set_ret(cvdr_prepare_nr_cmd(&g_cvdr_devs[0], &cmd_link_entry[i].cmd_buf,
								&(seg_cfg_tab->klt_cvdr_cfg_tab[LAYER_KLT_DO_KLT])), ret);
		if (ret) return ISP_IPP_ERR;

		loge_if_set_ret(klt_prepare_klt_cmd(&g_klt_devs[0], &cmd_link_entry[i].cmd_buf,
								&(seg_cfg_tab->klt_cfg_tab[LAYER_KLT_DO_KLT])), ret);
		if (ret) return ISP_IPP_ERR;

		loge_if_set_ret(klt_klt_start_cmd(&g_klt_devs[0], &cmd_link_entry[i++].cmd_buf,
								&(seg_cfg_tab->klt_cfg_tab[LAYER_KLT_DO_KLT].klt_klt_cfg.klt_start)), ret);
		if (ret) return ISP_IPP_ERR;

		// read stat data
		loge_if_set_ret(seg_klt_set_stat_data_cmdlst_buf(klt_req, cmd_link_entry, &i), ret);
		if (ret) return ISP_IPP_ERR;

		break;
	case KLT_DS_AND_KLT:
		loge_if_set_ret(klt_prepare_ds_cmd(&g_klt_devs[0], &cmd_link_entry[i].cmd_buf,
								&(seg_cfg_tab->klt_cfg_tab[LAYER_KLT_DS_FOR_PRE].klt_ds_cfg)), ret);
		if (ret) return ISP_IPP_ERR;

		loge_if_set_ret(cvdr_prepare_cmd(&g_cvdr_devs[0], &cmd_link_entry[i++].cmd_buf,
								&(seg_cfg_tab->klt_cvdr_cfg_tab[LAYER_KLT_DS_FOR_PRE])), ret);
		if (ret) return ISP_IPP_ERR;

		loge_if_set_ret(klt_prepare_ds_cmd(&g_klt_devs[0], &cmd_link_entry[i].cmd_buf,
								&(seg_cfg_tab->klt_cfg_tab[LAYER_KLT_DS_FOR_NXT].klt_ds_cfg)), ret);
		if (ret) return ISP_IPP_ERR;

		loge_if_set_ret(cvdr_prepare_cmd(&g_cvdr_devs[0], &cmd_link_entry[i++].cmd_buf,
								&(seg_cfg_tab->klt_cvdr_cfg_tab[LAYER_KLT_DS_FOR_NXT])), ret);
		if (ret) return ISP_IPP_ERR;

		loge_if_set_ret(cvdr_prepare_nr_cmd(&g_cvdr_devs[0], &cmd_link_entry[i].cmd_buf,
								&(seg_cfg_tab->klt_cvdr_cfg_tab[LAYER_KLT_DO_KLT])), ret);
		if (ret) return ISP_IPP_ERR;

		loge_if_set_ret(klt_prepare_klt_cmd(&g_klt_devs[0], &cmd_link_entry[i].cmd_buf,
								&(seg_cfg_tab->klt_cfg_tab[LAYER_KLT_DO_KLT])), ret);
		if (ret) return ISP_IPP_ERR;

		loge_if_set_ret(klt_klt_start_cmd(&g_klt_devs[0], &cmd_link_entry[i++].cmd_buf,
								&(seg_cfg_tab->klt_cfg_tab[LAYER_KLT_DO_KLT].klt_klt_cfg.klt_start)),
								ret);
		if (ret) return ISP_IPP_ERR;

		// read stat data
		loge_if_set_ret(seg_klt_set_stat_data_cmdlst_buf(klt_req, cmd_link_entry, &i), ret);
		if (ret) return ISP_IPP_ERR;

		break;
	default:
		loge("Failed: error klt work_mode.%d", work_mode);
		return ISP_IPP_ERR;
	}

	if (i != cmdlst_para->stripe_cnt) {
		loge("Failed: stripe cnt error, i = %u, stripe_cnt = %u", i, cmdlst_para->stripe_cnt);
		return ISP_IPP_ERR;
	}

	return ISP_IPP_OK;
}

static int seg_klt_prepare_cmdlst_para(
	msg_req_klt_t *klt_req,
	cmdlst_para_t *cmdlst_para,
	seg_klt_cfg_t *seg_cfg_tab,
	global_info_ipp_t *global_info)
{
	int ret = 0;

	ret = seg_klt_set_cmdlst_stripe(klt_req, cmdlst_para, global_info);
	if (ret != 0) {
		loge("Failed : seg_klt_set_cmdlst_stripe");
		return ISP_IPP_ERR;
	}

	ret = df_sched_prepare(cmdlst_para);
	if (ret != 0) {
		loge("Failed : df_sched_prepare");
		return ISP_IPP_ERR;
	}
	ret = seg_klt_set_cmdlst_buf(klt_req, cmdlst_para, seg_cfg_tab, global_info);
	if (ret != 0) {
		loge("Failed : seg_klt_set_cmdlst_buf");
		return ISP_IPP_ERR;
	}

	return ISP_IPP_OK;
}

static int seg_klt_get_cpe_mem(seg_klt_cfg_t **seg_klt_cfg, cmdlst_para_t **cmdlst_para,
	unsigned int work_module)
{
	unsigned long long va = 0;
	unsigned int da = 0;
	int ret = 0;
	// 1. klt cfg_tbl
	ret = cpe_mem_get(MEM_ID_KLT_CFG_TAB, work_module, &va, &da);
	if (ret != 0) {
		loge("Failed : cpe_mem_get %d", MEM_ID_KLT_CFG_TAB);
		return ISP_IPP_ERR;
	}

	*seg_klt_cfg = (seg_klt_cfg_t *)(uintptr_t)va;

	// 2. klt cmdlst_para
	ret = cpe_mem_get(MEM_ID_CMDLST_PARA_KLT, work_module, &va, &da);
	if (ret != 0) {
		loge("Failed : klt, cpe_mem_get %d", MEM_ID_CMDLST_PARA_KLT);
		goto out;
	}

	*cmdlst_para = (cmdlst_para_t *)(uintptr_t)va;

	return ISP_IPP_OK;
out:
	cpe_mem_free(MEM_ID_KLT_CFG_TAB, work_module);
	cpe_mem_free(MEM_ID_CMDLST_PARA_KLT, work_module);
	*seg_klt_cfg = NULL;
	*cmdlst_para = NULL;
	return ISP_IPP_ERR;
}

int seg_klt_request_handler(
	msg_req_klt_t *klt_req,
	global_info_ipp_t *global_info)
{
	int ret = 0;
	seg_klt_cfg_t *seg_klt_cfg = NULL;
	cmdlst_para_t *p_klt_cmdlst_para = NULL;

	if (klt_req->work_mode == KLT_OFF) {
		logi("klt's work_mode is KLT_OFF, do nothing");
		return ISP_IPP_OK;
	}

	if (seg_klt_check_req_param(klt_req)) {
		loge("Failed: seg_klt_check_req_param");
		return ISP_IPP_ERR;
	}
#ifdef IPP_UT_DEBUG
	seg_klt_dump_req(klt_req);
#endif
#ifdef IPP_UT_DIO_DEBUG
	set_dump_register_init(UT_REG_ADDR, IPP_MAX_REG_OFFSET, 0);
#endif

	ret = seg_klt_get_cpe_mem(&seg_klt_cfg, &p_klt_cmdlst_para, global_info->work_module);
	if (ret != 0) {
		loge("Failed: seg_klt_get_cpe_mem");
		return ISP_IPP_ERR;
	}

	ret = seg_klt_set_cfg_tbl(klt_req, seg_klt_cfg, global_info);
	if (ret != 0) {
		loge("Failed: seg_klt_set_cfg_tbl");
		goto SEG_KLT_BUFF_FREE;
	}

	ret = seg_klt_prepare_cmdlst_para(klt_req, p_klt_cmdlst_para, seg_klt_cfg, global_info);
	if (ret != 0) {
		loge("Failed: seg_klt_prepare_cmdlst_para");
		goto SEG_KLT_BUFF_FREE;
	}

	ret = df_sched_start(p_klt_cmdlst_para);
	if (ret != 0)
		loge("Failed: df_sched_start");

SEG_KLT_BUFF_FREE:
	cpe_mem_free(MEM_ID_KLT_CFG_TAB, global_info->work_module);
	return ret;
}

// end file

