/*******************************************************************
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2018. All rights reserved.
 * File name    segment_enh_arfeature.c
 * Description:
 * Date         2022-07-20
 ******************************************************************/
#include <linux/string.h>
#include <linux/printk.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/io.h>
#include "ipp.h"
#include "memory.h"
#include "segment_enh_arfeature.h"
#include "cmdlst_manager.h"
#include "orb_enh_drv.h"
#include "arfeature_drv.h"
#include "cvdr_drv.h"
#include "cmdlst_drv.h"
#include "cvdr_opt.h"
#include "ipp_top_reg_offset.h"
#include "cfg_table_ipp_path.h"
#include "arfeature_reg_offset.h"


#define LOG_TAG LOG_MODULE_ENH_ARFEATURE

#define CFG_ARF                                 (1)
#define UPDATA_NEXT_LAYER_VPWR_CFG              (1) // Update the output address of the VPWR port.
#define GET_STAT_MINSCR_KPTCNT                  (1)
#define GET_STAT_SUM_SCORE                      (1)

// rdr
#define UPDATE_RDR_TOTAL_KPT_NUM_CFG            (1)
#define UPDATE_RDR_KPT_NUMBER_CFG               (1)
#define UPDATE_RDR_CVDR_RD_FRAME_HEIGHT_CFG     (1)
// cmp
#define UPDATE_CMP_TOTAL_KPT_NUM_CFG            (1)
#define UPDATE_CMP_CUR_KPT_NUMBER_CFG           (1)
#define UPDATE_CMP_REF_KPT_NUMBER_CFG           (1)
#define UPDATE_CMP_CVDR_VP_RD_FRAME_HEIGHT_CFG  (1)

typedef struct _seg_enh_arf_stripe_t {
	unsigned int detect_total_stripe; // Records the number of stripes at the detect layer.
	unsigned int gauss_total_stripe; // Records the number of stripes at the gauss layer.
	unsigned int layer_stripe_num; // Records the number of stripes at each Detect layer.
	unsigned int cur_frame_stripe_cnt; // cur only
	unsigned int total_stripe_num; // cur+ref  = 2*cur+2

	unsigned int channel_id;
} seg_enh_arf_stripe_t;


#ifdef IPP_UT_DEBUG
static void seg_enh_arf_dump_req(msg_req_enh_arf_t* req);
static void seg_enh_arf_dump_arf_req(msg_req_arf_t *req_arf, unsigned int mode_cnt);
static void seg_enh_arf_dump_enh_req(msg_req_enh_t *req_enh);
#endif

static int ipp_top_do_config(cmd_buf_t *cmd_buf, global_info_ipp_t *global_info)
{
	unsigned int enh_vpb_value = 0;
	if (global_info->enh_en == 1) { // new : Set by cmdlst.
		if (global_info->arf_en == 1)
			enh_vpb_value = CONNECT_TO_ARF | CONNECT_TO_CVDR;
		else
			enh_vpb_value = CONNECT_TO_CVDR;

		cmdlst_set_reg(cmd_buf, IPP_BASE_ADDR_TOP + IPP_TOP_ENH_VPB_CFG_REG, enh_vpb_value);
	}
	return ISP_IPP_OK;
}

static int seg_enh_arf_cfg_arf_cvdr_rd_port_0_1(msg_req_arf_t *req_arf,
		cfg_tab_cvdr_t *cvdr_cfg_tab, unsigned int stripe_index)
{
	cvdr_opt_fmt_t cfmt;
	pix_format_e format = PIXEL_FMT_IPP_MAX;
	unsigned int line_stride = 0;
	if (memset_s(&cfmt, sizeof(cvdr_opt_fmt_t), 0, sizeof(cvdr_opt_fmt_t))) {
		loge("Failed : memset_s");
		return ISP_IPP_ERR;
	}

	if (req_arf->streams[stripe_index][BI_ARFEATURE_0].buffer) {
		cfmt.id = IPP_VP_RD_ARF_0;
		cfmt.width = req_arf->streams[stripe_index][BI_ARFEATURE_0].width;
		cfmt.height = req_arf->streams[stripe_index][BI_ARFEATURE_0].height;
		cfmt.full_width = cfmt.width / 2;
		cfmt.line_size = cfmt.width / 2; // 2PF16 or 2PF8
		cfmt.addr = req_arf->streams[stripe_index][BI_ARFEATURE_0].buffer;
		cfmt.expand = 0;

		if ((req_arf->reg_cfg[stripe_index].top_cfg.bits.mode == 0) ||
			(req_arf->reg_cfg[stripe_index].top_cfg.bits.mode == 1)) {
			format = PIXEL_FMT_IPP_2PF8;
		} else {
			format = PIXEL_FMT_IPP_D32; // Actually, it should be 2PF16.
		}
		line_stride = 0;
		if (dataflow_cvdr_rd_cfg_vp(cvdr_cfg_tab, &cfmt, line_stride, ISP_IPP_CLK, format)) {
			loge("Failed: dataflow_cvdr_rd_cfg_vp");
			return ISP_IPP_ERR;
		}
	}

	if (req_arf->streams[stripe_index][BI_ARFEATURE_1].buffer) {
		cfmt.id = IPP_VP_RD_ARF_1;
		cfmt.width = req_arf->streams[stripe_index][BI_ARFEATURE_1].width;
		cfmt.height = req_arf->streams[stripe_index][BI_ARFEATURE_1].height;
		cfmt.full_width = cfmt.width / 2;
		cfmt.line_size = cfmt.width / 2;
		cfmt.addr = req_arf->streams[stripe_index][BI_ARFEATURE_1].buffer;
		cfmt.expand = 0;
		format = PIXEL_FMT_IPP_D32;
		line_stride = 0;
		if (dataflow_cvdr_rd_cfg_vp(cvdr_cfg_tab, &cfmt, line_stride, ISP_IPP_CLK, format)) {
			loge("Failed: dataflow_cvdr_rd_cfg_vp");
			return ISP_IPP_ERR;
		}
	}

	return ISP_IPP_OK;
}

static int seg_enh_arf_cfg_arf_cvdr_rd_port_2_3(msg_req_arf_t *req_arf,
		cfg_tab_cvdr_t *cvdr_cfg_tab, unsigned int stripe_index)
{
	cvdr_opt_fmt_t cfmt;
	pix_format_e format = PIXEL_FMT_IPP_MAX;
	unsigned int line_stride = 0;
	if (memset_s(&cfmt, sizeof(cvdr_opt_fmt_t), 0, sizeof(cvdr_opt_fmt_t))) {
		loge("Failed : memset_s");
		return ISP_IPP_ERR;
	}

	if (req_arf->streams[stripe_index][BI_ARFEATURE_2].buffer) {
		cfmt.id = IPP_VP_RD_ARF_2;
		cfmt.width = req_arf->streams[stripe_index][BI_ARFEATURE_2].width;
		cfmt.height = req_arf->streams[stripe_index][BI_ARFEATURE_2].height;
		cfmt.full_width = cfmt.width / 2;
		cfmt.line_size = cfmt.width / 2;
		cfmt.addr = req_arf->streams[stripe_index][BI_ARFEATURE_2].buffer;
		cfmt.expand = 0;
		format = PIXEL_FMT_IPP_D32;
		line_stride = 0;
		if (dataflow_cvdr_rd_cfg_vp(cvdr_cfg_tab, &cfmt, line_stride, ISP_IPP_CLK, format)) {
			loge("Failed: dataflow_cvdr_rd_cfg_vp");
			return ISP_IPP_ERR;
		}
	}

	if (req_arf->streams[stripe_index][BI_ARFEATURE_3].buffer) {
		cfmt.id = IPP_VP_RD_ARF_3;
		cfmt.width = req_arf->streams[stripe_index][BI_ARFEATURE_3].width;
		cfmt.height = req_arf->streams[stripe_index][BI_ARFEATURE_3].height;
		cfmt.full_width = cfmt.width / 2;
		cfmt.line_size = cfmt.width / 2;
		cfmt.addr = req_arf->streams[stripe_index][BI_ARFEATURE_3].buffer;
		format = PIXEL_FMT_IPP_D32;
		cfmt.expand = 0;
		line_stride = 0;
		if (dataflow_cvdr_rd_cfg_vp(cvdr_cfg_tab, &cfmt, line_stride, ISP_IPP_CLK, format)) {
			loge("Failed: dataflow_cvdr_rd_cfg_vp error");
			return ISP_IPP_ERR;
		}
	}

	return ISP_IPP_OK;
}

static int seg_enh_arf_cfg_arf_cvdr_wr_port_pyr_1_2(msg_req_arf_t *req_arf,
		cfg_tab_cvdr_t *cvdr_cfg_tab, unsigned int stripe_index)
{
	cvdr_opt_fmt_t cfmt;
	pix_format_e format = PIXEL_FMT_IPP_MAX;
	unsigned int line_stride = 0;
	unsigned int arf_mode = req_arf->reg_cfg[stripe_index].top_cfg.bits.mode;

	if (memset_s(&cfmt, sizeof(cvdr_opt_fmt_t), 0, sizeof(cvdr_opt_fmt_t))) {
		loge("Failed : memset_s");
		return ISP_IPP_ERR;
	}

	if (arf_mode < DETECT_AND_DESC_AND_MFPDA_DISABLE) {
		if (req_arf->streams[stripe_index][BO_ARFEATURE_PYR_1].buffer) {
			cfmt.id = IPP_VP_WR_ARF_PRY_1;
			cfmt.width = req_arf->streams[stripe_index][BO_ARFEATURE_PYR_1].width;
			cfmt.height = req_arf->streams[stripe_index][BO_ARFEATURE_PYR_1].height;
			cfmt.full_width = cfmt.width / 2; // 2PF16
			cfmt.line_size = cfmt.width / 2;
			cfmt.addr = req_arf->streams[stripe_index][BO_ARFEATURE_PYR_1].buffer;
			cfmt.expand = 0;
			format = PIXEL_FMT_IPP_D32;
			line_stride = 0;
			if (dataflow_cvdr_wr_cfg_vp(cvdr_cfg_tab, &cfmt, line_stride, ISP_IPP_CLK, format)) {
				loge("Failed: dataflow_cvdr_wr_cfg_vp");
				return ISP_IPP_ERR;
			}
		}
	} else if ((arf_mode == DETECT_AND_DESC_AND_MFPDA_DISABLE || arf_mode == DETECT_AND_DESC_AND_MFPDA_ENABLE) &&
		req_arf->arf_stat_buff[ARF_DESC_BUFF].buffer != 0) {
		cfmt.id = IPP_VP_WR_ARF_PRY_1;
		cfmt.addr = req_arf->arf_stat_buff[ARF_DESC_BUFF].buffer;
		if (cfg_tbl_cvdr_wr_cfg_d64(cvdr_cfg_tab, &cfmt, ARF_OUT_DESC_SIZE)) {
			loge("Failed: cfg_tbl_cvdr_wr_cfg_d64");
			return ISP_IPP_ERR;
		}
	} else if (arf_mode == DETECT_AND_KLT_AND_MFPDA_ENABLE && req_arf->arf_stat_buff[ARF_DESC_BUFF].buffer != 0) {
		// KLT_PRE case, only mode 7 output coodr data;
		logd("[liang]do klt_pre, arf_mode.%d", arf_mode);
		cfmt.id = IPP_VP_WR_ARF_PRY_1;
		cfmt.addr = req_arf->arf_stat_buff[ARF_DESC_BUFF].buffer;
		if (cfg_tbl_cvdr_wr_cfg_d64(cvdr_cfg_tab, &cfmt, ARF_OUT_COORDINATE_SIZE)) {
			loge("Failed: cfg_tbl_cvdr_wr_cfg_d64");
			return ISP_IPP_ERR;
		}
	}

	if (req_arf->streams[stripe_index][BO_ARFEATURE_PYR_2].buffer) {
		cfmt.id = IPP_VP_WR_ARF_PRY_2;
		cfmt.width = req_arf->streams[stripe_index][BO_ARFEATURE_PYR_2].width;
		cfmt.height = req_arf->streams[stripe_index][BO_ARFEATURE_PYR_2].height;
		cfmt.full_width = cfmt.width / 2;
		cfmt.line_size = cfmt.width / 2;
		cfmt.addr = req_arf->streams[stripe_index][BO_ARFEATURE_PYR_2].buffer;
		cfmt.expand = 0;
		format = PIXEL_FMT_IPP_D32;
		line_stride = 0;
		if (dataflow_cvdr_wr_cfg_vp(cvdr_cfg_tab, &cfmt, line_stride, ISP_IPP_CLK, format)) {
			loge("Failed: IPP_VP_WR_ARF_PRY_2, dataflow_cvdr_wr_cfg_vp");
			return ISP_IPP_ERR;
		}
	}

	return ISP_IPP_OK;
}

