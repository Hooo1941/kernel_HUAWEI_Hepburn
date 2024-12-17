/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : linux ioctl配置命令
 * 作    者 :
 * 创建日期 : 2012年12月10日
 */

#include "oal_cfg80211.h"
#include "oal_ext_if.h"
#include "oal_kernel_file.h"
#include "oam_ext_if.h"
#include "frw_ext_if.h"
#if (_PRE_FRW_FEATURE_PROCCESS_ENTITY_TYPE == _PRE_FRW_FEATURE_PROCCESS_ENTITY_THREAD)
#ifndef CONFIG_HI110X_KERNEL_MODULES_BUILD_SUPPORT
#include "frw_task.h"
#endif
#endif

#include "wlan_spec.h"
#include "wlan_types.h"
#include "wlan_chip_i.h"
#include "mac_vap.h"
#include "mac_resource.h"
#include "hmac_resource.h"
#include "hmac_ext_if.h"
#include "hmac_chan_mgmt.h"
#include "hmac_tx_data.h"
#include "hmac_config.h"
#include "wal_config_alg_cfg.h"
#include "wal_main.h"
#include "wal_ext_if.h"
#include "wal_linux_ioctl.h"
#include "wal_cfg_ioctl.h"
#include "wal_linux_bridge.h"
#include "wal_linux_flowctl.h"
#include "wal_linux_cfg80211.h"
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include <linux/inetdevice.h>
#include <net/addrconf.h>
#endif
#include "securec.h"
#include "securectype.h"
#ifdef _PRE_WLAN_CHBA_MGMT
#include "hmac_chba_coex.h"
#include "wal_hipriv_chba.h"
#endif
#include "wal_config_alg_cfg.h"
#ifdef _PRE_WLAN_FEATURE_VSP
#include "hmac_vsp_if.h"
#endif

#if defined(CONFIG_HISI_CMDLINE_PARSE) || defined(CONFIG_CMDLINE_PARSE)
unsigned int get_boot_into_recovery_flag(void);
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID      OAM_FILE_ID_WAL_LINUX_IOCTL_C

OAL_STATIC oal_proc_dir_entry_stru *g_pst_proc_entry = NULL;
OAL_STATIC wal_msg_queue g_wal_wid_msg_queue;

wal_ap_config_stru g_st_ap_config_info = { 0 }; /* AP配置信息,需要在vap 创建后下发的 */

/* mp12-cb add sys for 51/02 */
#ifdef _PRE_CONFIG_CONN_HISI_SYSFS_SUPPORT
struct kobject *g_gp_sys_kobject;
#endif

oal_wait_queue_head_stru g_wlan_cali_complete_wq;
uint8_t g_go_cac = OAL_TRUE;

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
OAL_STATIC struct notifier_block g_wal_hipriv_notifier = {
    .notifier_call = wal_hipriv_inetaddr_notifier_call
};
OAL_STATIC struct notifier_block g_wal_hipriv_notifier_ipv6 = {
    .notifier_call = wal_hipriv_inet6addr_notifier_call
};
#endif

#ifdef _PRE_CONFIG_CONN_HISI_SYSFS_SUPPORT
/* mp12-cb add sys for 51/02 */
OAL_STATIC oal_ssize_t wal_hipriv_sys_write(struct kobject *dev, struct kobj_attribute *attr,
    const char *buf, oal_size_t count);
OAL_STATIC oal_ssize_t wal_hipriv_sys_read(struct kobject *dev, struct kobj_attribute *attr, char *buf);
OAL_STATIC struct kobj_attribute g_dev_attr_hipriv =
    __ATTR(hipriv, (OAL_S_IRUGO | OAL_S_IWUSR), wal_hipriv_sys_read, wal_hipriv_sys_write);
OAL_STATIC struct attribute *g_hipriv_sysfs_entries[] = {
    &g_dev_attr_hipriv.attr,
    NULL
};
OAL_STATIC struct attribute_group g_hipriv_attribute_group = {
    .attrs = g_hipriv_sysfs_entries,
};
/* mp12-cb add sys for 51/02 */
#endif

int32_t wal_init_wlan_vap(oal_net_device_stru *net_dev);
int32_t wal_deinit_wlan_vap(oal_net_device_stru *net_dev);
int32_t wal_start_vap(oal_net_device_stru *net_dev);
int32_t wal_stop_vap(oal_net_device_stru *net_dev);
int32_t wal_init_wlan_netdev(oal_wiphy_stru *pst_wiphy, const char *dev_name);


const wal_hipriv_cmd_entry_stru  g_ast_hipriv_cmd[] = {
    /* 商用对外发布的私有命令 */
    { "set_sta_pm_on", wal_hipriv_sta_pm_on }, /* sh hipriv.sh 'wlan0 set_sta_pm_on xx xx xx xx */
};

/* net_device上挂接的net_device_ops函数 */
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
oal_net_device_ops_stru g_st_wal_net_dev_ops = {
    .ndo_get_stats = wal_netdev_get_stats,
    .ndo_open = wal_netdev_open_ext,
    .ndo_stop = wal_netdev_stop,
    .ndo_start_xmit = wal_bridge_vap_xmit,

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))
    /* Temp blank */
#else
    .ndo_set_multicast_list = NULL,
#endif

    .ndo_do_ioctl = wal_net_device_ioctl,
    .ndo_change_mtu = oal_net_device_change_mtu,
    .ndo_init = oal_net_device_init,

    .ndo_select_queue = wal_netdev_select_queue,

    .ndo_set_mac_address = wal_netdev_set_mac_addr
};

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))
/* 不支持ethtool_ops 相关函数操作,需要显示定义空的结构体，否则将采用内核默认的ethtool_ops会导致空指针异常 */
oal_ethtool_ops_stru g_st_wal_ethtool_ops = { 0 };
#endif

#ifdef _PRE_WLAN_FEATURE_NAN
/* xmit回调必须注册，NAN注册一个空函数。其他接口不需要注册。 */
OAL_STATIC oal_net_dev_tx_enum wal_nan_xmit(oal_netbuf_stru *buf, oal_net_device_stru *dev)
{
    oal_netbuf_free(buf);
    return NETDEV_TX_OK;
}

OAL_STATIC oal_net_device_ops_stru g_nan_net_dev_ops = {
    .ndo_start_xmit = wal_nan_xmit,
};
#endif
#endif

/* 标准ioctl命令函数表 */
OAL_STATIC const oal_iw_handler g_ast_iw_handlers[] = {
    NULL,                         /* SIOCSIWCOMMIT, */
    (oal_iw_handler)wal_ioctl_get_iwname, /* SIOCGIWNAME, */
    NULL,                         /* SIOCSIWNWID, */
    NULL,                         /* SIOCGIWNWID, */
    NULL,                         /* SIOCSIWFREQ, 设置频点信道 */
    NULL,                         /* SIOCGIWFREQ, 获取频点信道 */
    NULL,                         /* SIOCSIWMODE, 设置bss type */
    NULL,                         /* SIOCGIWMODE, 获取bss type */
    NULL,                         /* SIOCSIWSENS, */
    NULL,                         /* SIOCGIWSENS, */
    NULL, /* SIOCSIWRANGE, */     /* not used */
    NULL,                         /* SIOCGIWRANGE, */
    NULL, /* SIOCSIWPRIV, */      /* not used */
    NULL, /* SIOCGIWPRIV, */      /* kernel code */
    NULL, /* SIOCSIWSTATS, */     /* not used */
    NULL,                         /* SIOCGIWSTATS, */
    NULL,                         /* SIOCSIWSPY, */
    NULL,                         /* SIOCGIWSPY, */
    NULL,                         /* -- hole -- */
    NULL,                         /* -- hole -- */
    NULL,                         /* SIOCSIWAP, */
    (oal_iw_handler)wal_ioctl_get_apaddr, /* SIOCGIWAP, */
    NULL,                         /* SIOCSIWMLME, */
    NULL,                         /* SIOCGIWAPLIST, */
    NULL,                         /* SIOCSIWSCAN, */
    NULL,                         /* SIOCGIWSCAN, */
    NULL,                         /* SIOCSIWESSID, 设置ssid */
    (oal_iw_handler)wal_ioctl_get_essid,  /* SIOCGIWESSID, 获取ssid */
    NULL,                         /* SIOCSIWNICKN */
    NULL,                         /* SIOCGIWNICKN */
    NULL,                         /* -- hole -- */
    NULL,                         /* -- hole -- */
    NULL,                         /* SIOCSIWRATE */
    NULL,                         /* SIOCGIWRATE  get_iwrate */
    NULL,                         /* SIOCSIWRTS */
    NULL,                         /* SIOCGIWRTS  get_rtsthres */
    NULL,                         /* SIOCSIWFRAG */
    NULL,                         /* SIOCGIWFRAG  get_fragthres */
    NULL,                         /* SIOCSIWTXPOW, 设置传输功率限制 */
    NULL,                         /* SIOCGIWTXPOW, 设置传输功率限制 */
    NULL,                         /* SIOCSIWRETRY */
    NULL,                         /* SIOCGIWRETRY */
    NULL,                         /* SIOCSIWENCODE */
    NULL,                         /* SIOCGIWENCODE  get_iwencode */
    NULL,                         /* SIOCSIWPOWER */
    NULL,                         /* SIOCGIWPOWER */
    NULL,                         /* -- hole -- */
    NULL,                         /* -- hole -- */
    NULL,                         /* SIOCSIWGENIE */
    NULL,                         /* SIOCGIWGENIE */
    NULL,                         /* SIOCSIWAUTH */
    NULL,                         /* SIOCGIWAUTH */
    NULL,                         /* SIOCSIWENCODEEXT */
    NULL                          /* SIOCGIWENCODEEXT */
};

/* 私有ioctl命令参数定义 */
OAL_STATIC const oal_iw_priv_args_stru g_ast_iw_priv_args[] = {
    { WAL_IOCTL_PRIV_SET_AP_CFG,  OAL_IW_PRIV_TYPE_CHAR | OAL_IW_PRIV_TYPE256, 0, "AP_SET_CFG" },
    { WAL_IOCTL_PRIV_AP_MAC_FLTR, OAL_IW_PRIV_TYPE_CHAR | OAL_IW_PRIV_TYPE256,
      OAL_IW_PRIV_TYPE_CHAR | OAL_IW_PRIV_SIZE_FIXED | 0,         "AP_SET_MAC_FLTR" },
    { WAL_IOCTL_PRIV_AP_GET_STA_LIST, 0, OAL_IW_PRIV_TYPE_CHAR | OAL_IW_PRIV_TYPE1024, "AP_GET_STA_LIST" },
    { WAL_IOCTL_PRIV_AP_STA_DISASSOC, OAL_IW_PRIV_TYPE_CHAR | OAL_IW_PRIV_TYPE256,
      OAL_IW_PRIV_TYPE_CHAR | 0, "AP_STA_DISASSOC" },

    { WLAN_CFGID_ADD_BLACK_LIST, OAL_IW_PRIV_TYPE_CHAR | OAL_IW_PRIV_TYPE40,        0,    "blkwhtlst_add" },
    { WLAN_CFGID_DEL_BLACK_LIST, OAL_IW_PRIV_TYPE_CHAR | OAL_IW_PRIV_TYPE40,        0,    "blkwhtlst_del" },
    { WLAN_CFGID_CLR_BLACK_LIST, OAL_IW_PRIV_TYPE_CHAR | OAL_IW_PRIV_TYPE40,        0,    "blkwhtlst_clr" },
    /* 配置黑名单模式 */
    { WLAN_CFGID_BLACKLIST_MODE, OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, 0,    "blkwhtlst_mode" },
    /* 黑名单模式打印 */
    { WLAN_CFGID_BLACKLIST_MODE, 0, OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1,    "get_blkwhtmode" },

    { WLAN_CFGID_GET_2040BSS_SW, 0, OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1,    "get_obss_sw" },
    { WLAN_CFGID_2040BSS_ENABLE, OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, 0,    "set_obss_sw" },
};

