/*******************************************************************
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2018. All rights reserved.
 * File name    orb_enh_drv_priv.h
 * Description:
 *
 * Date        2022-07-01
 ******************************************************************/

#ifndef __ORB_ENH_DRV_PRIV_H__
#define __ORB_ENH_DRV_PRIV_H__

union enh_enable_cfg {
	struct {
		unsigned int    enh_en : 1  ; /* [0] */
		unsigned int    rsv_0  : 31  ; /* [31:1] */
	} bits;

	unsigned int    u32;
};

union enh_blk_num {
	struct {
		unsigned int    blknumx : 4  ; /* [3:0] */
		unsigned int    blknumy : 4  ; /* [7:4] */
		unsigned int    rsv_1   : 24  ; /* [31:8] */
	} bits;

	unsigned int    u32;
};

union enh_blk_size {
	struct {
		unsigned int    blk_xsize : 8  ; /* [7:0] */
		unsigned int    blk_ysize : 8  ; /* [15:8] */
		unsigned int    rsv_2     : 16  ; /* [31:16] */
	} bits;

	unsigned int    u32;
};

union enh_inv_blk_size_y {
	struct {
		unsigned int    inv_ysize : 17  ; /* [16:0] */
		unsigned int    rsv_3     : 15  ; /* [31:17] */
	} bits;

	unsigned int    u32;
};

union enh_inv_blk_size_x {
	struct {
		unsigned int    inv_xsize : 17  ; /* [16:0] */
		unsigned int    rsv_4     : 15  ; /* [31:17] */
	} bits;

	unsigned int    u32;
};

union enh_extend_cfg {
	struct {
		unsigned int    edgeex_x : 11  ; /* [10:0] */
		unsigned int    rsv_5    : 5  ; /* [15:11] */
		unsigned int    edgeex_y : 11  ; /* [26:16] */
		unsigned int    rsv_6    : 5  ; /* [31:27] */
	} bits;

	unsigned int    u32;
};

union enh_hist_climit {
	struct {
		unsigned int    climit : 11  ; /* [10:0] */
		unsigned int    rsv_7  : 21  ; /* [31:11] */
	} bits;

	unsigned int    u32;
};

union enh_lut_scale {
	struct {
		unsigned int    lutscale : 18  ; /* [17:0] */
		unsigned int    rsv_8    : 14  ; /* [31:18] */
	} bits;

	unsigned int    u32;
};

// ==============================================================================
/* Define the global struct */
typedef struct {
	union enh_enable_cfg     enable_cfg     ; /* 0 */
	union enh_blk_num        blk_num        ; /* 4 */
	union enh_blk_size       blk_size       ; /* 8 */
	union enh_inv_blk_size_y inv_blk_size_y ; /* c */
	union enh_inv_blk_size_x inv_blk_size_x ; /* 10 */
	union enh_extend_cfg     extend_cfg     ; /* 14 */
	union enh_hist_climit    hist_climit    ; /* 18 */
	union enh_lut_scale      lut_scale      ; /* 1c */
} S_ORB_ENH_REGS_TYPE;

#endif // __ORB_ENH_DRV_PRIV_H__
