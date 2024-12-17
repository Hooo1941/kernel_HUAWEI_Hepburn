/*******************************************************************
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2018. All rights reserved.
 * File name    ippmessage_arfeature.h
 * Description:
 *
 * Date        2022-06-25
 ******************************************************************/

#ifndef __IPPMESSAGE_ARFEATURE_H__
#define __IPPMESSAGE_ARFEATURE_H__
#include "ippmessage_common.h"
#include "cfg_table_arfeature.h"
#include "cfg_table_orb_enh.h"

typedef enum {
	CUR_ONLY = 0,
	CUR_REF = 1,
} work_frame_e;

typedef enum {
	BI_ENH_YHIST = 0,
	BI_ENH_YIMAGE,
	BO_ENH_OUT,
	ORB_ENH_STREAM_MAX,
} enh_buf_usage_e;

typedef enum {
	BI_ARFEATURE_0 = 0, // 0
	BI_ARFEATURE_1,
	BI_ARFEATURE_2,
	BI_ARFEATURE_3,
	BO_ARFEATURE_PYR_1,
	BO_ARFEATURE_PYR_2, // 5
	BO_ARFEATURE_DOG_0,
	BO_ARFEATURE_DOG_1,
	BO_ARFEATURE_DOG_2,
	BO_ARFEATURE_DOG_3,
	BO_ARFEATURE_PYR_2_DS, // 10
	ARFEATURE_STREAM_MAX,
} arf_stream_buf_usage_e;

typedef enum {
	ARF_DESC_BUFF = 0,
	ARF_MINSCR_KPTCNT_BUFF, // Save 6 layers's data
	ARF_SUM_SCORE_BUFF,
	ARF_KPT_NUM_BUFF,
	ARF_TOTAL_KPT_BUFF,
	ARF_KLTPRE_TOTAL_KPT_NUM, // new in r02
	ARF_NXT_DOG_THRESHOLD, // new in r02
	ARF_KPT_LIMIT_BUFF, // new in r02
	ARF_STAT_BUFF_MAX,
} arf_stat_buf_usage_e;

typedef struct _msg_req_enh_t {
	ipp_stream_t streams[ORB_ENH_STREAM_MAX];
	cfg_tab_orb_enh_t reg_cfg;
} msg_req_enh_t;

typedef struct _msg_req_arf_t {
	ipp_stream_t  streams[ARFEATURE_MAX_LAYER][ARFEATURE_STREAM_MAX];
	cfg_tab_arf_t reg_cfg[ARFEATURE_MAX_LAYER];

	// stat out buff
	ipp_stream_t  arf_stat_buff[ARF_STAT_BUFF_MAX];
} msg_req_arf_t;

typedef struct _msg_req_enh_arf_t {
	unsigned int   mode_cnt; // arf's layer cnt
	work_frame_e   work_frame; // cur, cur+ref

	// cur
	msg_req_enh_t  req_enh_cur;
	msg_req_arf_t  req_arf_cur;

	// ref
	msg_req_enh_t  req_enh_ref;
	msg_req_arf_t  req_arf_ref;
} msg_req_enh_arf_t;

#endif // __IPPMESSAGE_ARFEATURE_H__ 
