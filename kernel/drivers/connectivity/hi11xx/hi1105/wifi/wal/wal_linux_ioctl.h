/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : wal_linux_flowctl.c 的头文件
 * 作    者 :
 * 创建日期 : 2012年12月10日
 */

#ifndef WAL_LINUX_IOCTL_H
#define WAL_LINUX_IOCTL_H
#include "wal_linux_ioctl_macro.h"
#include "oal_types.h"
#include "hmac_vap.h"
#include "oal_util.h"
#include "oal_ext_if.h"
#include "wlan_types.h"
#include "wlan_spec.h"
#include "mac_vap.h"
#include "hmac_ext_if.h"
#include "wal_ext_if.h"
#include "wal_config.h"
#include "alg_cfg.h"
#include "wal_linux_vendor.h"
#include "plat_pm_wlan.h"
#include "hmac_resource.h"
#include "plat_firmware.h"
#ifdef _PRE_WLAN_FEATURE_PMF
#include "hmac_11w.h"
#endif
#include "hmac_config.h"
#include "hmac_auto_adjust_freq.h"
#ifdef CONFIG_DOZE_FILTER
#include <huawei_platform/power/wifi_filter/wifi_filter.h>
#endif /* CONFIG_DOZE_FILTER */
#ifdef _PRE_WLAN_FEATURE_TAS_ANT_SWITCH
#include "hisi_customize_wifi.h"
#endif


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

extern oal_iw_handler_def_stru g_st_iw_handler_def;
extern oal_net_device_ops_stru g_st_wal_net_dev_ops;
extern oal_ethtool_ops_stru g_st_wal_ethtool_ops;
extern oal_wait_queue_head_stru g_wlan_cali_complete_wq;
/* mpxx 用于将上层下发字符串命令序列化 */
typedef struct {
    uint8_t     *pc_priv_cmd;
    uint32_t     case_entry;
} wal_ioctl_priv_cmd_stru;
/* 私有命令入口结构定义 */
typedef struct {
    int8_t *pc_cmd_name;      /* 命令字符串 */
    wal_hipriv_cmd_func p_func; /* 命令对应处理函数 */
} wal_hipriv_cmd_entry_stru;

/* VoWiFi命令的转换结构体 */
typedef struct {
    int8_t *pc_vowifi_cmd_name;            /* 命令字符串 */
    mac_vowifi_cmd_enum_uint8 en_vowifi_cmd; /* 命令对应类型 */
    uint8_t auc_resv[3]; /* 3实现字节对齐 */
} wal_vowifi_cmd_stru;

/* 协议模式与字符串映射 */
typedef struct {
    int8_t *pc_name;                              /* 模式名字符串 */
    wlan_protocol_enum_uint8 en_mode;               /* 协议模式 */
    wlan_channel_band_enum_uint8 en_band;           /* 频段 */
    wlan_channel_bandwidth_enum_uint8 en_bandwidth; /* 带宽 */
    uint8_t auc_resv[1];
} wal_ioctl_mode_map_stru;

/* 参数配置表结构体 */
typedef struct {
    int8_t *pc_name;                            /* 配置命令字符串 */
    uint8_t idx;                                /* 配置命令对应的枚举值 */
    uint8_t auc_resv[3];                        /* 3实现字节对齐 */
} wal_ioctl_cmd_tbl_stru;

/* 算法参数配置结构体 */
typedef struct {
    int8_t *pc_name;                 /* 配置命令字符串 */
    mac_alg_cfg_param_enum_uint8 en_alg_cfg; /* 配置命令对应的枚举值 */
    uint8_t auc_resv[3];             /* 3实现字节对齐 */
} wal_ioctl_alg_cfg_stru;

#ifdef _PRE_WLAN_RR_PERFORMANCE
typedef struct wal_wifi_pc_cmd {
    int32_t   l_verify;    /* 校验位 */
    uint32_t  cmd;      /* 命令号 */
} wal_wifi_pc_cmd_stru;
#endif /* _PRE_WLAN_RR_PERFORMANCE */
typedef struct {
    int8_t *pc_name;                    /* 配置命令字符串 */
    wlan_cfgid_enum_uint16 en_tlv_cfg_id; /* 配置命令对应的枚举值 */
    uint8_t auc_resv[2];                /* 2实现字节对齐 */
} wal_ioctl_tlv_stru;

