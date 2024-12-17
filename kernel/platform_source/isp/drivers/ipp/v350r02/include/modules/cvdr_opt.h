/******************************************************************
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2018. All rights reserved.
 * File name    df_cvdr_opt.h
 * Description:
 *
 * Date         2019-05-24 11:23:22
 *******************************************************************/

#ifndef _CVDR_OPT_CS_H
#define _CVDR_OPT_CS_H
#include "ippmessage_common.h"
#include "cfg_table_cvdr.h"
#include "cvdr_drv.h"
#include "cvdr.h"

#define CVDR_TOTAL_BYTES_ALIGN  0x8000
#define DEFAULT_LINE_WRAP       0x3FFF
#define IPP_CVDR_VP_WR_NBR    (IPP_VP_WR_MAX)
#define IPP_CVDR_VP_RD_NBR    (IPP_VP_RD_MAX)
#define IPP_CVDR_NR_WR_NBR    (IPP_NR_WR_MAX)
#define IPP_CVDR_NR_RD_NBR    (IPP_NR_RD_MAX)
#define BITS_NUM_OF_BYTE	  8

typedef struct _cvdr_opt_bw_t {
	unsigned int       srt;
	unsigned int       pclk;
	unsigned int       throughput;
} cvdr_opt_bw_t;

typedef struct _cvdr_opt_fmt_t {
	unsigned int       id;
	unsigned int       width;
	unsigned int       height;
	unsigned int       line_size;
	unsigned int       addr;
	unsigned int       expand;
	unsigned int       full_width;
	cvdr_pix_fmt_e     pix_fmt;
} cvdr_opt_fmt_t;

typedef struct _pix_fmt_info_t {
	char name[64];
	int  compact_size;
	int  expand_size;
	int  pix_num;
	int  pix_fmt;
} pix_fmt_info_t;

int cfg_tbl_cvdr_nr_wr_cfg(cfg_tab_cvdr_t *p_cvdr_cfg, int id);
int cfg_tbl_cvdr_nr_rd_cfg(cfg_tab_cvdr_t *p_cvdr_cfg, int id);
int dataflow_cvdr_wr_cfg_vp(cfg_tab_cvdr_t *p_cvdr_cfg, cvdr_opt_fmt_t *cfmt,
							unsigned int line_stride, unsigned int isp_clk, pix_format_e format);
int dataflow_cvdr_rd_cfg_vp(cfg_tab_cvdr_t *p_cvdr_cfg, cvdr_opt_fmt_t *cfmt,
							unsigned int line_stride, unsigned int isp_clk, pix_format_e format);
int dataflow_cvdr_wr_cfg_d32(cfg_tab_cvdr_t *p_cvdr_cfg, cvdr_opt_fmt_t *cfmt,
							unsigned int line_stride, unsigned int total_bytes);
int cfg_tbl_cvdr_rd_cfg_d64(cfg_tab_cvdr_t *p_cvdr_cfg, cvdr_opt_fmt_t *cfmt,
							unsigned int total_bytes, unsigned int line_stride);
int cfg_tbl_cvdr_wr_cfg_d64(cfg_tab_cvdr_t *p_cvdr_cfg, cvdr_opt_fmt_t *cfmt,
							unsigned int total_bytes);

#endif /* _CVDR_OPT_CS_H */

/* *******************************end************************************ */

