/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : cb对外公共接口头文件
 * 作    者 :
 * 创建日期 : 2012年9月20日
 */

#ifndef MAC_CB_TX_H
#define MAC_CB_TX_H

/* 1 其他头文件包含 */
#include "oal_types.h"
#include "wlan_types.h"
#include "mac_frame_common.h"

// 此处不加extern "C" UT编译不过
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/* 专用于CB字段自定义帧类型、帧子类型枚举值 */
typedef enum {
    WLAN_CB_FRAME_TYPE_START = 0,  /* cb默认初始化为0 */
    WLAN_CB_FRAME_TYPE_ACTION = 1, /* action帧 */
    WLAN_CB_FRAME_TYPE_DATA = 2,   /* 数据帧 */
    WLAN_CB_FRAME_TYPE_MGMT = 3,   /* 管理帧，用于p2p等需要上报host */

    WLAN_CB_FRAME_TYPE_BUTT
} wlan_cb_frame_type_enum;
typedef uint8_t wlan_cb_frame_type_enum_uint8;

/* cb字段action帧子类型枚举定义 */
typedef enum {
    WLAN_ACTION_BA_ADDBA_REQ = 0, /* 聚合action */
    WLAN_ACTION_BA_ADDBA_RSP = 1,
    WLAN_ACTION_BA_DELBA = 2,
#ifdef _PRE_WLAN_FEATURE_WMMAC
    WLAN_ACTION_BA_WMMAC_ADDTS_REQ = 3,
    WLAN_ACTION_BA_WMMAC_ADDTS_RSP = 4,
    WLAN_ACTION_BA_WMMAC_DELTS = 5,
#endif
    WLAN_ACTION_SMPS_FRAME_SUBTYPE = 6,   /* SMPS节能action */
    WLAN_ACTION_OPMODE_FRAME_SUBTYPE = 7, /* 工作模式通知action */
    WLAN_ACTION_P2PGO_FRAME_SUBTYPE = 8,  /* host发送的P2P go帧，主要是discoverability request */

#ifdef _PRE_WLAN_FEATURE_TWT
    WLAN_ACTION_TWT_SETUP_REQ = 9,
    WLAN_ACTION_TWT_TEARDOWN_REQ = 10,
#endif
#ifdef _PRE_WLAN_FEATURE_HIEX
    WLAN_ACTION_HIEX = 11,
#endif
#ifdef _PRE_WLAN_FEATURE_NAN
    WLAN_ACTION_NAN_PUBLISH = 12,
    WLAN_ACTION_NAN_FLLOWUP = 13,
    WLAN_ACTION_NAN_SUBSCRIBE = 14,
#endif
#ifdef _PRE_WLAN_FEATURE_FTM
    WLAN_ACTION_FTM_REQUEST = 15,
    WLAN_ACTION_FTM_RESPONE = 16,
    WLAN_ACTION_FTM_RADIO_MEASUREMENT_REPORT = 17,
    WLAN_ACTION_FTM_NEIGHBOR_REPORT_REQUEST = 18,
#endif
    WLAN_FRAME_TYPE_ACTION_BUTT
} wlan_cb_action_subtype_enum;
typedef uint8_t wlan_cb_frame_subtype_enum_uint8;

/* 1字节对齐 */
#pragma pack(push, 1)
typedef struct {
    mac_ieee80211_frame_stru *pst_frame_header; /* 该MPDU的帧头指针 */
    uint16_t us_seqnum;                       /* 记录软件分配的seqnum */
    wlan_frame_type_enum_uint8 en_frame_type;   /* 该帧是控制针、管理帧、数据帧 */
    uint8_t bit_80211_mac_head_type : 1;      /* 0: 802.11 mac头不在skb中,另外申请了内存存放;1:802.11 mac头在skb中 */
    uint8_t en_res : 7;                       /* 是否使用4地址，由WDS特性决定 */
    uint16_t ether_type;                      /* 以太网类型 */
    uint8_t resv1 : 1;
    uint8_t is_icmp : 1;                      /* 是否为icmp */
    uint8_t ipv6_tcp_ack : 1;                 /* 是否为IPV6 tcp_ack */
    uint8_t resv2 : 5;
} mac_tx_expand_cb_stru;

