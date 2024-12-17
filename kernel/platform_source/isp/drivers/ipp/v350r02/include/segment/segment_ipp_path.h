/*******************************************************************
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2018. All rights reserved.
 * File name    segment_ipp_path.h
 * Description:
 *
 * Date        2020-04-16
 ******************************************************************/

#ifndef __SEG_IPP_PATH_CS_H__
#define __SEG_IPP_PATH_CS_H__

#include "ipp.h"
#include "ippmessage.h"
#include "segment_common.h"
#include "segment_enh_arfeature.h"
#include "segment_mc.h"
#include "segment_matcher.h"
#include "segment_klt.h"

int ipp_path_request_handler(struct msg_req_ipp_cmd_req_t *req);
#endif // __SEG_IPP_PATH_CS_H__