/******************************************************************
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2018. All rights reserved.
 * File name  :
 * Description: arfeature_drv_priv.h
 *
 * Date         2022-07-02
 ********************************************************************/

#ifndef __AR_FEATURE_DRV_PRIV_H__
#define __AR_FEATURE_DRV_PRIV_H__

union arfeature_top_cfg {
	struct {
		unsigned int    ar_feature_en : 1  ; /* [0] */
		unsigned int    mode          : 3  ; /* [3:1] */
		unsigned int    kltpre_out_en : 1  ; /* [4] */
		unsigned int    orient_en     : 1  ; /* [5] */
		unsigned int    layer         : 2  ; /* [7:6] */
		unsigned int    iter_num      : 1  ; /* [8] */
		unsigned int    first_detect  : 1  ; /* [9] */
		unsigned int    rsv_0         : 22  ; /* [31:10] */
	} bits;

	unsigned int    u32;
};

union arfeature_image_size {
	struct {
		unsigned int    width  : 11  ; /* [10:0] */
		unsigned int    rsv_1  : 5  ; /* [15:11] */
		unsigned int    height : 11  ; /* [26:16] */
		unsigned int    rsv_2  : 5  ; /* [31:27] */
	} bits;

	unsigned int    u32;
};

union arfeature_block_num_cfg {
	struct {
		unsigned int    blk_v_num : 4  ; /* [3:0] */
		unsigned int    blk_h_num : 4  ; /* [7:4] */
		unsigned int    rsv_3     : 24  ; /* [31:8] */
	} bits;

	unsigned int    u32;
};

union arfeature_block_size_cfg_inv {
	struct {
		unsigned int    blk_v_size_inv : 16  ; /* [15:0] */
		unsigned int    blk_h_size_inv : 16  ; /* [31:16] */
	} bits;

	unsigned int    u32;
};

union arfeature_detect_number_limit {
	struct {
		unsigned int    max_kpnum : 12  ; /* [11:0] */
		unsigned int    cur_kpnum : 12  ; /* [23:12] */
		unsigned int    rsv_4     : 8  ; /* [31:24] */
	} bits;

	unsigned int    u32;
};

union arfeature_sigma_coef {
	struct {
		unsigned int    sigma_des : 10  ; /* [9:0] */
		unsigned int    rsv_5     : 6  ; /* [15:10] */
		unsigned int    sigma_ori : 10  ; /* [25:16] */
		unsigned int    rsv_6     : 6  ; /* [31:26] */
	} bits;

	unsigned int    u32;
};

union arfeature_gauss_org {
	struct {
		unsigned int    gauss_org_0 : 8  ; /* [7:0] */
		unsigned int    gauss_org_1 : 8  ; /* [15:8] */
		unsigned int    gauss_org_2 : 8  ; /* [23:16] */
		unsigned int    gauss_org_3 : 8  ; /* [31:24] */
	} bits;

	unsigned int    u32;
};

union arfeature_gsblur_1st {
	struct {
		unsigned int    gauss_1st_0 : 8  ; /* [7:0] */
		unsigned int    gauss_1st_1 : 8  ; /* [15:8] */
		unsigned int    gauss_1st_2 : 8  ; /* [23:16] */
		unsigned int    rsv_7       : 8  ; /* [31:24] */
	} bits;

	unsigned int    u32;
};

union arfeature_gauss_2nd {
	struct {
		unsigned int    gauss_2nd_0 : 8  ; /* [7:0] */
		unsigned int    gauss_2nd_1 : 8  ; /* [15:8] */
		unsigned int    gauss_2nd_2 : 8  ; /* [23:16] */
		unsigned int    gauss_2nd_3 : 8  ; /* [31:24] */
	} bits;

	unsigned int    u32;
};

union arfeature_dog_edge_threshold {
	struct {
		unsigned int    edge_threshold : 16  ; /* [15:0] */
		unsigned int    dog_threshold  : 15  ; /* [30:16] */
		unsigned int    rsv_8          : 1  ; /* [31] */
	} bits;

	unsigned int    u32;
};

union arfeature_descriptor_threshold {
	struct {
		unsigned int    mag_threshold : 8  ; /* [7:0] */
		unsigned int    rsv_9         : 24  ; /* [31:8] */
	} bits;

	unsigned int    u32;
};

union arfeature_cvdr_rd_cfg {
	struct {
		unsigned int    vprd_pixel_format    : 5  ; /* [4:0] */
		unsigned int    vprd_pixel_expansion : 1  ; /* [5] */
		unsigned int    vprd_allocated_du    : 5  ; /* [10:6] */
		unsigned int    rsv_10               : 2  ; /* [12:11] */
		unsigned int    vprd_last_page       : 19  ; /* [31:13] */
	} bits;

