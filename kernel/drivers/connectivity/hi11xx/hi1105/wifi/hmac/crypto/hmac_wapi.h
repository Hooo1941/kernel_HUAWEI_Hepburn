/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : hmac_wapi.c的头文件
 * 作    者 : wifi
 * 创建日期 : 2015年5月27日
 */
#ifndef HMAC_WAPI_H
#define HMAC_WAPI_H

#ifdef _PRE_WLAN_FEATURE_WAPI

#include "oal_net.h"
#include "mac_resource.h"
#include "hmac_vap.h"
#include "hmac_user.h"
#include "hmac_rx_data.h"
#include "securec.h"
#define WAPI_UCAST_INC 2                  /* 发送或者接收单播帧,pn的步进值 */
#define WAPI_BCAST_INC 1                  /* 发送或者接收组播帧,pn的步进值 */
#define WAPI_WAI_TYPE  (uint16_t)0x88B4 /* wapi的以太类型 */

#define WAPI_BCAST_KEY_TYPE 1
#define WAPI_UCAST_KEY_TYPE 0

#define SMS4_MIC_LEN (uint8_t)16 /* SMS4封包MIC的长度 */

#define SMS4_PN_LEN           16 /* wapi pn的长度 */
#define SMS4_KEY_IDX          1  /* WAPI头中 keyidx占1个字节 */
#define SMS4_WAPI_HDR_RESERVE 1  /* WAPI头中保留字段 */
#define HMAC_WAPI_HDR_LEN     (uint8_t)(SMS4_PN_LEN + SMS4_KEY_IDX + SMS4_WAPI_HDR_RESERVE)
#define WAPI_PDU_LEN          2  /* wapi头中，wapi pdu len字段所占字节数 */

#define WAPI_IE_VERSION        1  /* wapi的version */
#define WAPI_IE_VER_SIZE       2  /* wapi ver-ie 所占字节数 */
#define WAPI_IE_SUIT_TYPE_SIZE 1  /* suit type size */
#define WAPI_IE_WAPICAP_SIZE   2  /* wapi cap字段所占字节数 */
#define WAPI_IE_BKIDCNT_SIZE   2  /* wapi bkid数字段所占字节数 */
#define WAPI_IE_BKID_SIZE      16 /* 一个bkid所占字节数 */
#define WAPI_IE_OUI_SIZE       3  /* wapi oui字节数 */
#define WAPI_IE_SMS4           1  /* wapi加密类型为sms4 */
#define WAPI_IE_SUITCNT_SIZE   2  /* wapi suit count所占字节数 */
/* wapi key len */
static inline oal_bool_enum_uint8 wapi_port_flag(hmac_wapi_stru *wapi)
{
    return (oal_bool_enum_uint8)wapi->uc_port_valid;
}

#ifdef _PRE_WAPI_DEBUG
static inline void wapi_tx_drop_inc(hmac_wapi_stru *wapi)
{
    wapi->st_debug.ultx_ucast_drop++;
}
static inline void wapi_tx_wai_inc(hmac_wapi_stru *wapi)
{
    wapi->st_debug.ultx_wai++;
}
static inline void wapi_tx_port_valid(hmac_wapi_stru *wapi)
{
    wapi->st_debug.ultx_port_valid++;
}
static inline void wapi_rx_port_valid(hmac_wapi_stru *wapi)
{
    wapi->st_debug.ulrx_port_valid++;
}
static inline void wapi_rx_idx_err(hmac_wapi_stru *wapi)
{
    wapi->st_debug.ulrx_idx_err++;
}
static inline void wapi_rx_netbuf_len_err(hmac_wapi_stru *wapi)
{
    wapi->st_debug.ulrx_netbuff_len_err++;
}
static inline void wapi_rx_idx_update_err(hmac_wapi_stru *wapi)
{
    wapi->st_debug.ulrx_idx_update_err++;
}
static inline void wapi_rx_key_en_err(hmac_wapi_stru *wapi)
{
    wapi->st_debug.ulrx_key_en_err++;
}

static inline void wapi_rx_memalloc_err(hmac_wapi_stru *wapi)
{
    wapi->st_debug.ulrx_memalloc_err++;
}
static inline void wapi_rx_mic_err(hmac_wapi_stru *wapi)
{
    wapi->st_debug.ulrx_mic_calc_fail++;
}
static inline void wapi_rx_decrypt_ok(hmac_wapi_stru *wapi)
{
    wapi->st_debug.ulrx_decrypt_ok++;
}

