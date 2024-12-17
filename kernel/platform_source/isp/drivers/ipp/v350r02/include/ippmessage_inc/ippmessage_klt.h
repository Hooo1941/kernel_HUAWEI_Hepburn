/******************************************************************
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2018. All rights reserved.
 * File name    ippmessage_klt.h
 * Description:
 *
 * Date         2022-06-29 16:28:10
 ********************************************************************/

#ifndef __IPPMESSAGE_KLT_H__
#define __IPPMESSAGE_KLT_H__

#include "ippmessage_common.h"
#include "cfg_table_klt.h"

typedef enum {
	KLT_OFF = 0,
	KLT_ONLY_DS,
	KLT_ONLY_KLT,
	KLT_DS_AND_KLT,
	KLT_WORK_MODE_MAX,
} klt_work_mode_e;

typedef enum {
	KLT_DS_PREV_IMG_BUFF = 0, // Y img, which needs to be processed by DS;
	KLT_DS_NEXT_IMG_BUFF,

	// these is uesed to save DS's output;
	KLT_PREV_LAYER0_BUFF,
	KLT_PREV_LAYER1_BUFF,
	KLT_PREV_LAYER2_BUFF,
	KLT_PREV_LAYER3_BUFF,
	KLT_PREV_LAYER4_BUFF,
	KLT_NEXT_LAYER0_BUFF,
	KLT_NEXT_LAYER1_BUFF,
	KLT_NEXT_LAYER2_BUFF,
	KLT_NEXT_LAYER3_BUFF,
	KLT_NEXT_LAYER4_BUFF,

	KLT_STREAM_MAX,
} klt_stream_buf_usage_e;

typedef enum {
	KLT_KP_PREV_BUFF = 0,
	KLT_KP_NEXT_BUFF,
	KLT_STAT_BUFF_MAX,
} klt_stat_buf_usage_e;

typedef struct _msg_req_klt_t {
	klt_work_mode_e work_mode;
	ipp_stream_t	streams[KLT_STREAM_MAX];
	cfg_tab_klt_t   reg_cfg;

	// stat data out buff
	ipp_stream_t    klt_stat_buff[KLT_STAT_BUFF_MAX];
} msg_req_klt_t;

#endif // __IPPMESSAGE_KLT_H__

