/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明   : 定义HOST/DEVICE传递事件枚举/结构体 的头文件
 * 作者       :
 * 创建日期   : 2020年10月23日
 */

#ifndef HD_EVENT_D2H_H
#define HD_EVENT_D2H_H

#include "mac_common.h"
#include "wlan_spec.h"

typedef struct {
    dmac_disasoc_misc_reason_enum_uint16 en_disasoc_reason;
    uint16_t us_user_idx;
} dmac_disasoc_misc_stru; /* hd_event */

typedef struct {
    uint8_t uc_reason;
    uint8_t uc_event;
    uint8_t auc_des_addr[WLAN_MAC_ADDR_LEN];
} dmac_diasoc_deauth_event; /* hd_event */

typedef struct {
    int8_t rssi0;
    int8_t rssi1;
} dmac_to_hmac_sniffer_info_event_stru;

typedef struct {
    uint16_t us_user_id;
    uint8_t uc_ba_size;
    oal_bool_enum_uint8 en_need_delba;
} dmac_to_hmac_btcoex_rx_delba_trigger_event_stru; /* hd_event */

typedef struct {
    uint16_t us_user_id;
    oal_bool_enum_uint8 en_rx_amsdu;
    uint8_t uc_ps_type;
} dmac_to_hmac_ps_rx_delba_event_stru; /* hd_event */

typedef struct {
    uint16_t us_user_id;
    oal_bool_enum_uint8 en_arp_probe;
    uint8_t uc_ps_type;
} dmac_to_hmac_ps_arp_probe_event_stru; /* hd_event */

typedef struct {
    uint16_t us_user_index;
    uint8_t uc_tid;
    uint8_t uc_vap_id;
    uint8_t uc_cur_protocol;
    uint8_t auc_reserve[NUM_3_BYTES];
} dmac_to_hmac_ctx_event_stru; /* hd_event */

typedef struct {
    uint16_t us_user_index;
    uint8_t uc_cur_bandwidth;
    uint8_t uc_cur_protocol;
} dmac_to_hmac_syn_info_event_stru; /* hd_event */

typedef struct {
    uint8_t uc_vap_id;
    uint8_t en_voice_aggr; /* 是否支持Voice聚合 */
    uint8_t auc_resv[NUM_2_BYTES];
} dmac_to_hmac_voice_aggr_event_stru; /* hd_event */

/* d2h hal vap信息同步 */
typedef struct {
    uint8_t hal_dev_id;
    uint8_t mac_vap_id;
    uint8_t hal_vap_id;
    uint8_t valid;
} d2h_hal_vap_info_syn_event; /* hd_event */

/* mic攻击 */
typedef struct {
    uint8_t auc_user_mac[WLAN_MAC_ADDR_LEN];
    uint8_t auc_reserve[NUM_2_BYTES];
    oal_nl80211_key_type en_key_type;
    int32_t l_key_id;
} dmac_to_hmac_mic_event_stru;  /* hd_event */

typedef struct {
    uint32_t tx_ring_base_addr;
    uint32_t ba_info_ring_base_addr_dev0;
    uint32_t ba_info_ring_base_addr_dev1;
} mac_d2h_tx_ring_base_addr_stru; /* hd_event */

typedef struct {
    uint32_t device_vspinfo_addr;
} mac_d2h_device_vspinfo_addr; /* hd_event */

/* dmac将bt状态同步到hmac,用于hmac在共存场景做处理 */
typedef struct {
    uint16_t bt_reg0;
    uint16_t bt_reg1;
    uint16_t bt_reg2;
    uint16_t bt_reg3;
} mac_btcoex_bt_state_stru; /* hd_event */

typedef enum {
    NON_CHBA_NO_CSA = 0,
    NON_CHBA_CSA_DOING,
    NON_CHBA_CSA_DONE,

    NON_CHBA_CSA_BUTT
} dmac_dbac_non_chba_csa_status_enum;

#ifdef _PRE_WLAN_FEATURE_NAN
typedef enum {
    NAN_CFG_TYPE_SET_PARAM = 0,
    NAN_CFG_TYPE_SET_TX_MGMT,

    NAN_CFG_TYPE_BUTT
} mac_nan_cfg_type_enum;
typedef uint8_t mac_nan_cfg_type_enum_uint8;

typedef enum {
    NAN_RSP_TYPE_SET_PARAM = 0,
    NAN_RSP_TYPE_SET_TX_MGMT,
    NAN_RSP_TYPE_CANCEL_TX,
    NAN_RSP_TYPE_ERROR,

    NAN_RSP_TYPE_BUTT
} dmac_nan_rsp_type_enum;
typedef uint8_t dmac_nan_rsp_type_enum_uint8;

typedef struct {
    uint32_t duration;
    uint32_t period;
    uint8_t type;
    uint8_t band;
    uint8_t channel;
} mac_nan_param_stru; /* hd_event */

typedef struct {
    uint8_t action;
    uint8_t periodic;
    uint16_t len;
    uint8_t *data;
} mac_nan_mgmt_info_stru; /* hd_event */

typedef struct {
    mac_nan_cfg_type_enum_uint8 type;
    uint16_t transaction_id;
    mac_nan_param_stru param;
    mac_nan_mgmt_info_stru mgmt_info;
} mac_nan_cfg_msg_stru; /* hd_event */

typedef struct {
    uint16_t transaction;
    uint8_t type;
    uint8_t status;
    uint8_t action; /* nan action类型 */
} mac_nan_rsp_msg_stru; /* hd_event */
#endif /* _PRE_WLAN_FEATURE_NAN */

typedef struct dmac_dbac_notify_to_hmac_tag {
    oal_bool_enum_uint8 dbac_enabled;
    uint8_t dbac_type;
    uint8_t led_vap_id;
    uint8_t flw_vap_id;
    uint8_t dbac_non_chba_csa_status; // 0:不涉及 1：csa doing 2：csa done
} dmac_dbac_notify_to_hmac_stru;

#endif /* end of hd_event_d2h.h */