typedef struct {
    int8_t *pc_name;                    /* 配置命令字符串 */
    wlan_cfgid_enum_uint16 en_tlv_cfg_id; /* 配置命令对应的枚举值 */
    wal_ioctl_tlv_stru *pst_cfg_table;
} wal_ioctl_str_stru;
typedef struct {
    uint32_t ap_max_user;           /* ap最大用户数 */
    int8_t ac_ap_mac_filter_mode[257]; /* 257代表AP mac地址过滤命令参数,最长256 */
} wal_ap_config_stru;

typedef struct {
    int8_t *pc_country;                    /* 国家字符串 */
    mac_dfs_domain_enum_uint8 en_dfs_domain; /* DFS 雷达标准 */
} wal_dfs_domain_entry_stru;
/* 驱动配置命令对外公共入参数据结构 */
typedef struct {
    oal_net_device_stru   *pst_net_dev;
    wlan_cfgid_enum_uint16 en_wlan_cfg_id;

    /* 入参字符指针数组，按序指向每个参数字符串首地址 */
    uint8_t              uc_args_num; /* 入参个数 */
    char              *apc_args[WLAN_CFG_MAX_ARGS_NUM];

    /* 驱动自定义处理返回字符串填写处 */
    char              *pc_output;
    uint16_t             us_output_len;

    uint32_t             process_ret;
} wlan_cfg_param_stru;

/* AUTORATE LOG 算法参数枚举，参数值 */
typedef struct {
    mac_alg_cfg_param_enum_uint8 en_alg_cfg;         /* 配置命令枚举 */
    uint8_t auc_mac_addr[WLAN_MAC_ADDR_LEN]; /*  */
    uint8_t uc_ac_no;                        /* AC类型 */
    uint8_t auc_resv[NUM_2_BYTES];
    uint16_t us_value; /* 配置参数值 */
} wal_ioctl_alg_ar_log_param_stru;

/* TXMODE LOG 算法参数枚举，参数值 */
typedef struct {
    mac_alg_cfg_param_enum_uint8 en_alg_cfg;         /* 配置命令枚举 */
    uint8_t uc_ac_no;                        /* AC类型 */
    uint8_t auc_mac_addr[WLAN_MAC_ADDR_LEN]; /*  */
    uint8_t auc_resv1[NUM_2_BYTES];
    uint16_t us_value; /* 配置参数值 */
} wal_ioctl_alg_txbf_log_param_stru;

/* TPC LOG 算法参数枚举，参数值 */
typedef struct {
    mac_alg_cfg_param_enum_uint8 en_alg_cfg;         /* 配置命令枚举 */
    uint8_t auc_mac_addr[WLAN_MAC_ADDR_LEN]; /*  */
    uint8_t uc_ac_no;                        /* AC类型 */
    uint16_t us_value;                       /* 配置参数值 */
    int8_t *pc_frame_name;                   /* 获取特定帧功率使用该变量 */
} wal_ioctl_alg_tpc_log_param_stru;

/* cca opt LOG 算法参数枚举，参数值 */
typedef struct {
    mac_alg_cfg_param_enum_uint8 en_alg_cfg; /* 配置命令枚举 */
    uint16_t us_value;               /* 统计总时间 */
    uint8_t auc_resv;
} wal_ioctl_alg_intfdet_log_param_stru;

/* HiD2D 命令配置结构体 */
typedef struct {
    uint8_t hid2d_cfg_type;  /* 配置命令类型 hmac_hid2d_cmd_enum */
    wal_hid2d_vendor_cmd_func func;           /* 命令对应处理函数 */
} wal_hid2d_cmd_entry_stru;

typedef struct {
    int8_t *pc_cmd_name;          /* 命令字符串 */
    uint8_t uc_is_check_para;     /* 是否检查获取的参数 */
    uint32_t bit;              /* 需要置位的命令 */
} wal_ftm_cmd_entry_stru; // only host

#ifdef WIFI_DEBUG_ENABLE
extern const wal_hipriv_cmd_entry_stru g_ast_hipriv_cmd_debug[];
#endif
extern wal_ap_config_stru g_st_ap_config_info;
extern const wal_ioctl_str_stru g_ast_set_str_table[];
extern const uint32_t g_set_str_table_size;
extern const int8_t *g_pauc_bw_tbl[WLAN_BANDWITH_CAP_BUTT];
extern const int8_t *g_pauc_tx_dscr_param_name[WAL_DSCR_PARAM_BUTT];
extern const int8_t *g_pauc_tx_pow_param_name[WAL_TX_POW_PARAM_BUTT];