static inline void wapi_tx_memalloc_err(hmac_wapi_stru *wapi)
{
    wapi->st_debug.ultx_memalloc_err++;
}
static inline void wapi_tx_mic_err(hmac_wapi_stru *wapi)
{
    wapi->st_debug.ultx_mic_calc_fail++;
}
static inline void wapi_tx_encrypt_ok(hmac_wapi_stru *wapi)
{
    wapi->st_debug.ultx_encrypt_ok++;
}

#else
static inline void wapi_tx_drop_inc(hmac_wapi_stru *wapi)
{
}
static inline void wapi_tx_wai_inc(hmac_wapi_stru *wapi)
{
}
static inline void wapi_tx_port_valid(hmac_wapi_stru *wapi)
{
}
static inline void wapi_rx_port_valid(hmac_wapi_stru *wapi)
{
}
static inline void wapi_rx_idx_err(hmac_wapi_stru *wapi)
{
}
static inline void wapi_rx_netbuf_len_err(hmac_wapi_stru *wapi)
{
}
static inline void wapi_rx_idx_update_err(hmac_wapi_stru *wapi)
{
}
static inline void wapi_rx_key_en_err(hmac_wapi_stru *wapi)
{
}
static inline void wapi_rx_memalloc_err(hmac_wapi_stru *wapi)
{
}
static inline void wapi_rx_mic_err(hmac_wapi_stru *wapi)
{
}
static inline void wapi_rx_decrypt_ok(hmac_wapi_stru *wapi)
{
}
static inline void wapi_tx_memalloc_err(hmac_wapi_stru *wapi)
{
}
static inline void wapi_tx_mic_err(hmac_wapi_stru *wapi)
{
}
static inline void wapi_tx_wai_drop_inc(hmac_wapi_stru *wapi)
{
}
static inline void wapi_tx_encrypt_ok(hmac_wapi_stru *wapi)
{
}

#endif /* #ifdef WAPI_DEBUG_MODE */

#define WAPI_PN_LEN 16  /* WAPI协议的PN长度为16字节 */
#pragma pack (1) /* 指定按1字节对齐 */
typedef struct {
    uint8_t key_index;
    uint8_t res;
    uint8_t pn[WAPI_PN_LEN];
} wapi_head_stru;
#pragma pack()

typedef struct {
    uint8_t (*wapi_filter_frame)(struct tag_hmac_wapi_stru *hmac_wapi, oal_netbuf_stru *netbuff);
    uint32_t (*wapi_decrypt)(hmac_user_stru *hmac_user, oal_netbuf_stru *netbuf);
    oal_netbuf_stru* (*wapi_encrypt)(hmac_user_stru *hmac_user, oal_netbuf_stru *netbuf);
    oal_netbuf_stru *(*wapi_netbuff_txhandle)(hmac_user_stru *hmac_user, oal_netbuf_stru *netbuf);
    oal_netbuf_stru *(*wapi_netbuff_rxhandle)(hmac_user_stru *hmac_user, oal_netbuf_stru *netbuf);
} hmac_wapi_info_stru;
void hmac_user_wapi_deinit(hmac_user_stru *hmac_user);
uint32_t hmac_user_wapi_init(hmac_user_stru *hmac_user, uint8_t pairwise);
hmac_wapi_info_stru *hmac_get_wapi_info(void);
#ifdef _PRE_WAPI_DEBUG
uint32_t hmac_wapi_display_info(mac_vap_stru *pst_mac_vap, uint16_t user_idx);
uint32_t hmac_wapi_set_debug_hw_wapi(mac_vap_stru *mac_vap, uint32_t *params);
#endif
uint32_t hmac_wapi_add_key(hmac_user_stru *hmac_user, oal_bool_enum_uint8 pairwise, uint8_t key_idx, uint8_t *wapi_key);
void hmac_wapi_set_port(hmac_wapi_stru *hmac_wapi, oal_bool_enum_uint8 value);
oal_netbuf_stru *hmac_wapi_netbuf_tx_encrypt_mp16(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    oal_netbuf_stru *netbuf);
uint32_t hmac_wapi_netbuf_rx_decrypt_mp16(hmac_host_rx_context_stru *rx_context);
oal_netbuf_stru *hmac_wapi_encrypt_netbuf(hmac_user_stru *hmac_user, oal_netbuf_stru *netbuf);
void hmac_wapi_init(void);
void hmac_wapi_update_pn(uint8_t *pn, uint8_t inc);

#endif /* end of _PRE_WLAN_FEATURE_WAPI */

#endif /* end of hmac_wapi.h */