	unsigned int    u32;
};

union arfeature_cvdr_rd_lwg {
	struct {
		unsigned int    vprd_line_size : 15  ; /* [14:0] */
		unsigned int    rsv_11         : 9  ; /* [23:15] */
		unsigned int    vprd_ihleft    : 6  ; /* [29:24] */
		unsigned int    rsv_12         : 2  ; /* [31:30] */
	} bits;

	unsigned int    u32;
};

union arfeature_cvdr_rd_fhg {
	struct {
		unsigned int    vprd_frame_size : 15  ; /* [14:0] */
		unsigned int    rsv_13          : 17  ; /* [31:15] */
	} bits;

	unsigned int    u32;
};

union arfeature_cvdr_rd_axi_fs {
	struct {
		unsigned int    rsv_14               : 2  ; /* [1:0] */
		unsigned int    vprd_axi_frame_start : 30  ; /* [31:2] */
	} bits;

	unsigned int    u32;
};

union arfeature_cvdr_rd_axi_line {
	struct {
		unsigned int    vprd_line_stride_0 : 14  ; /* [13:0] */
		unsigned int    rsv_15             : 4  ; /* [17:14] */
		unsigned int    vprd_line_wrap_0   : 14  ; /* [31:18] */
	} bits;

	unsigned int    u32;
};

union arfeature_cvdr_rd_if_cfg {
	struct {
		unsigned int    rsv_16                                 : 16  ; /* [15:0] */
		unsigned int    vp_rd_stop_enable_du_threshold_reached : 1  ; /* [16] */
		unsigned int    vp_rd_stop_enable_flux_ctrl            : 1  ; /* [17] */
		unsigned int    vp_rd_stop_enable_pressure             : 1  ; /* [18] */
		unsigned int    rsv_17                                 : 5  ; /* [23:19] */
		unsigned int    vp_rd_stop_ok                          : 1  ; /* [24] */
		unsigned int    vp_rd_stop                             : 1  ; /* [25] */
		unsigned int    rsv_18                                 : 5  ; /* [30:26] */
		unsigned int    vprd_prefetch_bypass                   : 1  ; /* [31] */
	} bits;

	unsigned int    u32;
};

union arfeature_pre_cvdr_rd_fhg {
	struct {
		unsigned int    pre_vprd_frame_size : 15  ; /* [14:0] */
		unsigned int    rsv_19              : 17  ; /* [31:15] */
	} bits;

	unsigned int    u32;
};

union arfeature_cvdr_wr_cfg {
	struct {
		unsigned int    vpwr_pixel_format    : 5  ; /* [4:0] */
		unsigned int    vpwr_pixel_expansion : 1  ; /* [5] */
		unsigned int    rsv_20               : 7  ; /* [12:6] */
		unsigned int    vpwr_last_page       : 19  ; /* [31:13] */
	} bits;

	unsigned int    u32;
};

union arfeature_cvdr_wr_axi_fs {
	struct {
		unsigned int    rsv_21                   : 2  ; /* [1:0] */
		unsigned int    vpwr_address_frame_start : 30  ; /* [31:2] */
	} bits;

	unsigned int    u32;
};

union arfeature_cvdr_wr_axi_line {
	struct {
		unsigned int    vpwr_line_stride      : 14  ; /* [13:0] */
		unsigned int    vpwr_line_start_wstrb : 4  ; /* [17:14] */
		unsigned int    vpwr_line_wrap        : 14  ; /* [31:18] */
	} bits;

	unsigned int    u32;
};

union arfeature_cvdr_wr_if_cfg {
	struct {
		unsigned int    rsv_22                                 : 16  ; /* [15:0] */
		unsigned int    vp_wr_stop_enable_du_threshold_reached : 1  ; /* [16] */
		unsigned int    vp_wr_stop_enable_flux_ctrl            : 1  ; /* [17] */
		unsigned int    vp_wr_stop_enable_pressure             : 1  ; /* [18] */
		unsigned int    rsv_23                                 : 5  ; /* [23:19] */
		unsigned int    vp_wr_stop_ok                          : 1  ; /* [24] */
		unsigned int    vp_wr_stop                             : 1  ; /* [25] */
		unsigned int    rsv_24                                 : 5  ; /* [30:26] */
		unsigned int    vpwr_prefetch_bypass                   : 1  ; /* [31] */
	} bits;

	unsigned int    u32;
};

union arfeature_pre_cvdr_wr_axi_fs {
	struct {
		unsigned int    rsv_25                       : 2  ; /* [1:0] */
		unsigned int    pre_vpwr_address_frame_start : 30  ; /* [31:2] */
	} bits;