/* 私有ioctl命令函数表 */
OAL_STATIC const oal_iw_handler g_ast_iw_priv_handlers[] = {
    NULL, /* SIOCWFIRSTPRIV+0 */                                   /* sub-ioctl set 入口 */
    NULL, /* SIOCWFIRSTPRIV+1 */                                   /* sub-ioctl get 入口 */
    NULL, /* SIOCWFIRSTPRIV+2 */                                   /* setkey */
    NULL, /* SIOCWFIRSTPRIV+3 */                                   /* setwmmparams */
    NULL, /* SIOCWFIRSTPRIV+4 */                                   /* delkey */
    NULL, /* SIOCWFIRSTPRIV+5 */                                   /* getwmmparams */
    NULL, /* SIOCWFIRSTPRIV+6 */                                   /* setmlme */
    NULL, /* SIOCWFIRSTPRIV+7 */                                   /* getchaninfo */
    NULL, /* SIOCWFIRSTPRIV+8 */                                   /* setcountry */
    NULL, /* SIOCWFIRSTPRIV+9 */                                   /* getcountry */
    NULL, /* SIOCWFIRSTPRIV+10 */                                  /* addmac */
    NULL, /* SIOCWFIRSTPRIV+11 */                                  /* getscanresults */
    NULL, /* SIOCWFIRSTPRIV+12 */                                  /* delmac */
    NULL, /* SIOCWFIRSTPRIV+13 */                                  /* getchanlist */
    NULL, /* SIOCWFIRSTPRIV+14 */                                  /* setchanlist */
    NULL, /* SIOCWFIRSTPRIV+15 */                                  /* setmac */
    NULL, /* SIOCWFIRSTPRIV+16 */                                  /* chanswitch */
    NULL, /* SIOCWFIRSTPRIV+17 */                                  /* 获取模式, 例: iwpriv vapN get_mode */
    NULL, /* SIOCWFIRSTPRIV+18 */                                  /* 设置模式, 例: iwpriv vapN mode 11g */
    NULL, /* SIOCWFIRSTPRIV+19 */                                  /* getappiebuf */
    NULL, /* SIOCWFIRSTPRIV+20 */                                  /* null */
    (oal_iw_handler)wal_ioctl_get_assoc_list, /* SIOCWFIRSTPRIV+21 */      /* APUT取得关联STA列表 */
    (oal_iw_handler)wal_ioctl_set_mac_filters, /* SIOCWFIRSTPRIV+22 */     /* APUT设置STA过滤 */
    (oal_iw_handler)wal_ioctl_set_ap_config, /* SIOCWFIRSTPRIV+23 */       /* 设置APUT参数 */
    (oal_iw_handler)wal_ioctl_set_ap_sta_disassoc, /* SIOCWFIRSTPRIV+24 */ /* APUT去关联STA */
    NULL, /* SIOCWFIRSTPRIV+25 */                                  /* getStatistics */
    NULL, /* SIOCWFIRSTPRIV+26 */                                  /* sendmgmt */
    NULL, /* SIOCWFIRSTPRIV+27 */                                  /* null */
    NULL, /* SIOCWFIRSTPRIV+28 */                                  /* null */
    NULL, /* SIOCWFIRSTPRIV+29 */                                  /* null */
    NULL, /* SIOCWFIRSTPRIV+30 */                                  /* sethbrparams */

    NULL,
    /* SIOCWFIRSTPRIV+31 */ /* setrxtimeout */
};

/* 无线配置iw_handler_def定义 */
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
oal_iw_handler_def_stru g_st_iw_handler_def = {
    .standard = g_ast_iw_handlers,
    .num_standard = oal_array_size(g_ast_iw_handlers),
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 59))
#ifdef CONFIG_WEXT_PRIV
    .private = g_ast_iw_priv_handlers,
    .num_private = oal_array_size(g_ast_iw_priv_handlers),
    .private_args = g_ast_iw_priv_args,
    .num_private_args = oal_array_size(g_ast_iw_priv_args),
#endif
#else
    .private = g_ast_iw_priv_handlers,
    .num_private = oal_array_size(g_ast_iw_priv_handlers),
    .private_args = g_ast_iw_priv_args,
    .num_private_args = oal_array_size(g_ast_iw_priv_args),
#endif
    .get_wireless_stats = NULL
};

#elif (_PRE_OS_VERSION_WIN32 == _PRE_OS_VERSION)
oal_iw_handler_def_stru g_st_iw_handler_def = {
    NULL, /* 标准ioctl handler */
    0,
    oal_array_size(g_ast_iw_priv_handlers),
    { 0, 0 }, /* 字节对齐 */
    oal_array_size(g_ast_iw_priv_args),
    g_ast_iw_priv_handlers, /* 私有ioctl handler */
    g_ast_iw_priv_args,
    NULL
};
#endif

const int8_t *g_pauc_bw_tbl[WLAN_BANDWITH_CAP_BUTT] = {
    "20",
    "40",
    "d40",
    "80",
    "d80",
    "160",
    "d160",
    "80_80",
#ifdef  _PRE_WLAN_FEATURE_11AX_ER_SU_DCM
    "242tone",
    "106tone",
#endif
};

const int8_t *g_pauc_non_ht_rate_tbl[WLAN_LEGACY_RATE_VALUE_BUTT] = {
    "1",
    "2",
    "5.5",
    "11",
    "rsv0",
    "rsv1",
    "rsv2",
    "rsv3",
    "48",
    "24",
    "12",
    "6",
    "54",
    "36",
    "18",
    "9"
};

const wal_ioctl_cmd_tbl_stru g_ast_dyn_cali_cfg_map[] = {
    { "realtime_cali_adjust", MAC_DYN_CALI_CFG_SET_EN_REALTIME_CALI_ADJUST },
    { "2g_dscr_interval",     MAC_DYN_CALI_CFG_SET_2G_DSCR_INT },
    { "5g_dscr_interval",     MAC_DYN_CALI_CFG_SET_5G_DSCR_INT },
    { "chain_interval",       MAC_DYN_CALI_CFG_SET_CHAIN_INT },
    { "pdet_min_th",          MAC_DYN_CALI_CFG_SET_PDET_MIN_TH },
    { "pdet_max_th",          MAC_DYN_CALI_CFG_SET_PDET_MAX_TH },
    { "debug_switch",         MAC_DYN_CALI_CFG_DEBUG_SWITCH },
    { NULL }
};
const uint32_t g_dyn_cali_cfg_map_size = oal_array_size(g_ast_dyn_cali_cfg_map);

const wal_ioctl_tlv_stru g_ast_set_tlv_table[] = {
    /* cmd: wlan0 set_tlv xx xx */
    { "tx_pkts_stat", WLAN_CFGID_SET_DEVICE_PKT_STAT },
    { "auto_freq", WLAN_CFGID_SET_DEVICE_FREQ },

    { "set_adc_dac_freq", WLAN_CFGID_SET_ADC_DAC_FREQ }, /* 设置ADC DAC频率 */
    { "set_mac_freq", WLAN_CFGID_SET_MAC_FREQ },         /* 设MAC频率 */

    { "rx_ampdu_num", WLAN_CFGID_SET_ADDBA_RSP_BUFFER },
    { "data_collect", WLAN_CFGID_DATA_COLLECT },    /* 第一个参数为模式，第二个参数为点数 */
    /* cmd: wlan0 set_val xx */
    { "tx_ampdu_type",  WLAN_CFGID_SET_TX_AMPDU_TYPE },  /* 设置聚合类型的开关 */
    { "tx_ampdu_amsdu", WLAN_CFGID_AMSDU_AMPDU_SWITCH }, /* 设置tx amsdu ampdu联合聚合功能的开关 */
    { "rx_ampdu_amsdu", WLAN_CFGID_SET_RX_AMPDU_AMSDU }, /* 设置rx ampdu amsdu 联合聚合功能的开关 */
    { "rx_anti_immu", WLAN_CFGID_SET_RX_ANTI_IMMU },
    { "sk_pacing_shift", WLAN_CFGID_SET_SK_PACING_SHIFT },

    { "performance_log", WLAN_CFGID_SET_TRX_STAT_LOG }, /* 设置tcp ack缓存时吞吐量统计维测开关，其他模块可参考 */
    { "mimo_blacklist", WLAN_CFGID_MIMO_BLACKLIST },    /* 设置探测MIMO黑名单机制开关 */
    /* cmd: wlan0 set_val dfs_debug 0|1 */
    { "dfs_debug", WLAN_CFGID_SET_DFS_MODE },           /* 设置dfs是否为检测率模式的开关 */
    { "warning_mode", WLAN_CFGID_SET_WARNING_MODE },    /* 设置部分WARNING是否为测试模式的开关 */

    { "chr_mode", WLAN_CFGID_SET_CHR_MODE },            /* 设置chr模式的开关 */
    {"linkloss_csa_dis", WLAN_CFGID_SET_LINKLOSS_DISABLE_CSA},    /* 配置linkloss是否禁止csa */
    {"rx_filter_frag", WLAN_CFGID_RX_FILTER_FRAG},    /* 配置rx过滤功能 */
    {"dyn_pcie", WLAN_CFGID_SET_DYN_PCIE},
#if defined(_PRE_WLAN_FEATURE_HID2D) && defined(_PRE_WLAN_FEATURE_HID2D_PRESENTATION)
    { "hid2d_presentation", WLAN_CFGID_HID2D_PRESENTATION_MODE }, /* 配置为发布会场景 */
#endif
    { NULL }
};

#ifdef _PRE_WLAN_FEATURE_11AX
const wal_ioctl_tlv_stru g_ast_11ax_debug_table[] = {
    /* cmd: wlan0 xxx  2  xxx 1 xxx 0 */
    { "print_log",        MAC_VAP_11AX_DEBUG_PRINT_LOG },
    { "print_rx_log",     MAC_VAP_11AX_DEBUG_PRINT_RX_LOG },
    { "tid",              MAC_VAP_11AX_DEBUG_HE_TB_PPDU_TID_NUM },
    { "htc_order",        MAC_VAP_11AX_DEBUG_HE_TB_PPDU_HTC_ORGER },
    { "htc_val",          MAC_VAP_11AX_DEBUG_HE_TB_PPDU_HTC_VALUE },
    { "fix_power",        MAC_VAP_11AX_DEBUG_HE_TB_PPDU_FIX_POWER },
    { "power_val",        MAC_VAP_11AX_DEBUG_HE_TB_PPDU_POWER_VALUE },
    { "disable_ba_check", MAC_VAP_11AX_DEBUG_HE_TB_PPDU_DISABLE_BA_CHECK },
    { "disable_mu_edca",  MAC_VAP_11AX_DEBUG_DISABLE_MU_EDCA },
    { "manual_cfo",       MAC_VAP_11AX_DEBUG_MANUAL_CFO },
    { "bsrp",             MAC_VAP_11AX_DEBUG_BSRP_CFG },
    { "bsrp_tid",         MAC_VAP_11AX_DEBUG_BSRP_TID },
    { "bsrp_size",        MAC_VAP_11AX_DEBUG_BSRP_QUEUE_SIZE },
    { "mpad_dur",         MAC_VAP_11AX_DEBUG_MAC_PADDING },
    { "tb_pwr_test",      MAC_VAP_11AX_DEBUG_POW_TEST },
    { "tb_ppdu_len",      MAC_VAP_11AX_DEBUG_TB_PPDU_LEN },
    { "tb_ppdu_ac",       MAC_VAP_11AX_DEBUG_TB_PPDU_AC },
    { "print_rx_trig",    MAC_VAP_11AX_DEBUG_PRINT_RX_TRIG_FRAME },
    { "om_auth_flag",     MAC_VAP_11AX_DEBUG_OM_AUTH_FLAG },
    { "tom_bw",           MAC_VAP_11AX_DEBUG_TOM_BW_FLAG },
    { "tom_nss",          MAC_VAP_11AX_DEBUG_TOM_NSS_FLAG },
    { "tom_ul_mu_disable", MAC_VAP_11AX_DEBUG_TOM_UL_MU_DISABLE_FLAG },
    { "uora_ocw",         MAC_VAP_11AX_DEBUG_UORA_OCW_UPDATE},
    { "uora_obo_bypass",  MAC_VAP_11AX_DEBUG_UORA_OBO_BYPASS},
    { "bss_color",        MAC_VAP_11AX_DEBUG_BSS_COLOR },
    { "tx_q_status",      MAC_VAP_11AX_DEBUG_TX_Q_STATUS},
    { "print_qos_null",   MAC_VAP_11AX_DEBUG_PRINT_QOSNULL},

    { NULL }
};
#endif

/* MAC TX RX common info report命令表格 */
const wal_ioctl_tlv_stru g_ast_mac_tx_report_debug_table[] = {
    /* cmd: sh hipriv.sh "wlan0 set_str mac_report_tx XXX(命令参数个数) index XXX status XXX bw XXX protocol XXX
     *                    fr_type XXX sub_type XXX ampdu XXX psdu XXX hw_retry XXX"
     * MAC TX common命令举例:
     * 1.统计TB QoS NULL: sh hipriv.sh "wlan0 set_str mac_report_tx 4 index 0 protocol 11 fr_type 2 sub_type 12"
     * 2.清除统计寄存器配置信息:sh hipriv.sh "wlan0 set_str mac_report_tx 2 index 0 clear 1"
     * 注意:(1)tx只有index0支持status的配置，其他index不支持 (2)清除统计后重新统计需要写0之后才能正常计数
     *      (3)在一条命令中配置多个选项可能导致命令长度超过限制，可以分次进行配置
     * MAC寄存器CFG_TX_COMMON_CNT_CTRL各bit位配置详见寄存器表单
     */
    { "index",  MAC_TX_COMMON_REPORT_INDEX },            /* tx rx分别有8个 index:0-7 */
    { "status", MAC_TX_COMMON_REPORT_STATUS },           /* 0：发送成功 1：响应帧错误 2：响应帧超时 3：发送异常结束 */
    { "bw",     MAC_TX_COMMON_REPORT_BW_MODE },          /* bandwidth 0:20M 1:40M 2:80M 3:160M */
    /* 0000: The data rate is 11b type                    0001: The data rate is legacy OFDM type
     * 0010: The data rate is HT Mixed mode Frame type    0011: The data rate is HT Green Field Frame type
     * 0100: The data rate is VHT type                    0101~0111：reserved
     * 1000：The data rate is HE  SU Format type          1001：The data rate is HE  MU Format type
     * 1010：The data rate is HE  EXT SU Format type      1011：The data rate is HE  TRIG Format type
     * 1100~1111:reserved
     */
    { "protocol", MAC_TX_COMMON_REPORT_PROTOCOL_MODE },  /* 协议模式 */
    { "fr_type",  MAC_TX_COMMON_REPORT_FRAME_TYPE },     /* 帧类型 */
    { "sub_type", MAC_TX_COMMON_REPORT_SUB_TYPE },       /* 子类型 */
    { "ampdu",    MAC_TX_COMMON_REPORT_APMDU },          /* 0：非ampdu时统计 1：ampdu时统计 */
    { "psdu",     MAC_TX_COMMON_REPORT_PSDU },           /* 按psdu统计还是按mpdu统计：0：按mpdu统计 1：按psdu统计 */
    { "hw_retry", MAC_TX_COMMON_REPORT_HW_RETRY },       /* 0：非hw retry帧时统计 1：hw retry帧时统计 */
    { "clear",    MAC_TX_COMMON_REPORT_CTRL_REG_CLEAR }, /* 清除对应index的寄存器配置 */

    { NULL }
};

