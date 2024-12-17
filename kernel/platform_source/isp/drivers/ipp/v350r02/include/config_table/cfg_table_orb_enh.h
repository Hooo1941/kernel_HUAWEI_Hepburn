/******************************************************************
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2018. All rights reserved.
 * File name  :
 * Description:
 *
 * Date         2022-06-25 16:28:10
 ********************************************************************/

#ifndef __CONFIG_TABLE_ORB_ENH_H__
#define __CONFIG_TABLE_ORB_ENH_H__

#include "orb_enh_drv_priv.h"

typedef union enh_enable_cfg     enh_enable_cfg_t;
typedef union enh_blk_num        enh_blk_num_t;
typedef union enh_blk_size       enh_blk_size_t;
typedef union enh_inv_blk_size_x enh_inv_blk_size_x_t;
typedef union enh_inv_blk_size_y enh_inv_blk_size_y_t;
typedef union enh_extend_cfg     enh_extend_cfg_t;
typedef union enh_hist_climit    enh_hist_climit_t;
typedef union enh_lut_scale      enh_lut_scale_t;

typedef struct _enh_cal_hist_cfg_t {
	enh_blk_num_t     blk_num;
	enh_blk_size_t    blk_size;
	enh_extend_cfg_t  extend_cfg;
	enh_hist_climit_t climit_cfg;
} enh_cal_hist_cfg_t;

typedef struct _enh_adjust_hist_cfg_t {
	enh_lut_scale_t lutscale_cfg;
} enh_adjust_hist_cfg_t;

typedef struct _enh_adjust_image_cfg_t {
	enh_inv_blk_size_x_t inv_blk_size_x;
	enh_inv_blk_size_y_t inv_blk_size_y;
} enh_adjust_image_cfg_t;

typedef struct _cfg_tab_orb_enh_t {
	enh_enable_cfg_t       enable_cfg;
	enh_adjust_hist_cfg_t  adjust_hist_cfg;
	enh_cal_hist_cfg_t     cal_hist_cfg;
	enh_adjust_image_cfg_t adjust_image_cfg;
} cfg_tab_orb_enh_t;

#endif /* __CONFIG_TABLE_ORB_ENH_H__ */
/* ********************************* END ********************************* */
