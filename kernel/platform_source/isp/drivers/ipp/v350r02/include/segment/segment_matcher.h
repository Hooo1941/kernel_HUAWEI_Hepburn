/******************************************************************
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2018. All rights reserved.
 * File name    segment_matcher.h
 * Description:
 *
 * Date         2020-04-17 16:28:10
 ********************************************************************/

#ifndef __SEG_MATCHER_CS_H__
#define __SEG_MATCHER_CS_H__

#include "ippmessage.h"
#include "segment_common.h"

int seg_matcher_cmp_request_handler(msg_req_matcher_t *matcher_req, global_info_ipp_t *global_info);
int seg_matcher_rdr_request_handler(msg_req_matcher_t *matcher_req, global_info_ipp_t *global_info);

#endif
