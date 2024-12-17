/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: This module is to support device-pipe synergy data frame sending and reveiving, framwork communication.
 * Create: 2023-11-23
 */
#ifndef DPS_DATA_TXRX_H
#define DPS_DATA_TXRX_H

#include "amax_multilink_main.h"

typedef enum {
	DPS_DEV_MAIN = 0,
	DPS_DEV_ASSIST,
	DPS_DEV_DOUBLE,
	DPS_DEV_BUTT
} dps_fn_type;

typedef struct {
	unsigned char send_dev;
	unsigned char bssid[ETHER_ADDR_LEN];
	unsigned char data[0];
} dps_mgmt_msg;

uint32_t check_pkt_is_dps_data(struct sk_buff *netbuf);
void dps_report_data_to_userspace(struct sk_buff *netbuf);
void dps_ioctl_msg_proc(uint8_t *msg_info, unsigned int msg_len);

#endif
