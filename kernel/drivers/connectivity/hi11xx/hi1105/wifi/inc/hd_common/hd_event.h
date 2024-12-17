/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明   : 定义HOST/DEVICE传递事件枚举/结构体 的头文件
 * 作者       :
 * 创建日期   : 2020年10月23日
 */

#ifndef HD_EVENT_H
#define HD_EVENT_H

#include "mac_common.h"
#include "wlan_spec.h"
#include "hd_event_vap.h"
#include "hd_event_user.h"
#include "hd_event_psm.h"
#include "hd_event_m2s.h"
#include "hd_event_d2h.h"
#include "hd_event_scan.h"
#include "hd_event_chan_mgmt.h"

typedef struct {
    uint8_t uc_mac_rate; /* MAC对应速率 */
    uint8_t uc_phy_rate; /* PHY对应速率 */
    uint8_t uc_mbps;     /* 速率 */
    uint8_t auc_resv[1];
} mac_data_rate_stru; /* hd_event */

typedef struct {
    uint16_t us_wifi_center_freq;  /* WiFi中心频率 */
    uint16_t us_modem_center_freq; /* Modem中心频率 */
    uint8_t uc_wifi_bw;            /* WiFi带宽 */
    uint8_t uc_modem_bw;           /* Modem带宽 */
    uint8_t uc_wifi_avoid_flag;    /* WiFi当前规避手段 */
    uint8_t uc_wifi_priority;      /* WiFi优先级 */
} mac_nrcoex_stat_stru; /* hd_event */

typedef struct {
    uint8_t auc_mac_addr[OAL_MAC_ADDR_LEN];
    uint8_t auc_rsv[NUM_2_BYTES];
} mac_cfg80211_init_port_stru; /* hd_event */

/* device reset事件同步结构体 */
typedef struct {
    mac_reset_sys_type_enum_uint8 en_reset_sys_type; /* 复位同步类型 */
    uint8_t uc_value;                              /* 同步信息值 */
    uint8_t uc_resv[NUM_2_BYTES];
} mac_reset_sys_stru; /* hd_event */

/* HAL模块和DMAC模块共用的WLAN RX结构体 */
typedef struct {
    oal_netbuf_stru *pst_netbuf; /* netbuf链表一个元素 */
    uint16_t us_netbuf_num;    /* netbuf链表的个数 */
    uint8_t auc_resv[BYTE_OFFSET_2];       /* 字节对齐 */
} hal_cali_hal2hmac_event_stru; /* hd_event */

typedef struct {
    uint32_t packet_idx;
    uint8_t auc_payoald[4];  /* 地址 4 */
} hal_cali_hal2hmac_payload_stru; /* hd_event mp13/mp15 */

/* DMAC与HMAC模块共用的WLAN DRX事件结构体 */
typedef struct {
    oal_netbuf_stru *pst_netbuf; /* netbuf链表一个元素 */
    uint16_t us_netbuf_num;    /* netbuf链表的个数 */
    uint8_t auc_resv[NUM_2_BYTES];       /* 字节对齐 */
} dmac_wlan_drx_event_stru; /* hd_event */

/* DMAC与HMAC模块共用的WLAN CRX事件结构体 */
typedef struct {
    oal_netbuf_stru *pst_netbuf; /* 指向管理帧对应的netbuf */
} dmac_wlan_crx_event_stru; /* hd_event */

/* DMAC与HMAC模块共用的DTX事件结构体 */
typedef struct {
    oal_netbuf_stru *pst_netbuf; /* netbuf链表一个元素 */
    uint16_t us_frame_len;
    uint16_t us_remain;
} dmac_tx_event_stru; /* hd_event */

typedef struct {
    unsigned long *memaddr; /* netbuf链表一个元素 */
    uint32_t us_mem_len;
} dmac_tx_memaddr_stru; /* hd_event */

/* 挂载在dmac_ctx_action_event_stru传递给dmac */
typedef struct {
    uint8_t uc_ac;
    uint8_t bit_psb : 1;
    uint8_t bit_direction : 7;
} dmac_addts_info_stru;