extern const wal_ioctl_cmd_tbl_stru g_ast_dyn_cali_cfg_map[];
extern const uint32_t g_dyn_cali_cfg_map_size;
extern const wal_ioctl_tlv_stru g_ast_set_tlv_table[];
extern const uint32_t g_set_tlv_table_size;

typedef uint32_t (*wal_hipriv_m2s_switch_func)(mac_m2s_mgr_stru *m2s_mgr, int8_t *param,
    uint32_t *cmd_off_set, uint8_t *name);
static inline oal_bool_enum_uint8 wal_ioctl_is_invalid_fixed_rate(int32_t val, mac_cfg_set_dscr_param_stru *stu)
{
    return (((val == WAL_HIPRIV_RATE_INVALID) &&
            (stu->uc_function_index >= WAL_DSCR_PARAM_RATE) &&
            (stu->uc_function_index <= WAL_DSCR_PARAM_MCSAC)) ||
            (val == 0 && (stu->uc_function_index == WAL_DSCR_PARAM_RATE)));
}
/* 8 UNION定义 */
/* 9 OTHERS定义 */
/* 10 函数声明 */
uint32_t wal_hipriv_ampdu_tx_on(oal_net_device_stru *pst_net_dev, int8_t *pc_param);
uint32_t wal_hipriv_amsdu_tx_on(oal_net_device_stru *pst_net_dev, int8_t *pc_param);
uint32_t wal_hipriv_set_rate(oal_net_device_stru *pst_net_dev, int8_t *pc_param);
uint32_t wal_hipriv_set_mcs(oal_net_device_stru *pst_net_dev, int8_t *pc_param);
uint32_t wal_hipriv_set_mcsac(oal_net_device_stru *pst_net_dev, int8_t *pc_param);
#ifdef _PRE_WLAN_FEATURE_11AX
uint32_t wal_hipriv_set_mcsax(oal_net_device_stru *pst_net_dev, int8_t *pc_param);
#ifdef  _PRE_WLAN_FEATURE_11AX_ER_SU_DCM
uint32_t _wal_hipriv_set_mcsax_er(oal_net_device_stru *pst_net_dev, int8_t *pc_param);
#endif
#endif
uint32_t wal_hipriv_vap_info(oal_net_device_stru *pst_net_dev, int8_t *pc_param);
uint32_t wal_hipriv_set_ip_filter(oal_net_device_stru *pst_net_dev, int8_t *pc_param);
uint32_t wal_hipriv_create_proc(void *p_proc_arg);
uint32_t wal_hipriv_del_vap(oal_net_device_stru *pst_net_dev, int8_t *pc_param);
uint32_t wal_hipriv_remove_proc(void);
uint32_t wal_alloc_cfg_event(oal_net_device_stru *pst_net_dev, frw_event_mem_stru **ppst_event_mem,
                             const void *pst_resp_addr, wal_msg_stru **ppst_cfg_msg, uint16_t us_len);
int32_t wal_send_cfg_event(oal_net_device_stru *pst_net_dev, uint8_t en_msg_type, uint16_t us_len, uint8_t *puc_param,
    oal_bool_enum_uint8 en_need_rsp, wal_msg_stru **ppst_rsp_msg);
uint32_t wal_hipriv_process_rate_params(oal_net_device_stru *pst_net_dev, int8_t *ac_arg, uint32_t arg_len,
    mac_cfg_set_dscr_param_stru *pc_stu);
int32_t wal_start_vap(oal_net_device_stru *pst_net_dev);
int32_t wal_stop_vap(oal_net_device_stru *pst_net_dev);
int32_t wal_init_wlan_vap(oal_net_device_stru *pst_net_dev);
#ifdef _PRE_WLAN_CHBA_MGMT
int32_t wal_setup_chba(oal_net_device_stru *net_dev);
int32_t wal_init_chba_netdev(oal_wiphy_stru *pst_wiphy, const char *dev_name);
int32_t wal_ioctl_chba_channel_adjust(oal_net_device_stru *net_dev, uint8_t *command,
    wal_wifi_priv_cmd_stru priv_cmd);
int32_t wal_ioctl_set_battery_level(oal_net_device_stru *net_dev, int8_t *command,
    wal_wifi_priv_cmd_stru priv_cmd);
