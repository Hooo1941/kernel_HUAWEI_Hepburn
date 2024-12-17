/******************************************************************
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2018. All rights reserved.
 * File name  :
 * Description:
 *
 * Date         2020-04-17 16:28:10
 ********************************************************************/

#ifndef __MC_DRV_PRIV_H__
#define __MC_DRV_PRIV_H__

union mc_en_cfg {
	struct {
		unsigned int    mc_en : 1  ; /* [0] */
		unsigned int    rsv_0 : 31  ; /* [31:1] */
	} bits;

	unsigned int    u32;
};

union mc_imu_cfg {
	struct {
		unsigned int    imu_en     : 1  ; /* [0] */
		unsigned int    imu_static : 1  ; /* [1] */
		unsigned int    rsv_1      : 30  ; /* [31:2] */
	} bits;

	unsigned int    u32;
};

union mc_cfg {
	struct {
		unsigned int    cfg_mode        : 1  ; /* [0] */
		unsigned int    rsv_2           : 1  ; /* [1] */
		unsigned int    push_inliers_en : 1  ; /* [2] */
		unsigned int    stat_en         : 1  ; /* [3] */
		unsigned int    max_iterations  : 12  ; /* [15:4] */
		unsigned int    svd_iterations  : 5  ; /* [20:16] */
		unsigned int    rsv_3           : 11  ; /* [31:21] */
	} bits;

	unsigned int    u32;
};

union mc_threshold_cfg {
	struct {
		unsigned int    inlier_th : 20  ; /* [19:0] */
		unsigned int    rsv_4     : 12  ; /* [31:20] */
	} bits;

	unsigned int    u32;
};

union mc_inlier_number {
	struct {
		unsigned int    inlier_num_th : 11  ; /* [10:0] */
		unsigned int    rsv_5         : 21  ; /* [31:11] */
	} bits;

	unsigned int    u32;
};

union mc_kpt_size {
	struct {
		unsigned int    kpt_size : 10  ; /* [9:0] */
		unsigned int    rsv_6    : 22  ; /* [31:10] */
	} bits;

	unsigned int    u32;
};

union mc_static_cfg1 {
	struct {
		unsigned int    static_ratio : 6  ; /* [5:0] */
		unsigned int    rsv_7        : 2  ; /* [7:6] */
		unsigned int    inv_ds_ratio : 12  ; /* [19:8] */
		unsigned int    rsv_8        : 12  ; /* [31:20] */
	} bits;

	unsigned int    u32;
};

union mc_static_cfg2 {
	struct {
		unsigned int    matched_num_th : 11  ; /* [10:0] */
		unsigned int    rsv_9          : 21  ; /* [31:11] */
	} bits;

	unsigned int    u32;
};

union mc_ref_prefetch_cfg {
	struct {
		unsigned int    ref_first_32k_page  : 19  ; /* [18:0] */
		unsigned int    rsv_10              : 1  ; /* [19] */
		unsigned int    ref_prefetch_enable : 1  ; /* [20] */
		unsigned int    rsv_11              : 11  ; /* [31:21] */
	} bits;

	unsigned int    u32;
};

union mc_cur_prefetch_cfg {
	struct {
		unsigned int    cur_first_32k_page  : 19  ; /* [18:0] */
		unsigned int    rsv_12              : 1  ; /* [19] */
		unsigned int    cur_prefetch_enable : 1  ; /* [20] */
		unsigned int    rsv_13              : 11  ; /* [31:21] */
	} bits;

	unsigned int    u32;
};

union mc_match_points {
	struct {
		unsigned int    matched_kpts : 11  ; /* [10:0] */
		unsigned int    rsv_14       : 21  ; /* [31:11] */
	} bits;

	unsigned int    u32;
};

union mc_index_cfg {
	struct {
		unsigned int    cfg_cur_index : 12  ; /* [11:0] */
		unsigned int    cfg_ref_index : 12  ; /* [23:12] */
		unsigned int    rsv_15        : 8  ; /* [31:24] */
	} bits;

	unsigned int    u32;
};

union mc_index_pairs {
	struct {
		unsigned int    cur_index : 12  ; /* [11:0] */
		unsigned int    ref_index : 12  ; /* [23:12] */
		unsigned int    rsv_16    : 8  ; /* [31:24] */
	} bits;

	unsigned int    u32;
};

union mc_coordinate_cfg {
	struct {
		unsigned int    cfg_coordinate_x : 16  ; /* [15:0] */
		unsigned int    cfg_coordinate_y : 16  ; /* [31:16] */
	} bits;

	unsigned int    u32;
};

union mc_coordinate_pairs {
	struct {
		unsigned int    coordinate_x : 16  ; /* [15:0] */
		unsigned int    coordinate_y : 16  ; /* [31:16] */
	} bits;

	unsigned int    u32;
};

union mc_debug_0 {
	struct {
		unsigned int    debug_0 : 32  ; /* [31:0] */
	} bits;

	unsigned int    u32;
};

union mc_debug_1 {
	struct {
		unsigned int    debug_1 : 32  ; /* [31:0] */
	} bits;

	unsigned int    u32;
};

union mc_ec_0 {
	struct {
		unsigned int    ec_0 : 32  ; /* [31:0] */
	} bits;

	unsigned int    u32;
};

union mc_ec_1 {
	struct {
		unsigned int    ec_1 : 32  ; /* [31:0] */
	} bits;

	unsigned int    u32;
};

// ==============================================================================
/* define the global struct */
typedef struct {
	union mc_en_cfg           en_cfg           ; /* 0 */
	union mc_imu_cfg          imu_cfg          ; /* 18 */
	union mc_cfg              mc_cfg           ; /* 1c */
	union mc_threshold_cfg    threshold_cfg    ; /* 20 */
	union mc_inlier_number    inlier_number    ; /* 24 */
	union mc_kpt_size         kpt_size         ; /* 28 */
	union mc_static_cfg1      static_cfg1      ; /* 2c */
	union mc_static_cfg2      static_cfg2      ; /* 30 */
	union mc_ref_prefetch_cfg ref_prefetch_cfg ; /* 34 */
	union mc_cur_prefetch_cfg cur_prefetch_cfg ; /* 38 */
	union mc_match_points     match_points     ; /* 3c */
	union mc_index_cfg        index_cfg[1600]  ; /* 40 */
	union mc_index_pairs      index_pairs      ; /* 1a00 */
	union mc_coordinate_cfg   coordinate_cfg   ; /* 1a80 */
	union mc_coordinate_pairs coordinate_pairs ; /* 1b00 */
	union mc_debug_0          debug_0          ; /* 1c00 */
	union mc_debug_1          debug_1          ; /* 1c04 */
	union mc_ec_0             ec_0             ; /* 1c08 */
	union mc_ec_1             ec_1             ; /* 1c0c */
} s_mc_regs_type;

#endif // __MC_DRV_PRIV_H__
