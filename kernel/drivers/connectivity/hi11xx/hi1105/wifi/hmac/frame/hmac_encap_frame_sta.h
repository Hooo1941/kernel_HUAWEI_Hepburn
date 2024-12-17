/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : hmac_encap_frame_sta.c 的头文件
 * 作    者 : wifi
 * 创建日期 : 2013年6月28日
 */

#ifndef HMAC_ENCAP_FRAME_STA_H
#define HMAC_ENCAP_FRAME_STA_H

#include "oal_ext_if.h"
#include "oal_types.h"
#include "hmac_vap.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

uint32_t hmac_mgmt_encap_asoc_req_sta(hmac_vap_stru *pst_hmac_sta,
    uint8_t *puc_req_frame, uint8_t *puc_curr_bssid, uint8_t *dest_addr);
uint16_t hmac_mgmt_encap_auth_req(hmac_vap_stru *pst_sta, uint8_t *puc_mgmt_frame, uint8_t *dest_addr);
uint16_t hmac_mgmt_encap_auth_req_seq3(hmac_vap_stru *pst_sta, uint8_t *puc_mgmt_frame, uint8_t *puc_mac_hrd,
    mac_rx_ctl_stru *rx_ctl);
void hmac_add_cowork_ie(hmac_vap_stru *hmac_sta, uint8_t *puc_buffer, uint16_t *pus_ie_len);
void hmac_set_supported_rates_ie_asoc_req(hmac_vap_stru *pst_hmac_sta, uint8_t *puc_buffer, uint8_t *puc_ie_len);
void hmac_set_exsup_rates_ie_asoc_req(hmac_vap_stru *pst_hmac_sta, uint8_t *puc_buffer, uint8_t *puc_ie_len);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_encap_frame_sta.h */
