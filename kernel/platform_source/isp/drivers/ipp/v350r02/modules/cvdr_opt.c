/******************************************************************
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2018. All rights reserved.
 * File name  : cvdr_opt.c
 * Description:
 *
 * Date         2022-07-13 16:28:10
 ********************************************************************/
#include <linux/printk.h>
#include "ipp.h"
#include "ippmessage_common.h"
#include "cfg_table_cvdr.h"
#include "cvdr_opt.h"

#define LOG_TAG LOG_MODULE_CVDR_OPT

enum cvdr_dev_e {
	CVDR_RT  = 0,
	CVDR_SRT = 1,
};

#define ISP_CLK   600  // need confirm throughput
#define DERATE    1.2

static cvdr_opt_bw_t cvdr_vp_wr_bw[IPP_CVDR_VP_WR_NBR] = {
	[IPP_VP_WR_CMDLST]          = {CVDR_SRT, ISP_CLK, (float)DERATE * ISP_CLK},
	[IPP_VP_WR_ORB_ENH_Y]       = {CVDR_SRT, ISP_CLK, ISP_CLK}, // need confirm throughput
	[IPP_VP_WR_ARF_PRY_1]       = {CVDR_SRT, ISP_CLK, ISP_CLK},
	[IPP_VP_WR_ARF_PRY_2]       = {CVDR_SRT, ISP_CLK, ISP_CLK},
	[IPP_VP_WR_ARF_DOG_0]       = {CVDR_SRT, ISP_CLK, ISP_CLK},
	[IPP_VP_WR_ARF_DOG_1]       = {CVDR_SRT, ISP_CLK, ISP_CLK},
	[IPP_VP_WR_ARF_DOG_2]       = {CVDR_SRT, ISP_CLK, ISP_CLK},
	[IPP_VP_WR_ARF_DOG_3]       = {CVDR_SRT, ISP_CLK, ISP_CLK},
	[IPP_VP_WR_ARF_PYR_2_DS]    = {CVDR_SRT, ISP_CLK, ISP_CLK},
	[IPP_VP_WR_MC_RESULT]       = {CVDR_SRT, ISP_CLK, ISP_CLK},
	[IPP_VP_WR_KLT_NEXT_LAYER0] = {CVDR_SRT, ISP_CLK, ISP_CLK},
	[IPP_VP_WR_KLT_NEXT_LAYER1] = {CVDR_SRT, ISP_CLK, ISP_CLK},
	[IPP_VP_WR_KLT_NEXT_LAYER2] = {CVDR_SRT, ISP_CLK, ISP_CLK},
	[IPP_VP_WR_KLT_NEXT_LAYER3] = {CVDR_SRT, ISP_CLK, ISP_CLK},
	[IPP_VP_WR_KLT_NEXT_LAYER4] = {CVDR_SRT, ISP_CLK, ISP_CLK},
};

static cvdr_opt_bw_t cvdr_vp_rd_bw[IPP_CVDR_VP_RD_NBR] = {
	[IPP_VP_RD_CMDLST]          = {CVDR_SRT, ISP_CLK, (float)DERATE * ISP_CLK},
	[IPP_VP_RD_ORB_ENH_Y_HIST]  = {CVDR_SRT, ISP_CLK, ISP_CLK},
	[IPP_VP_RD_ORB_ENH_Y_IMAGE] = {CVDR_SRT, ISP_CLK, ISP_CLK},
	[IPP_VP_RD_RDR]             = {CVDR_SRT, ISP_CLK, ISP_CLK},
	[IPP_VP_RD_CMP]             = {CVDR_SRT, ISP_CLK, ISP_CLK},
	[IPP_VP_RD_ARF_0]           = {CVDR_SRT, ISP_CLK, ISP_CLK},
	[IPP_VP_RD_ARF_1]           = {CVDR_SRT, ISP_CLK, ISP_CLK},
	[IPP_VP_RD_ARF_2]           = {CVDR_SRT, ISP_CLK, ISP_CLK},
	[IPP_VP_RD_ARF_3]           = {CVDR_SRT, ISP_CLK, ISP_CLK},
	[IPP_VP_RD_KLT_SRC]         = {CVDR_SRT, ISP_CLK, ISP_CLK},
};

static cvdr_opt_bw_t cvdr_nr_wr_bw[IPP_CVDR_NR_WR_NBR] = {
	[IPP_NR_WR_RDR]             = {CVDR_SRT, ISP_CLK, ISP_CLK},
};