const wal_ioctl_tlv_stru g_ast_mac_rx_report_debug_table[] = {
    /* cmd: sh hipriv.sh "wlan0 set_str mac_report_rx 11(命令参数个数) index 5 status 1 bw 2 protocol 1 fr_type 2
     *                     sub_type 4 ampdu 1 psdu 1 vap_mode 3 bss 1 direct 1"
     *  clear命令: sh hipriv.sh "wlan0 set_str mac_report_rx index 5 clear 1"
     *  注意：清除统计后重新统计需要写0之后才能正常计数
     */
    /* 0：Invalid  1：RX successful
     * 2：Duplicate detected 3：FCS check failed
     * 4：Key search failed 5：MIC check failed
     * 6：ICV failed        others：Reserved
     */
    { "index",  MAC_RX_COMMON_REPORT_INDEX },            /* tx rx分别有8个 index:0-7 */
    { "status", MAC_RX_COMMON_REPORT_STATUS },           /* 接收状态 */
    { "bw", MAC_RX_COMMON_REPORT_BW_MODE },              /* bandwidth 0:20M 1:40M 2:80M 3:160M */
    /* 0000: The data rate is 11b type                    0001: The data rate is legacy OFDM type
       0010: The data rate is HT Mixed mode Frame type    0011: The data rate is HT Green Field Frame type
       0100: The data rate is VHT type                    0101~0111：reserved
       1000：The data rate is HE  SU Format type          1001：The data rate is HE  MU Format type
       1010：The data rate is HE  EXT SU Format type      1011：The data rate is HE  TRIG Format type
       1100~1111:reserved */
    { "protocol", MAC_RX_COMMON_REPORT_PROTOCOL_MODE },  /* 协议模式 */
    { "fr_type",  MAC_RX_COMMON_REPORT_FRAME_TYPE },     /* 帧类型 */
    { "sub_type", MAC_RX_COMMON_REPORT_SUB_TYPE },       /* 子类型 */
    { "ampdu",    MAC_RX_COMMON_REPORT_APMDU },          /* 0：非ampdu时统计 1：ampdu时统计 */
    { "psdu",     MAC_RX_COMMON_REPORT_PSDU },           /* 按psdu统计还是按mpdu统计：0：按mpdu统计 1：按psdu统计 */
    { "vap_mode", MAC_RX_COMMON_REPORT_VAP_CHK },        /* 是否按vap统计:0~4：接收到vap0~4计数 5~6:resv 7:不区分vap */
    { "bss",      MAC_RX_COMMON_REPORT_BSS_CHK },        /* 0：非本bss时统计 1：本bss时统计 */
    { "direct",   MAC_RX_COMMON_REPORT_DIRECT_CHK },     /* 0：非direct帧时统计 1：direct帧时统计 */
    { "clear",    MAC_RX_COMMON_REPORT_CTRL_REG_CLEAR }, /* 清除对应index的寄存器配置 */

    { NULL }
};

const wal_ioctl_tlv_stru g_ast_common_debug_table[] = {
    { "pg_switch", PG_EFFICIENCY_STATISTICS_ENABLE },
    { "pg_info", PG_EFFICIENCY_INFO },

    { "hw_txq", MAC_VAP_COMMON_SET_TXQ },
    { "tx_info", MAC_VAP_COMMON_TX_INFO },

#ifdef _PRE_WLAN_FEATURE_MBO
    { "mbo_switch",                  MBO_SWITCH },        /* 测试用例1:MBO特性开关， out-of-the-box test */
    { "mbo_cell_capa",               MBO_CELL_CAP },      /* 测试用例2:MBO Capability Indication test  */
    { "mbo_assoc_disallowed_switch", MBO_ASSOC_DISALLOWED_SWITCH }, /* 测试用例6 */
#endif

#ifdef _PRE_WLAN_FEATURE_HIEX
    { "set_dev_hiex",                 HIEX_DEV_CAP },     /* mac device hiex cap cmd */
    { "hiex_debug_switch",            HIEX_DEBUG}, /* hiex debug */
#endif

    { "rifs_en",                      RIFS_ENABLE },
    { "greenfield_en",                GREENFIELD_ENABLE },
    { "protect_info",                 PROTECTION_LOG_SWITCH},

#ifdef _PRE_WLAN_FEATURE_11AX
    {"rx_frame_cnt",                 DMAC_RX_FRAME_STATISTICS}, /* 接收帧统计 */
    {"tx_frame_cnt",                 DMAC_TX_FRAME_STATISTICS}, /* 发送帧统计 */
    {"su_1xltf_0.8us_gi",            SU_PPDU_1XLTF_08US_GI_SWITCH},
#endif
    {"auth_rsp_timeout",             AUTH_RSP_TIMEOUT},
    {"forbid_open_auth",             FORBIT_OPEN_AUTH},

#ifdef _PRE_WLAN_RX_LISTEN_POWER_SAVING
    {"hal_ps_debug_switch",          HAL_PS_DEBUG_SWITCH},
#endif
    {"ht_self_cure",                 HT_SELF_CURE_DEBUG},
    {"set_bindcpu",                  USERCTL_BINDCPU},
#ifdef _PRE_WLAN_FEATURE_MCAST_AMPDU
    {"set_mcast_ampdu_retry",        MCAST_AMPDU_RETRY},
    {"mcast_ampdu_pack_switch",      MCAST_AMPDU_PACK_SWITCH},
#endif
#ifdef _PRE_WLAN_FEATURE_PSD_ANALYSIS
    {"psd_cfg",                      PSD_CFG},    /* PSD初始化配置 */
    {"psd_enable",                   PSD_ENABLE}, /* 使能一个PSD周期 */
    {"psd_debug",                    PSD_DEBUG},
#endif
#ifdef _PRE_WLAN_FEATURE_OTHER_CTS
    {"other_cts_debug",              OTHER_CTS_DEBUG},
#endif
    { NULL }
};

OAL_STATIC const wal_ioctl_tlv_stru g_ast_debug_log_table[] = {
    { "trx_log", MAC_LOG_TRX_LOG },
    { "ota_switch", MAC_LOG_SET_ALG_OTA },  /* bit[7:4]:0 close/1 open; bit[3:0]:0 tx/1 rx */
    { "temp_protect_log", MAC_LOG_SET_TEMP_PROTECT_LOG },
    { "himit_probe_enable", MAC_LOG_SET_HIMIT_PROBE_ENABLE },
    { "himit_set_htc", MAC_LOG_SET_HIMIT_HTC },
    { "monitor_ota", MAC_LOG_MONITOR_OTA_RPT },
    { "wifi_delay", MAC_LOG_DELAY_RPT },
    { "device_memory", MAC_LOG_DEVICE_MEMORY },
    { NULL }
};
OAL_STATIC const wal_ioctl_tlv_stru g_ast_multi_tid_table[] = {
    { "tid0_ampdu", MAC_TID0_AMPDU },
    { "tid0_amsdu", MAC_TID0_AMSDU },
    { "tid1_ampdu", MAC_TID1_AMPDU },
    { "tid1_amsdu", MAC_TID1_AMSDU },
    { "tid2_ampdu", MAC_TID2_AMPDU },
    { "tid2_amsdu", MAC_TID2_AMSDU },
    { "tid3_ampdu", MAC_TID3_AMPDU },
    { "tid3_amsdu", MAC_TID3_AMSDU },
    { "tid4_ampdu", MAC_TID4_AMPDU },
    { "tid4_amsdu", MAC_TID4_AMSDU },
    { "tid5_ampdu", MAC_TID5_AMPDU },
    { "tid5_amsdu", MAC_TID5_AMSDU },
    { "tid6_ampdu", MAC_TID6_AMPDU },
    { "tid6_amsdu", MAC_TID6_AMSDU },
    { "tid7_ampdu", MAC_TID7_AMPDU },
    { "tid7_amsdu", MAC_TID7_AMSDU },
    { NULL }
};
OAL_STATIC const wal_ioctl_tlv_stru g_ast_trx_debug_table[] = {
    { "tx_ring_switch", MAC_TRX_TX_RING_SWITCH },
    { "ring_switch_independent", MAC_TRX_RING_SWITCH_INDEPENDENT },
    { "host_update_wptr", MAC_TRX_HOST_UPDATE_WPTR },
    { "soft_irq_sched", MAC_TRX_SOFT_IRQ_SCHED },
    { "device_loop_sched", MAC_TRX_DEVICE_LOOP_SCHED },
    { NULL }
};
/* cmd: wlan0 set_str xxx  2  xxx 1 xxx 0 */
const wal_ioctl_str_stru g_ast_set_str_table[] = {
    { "11ax_debug", WLAN_CFGID_11AX_DEBUG, (wal_ioctl_tlv_stru *)&g_ast_11ax_debug_table },
    { "mac_report_tx", WLAN_CFGID_MAC_TX_COMMON_REPORT, (wal_ioctl_tlv_stru *)&g_ast_mac_tx_report_debug_table },
    { "mac_report_rx", WLAN_CFGID_MAC_RX_COMMON_REPORT, (wal_ioctl_tlv_stru *)&g_ast_mac_rx_report_debug_table },
    { "common_debug",  WLAN_CFGID_COMMON_DEBUG,         (wal_ioctl_tlv_stru *)&g_ast_common_debug_table },
    { "debug_log",     WLAN_CFGID_LOG_DEBUG,            (wal_ioctl_tlv_stru *)&g_ast_debug_log_table },
    { "multi_tid",     WLAN_CFGID_SET_MULTI_TID,        (wal_ioctl_tlv_stru *)&g_ast_multi_tid_table },
    { "trx_debug",     WLAN_CFGID_SET_TRX_DEBUG,        (wal_ioctl_tlv_stru *)&g_ast_trx_debug_table },
    { NULL }
};
const uint32_t g_set_str_table_size = oal_array_size(g_ast_set_str_table);

uint32_t wal_hipriv_get_tlv_table_idx(int8_t *ac_name, uint8_t *map_idx)
{
    wal_ioctl_tlv_stru st_tlv_cfg = {0};
    uint8_t idx = 0;

    /* 寻找匹配的命令 */
    st_tlv_cfg = g_ast_set_tlv_table[0];
    while (st_tlv_cfg.pc_name != NULL) {
        if (oal_strcmp(st_tlv_cfg.pc_name, ac_name) == 0) {
            break;
        }
        st_tlv_cfg = g_ast_set_tlv_table[++idx];
    }

    /* 没有找到对应的命令，则报错 */
    if (st_tlv_cfg.pc_name == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_set_val_cmd::invalid alg_cfg command!}\r\n");
        return OAL_FAIL;
    }

    *map_idx = idx;
    return OAL_SUCC;
}

void wal_hipriv_set_val_cmd_msg(uint16_t cfg_id, uint32_t value, uint8_t cmd_type, wal_msg_write_stru *write_msg)
{
    mac_cfg_set_tlv_stru *pst_set_tlv = (mac_cfg_set_tlv_stru *)(write_msg->auc_value);

    pst_set_tlv->uc_cmd_type = cmd_type;
    pst_set_tlv->us_cfg_id = cfg_id;
    pst_set_tlv->uc_len = sizeof(mac_cfg_set_tlv_stru);
    pst_set_tlv->value = value;

    oam_warning_log3(0, OAM_SF_ANY,
        "{wal_hipriv_set_val_cmd_msg:: cfg id[%d], cfg type[%d],set_val[%d]!}", cfg_id, cmd_type, value);
}


