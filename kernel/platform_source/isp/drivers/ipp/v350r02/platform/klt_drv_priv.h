/******************************************************************
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2018. All rights reserved.
 * File name    segment_mc.h
 * Description:
 *
 * Date         2021-11-27 16:28:10
 ********************************************************************/

#ifndef __KLT_DRV_PRIV_H__
#define __KLT_DRV_PRIV_H__

union klt_ds_config {
	struct {
		unsigned int    ds_width    : 10  ; /* [9:0] */
		unsigned int    ds_height   : 10  ; /* [19:10] */
		unsigned int    ds_nb_layer : 3  ; /* [22:20] */
		unsigned int    rsv_0       : 9  ; /* [31:23] */
	} bits;

	unsigned int    u32;
} ;

union klt_klt_start {
	struct {
		unsigned int    start_klt    : 1  ; /* [0] */
		unsigned int    kp_numbers   : 9  ; /* [9:1] */
		unsigned int    frame_width  : 10  ; /* [19:10] */
		unsigned int    frame_height : 10  ; /* [29:20] */
		unsigned int    rsv_1        : 2  ; /* [31:30] */
	} bits;

	unsigned int    u32;
} ;

union klt_fwd_config {
	struct {
		unsigned int    fwd_pyr_num       : 3  ; /* [2:0] */
		unsigned int    fwd_patch_size    : 5  ; /* [7:3] */
		unsigned int    fwd_iter_accuracy : 10  ; /* [17:8] */
		unsigned int    rsv_2             : 14  ; /* [31:18] */
	} bits;

	unsigned int    u32;
} ;

union klt_bwd_config {
	struct {
		unsigned int    bwd_pyr_num       : 1  ; /* [0] */
		unsigned int    rsv_3             : 2  ; /* [2:1] */
		unsigned int    bwd_patch_size    : 5  ; /* [7:3] */
		unsigned int    bwd_iter_accuracy : 10  ; /* [17:8] */
		unsigned int    rsv_4             : 14  ; /* [31:18] */
	} bits;

	unsigned int    u32;
} ;

union klt_iter_config_1 {
	struct {
		unsigned int    fwd_iter_guaranteed_nr_4 : 5  ; /* [4:0] */
		unsigned int    fwd_iter_guaranteed_nr_3 : 5  ; /* [9:5] */
		unsigned int    fwd_iter_guaranteed_nr_2 : 5  ; /* [14:10] */
		unsigned int    fwd_iter_guaranteed_nr_1 : 5  ; /* [19:15] */
		unsigned int    fwd_iter_guaranteed_nr_0 : 5  ; /* [24:20] */
		unsigned int    bwd_iter_guaranteed_nr_0 : 4  ; /* [28:25] */
		unsigned int    rsv_5                    : 3  ; /* [31:29] */
	} bits;

	unsigned int    u32;
} ;

union klt_iter_config_2 {
	struct {
		unsigned int    fwd_iter_max_nr_4 : 5  ; /* [4:0] */
		unsigned int    fwd_iter_max_nr_3 : 5  ; /* [9:5] */
		unsigned int    fwd_iter_max_nr_2 : 5  ; /* [14:10] */
		unsigned int    fwd_iter_max_nr_1 : 5  ; /* [19:15] */
		unsigned int    fwd_iter_max_nr_0 : 5  ; /* [24:20] */
		unsigned int    bwd_iter_max_nr_0 : 4  ; /* [28:25] */
		unsigned int    rsv_6             : 3  ; /* [31:29] */
	} bits;

	unsigned int    u32;
} ;

union klt_eval_param {
	struct {
		unsigned int    min_eig_thresh : 9  ; /* [8:0] */
		unsigned int    rsv_7          : 7  ; /* [15:9] */
		unsigned int    motion_epsilon : 9  ; /* [24:16] */
		unsigned int    rsv_8          : 7  ; /* [31:25] */
	} bits;

	unsigned int    u32;
} ;