int32_t wal_ioctl_chba_module_init(oal_net_device_stru *net_dev, int8_t *command, wal_wifi_priv_cmd_stru priv_cmd);
#endif
int32_t wal_deinit_wlan_vap(oal_net_device_stru *pst_net_dev);
int32_t wal_init_wlan_netdev(oal_wiphy_stru *pst_wiphy, const char *dev_name);
void wal_init_netdev_priv(oal_net_device_stru *net_dev);
int32_t wal_setup_ap(oal_net_device_stru *pst_net_dev);
int32_t wal_host_dev_init(oal_net_device_stru *pst_net_dev);
#ifdef _PRE_WLAN_FEATURE_NAN
void wal_init_nan_netdev(oal_wiphy_stru *wiphy);
#endif
int32_t wal_ioctl_judge_input_param_length(wal_wifi_priv_cmd_stru priv_cmd, uint32_t cmd_length, uint16_t adjust_val);
#ifdef _PRE_WLAN_FEATURE_11D
uint32_t wal_regdomain_update_sta(uint8_t uc_vap_id);
int32_t wal_regdomain_update_country_code(oal_net_device_stru *pst_net_dev, int8_t *pc_country);
#ifdef _PRE_WLAN_COUNTRYCODE_SELFSTUDY
int32_t wal_regdomain_update_selfstudy_country_code(oal_net_device_stru *pst_net_dev, int8_t *pc_country);
void wal_selfstudy_regdomain_update_by_ap(frw_event_stru *pst_event);
#endif
#endif

int32_t wal_set_random_mac_to_mib(oal_net_device_stru *pst_net_dev);
wlan_p2p_mode_enum_uint8 wal_wireless_iftype_to_mac_p2p_mode(enum nl80211_iftype en_iftype);
uint32_t wal_hipriv_arp_offload_enable(oal_net_device_stru *pst_net_dev, int8_t *pc_param);
void wal_set_wdev_iftype(oal_wireless_dev_stru *wdev, wlan_vap_mode_enum_uint8 vap_mode,
    wlan_p2p_mode_enum_uint8 p2p_mode);
int32_t wal_cfg_vap_h2d_event(oal_net_device_stru *pst_net_dev);

int32_t wal_set_custom_process_func(void);
uint32_t wal_custom_cali(void);
void hwifi_config_init_force(void);
void hwifi_config_host_global_ini_param(void);
oal_bool_enum_uint8 wal_sniffer_is_single_sta_mode(oal_net_device_stru *net_dev);
uint32_t wal_hipriv_set_essid(oal_net_device_stru *pst_net_dev, int8_t *pc_param);
uint32_t wal_hipriv_set_mcs_by_cfgid(oal_net_device_stru *net_dev, int8_t *pc_param, wlan_cfgid_enum_uint16 cfgid,
    int32_t mcs_min, int32_t mcs_max);
uint32_t wal_get_cmd_one_arg(int8_t *pc_cmd, int8_t *pc_arg, uint32_t arg_len, uint32_t *pul_cmd_offset);
uint32_t wal_hipriv_send_cfg_uint32_data(oal_net_device_stru *net_dev, int8_t *pc_param, uint8_t param_len,
    wlan_cfgid_enum_uint16 cfgid);
#ifdef WIFI_DEBUG_ENABLE
uint32_t wal_hipriv_get_debug_cmd_size(void);
#endif
uint32_t wal_hipriv_alg_cfg(oal_net_device_stru *pst_net_dev, int8_t *pc_param);
uint32_t wal_hipriv_alg_cfg_entry(oal_net_device_stru *net_dev, int8_t *param);

uint32_t wal_hipriv_parse_cmd(int8_t *pc_cmd);
int32_t wal_set_assigned_filter_enable(int32_t l_filter_id, int32_t l_on);
int32_t wal_set_ip_filter_enable(int32_t l_on);
int32_t wal_add_ip_filter_items(wal_hw_wifi_filter_item *pst_items, int32_t l_count);
int32_t wal_clear_ip_filter(void);
#ifdef _PRE_WLAN_FEATURE_M2S
uint32_t wal_hipriv_set_m2s_switch(oal_net_device_stru *pst_net_dev, int8_t *pc_param);
#endif
uint8_t wal_hipriv_m2s_switch_get_mcm_chain(uint16_t param);
int32_t wal_register_ip_filter(wal_hw_wlan_filter_ops *pg_st_ip_filter_ops);
int32_t wal_unregister_ip_filter(void);