static int seg_enh_arf_cfg_arf_cvdr_wr_port_dog_0_1(msg_req_arf_t *req_arf,
		cfg_tab_cvdr_t *cvdr_cfg_tab, unsigned int stripe_index)
{
	cvdr_opt_fmt_t cfmt;
	pix_format_e format = PIXEL_FMT_IPP_MAX;
	unsigned int line_stride = 0;
	if (memset_s(&cfmt, sizeof(cvdr_opt_fmt_t), 0, sizeof(cvdr_opt_fmt_t))) {
		loge("Failed : memset_s");
		return ISP_IPP_ERR;
	}

	if (req_arf->streams[stripe_index][BO_ARFEATURE_DOG_0].buffer) {
		cfmt.id = IPP_VP_WR_ARF_DOG_0;
		cfmt.width = req_arf->streams[stripe_index][BO_ARFEATURE_DOG_0].width;
		cfmt.height = req_arf->streams[stripe_index][BO_ARFEATURE_DOG_0].height;
		cfmt.full_width = cfmt.width / 2;
		cfmt.line_size = cfmt.width / 2;
		cfmt.addr = req_arf->streams[stripe_index][BO_ARFEATURE_DOG_0].buffer;
		cfmt.expand = 0;
		format = PIXEL_FMT_IPP_D32;
		line_stride = 0;
		if (dataflow_cvdr_wr_cfg_vp(cvdr_cfg_tab, &cfmt, line_stride, ISP_IPP_CLK, format)) {
			loge("Failed: dataflow_cvdr_wr_cfg_vp, IPP_VP_WR_ARF_DOG_0");
			return ISP_IPP_ERR;
		}
	}

	if (req_arf->streams[stripe_index][BO_ARFEATURE_DOG_1].buffer) {
		cfmt.id = IPP_VP_WR_ARF_DOG_1;
		cfmt.width = req_arf->streams[stripe_index][BO_ARFEATURE_DOG_1].width;
		cfmt.height = req_arf->streams[stripe_index][BO_ARFEATURE_DOG_1].height;
		cfmt.full_width = cfmt.width / 2;
		cfmt.line_size = cfmt.width / 2;
		cfmt.expand = 0;
		format = PIXEL_FMT_IPP_D32;
		line_stride = 0;
		cfmt.addr = req_arf->streams[stripe_index][BO_ARFEATURE_DOG_1].buffer;
		if (dataflow_cvdr_wr_cfg_vp(cvdr_cfg_tab, &cfmt, line_stride, ISP_IPP_CLK, format)) {
			loge("Failed: dataflow_cvdr_wr_cfg_vp, IPP_VP_WR_ARF_DOG_1");
			return ISP_IPP_ERR;
		}
	}

	return ISP_IPP_OK;
}

static int seg_enh_arf_cfg_arf_cvdr_wr_port_dog_2_3(msg_req_arf_t *req_arf,
		cfg_tab_cvdr_t *cvdr_cfg_tab, unsigned int stripe_index)
{
	cvdr_opt_fmt_t cfmt;
	pix_format_e format = PIXEL_FMT_IPP_MAX;
	unsigned int line_stride = 0;
	if (memset_s(&cfmt, sizeof(cvdr_opt_fmt_t), 0, sizeof(cvdr_opt_fmt_t))) {
		loge("Failed : memset_s");
		return ISP_IPP_ERR;
	}

	if (req_arf->streams[stripe_index][BO_ARFEATURE_DOG_2].buffer) {
		cfmt.id = IPP_VP_WR_ARF_DOG_2;
		cfmt.width = req_arf->streams[stripe_index][BO_ARFEATURE_DOG_2].width;
		cfmt.height = req_arf->streams[stripe_index][BO_ARFEATURE_DOG_2].height;
		cfmt.full_width = cfmt.width / 2;
		cfmt.line_size = cfmt.width / 2;
		cfmt.addr = req_arf->streams[stripe_index][BO_ARFEATURE_DOG_2].buffer;
		cfmt.expand = 0;
		format = PIXEL_FMT_IPP_D32;
		line_stride = 0;
		if (dataflow_cvdr_wr_cfg_vp(cvdr_cfg_tab, &cfmt, line_stride, ISP_IPP_CLK, format)) {
			loge("Failed: dataflow_cvdr_wr_cfg_vp, IPP_VP_WR_ARF_DOG_2");
			return ISP_IPP_ERR;
		}
	}

	if (req_arf->streams[stripe_index][BO_ARFEATURE_DOG_3].buffer) {
		cfmt.id = IPP_VP_WR_ARF_DOG_3;
		cfmt.width = req_arf->streams[stripe_index][BO_ARFEATURE_DOG_3].width;
		cfmt.height = req_arf->streams[stripe_index][BO_ARFEATURE_DOG_3].height;
		cfmt.full_width = cfmt.width / 2;
		cfmt.line_size = cfmt.width / 2;
		cfmt.addr = req_arf->streams[stripe_index][BO_ARFEATURE_DOG_3].buffer;
		cfmt.expand = 0;
		format = PIXEL_FMT_IPP_D32;
		line_stride = 0;
		if (dataflow_cvdr_wr_cfg_vp(cvdr_cfg_tab, &cfmt, line_stride, ISP_IPP_CLK, format)) {
			loge("Failed: dataflow_cvdr_wr_cfg_vp, IPP_VP_WR_ARF_DOG_3");
			return ISP_IPP_ERR;
		}
	}

	if (req_arf->streams[stripe_index][BO_ARFEATURE_PYR_2_DS].buffer) {
		cfmt.id = IPP_VP_WR_ARF_PYR_2_DS;
		cfmt.width = req_arf->streams[stripe_index][BO_ARFEATURE_PYR_2_DS].width;
		cfmt.height = req_arf->streams[stripe_index][BO_ARFEATURE_PYR_2_DS].height;
		cfmt.full_width = cfmt.width / 2;
		cfmt.line_size = cfmt.width / 2;
		cfmt.addr = req_arf->streams[stripe_index][BO_ARFEATURE_PYR_2_DS].buffer;
		line_stride = 0;
		cfmt.expand = 0;
		format = PIXEL_FMT_IPP_D32;
		if (dataflow_cvdr_wr_cfg_vp(cvdr_cfg_tab, &cfmt, line_stride, ISP_IPP_CLK, format)) {
			loge("Failed: dataflow_cvdr_wr_cfg_vp, IPP_VP_WR_ARF_PYR_2_DS");
			return ISP_IPP_ERR;
		}
	}

	return ISP_IPP_OK;
}

static int seg_enh_arf_set_arf_cvdr_cfg(
	msg_req_enh_arf_t *req, seg_enh_arf_cfg_t *seg_enh_arf_cfg,
	unsigned int stripe_index, frame_flag_e frame_flag,
	global_info_ipp_t *global_info)
{
	cfg_tab_cvdr_t *cvdr_cfg_tab = NULL;
	msg_req_arf_t *req_arf = NULL;
	int ret = ISP_IPP_OK;

	logd("stripe_index = %d", stripe_index);
	req_arf = (frame_flag == FRAME_CUR) ?
					&(req->req_arf_cur) :
					&(req->req_arf_ref);
	cvdr_cfg_tab = (frame_flag == FRAME_CUR) ?
					&(seg_enh_arf_cfg->arf_cvdr_cur[stripe_index]) :
					&(seg_enh_arf_cfg->arf_cvdr_ref[stripe_index]);

	ret = seg_enh_arf_cfg_arf_cvdr_rd_port_0_1(req_arf, cvdr_cfg_tab, stripe_index);
	if (ret != ISP_IPP_OK) {
		loge(" Failed : seg_enh_arf_cfg_arf_cvdr_rd_port_0_1");
		return ISP_IPP_ERR;
	}

	ret = seg_enh_arf_cfg_arf_cvdr_rd_port_2_3(req_arf, cvdr_cfg_tab, stripe_index);
	if (ret != ISP_IPP_OK) {
		loge(" Failed : seg_enh_arf_cfg_arf_cvdr_rd_port_2_3");
		return ISP_IPP_ERR;
	}

	ret = seg_enh_arf_cfg_arf_cvdr_wr_port_pyr_1_2(req_arf, cvdr_cfg_tab, stripe_index);
	if (ret != ISP_IPP_OK) {
		loge(" Failed : seg_enh_arf_cfg_arf_cvdr_wr_port_pyr_1_2");
		return ISP_IPP_ERR;
	}

	ret = seg_enh_arf_cfg_arf_cvdr_wr_port_dog_0_1(req_arf, cvdr_cfg_tab, stripe_index);
	if (ret != ISP_IPP_OK) {
		loge(" Failed : seg_enh_arf_cfg_arf_cvdr_wr_port_dog_0_1");
		return ISP_IPP_ERR;
	}

	ret = seg_enh_arf_cfg_arf_cvdr_wr_port_dog_2_3(req_arf, cvdr_cfg_tab, stripe_index);
	if (ret != ISP_IPP_OK) {
		loge(" Failed : seg_enh_arf_cfg_arf_cvdr_wr_port_dog_2_3");
		return ISP_IPP_ERR;
	}

	return ret;
}

static int seg_enh_arf_set_enh_cvdr_cfg(
	msg_req_enh_arf_t *req, seg_enh_arf_cfg_t *seg_enh_arf_cfg,
	unsigned int stripe_index, frame_flag_e frame_flag,
	global_info_ipp_t *global_info)
{
	cfg_tab_cvdr_t *cvdr_cfg_tab;
	msg_req_enh_t *req_enh;
	cvdr_opt_fmt_t cfmt;
	pix_format_e format = PIXEL_FMT_IPP_MAX;
	int ret = 0;
	unsigned int line_stride = 0;
	if (memset_s(&cfmt, sizeof(cvdr_opt_fmt_t), 0, sizeof(cvdr_opt_fmt_t))) {
		loge("Failed : memset_s");
		return ISP_IPP_ERR;
	}

	cvdr_cfg_tab = (frame_flag == FRAME_CUR) ?
					&(seg_enh_arf_cfg->arf_cvdr_cur[stripe_index]) :
					&(seg_enh_arf_cfg->arf_cvdr_ref[stripe_index]);
	req_enh = (frame_flag == FRAME_CUR) ?
					&(req->req_enh_cur):
					&(req->req_enh_ref);

	if (req_enh->streams[BI_ENH_YHIST].buffer) {
		cfmt.id = IPP_VP_RD_ORB_ENH_Y_HIST;
		cfmt.width = req_enh->streams[BI_ENH_YHIST].width;
		cfmt.height = req_enh->streams[BI_ENH_YHIST].height;
		cfmt.full_width = req_enh->streams[BI_ENH_YHIST].width / 2;
		cfmt.addr = req_enh->streams[BI_ENH_YHIST].buffer;
		cfmt.line_size = cfmt.width / 2; // 2PF8
		cfmt.expand = 0;
		format = req_enh->streams[BI_ENH_YHIST].format;
		line_stride = 0;
		ret = dataflow_cvdr_rd_cfg_vp(cvdr_cfg_tab, &cfmt, line_stride, ISP_IPP_CLK, format);
		if (ret) {
			loge("do dataflow_cvdr_rd_cfg_vp failed");
			return ISP_IPP_ERR;
		}
	}

	if (req_enh->streams[BI_ENH_YIMAGE].buffer) {
		cfmt.id = IPP_VP_RD_ORB_ENH_Y_IMAGE;
		cfmt.width = req_enh->streams[BI_ENH_YIMAGE].width;
		cfmt.height = req_enh->streams[BI_ENH_YIMAGE].height;
		cfmt.full_width = req_enh->streams[BI_ENH_YIMAGE].width / 2;
		cfmt.addr = req_enh->streams[BI_ENH_YIMAGE].buffer;
		cfmt.line_size = cfmt.width / 2;
		cfmt.expand = 0;
		format = req_enh->streams[BI_ENH_YIMAGE].format;
		line_stride = 0;
		ret = dataflow_cvdr_rd_cfg_vp(cvdr_cfg_tab, &cfmt, line_stride, ISP_IPP_CLK, format);
		if (ret) {
			loge("do dataflow_cvdr_rd_cfg_vp failed");
			return ISP_IPP_ERR;
		}
	}

	if (req_enh->streams[BO_ENH_OUT].buffer) {
		cfmt.id = IPP_VP_WR_ORB_ENH_Y;
		cfmt.width = req_enh->streams[BO_ENH_OUT].width;
		cfmt.height = req_enh->streams[BO_ENH_OUT].height;
		cfmt.full_width = req_enh->streams[BO_ENH_OUT].width / 2;
		cfmt.addr = req_enh->streams[BO_ENH_OUT].buffer;
		cfmt.expand = 0;
		format = req_enh->streams[BO_ENH_OUT].format;
		line_stride = 0;
		ret = dataflow_cvdr_wr_cfg_vp(cvdr_cfg_tab, &cfmt, line_stride, ISP_IPP_CLK, format);
		if (ret) {
			loge("do dataflow_cvdr_rd_cfg_vp failed");
			return ISP_IPP_ERR;
		}
	}
	return ISP_IPP_OK;
}