#define WAL_HIPRIV_TLV_CMD_ONLY_VAL 0
#define WAL_HIPRIV_TLV_CMD_WITH_TYPE 1
uint32_t wal_hipriv_set_tlv_val_cmd(oal_net_device_stru *net_dev, int8_t *param, uint8_t func_type)
{
    uint32_t off_set;
    int8_t name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint32_t ret;
    uint8_t map_index = 0;
    wal_msg_write_stru write_msg;
    uint8_t len = sizeof(mac_cfg_set_tlv_stru);
    uint16_t cfg_id;
    uint32_t value;
    uint8_t cmd_type = 0xFF;

    /* 获取配置参数名称 */
    ret = wal_get_cmd_one_arg(param, name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_val_cmd::get name err[%d]!}\r\n", ret);
        return ret;
    }
    param += off_set;

    ret = wal_hipriv_get_tlv_table_idx(name, &map_index);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_val_cmd::wal_hipriv_get_tlv_table_idx err[%d]!}\r\n", ret);
        return ret;
    }

    /* 记录命令对应的枚举值 */
    cfg_id = g_ast_set_tlv_table[map_index].en_tlv_cfg_id;

    /* 获取设定值 */
    ret = wal_get_cmd_one_arg(param, name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_val_cmd::get val err[%d]!}\r\n", ret);
        return ret;
    }
    param += off_set;

    if (func_type == WAL_HIPRIV_TLV_CMD_WITH_TYPE) {
        cmd_type = (uint8_t)oal_atoi(name);

        /* 获取设定置 */
        ret = wal_get_cmd_one_arg(param, name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
        if (ret != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_val_cmd::get type err[%d]!}\r\n", ret);
            return ret;
        }
        param += off_set;
    }

    /* 记录命令参数对应的赋值 */
    value = (uint32_t)oal_atoi(name);
    wal_hipriv_set_val_cmd_msg(cfg_id, value, cmd_type, &write_msg);

    /***************************************************************************
                             抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&write_msg, cfg_id, len);

    ret = (uint32_t)wal_send_cfg_event(net_dev, WAL_MSG_TYPE_WRITE, WAL_MSG_WRITE_MSG_HDR_LENGTH + len,
                                       (uint8_t *)&write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_set_val_cmd::wal_send_cfg_event return err_code [%d]!}\r\n", ret);
        return ret;
    }

    return OAL_SUCC;
}

/*
 * 功能描述  : 设置TLV类型命令统一接口
 * 1.日    期  : 2017年6月15日
  *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_set_tlv_cmd(oal_net_device_stru *net_dev, int8_t *param)
{
    return wal_hipriv_set_tlv_val_cmd(net_dev, param, WAL_HIPRIV_TLV_CMD_WITH_TYPE);
}

/*
 * 功能描述  : 设置某个变量赋值命令的统一接口
 * 1.日    期  : 2017年6月15日
  *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_set_val_cmd(oal_net_device_stru *net_dev, int8_t *param)
{
    return wal_hipriv_set_tlv_val_cmd(net_dev, param, WAL_HIPRIV_TLV_CMD_ONLY_VAL);
}

/*
 * 函 数 名  : wal_get_cmd_one_arg
 * 功能描述  : 获取字符串第一个参数 以空格为参数区分标识
 * 1.日    期  : 2012年12月13日
  *   修改内容  : 新生成函数
 */
uint32_t wal_get_cmd_one_arg(int8_t *pc_cmd, int8_t *pc_arg, uint32_t arg_len, uint32_t *pul_cmd_offset)
{
    int8_t *pc_cmd_copy = NULL;
    uint32_t pos = 0;

    if (oal_unlikely(oal_any_null_ptr3(pc_cmd, pc_arg, pul_cmd_offset))) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_get_cmd_one_arg::pc_cmd/pc_arg/pul_cmd_offset is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pc_cmd_copy = pc_cmd;

    /* 去掉字符串开始的空格 */
    while (*pc_cmd_copy == ' ') {
        ++pc_cmd_copy;
    }

    while ((*pc_cmd_copy != ' ') && (*pc_cmd_copy != '\0')) {
        pc_arg[pos] = *pc_cmd_copy;
        ++pos;
        ++pc_cmd_copy;

        if (oal_unlikely(pos >= arg_len)) {
            oam_warning_log1(0, OAM_SF_ANY, "{wal_get_cmd_one_arg::pos>=HIPRIV_CMD_NAME_MAX_LEN, pos:%d}", pos);
            return OAL_ERR_CODE_ARRAY_OVERFLOW;
        }
    }

    pc_arg[pos] = '\0';

    /* 字符串到结尾，返回错误码(命令码之后无) */
    if (pos == 0) {
        oam_info_log0(0, OAM_SF_ANY, "{wal_get_cmd_one_arg::return param pc_arg is null!}");
        return OAL_ERR_CODE_CONFIG_ARGS_OVER;
    }

    *pul_cmd_offset = (uint32_t)(pc_cmd_copy - pc_cmd);

    return OAL_SUCC;
}

/*
 * 功能描述  : init the wid response queue
 * 1.日    期  : 2015年11月10日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void wal_msg_queue_init(void)
{
    memset_s((void *)&g_wal_wid_msg_queue, sizeof(g_wal_wid_msg_queue), 0, sizeof(g_wal_wid_msg_queue));
    oal_dlist_init_head(&g_wal_wid_msg_queue.st_head);
    g_wal_wid_msg_queue.count = 0;
    oal_spin_lock_init(&g_wal_wid_msg_queue.st_lock);
    oal_wait_queue_init_head(&g_wal_wid_msg_queue.st_wait_queue);
}

/*
 * 功能描述  : return the count of the request message
 * 1.日    期  : 2015年11月10日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_get_request_msg_count(void)
{
    return g_wal_wid_msg_queue.count;
}

uint32_t wal_check_and_release_msg_resp(wal_msg_stru *rsp_msg)
{
    wal_msg_write_rsp_stru *pst_write_rsp_msg = NULL;
    if (rsp_msg != NULL) {
        uint32_t err_code;
        wlan_cfgid_enum_uint16 en_wid;
        pst_write_rsp_msg = (wal_msg_write_rsp_stru *)(rsp_msg->auc_msg_data);
        err_code = pst_write_rsp_msg->err_code;
        en_wid = pst_write_rsp_msg->en_wid;
        oal_free(rsp_msg);

        if (err_code != OAL_SUCC) {
            oam_warning_log2(0, OAM_SF_SCAN,
                "{wal_check_and_release_msg_resp::detect err code:[%u],wid:[%u]}", err_code, en_wid);
            return err_code;
        }
    }

    return OAL_SUCC;
}

/*
 * 功能描述  : add the request message into queue
 * 1.日    期  : 2015年11月10日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void wal_msg_request_add_queue(wal_msg_request_stru *msg)
{
    oal_spin_lock_bh(&g_wal_wid_msg_queue.st_lock);
    oal_dlist_add_tail(&msg->pst_entry, &g_wal_wid_msg_queue.st_head);
    g_wal_wid_msg_queue.count++;
    oal_spin_unlock_bh(&g_wal_wid_msg_queue.st_lock);
}

/*
 * 功能描述  : remove the request message into queue
 * 1.日    期  : 2015年11月10日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void wal_msg_request_remove_queue(wal_msg_request_stru *msg)
{
    oal_spin_lock_bh(&g_wal_wid_msg_queue.st_lock);
    g_wal_wid_msg_queue.count--;
    oal_dlist_delete_entry(&msg->pst_entry);
    oal_spin_unlock_bh(&g_wal_wid_msg_queue.st_lock);
}

/*
 * 功能描述  : set the request message response by the request message's addr
 * 1.日    期  : 2015年11月10日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
int32_t wal_set_msg_response_by_addr(uintptr_t addr, void *resp_mem, uint32_t resp_ret, uint32_t rsp_len)
{
    int32_t ret = -OAL_EINVAL;
    oal_dlist_head_stru *pos = NULL;
    oal_dlist_head_stru *entry_temp = NULL;
    wal_msg_request_stru *request = NULL;

    oal_spin_lock_bh(&g_wal_wid_msg_queue.st_lock);
    oal_dlist_search_for_each_safe(pos, entry_temp, (&g_wal_wid_msg_queue.st_head))
    {
        request = (wal_msg_request_stru *)oal_dlist_get_entry(pos, wal_msg_request_stru, pst_entry);
        if (request->request_address == addr) {
            /* address match */
            if (oal_unlikely(request->pst_resp_mem != NULL)) {
                oam_error_log0(0, OAM_SF_ANY,
                    "{wal_set_msg_response_by_addr::wal_set_msg_response_by_addr rsp had set!");
            }
            request->pst_resp_mem = resp_mem;
            request->ret = resp_ret;
            request->resp_len = rsp_len;
            ret = OAL_SUCC;
            break;
        }
    }

    oal_spin_unlock_bh(&g_wal_wid_msg_queue.st_lock);

    return ret;
}

/*
 * 功能描述  : WAL申请事件，并填充事件头
 * 1.日    期  : 2013年1月17日
  *   修改内容  : 新生成函数
 */
uint32_t wal_alloc_cfg_event(oal_net_device_stru *net_dev, frw_event_mem_stru **event_mem, const void *resp_addr,
    wal_msg_stru **cfg_msg, uint16_t len)
{
    mac_vap_stru       *vap     = NULL;
    frw_event_mem_stru *evt_mem = NULL;
    frw_event_stru     *event   = NULL;
    uint16_t           resp_len = 0;
    wal_msg_rep_hdr    *rep_hdr = NULL;

    vap = oal_net_dev_priv(net_dev);
    if (oal_unlikely(vap == NULL)) {
        /* 规避wifi关闭状态下，下发hipriv命令显示error日志 */
        oam_warning_log0(0, OAM_SF_ANY, "{wal_alloc_cfg_event::OAL_NET_DEV_PRIV(net_dev) null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    resp_len += sizeof(wal_msg_rep_hdr);
    len += resp_len;

    evt_mem = frw_event_alloc_m(len);
    if (oal_unlikely(evt_mem == NULL)) {
        oam_error_log2(vap->uc_vap_id, OAM_SF_ANY,
            "{wal_alloc_cfg_event::evt_mem null ptr error,request size:len:%d,resp_len:%d}", len, resp_len);
        return OAL_ERR_CODE_PTR_NULL;
    }

    *event_mem = evt_mem; /* 出参赋值 */
    event = frw_get_event_stru(evt_mem);
    /* 填写事件头 */
    frw_event_hdr_init(&(event->st_event_hdr), FRW_EVENT_TYPE_HOST_CRX, WAL_HOST_CRX_SUBTYPE_CFG, len,
        FRW_EVENT_PIPELINE_STAGE_0, vap->uc_chip_id, vap->uc_device_id, vap->uc_vap_id);
    /* fill the resp hdr */
    rep_hdr = (wal_msg_rep_hdr *)event->auc_event_data;
    if (resp_addr == NULL) {
        /* no response */
        rep_hdr->request_address = (uintptr_t)0;
    } else {
        /* need response */
        rep_hdr->request_address = (uintptr_t)resp_addr;
    }

    *cfg_msg = (wal_msg_stru *)((uint8_t *)event->auc_event_data + resp_len);

    return OAL_SUCC;
}

/*
 * 功能描述  : 判断wal response 完成条件是否满足
 * 1.日    期  : 2015年11月10日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC OAL_INLINE int32_t wal_request_wait_event_condition(wal_msg_request_stru *msg_stru)
{
    int32_t ret = OAL_FALSE;
    oal_spin_lock_bh(&g_wal_wid_msg_queue.st_lock);
    if ((msg_stru->pst_resp_mem != NULL) || (msg_stru->ret != OAL_SUCC)) {
        ret = OAL_TRUE;
    }
    oal_spin_unlock_bh(&g_wal_wid_msg_queue.st_lock);
    return ret;
}

void wal_cfg_msg_task_sched(void)
{
    oal_wait_queue_wake_up(&g_wal_wid_msg_queue.st_wait_queue);
}

int32_t wal_send_cfg_event_process(wal_msg_request_stru *msg_request, wal_msg_stru  *pst_cfg_msg,
    uint16_t us_len, oal_bool_enum_uint8 en_need_rsp, wal_msg_stru **response)
{
    wal_msg_stru       *rsp_msg = NULL;
    wal_msg_write_stru  *write_msg = NULL;
    int32_t           l_ret;
    /* 等待事件返回 */
    wal_wake_lock();

    l_ret = oal_wait_event_timeout_m((g_wal_wid_msg_queue.st_wait_queue),
        (oal_bool_enum_uint8)(OAL_TRUE == wal_request_wait_event_condition(msg_request)),
        (HMAC_WAIT_EVENT_RSP_TIME * OAL_TIME_HZ));

    /* response had been set, remove it from the list */
    if (en_need_rsp == OAL_TRUE) {
        wal_msg_request_remove_queue(msg_request);
    }
    if (oal_warn_on(l_ret == 0)) {
        write_msg = (wal_msg_write_stru *)pst_cfg_msg->auc_msg_data;
        /* 超时 */
        oam_error_log3(0, OAM_SF_ANY, "{wal_send_cfg_event:: wait queue timeout,%ds!cfg_id=%d,len=%d}",
            HMAC_WAIT_EVENT_RSP_TIME, write_msg->en_wid, write_msg->us_len);
        if (msg_request->pst_resp_mem != NULL) {
            oal_free(msg_request->pst_resp_mem);
        }
        wal_wake_unlock();
        declare_dft_trace_key_info("wal_send_cfg_timeout", OAL_DFT_TRACE_FAIL);
        /* 打印CFG EVENT内存，方便定位 */
        oal_print_hex_dump((uint8_t *)pst_cfg_msg, (WAL_MSG_WRITE_MSG_HDR_LENGTH + us_len),
            HEX_DUMP_GROUP_SIZE, "cfg event: ");
        /* 上行小包100%跑流，cpu 100%会触发打印，串口打印太多，取消打印 */
#if (_PRE_FRW_FEATURE_PROCCESS_ENTITY_TYPE == _PRE_FRW_FEATURE_PROCCESS_ENTITY_THREAD)
#ifndef CONFIG_HI110X_KERNEL_MODULES_BUILD_SUPPORT
        if (g_st_event_task[0].pst_event_kthread) {
            sched_show_task(g_st_event_task[0].pst_event_kthread);
        }
#endif
#endif
        return -OAL_ETIMEDOUT;
    }
    rsp_msg = (wal_msg_stru *)(msg_request->pst_resp_mem);
    if (rsp_msg == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_send_cfg_event:: msg mem null!}");
        // tscancode-suppress *
        *response = NULL;
        wal_wake_unlock();
        return -OAL_EFAUL;
    }

    if (rsp_msg->st_msg_hdr.us_msg_len == 0) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_send_cfg_event:: no msg resp!}");
        *response = NULL;
        oal_free(rsp_msg);
        wal_wake_unlock();
        return -OAL_EFAUL;
    }
    /* 发送配置事件返回的状态信息 */
    *response = rsp_msg;
    wal_wake_unlock();
    return OAL_SUCC;
}
/*
 *   功能描述  : WAL发送事件
 * 1.日    期  : 2013年6月6日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
int32_t wal_send_cfg_event(oal_net_device_stru *pst_net_dev, uint8_t msg_type, uint16_t us_len,
    uint8_t *puc_param, oal_bool_enum_uint8 en_need_rsp, wal_msg_stru **response)
{
    wal_msg_stru       *pst_cfg_msg = NULL;
    frw_event_mem_stru *pst_event_mem = NULL;
    uint32_t          ret;
    wal_msg_request_stru st_msg_request;

    if (response != NULL) {
        *response = NULL;
    }

    if (oal_warn_on((en_need_rsp == OAL_TRUE) && (response == NULL))) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_send_cfg_event::response == NULL!}");
        return -OAL_EINVAL;
    }

    /* 申请事件 */
    wal_msg_req_stru_init(&st_msg_request);
    ret = wal_alloc_cfg_event(pst_net_dev, &pst_event_mem, ((en_need_rsp == OAL_TRUE) ? &st_msg_request : NULL),
                              &pst_cfg_msg, WAL_MSG_WRITE_MSG_HDR_LENGTH + us_len);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_send_cfg_event::wal_alloc_cfg_event err %d!}", ret);
        return -OAL_ENOMEM;
    }

    /* 填写配置消息 */
    wal_cfg_msg_hdr_init(&(pst_cfg_msg->st_msg_hdr), msg_type, us_len, (uint8_t)wal_get_msg_sn());

    /* 填写WID消息 */
    if (EOK != memcpy_s(pst_cfg_msg->auc_msg_data, us_len, puc_param, us_len)) {
        oam_error_log0(0, OAM_SF_ANY, "wal_send_cfg_event::memcpy fail!");
        frw_event_free_m(pst_event_mem);
        return -OAL_EINVAL;
    }

    if (en_need_rsp == OAL_TRUE) {
        /* add queue before post event! */
        wal_msg_request_add_queue(&st_msg_request);
    }

    /* 分发事件 */
    frw_event_dispatch_event(pst_event_mem);
    frw_event_free_m(pst_event_mem);

    /* win32 UT模式，触发一次事件调度 */