	unsigned int    u32;
};

union arfeature_klt_pre_block_size_cfg_inv {
	struct {
		unsigned int    kltpre_blk_v_size_inv : 16  ; /* [15:0] */
		unsigned int    kltpre_blk_h_size_inv : 16  ; /* [31:16] */
	} bits;

	unsigned int    u32;
};

union arfeature_klt_pre_block_cfg {
	struct {
		unsigned int    kltpre_blk_v_num     : 4  ; /* [3:0] */
		unsigned int    kltpre_blk_h_num     : 4  ; /* [7:4] */
		unsigned int    kltpre_blk_max_kpnum : 3  ; /* [10:8] */
		unsigned int    rsv_26               : 21  ; /* [31:11] */
	} bits;

	unsigned int    u32;
};

union arfeature_mfpda_cfg {
	struct {
		unsigned int    mfpda_kpnum_th   : 12  ; /* [11:0] */
		unsigned int    rsv_27           : 4  ; /* [15:12] */
		unsigned int    mfpda_score_step : 13  ; /* [28:16] */
		unsigned int    rsv_28           : 3  ; /* [31:29] */
	} bits;

	unsigned int    u32;
};

union arfeature_block_mfpda_cfg {
	struct {
		unsigned int    mfpda_grid_kpnum_th   : 8  ; /* [7:0] */
		unsigned int    rsv_29                : 8  ; /* [15:8] */
		unsigned int    mfpda_grid_score_step : 13  ; /* [28:16] */
		unsigned int    rsv_30                : 3  ; /* [31:29] */
	} bits;

	unsigned int    u32;
};

union arfeature_roi_window_start_cfg {
	struct {
		unsigned int    roi_start_x : 10  ; /* [9:0] */
		unsigned int    rsv_31      : 6  ; /* [15:10] */
		unsigned int    roi_start_y : 10  ; /* [25:16] */
		unsigned int    rsv_32      : 6  ; /* [31:26] */
	} bits;

	unsigned int    u32;
};

union arfeature_roi_window_end_cfg {
	struct {
		unsigned int    roi_end_x : 10  ; /* [9:0] */
		unsigned int    rsv_33    : 6  ; /* [15:10] */
		unsigned int    roi_end_y : 10  ; /* [25:16] */
		unsigned int    rsv_34    : 6  ; /* [31:26] */
	} bits;

	unsigned int    u32;
};

union arfeature_roi_window_mode_cfg {
	struct {
		unsigned int    roi_win_mode : 1  ; /* [0] */
		unsigned int    rsv_35       : 31  ; /* [31:1] */
	} bits;

	unsigned int    u32;
};

union arfeature_kpt_limit {
	struct {
		unsigned int    score_th       : 15  ; /* [14:0] */
		unsigned int    rsv_36         : 1  ; /* [15] */
		unsigned int    grid_max_kpnum : 8  ; /* [23:16] */
		unsigned int    rsv_37         : 8  ; /* [31:24] */
	} bits;

	unsigned int    u32;
};

union arfeature_kpt_number {
	struct {
		unsigned int    kpt_num : 8  ; /* [7:0] */
		unsigned int    rsv_38  : 24  ; /* [31:8] */
	} bits;

	unsigned int    u32;
};

union arfeature_gridstat_1 {
	struct {
		unsigned int    min_score : 15  ; /* [14:0] */
		unsigned int    rsv_39    : 1  ; /* [15] */
		unsigned int    kpt_count : 8  ; /* [23:16] */
		unsigned int    rsv_40    : 8  ; /* [31:24] */
	} bits;

	unsigned int    u32;
};

union arfeature_gridstat_2 {
	struct {
		unsigned int    sum_score : 23  ; /* [22:0] */
		unsigned int    rsv_41    : 9  ; /* [31:23] */
	} bits;

	unsigned int    u32;
};

union arfeature_total_kpt_num {
	struct {
		unsigned int    total_kpt_num : 12  ; /* [11:0] */
		unsigned int    rsv_42        : 20  ; /* [31:12] */
	} bits;

	unsigned int    u32;
};

union arfeature_total_kltpre_kpt_num {
	struct {
		unsigned int    kltpre_total_kpt_num : 8  ; /* [7:0] */
		unsigned int    rsv_43               : 24  ; /* [31:8] */
	} bits;

	unsigned int    u32;
};

union arfeature_mfpda_update_cfg {
	struct {
		unsigned int    nxt_dog_threshold : 15  ; /* [14:0] */
		unsigned int    rsv_44            : 17  ; /* [31:15] */
	} bits;