static int seg_enh_arf_set_enh_cfg_tab(
	msg_req_enh_arf_t *req, seg_enh_arf_cfg_t *seg_enh_arf_cfg,
	frame_flag_e frame_flag)
{
	msg_req_enh_t *req_enh = NULL;
	cfg_tab_orb_enh_t *p_enh_cfg_tab = NULL;
	req_enh = (frame_flag == FRAME_CUR) ? &(req->req_enh_cur) : &(req->req_enh_ref);
	p_enh_cfg_tab = (frame_flag == FRAME_CUR) ? &(seg_enh_arf_cfg->enh_cur) : &(seg_enh_arf_cfg->enh_ref);

	if (memcpy_s(p_enh_cfg_tab, sizeof(cfg_tab_orb_enh_t), &req_enh->reg_cfg, sizeof(cfg_tab_orb_enh_t))) {
		loge("Failed: memcpy_s");
		return ISP_IPP_ERR;
	}
	return ISP_IPP_OK;
}

static int seg_enh_arf_set_arf_cfg_tab(
	msg_req_enh_arf_t *req, seg_enh_arf_cfg_t *seg_enh_arf_cfg,
	unsigned int stripe_index, frame_flag_e frame_flag)
{
	msg_req_arf_t *req_arf = NULL;
	cfg_tab_arf_t *arf_cfg_tab = NULL;

	req_arf = (frame_flag == FRAME_CUR) ?
					&(req->req_arf_cur) :
					&(req->req_arf_ref);
	arf_cfg_tab = (frame_flag == FRAME_CUR) ?
					&(seg_enh_arf_cfg->arf_cur[stripe_index]) :
					&(seg_enh_arf_cfg->arf_ref[stripe_index]);

	if (memcpy_s(arf_cfg_tab, sizeof(cfg_tab_arf_t), &req_arf->reg_cfg[stripe_index], sizeof(cfg_tab_arf_t))) {
		loge("Failed: memcpy_s");
		return ISP_IPP_ERR;
	}

	// mean: the first detect layer and the first iter; we get the kpt buf from 'stream';
	if (arf_cfg_tab->top_cfg.bits.mode == DETECT_AND_DESC_AND_MFPDA_DISABLE ||
		arf_cfg_tab->top_cfg.bits.mode == DETECT_AND_DESC_AND_MFPDA_ENABLE) {
			arf_cfg_tab->cvdr_cfg.wr_cfg.pre_wr_axi_fs.bits.pre_vpwr_address_frame_start =
				req_arf->arf_stat_buff[ARF_DESC_BUFF].buffer >> 4; // the first addr of kpt buf
			arf_cfg_tab->cvdr_cfg.rd_cfg.pre_rd_fhg.bits.pre_vprd_frame_size = 0;
	}
	return ISP_IPP_OK;
}

static int seg_enh_arf_set_cfg_tbl(
	msg_req_enh_arf_t *req, seg_enh_arf_cfg_t *seg_enh_arf_cfg, global_info_ipp_t *global_info)
{
	unsigned int i = 0;
	frame_flag_e frame_flag = FRAME_CUR;
	int ret = 0;

	if (global_info->enh_en) {
		logd("config ENH cur frame's module and cvdr");
		ret = seg_enh_arf_set_enh_cvdr_cfg(req, seg_enh_arf_cfg, 0, frame_flag, global_info);
		if (ret != ISP_IPP_OK) {
			loge(" Failed : seg_enh_arf_set_enh_cvdr_cfg");
			return ISP_IPP_ERR;
		}
		ret = seg_enh_arf_set_enh_cfg_tab(req, seg_enh_arf_cfg, frame_flag);
		if (ret != ISP_IPP_OK) {
			loge(" Failed : seg_enh_arf_set_enh_cfg_tab");
			return ISP_IPP_ERR;
		}
	}

	if (global_info->arf_en) {
		for (i = 0; i < req->mode_cnt; i++) {
			logd("config ARF cur frame's module and cvdr");
			ret = seg_enh_arf_set_arf_cvdr_cfg(req, seg_enh_arf_cfg, i, frame_flag, global_info);
			if (ret != ISP_IPP_OK) {
				loge(" Failed : seg_enh_arf_set_arf_cvdr_cfg");
				return ISP_IPP_ERR;
			}
			ret = seg_enh_arf_set_arf_cfg_tab(req, seg_enh_arf_cfg, i, frame_flag);
			if (ret != ISP_IPP_OK) {
				loge(" Failed : seg_enh_arf_set_arf_cfg_tab");
				return ISP_IPP_ERR;
			}
		}

		if (req->work_frame == CUR_REF) {
			frame_flag = FRAME_REF;

			if (global_info->enh_en) {
				logd("config ENH ref frame's module and cvdr");
				ret = seg_enh_arf_set_enh_cvdr_cfg(req, seg_enh_arf_cfg, 0, frame_flag, global_info);
				if (ret != ISP_IPP_OK) {
					loge(" Failed : seg_enh_arf_set_enh_cvdr_cfg");
					return ISP_IPP_ERR;
				}
				ret = seg_enh_arf_set_enh_cfg_tab(req, seg_enh_arf_cfg, frame_flag);
				if (ret != ISP_IPP_OK) {
					loge(" Failed : seg_enh_arf_set_enh_cfg_tab");
					return ISP_IPP_ERR;
				}
			}

			for (i = 0; i < req->mode_cnt; i++) {
				logd("config ARF ref frame's module and cvdr");
				ret = seg_enh_arf_set_arf_cvdr_cfg(req, seg_enh_arf_cfg, i, frame_flag, global_info);
				if (ret != ISP_IPP_OK) {
					loge(" Failed : seg_enh_arf_set_arf_cvdr_cfg");
					return ISP_IPP_ERR;
				}
				ret = seg_enh_arf_set_arf_cfg_tab(req, seg_enh_arf_cfg, i, frame_flag);
				if (ret != ISP_IPP_OK) {
					loge(" Failed : seg_enh_arf_set_arf_cfg_tab");
					return ISP_IPP_ERR;
				}
			}
		}
	}

	return ISP_IPP_OK;
}

static int seg_enh_arf_cal_stripe_num_info(msg_req_enh_arf_t *req,
		seg_enh_arf_stripe_t *stripe_info, unsigned int i)
{
	unsigned int arf_mode = req->req_arf_cur.reg_cfg[i].top_cfg.bits.mode;

	// Cyclically calculate how many cmdlst_stripe are needed in the current msg;
	if (arf_mode < DETECT_AND_DESC_AND_MFPDA_DISABLE) {
		// a: gauss mode
		stripe_info->total_stripe_num += CFG_ARF;
		stripe_info->gauss_total_stripe += CFG_ARF;
	} else {
		// b: detect mode
		// Cfg current layer's arfeature and updata next layer's vp_WR port's cfg para.
		if (arf_mode == DETECT_AND_DESC_AND_MFPDA_DISABLE || arf_mode == DETECT_AND_DESC_AND_MFPDA_ENABLE) {
			stripe_info->total_stripe_num = (i != (req->mode_cnt - 1)) ?
										(stripe_info->total_stripe_num + CFG_ARF + UPDATA_NEXT_LAYER_VPWR_CFG) :
										(stripe_info->total_stripe_num + CFG_ARF);
			stripe_info->detect_total_stripe = (i != (req->mode_cnt - 1)) ?
										(stripe_info->detect_total_stripe + CFG_ARF + UPDATA_NEXT_LAYER_VPWR_CFG) :
										(stripe_info->detect_total_stripe + CFG_ARF);
			stripe_info->layer_stripe_num = CFG_ARF + UPDATA_NEXT_LAYER_VPWR_CFG; // except last layer;
		} else if (arf_mode == DETECT_AND_KLT_AND_MFPDA_DISABLE || arf_mode == DETECT_AND_KLT_AND_MFPDA_ENABLE) {
			stripe_info->total_stripe_num    = (stripe_info->total_stripe_num + CFG_ARF);
			stripe_info->detect_total_stripe = (stripe_info->detect_total_stripe + CFG_ARF);
			stripe_info->layer_stripe_num    = CFG_ARF;
		} else {
			loge("Failed: error arf_mode.%d", arf_mode);
			return ISP_IPP_ERR;
		}

		// Note: 6 Layer's MINSCR_KPTCNT stat_data are all stored in this buffer;
		if (req->req_arf_cur.arf_stat_buff[ARF_MINSCR_KPTCNT_BUFF].buffer) {
			stripe_info->total_stripe_num    += GET_STAT_MINSCR_KPTCNT;
			stripe_info->detect_total_stripe += GET_STAT_MINSCR_KPTCNT;
			stripe_info->layer_stripe_num    += GET_STAT_MINSCR_KPTCNT;
		}

		// Note: 6 Layer's SUM_SCORE stat_data are all stored in this buffer;
		if (req->req_arf_cur.arf_stat_buff[ARF_SUM_SCORE_BUFF].buffer) {
			stripe_info->total_stripe_num    += GET_STAT_SUM_SCORE;
			stripe_info->detect_total_stripe += GET_STAT_SUM_SCORE;
			stripe_info->layer_stripe_num    += GET_STAT_SUM_SCORE;
		}
	}

	return ISP_IPP_OK;
}

static int seg_enh_arf_cfg_dmap_gauss_first_irq(unsigned int *irq)
{
	*irq = *irq | (((unsigned int)(1)) << IPP_ARF_CVDR_VP_RD_EOF_CMDLST);
	*irq = *irq | (((unsigned int)(1)) << IPP_ARF_CVDR_VP_RD_EOF_IN_0);
	*irq = *irq | (((unsigned int)(1)) << IPP_ARF_CVDR_VP_WR_EOF_PRY_2);
	*irq = *irq | (((unsigned int)(1)) << IPP_ARF_CVDR_VP_WR_EOF_DOG_1);
	*irq = *irq | (((unsigned int)(1)) << IPP_ARF_CVDR_VP_WR_EOF_PRY_2_DS);
	*irq = *irq | (((unsigned int)(1)) << IPP_ARF_DONE_IRQ);
	return ISP_IPP_OK;
}

static int seg_enh_arf_cfg_ar_gauss_first_irq(unsigned int *irq)
{
	*irq = *irq | (((unsigned int)(1)) << IPP_ARF_CVDR_VP_RD_EOF_CMDLST);
	*irq = *irq | (((unsigned int)(1)) << IPP_ARF_CVDR_VP_RD_EOF_IN_0);
	*irq = *irq | (((unsigned int)(1)) << IPP_ARF_CVDR_VP_WR_EOF_PRY_1);
	*irq = *irq | (((unsigned int)(1)) << IPP_ARF_CVDR_VP_WR_EOF_PRY_2);
	*irq = *irq | (((unsigned int)(1)) << IPP_ARF_CVDR_VP_WR_EOF_DOG_0);
	*irq = *irq | (((unsigned int)(1)) << IPP_ARF_CVDR_VP_WR_EOF_DOG_1);
	*irq = *irq | (((unsigned int)(1)) << IPP_ARF_CVDR_VP_WR_EOF_PRY_2_DS);
	*irq = *irq | (((unsigned int)(1)) << IPP_ARF_DONE_IRQ);
	return ISP_IPP_OK;
}

static int seg_enh_arf_cfg_gauss_middle_irq(unsigned int *irq)
{
	*irq = *irq | (((unsigned int)(1)) << IPP_ARF_CVDR_VP_RD_EOF_CMDLST);
	*irq = *irq | (((unsigned int)(1)) << IPP_ARF_CVDR_VP_RD_EOF_IN_0);
	*irq = *irq | (((unsigned int)(1)) << IPP_ARF_CVDR_VP_RD_EOF_IN_3); // new add r02
	*irq = *irq | (((unsigned int)(1)) << IPP_ARF_CVDR_VP_WR_EOF_PRY_1);
	*irq = *irq | (((unsigned int)(1)) << IPP_ARF_CVDR_VP_WR_EOF_PRY_2);
	*irq = *irq | (((unsigned int)(1)) << IPP_ARF_CVDR_VP_WR_EOF_DOG_0);
	*irq = *irq | (((unsigned int)(1)) << IPP_ARF_CVDR_VP_WR_EOF_DOG_1);
	*irq = *irq | (((unsigned int)(1)) << IPP_ARF_CVDR_VP_WR_EOF_DOG_2);
	*irq = *irq | (((unsigned int)(1)) << IPP_ARF_CVDR_VP_WR_EOF_DOG_3);
	*irq = *irq | (((unsigned int)(1)) << IPP_ARF_CVDR_VP_WR_EOF_PRY_2_DS);
	*irq = *irq | (((unsigned int)(1)) << IPP_ARF_DONE_IRQ);
	return ISP_IPP_OK;
}