#if (_PRE_TEST_MODE == _PRE_TEST_MODE_UT || _PRE_TEST_MODE == _PRE_TEST_MODE_ST)
    frw_event_process_all_event(0);
#endif

    if (en_need_rsp == OAL_FALSE) {
        return OAL_SUCC;
    }

    /* context can't in interrupt */
    if (oal_warn_on(oal_in_interrupt())) {
        declare_dft_trace_key_info("wal_cfg_in_interrupt", OAL_DFT_TRACE_EXCEP);
    }

    if (oal_warn_on(oal_in_atomic())) {
        declare_dft_trace_key_info("wal_cfg_in_atomic", OAL_DFT_TRACE_EXCEP);
    }

    return wal_send_cfg_event_process(&st_msg_request, pst_cfg_msg, us_len, en_need_rsp, response);
}

/*
 * 功能描述  : 02 host device_sruc配置接口，目前用于上下电流程
 * 1.日    期  : 2015年11月24日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC int32_t wal_host_dev_config(oal_net_device_stru *net_dev, wlan_cfgid_enum_uint16 en_wid)
{
    oal_wireless_dev_stru *pst_wdev = NULL;
    mac_wiphy_priv_stru *wiphy_priv = NULL;
    hmac_vap_stru *pst_cfg_hmac_vap = NULL;
    mac_device_stru *pst_mac_device = NULL;
    oal_net_device_stru *cfg_net_dev = NULL;

    int32_t ret;
    wal_msg_stru *rsp_msg = NULL;
    uint32_t err_code;
    wal_msg_write_stru write_msg = {0};

    pst_wdev = oal_netdevice_wdev(net_dev);
    if (pst_wdev == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_host_dev_config::pst_wdev is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    wiphy_priv = (mac_wiphy_priv_stru *)oal_wiphy_priv(pst_wdev->wiphy);
    if (wiphy_priv == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_host_dev_config::pst_mac_device is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    pst_mac_device = wiphy_priv->pst_mac_device;
    if (pst_mac_device == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_host_dev_config::pst_mac_device is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_cfg_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_device->uc_cfg_vap_id);
    if (pst_cfg_hmac_vap == NULL) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_host_dev_config::pst_cfg_hmac_vap is null vap_id:%d!}",
            pst_mac_device->uc_cfg_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    cfg_net_dev = pst_cfg_hmac_vap->pst_net_device;
    if (cfg_net_dev == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_host_dev_config::cfg_net_dev is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    /* 抛事件到wal层处理 */
    /* 填写消息 */
    wal_write_msg_hdr_init(&write_msg, en_wid, 0);

    /* 发送消息 */
    ret = wal_send_cfg_event(cfg_net_dev, WAL_MSG_TYPE_WRITE, WAL_MSG_WRITE_MSG_HDR_LENGTH,
                             (uint8_t *)&write_msg, OAL_TRUE, &rsp_msg);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_host_dev_config::wal_alloc_cfg_event err %d!}", ret);
        return ret;
    }

    /* 处理返回消息 */
    err_code = wal_check_and_release_msg_resp(rsp_msg);
    if (err_code != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_host_dev_config::hmac cfg vap h2d fail,err code[%u]", err_code);
        return -OAL_EINVAL;
    }

    return OAL_SUCC;
}

/*
 *   功能描述  : 02 host device_sruc的初始化接口，目前用于上下电流程
 * 1.日    期  : 2015年11月24日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
int32_t wal_host_dev_init(oal_net_device_stru *net_dev)
{
    return wal_host_dev_config(net_dev, WLAN_CFGID_HOST_DEV_INIT);
}

/*
 *   功能描述  : 02 host device_sruc的去初始化接口，目前用于上下电流程
 * 1.日    期  : 2015年11月26日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
int32_t wal_host_dev_exit(oal_net_device_stru *net_dev)
{
    return wal_host_dev_config(net_dev, WLAN_CFGID_HOST_DEV_EXIT);
}

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
void wal_vap_get_wlan_mode_para(wal_msg_write_stru *write_msg)
{
    mac_cfg_add_vap_param_stru *vap_param = (mac_cfg_add_vap_param_stru *)write_msg->auc_value;

    vap_param->bit_11ac2g_enable = wlan_chip_get_11ac2g_enable();

    vap_param->bit_disable_capab_2ght40 = g_wlan_cust.uc_disable_capab_2ght40;
}
#endif

void wal_set_wdev_iftype(oal_wireless_dev_stru *wdev,
    wlan_vap_mode_enum_uint8 vap_mode, wlan_p2p_mode_enum_uint8 p2p_mode)
{
    if (vap_mode == WLAN_VAP_MODE_BSS_AP) {
        wdev->iftype = NL80211_IFTYPE_AP;
    } else if (vap_mode == WLAN_VAP_MODE_BSS_STA) {
        wdev->iftype = NL80211_IFTYPE_STATION;
    }
    if (p2p_mode == WLAN_P2P_DEV_MODE) {
        wdev->iftype = NL80211_IFTYPE_P2P_DEVICE;
    } else if (p2p_mode == WLAN_P2P_CL_MODE) {
        wdev->iftype = NL80211_IFTYPE_P2P_CLIENT;
    } else if (p2p_mode == WLAN_P2P_GO_MODE) {
        wdev->iftype = NL80211_IFTYPE_P2P_GO;
    }
}

void wal_set_netdev_priv(oal_netdev_priv_stru *pst_netdev_priv)
{
#ifdef _PRE_CONFIG_HISI_110X_NAPI_DISABLE
    pst_netdev_priv->uc_napi_enable     = OAL_FALSE;
    pst_netdev_priv->uc_gro_enable      = OAL_FALSE;
#else
    pst_netdev_priv->uc_napi_enable = OAL_TRUE;
    pst_netdev_priv->uc_gro_enable = OAL_TRUE;
#endif
    pst_netdev_priv->uc_napi_weight = NAPI_POLL_WEIGHT_LEV1;
    pst_netdev_priv->queue_len_max = NAPI_NETDEV_PRIV_QUEUE_LEN_MAX;
    pst_netdev_priv->uc_state = 0;
    pst_netdev_priv->period_pkts = 0;
    pst_netdev_priv->period_start = 0;
}

/*
 * 功能描述  : 开启或关闭ampdu发送功能
 * 1.日    期  : 2013年8月27日
  *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_ampdu_tx_on(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    wal_msg_write_stru st_write_msg;
    uint32_t off_set;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint32_t ret;
    int32_t l_ret;
    mac_cfg_ampdu_tx_on_param_stru *pst_aggr_tx_on_param = NULL;
    uint8_t uc_aggr_tx_on;
    uint8_t uc_snd_type = 0;

    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_ampdu_tx_on::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ret);
        return ret;
    }
    pc_param += off_set;

    uc_aggr_tx_on = (uint8_t)oal_atoi(ac_name);
    /* 只有硬件聚合需要配置第二参数 */
    if ((uc_aggr_tx_on & BIT3) || (uc_aggr_tx_on & BIT2) || (uc_aggr_tx_on & BIT1)) {
        ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
        if (ret != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_ANY,
                "{wal_hipriv_ampdu_tx_on::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ret);
            return ret;
        }

        uc_snd_type = (uint8_t)oal_atoi(ac_name);
    }

    /***************************************************************************
                             抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&st_write_msg, WLAN_CFGID_AMPDU_TX_ON, sizeof(mac_cfg_ampdu_tx_on_param_stru));

    /* 设置配置命令参数 */
    pst_aggr_tx_on_param = (mac_cfg_ampdu_tx_on_param_stru *)(st_write_msg.auc_value);
    pst_aggr_tx_on_param->uc_aggr_tx_on = uc_aggr_tx_on;
    if (uc_snd_type > 1) {
        pst_aggr_tx_on_param->uc_snd_type = 1;
        pst_aggr_tx_on_param->en_aggr_switch_mode = AMPDU_SWITCH_BY_BA_LUT;
    } else {
        pst_aggr_tx_on_param->uc_snd_type = uc_snd_type;
        pst_aggr_tx_on_param->en_aggr_switch_mode = AMPDU_SWITCH_BY_DEL_BA;
    }

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_ampdu_tx_on_param_stru),
                               (uint8_t *)&st_write_msg,
                               OAL_FALSE,
                               NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_ampdu_tx_on::return err code [%d]!}\r\n", l_ret);
        return (uint32_t)l_ret;
    }
    return OAL_SUCC;
}

/*
 * 函 数 名  : wal_hipriv_find_cmd
 * 功能描述  : 查找私有命令表
 * 1.日    期  : 2012年12月13日
  *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_hipriv_find_cmd(int8_t *pc_cmd_name,
    uint8_t uc_cmd_name_len, wal_hipriv_cmd_entry_stru **pst_cmd_id)
{
    uint32_t en_cmd_idx;
    int ret;
    *pst_cmd_id = NULL;

    if (oal_unlikely(oal_any_null_ptr2(pc_cmd_name, pst_cmd_id))) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_hipriv_find_cmd::pc_cmd_name/puc_cmd_id is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (uc_cmd_name_len > WAL_HIPRIV_CMD_NAME_MAX_LEN) {
        return OAL_FAIL;
    }

    for (en_cmd_idx = 0; en_cmd_idx < oal_array_size(g_ast_hipriv_cmd); en_cmd_idx++) {
        ret = oal_strcmp(g_ast_hipriv_cmd[en_cmd_idx].pc_cmd_name, pc_cmd_name);
        if (ret == 0) {
            *pst_cmd_id = (wal_hipriv_cmd_entry_stru *)&g_ast_hipriv_cmd[en_cmd_idx];
            oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_find_cmd::wal_hipriv_find_cmd en_cmd_idx = %d.}", en_cmd_idx);
            return OAL_SUCC;
        }
    }

#ifdef WIFI_DEBUG_ENABLE
    for (en_cmd_idx = 0; en_cmd_idx < wal_hipriv_get_debug_cmd_size(); en_cmd_idx++) {
        ret = oal_strcmp(g_ast_hipriv_cmd_debug[en_cmd_idx].pc_cmd_name, pc_cmd_name);
        if (ret == 0) {
            *pst_cmd_id = (wal_hipriv_cmd_entry_stru *)&g_ast_hipriv_cmd_debug[en_cmd_idx];
            oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_find_cmd::wal_hipriv_find_cmd en_cmd_idx:%d.}", en_cmd_idx);

            return OAL_SUCC;
        }
    }
#endif
    oam_io_printk("cmd name[%s] is not exist. \r\n", pc_cmd_name);
    return OAL_FAIL;
}

/*
 * 函 数 名  : wal_hipriv_get_cmd_net_dev
 * 功能描述  : 获取命令对应的net_dev
 * 1.日    期  : 2012年12月18日
  *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_hipriv_get_cmd_net_dev(int8_t *pc_cmd, oal_net_device_stru **net_device, uint32_t *off_set)
{
    oal_net_device_stru *net_dev = NULL;
    int8_t ac_dev_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = { 0 };
    uint32_t result;

    if (oal_any_null_ptr3(pc_cmd, net_device, off_set)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_hipriv_get_cmd_net_dev::cmd/net_device/off_set is null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    result = wal_get_cmd_one_arg(pc_cmd, ac_dev_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, off_set);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_get_cmd_net_dev::get cmd one arg err[%d]!}", result);
        return result;
    }

    /* 根据dev_name找到dev */
    net_dev = wal_config_get_netdev(ac_dev_name, OAL_STRLEN(ac_dev_name));
    if (net_dev == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_get_cmd_net_dev::wal_config_get_netdev return null ptr!}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 调用oal_dev_get_by_name后，必须调用oal_dev_put使net_dev的引用计数减一 */
    oal_dev_put(net_dev);

    *net_device = net_dev;

    return OAL_SUCC;
}

