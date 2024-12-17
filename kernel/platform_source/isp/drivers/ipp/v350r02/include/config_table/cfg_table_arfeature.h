/******************************************************************
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2018. All rights reserved.
 * File name  :
 * Description:
 *
 * Date         2022-06-25 16:28:10
 ********************************************************************/

#ifndef __CFG_TABLE_ARFEATURE_CS_H__
#define __CFG_TABLE_ARFEATURE_CS_H__

#include "arfeature_drv_priv.h"

#define ARFEATURE_MAX_BLK_NUM               (16*16)
#define ARFEATURE_MAX_ROI_WINDOW_NUM        (8)
#define ARFEATURE_MFPDA_CFG_RANGE           (6)

typedef union arfeature_top_cfg                    arf_top_cfg_t;             
typedef union arfeature_image_size                 arf_img_size_t;
typedef union arfeature_block_num_cfg              arf_blk_num_cfg_t;
typedef union arfeature_block_size_cfg_inv         arf_blk_size_cfg_inv_t;
typedef union arfeature_detect_number_limit        arf_detect_num_lmt_t;
typedef union arfeature_sigma_coef                 arf_sigma_coef_t;
typedef union arfeature_gauss_org                  arf_gauss_org_t;
typedef union arfeature_gsblur_1st                 arf_gsblur_1st_t;
typedef union arfeature_gauss_2nd                  arf_gauss_2nd_t;
typedef union arfeature_dog_edge_threshold         arf_dog_edge_thd_t;
typedef union arfeature_descriptor_threshold       arf_descriptor_thd_t;
typedef union arfeature_cvdr_rd_cfg                arf_cvdr_rd_cfg_t;
typedef union arfeature_cvdr_rd_lwg                arf_cvdr_rd_lwg_t;
typedef union arfeature_cvdr_rd_fhg                arf_cvdr_rd_fhg_t;
typedef union arfeature_cvdr_rd_axi_fs             arf_cvdr_rd_axi_fs_t;
typedef union arfeature_cvdr_rd_axi_line           arf_cvdr_rd_axi_line_t;
typedef union arfeature_cvdr_rd_if_cfg             arf_cvdr_rd_if_cfg_t;
typedef union arfeature_pre_cvdr_rd_fhg            arf_pre_cvdr_rd_fhg_t;
typedef union arfeature_cvdr_wr_cfg                arf_cvdr_wr_cfg_t;
typedef union arfeature_cvdr_wr_axi_fs             arf_cvdr_wr_axi_fs_t;
typedef union arfeature_cvdr_wr_axi_line           arf_cvdr_wr_axi_line_t;
typedef union arfeature_cvdr_wr_if_cfg             arf_cvdr_wr_if_cfg_t;
typedef union arfeature_pre_cvdr_wr_axi_fs         arf_pre_cvdr_wr_axi_fs_t;
typedef union arfeature_klt_pre_block_size_cfg_inv arf_klt_pre_blk_size_cfg_inv_t;
typedef union arfeature_klt_pre_block_cfg          arf_klt_pre_blk_cfg_t;
typedef union arfeature_mfpda_cfg                  arf_mfpda_cfg_t;
typedef union arfeature_block_mfpda_cfg            arf_blk_mfpda_cfg_t;
typedef union arfeature_roi_window_start_cfg       arf_roi_window_start_cfg_t;
typedef union arfeature_roi_window_end_cfg         arf_roi_window_end_cfg_t;
typedef union arfeature_roi_window_mode_cfg        arf_roi_window_mode_cfg_t;
typedef union arfeature_kpt_limit                  arf_kpt_limit_t;

typedef struct _arf_blk_cfg_t {
	arf_blk_num_cfg_t      blk_num;
	arf_blk_size_cfg_inv_t blk_size;
} arf_blk_cfg_t;

typedef struct _arf_gauss_coeff_t {
	arf_gauss_org_t   gauss_org;
	arf_gsblur_1st_t  gsblur_1st;
	arf_gauss_2nd_t   gauss_2nd;
} arf_gauss_coeff_t;

typedef struct _arf_klt_pre_cfg_t {
	arf_klt_pre_blk_size_cfg_inv_t blk_inv;
	arf_klt_pre_blk_cfg_t          blk_num;
} arf_klt_pre_cfg_t;

typedef struct _arf_mfpda_para_cfg_t {
	arf_mfpda_cfg_t     mfpda_cfg[ARFEATURE_MFPDA_CFG_RANGE];
	arf_blk_mfpda_cfg_t blk_mfpda_cfg[ARFEATURE_MFPDA_CFG_RANGE];
} arf_mfpda_para_cfg_t;

typedef struct _arf_roi_window_cfg_t {
	arf_roi_window_mode_cfg_t  window_mode;
	arf_roi_window_start_cfg_t window_start[ARFEATURE_MAX_ROI_WINDOW_NUM];
	arf_roi_window_end_cfg_t   window_end[ARFEATURE_MAX_ROI_WINDOW_NUM];
} arf_roi_window_cfg_t;

typedef struct _arf_cvdr_rd_cfg_para_t {
	arf_cvdr_rd_cfg_t      rd_base_cfg;
	arf_cvdr_rd_lwg_t      rd_lwg;
	arf_pre_cvdr_rd_fhg_t  pre_rd_fhg;
	arf_cvdr_rd_axi_fs_t   rd_axi_fs;
	arf_cvdr_rd_axi_line_t rd_axi_line;
	arf_cvdr_rd_if_cfg_t   rd_if_cfg;
} arf_cvdr_rd_cfg_para_t;

typedef struct _arf_cvdr_wr_cfg_para_t {
	arf_cvdr_wr_cfg_t        wr_base_cfg;
	arf_pre_cvdr_wr_axi_fs_t pre_wr_axi_fs;
	arf_cvdr_wr_axi_line_t   wr_axi_line;
	arf_cvdr_wr_if_cfg_t     wr_if_cfg;
} arf_cvdr_wr_cfg_para_t;

typedef struct _arf_cvdr_cfg_t {
	arf_cvdr_rd_cfg_para_t  rd_cfg;
	arf_cvdr_wr_cfg_para_t  wr_cfg;
} arf_cvdr_cfg_t;

typedef struct _cfg_tab_arf_t {
	arf_top_cfg_t         top_cfg;
	arf_img_size_t        img_size;
	arf_blk_cfg_t         blk_cfg;
	arf_detect_num_lmt_t  detect_num_lmt;
	arf_sigma_coef_t      sigma_coeff_cfg;
	arf_gauss_coeff_t     gauss_coeff_cfg;
	arf_dog_edge_thd_t    dog_edge_thd_cfg;
	arf_descriptor_thd_t  mag_thd_cfg;
	arf_klt_pre_cfg_t     kptpre_cfg;
	arf_mfpda_para_cfg_t  mfpda_para;
	arf_roi_window_cfg_t  roi_window_cfg;
	arf_kpt_limit_t       kpt_lmt_cfg[ARFEATURE_MAX_BLK_NUM];
	arf_cvdr_cfg_t        cvdr_cfg;
} cfg_tab_arf_t;

#endif // __CFG_TABLE_ARFEATURE_CS_H__