static int seg_enh_arf_cfg_gauss_last_irq(unsigned int *irq)
{
	*irq = *irq | (((unsigned int)(1)) << IPP_ARF_CVDR_VP_RD_EOF_CMDLST);
	*irq = *irq | (((unsigned int)(1)) << IPP_ARF_CVDR_VP_RD_EOF_IN_0);
	*irq = *irq | (((unsigned int)(1)) << IPP_ARF_CVDR_VP_RD_EOF_IN_3); // new add r02
	*irq = *irq | (((unsigned int)(1)) << IPP_ARF_CVDR_VP_WR_EOF_DOG_2);
	*irq = *irq | (((unsigned int)(1)) << IPP_ARF_CVDR_VP_WR_EOF_DOG_3);
	*irq = *irq | (((unsigned int)(1)) << IPP_ARF_DONE_IRQ);
	return ISP_IPP_OK;
}

static int seg_enh_arf_cfg_detect_and_desc_irq(unsigned int *irq, unsigned int pre_stripe_num,
		unsigned int j)
{
	if (j == pre_stripe_num) { // The configuration of the first stripe of the current layer.
		*irq = *irq | (((unsigned int)(1)) << IPP_ARF_CVDR_VP_RD_EOF_CMDLST);
		*irq = *irq | (((unsigned int)(1)) << IPP_ARF_CVDR_VP_RD_EOF_IN_0);
		*irq = *irq | (((unsigned int)(1)) << IPP_ARF_CVDR_VP_RD_EOF_IN_1);
		*irq = *irq | (((unsigned int)(1)) << IPP_ARF_CVDR_VP_RD_EOF_IN_2);
		*irq = *irq | (((unsigned int)(1)) << IPP_ARF_CVDR_VP_RD_EOF_IN_3);
		*irq = *irq | (((unsigned int)(1)) << IPP_ARF_CVDR_VP_WR_EOF_PRY_1);
		*irq = *irq | (((unsigned int)(1)) << IPP_ARF_DONE_IRQ);
	} else {
		// These layers are all read and write operations of cmdlst,
		// so it is enough to monitor the read and write interruption of cmdlst.
		*irq = *irq | (((unsigned int)(1)) << IPP_ARF_CVDR_VP_RD_EOF_CMDLST);
		*irq = *irq | (((unsigned int)(1)) << IPP_ARF_CVDR_VP_WR_EOF_CMDLST);
	}

	return ISP_IPP_OK;
}

static int seg_enh_arf_cfg_detect_and_desc_irq_kltpre(unsigned int *irq, unsigned int pre_stripe_num,
		unsigned int j, unsigned int mode)
{
	logd("[liang]do klt_pre, arf_mode.%d", mode);

	if (j == pre_stripe_num) { // The configuration of the first stripe of the current layer. cfg module;
		if (mode == DETECT_AND_KLT_AND_MFPDA_DISABLE) {
			*irq = *irq | (((unsigned int)(1)) << IPP_ARF_CVDR_VP_RD_EOF_CMDLST);
			*irq = *irq | (((unsigned int)(1)) << IPP_ARF_CVDR_VP_RD_EOF_IN_1);
			*irq = *irq | (((unsigned int)(1)) << IPP_ARF_CVDR_VP_RD_EOF_IN_2);
			*irq = *irq | (((unsigned int)(1)) << IPP_ARF_CVDR_VP_RD_EOF_IN_3);
			*irq = *irq | (((unsigned int)(1)) << IPP_ARF_DONE_IRQ);
		} else if (mode == DETECT_AND_KLT_AND_MFPDA_ENABLE) {
			*irq = *irq | (((unsigned int)(1)) << IPP_ARF_CVDR_VP_RD_EOF_CMDLST);
			*irq = *irq | (((unsigned int)(1)) << IPP_ARF_CVDR_VP_RD_EOF_IN_1);
			*irq = *irq | (((unsigned int)(1)) << IPP_ARF_CVDR_VP_RD_EOF_IN_2);
			*irq = *irq | (((unsigned int)(1)) << IPP_ARF_CVDR_VP_RD_EOF_IN_3);
			*irq = *irq | (((unsigned int)(1)) << IPP_ARF_CVDR_VP_WR_EOF_PRY_1);
			*irq = *irq | (((unsigned int)(1)) << IPP_ARF_DONE_IRQ);
		}
	} else {
		// These layers are used by the cmdlst module to process read and write operations.
		// So, it is enough to monitor the read and write interruption of cmdlst's VP_port.
		*irq = *irq | (((unsigned int)(1)) << IPP_ARF_CVDR_VP_RD_EOF_CMDLST);
		*irq = *irq | (((unsigned int)(1)) << IPP_ARF_CVDR_VP_WR_EOF_CMDLST);
	}

	return ISP_IPP_OK;
}

static int seg_enh_arf_cfg_kptnum_totalkpt_cmdlst_stripe(msg_req_enh_arf_t *req,
		cmdlst_stripe_info_t *cmdlst_stripe, seg_enh_arf_stripe_t *stripe_info,
		unsigned int pre_stripe_num)
{
	unsigned int j = 0;
	unsigned int irq = 0;
	logd("+");

	if (req->req_arf_cur.arf_stat_buff[ARF_KPT_NUM_BUFF].buffer)
		stripe_info->total_stripe_num += 1;

	if (req->req_arf_cur.arf_stat_buff[ARF_TOTAL_KPT_BUFF].buffer)
		stripe_info->total_stripe_num += 1;

	if (req->req_arf_cur.arf_stat_buff[ARF_KLTPRE_TOTAL_KPT_NUM].buffer)
		stripe_info->total_stripe_num += 1;

	if (req->req_arf_cur.arf_stat_buff[ARF_NXT_DOG_THRESHOLD].buffer)
		stripe_info->total_stripe_num += 1;

	if (req->req_arf_cur.arf_stat_buff[ARF_KPT_LIMIT_BUFF].buffer)
		stripe_info->total_stripe_num += 1;

	if (stripe_info->total_stripe_num > CMDLST_STRIPE_MAX_NUM) {
		loge("error : invalid total_stripe_num = %d", stripe_info->total_stripe_num);
		return ISP_IPP_ERR;
	}

	for (j = pre_stripe_num; j < (stripe_info->total_stripe_num); j++) {
		irq = 0;
		cmdlst_stripe[j].resource_share  = 1 << IPP_CMD_RES_SHARE_ARFEATURE;
		cmdlst_stripe[j].irq_module      = 1 << IPP_CMD_IRQ_MODULE_ARFEATURE;
		cmdlst_stripe[j].is_need_set_sop = 0;
		cmdlst_stripe[j].en_link         = 0;
		cmdlst_stripe[j].ch_link         = 0;
		cmdlst_stripe[j].ch_link_act_nbr = 0;
		cmdlst_stripe[j].is_last_stripe  = 0;
		cmdlst_stripe[j].hw_priority     = CMD_PRIO_HIGH;
		irq = irq | (((unsigned int)(1)) << IPP_ARF_CVDR_VP_RD_EOF_CMDLST);
		irq = irq | (((unsigned int)(1)) << IPP_ARF_CVDR_VP_WR_EOF_CMDLST);
		cmdlst_stripe[j].irq_mode  = irq;
	}

	logd("-");
	return ISP_IPP_OK;
}

static void seg_enh_arf_init_cmdlst_stripe(cmdlst_stripe_info_t *cmdlst_stripe, unsigned int j)
{
	cmdlst_stripe[j].resource_share  = 1 << IPP_CMD_RES_SHARE_ARFEATURE;
	cmdlst_stripe[j].irq_module 	 = 1 << IPP_CMD_IRQ_MODULE_ARFEATURE;
	cmdlst_stripe[j].is_need_set_sop = 0;
	cmdlst_stripe[j].en_link         = 0;
	cmdlst_stripe[j].ch_link         = 0;
	cmdlst_stripe[j].ch_link_act_nbr = 0;
	cmdlst_stripe[j].is_last_stripe  = 0;
	cmdlst_stripe[j].hw_priority     = CMD_PRIO_HIGH;
	return;
}

static void seg_enh_arf_cfg_cmdlst_stripe(
	msg_req_enh_arf_t *req, seg_enh_arf_stripe_t *stripe_info,
	cmdlst_stripe_info_t *cmdlst_stripe, global_info_ipp_t *global_info,
	seg_enh_arf_tmp_info *tmp_info)
{
	unsigned int i = 0, j = 0, mode = 0;
	unsigned int irq = 0;
	unsigned int pre_stripe_num = 0;

	i = tmp_info->i;
	pre_stripe_num = tmp_info->pre_stripe_num;
	logd("cur_layer.%d, pre_stripe_num.%d, total_stripe_num.%d", i, pre_stripe_num, stripe_info->total_stripe_num);

	// (total_stripe_num - pre_stripe_num) means the current layer's cmdlst_stripe num.
	for (j = pre_stripe_num; j < stripe_info->total_stripe_num; j++) {
		irq = 0;
		seg_enh_arf_init_cmdlst_stripe(cmdlst_stripe, j);

		mode = req->req_arf_cur.reg_cfg[i].top_cfg.bits.mode;
		switch (mode) {
		case DMAP_GAUSS_FIRST:
			seg_enh_arf_cfg_dmap_gauss_first_irq(&irq);

			if (global_info->enh_en == 1 && pre_stripe_num == 0) {
				// enh+arf mode 0 use one stripe;
				irq = irq & (~(((unsigned int)(1)) << IPP_ARF_CVDR_VP_RD_EOF_IN_0)); // This signal is not monitored.
				irq = irq | (((unsigned int)(1)) << IPP_ENH_DONE_IRQ);
			}
			break;

		case AR_GAUSS_FIRST:
			seg_enh_arf_cfg_ar_gauss_first_irq(&irq);

			if (global_info->enh_en == 1 && pre_stripe_num == 0) {
				// enh+arf mode 1 use one stripe; input from enh directly
				irq = irq & (~(((unsigned int)(1)) << IPP_ARF_CVDR_VP_RD_EOF_IN_0));
				irq = irq | (((unsigned int)(1)) << IPP_ENH_DONE_IRQ);
			}
			break;

		case GAUSS_MIDDLE:
			seg_enh_arf_cfg_gauss_middle_irq(&irq);
			break;

		case GAUSS_LAST:
			seg_enh_arf_cfg_gauss_last_irq(&irq);
			break;

		case DETECT_AND_DESC_AND_MFPDA_DISABLE:
		case DETECT_AND_DESC_AND_MFPDA_ENABLE:
			seg_enh_arf_cfg_detect_and_desc_irq(&irq, pre_stripe_num, j);
			break;

		case DETECT_AND_KLT_AND_MFPDA_DISABLE:
		case DETECT_AND_KLT_AND_MFPDA_ENABLE:
			seg_enh_arf_cfg_detect_and_desc_irq_kltpre(&irq, pre_stripe_num, j, mode);
			break;

		default:
			loge("Failed : arf mode is error, mode = %d", mode);
			break;
		}

		cmdlst_stripe[j].irq_mode  = irq;
		logd("stripe.%d, irq = 0x%x, arf mode = %d", j, irq, mode);
	}

	return;
}

static unsigned int seg_enh_arf_cal_stripe_num_and_cfg_cmdlst_stripe(
	msg_req_enh_arf_t *req, seg_enh_arf_stripe_t *stripe_info,
	cmdlst_stripe_info_t *cmdlst_stripe, global_info_ipp_t *global_info)
{
	unsigned int i = 0;
	unsigned int pre_stripe_num = 0;
	seg_enh_arf_tmp_info tmp_info = { 0 };

	for (i = 0; i < req->mode_cnt; i++) {
		// 1.Calculate the total_stripe_num.
		seg_enh_arf_cal_stripe_num_info(req, stripe_info, i);
		logd("cur_frame: cur_layer = %d, detect_total_stripe = %d, layer_stripe_num = %d,"
			"gauss_total_stripe = %d, cur_frame_stripe_cnt = %d, total_stripe_num = %d",
			i, stripe_info->detect_total_stripe, stripe_info->layer_stripe_num,
			stripe_info->gauss_total_stripe, stripe_info->cur_frame_stripe_cnt, stripe_info->total_stripe_num);

		// 2.CFG the cmdlst_stripe;
		tmp_info.i = i;
		tmp_info.pre_stripe_num = pre_stripe_num;
		seg_enh_arf_cfg_cmdlst_stripe(req, stripe_info, cmdlst_stripe, global_info, &tmp_info);

		// 3.Updata the pre_stripe_num;
		pre_stripe_num = stripe_info->total_stripe_num;
	}

	return pre_stripe_num;
}

