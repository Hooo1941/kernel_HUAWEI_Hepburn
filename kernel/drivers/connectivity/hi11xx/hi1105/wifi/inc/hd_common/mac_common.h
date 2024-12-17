/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : dmac对外公共接口头文件
 * 作    者 :
 * 创建日期 : 2012年9月20日
 */

#ifndef MAC_COMMON_H
#define MAC_COMMON_H

/* 1 其他头文件包含 */
#include "oal_types.h"
#include "wlan_types.h"
#include "mac_common_subevt.h"
#include "hal_common.h"
#include "mac_device_common.h"
#include "mac_user_common.h"
#include "mac_vap_common.h"
#include "mac_cb_tx.h"
#include "mac_cb_rx.h"

// 此处不加extern "C" UT编译不过
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define is_rw_ring_full(read, write)                                    \
    (((read)->st_rw_ptr.bit_rw_ptr == (write)->st_rw_ptr.bit_rw_ptr) && \
        ((read)->st_rw_ptr.bit_wrap_flag != (write)->st_rw_ptr.bit_wrap_flag))

/* 抛往dmac侧消息头的长度 */
#define CUSTOM_MSG_DATA_HDR_LEN (sizeof(custom_cfgid_enum_uint32) + sizeof(uint32_t))

#ifdef _PRE_WLAN_FEATURE_NRCOEX
#define DMAC_WLAN_NRCOEX_INTERFERE_RULE_NUM (4) /* 5gnr共存干扰参数表，目前共4组 */
#endif

#define mac_user_is_he_user(_user)          (MAC_USER_HE_HDL_STRU(_user)->en_he_capable)
#define mac_user_arp_probe_close_htc(_user) ((_user)->bit_arp_probe_close_htc)

#define mac_user_tx_data_include_htc(_user) ((_user)->bit_tx_data_include_htc)
#define mac_user_tx_data_include_om(_user)  ((_user)->bit_tx_data_include_om)

#define mac_user_not_support_htc(_user)  \
    (((_user)->st_cap_info.bit_qos == 0) || (mac_user_tx_data_include_htc(_user) == 0))
typedef struct {
    uint32_t ftm_cali_time;
    uint8_t vap_id;
} dmac_to_hmac_ftm_ctx_event_stru; /* hd_event */
typedef struct {
    custom_cfgid_enum_uint32 en_syn_id; /* 同步配置ID */
    uint32_t len;                  /* DATA payload长度 */
    uint8_t auc_msg_body[NUM_4_BYTES];          /* DATA payload */
} hmac_to_dmac_cfg_custom_data_stru;

/* BA会话的状态枚举 */
typedef enum {
    DMAC_BA_INIT = 0,   /* BA会话未建立 */
    DMAC_BA_INPROGRESS, /* BA会话建立过程中 */
    DMAC_BA_COMPLETE,   /* BA会话建立完成 */
    DMAC_BA_HALTED,     /* BA会话节能暂停 */
    DMAC_BA_FAILED,     /* BA会话建立失败 */

    DMAC_BA_BUTT
} dmac_ba_conn_status_enum;
typedef uint8_t dmac_ba_conn_status_enum_uint8;

/* 配置命令带宽能力枚举 */
typedef enum {
    WLAN_BANDWITH_CAP_20M,
    WLAN_BANDWITH_CAP_40M,
    WLAN_BANDWITH_CAP_40M_DUP,
    WLAN_BANDWITH_CAP_80M,
    WLAN_BANDWITH_CAP_80M_DUP,
    WLAN_BANDWITH_CAP_160M,
    WLAN_BANDWITH_CAP_160M_DUP,
    WLAN_BANDWITH_CAP_80PLUS80,
#ifdef  _PRE_WLAN_FEATURE_11AX_ER_SU_DCM
    WLAN_BANDWITH_CAP_ER_242_TONE,
    WLAN_BANDWITH_CAP_ER_106_TONE,
#endif
    WLAN_BANDWITH_CAP_BUTT
} wlan_bandwith_cap_enum;
typedef uint8_t wlan_bandwith_cap_enum_uint8;

