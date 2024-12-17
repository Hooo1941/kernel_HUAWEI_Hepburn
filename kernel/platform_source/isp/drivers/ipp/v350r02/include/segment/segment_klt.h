/******************************************************************
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2018. All rights reserved.
 * File name    segment_klt.h
 * Description:
 *
 * Date         2022-08-07 16:28:10
 ********************************************************************/

#ifndef _SEGMENT_KLT_CS_H
#define _SEGMENT_KLT_CS_H

#include "ippmessage.h"
#include "segment_common.h"

enum klt_ds_layer_vp_wr_idx_e {
	TAG_FOR_KLT_LAYER_0_VP_WR = 0,
	TAG_FOR_KLT_LAYER_1_VP_WR = 1,
	TAG_FOR_KLT_LAYER_2_VP_WR = 2,
	TAG_FOR_KLT_LAYER_3_VP_WR = 3,
	TAG_FOR_KLT_LAYER_4_VP_WR = 4,
	MAX_TAG_FOR_KLT_LAYER,
};

int seg_klt_request_handler(msg_req_klt_t *klt_req, global_info_ipp_t *global_info);

#endif // _SEGMENT_KLT_CS_H