/************************************************************************************
name:seg_enh_arf_cal_stripe_info
Max stripe_map:36 stripes
	gauss1,gauss2,gauss3,gauss4,
	detect00+sumscore+minscore+updata
	detect01+sumscore+minscore+updata
	detect10+sumscore+minscore+updata
	detect11+sumscore+minscore+updata
	detect20+sumscore+minscore+updata
	detect21+sumscore+minscore
	ARF_KPT_NUM_BUFF (for stat)
	ARF_TOTAL_KPT_BUFF
	ARF_KLTPRE_TOTAL_KPT_NUM
	ARF_NXT_DOG_THRESHOLD
	ARF_KPT_LIMIT_BUFF
	UPDATE_RDR_TOTAL_KPT_NUM_CFG (for matchers)
	UPDATE_RDR_KPT_NUMBER_CFG
	UPDATE_RDR_CVDR_RD_FRAME_HEIGHT_CFG
	UPDATE_CMP_CUR_KPT_NUMBER_CFG
************************************************************************************/
static int seg_enh_arf_set_cmdlst_stripe(
	msg_req_enh_arf_t *req, cmdlst_para_t *cmdlst_para,
	seg_enh_arf_stripe_t *stripe_info, global_info_ipp_t *global_info)
{
	int ret = 0;
	unsigned int i = 0, updata_stripe_cnt = 0;
	unsigned int pre_stripe_num = 0;
	cmdlst_stripe_info_t *cmdlst_stripe = NULL;
	unsigned int stat_irq = 0;

	cmdlst_stripe = cmdlst_para->cmd_stripe_info;
	stat_irq = (((unsigned int)(1)) << IPP_ARF_CVDR_VP_RD_EOF_CMDLST) |
				(((unsigned int)(1)) << IPP_ARF_CVDR_VP_WR_EOF_CMDLST);

	pre_stripe_num = seg_enh_arf_cal_stripe_num_and_cfg_cmdlst_stripe(req,
					 stripe_info, cmdlst_stripe, global_info);
	logd("#1 stripe_info: cur_frame_stripe_cnt=%d, detect_total_stripe=%d, gauss_total_stripe=%d, "
		"layer_stripe_num=%d, total_stripe_num=%d",
		stripe_info->cur_frame_stripe_cnt, stripe_info->detect_total_stripe, stripe_info->gauss_total_stripe,
		stripe_info->layer_stripe_num, stripe_info->total_stripe_num);

	// 3.Handle the case where KPT_NUM_BUFF and KPT_NUM_BUFF are not empty;
	ret = seg_enh_arf_cfg_kptnum_totalkpt_cmdlst_stripe(req, cmdlst_stripe, stripe_info, pre_stripe_num);
	if (ret) {
		loge("error : do seg_enh_arf_cfg_kptnum_totalkpt_cmdlst_stripe failed");
		return ISP_IPP_ERR;
	}

	logd("#2 stripe_info: cur_frame_stripe_cnt=%d, detect_total_stripe=%d, gauss_total_stripe=%d, "
		"layer_stripe_num=%d, total_stripe_num=%d",
		stripe_info->cur_frame_stripe_cnt, stripe_info->detect_total_stripe, stripe_info->gauss_total_stripe,
		stripe_info->layer_stripe_num, stripe_info->total_stripe_num);

	if (global_info->matcher_update_flag == 1) {
		// 4:update rdr kpt, cmp kpt, rdr total kpt, rdr frame height;
		updata_stripe_cnt = UPDATE_RDR_TOTAL_KPT_NUM_CFG + UPDATE_RDR_KPT_NUMBER_CFG +
							UPDATE_RDR_CVDR_RD_FRAME_HEIGHT_CFG + UPDATE_CMP_CUR_KPT_NUMBER_CFG;
		for (i = 0; i < updata_stripe_cnt; i++) {
			seg_enh_arf_init_cmdlst_stripe(cmdlst_stripe, stripe_info->total_stripe_num);
			cmdlst_stripe[stripe_info->total_stripe_num].irq_mode = stat_irq;
			stripe_info->total_stripe_num += 1;
		}

		stripe_info->cur_frame_stripe_cnt = stripe_info->total_stripe_num;
		logd("#3 stripe_info: cur_frame_stripe_cnt=%d, detect_total_stripe=%d, gauss_total_stripe=%d, "
			"layer_stripe_num=%d, total_stripe_num=%d",
			stripe_info->cur_frame_stripe_cnt, stripe_info->detect_total_stripe, stripe_info->gauss_total_stripe,
			stripe_info->layer_stripe_num, stripe_info->total_stripe_num);

		if (req->work_frame == CUR_REF) {
			stripe_info->total_stripe_num = stripe_info->total_stripe_num * 2;
			if (stripe_info->total_stripe_num > CMDLST_STRIPE_MAX_NUM) {
				loge("error : invalid total_stripe_num = %d", stripe_info->total_stripe_num);
				return ISP_IPP_ERR;
			}

			for (i = stripe_info->cur_frame_stripe_cnt; i < stripe_info->total_stripe_num; i++) {
				seg_enh_arf_init_cmdlst_stripe(cmdlst_stripe, i);
				cmdlst_stripe[i].irq_mode = cmdlst_stripe[i - stripe_info->cur_frame_stripe_cnt].irq_mode;
			}

			// ref_frame: need to updata cmp's total_kpt/cmp_frame_height;
			stripe_info->total_stripe_num += UPDATE_CMP_TOTAL_KPT_NUM_CFG;
			seg_enh_arf_init_cmdlst_stripe(cmdlst_stripe, stripe_info->total_stripe_num - 1);
			cmdlst_stripe[stripe_info->total_stripe_num - 1].irq_mode = stat_irq;

			stripe_info->total_stripe_num += UPDATE_CMP_CVDR_VP_RD_FRAME_HEIGHT_CFG;
			seg_enh_arf_init_cmdlst_stripe(cmdlst_stripe, stripe_info->total_stripe_num - 1);
			cmdlst_stripe[stripe_info->total_stripe_num - 1].irq_mode = stat_irq;
		}
	}

	if (stripe_info->total_stripe_num >= 1)
		cmdlst_stripe[stripe_info->total_stripe_num - 1].is_last_stripe = 1;
	cmdlst_stripe[0].hw_priority = CMD_PRIO_LOW;

	if (global_info->matcher_update_flag == 1 && stripe_info->total_stripe_num >= 1) {
		cmdlst_stripe[stripe_info->total_stripe_num - 1].en_link         = 1;
		cmdlst_stripe[stripe_info->total_stripe_num - 1].ch_link         = global_info->rdr_channel;
		cmdlst_stripe[stripe_info->total_stripe_num - 1].ch_link_act_nbr = (req->work_frame == CUR_REF) ?
																		STRIPE_NUM_EACH_RDR * 2 : STRIPE_NUM_EACH_RDR;
	} else if (global_info->klt_en == 1 && stripe_info->total_stripe_num >= 1) {
		cmdlst_stripe[stripe_info->total_stripe_num - 1].en_link    = 1;
		cmdlst_stripe[stripe_info->total_stripe_num - 1].ch_link    = global_info->klt_channel;
		cmdlst_stripe[stripe_info->total_stripe_num - 1].ch_link_act_nbr = global_info->klt_stripe_cnt;
	}

	if (stripe_info->total_stripe_num > CMDLST_STRIPE_MAX_NUM) {
		loge("error : invalid total_stripe_num = %d", stripe_info->total_stripe_num);
		return ISP_IPP_ERR;
	}

	cmdlst_para->stripe_cnt  = stripe_info->total_stripe_num;
	cmdlst_para->channel_id  = global_info->enh_arf_channel;
	cmdlst_para->work_module = global_info->work_module;
	cmdlst_para->cmd_entry_mem_id = MEM_ID_CMDLST_ENTRY_ENH_ARF;
	cmdlst_para->cmd_buf_mem_id = MEM_ID_CMDLST_BUF_ENH_ARF;

	logi("final arf: total_stripe_num = %d, detect_total_stripe = %d, "
		 "gauss_total_stripe = %d, layer_stripe_num = %d, cur_frame_stripe_cnt = %d",
		 stripe_info->total_stripe_num, stripe_info->detect_total_stripe,
		 stripe_info->gauss_total_stripe, stripe_info->layer_stripe_num, stripe_info->cur_frame_stripe_cnt);
	return ISP_IPP_OK;
}

static int seg_enh_arf_update_matcher_cmdlst(
	schedule_cmdlst_link_t *cmd_link_entry, unsigned int cmd_index,
	global_info_ipp_t *global_info, frame_flag_e frame_flag)
{
	cmdlst_wr_cfg_t cmdlst_wr_cfg;
	if (memset_s(&cmdlst_wr_cfg, sizeof(cmdlst_wr_cfg_t), 0, sizeof(cmdlst_wr_cfg_t))) {
		loge("Failed : memset_s");
		return ISP_IPP_ERR;
	}

	cmdlst_wr_cfg.is_incr = 1;
	cmdlst_wr_cfg.read_mode = 0;
	logd("+");
	logd("cmd_index = %d", cmd_index);

	cmdlst_wr_cfg.data_size = SEG_ENH_ARF_GRID_NUM_RANG;
	cmdlst_wr_cfg.is_wstrb = 0; // addr not need strobe
	cmdlst_wr_cfg.buff_wr_addr = (frame_flag == FRAME_CUR) ?
								 global_info->cmp_kpt_num_addr_cur :
								 global_info->cmp_kpt_num_addr_ref;
	cmdlst_wr_cfg.reg_rd_addr = IPP_BASE_ADDR_ARF + AR_FEATURE_KPT_NUMBER_0_REG;
	loge_if(seg_ipp_set_cmdlst_wr_buf(&cmd_link_entry[cmd_index++].cmd_buf, &cmdlst_wr_cfg));

	cmdlst_wr_cfg.buff_wr_addr = (frame_flag == FRAME_CUR) ?
								 global_info->rdr_kpt_num_addr_cur : global_info->rdr_kpt_num_addr_ref;
	loge_if(seg_ipp_set_cmdlst_wr_buf(&cmd_link_entry[cmd_index++].cmd_buf, &cmdlst_wr_cfg));

	cmdlst_wr_cfg.data_size = 1;
	cmdlst_wr_cfg.is_wstrb = 1;
	cmdlst_wr_cfg.buff_wr_addr = (frame_flag == FRAME_CUR) ?
								 global_info->rdr_total_num_addr_cur : global_info->rdr_total_num_addr_ref;
	cmdlst_wr_cfg.reg_rd_addr = IPP_BASE_ADDR_ARF + AR_FEATURE_TOTAL_KPT_NUM_REG;
	loge_if(seg_ipp_set_cmdlst_wr_buf(&cmd_link_entry[cmd_index++].cmd_buf, &cmdlst_wr_cfg));

	if (frame_flag == FRAME_REF) {
		cmdlst_wr_cfg.buff_wr_addr = global_info->cmp_total_num_addr;
		loge_if(seg_ipp_set_cmdlst_wr_buf(&cmd_link_entry[cmd_index++].cmd_buf, &cmdlst_wr_cfg));
	}

	cmdlst_wr_cfg.buff_wr_addr = (frame_flag == FRAME_CUR) ?
								 global_info->rdr_frame_height_addr_cur : global_info->rdr_frame_height_addr_ref;
	cmdlst_wr_cfg.reg_rd_addr = IPP_BASE_ADDR_ARF + AR_FEATURE_CVDR_RD_FHG_REG;
	loge_if(seg_ipp_set_cmdlst_wr_buf(&cmd_link_entry[cmd_index++].cmd_buf, &cmdlst_wr_cfg));

	if (frame_flag == FRAME_REF) {
		cmdlst_wr_cfg.buff_wr_addr = global_info->cmp_frame_height_addr;
		cmdlst_wr_cfg.reg_rd_addr = IPP_BASE_ADDR_ARF + AR_FEATURE_CVDR_RD_FHG_REG;
		loge_if(seg_ipp_set_cmdlst_wr_buf(&cmd_link_entry[cmd_index++].cmd_buf, &cmdlst_wr_cfg));
	}

	logd("-");
	return ISP_IPP_OK;
}