typedef enum {
    DMAC_CHANNEL1    = 1,
    DMAC_CHANNEL11   = 11,
    DMAC_CHANNEL12   = 12,
    DMAC_CHANNEL36   = 36,
    DMAC_CHANNEL48   = 48,
    DMAC_CHANNEL149  = 149,
    DMAC_CHANNEL165  = 165,
    DMAC_PC_PASSIVE_CHANNEL_BUTT
} dmac_pc_passive_channel_enum;

typedef struct {
    uint16_t us_baw_start;                          /* 第一个未确认的MPDU的序列号 */
    uint16_t us_last_seq_num;                       /* 最后一个发送的MPDU的序列号 */
    uint16_t us_baw_size;                           /* Block_Ack会话的buffer size大小 */
    uint16_t us_baw_head;                           /* bitmap中记录的第一个未确认的包的位置 */
    uint16_t us_baw_tail;                           /* bitmap中下一个未使用的位置 */
    oal_bool_enum_uint8 en_is_ba;                     /* Session Valid Flag */
    dmac_ba_conn_status_enum_uint8 en_ba_conn_status; /* BA会话的状态 */
    mac_back_variant_enum_uint8 en_back_var;          /* BA会话的变体 */
    uint8_t uc_dialog_token;                        /* ADDBA交互帧的dialog token */
    uint8_t uc_ba_policy;                           /* Immediate=1 Delayed=0 */
    oal_bool_enum_uint8 en_amsdu_supp;                /* BLOCK ACK支持AMSDU的标识 */
    uint8_t *puc_dst_addr;                          /* BA会话接收端地址 */
    uint16_t us_ba_timeout;                         /* BA会话交互超时时间 */
    uint8_t uc_ampdu_max_num;                       /* BA会话下，能够聚合的最大的mpdu的个数 */
    uint8_t uc_tx_ba_lut;                           /* BA会话LUT session index */
    uint16_t us_mac_user_idx;
#ifdef _PRE_WLAN_FEATURE_DFR
    uint16_t us_pre_baw_start;    /* 记录前一次判断ba窗是否卡死时的ssn */
    uint16_t us_pre_last_seq_num; /* 记录前一次判断ba窗是否卡死时的lsn */
    uint16_t us_ba_jamed_cnt;     /* BA窗卡死统计次数 */
#else
    uint8_t auc_resv[NUM_2_BYTES];
#endif
    uint32_t aul_tx_buf_bitmap[DMAC_TX_BUF_BITMAP_WORDS];
} dmac_ba_tx_stru;

typedef enum {
    OAL_QUERY_STATION_INFO_EVENT = 1,
    OAL_ATCMDSRV_DBB_NUM_INFO_EVENT = 2,
    OAL_ATCMDSRV_FEM_PA_INFO_EVENT = 3,
    OAL_ATCMDSRV_GET_RX_PKCG = 4,
    OAL_ATCMDSRV_LTE_GPIO_CHECK = 5,
    OAL_ATCMDSRV_GET_ANT = 6,
    OAL_QUERY_EVNET_BUTT
} oal_query_event_id_enum;

typedef union {
    uint16_t rw_ptr; /* write index */
    struct {
        uint16_t bit_rw_ptr    : 15,
                 bit_wrap_flag : 1;
    } st_rw_ptr;
} un_rw_ptr;

typedef enum {
    TID_CMDID_CREATE                 = 0,
    TID_CMDID_DEL,
    TID_CMDID_ENQUE,
    TID_CMDID_STOP_TX,

    TID_CMDID_BUTT,
} tid_cmd_enum;
typedef uint8_t tid_cmd_enum_uint8;

typedef enum {
    RING_PTR_EQUAL = 0,
    RING_PTR_SMALLER,
    RING_PTR_GREATER,
} ring_ptr_compare_enum;
typedef uint8_t ring_ptr_compare_enum_uint8;