/* 裸系统cb字段 只有20字节可用, 当前使用19字节; HCC[5]PAD[1]FRW[3]+CB[19]+MAC HEAD[36] */
struct mac_tx_ctl {
    /* byte1 */
    /* 取值:FRW_EVENT_TYPE_WLAN_DTX和FRW_EVENT_TYPE_HOST_DRX，作用:在释放时区分是内存池的netbuf还是原生态的 */
    uint8_t bit_event_type : 5;
    uint8_t bit_event_sub_type : 3;
    /* byte2-3 */
    wlan_cb_frame_type_enum_uint8 uc_frame_type;       /* 自定义帧类型 */
    wlan_cb_frame_subtype_enum_uint8 uc_frame_subtype; /* 自定义帧子类型 */
    /* byte4 */
    uint8_t bit_mpdu_num : 7;   /* ampdu中包含的MPDU个数,实际描述符填写的值为此值-1 */
    uint8_t bit_netbuf_num : 1; /* 每个MPDU占用的netbuf数目 */
    /* 在每个MPDU的第一个NETBUF中有效 */
    /* byte5-6 */
    uint16_t us_mpdu_payload_len; /* 每个MPDU的长度不包括mac header length */
    /* byte7 */
    uint8_t bit_frame_header_length : 6; /* 51四地址32 */ /* 该MPDU的802.11头长度 */
    uint8_t bit_is_amsdu : 1;                             /* 是否AMSDU: OAL_FALSE不是，OAL_TRUE是 */
    uint8_t bit_is_first_msdu : 1;                        /* 是否是第一个子帧，OAL_FALSE不是 OAL_TRUE是 */
    /* byte8 */
    uint8_t bit_tid : 4;                  /* dmac tx 到 tx complete 传递的user结构体，目标用户地址 */
    wlan_wme_ac_type_enum_uint8 bit_ac : 3; /* ac */
    uint8_t bit_ismcast : 1;              /* 该MPDU是单播还是多播:OAL_FALSE单播，OAL_TRUE多播 */
    /* byte9 */
    uint8_t bit_retried_num : 4;   /* 重传次数 */
    uint8_t bit_mgmt_frame_id : 4; /* wpas 发送管理帧的frame id */
    /* byte10 */
    uint8_t bit_tx_user_idx : 6;          /* 比描述符中userindex多一个bit用于标识无效index */
    uint8_t bit_roam_data : 1;            /* 漫游期间帧发送标记 */
    uint8_t bit_is_get_from_ps_queue : 1; /* 节能特性用，标识一个MPDU是否从节能队列中取出来的 */
    /* byte11 */
    uint8_t bit_tx_vap_index : 3;
    wlan_tx_ack_policy_enum_uint8 en_ack_policy : 3;
    uint8_t bit_is_needretry : 1;
    uint8_t bit_need_rsp : 1; /* WPAS send mgmt,need dmac response tx status */
    /* byte12 */
    uint8_t en_is_probe_data : 3; /* 是否探测帧 */
    uint8_t bit_is_eapol_key_ptk : 1;                  /* 4 次握手过程中设置单播密钥EAPOL KEY 帧标识 */
    uint8_t bit_is_m2u_data : 1;                       /* 是否是组播转单播的数据 */
    uint8_t bit_is_tcp_ack : 1;                        /* 用于标记tcp ack */
    uint8_t bit_is_rtsp : 1;
    uint8_t en_use_4_addr : 1; /* 是否使用4地址，由WDS特性决定 */
    /* byte13-16 */
    uint16_t us_timestamp_ms;   /* 维测使用入TID队列的时间戳, 单位1ms精度 */
    uint8_t bit_is_qosnull : 1; /* 用于标记qos null帧 */
    uint8_t bit_is_himit : 1; /* 用于标记himit帧 */
    uint8_t bit_hiex_traced : 1; /* 用于标记hiex帧 */
    uint8_t bit_is_game_marked : 1; /* 用于标记游戏帧 */
    uint8_t hal_device_id : 1; /* 用于标识该帧属于哪一路hal_device */
    uint8_t bit_is_802_3_snap : 1; /* 是否是802.3 snap */
    uint8_t bit_is_tcp_data : 1;
    uint8_t urgent_tcp_ack : 1;
    uint8_t uc_data_type : 4; /* 数据帧类型, ring tx使用, 对应data_type_enum */
#ifdef _PRE_WLAN_EXPORT
    uint8_t bit_change_level_rsp : 3;
#else
    uint8_t csum_type : 3;
#endif
    uint8_t bit_is_pt_mcast : 1;  /* 标识帧是否为组播图传picture transmission协议类型 */
    /* byte17-18 */
    uint8_t uc_alg_pktno;     /* 算法用到的字段，唯一标示该报文 */
    uint8_t uc_alg_frame_tag : 2; /* 用于算法对帧进行标记 */
    uint8_t uc_hid2d_tx_delay_time : 6; /* 用于保存hid2d数据帧在dmac的允许传输时间 */
    /* byte19 */
    uint8_t bit_large_amsdu_level : 2;  /* offload下大包AMSDU聚合度 */
    uint8_t bit_align_padding : 2;      /* SKB 头部包含ETHER HEADER时,4字节对齐需要的padding */
    uint8_t bit_msdu_num_in_amsdu : 2;  /* 大包聚合时每个AMSDU子帧数 */
    uint8_t bit_is_large_skb_amsdu : 1; /* 是否是多子帧大包聚合 */
    uint8_t bit_htc_flag : 1;           /* 用于标记htc */

#if defined(_PRE_PRODUCT_ID_HI110X_HOST)
    /* OFFLOAD架构下，HOST相对DEVICE的CB增量 */
    mac_tx_expand_cb_stru st_expand_cb;
#endif
} DECLARE_PACKED;
typedef struct mac_tx_ctl mac_tx_ctl_stru;
#pragma pack(pop)