static unsigned int seg_enh_arf_cfg_kptnum_totalkpt_cmd_buf(
	msg_req_arf_t *req_arf, schedule_cmdlst_link_t *cmd_link_entry, unsigned int index)
{
	cmdlst_wr_cfg_t cmdlst_wr_cfg;
	if (memset_s(&cmdlst_wr_cfg, sizeof(cmdlst_wr_cfg_t), 0, sizeof(cmdlst_wr_cfg_t))) {
		loge("Failed : memset_s");
		return ISP_IPP_ERR;
	}
	cmdlst_wr_cfg.is_incr = 1;
	cmdlst_wr_cfg.read_mode = 0;
	logd("+");

	if (req_arf->arf_stat_buff[ARF_KPT_NUM_BUFF].buffer) {
		cmdlst_wr_cfg.data_size = SEG_ENH_ARF_GRID_NUM_RANG;
		cmdlst_wr_cfg.is_wstrb = 0;
		cmdlst_wr_cfg.buff_wr_addr = req_arf->arf_stat_buff[ARF_KPT_NUM_BUFF].buffer;
		cmdlst_wr_cfg.reg_rd_addr = IPP_BASE_ADDR_ARF + AR_FEATURE_KPT_NUMBER_0_REG;
		loge_if(seg_ipp_set_cmdlst_wr_buf(&cmd_link_entry[index].cmd_buf, &cmdlst_wr_cfg));
		index++;
	}

	if (req_arf->arf_stat_buff[ARF_TOTAL_KPT_BUFF].buffer) {
		cmdlst_wr_cfg.data_size = 1;
		cmdlst_wr_cfg.is_wstrb = 0;
		cmdlst_wr_cfg.buff_wr_addr = req_arf->arf_stat_buff[ARF_TOTAL_KPT_BUFF].buffer;
		cmdlst_wr_cfg.reg_rd_addr = IPP_BASE_ADDR_ARF + AR_FEATURE_TOTAL_KPT_NUM_REG;
		loge_if(seg_ipp_set_cmdlst_wr_buf(&cmd_link_entry[index].cmd_buf, &cmdlst_wr_cfg));
		index++;
	}

	if (req_arf->arf_stat_buff[ARF_KLTPRE_TOTAL_KPT_NUM].buffer) {
		cmdlst_wr_cfg.data_size = 1;
		cmdlst_wr_cfg.is_wstrb = 0;
		cmdlst_wr_cfg.buff_wr_addr = req_arf->arf_stat_buff[ARF_KLTPRE_TOTAL_KPT_NUM].buffer;
		cmdlst_wr_cfg.reg_rd_addr = IPP_BASE_ADDR_ARF + AR_FEATURE_TOTAL_KLTPRE_KPT_NUM_REG;
		loge_if(seg_ipp_set_cmdlst_wr_buf(&cmd_link_entry[index].cmd_buf, &cmdlst_wr_cfg));
		index++;
	}

	if (req_arf->arf_stat_buff[ARF_NXT_DOG_THRESHOLD].buffer) {
		cmdlst_wr_cfg.data_size = 1;
		cmdlst_wr_cfg.is_wstrb = 0;
		cmdlst_wr_cfg.buff_wr_addr = req_arf->arf_stat_buff[ARF_NXT_DOG_THRESHOLD].buffer;
		cmdlst_wr_cfg.reg_rd_addr = IPP_BASE_ADDR_ARF + AR_FEATURE_MFPDA_UPDATE_CFG_REG;
		loge_if(seg_ipp_set_cmdlst_wr_buf(&cmd_link_entry[index].cmd_buf, &cmdlst_wr_cfg));
		index++;
	}

	if (req_arf->arf_stat_buff[ARF_KPT_LIMIT_BUFF].buffer) {
		cmdlst_wr_cfg.data_size = SEG_ENH_ARF_KPT_LIMIT_RANG;
		cmdlst_wr_cfg.is_wstrb = 0;
		cmdlst_wr_cfg.buff_wr_addr = req_arf->arf_stat_buff[ARF_KPT_LIMIT_BUFF].buffer;
		cmdlst_wr_cfg.reg_rd_addr = IPP_BASE_ADDR_ARF + AR_FEATURE_KPT_LIMIT_0_REG;
		loge_if(seg_ipp_set_cmdlst_wr_buf(&cmd_link_entry[index].cmd_buf, &cmdlst_wr_cfg));
		index++;
	}

	logd("-");
	return index;
}

static unsigned int seg_enh_arf_cfg_minscr_and_sumscore_cmd_buf(
	msg_req_arf_t *req_arf, schedule_cmdlst_link_t *cmd_link_entry,
	unsigned int i, unsigned int offset, unsigned int base_index)
{
	cmdlst_wr_cfg_t cmdlst_wr_cfg;
	if (memset_s(&cmdlst_wr_cfg, sizeof(cmdlst_wr_cfg_t), 0, sizeof(cmdlst_wr_cfg_t))) {
		loge("Failed : memset_s");
		return ISP_IPP_ERR;
	}

	cmdlst_wr_cfg.is_incr = 1;
	cmdlst_wr_cfg.read_mode = 0;

	if (req_arf->arf_stat_buff[ARF_MINSCR_KPTCNT_BUFF].buffer) {
		cmdlst_wr_cfg.data_size = SEG_ENH_ARF_GRID_NUM_RANG;
		cmdlst_wr_cfg.is_wstrb = 0;
		cmdlst_wr_cfg.buff_wr_addr = req_arf->arf_stat_buff[ARF_MINSCR_KPTCNT_BUFF].buffer +
									SEG_ENH_ARF_MINSCR_KPTCNT_SIZE * offset;
		cmdlst_wr_cfg.reg_rd_addr = IPP_BASE_ADDR_ARF + AR_FEATURE_GRIDSTAT_1_0_REG;
		loge_if(seg_ipp_set_cmdlst_wr_buf(&cmd_link_entry[i + base_index].cmd_buf, &cmdlst_wr_cfg));
		i++;
	}

	if (req_arf->arf_stat_buff[ARF_SUM_SCORE_BUFF].buffer) {
		cmdlst_wr_cfg.data_size = SEG_ENH_ARF_GRID_NUM_RANG;
		cmdlst_wr_cfg.is_wstrb = 0;
		cmdlst_wr_cfg.buff_wr_addr = req_arf->arf_stat_buff[ARF_SUM_SCORE_BUFF].buffer +
									SEG_ENH_ARF_SUM_SCORE_SIZE * offset;
		cmdlst_wr_cfg.reg_rd_addr = IPP_BASE_ADDR_ARF + AR_FEATURE_GRIDSTAT_2_0_REG;
		loge_if(seg_ipp_set_cmdlst_wr_buf(&cmd_link_entry[i + base_index].cmd_buf, &cmdlst_wr_cfg));
		i++;
	}

	return i;
}

static unsigned int seg_enh_arf_cfg_detect_layers_cmd_buf(
	msg_req_arf_t *req_arf, seg_enh_arf_cfg_t *enh_arf_cfg,
	frame_flag_e frame_flag, schedule_cmdlst_link_t *cmd_link_entry,
	seg_enh_arf_stripe_t *stripe_info, unsigned int base_index)
{
	unsigned int cur_layer = 0;
	unsigned int i = 0, offset = 0;
	cmdlst_wr_cfg_t cmdlst_wr_cfg;
	cfg_tab_cvdr_t *cvdr_cfg_tmp = NULL;
	cfg_tab_arf_t *arf_cfg = NULL;
	logd("+");
	if (memset_s(&cmdlst_wr_cfg, sizeof(cmdlst_wr_cfg_t), 0, sizeof(cmdlst_wr_cfg_t))) {
		loge("Failed : memset_s");
		return ISP_IPP_ERR;
	}
	cmdlst_wr_cfg.is_incr = 1;
	cmdlst_wr_cfg.read_mode = 0;

	logd("gauss_total_stripe = %d, detect_total_stripe = %d, layer_stripe_num = %d, base_index = %d",
		stripe_info->gauss_total_stripe, stripe_info->detect_total_stripe, stripe_info->layer_stripe_num, base_index);
	for (i = stripe_info->gauss_total_stripe; i < (stripe_info->gauss_total_stripe + stripe_info->detect_total_stripe);) {
		logd("## i = %d start", i);
		// 1: cfg arf+cvdr
		if (stripe_info->gauss_total_stripe == i) { // first detect;
			cur_layer = stripe_info->gauss_total_stripe +
							((i - stripe_info->gauss_total_stripe) / stripe_info->layer_stripe_num);
			logd("#_1, cur_layer = %d, i = %d, base_index = %d", cur_layer, i, base_index);
			cvdr_cfg_tmp = (frame_flag == FRAME_CUR) ?
							&enh_arf_cfg->arf_cvdr_cur[cur_layer] :
							&enh_arf_cfg->arf_cvdr_ref[cur_layer];
			logd("#_1, set detect layer, stripe_id.%d", (i + base_index));
			arf_cfg = (frame_flag == FRAME_CUR) ? &enh_arf_cfg->arf_cur[cur_layer] : &enh_arf_cfg->arf_ref[cur_layer];
			arfeature_prepare_cmd(&g_arfeature_devs[0], &cmd_link_entry[i + base_index].cmd_buf, arf_cfg);
			cvdr_prepare_cmd(&g_cvdr_devs[0], &cmd_link_entry[i + base_index].cmd_buf, cvdr_cfg_tmp);
			i++;
		} else {
			// update arf next stripe cvdr axi fs for AR mode;
			cur_layer = stripe_info->gauss_total_stripe +
							((i - stripe_info->gauss_total_stripe + 1) / stripe_info->layer_stripe_num);
			cvdr_cfg_tmp = (frame_flag == FRAME_CUR) ?
							&enh_arf_cfg->arf_cvdr_cur[cur_layer] :
							&enh_arf_cfg->arf_cvdr_ref[cur_layer];
			arf_cfg = (frame_flag == FRAME_CUR) ?
							&enh_arf_cfg->arf_cur[cur_layer] :
							&enh_arf_cfg->arf_ref[cur_layer];

			logd("#_2, cur_layer = %d, i = %d, base_index = %d", cur_layer, i, base_index);
			if (arf_cfg->top_cfg.bits.mode == DETECT_AND_DESC_AND_MFPDA_DISABLE ||
				arf_cfg->top_cfg.bits.mode == DETECT_AND_DESC_AND_MFPDA_ENABLE) {
				logd("#_2, set detect layer, stripe_id.%d", (i + 1 + base_index));

				cmdlst_set_vp_wr_flush(&g_cmdlst_devs[0], &(cmd_link_entry[i + 1 + base_index].cmd_buf), stripe_info->channel_id);
				arfeature_prepare_cmd(&g_arfeature_devs[0], &(cmd_link_entry[i + 1 + base_index].cmd_buf), arf_cfg);
				cvdr_cfg_tmp->arf_updata_wr_addr_flag = 1;
				cvdr_prepare_cmd(&g_cvdr_devs[0], &(cmd_link_entry[i + 1 + base_index].cmd_buf), cvdr_cfg_tmp);
				cvdr_cfg_tmp->arf_updata_wr_addr_flag = 0;

				// Update the write address of the VP port;
				cmdlst_wr_cfg.data_size = 1;
				cmdlst_wr_cfg.is_wstrb = 1;
				cmdlst_wr_cfg.buff_wr_addr = cvdr_cfg_tmp->arf_cvdr_wr_addr;
				cmdlst_wr_cfg.reg_rd_addr = IPP_BASE_ADDR_ARF + AR_FEATURE_CVDR_WR_AXI_FS_REG;
				loge_if(seg_ipp_set_cmdlst_wr_buf(&(cmd_link_entry[i + base_index].cmd_buf), &cmdlst_wr_cfg));
				i = i + 2;
			} else if (arf_cfg->top_cfg.bits.mode == DETECT_AND_KLT_AND_MFPDA_DISABLE ||
				arf_cfg->top_cfg.bits.mode == DETECT_AND_KLT_AND_MFPDA_ENABLE) {
				logd("#_3, klt_pre mode, i = %d, base_index = %d", i, base_index);
				arfeature_prepare_cmd(&g_arfeature_devs[0], &(cmd_link_entry[i + base_index].cmd_buf), arf_cfg);
				cvdr_prepare_cmd(&g_cvdr_devs[0], &(cmd_link_entry[i + base_index].cmd_buf), cvdr_cfg_tmp);
				i++;
			} else {
				loge("Failed: current arf mode.%d not the detect mode", arf_cfg->top_cfg.bits.mode);
			}
		}

		// 2: get minscr + sumscore stat data
		offset = cur_layer - stripe_info->gauss_total_stripe;
		logd("cur_layer = %d, offset = %d", cur_layer, offset);
		i = seg_enh_arf_cfg_minscr_and_sumscore_cmd_buf(req_arf, cmd_link_entry, i, offset, base_index);
	}

	logd("-");
	return i + base_index;
}

