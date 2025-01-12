/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2020. All rights reserved.
 * Description: contexthub logbuff header file
 * Author: DIVS_SENSORHUB
 * Create: 2012-05-29
 */

#ifndef __LINUX_SENSORHUB_LOGBUFF_H__
#define __LINUX_SENSORHUB_LOGBUFF_H__

#include "contexthub_boot.h"
#include <platform_include/smart/linux/base/ap/protocol.h>

typedef struct {
	struct pkt_header hd;
	uint32_t index;
} log_buff_req_t;

extern int set_log_level(int tag, int argv[], int argc);
void dmd_manuflush(void);

#endif