/* 不区分offload架构的CB字段 */
#define mac_get_cb_is_4address(_pst_tx_ctrl)        ((_pst_tx_ctrl)->en_use_4_addr)
/* 标记是否是小包AMSDU帧 */
#define mac_get_cb_is_amsdu(_pst_tx_ctrl)           ((_pst_tx_ctrl)->bit_is_amsdu)
/* 自适应算法决定的聚合度 */
#define mac_get_cb_amsdu_level(_pst_tx_ctrl)        ((_pst_tx_ctrl)->bit_large_amsdu_level)
/* MSDU帧头部对齐字节 */
#define mac_get_cb_ether_head_padding(_pst_tx_ctrl) ((_pst_tx_ctrl)->bit_align_padding)
/* 实际组成的大包AMSDU帧包含的子帧数 */
#define mac_get_cb_has_msdu_number(_pst_tx_ctrl)    ((_pst_tx_ctrl)->bit_msdu_num_in_amsdu)
/* 标记是否组成了大包AMSDU帧 */
#define mac_get_cb_is_large_skb_amsdu(_pst_tx_ctrl) ((_pst_tx_ctrl)->bit_is_large_skb_amsdu)
#define mac_get_cb_is_first_msdu(_pst_tx_ctrl)      ((_pst_tx_ctrl)->bit_is_first_msdu)
#define mac_get_cb_is_need_resp(_pst_tx_ctrl)       ((_pst_tx_ctrl)->bit_need_rsp)
#define mac_get_cb_is_eapol_key_ptk(_pst_tx_ctrl)   ((_pst_tx_ctrl)->bit_is_eapol_key_ptk)
#define mac_get_cb_is_roam_data(_pst_tx_ctrl)       ((_pst_tx_ctrl)->bit_roam_data)
#define mac_get_cb_is_from_ps_queue(_pst_tx_ctrl)   ((_pst_tx_ctrl)->bit_is_get_from_ps_queue)
#define mac_get_cb_is_mcast(_pst_tx_ctrl)           ((_pst_tx_ctrl)->bit_ismcast)
#define mac_get_cb_is_pt_mcast(_pst_tx_ctrl)        ((_pst_tx_ctrl)->bit_is_pt_mcast)
#define mac_get_cb_is_needretry(_pst_tx_ctrl)       ((_pst_tx_ctrl)->bit_is_needretry)
#define mac_get_cb_is_probe_data(_pst_tx_ctrl)      ((_pst_tx_ctrl)->en_is_probe_data)
#define mac_get_cb_is_rtsp(_pst_tx_ctrl)            ((_pst_tx_ctrl)->bit_is_rtsp)
#define mac_get_cb_alg_tags(_pst_tx_ctrl)           ((_pst_tx_ctrl)->uc_alg_frame_tag)

