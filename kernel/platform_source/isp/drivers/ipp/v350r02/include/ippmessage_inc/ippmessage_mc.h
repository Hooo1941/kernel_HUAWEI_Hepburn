/******************************************************************
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2018. All rights reserved.
 * File name    ippmessage_mc.h
 * Description:
 *
 * Date         2022-06-27 16:28:10
 ********************************************************************/

#ifndef __IPPMESSAGE_MC_H__
#define __IPPMESSAGE_MC_H__
#include "ippmessage_common.h"
#include "cfg_table_mc.h"

/**
 * MC_REF_FIRST_PAGE_BUFF:
 *    When used with arf, this address must be the same as the descriptor buff address of the arf.
 * MC_H_MATRIX_BUFF:
 *    Include: h-matrix/stable_flag/inlier_num
 **/
typedef enum {
	MC_REF_FIRST_PAGE_BUFF = 0,
	MC_CUR_FIRST_PAGE_BUFF,
	MC_H_MATRIX_BUFF,
	MC_STREAM_MAX,
} mc_stream_buf_usage_e;

/**
 * MC_COORDINATE_PAIRS_BUFF:
 *    The buffer is not used in the normal process because the MC can output the interior
 *    point information through the VP interface. However, when push_inler_en is 0, the MC
 *    can still obtain the interior point information by reading the COORDENATE_PAIRS register.
 *    Therefore, this buffer is added. Currently, this buffer may be used for debugging and UT.
 **/
typedef enum {
	MC_COORDINATE_PAIRS_BUFF = 0,
	MC_LOW_KPT_BUFF, // r02: no use
	MC_STAT_BUFF_MAX,
} mc_stat_buf_usage_e;

typedef struct _msg_req_mc_t {
	ipp_stream_t streams[MC_STREAM_MAX];
	cfg_tab_mc_t reg_cfg;

	// stat out buff;
	ipp_stream_t mc_stat_buff[MC_STAT_BUFF_MAX]; // new
} msg_req_mc_t;

#endif // __IPPMESSAGE_MC_H__

