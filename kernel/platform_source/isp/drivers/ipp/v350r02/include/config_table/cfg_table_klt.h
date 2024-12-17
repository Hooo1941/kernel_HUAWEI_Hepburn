/******************************************************************
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2018. All rights reserved.
 * File name  :
 * Description:
 *
 * Date         2022-06-29
 ********************************************************************/

#ifndef __CFG_TABLE_KLT_CS_H__
#define __CFG_TABLE_KLT_CS_H__

#include "klt_drv_priv.h"

#define CFG_TAB_KLT_MAX_KP_NUM   (500)
#define KLT_MAX_FWD_PYR_NUM      (5)
#define KLT_MIN_FWD_PYR_NUM      (1)
#define KLT_MAX_BWD_PYR_NUM      (1)

typedef union klt_ds_config         klt_ds_cfg_t;
typedef union klt_klt_start         klt_start_cfg_t;
typedef union klt_fwd_config        klt_fwd_search_cfg_t;
typedef union klt_bwd_config        klt_bwd_search_cfg_t;
typedef union klt_iter_config_1     klt_iter_cfg1_t;
typedef union klt_iter_config_2     klt_iter_cfg2_t;
typedef union klt_eval_param        klt_eval_param_cfg_t;
typedef union klt_prefetch_ctrl     klt_prefetch_ctrl_t;
typedef union klt_rd_prev_base_addr klt_rd_prev_base_addr_t;
typedef union klt_rd_prev_stride    klt_rd_prev_stride_t;
typedef union klt_rd_next_base_addr klt_rd_next_base_addr_t;
typedef union klt_rd_next_stride    klt_rd_next_stride_t;
typedef union klt_kp_prev_buff_x    klt_kp_prev_buff_x_t;
typedef union klt_kp_prev_buff_y    klt_kp_prev_buff_y_t;

typedef struct _klt_iter_cfg_cfg_t {
	klt_iter_cfg1_t iter_cfg1;
	klt_iter_cfg2_t iter_cfg2;
} klt_iter_cfg_cfg_t;

typedef struct _klt_prev_and_next_rd_cfg_t {
	klt_rd_prev_base_addr_t rd_prev_base_addr[KLT_MAX_FWD_PYR_NUM];
	klt_rd_prev_stride_t    rd_prev_line_stride[KLT_MAX_FWD_PYR_NUM];
	klt_rd_next_base_addr_t rd_next_base_addr[KLT_MAX_FWD_PYR_NUM];
	klt_rd_next_stride_t    rd_next_line_stride[KLT_MAX_FWD_PYR_NUM];
} klt_prev_and_next_rd_cfg_t;

typedef union {
	struct {
		unsigned int    kp_prev_x_y : 18  ; /* [17:0] */
		unsigned int    rsv_15      : 14  ; /* [31:18] */
	} bits;

	unsigned int    u32;
} klt_kp_prev_buff_xy_t;

typedef struct _klt_prev_kp_coord_cfg_t {
	// cfg order: kp_prev_x/kp_prev_y/kp_prev_x/kp_prev_y ....
	klt_kp_prev_buff_xy_t kp_prev_xy[CFG_TAB_KLT_MAX_KP_NUM * 2];
} klt_prev_kp_coord_cfg_t;

typedef struct _klt_klt_cfg_t {
	klt_start_cfg_t            klt_start;
	klt_fwd_search_cfg_t       fwd_search;
	klt_bwd_search_cfg_t       bwd_search;
	klt_iter_cfg_cfg_t         iter_param;
	klt_eval_param_cfg_t       eval_param;
	klt_prev_and_next_rd_cfg_t rd_cfg;
	klt_prev_kp_coord_cfg_t    prev_kp_coord;
	klt_prefetch_ctrl_t        prefetch_ctrl;
} klt_klt_cfg_t;

typedef struct _cfg_tab_klt_t {
	klt_ds_cfg_t  klt_ds_cfg;
	klt_klt_cfg_t klt_klt_cfg;

	unsigned int  klt_cascade_en;
} cfg_tab_klt_t;

#endif /* __CFG_TABLE_KLT_CS_H__ */
