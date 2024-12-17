/*******************************************************************
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2018. All rights reserved.
 * File name    reorder_drv_priv.h
 * Description:
 *
 * Date        2021-12-28
 ******************************************************************/

#ifndef __REORDER_DRV_PRIV_H__
#define __REORDER_DRV_PRIV_H__

union reorder_cfg{
	struct {
		unsigned int    reorder_en      : 1  ; /* [0] */
		unsigned int    rsv_0           : 3  ; /* [3:1] */
		unsigned int    descriptor_type : 1  ; /* [4] */
		unsigned int    rsv_1           : 27  ; /* [31:5] */
	} bits;

	unsigned int    u32;
};

union reorder_block_cfg{
	struct {
		unsigned int    blk_v_num : 4  ; /* [3:0] */
		unsigned int    rsv_2     : 4  ; /* [7:4] */
		unsigned int    blk_h_num : 4  ; /* [11:8] */
		unsigned int    rsv_3     : 4  ; /* [15:12] */
		unsigned int    blk_num   : 8  ; /* [23:16] */
		unsigned int    rsv_4     : 8  ; /* [31:24] */
	} bits;

	unsigned int    u32;
};

union reorder_total_kpt_num{
	struct {
		unsigned int    total_kpt_num : 12  ; /* [11:0] */
		unsigned int    rsv_5         : 20  ; /* [31:12] */
	} bits;

	unsigned int    u32;
};

union reorder_prefetch_cfg{
	struct {
		unsigned int    first_32k_page  : 19  ; /* [18:0] */
		unsigned int    rsv_6           : 1  ; /* [19] */
		unsigned int    prefetch_enable : 1  ; /* [20] */
		unsigned int    rsv_7           : 11  ; /* [31:21] */
	} bits;

	unsigned int    u32;
};

union reorder_debug_0{
	struct {
		unsigned int    debug_0 : 32  ; /* [31:0] */
	} bits;

	unsigned int    u32;

};

union reorder_debug_1{
	struct {
		unsigned int    debug_1 : 32  ; /* [31:0] */
	} bits;

	unsigned int    u32;
};

union reorder_ec_0{
	struct {
		unsigned int    ec_0 : 32  ; /* [31:0] */
	} bits;

	unsigned int    u32;
};

union reorder_ec_1{
	struct {
		unsigned int    ec_1 : 32  ; /* [31:0] */
	} bits;

	unsigned int    u32;
};

union reorder_kpt_number{
	struct {
		unsigned int    kpt_num : 8  ; /* [7:0] */
		unsigned int    rsv_8   : 24  ; /* [31:8] */
	} bits;

	unsigned int    u32;
};

// ==============================================================================
/* define the global struct */
typedef struct {
	union reorder_cfg           reorder_cfg     ; /* 0 */
	union reorder_block_cfg     block_cfg       ; /* 4 */
	union reorder_total_kpt_num total_kpt_num   ; /* 8 */
	union reorder_prefetch_cfg  prefetch_cfg    ; /* 10 */
	union reorder_debug_0       debug_0         ; /* 20 */
	union reorder_debug_1       debug_1         ; /* 24 */
	union reorder_ec_0          ec_0            ; /* 28 */
	union reorder_ec_1          ec_1            ; /* 2c */
	union reorder_kpt_number    kpt_number[256] ; /* 100 */
} s_reorder_regs_type;

#endif // __REORDER_DRV_PRIV_H__