static cvdr_opt_bw_t cvdr_nr_rd_bw[IPP_CVDR_NR_RD_NBR] = {
	[IPP_NR_RD_MC]              = {CVDR_SRT, ISP_CLK, ISP_CLK}, // MC
	[IPP_NR_RD_CMP]             = {CVDR_SRT, ISP_CLK, ISP_CLK}, // CMP
	[IPP_NR_RD_KLT_PREV_FRAME]  = {CVDR_SRT, ISP_CLK, ISP_CLK}, // KLT
	[IPP_NR_RD_KLT_NEXT_FRAME]  = {CVDR_SRT, ISP_CLK, ISP_CLK}, // KLT
};

static const pix_fmt_info_t pix_fmt_info[DF_FMT_INVALID] = {
	{ "1PF8",   8,  8, 1,  8 }, // 0
	{ "2PF8",  16, 16, 2,  8 },
	{ "0",      0,  0, 0,  0 },
	{ "0",      0,  0, 0,  0 },
	{ "1PF10", 10, 16, 1, 10 }, // 4
	{ "0",     20, 32, 2, 10 },
	{ "0",     30, 32, 3, 10 },
	{ "0",     40, 64, 4, 10 },
	{ "0",     12, 16, 1, 12 },
	{ "0",     24, 32, 2, 12 },
	{ "0",     36, 64, 3, 12 },
	{ "0",     48, 64, 4, 12 },
	{ "0",     14, 16, 1, 14 },
	{ "2PF14", 28, 32, 2, 14 }, // 13
	{ "0",     42, 64, 3, 14 },
	{ "0",     56, 64, 4, 14 },
	{ "0",      8,  8, 1,  8 },
	{ "0",      0,  0, 0,  0 },
	{ "0",      0,  0, 0,  0 },
	{ "0",      0,  0, 0,  0 },
	{ "0",     10, 16, 1, 10 }, // 20
	{ "0",      0,  0, 0,  0 },
	{ "0",      0,  0, 0,  0 },
	{ "0",      0,  0, 0,  0 },
	{ "0",      0,  0, 0,  0 },
	{ "3PF8",  24, 32, 3,  8 }, // 25
	{ "0",      0,  0, 0,  0 },
	{ "0",      0,  0, 0,  0 },
	{ "D32",   32, 32, 1, 32 }, // 28
	{ "D48",   48, 64, 1, 48 }, // 29
	{ "D64",   64, 64, 1, 64 }, // 30
	{ "D128",  128, 128, 1, 128}, // 31
};

int calculate_cvdr_bw_limiter(cvdr_bw_cfg_t *bw, unsigned int throughput)
{
	// int num_75mbytes; // v350 no use
	// num_75mbytes  = (throughput + 74) / 75;
	bw->bw_limiter0  = 0xF;
	bw->bw_limiter1  = 0xF;
	bw->bw_limiter2  = 0xF;
	bw->bw_limiter3  = 0xF;
	bw->bw_limiter_reload = 0xF;
	logd("bw_limiter: %d, %d, %d, %d, reload: %x, throughput: %d",
		 bw->bw_limiter0, bw->bw_limiter1, bw->bw_limiter2,
		 bw->bw_limiter3, bw->bw_limiter_reload, throughput);
	return ISP_IPP_OK;
}

static unsigned short calculate_cvdr_allocated_du(unsigned int pclk, unsigned int throughput, int is_dgen)
{
	unsigned short num_du;

	if (throughput > 0)
		num_du = 6;
	else
		num_du = 0;

	return num_du;
}

int cfg_tbl_cvdr_nr_wr_cfg(cfg_tab_cvdr_t *p_cvdr_cfg, int id)
{
	p_cvdr_cfg->nr_wr_cfg[id].to_use = 1;
	p_cvdr_cfg->nr_wr_cfg[id].en     = 1;
	calculate_cvdr_bw_limiter(&(p_cvdr_cfg->nr_wr_cfg[id].bw), cvdr_nr_wr_bw[id].throughput);
	return ISP_IPP_OK;
}

