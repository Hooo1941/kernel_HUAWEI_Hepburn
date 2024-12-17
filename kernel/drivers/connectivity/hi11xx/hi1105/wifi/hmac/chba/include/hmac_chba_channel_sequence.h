/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : CHBA DBAC头文件
 * 作    者 :
 * 创建日期 : 2022年3月24日
 */

#ifndef HMAC_CHBA_CHANNEL_SEQUENCE_H
#define HMAC_CHBA_CHANNEL_SEQUENCE_H

#ifdef _PRE_WLAN_CHBA_MGMT
#include "hmac_vap.h"

void hmac_chba_set_channel_seq_bitmap(hmac_chba_vap_stru *chba_vap_info, uint32_t channel_seq_bitmap);
uint32_t hmac_chba_set_channel_seq_params(mac_vap_stru *mac_vap, uint32_t *params);
void hmac_chba_update_channel_seq_when_connect(void);
void hmac_chba_update_channel_seq_when_dbac_notify(dmac_dbac_notify_to_hmac_stru *dbac_notify_data);

void hmac_chba_update_sec_work_channel(const mac_channel_stru *work_channel,
    mac_channel_stru *second_work_channel);
uint32_t hmac_chba_dbac_update_fcs_config(mac_vap_stru *mac_vap, uint8_t len, uint8_t *param);
#endif
#endif /* HMAC_CHBA_CHANNEL_SEQUENCE_H */