static int seg_enh_arf_set_enh_cmdlst_stripe(cmdlst_para_t *cmdlst_para, global_info_ipp_t *global_info)
{
	cmdlst_stripe_info_t *cmdlst_stripe;
	unsigned int i = 0;
	unsigned int irq = 0;
	unsigned int stripe_cnt = 1; // only_enh mode, only one stripe;

	cmdlst_stripe = &cmdlst_para->cmd_stripe_info[0];
	for (i = 0; i < stripe_cnt; i++) {
		irq = 0;
		cmdlst_stripe[i].resource_share  = 1 << IPP_CMD_RES_SHARE_ARFEATURE;
		cmdlst_stripe[i].irq_module      = 1 << IPP_CMD_IRQ_MODULE_ORB_ENH;
		cmdlst_stripe[i].en_link         = 0;
		cmdlst_stripe[i].ch_link         = 0;
		cmdlst_stripe[i].ch_link_act_nbr = 0;
		cmdlst_stripe[i].is_last_stripe  = 0;
		cmdlst_stripe[i].is_need_set_sop = 0;
		irq = irq | (((unsigned int)(1)) << IPP_ORB_ENH_CVDR_VP_RD_EOF_CMDLST);
		irq = irq | (((unsigned int)(1)) << IPP_ORB_ENH_DONE);
		irq = irq | (((unsigned int)(1)) << IPP_ORB_ENH_CVDR_VP_RD_EOF_YHIST);
		irq = irq | (((unsigned int)(1)) << IPP_ORB_ENH_CVDR_VP_RD_EOF_YIMG);
		irq = irq | (((unsigned int)(1)) << IPP_ORB_ENH_CVDR_VP_WR_EOF_YGUASS);
		cmdlst_stripe[i].hw_priority     = CMD_PRIO_HIGH;
		cmdlst_stripe[i].irq_mode        = irq;
		logd("orb_enh_stripe=%u, irq=0x%x", i, irq);
	}

	cmdlst_stripe[stripe_cnt - 1].is_last_stripe  = 1;
	cmdlst_stripe[0].hw_priority = CMD_PRIO_LOW;
	cmdlst_para->stripe_cnt  = stripe_cnt;
	cmdlst_para->channel_id  = global_info->enh_arf_channel;
	cmdlst_para->work_module = global_info->work_module;
	cmdlst_para->cmd_entry_mem_id = MEM_ID_CMDLST_ENTRY_ENH_ARF;
	cmdlst_para->cmd_buf_mem_id = MEM_ID_CMDLST_BUF_ENH_ARF;

	if (global_info->klt_en == 1) {
		cmdlst_stripe[stripe_cnt - 1].en_link    = 1;
		cmdlst_stripe[stripe_cnt - 1].ch_link    = global_info->klt_channel;
		cmdlst_stripe[stripe_cnt - 1].ch_link_act_nbr = global_info->klt_stripe_cnt;
	}

	return ISP_IPP_OK;
}

static int seg_enh_arf_set_enh_cmdlst_buff(
	msg_req_enh_arf_t *req, cmdlst_para_t *cmdlst_para, seg_enh_arf_cfg_t *enh_arf_cfg,
	global_info_ipp_t *global_info, frame_flag_e frame_flag)
{
	unsigned int i = 0;
	cfg_tab_orb_enh_t *enh_cfg;
	cfg_tab_cvdr_t *cvdr_cfg_tmp;
	schedule_cmdlst_link_t *cmd_link_entry = (schedule_cmdlst_link_t *)cmdlst_para->cmd_entry;

	ipp_top_do_config(&cmd_link_entry[i].cmd_buf, global_info); // new r02
	enh_cfg = (frame_flag == FRAME_CUR) ? &enh_arf_cfg->enh_cur : &enh_arf_cfg->enh_ref;
	if (orb_enh_prepare_cmd(&g_orb_enh_devs[0], &cmd_link_entry[i].cmd_buf, enh_cfg)) {
		loge("Failed: orb_enh_prepare_cmd");
		return ISP_IPP_ERR;
	}

	// arf cvdr include enh cvdr
	cvdr_cfg_tmp = (frame_flag == FRAME_CUR) ? &enh_arf_cfg->arf_cvdr_cur[i] : &enh_arf_cfg->arf_cvdr_ref[i];
	if (cvdr_prepare_cmd(&g_cvdr_devs[0], &cmd_link_entry[i].cmd_buf, cvdr_cfg_tmp)) {
		loge("Failed: cvdr_prepare_cmd");
		return ISP_IPP_ERR;
	}
	return ISP_IPP_OK;
}

static int seg_enh_arf_set_cmdlst_buff(
	msg_req_enh_arf_t *ipp_path_req, cmdlst_para_t *cmdlst_para, seg_enh_arf_cfg_t *enh_arf_cfg,
	seg_enh_arf_stripe_t *stripe_info, global_info_ipp_t *global_info, frame_flag_e frame_flag)
{
	unsigned int i;
	unsigned int index;
	cfg_tab_arf_t *arf_cfg;
	cfg_tab_orb_enh_t *enh_cfg;
	cfg_tab_cvdr_t *cvdr_cfg_tmp;
	msg_req_arf_t *req_arf = (frame_flag == FRAME_CUR) ? &(ipp_path_req->req_arf_cur) : &(ipp_path_req->req_arf_ref);
	schedule_cmdlst_link_t *cmd_link_entry = (schedule_cmdlst_link_t *)cmdlst_para->cmd_entry;
	unsigned int base = (frame_flag == FRAME_CUR) ? 0 : stripe_info->cur_frame_stripe_cnt;

	logd("start set cmdlst buff: stripe_base = %d", base);
	// 1.gauss operation.
	for (i = 0; i < stripe_info->gauss_total_stripe; i++) {
		logd("### set gauss layer, stripe_id.%d", i);
		if (global_info->enh_en == 1 && i == 0) {
			ipp_top_do_config(&cmd_link_entry[i + base].cmd_buf, global_info); // new r02
			enh_cfg = (frame_flag == FRAME_CUR) ? &enh_arf_cfg->enh_cur : &enh_arf_cfg->enh_ref;
			if (orb_enh_prepare_cmd(&g_orb_enh_devs[0], &cmd_link_entry[i + base].cmd_buf, enh_cfg)) {
				loge("Failed: orb_enh_prepare_cmd");
				return ISP_IPP_ERR;
			}
		}

		arf_cfg = (frame_flag == FRAME_CUR) ? &enh_arf_cfg->arf_cur[i] : &enh_arf_cfg->arf_ref[i];
		if (arfeature_prepare_cmd(&g_arfeature_devs[0], &cmd_link_entry[i + base].cmd_buf, arf_cfg)) {
			loge("Failed: arfeature_prepare_cmd");
			return ISP_IPP_ERR;
		}
		// arf cvdr include enh cvdr
		cvdr_cfg_tmp = (frame_flag == FRAME_CUR) ? &enh_arf_cfg->arf_cvdr_cur[i] : &enh_arf_cfg->arf_cvdr_ref[i];
		if (cvdr_prepare_cmd(&g_cvdr_devs[0], &cmd_link_entry[i + base].cmd_buf, cvdr_cfg_tmp)) {
			loge("Failed: cvdr_prepare_cmd");
			return ISP_IPP_ERR;
		}
	}

	// 2.detect operation.
	stripe_info->channel_id = global_info->enh_arf_channel;
	index = seg_enh_arf_cfg_detect_layers_cmd_buf(req_arf, enh_arf_cfg, frame_flag, cmd_link_entry, stripe_info, base);
	logd("#1 index = %d", index);

	// 3.get stat data.
	index = seg_enh_arf_cfg_kptnum_totalkpt_cmd_buf(req_arf, cmd_link_entry, index);
	logd("#2 index = %d", index);

	if (global_info->matcher_update_flag == 1)
		if (seg_enh_arf_update_matcher_cmdlst(cmd_link_entry, index, global_info, frame_flag)) {
			loge("Failed: seg_enh_arf_update_matcher_cmdlst");
			return ISP_IPP_ERR;
		}

	return ISP_IPP_OK;
}

static int seg_enh_arf_prepare_cmdlst_para(
	msg_req_enh_arf_t *req, seg_enh_arf_cfg_t *enh_arf_cfg,
	cmdlst_para_t *cmdlst_para, global_info_ipp_t *global_info)
{
	seg_enh_arf_stripe_t stripe_info = {0};
	frame_flag_e frame_flag = FRAME_CUR;

	if (global_info->enh_en == 1 && global_info->arf_en == 0) { // ENH_ONLY
		if (seg_enh_arf_set_enh_cmdlst_stripe(cmdlst_para, global_info)) {
			loge("Failed: seg_enh_arf_set_enh_cmdlst_stripe");
			return ISP_IPP_ERR;
		}

		if (df_sched_prepare(cmdlst_para)) {
			loge("Failed: df_sched_prepare");
			return ISP_IPP_ERR;
		}

		if (seg_enh_arf_set_enh_cmdlst_buff(req, cmdlst_para, enh_arf_cfg, global_info, frame_flag)) {
			loge("Failed: seg_enh_arf_set_enh_cmdlst_buff");
			return ISP_IPP_ERR;
		}
	} else { // ARF_ONLY, ENH_ARF, ENH_ARF_MATCHER
		if (seg_enh_arf_set_cmdlst_stripe(req, cmdlst_para, &stripe_info, global_info)) {
			loge("Failed: seg_enh_arf_set_cmdlst_stripe");
			return ISP_IPP_ERR;
		}

		if (df_sched_prepare(cmdlst_para)) {
			loge("Failed: df_sched_prepare");
			return ISP_IPP_ERR;
		}

		if (seg_enh_arf_set_cmdlst_buff(req, cmdlst_para, enh_arf_cfg, &stripe_info, global_info, frame_flag)) {
			loge("Failed: seg_enh_arf_set_cmdlst_buff");
			return ISP_IPP_ERR;
		}

		if (req->work_frame == CUR_REF && global_info->matcher_en == 1) {
			frame_flag = FRAME_REF;
			if (seg_enh_arf_set_cmdlst_buff(req, cmdlst_para, enh_arf_cfg, &stripe_info, global_info, frame_flag)) {
				loge("Failed: seg_enh_arf_set_cmdlst_buff");
				return ISP_IPP_ERR;
			}
		}
	}
	return ISP_IPP_OK;
}

static int seg_enh_arf_get_cpe_mem(
	seg_enh_arf_cfg_t **seg_enh_arf_cfg,
	cmdlst_para_t **cmdlst_para,
	unsigned int work_module)
{
	unsigned long long va = 0;
	unsigned int da = 0;
	int ret = 0;

	// 1.get CFG_TAB
	ret = cpe_mem_get(MEM_ID_ENH_ARF_CFG_TAB, work_module, &va, &da);
	if (ret != 0) {
		loge("Failed : cpe_mem_get %d", MEM_ID_ENH_ARF_CFG_TAB);
		goto ipp_path_arf_get_cpe_mem_failed;
	}
	*seg_enh_arf_cfg = (seg_enh_arf_cfg_t *)(uintptr_t)va;

	// 2.get CMDLST_PARA
	ret = cpe_mem_get(MEM_ID_CMDLST_PARA_ENH_ARF, work_module, &va, &da);
	if (ret != 0) {
		loge("Failed : cpe_mem_get %d", MEM_ID_CMDLST_PARA_ENH_ARF);
		goto ipp_path_arf_get_cpe_mem_failed;
	}

	*cmdlst_para = (cmdlst_para_t *)(uintptr_t)va;

	return ISP_IPP_OK;

ipp_path_arf_get_cpe_mem_failed:
	cpe_mem_free(MEM_ID_ENH_ARF_CFG_TAB, work_module);
	cpe_mem_free(MEM_ID_CMDLST_PARA_ENH_ARF, work_module);
	*seg_enh_arf_cfg = NULL;
	*cmdlst_para = NULL;
	return ISP_IPP_ERR;
}

static int seg_enh_arf_check_stat_buf(msg_req_enh_arf_t *req)
{
	unsigned int i = 0;
	for (i = ARF_MINSCR_KPTCNT_BUFF; i <= ARF_KPT_LIMIT_BUFF; i++) {
		if ((req->req_arf_cur.arf_stat_buff[i].buffer == 0 && req->req_arf_ref.arf_stat_buff[i].buffer != 0) ||
			(req->req_arf_cur.arf_stat_buff[i].buffer != 0 && req->req_arf_ref.arf_stat_buff[i].buffer == 0)) {
			loge("Failed: enh_arf CUR_REF mode, invalid arf_stat_buff[%d]", i);
			return ISP_IPP_ERR;
		}
	}

	return ISP_IPP_OK;
}

static int seg_enh_arf_check_para(
	msg_req_enh_arf_t *req,
	global_info_ipp_t *global_info)
{
	unsigned int i = 0;

	if (global_info->enh_en == 0 && global_info->arf_en == 0) {
		loge("Failed: enh_en = 0 and arf_en = 0");
		return ISP_IPP_ERR;
	}
	if (req->mode_cnt > ARFEATURE_MAX_LAYER || req->work_frame < CUR_ONLY || req->work_frame > CUR_REF) {
		loge("Failed: invalid enh_arf req mode_cnt.%d, work_frame.%d", req->mode_cnt, req->work_frame);
		return ISP_IPP_ERR;
	}

	if (global_info->arf_en != 0 && req->work_frame == CUR_REF) {
		if (seg_enh_arf_check_stat_buf(req) != ISP_IPP_OK)
			return ISP_IPP_ERR;

		for (i = 0; i < req->mode_cnt; i++) {
			if (req->req_arf_cur.reg_cfg[i].top_cfg.bits.mode != req->req_arf_ref.reg_cfg[i].top_cfg.bits.mode) {
				loge("Failed: invalid arf mode, layer.%d", i);
				return ISP_IPP_ERR;
			}
		}
	}
	return ISP_IPP_OK;
}