/*
 * 函 数 名  : wal_hipriv_get_cmd_id
 * 功能描述  : 获取cmd id
 * 1.日    期  : 2012年12月18日
  *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t wal_hipriv_get_cmd_id(int8_t *pc_cmd,
    wal_hipriv_cmd_entry_stru **pst_cmd_id, uint32_t *off_set)
{
    int8_t ac_cmd_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    uint32_t result;

    if (oal_unlikely(oal_any_null_ptr3(pc_cmd, pst_cmd_id, off_set))) {
        oam_error_log0(0, OAM_SF_ANY,
            "{wal_hipriv_get_cmd_id::pc_cmd/puc_cmd_id/off_set is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    result = wal_get_cmd_one_arg(pc_cmd, ac_cmd_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, off_set);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_get_cmd_id::get cmd one arg err[%d]!}", result);
        return result;
    }

    /* 根据命令名找到命令枚举 */
    result = wal_hipriv_find_cmd(ac_cmd_name, sizeof(ac_cmd_name), pst_cmd_id);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_get_cmd_id::wal_hipriv_find_cmd return error cod [%d]!}", result);
        return result;
    }

    return OAL_SUCC;
}

/*
 * 函 数 名  : wal_hipriv_parse_cmd
 * 功能描述  : 解析私有配置命令
 * 1.日    期  : 2012年12月11日
  *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_parse_cmd(int8_t *pc_cmd)
{
    oal_net_device_stru *net_dev = NULL;
    wal_hipriv_cmd_entry_stru *pst_hipriv_cmd_entry = NULL;
    uint32_t off_set = 0;
    uint32_t result;
    if (oal_unlikely(pc_cmd == NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_hipriv_parse_cmd::pc_cmd null ptr error!}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    if (wal_process_cmd(pc_cmd) == OAL_SUCC) {
        return OAL_SUCC;
    }
    /*
        cmd格式约束
        网络设备名 命令      参数   Hisilicon0 create vap0
        1~15Byte   1~15Byte
    */
    result = wal_hipriv_get_cmd_net_dev(pc_cmd, &net_dev, &off_set);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_parse_cmd::wal_hipriv_get_cmd_net_dev error[%d]!}", result);
        return result;
    }

    pc_cmd += off_set;
    result = wal_hipriv_get_cmd_id(pc_cmd, &pst_hipriv_cmd_entry, &off_set);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_parse_cmd::wal_hipriv_get_cmd_id  error[%d]!}", result);
        return result;
    }

    pc_cmd += off_set;
    /* 调用命令对应的函数 */
    result = pst_hipriv_cmd_entry->p_func(net_dev, pc_cmd);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_parse_cmd::g_ast_hipriv_cmd  error[%d]!}", result);
        return result;
    }

    return OAL_SUCC;
}

#ifdef _PRE_CONFIG_CONN_HISI_SYSFS_SUPPORT
/*
 * 函 数 名  : wal_hipriv_sys_write  mp12-cb add sys for 51/02
 * 功能描述  : sys write函数
 * 1.日    期  : 2014年10月17日
  *   修改内容  : 新生成函数
 */
OAL_STATIC oal_ssize_t wal_hipriv_sys_write(struct kobject *dev, struct kobj_attribute *attr,
    const char *pc_buffer, oal_size_t count)
{
    int8_t *pc_cmd;
    uint32_t result;
    uint32_t len = (uint32_t)count;

    if (len > WAL_HIPRIV_CMD_MAX_LEN) {
        oam_error_log1(0, OAM_SF_ANY, "{wal_hipriv_sys_write::ul_len>WAL_HIPRIV_CMD_MAX_LEN, len [%d]!}", len);
        return -OAL_EINVAL;
    }

    pc_cmd = oal_mem_alloc_m(OAL_MEM_POOL_ID_LOCAL, WAL_HIPRIV_CMD_MAX_LEN, OAL_TRUE);
    if (oal_unlikely(pc_cmd == NULL)) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_proc_write::alloc mem return null ptr!}");
        return -OAL_ENOMEM;
    }

    memset_s(pc_cmd, WAL_HIPRIV_CMD_MAX_LEN, 0, WAL_HIPRIV_CMD_MAX_LEN);

    if (EOK != memcpy_s(pc_cmd, WAL_HIPRIV_CMD_MAX_LEN, pc_buffer, len)) {
        oam_error_log0(0, OAM_SF_ANY, "wal_hipriv_sys_write::memcpy fail!");
        oal_mem_free_m(pc_cmd, OAL_TRUE);
        return -OAL_EINVAL;
    }

    pc_cmd[len - 1] = '\0';

    result = wal_hipriv_parse_cmd(pc_cmd);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_proc_write::parse cmd return err code[%d]!}", result);
    }

    oal_mem_free_m(pc_cmd, OAL_TRUE);

    return (int32_t)len;
}

/*
 * 函 数 名  : wal_hipriv_sys_read  mp12-cb add sys for 51/02
 * 功能描述  : sys read函数 空函数；防止编译警告
 * 1.日    期  : 2014年10月17日
  *   修改内容  : 新生成函数
 */
#define SYS_READ_MAX_STRING_LEN (4096 - 40) /* 当前命令字符长度20字节内，预留40保证不会超出 */
OAL_STATIC oal_ssize_t wal_hipriv_sys_read(struct kobject *dev, struct kobj_attribute *attr, char *pc_buffer)
{
    uint32_t cmd_idx;
    uint32_t buff_index = 0;

    for (cmd_idx = 0; cmd_idx < oal_array_size(g_ast_hipriv_cmd); cmd_idx++) {
        buff_index += snprintf_s(pc_buffer + buff_index, (SYS_READ_MAX_STRING_LEN - buff_index),
                                 (SYS_READ_MAX_STRING_LEN - buff_index) - 1,
                                 "\t%s\n", g_ast_hipriv_cmd[cmd_idx].pc_cmd_name);
        if (buff_index > SYS_READ_MAX_STRING_LEN) {
            oam_error_log0(0, OAM_SF_ANY, "wal_hipriv_sys_read::snprintf_s error!");
            return buff_index;
        }
    }
#ifdef WIFI_DEBUG_ENABLE
    for (cmd_idx = 0; cmd_idx < wal_hipriv_get_debug_cmd_size(); cmd_idx++) {
        buff_index += snprintf_s(pc_buffer + buff_index, (SYS_READ_MAX_STRING_LEN - buff_index),
                                 (SYS_READ_MAX_STRING_LEN - buff_index) - 1,
                                 "\t%s\n", g_ast_hipriv_cmd_debug[cmd_idx].pc_cmd_name);
        if (buff_index > SYS_READ_MAX_STRING_LEN) {
            oam_error_log0(0, OAM_SF_ANY, "wal_hipriv_sys_read::snprintf_s error!");
            break;
        }
    }
#endif

    return buff_index;
}

#endif /* _PRE_OS_VERSION_LINUX */

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))
// use sys filesystem instead
#else
/*
 * 函 数 名  : wal_hipriv_proc_write
 * 功能描述  : proc write函数
 * 1.日    期  : 2012年12月10日
  *   修改内容  : 新生成函数
 */
OAL_STATIC int32_t wal_hipriv_proc_write(oal_file_stru *pst_file,
    const int8_t *pc_buffer, uint32_t len, void *p_data)
{
    int8_t *pc_cmd = NULL;
    uint32_t result;

    if (len > WAL_HIPRIV_CMD_MAX_LEN) {
        oam_error_log1(0, OAM_SF_ANY, "{wal_hipriv_proc_write::ul_len>WAL_HIPRIV_CMD_MAX_LEN, len [%d]!}", len);
        return -OAL_EINVAL;
    }

    pc_cmd = oal_mem_alloc_m(OAL_MEM_POOL_ID_LOCAL, WAL_HIPRIV_CMD_MAX_LEN, OAL_TRUE);
    if (oal_unlikely(pc_cmd == NULL)) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_proc_write::alloc mem return null ptr!}");
        return -OAL_ENOMEM;
    }

    memset_s(pc_cmd, WAL_HIPRIV_CMD_MAX_LEN, 0, WAL_HIPRIV_CMD_MAX_LEN);

    result = oal_copy_from_user((void *)pc_cmd, pc_buffer, len);
    /* copy_from_user函数的目的是从用户空间拷贝数据到内核空间，失败返回没有被拷贝的字节数，成功返回0 */
    if (result > 0) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_proc_write::oal_copy_from_user return result[%d]!}", result);
        oal_mem_free_m(pc_cmd, OAL_TRUE);

        return -OAL_EFAUL;
    }

    pc_cmd[len - 1] = '\0';

    result = wal_hipriv_parse_cmd(pc_cmd);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_proc_write::parse cmd return err code[%d]!}", result);
    }

    oal_mem_free_m(pc_cmd, OAL_TRUE);

    return (int32_t)len;
}
#endif
/*
 * 函 数 名  : wal_create_hipriv_proc
 * 功能描述  : 创建proc入口
 * 1.日    期  : 2012年12月10日
  *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_create_proc(void *p_proc_arg)
{
#ifdef _PRE_CONFIG_CONN_HISI_SYSFS_SUPPORT
    uint32_t result;
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))
    g_pst_proc_entry = NULL;
#else

    /* S_IRUSR文件所有者具可读取权限, S_IWUSR文件所有者具可写入权限, S_IRGRP用户组具可读取权限,
       S_IROTH其他用户具可读取权限 */
    /* 420代表十进制对应八进制是0644 linux模式定义 S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH); */
    g_pst_proc_entry = oal_create_proc_entry(WAL_HIPRIV_PROC_ENTRY_NAME, 420, NULL);
    if (g_pst_proc_entry == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_create_proc::oal_create_proc_entry return null ptr!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    g_pst_proc_entry->data = p_proc_arg;
    g_pst_proc_entry->nlink = 1; /* linux创建proc默认值 */
    g_pst_proc_entry->read_proc = NULL;

    g_pst_proc_entry->write_proc = (write_proc_t *)wal_hipriv_proc_write;
#endif

    /* mp12-cb add sys for 51/02 */
#ifdef _PRE_CONFIG_CONN_HISI_SYSFS_SUPPORT
    g_gp_sys_kobject = oal_get_sysfs_root_object();
    if (g_gp_sys_kobject == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_hipriv_create_proc::get sysfs root object failed!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    result = (uint32_t)oal_debug_sysfs_create_group(g_gp_sys_kobject, &g_hipriv_attribute_group);
    if (result) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_hipriv_create_proc::hipriv_attribute_group create failed!}");
        result = OAL_ERR_CODE_PTR_NULL;
        return result;
    }
#endif

    return OAL_SUCC;
}

/*
 * 函 数 名  : wal_hipriv_remove_proc
 * 功能描述  : 删除proc
 * 1.日    期  : 2012年12月10日
  *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_remove_proc(void)
{
    /*  */
    /* 卸载时删除sysfs */
#ifdef _PRE_CONFIG_CONN_HISI_SYSFS_SUPPORT
    if (g_gp_sys_kobject != NULL) {
        oal_debug_sysfs_remove_group(g_gp_sys_kobject, &g_hipriv_attribute_group);
        kobject_del(g_gp_sys_kobject);
        g_gp_sys_kobject = NULL;
    }
    oal_conn_sysfs_root_obj_exit();
    oal_conn_sysfs_root_boot_obj_exit();