/*
 *  (1)DMAC与HMAC模块共用的CTX子类型ACTION对应的事件的结构体
 *  (2)当DMAC自身产生DELBA帧时，使用该结构体向HMAC模块抛事件
 */
typedef struct {
    mac_category_enum_uint8 en_action_category; /* ACTION帧的类型 */
    uint8_t uc_action;                        /* 不同ACTION类下的子帧类型 */
    uint16_t us_user_idx;
    uint16_t us_frame_len; /* 帧长度 */
    uint8_t uc_hdr_len;    /* 帧头长度 */
    uint8_t uc_tidno;      /* tidno，部分action帧使用 */
    uint8_t uc_initiator;  /* 触发端方向 */

    /* 以下为接收到req帧，发送rsp帧后，需要同步到dmac的内容 */
    uint8_t uc_status;                       /* rsp帧中的状态 */
    uint16_t us_baw_start;                   /* 窗口开始序列号 */
    uint16_t us_baw_size;                    /* 窗口大小 */
    uint8_t uc_ampdu_max_num;                /* BA会话下的最多聚合的AMPDU的个数 */
    oal_bool_enum_uint8 en_amsdu_supp;         /* 是否支持AMSDU */
    uint16_t us_ba_timeout;                  /* BA会话交互超时时间 */
    mac_back_variant_enum_uint8 en_back_var;   /* BA会话的变体 */
    uint8_t uc_dialog_token;                 /* ADDBA交互帧的dialog token */
    uint8_t uc_ba_policy;                    /* Immediate=1 Delayed=0 */
    uint8_t uc_lut_index;                    /* LUT索引 */
    uint8_t auc_mac_addr[WLAN_MAC_ADDR_LEN]; /* 用于DELBA/DELTS查找HMAC用户 */
#ifdef _PRE_WLAN_FEATURE_WMMAC
    uint8_t uc_tsid;            /* TS相关Action帧中的tsid值 */
    uint8_t uc_ts_dialog_token; /* ADDTS交互帧的dialog token */
    dmac_addts_info_stru st_addts_info;
#endif
} dmac_ctx_action_event_stru; /* hd_event */

/* hmac to dmac配置同步消息结构 */
typedef struct {
    wlan_cfgid_enum_uint16 en_syn_id; /* 同步事件ID */
    uint16_t us_len;                /* 事件payload长度 */
    uint8_t auc_msg_body[NUM_4_BYTES];        /* 事件payload */
} hmac_to_dmac_cfg_msg_stru; /* hd_event */
typedef hmac_to_dmac_cfg_msg_stru dmac_to_hmac_cfg_msg_stru; /* hd_event */

typedef struct {
    uint32_t event_para; /* 消息传输的数据 */
    uint32_t fail_num;
    int16_t s_always_rx_rssi;
    uint8_t uc_event_id; /* 消息号 */
    uint8_t uc_reserved;
} dmac_atcmdsrv_atcmd_response_event; /* hd_event */

typedef struct {
    uint8_t flags;
    uint8_t mcs;
    uint16_t legacy;
    uint8_t nss;
    uint8_t bw;
    uint8_t rsv[1];
} tx_rate_info_stru;

typedef struct {
    tx_rate_info_stru tx_rate;
    uint16_t rx_rate_legacy;
    uint16_t trx_duty_ratio;
} txrx_rate_info_stru;

typedef struct {
    uint8_t flags;
    uint8_t mcs;
    uint16_t legacy;
    uint8_t nss;
    uint8_t bw;
    uint8_t beacon_rssi; /* 收到的beacon信号强度 */
    uint8_t rsv[NUM_2_BYTES];
} mac_rate_info_stru; /* hd_event */

