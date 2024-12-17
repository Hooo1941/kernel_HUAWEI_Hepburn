/*******************************************************************
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2018. All rights reserved.
 * File name    ippmessage.h
 * Description:
 *
 * Date        2022-06-25
 ******************************************************************/

#ifndef __IPP_MESSAGE_H__
#define __IPP_MESSAGE_H__

#include "ippmessage_enh_arfeature.h"
#include "ippmessage_matcher.h"
#include "ippmessage_mc.h"
#include "ippmessage_klt.h"

struct msg_req_ipp_cmd_req_t {
	unsigned int       instance_id;
	unsigned int       frame_num;
	unsigned int       rate_value;
	work_module_e      work_module;

	msg_req_enh_arf_t  req_enh_arf;
	msg_req_matcher_t  req_matcher;
	msg_req_mc_t       req_mc;
	msg_req_klt_t      req_klt;
};

#endif // __IPP_MESSAGE_H__