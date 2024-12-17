/*
* Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
* 功能说明   : tcp ack 缓存功能
* 作者       : wifi
* 创建日期   : 2020年07月22日
*/
#ifndef HMAC_TCP_ACK_BUF_H
#define HMAC_TCP_ACK_BUF_H

#include "hmac_user.h"
#include "hmac_vap.h"

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_TCP_ACK_BUF_H

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/* 2 宏定义 */
#define TCP_ACK_BUF_MAX_NUM           20  // 默认缓存的最大tcp ack个数
#define TCP_ACK_TIMEOUT               10 // tcp cak缓存机制的定时器周期 ms

typedef struct {
    uint8_t tcp_ack_buf[WLAN_VAP_SUPPORT_MAX_NUM_LIMIT];
    uint8_t tcp_ack_timeout;
    uint16_t tcp_ack_max_num;
} hmac_tcp_ack_buf_para_stru;
typedef struct {
    oal_netbuf_head_stru hdr;
    uint16_t tcp_ack_num;
    uint8_t resv[BYTE_OFFSET_2];
    hmac_vap_stru *hmac_vap;
    frw_timeout_stru tcp_ack_timer;
    oal_spin_lock_stru spin_lock;
} hmac_tcp_ack_buf_stru;

typedef struct {
    uint8_t uc_ini_tcp_ack_buf_en;
    uint8_t uc_ini_tcp_ack_buf_userctl_test_en;
    uint8_t uc_cur_tcp_ack_buf_en[2]; // 2代表2个vap
    uint8_t uc_ini_tcp_buf_en;
    uint8_t tcp_buf_resv;
    uint16_t us_tcp_ack_buf_throughput_high;
    uint16_t us_tcp_ack_buf_throughput_low;
    uint16_t tcp_ack_buf_throughput_high_40m;
    uint16_t tcp_ack_buf_throughput_low_40m;
    uint16_t tcp_ack_buf_throughput_high_80m;
    uint16_t tcp_ack_buf_throughput_low_80m;
    uint16_t tcp_ack_buf_throughput_high_160m;
    uint16_t tcp_ack_buf_throughput_low_160m;
    uint16_t us_tcp_ack_buf_userctl_high;
    uint16_t us_tcp_ack_buf_userctl_low;
    uint16_t us_tcp_ack_smooth_throughput;
} mac_tcp_ack_buf_switch_stru; // only host

#ifdef _PRE_WLAN_TCP_OPT
typedef struct {
    /* 定制化是否打开tcp ack过滤 */
    uint8_t uc_tcp_ack_filter_en;
    /* 当前状态 */
    uint8_t uc_cur_filter_status;
    uint16_t us_rx_filter_throughput_high;
    uint16_t us_rx_filter_throughput_low;
    uint16_t us_resv;
} mac_tcp_ack_filter_stru; // only host
extern mac_tcp_ack_filter_stru g_st_tcp_ack_filter;
#endif

mac_tcp_ack_buf_switch_stru *mac_vap_get_tcp_ack_buf_switch(void);
void hmac_tx_tcp_ack_buf_switch(uint32_t rx_throughput_mbps);
void hmac_device_tcp_buf_switch(uint32_t tx_throughput_mbps, uint32_t rx_throughput_mbps);
uint32_t hmac_tcp_ack_buff_config_mp13(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param);
uint32_t hmac_tcp_ack_buff_config_mp16(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param);
void hmac_tcp_ack_buf_init_user(hmac_user_stru *hmac_user);
void hmac_del_user_reset_tcp_ack_buf(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user);
uint32_t hmac_tx_tcp_ack_buf_process(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    oal_netbuf_stru *netbuf);
void hmac_tcp_ack_buf_init_para(void);
uint32_t hmac_config_tcp_ack_buf(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param);
void hmac_device_tcp_buf_init(uint8_t enabled);
uint8_t hmac_device_tcp_buf_duplicate_ack_proc(oal_ip_header_stru *ip_hdr);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif

