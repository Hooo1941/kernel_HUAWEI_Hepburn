/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明   : 定义HOST/DEVICE传递命令枚举 的头文件
 * 作者       :
 * 创建日期   : 2020年6月19日
 */

#ifndef WLAN_CFG_ID1_H
#define WLAN_CFG_ID1_H

#include "wlan_cfg_id_2.h"
#include "oal_ext_if.h"
/*****************************************************************************
    配置命令 ID
    第一段  MIB 类配置
    第二段  非MIB类配置
*****************************************************************************/
typedef enum {
    WLAN_CFGID_1_START = 500,
    WLAN_CFGID_THRUPUT_INFO = 501, /* 吞吐量数据由dmac同步到hmac */
    WLAN_CFGID_COUNTRY_FOR_DFS = 503, /* 设置国家码管制域dfs信息 */
    WLAN_CFGID_SET_P2P_PS_OPS = 504,  /* 配置P2P OPS节能 */
    WLAN_CFGID_SET_P2P_PS_NOA = 505,  /* 配置P2P NOA节能 */
    WLAN_CFGID_SET_P2P_PS_STAT = 506, /* 配置P2P 节能统计 */
#ifdef _PRE_WLAN_FEATURE_HS20
    WLAN_CFGID_SET_QOS_MAP = 507, /* 配置HotSpot 2.0 QoSMap参数 */
#endif
    WLAN_CFGID_SET_P2P_MIRACAST_STATUS = 508, /* 配置P2P投屏状态 mp16已删除 */
    WLAN_CFGID_SET_LP_AL_RX_MODE = 509,   /* 固定当前LP通道常收模式 */

    WLAN_CFGID_UAPSD_UPDATE = 510,

    WLAN_CFGID_SDIO_FLOWCTRL = 523,
    WLAN_CFGID_NSS = 524, /* 空间流信息的同步 */

    WLAN_CFGID_ENABLE_ARP_OFFLOAD = 526,   /* arp offload的配置事件 mp16已删除 */
    WLAN_CFGID_SET_IP_ADDR = 527,          /* IPv4/IPv6地址的配置事件 */
    WLAN_CFGID_SHOW_ARPOFFLOAD_INFO = 528, /* 打印device侧的 mp16已删除 */

    WLAN_CFGID_CFG_VAP_H2D = 529,   /* 配置vap下发device事件 */
    WLAN_CFGID_HOST_DEV_INIT = 530, /* 下发初始化host dev init事件 */
    WLAN_CFGID_HOST_DEV_EXIT = 531, /* 下发去初始化host dev exit事件 */

    WLAN_CFGID_AMPDU_MMSS = 532,

#ifdef _PRE_WLAN_TCP_OPT
    WLAN_CFGID_GET_TCP_ACK_STREAM_INFO = 533, /* 显示TCP ACK 过滤统计值 */
    WLAN_CFGID_TX_TCP_ACK_OPT_ENALBE = 534,   /* 设置发送TCP ACK优化使能 */
    WLAN_CFGID_RX_TCP_ACK_OPT_ENALBE = 535,   /* 设置接收TCP ACK优化使能 */
    WLAN_CFGID_TX_TCP_ACK_OPT_LIMIT = 536,    /* 设置发送TCP ACK LIMIT */
    WLAN_CFGID_RX_TCP_ACK_OPT_LIMIT = 537,    /* 设置接收TCP ACK LIMIT  */
#endif

    WLAN_CFGID_SET_MAX_USER = 538, /* 设置最大用户数 */
    WLAN_CFGID_GET_STA_LIST = 539, /* 设置最大用户数 */
    WLAN_CFGID_BTCOEX_RX_DELBA_TRIGGER = 540,
#ifdef _PRE_WLAN_DFT_STAT
    WLAN_CFGID_SET_PERFORMANCE_LOG_SWITCH = 541, /* 设置性能打印控制开关 */ /* 废弃 */
#endif

#if (defined(_PRE_WLAN_FEATURE_WAPI) || defined(_PRE_WLAN_FEATURE_PWL))
    WLAN_CFGID_WPI_INFO = 542,
#endif
#ifdef _PRE_WLAN_FEATURE_WAPI
    WLAN_CFGID_ADD_WAPI_KEY = 543,
#endif

#ifdef _PRE_WLAN_DFT_STAT
    WLAN_CFGID_QUERY_ANI = 544, /* 查询VAP抗干扰参数 mp16已删除 */
#endif
    WLAN_CFGID_DISABLE_DECR_ENCR = 545, /* 关闭芯片加解密能力 */

    WLAN_CFGID_ROAM_ENABLE = 550, /* 漫游使能 */
    WLAN_CFGID_ROAM_START = 553,  /* 漫游开始 */
    WLAN_CFGID_SET_ROAMING_MODE = 555,
    WLAN_CFGID_SET_ROAM_TRIGGER = 556,
    WLAN_CFGID_ROAM_HMAC_SYNC_DMAC = 557,
    WLAN_CFGID_SET_FT_IES = 558,
    WLAN_CFGID_ROAM_NOTIFY_STATE = 559,
    WLAN_CFGID_ROAM_SUCC_H2D_SYNC = 560,
    WLAN_CFGID_CFG80211_SET_PMKSA = 561,     /* 设置PMK缓存 */
    WLAN_CFGID_CFG80211_DEL_PMKSA = 562,     /* 删除PMK缓存 */
    WLAN_CFGID_CFG80211_FLUSH_PMKSA = 563,   /* 清空PMK缓存 */
    WLAN_CFGID_CFG80211_EXTERNAL_AUTH = 564, /* 触发SAE认证 */

    WLAN_CFGID_SET_PM_SWITCH = 570, /* 全局低功耗使能去使能 */
    WLAN_CFGID_SET_DEVICE_FREQ = 571, /* 设置device主频 */
    WLAN_CFGID_SET_POWER_TEST = 572,      /* 功耗测试模式使能 mp16已删除 */
    WLAN_CFGID_SET_DEVICE_PKT_STAT = 573, /* 设置device收帧统计 */

    WLAN_CFGID_2040BSS_ENABLE = 574, /* 20/40 bss判断使能开关 */
    WLAN_CFGID_DESTROY_VAP = 575,

    WLAN_CFGID_SET_ANT = 576, /* 设置使用的天线 */
    WLAN_CFGID_GET_ANT = 577, /* 获取天线状态 mp16已删除 */

    WLAN_CFGID_GREEN_AP_EN = 578,

    WLAN_CFGID_SET_TX_AMPDU_TYPE = 583,
    WLAN_CFGID_SET_RX_AMPDU_AMSDU = 584,
    WLAN_CFGID_SET_ADDBA_RSP_BUFFER = 585,
    WLAN_CFGID_SET_SK_PACING_SHIFT = 586,
    WLAN_CFGID_SET_TRX_STAT_LOG = 587,
    WLAN_CFGID_MIMO_COMPATIBILITY = 588, /* mimo兼容性AP */
    WLAN_CFGID_DATA_COLLECT = 589, /* 数采开关，模式配置 */
    WLAN_CFGID_SET_RX_ANTI_IMMU = 590,
    /* HISI-CUSTOMIZE */
    WLAN_CFGID_SET_CUS_DYN_CALI_PARAM = 605, /* 动态校准参数定制化 mp16已删除 */
    WLAN_CFGID_SET_ALL_LOG_LEVEL = 606,      /* 配置所有vap log level mp16已删除 */
    WLAN_CFGID_SET_CUS_RF = 607,             /* RF定制化 mp16已删除 */
    WLAN_CFGID_SET_CUS_DTS_CALI = 608,       /* DTS校准定制化 mp16已删除 */
    WLAN_CFGID_SET_CUS_NVRAM_PARAM = 609,    /* NVRAM参数定制化 mp16已删除 */
    /* HISI-CUSTOMIZE INFOS */
    WLAN_CFGID_SHOW_DEV_CUSTOMIZE_INFOS = 610, /* show device customize info mp16已删除 */

#ifdef _PRE_WLAN_FEATURE_DFR
    WLAN_CFGID_TEST_DFR_START = 611,
#endif  // _PRE_WLAN_FEATURE_DFR

    WLAN_CFGID_WFA_CFG_AIFSN = 612,
    WLAN_CFGID_WFA_CFG_CW = 613,

#ifdef _PRE_WLAN_FEATURE_TX_CLASSIFY_LAN_TO_WLAN
    WLAN_CFGID_TX_CLASSIFY_LAN_TO_WLAN_SWITCH = 614, /* 设置业务识别功能开关 */
#endif
    WLAN_CFGID_REDUCE_SAR = 615, /* 通过降低发射功率来降低SAR */
    WLAN_CFGID_DBB_SCALING_AMEND = 616, /* 调整dbb scaling值 */
    WLAN_CFGID_TAS_PWR_CTRL = 620, /* TAS功率控制发射 */
    WLAN_CFGID_SET_VENDOR_IE = 630, /* hostapd 设置IE元素到VAP */

#ifdef _PRE_WLAN_FEATURE_11K
    WLAN_CFGID_11K_SCAN_FAIL_RESTART_ROAM = 643,
    WLAN_CFGID_SEND_NEIGHBOR_REQ = 644,
    WLAN_CFGID_REQ_SAVE_BSS_INFO = 645,
    WLAN_CFGID_BCN_TABLE_SWITCH = 646,
#endif
    WLAN_CFGID_VOE_ENABLE = 647,

    WLAN_CFGID_SET_TXRX_CHAIN = 648,   /* 设置收发通道 */
    WLAN_CFGID_SET_2G_TXRX_PATH = 649, /* 设置2G通路 */

    WLAN_CFGID_VENDOR_CMD_GET_CHANNEL_LIST = 650, /* mp12 vendor cmd, 获取信道列表 */

    WLAN_CFGID_VOWIFI_INFO = 653,   /* host->device，VoWiFi模式配置; device->host,上报切换VoWiFi/VoLTE */
    WLAN_CFGID_VOWIFI_REPORT = 654, /* device->host,上报切换VoWiFi/VoLTE */

    WLAN_CFGID_SET_ALWAYS_TX_HW_CFG = 657,    /* 废弃 */ /* mp15 rom化保留 */

    WLAN_CFGID_NARROW_BW = 659,

    WLAN_CFGID_SET_ALWAYS_TX_NUM = 660, /* 设置mp12常发数目 */

#ifdef _PRE_WLAN_FEATURE_TAS_ANT_SWITCH
    WLAN_CFGID_TAS_RSSI_ACCESS = 661, /* 天线RSSI测量 */
#endif

    WLAN_CFGID_DC_STATUS = 667, /* DC场景状态识别 */

#ifdef _PRE_WLAN_FEATURE_WMMAC
    WLAN_CFGID_ADDTS_REQ = 671,    /* 发送ADDTS REQ的配置命令 */
    WLAN_CFGID_DELTS = 672,        /* 发送DELTS的配置命令 */
    WLAN_CFGID_REASSOC_REQ = 673,  /* 发送reassoc req 的配置命令 */ /* 废弃 */
    WLAN_CFGID_WMMAC_SWITCH = 674, /* 设置WMMAC SWITCH开关的配置命令 */
#endif
    WLAN_CFGID_SET_BW_FIXED = 681, /* 设置标记用户将带宽模式设置为固定的模式 mp16已删除 */
    WLAN_CFGID_SET_TX_POW = 682,   /* 设置发送功率 */

    WLAN_CFGID_CLR_BLACK_LIST = 693,

#if defined(_PRE_WLAN_FEATURE_11V_ENABLE)
    WLAN_CFGID_11V_BSST_SWITCH = 701, /* 禁用|开启11v bss transition特性功能开关 */
    WLAN_CFGID_11V_TX_QUERY = 702,    /* 11v 触发STA发送Query帧 */
#endif
    WLAN_CFGID_SET_MLME = 707,

    WLAN_CFGID_RSSI_LIMIT_CFG = 719, /* mp13 mp15,  rom化保留，废弃 */
    WLAN_CFGID_GET_BCAST_RATE = 720,
    WLAN_CFGID_GET_ALL_STA_INFO = 722,    /* 获取所有已关联sta信息 */
    WLAN_CFGID_GET_STA_INFO_BY_MAC = 723, /* 获取某个指定关联sta信息 */

    WLAN_CFGID_DPD = 728,       /* DPD配置命令 */
    WLAN_CFGID_GET_2040BSS_SW = 730, /* 获取20/40 bss的使能开关 */

    WLAN_CFGID_FBT_GET_STA_11H_ABILITY = 734, /* hilink获取sta 11h能力信息 */

    WLAN_CFGID_QUERY_PSST = 752, /* 查询用户省功耗状态 mp16已删除 */

    WLAN_CFGID_CALI_POWER = 755,
    WLAN_CFGID_SET_POLYNOMIAL_PARA = 756,
    WLAN_CFGID_GET_STA_INFO = 757,
    WLAN_CFGID_SET_DEVICE_MEM_FREE_STAT = 759, /* 设置device内存统计 */
    WLAN_CFGID_GET_PPM = 760,                  /* 获取ppm值 */
    WLAN_CFGID_GET_CALI_POWER = 761,           /* 获取校准功率值 */
    WLAN_CFGID_GET_DIEID = 762,

    WLAN_CFGID_VAP_WMM_SWITCH = 766, /* 打开或者关闭vap级别wmm开关 */

    WLAN_CFGID_PM_DEBUG_SWITCH = 768, /* 低功耗调试命令 */
    WLAN_CFGID_GET_BG_NOISE = 769, /* 获取通道底噪 */

    WLAN_CFGID_IP_FILTER = 770, /* 配置IP端口过滤的命令 */

    WLAN_CFGID_GET_UPC_PARA = 771, /* 获取产线校准upc值 */
    WLAN_CFGID_SET_UPC_PARA = 772, /* 写产线校准upc值 */
    WLAN_CFGID_SET_LOAD_MODE = 777, /* 设置负载模式 */
    WLAN_CFGID_SET_SUSPEND_MODE = 778, /* 设置亮暗屏状态 */
    WLAN_CFGID_SET_APF_FILTER = 779, /* 设置APF PROGRAM */
    WLAN_CFGID_REMOVE_APP_IE = 780, /* 移除某个上层配置的IE */
    WLAN_CFGID_SET_DFS_MODE = 781, /* dfs功能的寄存器调整接口 */
    WLAN_CFGID_SET_RADAR = 782,    /* 配置雷达事件 */

    WLAN_CFGID_MIMO_BLACKLIST = 783,

    WLAN_CFGID_SET_ADC_DAC_FREQ = 790, /* dfs功能的寄存器调整接口 */

#ifdef _PRE_WLAN_FEATURE_TCP_ACK_BUFFER
    WLAN_CFGID_TCP_ACK_BUF = 791,
#endif

    WLAN_CFGID_FORCE_STOP_FILTER = 792,

    WLAN_CFGID_GET_DBDC_INFO = 793,
#ifdef _PRE_WLAN_DELAY_STATISTIC
    WLAN_CFGID_PKT_TIME_SWITCH = 794,
#endif

    WLAN_CFGID_SET_OWE = 795, /* 设置APUT模式下支持的OWE group */

    WLAN_CFGID_SET_MAC_FREQ = 798, /* MAC频率的寄存器调整接口 */

#ifdef _PRE_WLAN_FEATURE_11AX
    WLAN_CFGID_11AX_DEBUG = 799,
    WLAN_CFGID_TWT_SETUP_REQ = 800,    /* 建立TWT会话的配置命令 */
    WLAN_CFGID_TWT_TEARDOWN_REQ = 801, /*  删尝TWT会话的配置命令 */
#endif

    WLAN_CFGID_SPEC_FEMLP_EN = 802, /* 特殊场景fem低功耗开启命令 */
    WLAN_CFGID_SOFTAP_MIMO_MODE = 803,

#ifdef _PRE_WLAN_RX_LISTEN_POWER_SAVING
    WLAN_CFGID_RX_LISTEN_PS_SWITCH = 804, /* 终端下发rx listen ps命令开关 */
#endif

#ifdef _PRE_WLAN_FEATURE_PMF
    WLAN_CFGID_PMF_CAP = 805, /* 配置PMF能力 */
#endif

    WLAN_CFGID_ASSIGNED_FILTER = 806, /* 配置指定数据帧过滤命令 */

    WLAN_CFGID_SET_ALWAYS_TX_AGGR_NUM = 807, /* 设置常发聚合个数 */

    WLAN_CFGID_MAC_TX_COMMON_REPORT = 809, /* mac tx common report命令 */
    WLAN_CFGID_MAC_RX_COMMON_REPORT = 810, /* mac rx common report命令 */
    WLAN_CFGID_COMMON_DEBUG = 811,         /* mp15 common debug命令 */

    WLAN_CFGID_SET_FASTSLEEP_PARA = 813, /* 配置fastsleep相关参数 */
    WLAN_CFGID_SET_RU_TEST = 814,
    WLAN_CFGID_SET_WARNING_MODE = 815,
    WLAN_CFGID_SET_CHR_MODE = 816,

#ifdef _PRE_WLAN_FEATURE_M2S
    WLAN_CFGID_SET_M2S_MODEM = 817,
#endif
    WLAN_CFGID_SET_LINKLOSS_DISABLE_CSA    = 818,  /* 配置linkloss是否禁止csa切换 */
    WLAN_CFGID_LOG_DEBUG = 819,
    WLAN_CFGID_TRIGGER_REASSOC = 820,    /* 触发重关联 */

#ifdef _PRE_WLAN_RR_PERFORMENCE_DEBUG
    WLAN_CFGID_ENABLE_RR_TIMEINFO = 821,    /* RR性能检测打点命令开关 */
#endif
#ifdef _PRE_WLAN_FEATURE_NAN
    WLAN_CFGID_NAN = 822,
#endif
#ifdef _PRE_WLAN_FEATURE_HID2D
#ifdef _PRE_WLAN_FEATURE_HID2D_PRESENTATION
    WLAN_CFGID_HID2D_SWITCH_CHAN = 823,  /* 触发GO 通过CSA 机制切换信道 */
    WLAN_CFGID_HID2D_SCAN_CHAN = 824,  /* 发布会apk下发扫描命令 */
    WLAN_CFGID_HID2D_PRESENTATION_MODE = 825, /* 设置发布会场景 */
#endif
    WLAN_CFGID_HID2D_ACS_MODE = 826, /* HiD2D自动信道切换总开关 */
    WLAN_CFGID_HID2D_ACS_REPORT = 827, /* HiD2D自动信道切换dmac上报信息 */
    WLAN_CFGID_HID2D_LINK_MEAS = 828, /* HiD2D信道探测命令 */
#endif
#ifdef _PRE_WLAN_FEATURE_FTM
    WLAN_CFGID_RTT_CONFIG = 829,
    WLAN_CFGID_RTT_RESULT_REPORT = 830,
#endif
#ifdef _PRE_WLAN_FEATURE_HID2D
    WLAN_CFGID_HID2D_ACS_STATE = 831, /* HiD2D更新方案状态 */
    WLAN_CFGID_CHAN_MEAS_EVENT = 832, /* 信道测量 */
#endif
#ifdef _PRE_WLAN_FEATURE_MCAST_AMPDU
    WLAN_CFGID_MCAST_AMPDU_RETRY = 833,
    WLAN_CFGID_MCAST_AMPDU_PACK_SWITCH = 834,
#endif
    WLAN_CFGID_START_DEMO_VAP = 835,
    WLAN_CFGID_RX_FILTER_FRAG = 836,
    WLAN_CFGID_QUERY_TSF = 837,
    WLAN_CFGID_SET_DYN_PCIE = 838,
    WLAN_CFGID_TX_DEAUTH_DISASSOC_COMPLETE = 839, /* disassoc/deauth发送完成事件 */

#ifdef _PRE_WLAN_CHBA_MGMT
    WLAN_CFGID_START_CHBA = 845,
    WLAN_CFGID_CHBA_SET_PS_THRES = 846,
    WLAN_CFGID_CHBA_CONN_NOTIFY = 847, /* supplicant下发的CHBA准备连接通知 */
    WLAN_CFGID_CHBA_SET_VAP_BITMAP_LEVEL = 848, /* chba vap bitmap配置更新 */
    WLAN_CFGID_CHBA_SET_USER_BITMAP_LEVEL = 849, /* chba user bitmap配置更新 */
    WLAN_CFGID_CHBA_DBAC_CHANNEL_FOLLOW = 852, /* STA+CHBA DBAC 场景，支持信道跟随 */
    WLAN_CFGID_CHBA_LOG_LEVEL = 853, /* CHBA维测日志打印等级 */
    WLAN_CFGID_CHBA_SET_CHANNEL_SEQ_PARAMS = 854, /* 配置CHBA 信道序列参数 */
    WLAN_CFGID_CHBA_MODULE_INIT = 855,
    WLAN_CFGID_CHBA_CONNECT_PREPARE = 856,
    WLAN_CFGID_CHBA_AUTO_BITMAP_CMD = 857,
    WLAN_CFGID_CHBA_SET_USER_BITMAP_CMD = 858,
    WLAN_CFGID_CHBA_SET_VAP_BITMAP_CMD = 859,
    WLAN_CFGID_CHBA_CHAN_SWITCH_TEST_CMD = 860,
    WLAN_CFGID_CHBA_ADJUST_ISLAND_CHAN = 861, /* CHBA全岛切信道命令 */
    WLAN_CFGID_CHBA_FEATURE_SWITCH = 862,
    WLAN_CFGID_CHBA_SET_BATTERY = 863, /* 配置电量，改变RP值 */
    WLAN_CFGID_CHBA_NOTIFY_MULTI_DOMAIN = 864,  /* chba工作时隙收到非bssid的beacon */
    WLAN_CFGID_CHBA_SYNC_REQUEST_FLAG = 866,
    WLAN_CFGID_CHBA_SYNC_DOMAIN_MERGE_FLAG = 867,
    WLAN_CFGID_CHBA_DEVICE_INFO_UPDATE = 868,
    WLAN_CFGID_CHBA_NOTIFY_NON_ALIVE_DEVICE = 869, /* DMAC不活跃设备上报 */
    WLAN_CFGID_CHBA_OWN_RP_SYNC      = 870,
    WLAN_CFGID_CHBA_SET_CHANNEL = 871, /* 配置CHBA信道接口 */
    WLAN_CFGID_CHBA_AUTO_ADJUST_BITMAP_LEVEL = 872, /* chba bitmap档位自动调整事件 */
    WLAN_CFGID_CHBA_CSA_LOST_DEVICE_NOTIFY = 873, /* 切信道存在设备没切 */
    WLAN_CFGID_CHBA_SET_COEX_CHAN_INFO = 874,
    WLAN_CFGID_CHBA_SYNC_EVENT = 876, /* chba dmac通知host 同步时隙开始或结束 */
    WLAN_CFGID_CHBA_UPDATE_TOPO_BITMAP_INFO = 877, /* chba dmac通知host更新topo中bitmap信息 */
    WLAN_CFGID_CHBA_DBAC_UPDATE_CHAN_SEQ_CONFIG = 878, /* chba dbac fcs配置变化时dmac通知host更新信道序列 */
    WLAN_CFGID_CHBA_DOMAIN_MERGE_EVENT = 879, /* chba域合并事件处理 */
#endif
    WLAN_CFGID_STA_CSA_STATUS_NOTIFY = 889,
    WLAN_CFGID_VSP_DEBUG = 890,
    WLAN_CFGID_SET_SAE_PWE = 891,
    WLAN_CFGID_SET_EXTLNA_BYPASS_RSSI = 892,
    WLAN_CFGID_MINTP_TEST = 893,
#ifdef _PRE_WLAN_FEATURE_PWL
    WLAN_CFGID_SET_PWL_ENABLE = 894,
    WLAN_CFGID_SET_PWL_PRE_KEY = 895,
    WLAN_CFGID_PWL_ROAM_EVENT_NOTIFY = 896,
    WLAN_CFGID_PWL_SYNC_PN = 897,
#endif
    WLAN_CFGID_GET_TEMPERATURE = 898,
    WLAN_CFGID_GET_HW_WORD = 899,
    WLAN_CFGID_GET_BOARD_SN = 900,
    WLAN_CFGID_SET_BOARD_SN = 901,
    WLAN_CFGID_FLASH_TEST = 902,
    WLAN_CFGID_LP_AGENT_LOG_ENABLE = 903,
    /* 注: 当前文件最大枚举定义到1999, 后续定义在wlan_cfg_id_2.h */
    WLAN_CFGID_1_END = WLAN_CFGID_2_START,
} wlan_cfgid1_enum;
#endif /* end of wlan_cfg_id.h */
