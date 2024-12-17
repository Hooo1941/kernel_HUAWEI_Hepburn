/******************************************************************
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2018. All rights reserved.
 * File name  :
 * Description:
 *
 * Date         2022-06-27 16:28:10
 ********************************************************************/

#ifndef __CFG_TABLE_MC_CS_H__
#define __CFG_TABLE_MC_CS_H__

#include "mc_drv_priv.h"

#define MC_KPT_NUM_MAX          (1600)

typedef union mc_en_cfg           mc_en_cfg_t;
typedef union mc_imu_cfg          mc_imu_cfg_t;
typedef union mc_cfg              mc_ctrl_cfg_t;
typedef union mc_threshold_cfg    mc_inlier_thld_t;
typedef union mc_inlier_number    mc_inlier_number_t;
typedef union mc_kpt_size         mc_kpt_size_cfg_t;
typedef union mc_static_cfg1      mc_static_cfg1_t;
typedef union mc_static_cfg2      mc_static_cfg2_t;
typedef union mc_ref_prefetch_cfg mc_ref_prefetch_cfg_t;
typedef union mc_cur_prefetch_cfg mc_cur_prefetch_cfg_t;
typedef union mc_match_points     mc_matched_kpts_cfg_t;
typedef union mc_index_cfg        mc_index_pairs_t;    
typedef union mc_coordinate_cfg   mc_coordinate_cfg_t;


// ---------------------------------------------------------------------------
typedef struct _mc_static_cfg_t {
	mc_static_cfg1_t static_cfg1;
	mc_static_cfg2_t static_cfg2;
} mc_static_cfg_t;

typedef struct _mc_prefetch_t {
	mc_ref_prefetch_cfg_t ref_prefetch_cfg;
	mc_cur_prefetch_cfg_t cur_prefetch_cfg;
} mc_prefetch_t;

typedef struct _mc_index_pairs_cfg_t {
	mc_index_pairs_t  index_pairs[MC_KPT_NUM_MAX];
} mc_index_pairs_cfg_t;

typedef struct _mc_coord_pairs_cfg_t {
	mc_coordinate_cfg_t  cur_coordinate[MC_KPT_NUM_MAX];
	mc_coordinate_cfg_t  ref_coordinate[MC_KPT_NUM_MAX];
} mc_coord_pairs_cfg_t;

typedef struct _cfg_tab_mc_t {
	mc_en_cfg_t            mc_en_cfg;
	mc_imu_cfg_t           imu_cfg;
	mc_ctrl_cfg_t          ctrl_cfg;
	mc_inlier_thld_t       inlier_thld_cfg;
	mc_inlier_number_t     inlier_number_cfg;
	mc_kpt_size_cfg_t      kpt_size_cfg;
	mc_static_cfg_t        static_cfg;
	mc_matched_kpts_cfg_t  matched_kpts_cfg;
	mc_prefetch_t          prefech_cfg;
	mc_index_pairs_cfg_t   index_pairs_cfg;
	mc_coord_pairs_cfg_t   coord_pairs_cfg;

	unsigned int           mc_cascade_en; // used for ipp_path
	unsigned int           mc_match_points_addr; // address in cmdlst_buffer
	unsigned int           mc_index_addr; // address in cmdlst_buffer
} cfg_tab_mc_t;

#endif /* __CFG_TABLE_MC_CS_H__ */
