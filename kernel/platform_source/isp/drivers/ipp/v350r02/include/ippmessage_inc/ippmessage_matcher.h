/******************************************************************
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2018. All rights reserved.
 * File name    ippmessage_matcher.h
 * Description:
 *
 * Date         2022-06-27 16:28:10
 ********************************************************************/

#ifndef __IPPMESSAGE_MATCHER_H__
#define __IPPMESSAGE_MATCHER_H__

#include "ippmessage_common.h"
#include "cfg_table_compare.h"
#include "cfg_table_reorder.h"

typedef enum {
	NORMAL_MATCHER_MODE = 0,
	SINGLE_RDR_MULTI_CMP, // e.g.: 1rdr + 7cmp
	MULTI_RDR_MULTI_CMP,  // e.g.: 7rdr + 6cmp
	MATCHER_MODE_MAX,
} matcher_work_mode_e;

typedef enum {
	BI_RDR_FP = 0,
	BO_RDR_FP_BLOCK,
	RDR_STREAM_MAX,
} rdr_stream_buf_usage_e;

typedef enum {
	BI_CMP_REF_FP = 0,
	BI_CMP_CUR_FP,
	BO_CMP_MATCHED_INDEX_OUT, // The multi-CMP must be met. include CMP_MATCHED_KPT;
	BO_CMP_MATCHED_DIST_OUT,
	CMP_STREAM_MAX,
} cmp_stream_buf_usage_e;

typedef struct _req_rdr_t {
	ipp_stream_t streams[MATCHER_LAYER_MAX][RDR_STREAM_MAX];
	cfg_tab_reorder_t reg_cfg[MATCHER_LAYER_MAX];
} req_rdr_t;

typedef struct _req_cmp_t {
	ipp_stream_t streams[MATCHER_LAYER_MAX][CMP_STREAM_MAX];
	cfg_tab_compare_t reg_cfg[MATCHER_LAYER_MAX];
} req_cmp_t;

/**
 * NORMAL_Matcher_MODE:
 * In NORMAL_Matcher_MODE, the value of rdr_pyramid_layer is 2, which is used in the cur and ref frames.
 * The value of cmp_pyramid_layer is 1, which is used to compare the descriptors of cur and ref.
 * The configuration of the cur frame and the ref frame (mainly the blk configuration parameters) of
 * RDR are the same. Therefore, it does not matter whether the reg_cfg[0] of the RDR is cur or ref.
 * However, other information needs to be managed by the process.
 **/
typedef struct _msg_req_matcher_t {
	unsigned int rdr_pyramid_layer;
	unsigned int cmp_pyramid_layer;
	matcher_work_mode_e  matcher_work_mode;

	req_rdr_t    req_rdr;
	req_cmp_t    req_cmp;
} msg_req_matcher_t;

#endif // __IPPMESSAGE_MATCHER_H__