int cfg_tbl_cvdr_nr_rd_cfg(cfg_tab_cvdr_t *p_cvdr_cfg, int id)
{
	p_cvdr_cfg->nr_rd_cfg[id].to_use = 1;
	p_cvdr_cfg->nr_rd_cfg[id].en     = 1;
	p_cvdr_cfg->nr_rd_cfg[id].allocated_du = 6; // 6->12? -ls
	calculate_cvdr_bw_limiter(&(p_cvdr_cfg->nr_rd_cfg[id].bw), cvdr_nr_rd_bw[id].throughput);
	logd("allocated_du: %d, pclk: %d", p_cvdr_cfg->nr_rd_cfg[id].allocated_du, cvdr_nr_rd_bw[id].pclk);
	return ISP_IPP_OK;
}

static int cvdr_wr_cfg_vp_format_convert(pix_format_e format, cvdr_pix_fmt_e *pix_fmt, unsigned int *config_num)
{
	switch (format) {
	case PIXEL_FMT_IPP_Y8:
	case PIXEL_FMT_IPP_1PF8:
		*pix_fmt    = DF_1PF8;
		*config_num = 1;
		break;

	case PIXEL_FMT_IPP_2PF8:
		*pix_fmt    = DF_2PF8;
		*config_num = 1;
		break;

	case PIXEL_FMT_IPP_2PF14:
		*pix_fmt    = DF_2PF14;
		*config_num = 1;
		break;

	case PIXEL_FMT_IPP_3PF8:
		*pix_fmt    = DF_3PF8;
		*config_num = 1;
		break;

	case PIXEL_FMT_IPP_D32:
		*pix_fmt    = DF_D32;
		*config_num = 1;
		break;

	case PIXEL_FMT_IPP_D64:
		*pix_fmt    = DF_D64;
		*config_num = 1;
		break;

	case PIXEL_FMT_IPP_D128:
		*pix_fmt    = DF_D128;
		*config_num = 1;
		break;

	default:
		loge("Failed:pix_fmt unsupported! format = %d", format);
		return ISP_IPP_ERR;
	}

	return ISP_IPP_OK;
}

int dataflow_cvdr_wr_cfg_vp(cfg_tab_cvdr_t *p_cvdr_cfg, cvdr_opt_fmt_t *cfmt,
							unsigned int line_stride, unsigned int isp_clk, pix_format_e format)
{
	unsigned int i;
	unsigned int pix_size;
	unsigned int config_num = 0;
	unsigned int config_width = 0;
	unsigned int total_bytes[2] = {0};
	cvdr_pix_fmt_e pix_fmt = DF_FMT_INVALID;
	cvdr_vp_wr_cfg_t *vp_wr_cfg = NULL;

	if (p_cvdr_cfg == NULL || cfmt == NULL) {
		loge("Failed:NULL ptr, p_cvdr_cfg.%pK, cfmt.%pK", p_cvdr_cfg, cfmt);
		return ISP_IPP_ERR;
	}

	if (cvdr_wr_cfg_vp_format_convert(format, &pix_fmt, &config_num)) {
		loge("Failed: cvdr_wr_cfg_vp_format_convert");
		return ISP_IPP_ERR;
	}
	cfmt->pix_fmt = pix_fmt;
	pix_size = (cfmt->expand == EXP_PIX) ? pix_fmt_info[pix_fmt].expand_size : pix_fmt_info[pix_fmt].compact_size;
	config_width   = align_up(cfmt->width * pix_size / BITS_NUM_OF_BYTE, CVDR_ALIGN_BYTES);
	total_bytes[0] = align_up(config_width * cfmt->height, CVDR_TOTAL_BYTES_ALIGN);
	total_bytes[1] = align_up(config_width * cfmt->height, CVDR_TOTAL_BYTES_ALIGN);

	for (i = cfmt->id; i < (cfmt->id + config_num); i++) {
		vp_wr_cfg         = &(p_cvdr_cfg->vp_wr_cfg[i]);
		vp_wr_cfg->to_use = 1;
		vp_wr_cfg->id     = i;
		vp_wr_cfg->fmt.fs_addr   = cfmt->addr + (i - cfmt->id) * (config_width * cfmt->height);
		vp_wr_cfg->fmt.last_page = (p_cvdr_cfg->vp_wr_cfg[i].fmt.fs_addr +
			align_up(align_up(cfmt->full_width * pix_size / BITS_NUM_OF_BYTE, CVDR_ALIGN_BYTES) * cfmt->height,
			CVDR_TOTAL_BYTES_ALIGN)) >> 15;
		vp_wr_cfg->fmt.pix_fmt     = pix_fmt;
		vp_wr_cfg->fmt.pix_expan   = cfmt->expand;
		vp_wr_cfg->fmt.line_stride =
			align_up(cfmt->full_width * pix_size / BITS_NUM_OF_BYTE, CVDR_ALIGN_BYTES) / CVDR_ALIGN_BYTES - 1;
		vp_wr_cfg->fmt.line_wrap   = DEFAULT_LINE_WRAP;
		calculate_cvdr_bw_limiter(&(vp_wr_cfg->bw), cvdr_vp_wr_bw[i].throughput);

		if (line_stride != 0) {
			unsigned int cvdr_total_bytes = align_up(line_stride, CVDR_ALIGN_BYTES) * cfmt->height;
			cvdr_total_bytes = align_up(cvdr_total_bytes, CVDR_TOTAL_BYTES_ALIGN);
			vp_wr_cfg->fmt.line_stride = align_up(line_stride, CVDR_ALIGN_BYTES) / CVDR_ALIGN_BYTES - 1;
			vp_wr_cfg->fmt.last_page   = (vp_wr_cfg->fmt.fs_addr +
											align_up(cvdr_total_bytes, CVDR_TOTAL_BYTES_ALIGN)) >> 15;
		}

		logd("fs_addr = 0x%x, id = %d, line_stride = %d, cfmt->full_width = %d",
			p_cvdr_cfg->vp_wr_cfg[i].fmt.fs_addr, i,
			p_cvdr_cfg->vp_wr_cfg[i].fmt.line_stride, cfmt->full_width);
		logd("height = %d, config_width = %d", cfmt->height, config_width);
		logd("last_page = 0x%x, total_byte = %d, pix_fmt = 0x%x", p_cvdr_cfg->vp_wr_cfg[i].fmt.last_page,
			total_bytes[i - cfmt->id], p_cvdr_cfg->vp_wr_cfg[i].fmt.pix_fmt);
	}

	return ISP_IPP_OK;
}