typedef struct {
    uint8_t query_event;         /* 消息号 */
    int8_t c_signal;             /* 信号强度 */
    uint16_t uplink_dealy;       /* 上行时延 */
    uint32_t rx_packets;      /* total packets received */
    uint32_t tx_packets;      /* total packets transmitted */
    uint32_t rx_bytes;        /* total bytes received     */
    uint32_t tx_bytes;        /* total bytes transmitted  */
    uint32_t tx_retries;      /* 发送重传次数 */
    uint32_t rx_dropped_misc; /* 接收失败次数 */
    uint32_t tx_failed;       /* 发送失败次数 */
    int16_t s_free_power;        /* 底噪 */
    uint16_t s_chload;           /* 信道繁忙程度 */
    // wifi：此成员如果放在结构体的最后面，导致本地UT工程一直编译不过，提示找不到此成员，交换位置后解决！！困惑ing
    station_info_extend_stru st_station_info_extend;
    union {
        mac_rate_info_stru  tx_rate;   /* ROM部分使用，不可更改，新增函数使用txrx_rate */
        txrx_rate_info_stru txrx_rate;
    } rate_info;
} dmac_query_station_info_response_event; /* hd_event */

typedef struct {
    station_info_extend_2_stru station_info_extend2;
} dmac_query_station_info_additional_response_event; /* hd_event */

typedef struct {
    uint32_t uc_dscr_status;
    uint8_t mgmt_frame_id;
    uint8_t auc_resv[1];
    uint16_t us_user_idx;
} dmac_crx_mgmt_tx_status_stru; /* hd_event */

typedef struct {
    uint16_t user_id;
    uint8_t ac_idx;
    uint8_t need_degrade;
} dmac_wmmac_info_sync_stru; /* hd_event */

#ifdef _PRE_WLAN_FEATURE_TAS_ANT_SWITCH
typedef struct {
    int32_t l_core_idx;
    int32_t l_rssi;
    int32_t aul_rsv[NUM_4_BYTES];
} dmac_tas_rssi_notify_stru; /* hd_event */

typedef struct {
    uint8_t uc_core_idx;
    oal_bool_enum_uint8 en_need_improved;
    uint8_t auc_rev[NUM_2_BYTES];
} mac_cfg_tas_pwr_ctrl_stru; /* hd_event */
#endif

#if defined(_PRE_WLAN_FEATURE_DATA_SAMPLE) || defined(_PRE_WLAN_FEATURE_PSD_ANALYSIS)
typedef struct {
    uint32_t type;
    uint32_t reg_num;
    uint32_t count;
    uint32_t event_data;
} dmac_sdt_sample_frame_stru; /* hd_event */
#endif

#ifdef _PRE_WLAN_FEATURE_APF
typedef enum {
    APF_SET_FILTER_CMD,
    APF_GET_FILTER_CMD
} mac_apf_cmd_type_enum;
typedef uint8_t mac_apf_cmd_type_uint8;

typedef struct {
    void *p_program; /* netbuf链表一个元素 */
    uint16_t us_program_len;
} dmac_apf_report_event_stru; /* hd_event */
#endif

typedef struct {
    uint8_t uc_cac_machw_en; /* 1min cac 发送队列开关状态 */
} dmac_set_cac_machw_info_stru; /* hd_event */

#ifdef _PRE_WLAN_FEATURE_TWT
typedef struct {
    mac_category_enum_uint8 en_action_category; /* ACTION帧的类型 */
    uint8_t uc_action;                        /* 不同ACTION类下的子帧类型 */
    uint16_t us_user_idx;
    mac_cfg_twt_stru st_twt_cfg;
    uint8_t twt_idx; /* 在host占用第几个资源 */
    uint8_t resv[NUM_3_BYTES];
} dmac_ctx_update_twt_stru; /* hd_event */
#endif

/* Update join req 需要配置的速率集参数 */
typedef struct {
    union {
        uint8_t uc_value;
        struct {
            uint8_t bit_support_11b : 1;  /* 该AP是否支持11b */
            uint8_t bit_support_11ag : 1; /* 该AP是否支持11ag */
            uint8_t bit_ht_capable : 1;   /* 是否支持ht */
            uint8_t bit_vht_capable : 1;  /* 是否支持vht */
            uint8_t bit_reserved : 4;
        } st_capable; /* 与dmac层wlan_phy_protocol_enum对应 */
    } un_capable_flag;
    uint8_t uc_min_rate[NUM_2_BYTES]; /* 第一个存储11b协议对应的速率，第二个存储11ag协议对应的速率 */
    uint8_t uc_reserved;
} dmac_set_rate_stru;