	unsigned int    u32;
};

union arfeature_debug_0 {
	struct {
		unsigned int    debug0 : 32  ; /* [31:0] */
	} bits;

	unsigned int    u32;
};

union arfeature_debug_1 {
	struct {
		unsigned int    debug1 : 32  ; /* [31:0] */
	} bits;

	unsigned int    u32;

};

union arfeature_debug_2 {
	struct {
		unsigned int    debug2 : 32  ; /* [31:0] */
	} bits;

	unsigned int    u32;
};

union arfeature_debug_3 {
	struct {
		unsigned int    debug3 : 32  ; /* [31:0] */
	} bits;

	unsigned int    u32;
};

// ==============================================================================
/* define the global struct */
typedef struct {
	union arfeature_top_cfg                    top_cfg                    ; /* 0 */
	union arfeature_image_size                 image_size                 ; /* 4 */
	union arfeature_block_num_cfg              block_num_cfg              ; /* 8 */
	union arfeature_block_size_cfg_inv         block_size_cfg_inv         ; /* c */
	union arfeature_detect_number_limit        detect_number_limit        ; /* 10 */
	union arfeature_sigma_coef                 sigma_coef                 ; /* 14 */
	union arfeature_gauss_org                  gauss_org                  ; /* 18 */
	union arfeature_gsblur_1st                 gsblur_1st                 ; /* 1c */
	union arfeature_gauss_2nd                  gauss_2nd                  ; /* 20 */
	union arfeature_dog_edge_threshold         dog_edge_threshold         ; /* 24 */
	union arfeature_descriptor_threshold       descriptor_threshold       ; /* 28 */
	union arfeature_cvdr_rd_cfg                cvdr_rd_cfg                ; /* 2c */
	union arfeature_cvdr_rd_lwg                cvdr_rd_lwg                ; /* 30 */
	union arfeature_cvdr_rd_fhg                cvdr_rd_fhg                ; /* 34 */
	union arfeature_cvdr_rd_axi_fs             cvdr_rd_axi_fs             ; /* 38 */
	union arfeature_cvdr_rd_axi_line           cvdr_rd_axi_line           ; /* 3c */
	union arfeature_cvdr_rd_if_cfg             cvdr_rd_if_cfg             ; /* 40 */
	union arfeature_pre_cvdr_rd_fhg            pre_cvdr_rd_fhg            ; /* 44 */
	union arfeature_cvdr_wr_cfg                cvdr_wr_cfg                ; /* 48 */
	union arfeature_cvdr_wr_axi_fs             cvdr_wr_axi_fs             ; /* 4c */
	union arfeature_cvdr_wr_axi_line           cvdr_wr_axi_line           ; /* 50 */
	union arfeature_cvdr_wr_if_cfg             cvdr_wr_if_cfg             ; /* 54 */
	union arfeature_pre_cvdr_wr_axi_fs         pre_cvdr_wr_axi_fs         ; /* 58 */
	union arfeature_klt_pre_block_size_cfg_inv klt_pre_block_size_cfg_inv ; /* 5c */
	union arfeature_klt_pre_block_cfg          klt_pre_block_cfg          ; /* 60 */
	union arfeature_mfpda_cfg                  mfpda_cfg[6]               ; /* 64 */
	union arfeature_block_mfpda_cfg            block_mfpda_cfg[6]         ; /* 7c */
	union arfeature_roi_window_start_cfg       roi_window_start_cfg[8]    ; /* 94 */
	union arfeature_roi_window_end_cfg         roi_window_end_cfg[8]      ; /* b4 */
	union arfeature_roi_window_mode_cfg        roi_window_mode_cfg        ; /* d4 */
	union arfeature_kpt_limit                  kpt_limit[256]             ; /* 100 */
	union arfeature_kpt_number                 kpt_number[256]            ; /* 500 */
	union arfeature_gridstat_1                 gridstat_1[256]            ; /* 900 */
	union arfeature_gridstat_2                 gridstat_2[256]            ; /* d00 */
	union arfeature_total_kpt_num              total_kpt_num              ; /* 1100 */
	union arfeature_total_kltpre_kpt_num       total_kltpre_kpt_num       ; /* 1104 */
	union arfeature_mfpda_update_cfg           mfpda_update_cfg           ; /* 1108 */
	union arfeature_debug_0                    debug_0                    ; /* 1200 */
	union arfeature_debug_1                    debug_1                    ; /* 1204 */
	union arfeature_debug_2                    debug_2                    ; /* 1208 */
	union arfeature_debug_3                    debug_3                    ; /* 120c */
} s_ar_feature_regs_type;

#endif // __AR_FEATURE_DRV_PRIV_H__