static int cvdr_rd_cfg_vp_format_convert(pix_format_e format, cvdr_pix_fmt_e *pix_fmt)
{
	switch (format) {
	case PIXEL_FMT_IPP_Y8:
	case PIXEL_FMT_IPP_1PF8:
		*pix_fmt = DF_1PF8;
		break;

	case PIXEL_FMT_IPP_1PF10:
		*pix_fmt = DF_1PF10;
		break;

	case PIXEL_FMT_IPP_2PF8:
		*pix_fmt = DF_2PF8;
		break;

	case PIXEL_FMT_IPP_3PF8:
		*pix_fmt = DF_3PF8;
		break;

	case PIXEL_FMT_IPP_D32:
		*pix_fmt = DF_D32;
		break;

	case PIXEL_FMT_IPP_D64:
		*pix_fmt = DF_D64;
		break;

	case PIXEL_FMT_IPP_D128:
		*pix_fmt = DF_D128;
		break;

	default:
		loge("Failed:pix_fmt unsupported! format = %d", format);
		return ISP_IPP_ERR;
	}

	return ISP_IPP_OK;
}


int dataflow_cvdr_rd_cfg_vp(cfg_tab_cvdr_t *p_cvdr_cfg, cvdr_opt_fmt_t *cfmt,
	unsigned int line_stride, unsigned int isp_clk, pix_format_e format)
{
	int ret = 0;
	unsigned short du;
	unsigned int pix_size;
	unsigned int id;
	cvdr_pix_fmt_e pix_fmt = DF_FMT_INVALID;

	loge_if_null_set_ret(p_cvdr_cfg, ret);
	if (ret) return ret;
	loge_if_null_set_ret(cfmt, ret);
	if (ret) return ret;

	id = cfmt->id;
	if (cvdr_rd_cfg_vp_format_convert(format, &pix_fmt)) {
		loge("Failed: cvdr_rd_cfg_vp_format_convert");
		return ISP_IPP_ERR;
	}
	pix_size = (cfmt->expand == EXP_PIX) ? pix_fmt_info[pix_fmt].expand_size : pix_fmt_info[pix_fmt].compact_size;
	du = calculate_cvdr_allocated_du(cvdr_vp_rd_bw[id].pclk, cvdr_vp_rd_bw[id].throughput, 0);
	p_cvdr_cfg->vp_rd_cfg[id].to_use = 1;
	p_cvdr_cfg->vp_rd_cfg[id].id     = id;
	p_cvdr_cfg->vp_rd_cfg[id].fmt.fs_addr      = cfmt->addr;
	p_cvdr_cfg->vp_rd_cfg[id].fmt.pix_fmt      = pix_fmt;
	p_cvdr_cfg->vp_rd_cfg[id].fmt.pix_expan    = cfmt->expand;
	p_cvdr_cfg->vp_rd_cfg[id].fmt.allocated_du = du;
	p_cvdr_cfg->vp_rd_cfg[id].fmt.line_size    = cfmt->line_size - 1;
	p_cvdr_cfg->vp_rd_cfg[id].fmt.vprd_ihleft  = 0; // set 0 in IPP;
	p_cvdr_cfg->vp_rd_cfg[id].fmt.frame_size   = cfmt->height - 1;
	p_cvdr_cfg->vp_rd_cfg[id].fmt.line_wrap    = DEFAULT_LINE_WRAP;
	p_cvdr_cfg->vp_rd_cfg[id].fmt.line_stride  =
		align_up(cfmt->full_width * pix_size / BITS_NUM_OF_BYTE, CVDR_ALIGN_BYTES) / CVDR_ALIGN_BYTES - 1;
	p_cvdr_cfg->vp_rd_cfg[id].fmt.last_page    = (cfmt->addr +
		align_up(align_up(cfmt->full_width * pix_size / BITS_NUM_OF_BYTE, CVDR_ALIGN_BYTES) * cfmt->height,
		CVDR_TOTAL_BYTES_ALIGN)) >> 15;

	if (line_stride != 0) {
		p_cvdr_cfg->vp_rd_cfg[id].fmt.line_stride = align_up(line_stride, CVDR_ALIGN_BYTES) / CVDR_ALIGN_BYTES - 1;
		p_cvdr_cfg->vp_rd_cfg[id].fmt.last_page   = (cfmt->addr +
			align_up(align_up(line_stride, CVDR_ALIGN_BYTES) * cfmt->height,
			CVDR_TOTAL_BYTES_ALIGN)) >> 15;
	}

	logd("fs_addr = 0x%x, id = %d, line_stride = %d, line_size = %d, frame_size = %d",
		 p_cvdr_cfg->vp_rd_cfg[id].fmt.fs_addr, id, p_cvdr_cfg->vp_rd_cfg[id].fmt.line_stride,
		 p_cvdr_cfg->vp_rd_cfg[id].fmt.line_size, p_cvdr_cfg->vp_rd_cfg[id].fmt.frame_size);
	logd("pix_fmt = 0x%x, pix_expan = 0x%x, allocated_du = 0x%x, last_page = 0x%x",
		 pix_fmt, p_cvdr_cfg->vp_rd_cfg[id].fmt.pix_expan,
		 p_cvdr_cfg->vp_rd_cfg[id].fmt.allocated_du, p_cvdr_cfg->vp_rd_cfg[id].fmt.last_page);
	calculate_cvdr_bw_limiter(&(p_cvdr_cfg->vp_rd_cfg[id].bw), cvdr_vp_rd_bw[id].throughput);
	return ISP_IPP_OK;
}

