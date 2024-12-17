/******************************************************************
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2018. All rights reserved.
 * File name  : cfg_table_cvdr.h
 * Description:
 *
 * Date         2020-04-17 16:28:10
 ********************************************************************/

#ifndef __CONFIG_TABLE_CVDR_CS_H_INCLUDED__
#define __CONFIG_TABLE_CVDR_CS_H_INCLUDED__

#include "cvdr.h"

enum cvdr_prefetch_mode_e {
	OLD_CVDR_PREFETCH_EN,
	OLD_CVDR_PREFETCH_BYPASS,
};

enum pix_type_e {
	STD_PIX = 0,
	EXP_PIX = 1,
}; // expanded or not

/********************************************************************************************
Pixel formats supported (see ref. 'vprd_pixel_format_0' reg)
ReturnToLine 0:3   : 1/2/4/8 PF8
ReturnToLine 4:7   : 1/2/4/8 PF10
ReturnToLine 8:11  : 1/2/4/8 PF12
ReturnToLine 12:15 : 1/2/4/8 PF14
ReturnToLine 16:23 : NA
ReturnToLine 24    : 1PF16 (from V350 R02)
ReturnToLine 25    : 3PF8 (IPP)
ReturnToLine 26    : 6PF12 (72 bits)
ReturnToLine 27    : 6PF16 (96 bits)
ReturnToLine 28    : D32 (or 2PF16)
ReturnToLine 29    : D48 
ReturnToLine 30    : D64
ReturnToLine 31    : D128.
********************************************************************************************/
typedef enum _cvdr_pix_fmt_e {
	DF_1PF8  = 0,
	DF_2PF8  = 1,
	DF_4PF8  = 2,
	DF_8PF8  = 3,
	DF_1PF10 = 4,
	DF_2PF10 = 5,
	DF_4PF10 = 6,
	DF_8PF10 = 7,
	DF_1PF12 = 8,
	DF_2PF12 = 9,
	DF_4PF12 = 10,
	DF_8PF12 = 11,
	DF_1PF14 = 12,
	DF_2PF14 = 13,
	DF_4PF14 = 14,
	DF_8PF14 = 15,
	DF_1PF16 = 24,
	DF_3PF8  = 25,
	DF_6PF12 = 26,
	DF_6PF16 = 27,
	DF_D32   = 28,
	DF_D48   = 29,
	DF_D64   = 30,
	DF_D128  = 31,
	DF_FMT_INVALID,
} cvdr_pix_fmt_e;

typedef struct _cvdr_bw_cfg_t {
	unsigned char       bw_limiter0;
	unsigned char       bw_limiter1;
	unsigned char       bw_limiter2;
	unsigned char       bw_limiter3;
	unsigned char       bw_limiter_reload;
} cvdr_bw_cfg_t;

typedef struct _cvdr_wr_fmt_desc_t {
	unsigned int        fs_addr;
	unsigned int        last_page;

	cvdr_pix_fmt_e      pix_fmt;
	unsigned char       pix_expan;
	unsigned short      line_stride;
	unsigned short      line_wrap;
	unsigned short      line_start_wstrb;
	unsigned short      mono_mode; // no use ???
} cvdr_wr_fmt_desc_t;

typedef struct _cvdr_rd_fmt_desc_t {
	unsigned int        fs_addr;
	unsigned int        last_page;
	cvdr_pix_fmt_e      pix_fmt;
	unsigned char       pix_expan;
	unsigned short      allocated_du;
	unsigned short      line_size;
	unsigned short      vprd_ihleft; // new val in r02, set 0 in IPP; We don't use the cropping function;
	unsigned short      frame_size;
	unsigned short      line_stride;
	unsigned short      line_wrap;
} cvdr_rd_fmt_desc_t;

typedef struct _cvdr_vp_wr_cfg_t {
	unsigned char       to_use;

	unsigned int        id;
	cvdr_wr_fmt_desc_t  fmt;
	cvdr_bw_cfg_t       bw;
} cvdr_vp_wr_cfg_t;

typedef struct _cvdr_vp_rd_cfg_t {
	unsigned char       to_use;

	unsigned char       id;
	cvdr_rd_fmt_desc_t  fmt;
	cvdr_bw_cfg_t       bw;
} cvdr_vp_rd_cfg_t;

typedef struct _cvdr_nr_wr_cfg_t {
	unsigned char       to_use;

	unsigned char       nr_wr_stop_en_du_thr;
	unsigned char       nr_wr_stop_en_flux_ctrl;
	unsigned char       nr_wr_stop_en_pressure;
	unsigned char       nr_wr_stop_ok;
	unsigned char       nr_wr_stop;
	unsigned char       en;
	cvdr_bw_cfg_t       bw;
} cvdr_nr_wr_cfg_t;

typedef struct _cvdr_nr_rd_cfg_t {
	unsigned char       to_use;

	unsigned short      allocated_du;
	unsigned char       nr_rd_stop_en_du_thr;
	unsigned char       nr_rd_stop_en_flux_ctrl;
	unsigned char       nr_rd_stop_en_pressure;
	unsigned char       nr_rd_stop_ok;
	unsigned char       nr_rd_stop;
	unsigned char       en;
	cvdr_bw_cfg_t       bw;
} cvdr_nr_rd_cfg_t;

typedef struct _cfg_tab_cvdr_t {
	cvdr_vp_wr_cfg_t  vp_wr_cfg[IPP_VP_WR_MAX];
	cvdr_vp_rd_cfg_t  vp_rd_cfg[IPP_VP_RD_MAX];
	cvdr_nr_wr_cfg_t  nr_wr_cfg[IPP_NR_WR_MAX];
	cvdr_nr_rd_cfg_t  nr_rd_cfg[IPP_NR_RD_MAX];

	unsigned int      rdr_cvdr_frame_size_addr; // address in cmdlst_buffer
	unsigned int      cmp_cvdr_frame_size_addr; // address in cmdlst_buffer
	unsigned int      arf_cvdr_wr_addr; // address in cmdlst_buffer

	unsigned int      arf_updata_wr_addr_flag;
} cfg_tab_cvdr_t;

#endif /* __CONFIG_TABLE_CVDR_CS_H_INCLUDED__ */

/* ********************************* END ********************************* */