typedef struct {
    uint8_t auc_transmitted_bssid[WLAN_MAC_ADDR_LEN]; /* transmitted bssid */
    uint8_t bit_is_non_transimitted_bss : 1,
              bit_ext_cap_multi_bssid_activated : 1,
              bit_rsv : 6;
    uint8_t auc_rcv[1];
} mac_multi_bssid_info; /* hd_event */

/* Update join req 参数写寄存器的结构体定义 */
typedef struct {
    uint8_t auc_bssid[WLAN_MAC_ADDR_LEN]; /* 加入的AP的BSSID  */
    uint16_t us_beacon_period;
    mac_channel_stru st_current_channel;                      /* 要切换的信道信息 */
    uint32_t beacon_filter;                              /* 过滤beacon帧的滤波器开启标识位 */
    uint32_t non_frame_filter;                           /* 过滤no_frame帧的滤波器开启标识位 */
    uint8_t auc_ssid[WLAN_SSID_MAX_LEN];                    /* 加入的AP的SSID  */
    uint8_t uc_dtim_period;                                 /* dtim period      */
    oal_bool_enum_uint8 en_dot11FortyMHzOperationImplemented; /* dot11FortyMHzOperationImplemented */
    uint8_t set_he_cap : 1,
            set_vht_cap : 1,
              bit_rsv : 6;
    dmac_set_rate_stru st_min_rate; /* Update join req 需要配置的速率集参数 */
    mac_multi_bssid_info st_mbssid_info;
    mac_ap_type_enum_uint16 en_ap_type;
    mac_sap_mode_enum_uint8 sap_mode;
    uint8_t ap_support_max_nss;
} dmac_ctx_join_req_set_reg_stru; /* hd_event */

/* wait join写寄存器参数的结构体定义 */
typedef struct {
    uint32_t dtim_period;              /* dtim period */
    uint32_t dtim_cnt;                 /* dtim count  */
    uint8_t auc_bssid[WLAN_MAC_ADDR_LEN]; /* 加入的AP的BSSID  */
    uint16_t us_tsf_bit0;                 /* tsf bit0  */
} dmac_ctx_set_dtim_tsf_reg_stru; /* hd_event */

/* wait join写寄存器参数的结构体定义 */
typedef struct {
    uint16_t uc_user_index; /* user index */
    uint8_t auc_resv[NUM_2_BYTES];
} dmac_ctx_asoc_set_reg_stru; /* hd_event */

/* sta更新edca参数寄存器的结构体定义 */
typedef struct {
    uint8_t uc_vap_id;
    mac_wmm_set_param_type_enum_uint8 en_set_param_type;
    uint16_t us_user_index;
    wlan_mib_Dot11QAPEDCAEntry_stru ast_wlan_mib_qap_edac[WLAN_WME_AC_BUTT];
} dmac_ctx_sta_asoc_set_edca_reg_stru; /* hd_event */
/* sta更新edca参数寄存器的结构体定义 */
/* sta更新sr参数寄存器的结构体定义 */
#ifdef _PRE_WLAN_FEATURE_11AX
typedef struct {
    mac_frame_he_spatial_reuse_parameter_set_ie_stru st_sr_ie;
} dmac_ctx_sta_asoc_set_sr_reg_stru; /* hd_event */
#endif
#ifdef _PRE_WLAN_FEATURE_LP_MIRACAST
typedef struct {
    uint8_t abort_switch;
    uint8_t bt_forbid_switch;
    uint8_t device_allow_m2s_siso;
} dmac_miracast_report_stru; /* hd_event */
#endif

typedef struct {
    uint8_t query_event;
    uint8_t auc_query_sta_addr[WLAN_MAC_ADDR_LEN];
} dmac_query_request_event; /* host only */

typedef struct {
    uint8_t query_event;
    uint8_t auc_query_sta_addr[WLAN_MAC_ADDR_LEN]; /* sta mac地址 */
} dmac_query_station_info_request_event; /* hd_event */

