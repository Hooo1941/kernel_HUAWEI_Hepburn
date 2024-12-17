/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019. All rights reserved.
 * Description: iomcu_logbuff.h.
 * Create: 2019/11/05
 */

#ifndef __IOMCU_LOGBUFF_H__
#define __IOMCU_LOGBUFF_H__

#include <platform_include/smart/linux/base/ap/protocol.h>

typedef struct {
	struct pkt_header hd;
	uint32_t index;
} log_buff_req_t;

extern int set_log_level(int tag, int argv[], int argc);

#endif
