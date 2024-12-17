/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : wal_linux_rx_rsp.c 的头文件
 * 作    者 : wifi
 * 创建日期 : 2013年8月26日
 */

#ifndef WAL_LINUX_RX_RSP_H
#define WAL_LINUX_RX_RSP_H

#include "oal_ext_if.h"
#include "oal_types.h"
#include "wal_ext_if.h"
#include "frw_ext_if.h"
#include "hmac_ext_if.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_WAL_LINUX_RX_RSP_H

/* 驱动sta上报内核的扫描结果 */
typedef struct {
    int32_t l_signal; /* 信号强度 */

    int16_t s_freq; /* bss所在信道的中心频率 */
    uint8_t auc_arry[2]; /* 2实现字节对齐 */

    uint32_t mgmt_len;            /* 管理帧长度 */
    oal_ieee80211_mgmt_stru *pst_mgmt; /* 管理帧起始地址 */
} wal_scanned_bss_info_stru;

/* 驱动sta上报内核的关联结果 */
typedef struct {
    uint8_t auc_bssid[WLAN_MAC_ADDR_LEN]; /* sta关联的ap mac地址 */
    uint16_t us_status_code;              /* ieee协议规定的16位状态码 */

    uint8_t *puc_rsp_ie; /* asoc_req_ie  */
    uint8_t *puc_req_ie;

    uint32_t req_ie_len; /* asoc_req_ie len */
    uint32_t rsp_ie_len;

    uint16_t us_connect_status;
    uint8_t auc_resv[2]; /* 2实现字节对齐 */
} oal_connet_result_stru;

/* 驱动sta上报内核的去关联结果 */
typedef struct {
    uint16_t us_reason_code; /* 去关联 reason code */
    uint8_t auc_resv[2]; /* 2实现字节对齐 */

    uint8_t *pus_disconn_ie;    /* 去关联关联帧 ie */
    uint32_t us_disconn_ie_len; /* 去关联关联帧 ie 长度 */
} oal_disconnect_result_stru;

#ifdef _PRE_WLAN_FEATURE_PWL
enum pwl_report_attributes {
    PWL_ATTRIBUTE_INVALID = 0,
    PWL_ATTRIBUTE_PRE_KEY_REQ,
    /* keep last */
    PWL_ATTRIBUTE_BUTT,
};
#endif

uint32_t wal_scan_comp_proc_sta(frw_event_mem_stru *pst_event_mem);
uint32_t wal_asoc_comp_proc_sta(frw_event_mem_stru *pst_event_mem);
#ifdef _PRE_WLAN_FEATURE_PMF
uint32_t wal_report_sta_pmf_port_auth(frw_event_mem_stru *pst_event_mem);
#endif
uint32_t wal_disasoc_comp_proc_sta(frw_event_mem_stru *pst_event_mem);
uint32_t wal_connect_new_sta_proc_ap(frw_event_mem_stru *pst_event_mem);
uint32_t wal_disconnect_sta_proc_ap(frw_event_mem_stru *pst_event_mem);
uint32_t wal_mic_failure_proc(frw_event_mem_stru *pst_event_mem);
uint32_t wal_send_mgmt_to_host(frw_event_mem_stru *pst_event_mem);
uint32_t wal_p2p_listen_timeout(frw_event_mem_stru *pst_event_mem);
uint32_t wal_report_external_auth_req(frw_event_mem_stru *pst_event_mem);
uint32_t wal_report_channel_switch(frw_event_mem_stru *pst_event_mem);
uint32_t wal_nan_response_event_process(frw_event_mem_stru *event_mem);
uint32_t wal_report_rtt_result(frw_event_mem_stru *event_mem);
uint32_t wal_ft_connect_fail_report(frw_event_mem_stru *event_mem);
uint32_t wal_roam_11v_succ_report(frw_event_mem_stru *event_mem);
#ifdef _PRE_WLAN_FEATURE_PWL
uint32_t wal_event_req_pwl_pre_key(frw_event_mem_stru *event_mem);
#endif
#endif /* end of wal_linux_rx_rsp.h */