uint32_t wal_hipriv_dyn_cali_cfg(oal_net_device_stru *pst_net_dev, int8_t *puc_param);
void wal_set_netdev_priv(oal_netdev_priv_stru *pst_netdev_priv);
uint32_t wal_ioctl_set_ap_mode(oal_net_device_stru *pst_net_dev, uint8_t uc_param);
uint32_t wal_hipriv_set_mcast_data_dscr_param(oal_net_device_stru *pst_net_dev, int8_t *pc_param);
int32_t wal_ioctl_get_psm_stat(oal_net_device_stru *pst_net_dev, mac_psm_query_type_enum_uint8 en_query_type,
    wal_wifi_priv_cmd_stru *priv_cmd);
uint32_t wal_hipriv_sta_pm_on(oal_net_device_stru *pst_cfg_net_dev, int8_t *pc_param);
uint32_t wal_hipriv_set_tlv_cmd(oal_net_device_stru *pst_net_dev, int8_t *pc_param);
uint32_t wal_hipriv_set_val_cmd(oal_net_device_stru *pst_net_dev, int8_t *pc_param);
void wal_vap_get_wlan_mode_para(wal_msg_write_stru *st_write_msg);
int32_t wal_host_dev_exit(oal_net_device_stru *pst_net_dev);
uint32_t wal_hipriv_setcountry(oal_net_device_stru *pst_net_dev, int8_t *pc_param);
uint32_t wal_ioctl_set_sta_ps_mode(oal_net_device_stru *pst_cfg_net_dev, int8_t uc_ps_mode);
uint32_t wal_ioctl_set_fast_sleep_para(oal_net_device_stru *pst_cfg_net_dev, int8_t *pc_param, uint8_t len);
void wal_set_power_mgmt_on(uint8_t power_mgmt_flag);
int32_t wal_set_power_on(oal_net_device_stru *pst_net_dev, int32_t power_flag);
int32_t wal_ext_priv_cmd(oal_net_device_stru *pst_net_dev, oal_ifreq_stru *pst_ifr);
#ifdef _PRE_WLAN_FEATURE_CSI
uint32_t wal_ioctl_set_csi_switch(oal_net_device_stru *pst_cfg_net_dev, int8_t *pc_param);
#endif
uint32_t wal_ioctl_set_mode(oal_net_device_stru *net_dev, int8_t *pc_param);
uint32_t wal_ioctl_set_freq(oal_net_device_stru *net_dev, int8_t *pc_param);
#ifdef _PRE_WLAN_FEATURE_SNIFFER
uint32_t wal_ioctl_set_sniffer(oal_net_device_stru *net_dev, int8_t *pc_param);
#endif
uint32_t wal_ioctl_set_monitor_mode(oal_net_device_stru *net_dev, int8_t *pc_param);
uint32_t wal_ioctl_set_al_rx(oal_net_device_stru *net_dev, int8_t *pc_param);
uint32_t wal_ioctl_set_pm_switch(mac_vap_stru *mac_vap, uint32_t *params);
int32_t wal_ioctl_priv_cmd_ext(oal_net_device_stru *pst_net_dev, oal_ifreq_stru *p_ifr,
    uint8_t *pc_cmd, wal_wifi_priv_cmd_stru *priv_cmd);
uint32_t wal_algcmd_char_extra_adapt(oal_net_device_stru *net_dev,
    const char *in_param, signed long in_len, char *out_param, signed long out_len);
uint32_t wal_hipriv_fem_lowpower(oal_net_device_stru *pst_net_dev, int8_t *pc_param);
uint8_t wal_cfg80211_convert_value_to_vht_width(int32_t l_channel_value);
uint32_t wal_hipriv_set_rfch(oal_net_device_stru *net_dev, int8_t *pc_param);
uint32_t wal_hipriv_set_rxch(oal_net_device_stru *net_dev, int8_t *pc_param);
uint32_t wal_hipriv_set_nss(oal_net_device_stru *net_dev, int8_t *pc_param);
uint32_t wal_hipriv_send_cw_signal(oal_net_device_stru *net_dev, int8_t *pc_param);
uint32_t wal_hipriv_adjust_ppm(oal_net_device_stru *net_dev, int8_t *pc_param);
uint32_t wal_hipriv_chip_check(oal_net_device_stru *net_dev, int8_t *pc_param);
uint32_t wal_hipriv_set_txpower(oal_net_device_stru *net_dev, int8_t *pc_param);
int32_t wal_ioctl_get_current_tsf(oal_net_device_stru *net_dev, oal_ifreq_stru *ifr,
    wal_wifi_priv_cmd_stru *priv_cmd);