#define mac_get_cb_mgmt_frame_id(_pst_tx_ctrl) ((_pst_tx_ctrl)->bit_mgmt_frame_id)
#define mac_get_cb_mpdu_len(_pst_tx_ctrl)      ((_pst_tx_ctrl)->us_mpdu_payload_len)
#define mac_get_cb_frame_type(_pst_tx_ctrl)    ((_pst_tx_ctrl)->uc_frame_type)
#define mac_get_cb_frame_subtype(_pst_tx_ctrl) ((_pst_tx_ctrl)->uc_frame_subtype)
#define mac_get_cb_data_type(_pst_tx_ctrl)     ((_pst_tx_ctrl)->uc_data_type)
#define mac_get_cb_is_802_3_snap(_pst_tx_ctrl)    ((_pst_tx_ctrl)->bit_is_802_3_snap)
#define mac_get_cb_timestamp(_pst_tx_ctrl)     ((_pst_tx_ctrl)->us_timestamp_ms)
#define mac_get_cb_is_qosnull(_pst_tx_ctrl)    ((_pst_tx_ctrl)->bit_is_qosnull)
#ifdef _PRE_WLAN_FEATURE_HIEX
#define mac_get_cb_hiex_traced(_pst_tx_ctrl) ((_pst_tx_ctrl)->bit_hiex_traced)
#define mac_get_rx_cb_hiex_traced(_pst_rx_ctl) ((_pst_rx_ctl)->bit_hiex_traced)
#endif
/* VIP数据帧 */
#define mac_get_cb_is_eapol(ptx)                           \
    ((mac_get_cb_frame_type(ptx) == WLAN_CB_FRAME_TYPE_DATA) && (mac_get_cb_frame_subtype(ptx) == MAC_DATA_EAPOL))
#define mac_get_cb_is_arp(ptx)                             \
    ((mac_get_cb_frame_type(ptx) == WLAN_CB_FRAME_TYPE_DATA) && \
     ((mac_get_cb_frame_subtype(ptx) == MAC_DATA_ARP_REQ) || (mac_get_cb_frame_subtype(ptx) == MAC_DATA_ARP_RSP)))
#define mac_get_cb_is_wapi(ptx)                             \
    ((mac_get_cb_frame_type(ptx) == WLAN_CB_FRAME_TYPE_DATA) && (mac_get_cb_frame_subtype(ptx) == MAC_DATA_WAPI))
#define mac_get_cb_is_vipframe(ptx)                             \
    ((mac_get_cb_frame_type(ptx) == WLAN_CB_FRAME_TYPE_DATA) && (mac_get_cb_frame_subtype(ptx) <= MAC_DATA_VIP_FRAME))

#define mac_get_cb_is_smps_frame(_pst_tx_ctrl)                             \
    ((WLAN_CB_FRAME_TYPE_ACTION == mac_get_cb_frame_type(_pst_tx_ctrl)) && \
        (WLAN_ACTION_SMPS_FRAME_SUBTYPE == mac_get_cb_frame_subtype(_pst_tx_ctrl)))
#define mac_get_cb_is_opmode_frame(_pst_tx_ctrl)                           \
    ((WLAN_CB_FRAME_TYPE_ACTION == mac_get_cb_frame_type(_pst_tx_ctrl)) && \
        (WLAN_ACTION_OPMODE_FRAME_SUBTYPE == mac_get_cb_frame_subtype(_pst_tx_ctrl)))
#define mac_get_cb_is_p2pgo_frame(_pst_tx_ctrl)                          \
    ((WLAN_CB_FRAME_TYPE_MGMT == mac_get_cb_frame_type(_pst_tx_ctrl)) && \
        (WLAN_ACTION_P2PGO_FRAME_SUBTYPE == mac_get_cb_frame_subtype(_pst_tx_ctrl)))
#ifdef _PRE_WLAN_FEATURE_TWT
#define mac_get_cb_is_twt_setup_frame(_pst_tx_ctrl)                        \
    ((WLAN_CB_FRAME_TYPE_ACTION == mac_get_cb_frame_type(_pst_tx_ctrl)) && \
        (WLAN_ACTION_TWT_SETUP_REQ == mac_get_cb_frame_subtype(_pst_tx_ctrl)))
#endif
#define mac_dbdc_send_m2s_action(mac_device, tx_ctl) \
    (mac_is_dbdc_running((mac_device)) && (mac_get_cb_is_smps_frame((tx_ctl)) || mac_get_cb_is_opmode_frame((tx_ctl))))