union klt_prefetch_ctrl {
	struct {
		unsigned int    prefetch_en    : 1  ; /* [0] */
		unsigned int    prefetch_delay : 11  ; /* [11:1] */
		unsigned int    rsv_9          : 20  ; /* [31:12] */
	} bits;

	unsigned int    u32;
} ;

union klt_rd_prev_base_addr {
	struct {
		unsigned int    rd_prev_base_addr : 27  ; /* [26:0] */
		unsigned int    rsv_10            : 5  ; /* [31:27] */
	} bits;

	unsigned int    u32;
} ;

union klt_rd_prev_stride {
	struct {
		unsigned int    rd_prev_line_stride     : 10  ; /* [9:0] */
		unsigned int    rd_prev_line_stridectrl : 1  ; /* [10] */
		unsigned int    rsv_11                  : 21  ; /* [31:11] */
	} bits;

	unsigned int    u32;
} ;

union klt_rd_next_base_addr {
	struct {
		unsigned int    rd_next_base_addr : 27  ; /* [26:0] */
		unsigned int    rsv_12            : 5  ; /* [31:27] */
	} bits;

	unsigned int    u32;
} ;

union klt_rd_next_stride {
	struct {
		unsigned int    rd_next_line_stride     : 10  ; /* [9:0] */
		unsigned int    rd_next_line_stridectrl : 1  ; /* [10] */
		unsigned int    rsv_13                  : 21  ; /* [31:11] */
	} bits;

	unsigned int    u32;
} ;

union klt_kp_prev_buff_x {
	struct {
		unsigned int    kp_prev_x : 18  ; /* [17:0] */
		unsigned int    rsv_14    : 14  ; /* [31:18] */
	} bits;

	unsigned int    u32;
} ;

union klt_kp_prev_buff_y {
	struct {
		unsigned int    kp_prev_y : 18  ; /* [17:0] */
		unsigned int    rsv_15    : 14  ; /* [31:18] */
	} bits;

	unsigned int    u32;
} ;

union klt_kp_next_buff_x {
	struct {
		unsigned int    kp_next_x : 18  ; /* [17:0] */
		unsigned int    rsv_16    : 14  ; /* [31:18] */
	} bits;

	unsigned int    u32;
} ;

union klt_kp_next_buff_y {
	struct {
		unsigned int    kp_next_y : 18  ; /* [17:0] */
		unsigned int    status    : 1  ; /* [18] */
		unsigned int    conf      : 8  ; /* [26:19] */
		unsigned int    rsv_17    : 5  ; /* [31:27] */
	} bits;

	unsigned int    u32;
} ;

// ==============================================================================
/* define the global struct */
typedef struct {
	union klt_ds_config         ds_config            ; /* 0 */
	union klt_klt_start         klt_start            ; /* 4 */
	union klt_fwd_config        fwd_config           ; /* 8 */
	union klt_bwd_config        bwd_config           ; /* c */
	union klt_iter_config_1     iter_config_1        ; /* 10 */
	union klt_iter_config_2     iter_config_2        ; /* 14 */
	union klt_eval_param        eval_param           ; /* 18 */
	union klt_prefetch_ctrl     prefetch_ctrl        ; /* 1c */
	union klt_rd_prev_base_addr rd_prev_base_addr[5] ; /* 20 */
	union klt_rd_prev_stride    rd_prev_stride[5]    ; /* 40 */
	union klt_rd_next_base_addr rd_next_base_addr[5] ; /* 60 */
	union klt_rd_next_stride    rd_next_stride[5]    ; /* 80 */
	union klt_kp_prev_buff_x    kp_prev_buff_x[500]  ; /* a0 */
	union klt_kp_prev_buff_y    kp_prev_buff_y[500]  ; /* a4 */
	union klt_kp_next_buff_x    kp_next_buff_x[500]  ; /* 1040 */
	union klt_kp_next_buff_y    kp_next_buff_y[500]  ; /* 1044 */
} s_klt_regs_type;

#endif // __KLT_DRV_PRIV_H__