#ifdef _PRE_WLAN_FEATURE_NAN
void wal_nan_correction_param(mac_nan_param_stru *param);
#endif
#ifdef _PRE_WLAN_FEATURE_HID2D
uint32_t wal_ioctl_set_hid2d_acs_mode(oal_net_device_stru *net_dev, int8_t *param);
uint32_t wal_ioctl_hid2d_link_meas(oal_net_device_stru *net_dev, int8_t *param);
uint32_t wal_ioctl_update_acs_state(oal_net_device_stru *net_dev, int8_t *param);
#endif
#ifdef _PRE_WLAN_FEATURE_11D
int32_t wal_regdomain_update_for_dfs(oal_net_device_stru *pst_net_dev, int8_t *pc_country);
int32_t wal_regdomain_update(oal_net_device_stru *pst_net_dev, int8_t *pc_country);
#endif
void wal_init_wlan_netdev_set_netdev(oal_net_device_stru *net_dev, oal_wireless_dev_stru *pst_wdev);
#ifdef _PRE_WLAN_FEATURE_TAS_ANT_SWITCH
int32_t wal_init_wifi_tas_state(void);
#endif
int wal_ioctl_set_ap_sta_disassoc(oal_net_device_stru *net_dev,
    oal_iw_request_info_stru *pst_info, oal_iwreq_data_union *pst_wrqu, char *pc_extra);
int wal_ioctl_set_mac_filters(oal_net_device_stru *net_dev,
    oal_iw_request_info_stru *pst_info, oal_iwreq_data_union *pst_wrqu, char *pc_extra);
int wal_ioctl_set_ap_config(oal_net_device_stru *net_dev,
    oal_iw_request_info_stru *pst_info, oal_iwreq_data_union *pst_wrqu, char *pc_extra);
void wal_config_mac_filter(oal_net_device_stru *net_dev, int8_t *pc_command);
int32_t wal_kick_sta(oal_net_device_stru *net_dev,
    uint8_t *auc_mac_addr, uint8_t uc_mac_addr_len, uint16_t us_reason_code);
int wal_ioctl_get_assoc_list(oal_net_device_stru *net_dev,
    oal_iw_request_info_stru *pst_info, oal_iwreq_data_union *pst_wrqu, char *pc_extra);
void wal_init_wlan_ap_mode_set_max_user_count_and_mac_filter(oal_net_device_stru *net_dev,
    oal_wireless_dev_stru *pst_wdev);
int wal_ioctl_get_essid(oal_net_device_stru *net_dev, oal_iw_request_info_stru *pst_info,
    oal_iwreq_data_union *pst_data, char *pc_ssid);
int wal_ioctl_get_apaddr(oal_net_device_stru *net_dev, oal_iw_request_info_stru *pst_info,
    oal_iwreq_data_union *pst_wrqu, char *pc_extra);
int wal_ioctl_get_iwname(oal_net_device_stru *net_dev,
    oal_iw_request_info_stru *pst_info, oal_iwreq_data_union *pst_wrqu, char *pc_extra);
#if ((defined(_PRE_WLAN_EXPORT)) && (defined(_PRE_WLAN_NARROW_BAND)))
int32_t wal_ioctl_get_tsensor_val(oal_net_device_stru *pst_net_dev, int32_t *pc_tsensor_val);
int32_t wal_ioctl_get_pdet_val(oal_net_device_stru *pst_net_dev, int32_t *pdet_val);
int32_t wal_ioctl_get_sw_version(oal_net_device_stru *pst_net_dev);
#endif
#ifdef _PRE_WLAN_FEATURE_LP_MIRACAST
int32_t wal_ioctl_set_lp_miracast(oal_net_device_stru *net_dev, uint8_t is_enable);
int32_t wal_ioctl_get_m2s_state(oal_net_device_stru *net_dev);
#endif
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
int32_t wal_hipriv_inet6addr_notifier_call(struct notifier_block *this, unsigned long event, void *ptr);
int32_t wal_hipriv_inetaddr_notifier_call(struct notifier_block *this, unsigned long event, void *ptr);
#endif
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif /* end of wal_linux_ioctl.h */