int dataflow_cvdr_wr_cfg_d32(cfg_tab_cvdr_t *p_cvdr_cfg, cvdr_opt_fmt_t *cfmt,
										unsigned int line_stride, unsigned int total_bytes)
{
	unsigned int id;
	int ret = 0;

	loge_if_null_set_ret(p_cvdr_cfg, ret);
	if (ret) return ret;
	loge_if_null_set_ret(cfmt, ret);
	if (ret) return ret;

	id = cfmt->id;
	p_cvdr_cfg->vp_wr_cfg[id].to_use = 1;
	p_cvdr_cfg->vp_wr_cfg[id].id = id;
	p_cvdr_cfg->vp_wr_cfg[id].fmt.fs_addr     = cfmt->addr;
	p_cvdr_cfg->vp_wr_cfg[id].fmt.last_page   = (p_cvdr_cfg->vp_wr_cfg[id].fmt.fs_addr + total_bytes) >> 15;
	p_cvdr_cfg->vp_wr_cfg[id].fmt.pix_fmt     = DF_D32;
	p_cvdr_cfg->vp_wr_cfg[id].fmt.pix_expan   = 1;
	p_cvdr_cfg->vp_wr_cfg[id].fmt.line_stride = line_stride;
	p_cvdr_cfg->vp_wr_cfg[id].fmt.line_wrap   = DEFAULT_LINE_WRAP;
	calculate_cvdr_bw_limiter(&(p_cvdr_cfg->vp_wr_cfg[id].bw), cvdr_vp_wr_bw[id].throughput);
	return ISP_IPP_OK;
}