/* 模块发送流程控制信息结构体的信息元素获取 */
#define mac_get_cb_mpdu_num(_pst_tx_ctrl)            ((_pst_tx_ctrl)->bit_mpdu_num)
#define mac_get_cb_netbuf_num(_pst_tx_ctrl)          ((_pst_tx_ctrl)->bit_netbuf_num)
#define mac_get_cb_frame_header_length(_pst_tx_ctrl) ((_pst_tx_ctrl)->bit_frame_header_length)
#define mac_get_cb_ack_policy(_pst_tx_ctrl)          ((_pst_tx_ctrl)->en_ack_policy)
#define mac_get_cb_tx_vap_index(_pst_tx_ctrl)        ((_pst_tx_ctrl)->bit_tx_vap_index)
#define mac_get_cb_tx_user_idx(_pst_tx_ctrl)         ((_pst_tx_ctrl)->bit_tx_user_idx)
#define mac_get_cb_wme_ac_type(_pst_tx_ctrl)         ((_pst_tx_ctrl)->bit_ac)
#define mac_get_cb_wme_tid_type(_pst_tx_ctrl)        ((_pst_tx_ctrl)->bit_tid)
#define mac_get_cb_event_type(_pst_tx_ctrl)          ((_pst_tx_ctrl)->bit_event_type)
#define mac_get_cb_event_subtype(_pst_tx_ctrl)       ((_pst_tx_ctrl)->bit_event_sub_type)
#define mac_get_cb_retried_num(_pst_tx_ctrl)         ((_pst_tx_ctrl)->bit_retried_num)
#define mac_get_cb_alg_pktno(_pst_tx_ctrl)           ((_pst_tx_ctrl)->uc_alg_pktno)
#define mac_get_cb_is_tcp_ack(_pst_tx_ctrl)          ((_pst_tx_ctrl)->bit_is_tcp_ack)
#define mac_get_cb_is_tcp_data(_pst_tx_ctrl)         ((_pst_tx_ctrl)->bit_is_tcp_data)
#define mac_get_cb_urgent_tcp_ack(_pst_tx_ctrl)      ((_pst_tx_ctrl)->urgent_tcp_ack)
#define mac_get_cb_htc_flag(_pst_tx_ctrl)            ((_pst_tx_ctrl)->bit_htc_flag)
#ifdef _PRE_WLAN_FEATURE_HIEX
#define mac_get_cb_is_himit_frame(_pst_tx_ctrl)      ((_pst_tx_ctrl)->bit_is_himit)
#define mac_get_cb_is_game_marked_frame(_pst_tx_ctrl)((_pst_tx_ctrl)->bit_is_game_marked)
#endif

#define mac_set_cb_is_qos_data(_pst_tx_ctrl, _flag)
#define mac_get_cb_is_qos_data(_pst_tx_ctrl) \
    ((mac_get_cb_wlan_frame_type(_pst_tx_ctrl) == WLAN_DATA_BASICTYPE) &&  \
     ((mac_get_cb_wlan_frame_subtype(_pst_tx_ctrl) == WLAN_QOS_DATA) || \
      (mac_get_cb_wlan_frame_subtype(_pst_tx_ctrl) == WLAN_QOS_NULL_FRAME)))

#define mac_get_cb_is_data_frame(_pst_tx_ctrl)                                \
    (mac_get_cb_is_qos_data(_pst_tx_ctrl) || \
     ((mac_get_cb_wlan_frame_type(_pst_tx_ctrl) == WLAN_DATA_BASICTYPE) &&     \
      (mac_get_cb_wlan_frame_subtype(_pst_tx_ctrl) == WLAN_DATA)))

#ifdef _PRE_WLAN_FEATURE_TXBF_HW
#define mac_frame_type_is_vht_ndpa(_uc_type, _uc_sub_type) \
        ((WLAN_CONTROL == (_uc_type)) && (WLAN_VHT_NDPA == (_uc_sub_type)))
#endif

#if defined(_PRE_PRODUCT_ID_HIMPXX_DEV)
#ifdef _PRE_WLAN_FEATURE_MAC_HEADER_IN_PAYLOAD
#define MAC_CB_FRM_OFFSET (OAL_MAX_CB_LEN + MAX_MAC_HEAD_LEN)
#else
#define MAC_CB_FRM_OFFSET OAL_MAX_CB_LEN
#endif

#define mac_get_cb_seq_num(_pst_tx_ctrl)             \
    (((mac_ieee80211_frame_stru *)((uint8_t *)(_pst_tx_ctrl) + OAL_MAX_CB_LEN))->bit_seq_num)
#define mac_set_cb_80211_mac_head_type(_pst_tx_ctrl, _flag)
#define mac_get_cb_80211_mac_head_type(_pst_tx_ctrl) 1 /* offload架构,MAC HEAD由netbuf index管理,不需要释放 */

