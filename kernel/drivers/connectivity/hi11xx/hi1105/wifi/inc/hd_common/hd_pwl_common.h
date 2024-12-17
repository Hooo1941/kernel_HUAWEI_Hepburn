/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : hmac_pwl.c头文件
 */
#ifndef __HD_PWL_COMMON_H__
#define __HD_PWL_COMMON_H__
#ifdef _PRE_WLAN_FEATURE_PWL
#include "wlan_spec.h"
#define PWL_MAX_KEY_ID 2
#define PWL_KEY_LEN 16
#define PWL_PN_LEN 16
#define PWL_UCAST_INC 2
#define PWL_BCAST_INC 2
#define PWL_PRE_RK_LEN 32
#define PWL_BPN_LEN 16
#define PWL_HBPN_OFFSET 3
#define PWL_PRE_RK_PN_LEN 13 /* 计算预置单播秘钥的需要PN的长度,高13字节 */
#define PWL_PRE_RK_PN_OFFSET 3 /* 计算预置秘钥的只取高13字节,所以偏移3 */
#define PWL_PRE_RK_MAC_ADDR_LEN 3 /* 计算预置单播秘钥的需要用户mac地址的长度第3~第5字节 */
/*
addkey_params->uc_key_index为约定下发的keyIndex，约定规则:
    BIT1: 0代表正式秘钥，1代表预置秘钥
    BIT0: 秘钥的keyIndex
PWL协议约定
    BIT6: 0代表正式秘钥，1代表预置秘钥
    BIT0: 秘钥的keyIndex
*/
#define PWL_PRE_KEY_INDEX       1 /* 预置秘钥index默认为1 */
#define PWL_KEY_INDEX_MASK      BIT0
#define PWL_PRE_KEY_MASK        BIT6

#define WLAN_CIPHER_SUITE_PWL_OFB 0x00e0fc01
#define WLAN_CIPHER_SUITE_PWL_GCM 0x00e0fc02
#define WLAN_CIPHER_SUITE_PWL_SSID 0x00e0fcff
#define WLAN_CIPHER_SUITE_PWL_PRE_RK 0x00e0fcfe
#define WLAN_AKM_PWL_CERT 0x00e0fc01
#define WLAN_AKM_PWL_FT 0x00e0fc02
#define PWL_OUI 0x00e0fc        /* oui */
#define PWL_OUI_TYPE 0xfe       /* oui 类型 */
#define PWL_OUI_LEN 4           /* oui 长度(包括oui和type) */
/* PWL信息元素ID */
#define PWL_SSID_TAG                     0x26 /* PWL_SSID 的TAG */
#define PWL_AEAD_TAG                     0x21 /* PWL_AEAD 的TAG */
#define PWL_SECPARA_TAG                  0x20 /* PWL_SecPara 的TAG */
#define PWL_MDE_TAG                      0x24 /* PWL_MDE 的TAG */

#define GCM_AAD_MAX_LEN 33
#define GCM_IV_LEN 12

typedef enum {
    PWL_HOST_ENCRYPT = 0,  /* Host 处理数据帧加密 */
    PWL_DEV_ENCRYPT = 1,  /* Device 处理数据帧加密 */
    PWL_ENCRYPT_SWTICHING = 2,  /* 加密host、dev切换过程中 */

    PWL_HOST_DECRYPT = 3,  /* Host 处理数据帧解密 */
    PWL_DEV_DECRYPT = 4,  /* Device 处理数据帧解密 */
    PWL_DECRYPT_SWTICHING = 5,  /* 解密host、dev切换过程中 */

    PWL_ENCRYPT_DECRYPT_BUTT
} pwl_encrypt_decrypt_switch_enum;

typedef enum {
    PWL_ROAM_NOTIFY_SAVE_KEY_EVENT,     // 用于漫游开始，下发保存秘钥的事件
    PWL_ROAM_NOTIFY_RESTORE_KEY_EVENT,  // 用于漫游失败，下发恢复秘钥的事件
    PWL_ROAM_NOTIFY_DROP_KEY_BAK_EVENT, // 用于漫游成功，下发释放秘钥备份内存的事件

    PWL_ROAM_NOTIFY_EVENT_BUTT,
} pwl_roam_notify_event_enum;
typedef struct {
    uint8_t user_id;
    uint8_t notify_event;
} mac_pwl_roam_notify_stru;

typedef enum {
    PWL_TX_PN,
    PWL_RX_PN,
} pwl_trx_pn_type_enum;

typedef enum {
    BE_RX_PN,
    BK_RX_PN,
    VI_RX_PN,
    VO_RX_PN,
    BASIC_DATA_RX_PN, // 该元素之前均为数据帧pn

    MGMT_RX_PN,
    RX_PN_MAX_NUM,
} pwl_rx_pn_type_enum;
typedef struct {
    uint8_t tx_pn[PWL_PN_LEN];
} mac_pwl_data_pn_stru;

typedef struct {
    uint8_t mic_key[PWL_KEY_LEN];
    uint8_t data_key[PWL_KEY_LEN];
    uint8_t tx_pn[PWL_PN_LEN];
    uint32_t cipher;
} mac_pwl_key_stru;
typedef struct {
    oal_bool_enum_uint8 port_valid;
    uint8_t key_index; // PWL协议，PWL帧体结构里PC字段的BIT0
    uint8_t pn_inc; // pn增长步长
    oal_bool_enum_uint8 is_pwl_pre_key; // 1为预置秘钥 0为正式秘钥 只有dmac使用
    mac_pwl_key_stru pwl_key[PWL_MAX_KEY_ID];
    uint8_t rx_pn[RX_PN_MAX_NUM][PWL_PN_LEN]; // 包括管理帧和数据帧，共计6组rx_pn
    mac_pwl_data_pn_stru data_pn; // dev侧保存数据帧的tx_pn，只有dmac使用
    uint8_t pwl_user_index; // 用于标识当前user申请的pwl_user全局变量的哪一个下标
    uint8_t rsv[NUM_3_BYTES];
} mac_user_pwl_stru;

typedef struct {
    oal_bool_enum_uint8 pwl_en;
    uint8_t pwl_roam_state;
    uint8_t rsv[NUM_2_BYTES];
    uint8_t pwl_pre_rk[PWL_PRE_RK_LEN];
    uint32_t cipher; /* pre rk的cipher */
    uint8_t pwl_bpn[PWL_BPN_LEN];
    uint8_t encrypt_mode;       // hal_encrypt_decrypt_switch_enum 类型
    uint8_t encrypt_to_dev_cnt; // 记录加密决策切到dev的次数，用于快速切到host，缓慢切到dev
    uint8_t decrypt_mode;       // hal_encrypt_decrypt_switch_enum 类型
    uint8_t decrypt_to_dev_cnt; // 记录解密决策切到dev的次数，用于快速切到host，缓慢切到dev
} dmac_pwl_vap_stru;
typedef struct {
    uint8_t sync_type; // 同步类型，同hal_encrypt_decrypt_switch_enum
    uint8_t user_id;
    uint8_t multi_user_pn[PWL_PN_LEN]; // 组播user pn
    uint8_t ucast_user_pn[PWL_PN_LEN]; // 单播user pn

    /* 用于rx_pn */
    uint8_t multi_user_rx_pn[RX_PN_MAX_NUM][PWL_PN_LEN];
    uint8_t ucast_user_rx_pn[RX_PN_MAX_NUM][PWL_PN_LEN];
} mac_sync_trxpn_stru;

#endif
#endif