#endif

    if (g_pst_proc_entry) {
        oal_remove_proc_entry(WAL_HIPRIV_PROC_ENTRY_NAME, NULL);
        g_pst_proc_entry = NULL;
    }

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_HS20
/*
 * 函 数 名  : wal_ioctl_set_qos_map
 * 功能描述  : 设置QoSMap信息元素
 * 1.日    期  : 2015年9月11日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC int32_t wal_ioctl_set_qos_map(oal_net_device_stru *net_dev, hmac_cfg_qos_map_param_stru *pst_qos_map_param)
{
    wal_msg_write_stru write_msg = {0};
    wal_msg_stru *rsp_msg = NULL;
    uint32_t err_code;
    int32_t ret;

    ret = memcpy_s(write_msg.auc_value, sizeof(write_msg.auc_value),
                   pst_qos_map_param, sizeof(hmac_cfg_qos_map_param_stru));
    if (ret != EOK) {
        oam_error_log0(0, OAM_SF_HS20, "wal_ioctl_set_qos_map::memcpy fail!");
        return -OAL_EFAIL;
    }

    /* 抛事件到wal层处理 */
    wal_write_msg_hdr_init(&write_msg, WLAN_CFGID_SET_QOS_MAP, sizeof(hmac_cfg_qos_map_param_stru));

    /* 发送消息 */
    ret = wal_send_cfg_event(net_dev,
                             WAL_MSG_TYPE_WRITE,
                             WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(hmac_cfg_qos_map_param_stru),
                             (uint8_t *)&write_msg,
                             OAL_TRUE,
                             &rsp_msg);
    if ((ret != OAL_SUCC) || (rsp_msg == NULL)) {
        oam_error_log1(0, OAM_SF_HS20, "{wal_ioctl_set_qos_map:: wal_alloc_cfg_event return err code %d!}", ret);
        return ret;
    }

    /* 读取返回的错误码 */
    err_code = wal_check_and_release_msg_resp(rsp_msg);
    if (err_code != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_HS20, "{wal_ioctl_set_qos_map::wal_check_and_release_msg_resp fail[%x!}",
                         err_code);
        return -OAL_EFAIL;
    }
    return OAL_SUCC;
}
#endif  // _PRE_WLAN_FEATURE_HS20

/*
 * 1.日    期  : 2014年7月29日
  *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_send_cfg_uint32_data(oal_net_device_stru *net_dev,
    int8_t *pc_param, uint8_t param_len, wlan_cfgid_enum_uint16 cfgid)
{
    wal_msg_write_stru write_msg = {0};
    int32_t ret;
    uint16_t len;
    uint32_t result;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = { 0 };
    uint32_t off_set = 0;
    uint32_t set_value;

    /* 抛事件到wal层处理 */
    memset_s(&write_msg, sizeof(wal_msg_write_stru), 0, sizeof(wal_msg_write_stru));
    result = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (result != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_send_cfg_uint32_data:wal_get_cmd_one_arg fail!}");
        return result;
    }

    pc_param += off_set;
    set_value = (uint32_t)oal_atoi((const int8_t *)ac_name);

    len = sizeof(uint32_t); /* sizeof(uint32_t) */
    *(uint32_t *)(write_msg.auc_value) = set_value;

    wal_write_msg_hdr_init(&write_msg, cfgid, len);

    ret = wal_send_cfg_event(net_dev, WAL_MSG_TYPE_WRITE, WAL_MSG_WRITE_MSG_HDR_LENGTH + len,
        (uint8_t *)&write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_CFG, "{wal_hipriv_send_cfg_uint32_data:send_cfg_event return[%d]}", ret);
        return (uint32_t)ret;
    }

    return OAL_SUCC;
}


#ifdef _PRE_WLAN_FEATURE_NAN
OAL_STATIC void wal_nan_netdev_setup(oal_net_device_stru *net_dev, oal_wireless_dev_stru *wdev,
    oal_wiphy_stru *wiphy)
{
    memset_s(wdev, sizeof(oal_wireless_dev_stru), 0, sizeof(oal_wireless_dev_stru));
    wdev->netdev = net_dev;
    wdev->iftype = NL80211_IFTYPE_STATION;
    wdev->wiphy = wiphy;

    net_dev->netdev_ops = &g_nan_net_dev_ops;
    net_dev->ethtool_ops = &g_st_wal_ethtool_ops;
    oal_netdevice_destructor(net_dev) = oal_net_free_netdev;
    oal_netdevice_ifalias(net_dev) = NULL;
    oal_netdevice_watchdog_timeo(net_dev) = 5;
    oal_netdevice_wdev(net_dev) = wdev;
    oal_netdevice_qdisc(net_dev, NULL);
    /* 将net device的flag设为down */
    oal_netdevice_flags(net_dev) &= ~OAL_IFF_RUNNING;
}

/*
 * 功能描述  : 初始化并注册nan0
 * 1.日    期  : 2020年4月23日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void wal_init_nan_netdev(oal_wiphy_stru *wiphy)
{
    oal_net_device_stru *net_dev = NULL;
    oal_wireless_dev_stru *wdev = NULL;
    int32_t ret;
    const char *dev_name = "nan0";

    if ((g_optimized_feature_switch_bitmap & BIT(CUSTOM_OPTIMIZE_FEATURE_NAN)) == 0) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_init_nan_netdev::NAN unsupported, Not create nan netdev!}");
        return;
    }
#if defined(CONFIG_HISI_CMDLINE_PARSE) || defined(CONFIG_CMDLINE_PARSE)
    if (get_boot_into_recovery_flag()) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_init_nan_netdev::recovery mode, Not create nan netdev!}");
        return;
    }
#endif
    /* 如果创建的net device已经存在，直接返回 */
    net_dev = wal_config_get_netdev(dev_name, OAL_STRLEN(dev_name));
    if (net_dev != NULL) {
        /* 调用oal_dev_get_by_name后，必须调用oal_dev_put使net_dev的引用计数减一 */
        oal_dev_put(net_dev);
        oam_warning_log0(0, OAM_SF_ANY, "{wal_init_nan_netdev::nan net_device is already exist!}");
        return;
    }

    /* 此函数第一个入参代表私有长度，此处不涉及为0 */
    net_dev = oal_net_alloc_netdev(0, dev_name, oal_ether_setup);
    if (oal_unlikely(net_dev == NULL)) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_init_nan_netdev::oal_net_alloc_netdev return null!}");
        return;
    }

    wdev = (oal_wireless_dev_stru *)oal_mem_alloc_m(OAL_MEM_POOL_ID_LOCAL,
        sizeof(oal_wireless_dev_stru), OAL_FALSE);
    if (oal_unlikely(wdev == NULL)) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_init_nan_netdev::alloc wdev fail!}");
        oal_net_free_netdev(net_dev);
        return;
    }

    wal_nan_netdev_setup(net_dev, wdev, wiphy);

    /* 注册net_device */
    ret = hmac_net_register_netdev(net_dev);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_init_nan_netdev::hmac_net_register_netdev err:%d!}", ret);
        oal_mem_free_m(wdev, OAL_FALSE);
        oal_net_free_netdev(net_dev);
        return;
    }
    oam_warning_log0(0, OAM_SF_ANY, "{wal_init_nan_netdev::add nan netdev succ!}");
}
#endif

/*
 * 函 数 名  : wal_hipriv_register_inetaddr_notifier
 * 功能描述  : 注册网络接口的通知链
 * 1.日    期  : 2015年5月21日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_register_inetaddr_notifier(void)
{
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    if (0 == register_inetaddr_notifier(&g_wal_hipriv_notifier)) {
        return OAL_SUCC;
    }

    oam_error_log0(0, OAM_SF_PWR, "{wal_hipriv_register_inetaddr_notifier::register inetaddr notifier failed.}");
    return OAL_FAIL;

#else
    return OAL_SUCC;
#endif
}

/*
 * 函 数 名  : wal_hipriv_unregister_inetaddr_notifier
 * 功能描述  : 注销网络接口的通知链
 * 1.日    期  : 2015年5月21日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_unregister_inetaddr_notifier(void)
{
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    if (0 == unregister_inetaddr_notifier(&g_wal_hipriv_notifier)) {
        return OAL_SUCC;
    }

    oam_error_log0(0, 0, "{wal_hipriv_unregister_inetaddr_notifier::hmac_unregister inetaddr notifier fail}");
    return OAL_FAIL;

#else
    return OAL_SUCC;
#endif
}

/*
 * 函 数 名  : wal_hipriv_register_inet6addr_notifier
 * 功能描述  : 注册IPv6网络接口的通知链
 * 1.日    期  : 2015年6月15日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_register_inet6addr_notifier(void)
{
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    if (0 == register_inet6addr_notifier(&g_wal_hipriv_notifier_ipv6)) {
        return OAL_SUCC;
    }

    oam_error_log0(0, OAM_SF_PWR, "{wal_hipriv_register_inet6addr_notifier::register inetaddr6 notifier failed.}");
    return OAL_FAIL;

#else
    return OAL_SUCC;
#endif
}

/*
 * 函 数 名  : wal_hipriv_unregister_inet6addr_notifier
 * 功能描述  : 注销IPv6网络接口的通知链
 * 1.日    期  : 2015年6月15日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_unregister_inet6addr_notifier(void)
{
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    if (0 == unregister_inet6addr_notifier(&g_wal_hipriv_notifier_ipv6)) {
        return OAL_SUCC;
    }

    oam_error_log0(0, OAM_SF_PWR, "{wal_hipriv_unregister_inet6addr_notifier::unreg inetaddr6 notifier fail}");
    return OAL_FAIL;

#else
    return OAL_SUCC;
#endif
}

/*
 * 函 数 名   : wal_split_cmd_line
 *              uc_args_num
 *              apc_args
 * 1.日    期   : 2018年6月20日
 *   作    者   :     wifi
 *   修改内容   : 新生成函数
 */
uint32_t wal_split_cmd_line(char *args, uint16_t len, wlan_cfg_param_stru *wlan_cfg_param,
    char (*cfg_args)[WLAN_CFG_MAX_LEN_EACH_ARG])
{
    uint32_t offset = 0;
    char  *copy_args = NULL;
    uint32_t ret;
    uint8_t index;

    if (args == NULL || wlan_cfg_param == NULL || cfg_args == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    args[len - 1] = '\0';

    copy_args = args;

    for (index = 0; index < WLAN_CFG_MAX_ARGS_NUM; index++) {
        copy_args += offset;

        ret = alg_cfg_get_cmd_one_arg(copy_args, cfg_args[index], WLAN_CFG_MAX_LEN_EACH_ARG, &offset);
        if (ret == OAL_SUCC) {
            wlan_cfg_param->apc_args[index] = cfg_args[index];
            wlan_cfg_param->uc_args_num++;
        } else if (ret == OAL_ERR_CODE_CONFIG_ARGS_OVER) {
            break;
        } else {
            return ret;
        }
    }

    return OAL_SUCC;
}

/*
 * 函 数 名   : wal_algcmd_char_extra_adapt
 * 功能描述   : iwpriv char extra型  适配层入口
 * 1.日    期   : 2019年10月31日
 *   修改内容   : 新生成函数
 */
uint32_t wal_algcmd_char_extra_adapt(oal_net_device_stru *net_dev, const char *in_param, signed long in_len,
    char *out_param, signed long out_len)
{
    uint16_t input_length; /* including '\0' */
    uint32_t ret = OAL_SUCC;
    wlan_cfg_param_stru wlan_cfg_param = { 0 };
    char (*cfg_args)[WLAN_CFG_MAX_LEN_EACH_ARG] = NULL;
    char *output = NULL;
    char args_str[IOCTL_IWPRIV_WLAN_CFG_CMD_MAX_LEN] = { 0 };
    errno_t result;

    if (oal_unlikely(in_param == NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    input_length = (uint16_t)OAL_STRLEN(in_param) + 1;

    result = memcpy_s(args_str, IOCTL_IWPRIV_WLAN_CFG_CMD_MAX_LEN, in_param, input_length);
    if (result != EOK) {
        oam_error_log2(0, OAM_SF_CALIBRATE, "copy fail len[%d] ret[%]", input_length, ret);
        return OAL_FAIL;
    }

    cfg_args = (char(*)[WLAN_CFG_MAX_LEN_EACH_ARG])oal_memalloc(WLAN_CFG_MAX_ARGS_NUM * WLAN_CFG_MAX_LEN_EACH_ARG);
    if (cfg_args == NULL) {
        oam_error_log0(0, OAM_SF_CFG, "wal_iwpriv_char_extra_adapt:alloc error: pointer null!");
        return OAL_FAIL;
    }

    output = (char *)oal_memalloc(WLAN_CFG_MAX_RSP_LEN);
    if (output == NULL) {
        oam_error_log0(0, OAM_SF_CFG, "wal_iwpriv_char_extra_adapt:alloc error: pointer null!");
        oal_free(cfg_args);
        return OAL_FAIL;
    }

    memset_s(cfg_args, WLAN_CFG_MAX_ARGS_NUM * WLAN_CFG_MAX_LEN_EACH_ARG, 0,
             WLAN_CFG_MAX_ARGS_NUM * WLAN_CFG_MAX_LEN_EACH_ARG);
    memset_s(output, WLAN_CFG_MAX_RSP_LEN, 0, WLAN_CFG_MAX_RSP_LEN);

    wlan_cfg_param.pst_net_dev = net_dev;
    wlan_cfg_param.en_wlan_cfg_id = WLAN_CFGID_ALG_ENTRY;
    wlan_cfg_param.pc_output = output;
    wlan_cfg_param.us_output_len = 0;

    ret = wal_split_cmd_line((char *)args_str, input_length, &wlan_cfg_param, cfg_args);
    if (ret == OAL_SUCC) {
        /* main process: call wlan_cfg module */
        ret = (uint32_t)wal_wlan_cfg_module_process_entry(&wlan_cfg_param);
    } else {
        oam_error_log1(0, OAM_SF_CFG, "{wal_algcmd_char_extra_adapt:wal_split_cmd_line fail[%d].}\r\n", ret);
    }

    /* format output according to specific requirement */
    oal_free(cfg_args);
    oal_free(output);

    return ret;
}

OAL_STATIC int32_t wal_ioctl_set_compete_ops(oal_net_device_stru *net_dev, wlan_compete_mode_stru *compete_param)
{
    wal_msg_write_stru write_msg;
    int32_t ret;

    if (memcpy_s(write_msg.auc_value, sizeof(write_msg.auc_value),
                     compete_param, sizeof(wlan_compete_mode_stru)) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "wal_ioctl_set_compete_ops::memcpy fail!");
        return -OAL_EFAIL;
    }
    /* 抛事件到wal层处理 */
    wal_write_msg_hdr_init(&write_msg, WLAN_CFGID_COMPETE_ASCEND, sizeof(wlan_compete_mode_stru));
    ret = wal_send_cfg_event(net_dev, WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(wlan_compete_mode_stru), (uint8_t *)&write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_error_log1(0, OAM_SF_ANY, "wal_ioctl_tas_rssi_access::send cfg event fail[%d]!", ret);
    }
    return OAL_SUCC;
}

OAL_STATIC int32_t wal_ioctl_priv_cmd_compete_mode(oal_net_device_stru *net_dev,
    int8_t *command, wal_wifi_priv_cmd_stru priv_cmd)
{
    wlan_compete_mode_stru compete_params = { 0 };
    if (wal_ioctl_judge_input_param_length(priv_cmd, CMD_SET_COMPETE_MODE_LEN,
        sizeof(wlan_compete_mode_stru)) != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_ioctl_priv_cmd_compete_mode::CMD_SET_COMPETE_MODE_LEN cmd len error}");
        return -OAL_EFAIL;
    }

    if (memcpy_s(&compete_params, sizeof(wlan_compete_mode_stru),
        command + CMD_SET_COMPETE_MODE_LEN + 1, sizeof(wlan_compete_mode_stru)) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "wal_ioctl_priv_cmd_compete_mode::memcpy fail!");
        return -OAL_EFAIL;
    }

    oam_warning_log1(0, OAM_SF_ANY, "{wal_ioctl_priv_cmd_compete_mode::CMD_SET_COMPETE_MODE_LEN enable[%d].}",
        compete_params.compete_enable);
    return wal_ioctl_set_compete_ops(net_dev, &compete_params);
}

