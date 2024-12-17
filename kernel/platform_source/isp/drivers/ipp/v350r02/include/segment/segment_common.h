/******************************************************************
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2018. All rights reserved.
 * File name    segment_common.h
 * Description:
 *
 * Date         2020-04-17 14:23:09
 ********************************************************************/

#ifndef SEGMENT_COMMON_CS_H_INCLUDED
#define SEGMENT_COMMON_CS_H_INCLUDED

#include <linux/types.h>
#include "ipp.h"
#include "cmdlst_common.h"
#include "ippmessage_common.h"

typedef struct _channel_map_t {
	unsigned int enh_arf_channel;
	unsigned int rdr_channel;
	unsigned int cmp_channel;
	unsigned int mc_channel;
	unsigned int klt_channel;
} channel_map_t;

typedef struct _global_info_ipp_t {
	unsigned int enh_arf_channel;
	unsigned int rdr_channel;
	unsigned int cmp_channel;
	unsigned int mc_channel;
	unsigned int klt_channel;

	unsigned int work_module; // = req->work_module

	// unsigned int work_module_bitset;
	unsigned int enh_en;
	unsigned int arf_en;
	unsigned int matcher_en;
	unsigned int mc_en;
	unsigned int klt_en;

	unsigned int matcher_update_flag; // update rdr kpt, cmp kpt, rdr total kpt, rdr frame height;
	unsigned int mc_update_flag; // updata mc's macther_kpt, index_cfg
	unsigned int klt_update_flag;

	unsigned int orb_enh_stripe_cnt;
	unsigned int arf_stripe_cnt;
	unsigned int rdr_stripe_cnt;
	unsigned int cmp_stripe_cnt; // cmp's total stripe cnt
	unsigned int cmp_layer_stripe_cnt;
	unsigned int mc_stripe_cnt;
	unsigned int klt_stripe_cnt;
	unsigned int total_stripe_cnt;

	unsigned int rdr_total_num_addr_cur;
	unsigned int rdr_kpt_num_addr_cur;
	unsigned int rdr_total_num_addr_ref;
	unsigned int rdr_kpt_num_addr_ref;
	unsigned int rdr_frame_height_addr_cur;
	unsigned int rdr_frame_height_addr_ref;
	unsigned int cmp_total_num_addr;
	unsigned int cmp_kpt_num_addr_cur;
	unsigned int cmp_kpt_num_addr_ref;
	unsigned int cmp_frame_height_addr;
	unsigned int mc_index_addr;
	unsigned int mc_matched_num_addr;
}global_info_ipp_t;

typedef enum {
	DMAP_GAUSS_FIRST = 0,
	AR_GAUSS_FIRST = 1,
	GAUSS_MIDDLE = 2,
	GAUSS_LAST = 3,
	DETECT_AND_DESC_AND_MFPDA_DISABLE = 4,
	DETECT_AND_DESC_AND_MFPDA_ENABLE = 5,
	DETECT_AND_KLT_AND_MFPDA_DISABLE = 6,
	DETECT_AND_KLT_AND_MFPDA_ENABLE = 7,
} arfeature_work_mode_e;

typedef enum {
	FRAME_CUR = 0,
	FRAME_REF = 1,
	MAX_FRAME_FLAG,
} frame_flag_e;

typedef enum {
	SET_IRQ = 0,
	CLR_IRQ,
} cfg_irq_type_e;

typedef enum {
	CONNECT_TO_CVDR = 1,
	CONNECT_TO_ARF  = 2,
	CONNECT_TO_CVDR_AND_ARF = 3,
} enh_vpb_cfg_e;

typedef struct _crop_region_info_t {
	unsigned int x;
	unsigned int y;
	unsigned int width;
	unsigned int height;
} crop_region_info_t;

typedef struct _ipp_size_t {
	unsigned int width;
	unsigned int height;
} ipp_size_t;

typedef struct _ipp_stripe_info_t {
	crop_region_info_t crop_region;
	ipp_size_t   pipe_work_size;
	ipp_size_t   full_size;

	unsigned int stripe_cnt;
	unsigned int overlap_left[MAX_CPE_STRIPE_NUM];
	unsigned int overlap_right[MAX_CPE_STRIPE_NUM];
	unsigned int stripe_width[MAX_CPE_STRIPE_NUM];
	unsigned int stripe_start_point[MAX_CPE_STRIPE_NUM];
	unsigned int stripe_end_point[MAX_CPE_STRIPE_NUM];
} ipp_stripe_info_t;

typedef struct _df_size_constrain_t {
	unsigned int hinc;
	unsigned int pix_align;
	unsigned int out_width;
} df_size_constrain_t;

typedef struct _split_stripe_tmp_t {
	unsigned int overlap;
	unsigned int in_full_width;
	unsigned int active_stripe_width;
	unsigned int max_in_stripe_align;
	unsigned int max_frame_width;
	unsigned int stripe_width;
	unsigned int input_align;
	unsigned int stripe_cnt;

	unsigned int stripe_start;
	unsigned int stripe_end;
	unsigned long long int active_stripe_end;
	unsigned int last_stripe_end;
} split_stripe_tmp_t;

// ***********************************Function Interface***************************************
extern channel_map_t g_channel_map[WORK_MODULE_MAX];
int seg_ipp_set_cmdlst_wr_buf(cmd_buf_t *cmd_buf, cmdlst_wr_cfg_t *wr_cfg);
int seg_ipp_set_cmdlst_wr_buf_cmp(cmd_buf_t *cmd_buf, cmdlst_wr_cfg_t *wr_cfg,
	unsigned int match_points_offset);

#endif /* SEGMENT_COMMON_CS_H_INCLUDED */