typedef enum {
    HOST_RING_TX_MODE = 0,
    DEVICE_RING_TX_MODE = 1,
    DEVICE_TX_MODE = 2,
    H2D_WAIT_SWITCHING_MODE = 3,
    H2D_SWITCHING_MODE = 4,
    D2H_WAIT_SWITCHING_MODE = 5,
    D2H_SWITCHING_MODE = 6,
    TX_MODE_DEBUG_DUMP = 7,
    H2D_SWITCHING_MODE_STAGE2 = 8,

    TX_MODE_BUTT,
} ring_tx_mode_enum;

typedef enum {
    DATA_TYPE_ETH = 0,    /* Eth II以太网报文类型, 无vlan */
    DATA_TYPE_1_VLAN_ETH = 1,    /* Eth II以太网报文类型, 有1个vlan */
    DATA_TYPE_2_VLAN_ETH = 2,    /* Eth II以太网报文类型, 有2个vlan */
    DATA_TYPE_80211 = 3,         /* 802.11报文类型 */
    DATA_TYPE_0_VLAN_8023 = 4,   /* 802.3以太网报文类型, 无vlan */
    DATA_TYPE_1_VLAN_8023 = 5,   /* 802.3以太网报文类型, 有1个vlan */
    DATA_TYPE_2_VLAN_8023 = 6,   /* 802.3以太网报文类型, 有2个vlan */
    DATA_TYPE_MESH = 7,          /* Mesh报文类型, 无extend addresss */
    DATA_TYPE_MESH_EXT_ADDR = 8, /* Mesh报文类型, 有extend addresss */

    DATA_TYPE_BUTT
} data_type_enum;

#define DATA_TYPE_802_3_SNAP (BIT(2))
#define MAC_KEY_FRAME_SOFT_RETRY_CNT 5

typedef struct {
    uint32_t msdu_addr_lsb;
    uint32_t msdu_addr_msb;
    uint32_t msdu_len      : 12;
    uint32_t data_type     : 4;
    uint32_t frag_num      : 4;
    uint32_t to_ds         : 1;
    uint32_t from_ds       : 1;
    uint32_t more_frag     : 1;
    uint32_t reserved      : 1;
    uint32_t aggr_msdu_num : 4;
    uint32_t first_msdu    : 1;
    uint32_t csum_type     : 3;
} msdu_info_stru;

/* Host Device公用部分 */
typedef struct {
    uint8_t size;
    uint8_t max_amsdu_num;
    uint8_t reserved[2];
    uint32_t base_addr_lsb;
    uint32_t base_addr_msb;
    uint16_t read_index;
    uint16_t write_index;
} msdu_info_ring_stru; /* hal_common.h? */

typedef struct {
    uint16_t user_id;
    uint8_t tid_no;
    uint8_t cmd;
    uint8_t ring_index; /* 受限于内存, device ring和tid无法做到1:1分配,
                         * host侧h2d切换时, 需要在bitmap中找到下一个可分配的ring index,
                         * device根据index从ring内存池中获取可用的ring,
                         * d2h切换时同理, host侧需要将某个已占用的ring index设置为可用
                         */
    uint8_t hal_dev_id;
    uint8_t tid_num; /* 需要切换的tid数量 */
    uint8_t resv;
} tx_state_switch_stru;

typedef struct {
    uint8_t device_loop_sche_enabled;
} tx_sche_switch_stru;

#ifdef _PRE_WLAN_FEATURE_NRCOEX
typedef struct {
    uint32_t freq;
    uint32_t gap0_40m_20m;
    uint32_t gap0_160m_80m;
    uint32_t gap1_40m_20m;
    uint32_t gap1_160m_80m;
    uint32_t gap2_140m_20m;
    uint32_t gap2_160m_80m;
    uint32_t smallgap0_act;
    uint32_t gap01_act;
    uint32_t gap12_act;
    int32_t l_rxslot_rssi;
} nrcoex_rule_stru;