OAL_STATIC int32_t wal_ioctl_priv_cmd_pkt_filter(oal_net_device_stru *net_dev,
    int8_t *command, wal_wifi_priv_cmd_stru priv_cmd)
{
    uint16_t program_len = 0;
    mac_apf_filter_cmd_stru apf_filter_cmd;
    wal_msg_write_stru write_msg;
    wal_msg_stru *rsp_msg = NULL;

    if (wal_ioctl_judge_input_param_length(priv_cmd, CMD_SET_PKT_FILTER_LEN,
        sizeof(program_len)) != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_ioctl_priv_cmd_pkt_filter::CMD_SET_PKT_FILTER_LEN cmd len error}");
        return -OAL_EFAIL;
    }

    program_len = *((uint16_t *)(command + CMD_SET_PKT_FILTER_LEN + 1));
    if (oal_unlikely(!program_len || program_len > APF_PROGRAM_MAX_LEN)) {
        oam_error_log1(0, OAM_SF_ANY, "wal_ioctl_priv_cmd_pkt_filter::program len[%d] invalid", program_len);
        return -OAL_EINVAL;
    }

    if (wal_ioctl_judge_input_param_length(priv_cmd, CMD_SET_PKT_FILTER_LEN,
        sizeof(program_len) + program_len) != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_ioctl_priv_cmd_pkt_filter::program data len error}");
        return -OAL_EFAIL;
    }
    apf_filter_cmd.us_program_len = program_len;
    apf_filter_cmd.puc_program = command + CMD_SET_PKT_FILTER_LEN + 1 + sizeof(program_len);
    apf_filter_cmd.en_cmd_type = APF_SET_FILTER_CMD;

    /***************************************************************************
        抛事件到wal层处理
    ***************************************************************************/
    wal_write_msg_hdr_init(&write_msg, WLAN_CFGID_SET_APF_FILTER, sizeof(apf_filter_cmd));
    if (memcpy_s(write_msg.auc_value, sizeof(write_msg.auc_value),
        &apf_filter_cmd, sizeof(apf_filter_cmd)) != EOK) {
        return -OAL_EFAIL;
    }

    /* 发送消息 */
    /* 需要将发送该函数设置为同步，否则hmac处理时会使用已释放的内存 */
    if (wal_send_cfg_event(net_dev, WAL_MSG_TYPE_WRITE, WAL_MSG_WRITE_MSG_HDR_LENGTH +
        sizeof(apf_filter_cmd), (uint8_t *)&write_msg, OAL_TRUE, &rsp_msg) != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_ioctl_priv_cmd_pkt_filter::wal_send_cfg_event fail!}");
        return -OAL_EFAIL;
    }
    if (wal_check_and_release_msg_resp(rsp_msg) != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_ioctl_priv_cmd_pkt_filter::check_and_release_msg_resp fail!}");
        return -OAL_EFAIL;
    }
    oam_warning_log1(0, 0, "wal_ioctl_priv_cmd_pkt_filter: program len %d", apf_filter_cmd.us_program_len);
    return OAL_SUCC;
}

#ifdef _PRE_WLAN_CHBA_MGMT
OAL_STATIC void wal_set_rx_mgmt_remain_on_chann_param(wlan_remain_on_channel_info_stru *remain_on_chan_info,
    struct ieee80211_channel *chan_info, uint32_t *duration)
{
    chan_info->center_freq = remain_on_chan_info->center_freq;
    chan_info->band = remain_on_chan_info->band;
    *duration = remain_on_chan_info->duration;
}

/* 功能描述: 上层用于接收管理帧而发起remain_on_channel的处理接口 */
OAL_STATIC int32_t wal_ioctl_rx_mgmt_remain_on_channel(oal_net_device_stru *net_dev,
    int8_t *command, wal_wifi_priv_cmd_stru priv_cmd)
{
    uint64_t cookie = 0;
    uint32_t duration = 0;
    mac_vap_stru *mac_vap = NULL;
    wlan_remain_on_channel_info_stru *remain_on_chan_info = NULL;
    struct ieee80211_channel chan_info = { 0 };
    oal_wireless_dev_stru *wdev = net_dev->ieee80211_ptr;
    if (wdev == NULL) {
        oam_warning_log0(0, OAM_SF_P2P, "{wal_ioctl_rx_mgmt_remain_on_channel:fail to get wdev!}");
        return -OAL_EFAIL;
    }

    /* 定制化未开启action协商优化时，此接口不应被调用 */
    if ((g_expand_feature_switch_bitmap & CUSTOM_ACTION_NEGOTIATION_OPTIMIZATION) == 0) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_ioctl_rx_mgmt_remain_on_channel:not support action optimization}");
        return -OAL_EFAIL;
    }
    /* 命令长度校验 */
    if (wal_ioctl_judge_input_param_length(priv_cmd, CMD_RX_MGMT_REMAIN_ON_CHANNEL_LEN,
        sizeof(wlan_remain_on_channel_info_stru)) != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ANY, "{wal_ioctl_rx_mgmt_remain_on_channel:cmd[%d] too short!}", priv_cmd.total_len);
        return -OAL_EFAIL;
    }

    mac_vap = oal_net_dev_priv(net_dev);
    if (mac_vap == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_ioctl_rx_mgmt_remain_on_channel:get mac vap failed!}");
        return -OAL_EFAIL;
    }

    /* 获取信道参数 */
    remain_on_chan_info = (wlan_remain_on_channel_info_stru *)(command + CMD_RX_MGMT_REMAIN_ON_CHANNEL_LEN + 1);
    wal_set_rx_mgmt_remain_on_chann_param(remain_on_chan_info, &chan_info, &duration);

    oam_warning_log3(mac_vap->uc_vap_id, OAM_SF_ANY,
        "{wal_ioctl_rx_mgmt_remain_on_channel:listen_freq[%d], bw[%d], time[%d]}",
        chan_info.center_freq, chan_info.band, duration);
    /* 开始信道监听 */
    return wal_drv_remain_on_channel(wdev, &chan_info, duration, &cookie, IEEE80211_ROC_TYPE_HIGH_PRIO_MGMT_RX);
}
#endif

/*
 * 函 数 名  : wal_ioctl_priv_cmd_ext
 * 日    期  : 2021年6月29日
 * 作    者  : wifi
 */
int32_t wal_ioctl_priv_cmd_ext(oal_net_device_stru *pst_net_dev, oal_ifreq_stru *p_ifr,
    uint8_t *pc_cmd, wal_wifi_priv_cmd_stru *priv_cmd)
{
    if (oal_strncasecmp(pc_cmd, CMD_GET_CURRENT_TSF, CMD_GET_CURRENT_TSF_LEN) == 0) {
        return wal_ioctl_get_current_tsf(pst_net_dev, p_ifr, priv_cmd);
#ifdef _PRE_WLAN_CHBA_MGMT
    } else if (oal_strncasecmp(pc_cmd, CMD_CHBA_MODULE_INIT, CMD_CHBA_MODULE_INIT_LEN) == 0) {
        return wal_ioctl_chba_module_init(pst_net_dev, pc_cmd, *priv_cmd);
    } else if (oal_strncasecmp(pc_cmd, CMD_SET_BATTERY_LEVEL, CMD_SET_BATTERY_LEVEL_LEN) == 0) {
        return wal_ioctl_set_battery_level(pst_net_dev, pc_cmd, *priv_cmd);
    } else if (oal_strncasecmp(pc_cmd, CMD_SET_CHAN_ADJUST, CMD_SET_CHAN_ADJUST_LEN) == 0) {
        return wal_ioctl_chba_channel_adjust(pst_net_dev, pc_cmd, *priv_cmd);
    } else if (oal_strncasecmp(pc_cmd, CMD_SET_SUPP_COEX_CHAN_LIST, CMD_SET_SUPP_COEX_CHAN_LIST_LEN) == 0) {
        return wal_ioctl_chba_set_supp_coex_chan_list(pst_net_dev, pc_cmd, *priv_cmd);
    } else if (oal_strncasecmp(pc_cmd, CMD_RX_MGMT_REMAIN_ON_CHANNEL, CMD_RX_MGMT_REMAIN_ON_CHANNEL_LEN) == 0) {
        return wal_ioctl_rx_mgmt_remain_on_channel(pst_net_dev, pc_cmd, *priv_cmd);
#endif
    } else if (oal_strncasecmp(pc_cmd, CMD_SET_COMPETE_MODE, CMD_SET_COMPETE_MODE_LEN) == 0) {
        return wal_ioctl_priv_cmd_compete_mode(pst_net_dev, pc_cmd, *priv_cmd);
    } else if (oal_strncasecmp(pc_cmd, CMD_SET_PKT_FILTER, CMD_SET_PKT_FILTER_LEN) == 0) {
        return wal_ioctl_priv_cmd_pkt_filter(pst_net_dev, pc_cmd, *priv_cmd);
    }
    return OAL_SUCC;
}

uint32_t wal_ioctl_set_al_rx(oal_net_device_stru *net_dev, int8_t *pc_param)
{
    wal_msg_write_stru write_msg = {0};
    uint32_t off_set = 0;
    uint32_t result;
    int32_t ret;
    int8_t ac_arg[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint8_t uc_rx_flag;
    int32_t l_idx = 0;
    wal_msg_stru *rsp_msg = NULL;

    /* 获取常收模式开关标志 */
    result = wal_get_cmd_one_arg(pc_param, ac_arg, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_always_rx::get cmd one arg err[%d]!}", result);
        return result;
    }

    /* 输入命令合法性检测 */
    while (ac_arg[l_idx] != '\0') {
        if (isdigit(ac_arg[l_idx])) {
            l_idx++;
            continue;
        } else {
            l_idx++;
            oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_always_rx::input illegal!}");
            return OAL_ERR_CODE_INVALID_CONFIG;
        }
    }

    /* 将命令参数值字符串转化为整数 */
    uc_rx_flag = (uint8_t)oal_atoi(ac_arg);
    if (uc_rx_flag > HAL_ALWAYS_RX_RESERVED) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_always_rx::input should be 0 or 1.}");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    if (uc_rx_flag && !wal_sniffer_is_single_sta_mode(net_dev)) {
        return OAL_FAIL;
    }
    *(uint8_t *)(write_msg.auc_value) = uc_rx_flag;

    wal_write_msg_hdr_init(&write_msg, WLAN_CFGID_SET_ALWAYS_RX, sizeof(uint8_t));
    ret = wal_send_cfg_event(net_dev, WAL_MSG_TYPE_WRITE,
                             WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(uint8_t),
                             (uint8_t *)&write_msg, OAL_TRUE, &rsp_msg);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_always_rx::err [%d]!}", ret);
        return (uint32_t)ret;
    }
    /* 读取返回的错误码 */
    result = wal_check_and_release_msg_resp(rsp_msg);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_DFR, "{wal_hipriv_always_rx fail, err code[%u]!}", result);
        return result;
    }

    return OAL_SUCC;
}