int seg_enh_arf_request_handler(
	msg_req_enh_arf_t *req,
	global_info_ipp_t *global_info)
{
	int ret = 0;
	seg_enh_arf_cfg_t *seg_enh_arf_cfg = NULL;
	cmdlst_para_t *cmdlst_para = NULL;

	logd("+");
#ifdef IPP_UT_DEBUG
	seg_enh_arf_dump_req(req);
#endif
	ret = seg_enh_arf_check_para(req, global_info);
	if (ret != 0) {
		loge(" Failed : seg_enh_arf_check_para");
		return ISP_IPP_ERR;
	}

	ret = seg_enh_arf_get_cpe_mem(&seg_enh_arf_cfg, &cmdlst_para, global_info->work_module);
	if (ret != 0) {
		loge(" Failed : seg_enh_arf_get_cpe_mem");
		return ISP_IPP_ERR;
	}

	ret = seg_enh_arf_set_cfg_tbl(req, seg_enh_arf_cfg, global_info);
	if (ret != 0) {
		loge(" Failed : seg_enh_arf_set_cfg_tbl");
		goto seg_enh_arf_mem_free;
	}

	ret = seg_enh_arf_prepare_cmdlst_para(req, seg_enh_arf_cfg, cmdlst_para, global_info);
	if (ret != 0) {
		loge(" Failed : seg_enh_arf_prepare_cmdlst_para");
		goto seg_enh_arf_mem_free;
	}

	ret = df_sched_start(cmdlst_para);
	if (ret != 0)
		loge(" Failed : df_sched_start");

	logi("-");

seg_enh_arf_mem_free:
	cpe_mem_free(MEM_ID_ENH_ARF_CFG_TAB, global_info->work_module);
	return ret;
}

#ifdef IPP_UT_DEBUG
static void seg_enh_arf_dump_req(msg_req_enh_arf_t* req)
{
	char *name_work_frame[] = {"CUR_ONLY", "CUR_REF"};
	if (req == NULL){
		loge("Failed : req ptr is NULL");
		return;
	}

	logi("mode_cnt.%d, work_frame.%s",req->mode_cnt, name_work_frame[req->work_frame]);
	logi("### Now dump CUR_frame info:");
	seg_enh_arf_dump_enh_req(&(req->req_enh_cur));
	seg_enh_arf_dump_arf_req(&(req->req_arf_cur), req->mode_cnt);

	if (req->work_frame == CUR_REF) {
		logi("### Now dump REF_frame info:");
		seg_enh_arf_dump_enh_req(&(req->req_enh_ref));
		seg_enh_arf_dump_arf_req(&(req->req_arf_ref), req->mode_cnt);
	}
	return;
}

static void seg_enh_arf_dump_enh_req(msg_req_enh_t *req_enh)
{
	unsigned int j;
	cfg_tab_orb_enh_t *enh_reg_cfg = &(req_enh->reg_cfg);

	if (req_enh == NULL) return;
	logi("---------dump enh start---------");
	logi("---------enh stream's info:");
	for (j = 0; j < ORB_ENH_STREAM_MAX; j++)
		logi("stream[%d]: width = %d, height = %d, stride = %d, fd = %d, offset = 0x%08x",
			j, req_enh->streams[j].width, req_enh->streams[j].height,
			req_enh->streams[j].stride, req_enh->streams[j].fd, req_enh->streams[j].offset);

	logi("---------enh reg_cfg:");
	logi("enh_en = %d", enh_reg_cfg->enable_cfg.bits.enh_en);
	logi("lutscale = %d", enh_reg_cfg->adjust_hist_cfg.lutscale_cfg.bits.lutscale);
	logi("inv_xsize = %d", enh_reg_cfg->adjust_image_cfg.inv_blk_size_x.bits.inv_xsize);
	logi("inv_ysize = %d", enh_reg_cfg->adjust_image_cfg.inv_blk_size_y.bits.inv_ysize);
	logi("blknumx = %d", enh_reg_cfg->cal_hist_cfg.blk_num.bits.blknumx);
	logi("blknumy = %d", enh_reg_cfg->cal_hist_cfg.blk_num.bits.blknumy);
	logi("blk_xsize = %d", enh_reg_cfg->cal_hist_cfg.blk_size.bits.blk_xsize);
	logi("blk_ysize = %d", enh_reg_cfg->cal_hist_cfg.blk_size.bits.blk_ysize);
	logi("climit = %d", enh_reg_cfg->cal_hist_cfg.climit_cfg.bits.climit);
	logi("edgeex_x = %d", enh_reg_cfg->cal_hist_cfg.extend_cfg.bits.edgeex_x);
	logi("edgeex_y = %d", enh_reg_cfg->cal_hist_cfg.extend_cfg.bits.edgeex_y);

	logi("---------dump enh end---------");
	return;
}

static void seg_enh_arf_dump_arf_req(msg_req_arf_t *req_arf, unsigned int mode_cnt)
{
	unsigned int i, j;
	ipp_stream_t (*arf_streams)[ARFEATURE_STREAM_MAX] = req_arf->streams;
	cfg_tab_arf_t *p_reg_cfg = &(req_arf->reg_cfg[0]);

	if (req_arf == NULL) return;
	// one mode corresponds to  one layer
	logi("---------dump arf start---------");

	for (i = 0; i < mode_cnt; ++i) {
		logi("---------arf layer.%d streams info:", i);
		logi("fd = %d, total_size = 0x%08x, fd_flag = %d",
			arf_streams[i][0].fd, arf_streams[i][0].total_size, arf_streams[i][0].fd_flag);
		for (j = 0; j < ARFEATURE_STREAM_MAX; ++j) {
			if (arf_streams[i][j].fd) {
				logi("streams[%d][%d].width  = %d", i, j, arf_streams[i][j].width);
				logi("streams[%d][%d].height = %d", i, j, arf_streams[i][j].height);
				logi("streams[%d][%d].format = %d", i, j, arf_streams[i][j].format);
				logi("streams[%d][%d].fd     = %d, offset = 0x%08x", i, j, arf_streams[i][j].fd, arf_streams[i][j].offset);
			}
		}

		logi("---------arf layer.%d reg_cfg:", i);
		logi("ctrl: mode = %d",         p_reg_cfg[i].top_cfg.bits.mode);
		logi("ctrl: orient_en = %d",    p_reg_cfg[i].top_cfg.bits.orient_en);
		logi("ctrl: layer = %d",        p_reg_cfg[i].top_cfg.bits.layer);
		logi("ctrl: iter_num = %d",     p_reg_cfg[i].top_cfg.bits.iter_num);
		logi("ctrl: first_detect = %d", p_reg_cfg[i].top_cfg.bits.first_detect);
		logi("ctrl: ar_feature_en= %d", p_reg_cfg[i].top_cfg.bits.ar_feature_en);
		logi("ctrl: kltpre_out_en= %d", p_reg_cfg[i].top_cfg.bits.kltpre_out_en);

		logi("img_size: width = %d, height = %d", p_reg_cfg[i].img_size.bits.width, p_reg_cfg[i].img_size.bits.height);
		logi("blk_cfg: blk_h_num = %d, blk_v_num = %d",
			p_reg_cfg[i].blk_cfg.blk_num.bits.blk_h_num, p_reg_cfg[i].blk_cfg.blk_num.bits.blk_v_num);
		logi("blk_cfg: blk_h_size_inv = %d, blk_v_size_inv = %d",
			p_reg_cfg[i].blk_cfg.blk_size.bits.blk_h_size_inv, p_reg_cfg[i].blk_cfg.blk_size.bits.blk_v_size_inv);
		logi("kptpre_cfg: kltpre_blk_v_num = %d, kltpre_blk_h_num = %d, "
			"kltpre_blk_v_size_inv = %d, kltpre_blk_h_size_inv = %d, kltpre_blk_max_kpnum = %d",
			p_reg_cfg[i].kptpre_cfg.blk_num.bits.kltpre_blk_v_num,
			p_reg_cfg[i].kptpre_cfg.blk_num.bits.kltpre_blk_h_num,
			p_reg_cfg[i].kptpre_cfg.blk_inv.bits.kltpre_blk_v_size_inv,
			p_reg_cfg[i].kptpre_cfg.blk_inv.bits.kltpre_blk_h_size_inv,
			p_reg_cfg[i].kptpre_cfg.blk_num.bits.kltpre_blk_max_kpnum);
		logi("sigma_coef: sigma_des = %d, sigma_ori = %d",
			p_reg_cfg[i].sigma_coeff_cfg.bits.sigma_des, p_reg_cfg[i].sigma_coeff_cfg.bits.sigma_ori);
		logi("gauss_org_0~3: %d, %d, %d, %d",
			p_reg_cfg[i].gauss_coeff_cfg.gauss_org.bits.gauss_org_0, p_reg_cfg[i].gauss_coeff_cfg.gauss_org.bits.gauss_org_1,
			p_reg_cfg[i].gauss_coeff_cfg.gauss_org.bits.gauss_org_2, p_reg_cfg[i].gauss_coeff_cfg.gauss_org.bits.gauss_org_3);
		logi("gauss_1st_0~2: %d, %d, %d",
			p_reg_cfg[i].gauss_coeff_cfg.gsblur_1st.bits.gauss_1st_0,
			p_reg_cfg[i].gauss_coeff_cfg.gsblur_1st.bits.gauss_1st_1,
			p_reg_cfg[i].gauss_coeff_cfg.gsblur_1st.bits.gauss_1st_2);
		logi("gauss_2nd_0~3: %d, %d, %d, %d",
			p_reg_cfg[i].gauss_coeff_cfg.gauss_2nd.bits.gauss_2nd_0, p_reg_cfg[i].gauss_coeff_cfg.gauss_2nd.bits.gauss_2nd_1,
			p_reg_cfg[i].gauss_coeff_cfg.gauss_2nd.bits.gauss_2nd_2, p_reg_cfg[i].gauss_coeff_cfg.gauss_2nd.bits.gauss_2nd_3);
		logi("dog_edge_thd_cfg: edge_threshold = %d, dog_threshold = %d",
			p_reg_cfg[i].dog_edge_thd_cfg.bits.edge_threshold, p_reg_cfg[i].dog_edge_thd_cfg.bits.dog_threshold);
		logi("mag_thd_cfg: mag_threshold = %d", p_reg_cfg[i].mag_thd_cfg.bits.mag_threshold);
		logi("detect_num_lmt: max_kpnum = %d, cur_kpnum = %d",
			p_reg_cfg[i].detect_num_lmt.bits.max_kpnum, p_reg_cfg[i].detect_num_lmt.bits.cur_kpnum);
		for (j = 0; j < ARFEATURE_MFPDA_CFG_RANGE; j++)
			logi("mfpda_para: mfpda_kpnum_th = %d, mfpda_score_step = %d, mfpda_grid_kpnum_th = %d, mfpda_grid_score_step = %d",
				p_reg_cfg[i].mfpda_para.mfpda_cfg[j].bits.mfpda_kpnum_th,
				p_reg_cfg[i].mfpda_para.mfpda_cfg[j].bits.mfpda_score_step,
				p_reg_cfg[i].mfpda_para.blk_mfpda_cfg[j].bits.mfpda_grid_kpnum_th,
				p_reg_cfg[i].mfpda_para.blk_mfpda_cfg[j].bits.mfpda_grid_score_step);

		logi("roi_window_cfg: roi_win_mode = %d", p_reg_cfg[i].roi_window_cfg.window_mode.bits.roi_win_mode);
		for (j = 0; j < ARFEATURE_MAX_ROI_WINDOW_NUM; j++)
			logi("roi_window_cfg.%d: roi_start_x = %d, roi_start_y = %d, roi_end_x = %d, roi_end_y = %d", j,
				p_reg_cfg[i].roi_window_cfg.window_start[j].bits.roi_start_x,
				p_reg_cfg[i].roi_window_cfg.window_start[j].bits.roi_start_y,
				p_reg_cfg[i].roi_window_cfg.window_end[j].bits.roi_end_x,
				p_reg_cfg[i].roi_window_cfg.window_end[j].bits.roi_end_y);
	}

	for (i=0; i < ARF_STAT_BUFF_MAX; i++) {
		if (req_arf->arf_stat_buff[i].fd != 0)
			logi("arf_stat_buff: i=%d, width.%d, height.%d, stride.%d, fd.%d, prot.%d, offset.0x%08x, format.%d",
				i, req_arf->arf_stat_buff[i].width, req_arf->arf_stat_buff[i].height, req_arf->arf_stat_buff[i].stride,
				req_arf->arf_stat_buff[i].fd, req_arf->arf_stat_buff[i].prot, req_arf->arf_stat_buff[i].offset,
				req_arf->arf_stat_buff[i].format);
	}

	logi("---------dump arf end---------");
	return;
}
#endif

// Endfile
