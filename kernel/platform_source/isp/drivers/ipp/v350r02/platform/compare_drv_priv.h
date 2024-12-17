/******************************************************************
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2018. All rights reserved.
 * File name  :
 * Description:
 *
 * Date         2020-04-17 16:28:10
 ********************************************************************/

#ifndef __SLAM_COMPARE_DRV_PRIV_H__
#define __SLAM_COMPARE_DRV_PRIV_H__

union compare_cfg {
	struct {
		unsigned int    compare_en : 1  ; /* [0] */
		unsigned int    rsv_0      : 31  ; /* [31:1] */
	} bits;

	unsigned int    u32;
};

union compare_block_cfg {
	struct {
		unsigned int    blk_v_num : 4  ; /* [3:0] */
		unsigned int    rsv_1     : 4  ; /* [7:4] */
		unsigned int    blk_h_num : 4  ; /* [11:8] */
		unsigned int    rsv_2     : 4  ; /* [15:12] */
		unsigned int    blk_num   : 8  ; /* [23:16] */
		unsigned int    rsv_3     : 8  ; /* [31:24] */
	} bits;

	unsigned int    u32;
};

union compare_search_cfg {
	struct {
		unsigned int    v_radius      : 2  ; /* [1:0] */
		unsigned int    h_radius      : 2  ; /* [3:2] */
		unsigned int    dis_ratio     : 13  ; /* [16:4] */
		unsigned int    dis_threshold : 15  ; /* [31:17] */
	} bits;

	unsigned int    u32;
};

union compare_stat_cfg {
	struct {
		unsigned int    bin_num    : 6  ; /* [5:0] */
		unsigned int    rsv_4      : 2  ; /* [7:6] */
		unsigned int    bin_factor : 8  ; /* [15:8] */
		unsigned int    max3_ratio : 5  ; /* [20:16] */
		unsigned int    rsv_5      : 3  ; /* [23:21] */
		unsigned int    stat_en    : 1  ; /* [24] */
		unsigned int    rsv_6      : 7  ; /* [31:25] */
	} bits;

	unsigned int    u32;
};

union compare_prefetch_cfg {
	struct {
		unsigned int    first_32k_page  : 19  ; /* [18:0] */
		unsigned int    rsv_7           : 1  ; /* [19] */
		unsigned int    prefetch_enable : 1  ; /* [20] */
		unsigned int    rsv_8           : 11  ; /* [31:21] */
	} bits;

	unsigned int    u32;
};

union compare_offset_cfg {
	struct {
		unsigned int    cenv_offset : 4  ; /* [3:0] */
		unsigned int    cenh_offset : 4  ; /* [7:4] */
		unsigned int    rsv_9       : 24  ; /* [31:8] */
	} bits;

	unsigned int    u32;
};

union compare_total_kpt_num {
	struct {
		unsigned int    total_kpt_num : 12  ; /* [11:0] */
		unsigned int    rsv_10        : 20  ; /* [31:12] */
	} bits;

	unsigned int    u32;
};

union compare_ref_kpt_number {
	struct {
		unsigned int    ref_kpt_num : 8  ; /* [7:0] */
		unsigned int    rsv_11      : 24  ; /* [31:8] */
	} bits;

	unsigned int    u32;
};

union compare_cur_kpt_number {
	struct {
		unsigned int    cur_kpt_num : 8  ; /* [7:0] */
		unsigned int    rsv_12      : 24  ; /* [31:8] */
	} bits;

	unsigned int    u32;
};

union compare_match_points {
	struct {
		unsigned int    matched_kpts : 11  ; /* [10:0] */
		unsigned int    rsv_13       : 21  ; /* [31:11] */
	} bits;

	unsigned int    u32;
};

union compare_index {
	struct {
		unsigned int    cur_index : 12  ; /* [11:0] */
		unsigned int    ref_index : 12  ; /* [23:12] */
		unsigned int    rsv_14    : 8  ; /* [31:24] */
	} bits;

	unsigned int    u32;
};

union compare_distance {
	struct {
		unsigned int    distance : 15  ; /* [14:0] */
		unsigned int    rsv_15   : 17  ; /* [31:15] */
	} bits;

	unsigned int    u32;
};

union compare_debug_0 {
	struct {
		unsigned int    debug0 : 32  ; /* [31:0] */
	} bits;

	unsigned int    u32;
};

union compare_debug_1 {
	struct {
		unsigned int    debug1 : 32  ; /* [31:0] */
	} bits;

	unsigned int    u32;
};

union compare_ec_0 {
	struct {
		unsigned int    ec_0 : 32  ; /* [31:0] */
	} bits;

	unsigned int    u32;
};

union compare_ec_1 {
	struct {
		unsigned int    ec_1 : 32  ; /* [31:0] */
	} bits;

	unsigned int    u32;
};

/* define the global struct */
typedef struct {
	union compare_cfg            compare_cfg         ; /* 0 */
	union compare_block_cfg      block_cfg           ; /* 8 */
	union compare_search_cfg     search_cfg          ; /* c */
	union compare_stat_cfg       stat_cfg            ; /* 10 */
	union compare_prefetch_cfg   prefetch_cfg        ; /* 14 */
	union compare_offset_cfg     offset_cfg          ; /* 18 */
	union compare_total_kpt_num  total_kpt_num       ; /* 1c */
	union compare_ref_kpt_number ref_kpt_number[256] ; /* 20 */
	union compare_cur_kpt_number cur_kpt_number[256] ; /* 420 */
	union compare_match_points   match_points        ; /* 8fc */
	union compare_index          index[1600]         ; /* 900 */
	union compare_distance       distance[1600]      ; /* 2200 */
	union compare_debug_0        debug_0             ; /* 3b00 */
	union compare_debug_1        debug_1             ; /* 3b04 */
	union compare_ec_0           ec_0                ; /* 3b08 */
	union compare_ec_1           ec_1                ; /* 3b0c */
} s_slam_compare_regs_type;

#endif // __SLAM_COMPARE_DRV_PRIV_H__