int cfg_tbl_cvdr_rd_cfg_d64(cfg_tab_cvdr_t *p_cvdr_cfg, cvdr_opt_fmt_t *cfmt,
										unsigned int total_bytes, unsigned int line_stride)
{
	int ret = 0;
	unsigned int id;
	unsigned short allocated_du = 2;

	loge_if_null_set_ret(p_cvdr_cfg, ret);
	if (ret) return ret;
	loge_if_null_set_ret(cfmt, ret);
	if (ret) return ret;

	id = cfmt->id;
	p_cvdr_cfg->vp_rd_cfg[id].to_use = 1;
	p_cvdr_cfg->vp_rd_cfg[id].id = cfmt->id;
	p_cvdr_cfg->vp_rd_cfg[id].fmt.fs_addr      = cfmt->addr;
	p_cvdr_cfg->vp_rd_cfg[id].fmt.last_page    = (cfmt->addr + total_bytes) >> 15;
	p_cvdr_cfg->vp_rd_cfg[id].fmt.pix_fmt      = DF_D64;
	p_cvdr_cfg->vp_rd_cfg[id].fmt.pix_expan    = 1;
	p_cvdr_cfg->vp_rd_cfg[id].fmt.allocated_du = allocated_du;
	p_cvdr_cfg->vp_rd_cfg[id].fmt.line_size    = cfmt->line_size - 1;
	p_cvdr_cfg->vp_rd_cfg[id].fmt.vprd_ihleft  = 0; // no use current version
	p_cvdr_cfg->vp_rd_cfg[id].fmt.frame_size   = cfmt->height - 1;
	p_cvdr_cfg->vp_rd_cfg[id].fmt.line_stride  = (line_stride > 0) ? (line_stride - 1) : 0;
	p_cvdr_cfg->vp_rd_cfg[id].fmt.line_wrap    = DEFAULT_LINE_WRAP;
	calculate_cvdr_bw_limiter(&(p_cvdr_cfg->vp_rd_cfg[id].bw), cvdr_vp_rd_bw[id].throughput);

	return ISP_IPP_OK;
}

int cfg_tbl_cvdr_wr_cfg_d64(cfg_tab_cvdr_t *p_cvdr_cfg, cvdr_opt_fmt_t *cfmt, unsigned int total_bytes)
{
	int ret = 0;
	unsigned int id;

	loge_if_null_set_ret(p_cvdr_cfg, ret);
	if (ret) return ret;
	loge_if_null_set_ret(cfmt, ret);
	if (ret) return ret;

	id = cfmt->id;
	p_cvdr_cfg->vp_wr_cfg[id].to_use = 1;
	p_cvdr_cfg->vp_wr_cfg[id].id = id;
	p_cvdr_cfg->vp_wr_cfg[id].fmt.fs_addr     = cfmt->addr;
	p_cvdr_cfg->vp_wr_cfg[id].fmt.last_page   = (p_cvdr_cfg->vp_wr_cfg[id].fmt.fs_addr + total_bytes) >> 15;
	p_cvdr_cfg->vp_wr_cfg[id].fmt.pix_fmt     = DF_D64;
	p_cvdr_cfg->vp_wr_cfg[id].fmt.pix_expan   = 1;
	p_cvdr_cfg->vp_wr_cfg[id].fmt.line_stride = 0;
	p_cvdr_cfg->vp_wr_cfg[id].fmt.line_wrap   = DEFAULT_LINE_WRAP;
	calculate_cvdr_bw_limiter(&(p_cvdr_cfg->vp_wr_cfg[id].bw), cvdr_vp_wr_bw[id].throughput);

	logd("fs_addr = 0x%x, id = %d, last_page = 0x%x, total_byte = %d",
		 p_cvdr_cfg->vp_wr_cfg[id].fmt.fs_addr, id,
		 p_cvdr_cfg->vp_wr_cfg[id].fmt.last_page, total_bytes);

	return ISP_IPP_OK;
}

/* ***************************end************************************** */