/* TID事件用于更新指针和调度信息 */
typedef struct {
    tid_cmd_enum_uint8 cmd;        /* 0-creat tid; 1-del tid; 2-tid enqueue; 3-tid dequeue; */
    uint16_t user_id;
    uint8_t tid_no;                /* TID属性，no大于64时，说明是建立了ba会话 */
    uint8_t update_wptr;
    uint32_t cur_msdus;            /* TID状态，队列中的msdu个数 */
    uint32_t cur_bytes;            /* TID状态，队列中的字节数 */
    msdu_info_ring_stru msdu_ring; /* TID属性，msdu_info_ring参数 */
    uint32_t timestamp_us;         /* MSG属性，此MSG生成的时间搓 */
} tid_update_stru; /* hd_event */

typedef enum {
    DMAC_CONFIG_IPV4 = 0, /* 配置IPv4地址 */
    DMAC_CONFIG_IPV6,     /* 配置IPv6地址 */
    DMAC_CONFIG_BUTT
} dmac_ip_type;
typedef uint8_t dmac_ip_type_enum_uint8;

typedef enum {
    DMAC_IP_ADDR_ADD = 0, /* 增加 */
    DMAC_IP_ADDR_DEL,     /* 删除 */
    DMAC_IP_OPER_BUTT
} dmac_ip_oper;
typedef uint8_t dmac_ip_oper_enum_uint8;

typedef struct {
    dmac_ip_type_enum_uint8 en_type;
    dmac_ip_oper_enum_uint8 en_oper;

    uint8_t auc_resv[NUM_2_BYTES];

    uint8_t auc_ip_addr[OAL_IP_ADDR_MAX_SIZE];
    uint8_t auc_mask_addr[OAL_IP_ADDR_MAX_SIZE];
} dmac_ip_addr_config_stru; /* hd_event */

typedef struct {
    mac_cfg_cali_hdr_stru cali_cfg_hdr;

    char input_args[WLAN_CALI_CFG_MAX_ARGS_NUM][WLAN_CALI_CFG_CMD_MAX_LEN];
    uint8_t input_args_num;
    uint8_t resv1[NUM_3_BYTES]; /* 4 字节对齐 */

    int32_t al_output_param[WLAN_CFG_MAX_ARGS_NUM];
    uint8_t uc_output_param_num;
    uint8_t resv2[NUM_3_BYTES]; /* 4 字节对齐 */
} mac_cfg_cali_param_stru; /* hd_event */

typedef struct {
    uint8_t uc_cfg_btcoex_mode; /* 0:参数查询模式; 1:参数配置模式 */
    uint8_t uc_cfg_btcoex_type; /* 0:门限类型; 1:聚合大小类型 2.rssi detect门限参数配置模式 */
    union {
        struct {
            wlan_nss_enum_uint8 en_btcoex_nss; /* 0:siso 1:mimo */
            uint8_t uc_20m_low;              /* 2G 20M low */
            uint8_t uc_20m_high;             /* 2G 20M high */
            uint8_t uc_40m_low;              /* 2G 40M low */
            uint16_t us_40m_high;            /* 2G 40M high */
        } threhold;
        struct {
            wlan_nss_enum_uint8 en_btcoex_nss; /* 0:siso 1:mimo */
            uint8_t uc_grade;                /* 0级或者1级 */
            uint8_t uc_rx_size0;             /* size0大小 */
            uint8_t uc_rx_size1;             /* size1大小 */
            uint8_t uc_rx_size2;             /* size2大小 */
            uint8_t uc_rx_size3;             /* size3大小 */
        } rx_size;
        struct {
            oal_bool_enum_uint8 en_rssi_limit_on;
            oal_bool_enum_uint8 en_rssi_log_on;
            uint8_t uc_cfg_rssi_detect_cnt; /* 6 rssi 配置时用于防护次数  高低门限配置 */
            int8_t c_cfg_rssi_detect_mcm_down_th; /* m2s */
            int8_t c_cfg_rssi_detect_mcm_up_th; /* s2m */
        } rssi_param;
    } pri_data;
} mac_btcoex_mgr_stru; /* hd_event */