typedef struct {
    uint32_t us_freq_low_limit : 16,
               us_freq_high_limit : 16;
    uint32_t us_20m_w2m_gap0 : 16,
               us_40m_w2m_gap0 : 16;
    uint32_t us_80m_w2m_gap0 : 16,
               us_160m_w2m_gap0 : 16;
    uint32_t us_20m_w2m_gap1 : 16,
               us_40m_w2m_gap1 : 16;
    uint32_t us_80m_w2m_gap1 : 16,
               us_160m_w2m_gap1 : 16;
    uint32_t us_20m_w2m_gap2 : 16,
               us_40m_w2m_gap2 : 16;
    uint32_t us_80m_w2m_gap2 : 16,
               us_160m_w2m_gap2 : 16;
    uint32_t uc_20m_smallgap0_act : 8,
               uc_40m_smallgap0_act : 8,
               uc_80m_smallgap0_act : 8,
               uc_160m_smallgap0_act : 8;
    uint32_t uc_20m_gap01_act : 8,
               uc_40m_gap01_act : 8,
               uc_80m_gap01_act : 8,
               uc_160m_gap01_act : 8;
    uint32_t uc_20m_gap12_act : 8,
               uc_40m_gap12_act : 8,
               uc_80m_gap12_act : 8,
               uc_160m_gap12_act : 8;
    uint32_t uc_20m_rxslot_rssi : 8,
               uc_40m_rxslot_rssi : 8,
               uc_80m_rxslot_rssi : 8,
               uc_160m_rxslot_rssi : 8;
} nrcoex_rule_detail_stru;

typedef union {
    nrcoex_rule_stru st_nrcoex_rule_data;
    nrcoex_rule_detail_stru st_nrcoex_rule_detail_data;
} nrcoex_rule_data_union;

typedef struct {
    uint8_t uc_nrcoex_enable;
    uint8_t nrcoex_version;
    uint8_t rsv[NUM_2_BYTES];  /* 保留字段:2字节 */
    nrcoex_rule_data_union un_nrcoex_rule_data[DMAC_WLAN_NRCOEX_INTERFERE_RULE_NUM];
} nrcoex_cfg_info_stru;
#endif

typedef struct {
    uint8_t cfg_type;
    oal_bool_enum_uint8 need_w4_dev_return;
    uint8_t resv[NUM_2_BYTES]; /* 4 字节对齐 */
    uint32_t dev_ret;
} mac_cfg_cali_hdr_stru;

#ifdef _PRE_WLAN_FEATURE_PHY_EVENT_INFO
typedef struct {
    uint8_t               event_rpt_en;
    uint8_t               rsv[NUM_3_BYTES]; /* 4 字节对齐 */
    uint32_t              wp_mem_num;       /* 事件统计个数 */
    uint32_t              wp_event0_type_sel;  /* event0事件类型选择参数 */
    uint32_t              wp_event1_type_sel;  /* event1事件类型选择参数 */
} mac_cfg_phy_event_rpt_stru;
#endif

enum vsp_tx_sync_stage_enum {
    VSP_TX_SYNC_LOCK_ACQUIRE = 0,
    VSP_TX_SYNC_CLEAR_FAIL,
    VSP_TX_SYNC_CLEAR_DONE,
    VSP_TX_SYNC_LOCK_RELEASE,
    VSP_TX_SYNC_INIT
};

typedef enum {
    MAC_MVAP_ERROR_SCENE_START = 0,
    MAC_MVAP_ERROR_SCENE_CONNECT = 1,
    MAC_MVAP_ERROR_SCENE_ROAM = 2,
    MAC_MVAP_ERROR_SCENE_CSA = 3,

    HMAC_MVAP_ERROR_BUTT
} mac_mvap_error_scene;

typedef struct {
    uint8_t vap_mode;
    uint8_t p2p_mode;;
    uint8_t chba_mode;
    uint8_t scene;
} mac_chr_mvap_info_stru;

typedef enum {
    CHR_VERSION_1 = 0,

    CHR_VERSION_BUTT
} chr_version_flag;

typedef enum {
    CHBA_VAP_MODE = 0,
    P2P_GO_MODE,
    P2P_GC_MODE,
    LEGACY_STA_MODE,
    LEGACY_AP_MODE,

    VAP_MODE_BUTT
} mac_dbac_vap_mode_enum;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of mac_common.h */