#define mac_get_cb_wlan_frame_type(_pst_tx_ctrl) \
    (((mac_ieee80211_frame_stru *)((uint8_t *)(_pst_tx_ctrl) + MAC_CB_FRM_OFFSET))->st_frame_control.bit_type)
#define mac_get_cb_wlan_frame_subtype(_pst_tx_ctrl) \
    (((mac_ieee80211_frame_stru *)((uint8_t *)(_pst_tx_ctrl) + MAC_CB_FRM_OFFSET))->st_frame_control.bit_sub_type)

#define mac_set_cb_frame_header_addr(_pst_tx_ctrl, _addr)
#define mac_get_cb_frame_header_addr(_pst_tx_ctrl) \
    ((mac_ieee80211_frame_stru *)((uint8_t *)(_pst_tx_ctrl) + OAL_MAX_CB_LEN))

#define mac_set_cb_is_bar(_pst_tx_ctrl, _flag)
#define mac_get_cb_is_bar(_pst_tx_ctrl) ((WLAN_CONTROL == mac_get_cb_wlan_frame_type(_pst_tx_ctrl)) && \
        (WLAN_BLOCKACK_REQ == mac_get_cb_wlan_frame_subtype(_pst_tx_ctrl)))

#else
#define mac_get_rx_cb_da_user_idx(_pst_rx_ctl)              ((_pst_rx_ctl)->st_expand_cb.us_da_user_idx)
#define mac_get_cb_wlan_frame_type(_pst_tx_ctrl)            ((_pst_tx_ctrl)->st_expand_cb.en_frame_type)
#define mac_get_cb_wlan_frame_subtype(_pst_tx_ctrl)         \
    (((_pst_tx_ctrl)->st_expand_cb.pst_frame_header)->st_frame_control.bit_sub_type)
#define mac_get_cb_seq_num(_pst_tx_ctrl)                    ((_pst_tx_ctrl)->st_expand_cb.us_seqnum)
#define mac_set_cb_frame_header_addr(_pst_tx_ctrl, _addr)   ((_pst_tx_ctrl)->st_expand_cb.pst_frame_header = (_addr))
#define mac_get_cb_frame_header_addr(_pst_tx_ctrl)          ((_pst_tx_ctrl)->st_expand_cb.pst_frame_header)
#define mac_set_cb_80211_mac_head_type(_pst_tx_ctrl, _flag) \
    ((_pst_tx_ctrl)->st_expand_cb.bit_80211_mac_head_type = (_flag))
#define mac_get_cb_80211_mac_head_type(_pst_tx_ctrl)        ((_pst_tx_ctrl)->st_expand_cb.bit_80211_mac_head_type)
#define mac_get_cb_ether_type(_pst_tx_ctrl)                 ((_pst_tx_ctrl)->st_expand_cb.ether_type)
#define mac_get_cb_is_icmp(_pst_tx_ctrl)                    ((_pst_tx_ctrl)->st_expand_cb.is_icmp)
#define mac_get_cb_ipv6_tcp_ack(_pst_tx_ctrl)               ((_pst_tx_ctrl)->st_expand_cb.ipv6_tcp_ack)
#endif  // #if defined(_PRE_PRODUCT_ID_HIMPXX_DEV)

#define alg_sched_para(tx_ctl) \
    (((mac_get_cb_is_data_frame(tx_ctl)) && (!mac_get_cb_is_vipframe(tx_ctl)) && (!mac_get_cb_is_mcast(tx_ctl))) || \
        (mac_get_cb_is_arp(tx_ctl) && (mac_get_cb_is_mcast(tx_ctl) == OAL_FALSE)))

/*
 * 功能描述  : 根据netbuf获取payload地址
 * 1.日    期  : 2016年10月24日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
static inline uint8_t *mac_get_80211_mgmt_payload(oal_netbuf_stru *netbuf)
{
#if defined(_PRE_PRODUCT_ID_HIMPXX_DEV)
#ifdef _PRE_WLAN_FEATURE_MAC_HEADER_IN_PAYLOAD
    return get_netbuf_payload(netbuf) + MAC_80211_FRAME_LEN;
#else
    return get_netbuf_payload(netbuf);
#endif
#else
    /* 注意! 所以如果mac header长度不是24字节的不要使用该函数 */
    return get_netbuf_payload(netbuf) + MAC_80211_FRAME_LEN;
#endif
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of mac_cb.h */