typedef struct {
    /* 0:硬件preempt; 1:软件preempt 2:ps 提前slot量 */
    uint8_t uc_cfg_preempt_mode;
    /* 0 noframe 1 self-cts 2 nulldata 3 qosnull  0/1 软件ps打开或者关闭 */
    uint8_t uc_cfg_preempt_type;
} mac_btcoex_preempt_mgr_stru; /* hd_event */

/* 配置命令格式 */
typedef struct {
    uint8_t uc_item_count;
    oal_bool_enum_uint8 en_enable; /* 下发功能使能标志 */
    mac_ip_filter_cmd_enum_uint8 en_cmd;
    uint8_t uc_resv;
    mac_ip_filter_item_stru ast_filter_items[1];
} mac_ip_filter_cmd_stru; /* hd_event */

#ifdef _PRE_WLAN_FEATURE_NEGTIVE_DET
typedef struct {
    oal_bool_enum_uint8 en_is_pk_mode;
    wlan_bw_cap_enum_uint8 en_curr_bw_cap;             /* 目前使用的带宽，本参数仅在lagency sta模式下生效 */
    wlan_protocol_cap_enum_uint8 en_curr_protocol_cap; /* 目前使用的协议模式，本参数仅在lagency sta模式下生效 */
    wlan_nss_enum_uint8 en_curr_num_spatial_stream;    /* 目前单双流的计数 */
    uint8_t pk_cnt; // 连续周期计数低于门限，允许退出pk mode
    uint8_t rsv[3]; // reserve 3

    uint32_t tx_bytes; /* WIFI 业务发送帧统计 */
    uint32_t rx_bytes; /* WIFI 业务接收帧统计 */
    uint32_t dur_time; /* 统计时间间隔 */
} mac_cfg_pk_mode_stru; /* hd_event */
#endif

/* 漫游切换状态 */
typedef enum {
    WLAN_ROAM_MAIN_BAND_STATE_2TO2 = 0,
    WLAN_ROAM_MAIN_BAND_STATE_5TO2 = 1,
    WLAN_ROAM_MAIN_BAND_STATE_2TO5 = 2,
    WLAN_ROAM_MAIN_BAND_STATE_5TO5 = 3,

    WLAN_ROAM_MAIN_BAND_STATE_BUTT
} wlan_roam_main_band_state_enum;
typedef uint8_t wlan_roam_main_band_state_enum_uint8; /* hd_event */

/* roam trigger 数据结构体 */
typedef struct {
    int8_t trigger_2g;
    int8_t trigger_5g;
    uint16_t us_roam_interval;
} mac_roam_trigger_stru; /* hd_event */

/* roam hmac 同步 dmac数据结构体 */
typedef struct {
    uint16_t us_sta_aid;
    uint16_t us_pad;
    mac_channel_stru st_channel;
    mac_user_cap_info_stru st_cap_info;
    mac_hisi_cap_vendor_ie_stru hisi_priv_cap;  /* 用户私有能力 */
    mac_key_mgmt_stru st_key_info;
    mac_user_tx_param_stru st_user_tx_info;
    uint32_t back_to_old;
} mac_h2d_roam_sync_stru; /* hd_event */

/* 校准数据传递子状态 */
enum cali_data_trans_sub_type {
    CALI_DATA_NORMAL_JOIN_TYPE = 0,
    CALI_DATA_CSA_TYPE  = 1,
};
typedef struct {
    mac_channel_stru channel;
    uint8_t mac_vap_id;
    uint8_t work_cali_data_sub_type;
} cali_data_req_stru;

typedef struct {
    uint8_t mac_vap_id;
    uint8_t band;
    uint8_t bandwidth;
    uint8_t subband_idx;
    uint16_t center_freq;
    uint16_t trans_id;
    int32_t curr_send_data_len;
    int32_t cali_data_total_len;
    uint8_t cali_data_type;
    uint8_t cali_data_subtype; // 1、正常入网(可覆盖原校准数据) 2、CSA(不能覆盖原校准数据)
    uint8_t netbuf_idx;
} h2d_cali_trans_stru;

typedef struct {
    uint8_t hal_dev_id;
    uint8_t resv;
    uint16_t size;
    uint32_t lsb;
    uint32_t msb;
} host_ba_ring_info_sync_stru;

#endif /* end of hd_event.h */
