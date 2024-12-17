/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : hmac_wapi.c的头文件
 * 作    者 :
 * 创建日期 : 2015年5月27日
 */
#ifndef __HMAC_WPI_H__
#define __HMAC_WPI_H__
#if (defined(_PRE_WLAN_FEATURE_WAPI) || defined(_PRE_WLAN_FEATURE_PWL))

#include "mac_vap.h"
#include "hd_pwl_common.h"
#define WPI_PN_LEN 16  /* WAPI/PWL协议的PN长度为16字节 */
#define WPI_HEAD_LEN   18  /* PWL/WAPI协议头长度都为18字节 */
typedef enum {
    KEY_TYPE_MIC = 0,
    KEY_TYPE_DATA = 1,
    KEY_TYPE_BUTT
} hmac_key_type_enum;

enum {
    PN_TYPE_INVALID,
    PN_TYPE_VALID_WITHOUT_UPDATE,
    PN_TYPE_VALID,
};

struct hmac_user_tag;

typedef struct {
    uint32_t rx_netbuf_alloc_fail;
    uint32_t rx_ctrl_copy_fail;
    uint32_t rx_mac_hdr_copy_fail;
    uint32_t rx_decrypt_fail;
    uint32_t rx_mic_calc_fail;
    uint32_t rx_mic_diff_fail;
    uint32_t rx_decrypt_ok;
} hmac_wpi_rx_debug;

typedef struct {
    uint32_t tx_netbuf_alloc_fail;
    uint32_t tx_ctrl_copy_fail;
    uint32_t tx_mac_hdr_copy_fail;
    uint32_t tx_mic_calc_fail;
    uint32_t tx_encrypt_data_alloc_fail;
    uint32_t tx_playload_copy_fail;
    uint32_t tx_mic_copy_fail;
    uint32_t tx_encrypt_fail;
    uint32_t tx_set_header_fail;
    uint32_t tx_encrypt_ok;
} hmac_wpi_tx_debug;

typedef struct {
    hmac_wpi_rx_debug rx_debug;
    hmac_wpi_tx_debug tx_debug;
} hmac_wpi_debug;

typedef struct hmac_encrypt_params {
    uint8_t *mic_key;
    uint8_t *data_key;
    uint8_t *tx_pn;
    uint8_t pn_inc;
    uint8_t reserve;
    uint16_t pc;
    hmac_wpi_tx_debug *wpi_tx_debug;

    uint32_t (*hmac_wpi_set_head)(struct hmac_encrypt_params*, oal_netbuf_stru*);
    void (*hmac_wpi_update_tx_pn)(struct hmac_encrypt_params*);
    uint32_t (*encrypt_data)(struct hmac_encrypt_params *, oal_netbuf_stru *, oal_netbuf_stru *);
} hmac_encrypt_params_stru;

typedef struct hmac_decrypt_params {
    uint8_t *mic_key;
    uint8_t *data_key;
    uint8_t pn_inc;
    uint8_t reserve[3];
    uint8_t *rx_pn; // user下的rx pn
    hmac_wpi_rx_debug *wpi_rx_debug;
    uint16_t (*hmac_wpi_get_rx_pc)(uint8_t*); /* 获取报文中pc的钩子 */
    uint8_t* (*hmac_wpi_get_rx_pn)(uint8_t*); /* 获取报文中pn的钩子 */
    uint32_t (*decrypt_data)(struct hmac_decrypt_params *, oal_netbuf_stru *, oal_netbuf_stru *, uint8_t *, uint16_t);
    uint8_t (*hmac_wpi_check_rx_pn)(uint8_t *user_pn, uint8_t *buf_pn);
} hmac_decrypt_params_stru;

/* 数据帧加解密流程，提取pwl或wapi的钩子函数 */
typedef struct hmac_wpi_cb_tag {
    /* 通用流程，根据keyIndex和keyType获取add key下发的key */
    uint8_t* (*hmac_wpi_get_key_cb)(struct hmac_user_tag*, uint8_t, uint8_t);

    /* 加密流程，获取add key下发的keyIndex (PWL[pc]，WAPI[keyIndex] */
    uint8_t (*hmac_wpi_get_tx_key_index_cb)(struct hmac_user_tag*);

    /* 加密流程，获取add key下发的pn */
    uint8_t* (*hmac_wpi_get_tx_pn_cb)(struct hmac_user_tag*);
    /* 加密流程，获取步长信息 pn_inc */
    uint16_t (*hmac_wpi_get_pn_inc)(struct hmac_user_tag*);
    /* 加密流程，设置协议头(PWL或WAPI) */
    uint32_t (*hmac_wpi_set_head_cb)(hmac_encrypt_params_stru*, oal_netbuf_stru*);

    /* 解密流程，从数据帧的协议头里获取keyIndex */
    uint8_t (*hmac_wpi_get_rx_key_index_cb)(uint8_t*);
    /* 解密流程，从数据帧的协议头里获取pn */
    uint8_t* (*hmac_wpi_get_rx_pn_cb)(uint8_t*);
    /* 解密流程，从数据帧的协议头里获取pc */
    uint16_t (*hmac_wpi_get_rx_pc_cb)(uint8_t*);
    /* 解密流程，获取user下的rx pn */
    uint8_t* (*hmac_wpi_get_user_rx_pn_cb)(struct hmac_user_tag*, mac_rx_ctl_stru *);
    /* 加密netbuf的函数钩子 */
    uint32_t (*encrypt_data)(struct hmac_encrypt_params *, oal_netbuf_stru *, oal_netbuf_stru *);
    /* 解密netbuf的函数钩子 */
    uint32_t (*decrypt_data)(struct hmac_decrypt_params *, oal_netbuf_stru *, oal_netbuf_stru *, uint8_t *, uint16_t);
    /* 检查rx_pn的函数钩子 */
    uint8_t (*hmac_wpi_check_rx_pn)(uint8_t *user_pn, uint8_t *buf_pn);
} hmac_wpi_cb;

static inline void hmac_wpi_insert_netbuf(oal_netbuf_stru *netbuf_old, oal_netbuf_stru *netbuf_new)
{
    oal_netbuf_next(netbuf_new) = oal_netbuf_next(netbuf_old);
    oal_netbuf_next(netbuf_old) = netbuf_new;
}
uint32_t hmac_wpi_decrypt_rx_data(struct hmac_user_tag *hmac_user, oal_netbuf_stru *netbuf);
oal_netbuf_stru* hmac_wpi_encrypt_tx_data(struct hmac_user_tag *hmac_user, oal_netbuf_stru *netbuf);
uint32_t hmac_user_debug_display_wpi_info(mac_vap_stru *mac_vap, uint16_t user_idx);
void hmac_wpi_swap_pn(uint8_t *pn, uint8_t len);
uint32_t hmac_wpi_encrypt_data(hmac_encrypt_params_stru *encrypt_params,
    oal_netbuf_stru *netbuf_new, oal_netbuf_stru *netbuf_old);
uint32_t hmac_wpi_decrypt_data(
    hmac_decrypt_params_stru *decrypt_params, oal_netbuf_stru *netbuf_new,
    oal_netbuf_stru *netbuf_old, uint8_t *pn, uint16_t pc);
#endif
#endif /* end of hmac_wapi_wpi.h */
