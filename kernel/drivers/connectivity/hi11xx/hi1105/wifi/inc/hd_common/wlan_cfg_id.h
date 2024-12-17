/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明   : 定义HOST/DEVICE传递命令枚举 的头文件
 * 作者       :
 * 创建日期   : 2020年6月19日
 */

#ifndef WLAN_CFG_ID_H
#define WLAN_CFG_ID_H
#include "wlan_cfg_id_1.h"
#include "wlan_cfg_id_2.h"
#include "oal_ext_if.h"
/*****************************************************************************
    配置命令 ID
    第一段  MIB 类配置
    第二段  非MIB类配置
*****************************************************************************/
typedef enum {
    /************************************************************************
        第一段 MIB 类配置
    *************************************************************************/
    /* *********************dot11smt OBJECT IDENTIFIER ::= { ieee802dot11 1 }**************************/
    /* --  dot11StationConfigTable ::= { dot11smt 1 } */
    WLAN_CFGID_STATION_ID = 0,      /* dot11StationID MacAddress,  */
    WLAN_CFGID_BSS_TYPE = 4,        /* dot11DesiredBSSType INTEGER, mp16已删除 */
    WLAN_CFGID_SSID = 5,            /* dot11DesiredSSID OCTET STRING, SIZE(0..32) */
    WLAN_CFGID_BEACON_INTERVAL = 6, /* dot11BeaconPeriod Unsigned32, mp16已删除 */
    WLAN_CFGID_DTIM_PERIOD = 7,     /* dot11DTIMPeriod Unsigned32, mp16已删除 */
    WLAN_CFGID_UAPSD_EN = 11,       /* dot11APSDOptionImplemented TruthValue, */
    WLAN_CFGID_SMPS_EN = 13,
    /* --  dot11PrivacyTable ::= { dot11smt 5 } */
    /************************dot11mac OBJECT IDENTIFIER ::= { ieee802dot11 2 } **************************/
    /* --  dot11OperationTable ::= { dot11mac 1 } */
    /* --  dot11CountersTable ::= { dot11mac 2 }  */
    /* --  dot11EDCATable ::= { dot11mac 4 } */
    WLAN_CFGID_EDCA_TABLE_CWMIN = 41,         /* dot11EDCATableCWmin Unsigned32 */
    WLAN_CFGID_EDCA_TABLE_CWMAX = 42,         /* dot11EDCATableCWmax Unsigned32 */
    WLAN_CFGID_EDCA_TABLE_AIFSN = 43,         /* dot11EDCATableAIFSN Unsigned32 */
    WLAN_CFGID_EDCA_TABLE_TXOP_LIMIT = 44,    /* dot11EDCATableTXOPLimit Unsigned32 */
    WLAN_CFGID_EDCA_TABLE_MSDU_LIFETIME = 45, /* dot11EDCATableMSDULifetime Unsigned32 */
    WLAN_CFGID_EDCA_TABLE_MANDATORY = 46,     /* dot11EDCATableMandatory TruthValue */

    /* --  dot11QAPEDCATable ::= { dot11mac 5 } */
    WLAN_CFGID_QEDCA_TABLE_CWMIN = 51,         /* dot11QAPEDCATableCWmin Unsigned32 */
    WLAN_CFGID_QEDCA_TABLE_CWMAX = 52,         /* dot11QAPEDCATableCWmax Unsigned32 */
    WLAN_CFGID_QEDCA_TABLE_AIFSN = 53,         /* dot11QAPEDCATableAIFSN Unsigned32 */
    WLAN_CFGID_QEDCA_TABLE_TXOP_LIMIT = 54,    /* dot11QAPEDCATableTXOPLimit Unsigned32 */
    WLAN_CFGID_QEDCA_TABLE_MSDU_LIFETIME = 55, /* dot11QAPEDCATableMSDULifetime Unsigned32 */
    WLAN_CFGID_QEDCA_TABLE_MANDATORY = 56,     /* dot11QAPEDCATableMandatory TruthValue */

    /*************************dot11res OBJECT IDENTIFIER ::= { ieee802dot11 3 } **************************/
    /*************************dot11phy OBJECT IDENTIFIER ::= { ieee802dot11 4 } *************************/
    /* --  dot11PhyHRDSSSTable ::= { dot11phy 12 } */
    WLAN_CFGID_SHORT_PREAMBLE = 60, /* dot11ShortPreambleOptionImplemented TruthValue */

    /* --  dot11PhyERPTable ::= { dot11phy 14 } */
    /* --  dot11PhyHTTable  ::= { dot11phy 15 } */
    WLAN_CFGID_SHORTGI = 80,        /* dot11ShortGIOptionInTwentyActivated TruthValue */
    WLAN_CFGID_SHORTGI_FORTY = 81,  /* dot11ShortGIOptionInFortyActivated TruthValue */
    WLAN_CFGID_CURRENT_CHANEL = 82, /* dot11CurrentChannel Unsigned32 */

    /* --  dot11PhyVHTTable  ::= { dot11phy 23 }  -- */
    WLAN_CFGID_SHORTGI_EIGHTY = 90, /* dot11VHTShortGIOptionIn80Activated TruthValue */

    /************************dot11Conformance OBJECT IDENTIFIER ::= { ieee802dot11 5 } ********************/
    /************************dot11imt         OBJECT IDENTIFIER ::= {ieee802dot11 6} **********************/
    /************************dot11MSGCF       OBJECT IDENTIFIER ::= { ieee802dot11 7} *********************/
    /************************************************************************
        第二段 非MIB 类配置
    *************************************************************************/
    /* 以下为ioctl下发的命令 */
    WLAN_CFGID_ADD_VAP = 100,   /* 创建VAP */
    WLAN_CFGID_START_VAP = 101, /* 启用VAP */
    WLAN_CFGID_DEL_VAP = 102,   /* 删除VAP */
    WLAN_CFGID_DOWN_VAP = 103,  /* 停用VAP */
    WLAN_CFGID_MODE = 105,      /* 模式: 包括协议 频段 带宽 */
    WLAN_CFGID_SNIFFER = 106,      /* 抓包开关 */
    WLAN_CFGID_MONITOR_MODE = 107, /* 开启monitor模式，不进行帧过滤,关闭monitor模式，进行帧过滤 */
    WLAN_CFGID_PROT_MODE = 108, /* 保护模式 mp16已删除 */
    WLAN_CFGID_AUTH_MODE = 109, /* 认证模式 */
    WLAN_CFGID_NO_BEACON = 110, /* 设置no beacon能力位 mp16已删除 */

    WLAN_CFGID_TX_POWER = 113, /* 传输功率 */
    WLAN_CFGID_VAP_INFO = 114, /* 打印vap参数信息 */
    WLAN_CFGID_VAP_STATE_SYN = 115,
    WLAN_CFGID_BANDWIDTH = 116,
    WLAN_CFGID_CHECK_FEM_PA = 117,

    WLAN_CFGID_STOP_SCHED_SCAN = 118,
    WLAN_CFGID_STA_VAP_INFO_SYN = 119,

    /* wpa-wpa2 */
    WLAN_CFGID_ADD_KEY = 120,
    WLAN_CFGID_DEFAULT_KEY = 121,
    WLAN_CFGID_REMOVE_KEY = 122,
    WLAN_CFGID_GET_KEY = 123,

#ifdef _PRE_WLAN_FEATURE_TXOPPS
    WLAN_CFGID_STA_TXOP_AID = 124,
    WLAN_CFGID_TXOP_PS_MACHW = 125, /* 配置mac txopps使能寄存器 */
#endif
    WLAN_CFGID_CHECK_LTE_GPIO = 126,
    WLAN_CFGID_SET_PT_MCAST = 127,
    WLAN_CFGID_SAVE_DPD_DATA = 128,
    WLAN_CFGID_REMOVE_WEP_KEY = 131,
    WLAN_CFGID_ADD_WEP_ENTRY = 132,
    WLAN_CFGID_DEGRADATION_SWITCH = 133,

    WLAN_CFGID_EVENT_SWITCH = 144,       /* event模块开关 */ /* 废弃 */
    WLAN_CFGID_ETH_SWITCH = 145,         /* 以太网帧收发开关 */
    WLAN_CFGID_80211_UCAST_SWITCH = 146, /* 80211单播帧上报开关 */
    WLAN_CFGID_BTCOEX_PREEMPT_TYPE = 153,   /* preempt frame type */
    WLAN_CFGID_BTCOEX_SET_PERF_PARAM = 154, /* 调整性能参数 */
#ifdef _PRE_WLAN_FEATURE_NRCOEX
    WLAN_CFGID_NRCOEX_PRIORITY_SET = 156,
    WLAN_CFGID_NRCOEX_TEST = 157,
    WLAN_CFGID_QUERY_NRCOEX_STAT = 158,
#endif

    WLAN_CFGID_REPORT_VAP_INFO = 159, /* 上报vap信息 */
#ifdef _PRE_WLAN_FEATURE_UWBCOEX
    WLAN_CFGID_UWBCOEX_TEST = 160,
#endif
#ifdef _PRE_WLAN_DFT_STAT
    WLAN_CFGID_PHY_STAT_EN = 161,    /* 设置phy统计使能节点,无用命令mp16已删除,待mp15同步后删除 */
    WLAN_CFGID_DBB_ENV_PARAM = 165,  /* 空口环境类参数上报或者停止上报,无用命令mp16已删除,待mp15同步后删除  */
    WLAN_CFGID_USR_QUEUE_STAT = 166, /* 用户tid队列和节能队列统计信息 */
    WLAN_CFGID_VAP_STAT = 167,       /* vap吞吐统计计信息 */
    WLAN_CFGID_ALL_STAT = 168,       /* 所有统计信息 */
#endif

    WLAN_CFGID_PHY_DEBUG_SWITCH = 169,        /* 打印接收报文的phy debug信息(inc: rssi, snr, trlr etc.)的调试开关 */
    WLAN_CFGID_80211_MCAST_SWITCH = 170,      /* 80211组播\广播帧上报开关 */
    WLAN_CFGID_PROBE_SWITCH = 171,            /* probe requese 和 probe response上报开关 */
    WLAN_CFGID_GET_MPDU_NUM = 172,            /* 获取mpdu数目 */ /* mp13 mp15 rom化保留, 废弃 */
    WLAN_CFGID_OTA_SWITCH = 173,      /* ota模块设置ota接收描述符上报开关 */
    WLAN_CFGID_OTA_RX_DSCR_SWITCH = 174,
    WLAN_CFGID_OTA_BEACON_SWITCH = 175,       /* 设置是否上报beacon帧的开关 */
    WLAN_CFGID_OAM_OUTPUT_TYPE = 176,         /* oam模块输出的位置 */ /* 废弃 */
    WLAN_CFGID_ADD_USER = 178,                /* 添加用户配置命令 */
    WLAN_CFGID_DEL_USER = 179,                /* 删除用户配置命令 */
    WLAN_CFGID_DEL_MULTI_USER = 180,          /* 删除用户配置命令 */
    WLAN_CFGID_AMPDU_END = 182,               /* 关闭AMPDU的配置命令 */
    WLAN_CFGID_ADDBA_REQ = 185,               /* 建立BA会话的配置命令 */
    WLAN_CFGID_DELBA_REQ = 186,               /* 删除BA会话的配置命令 */
    WLAN_CFGID_SET_LOG_LEVEL = 187,           /* 设置LOG配置级别开关 */
    WLAN_CFGID_SET_FEATURE_LOG = 188,         /* 设置日志特性开关 */
    WLAN_CFGID_SET_ALL_OTA = 189,             /* 设置所有用户帧上报的所有开关 */
    WLAN_CFGID_SET_BEACON_OFFLOAD_TEST = 190, /* Beacon offload相关的测试，仅用于测试 */

    WLAN_CFGID_SET_RANDOM_MAC_ADDR_SCAN = 194, /* 设置随机mac addr扫描开关 */
    WLAN_CFGID_SET_RANDOM_MAC_OUI = 195,       /* 设置随机mac oui */
    WLAN_CFGID_SEND_DISASSOC = 196,            /* WAL下发发送disassoc帧事件 */
    WLAN_CFGID_LIST_AP = 200,                  /* 列举扫描到的AP */ /* 废弃 */
    WLAN_CFGID_LIST_STA = 201,                 /* 列举关联的STA */
    WLAN_CFGID_JOIN_BSSID = 202,               /* 触发加入认证并关联 */
    WLAN_CFGID_DUMP_ALL_RX_DSCR = 203,         /* 打印所有的接收描述符 */ /* 废弃 */
    WLAN_CFGID_START_SCAN = 204,               /* 触发初始扫描 */
    WLAN_CFGID_START_JOIN = 205,               /* 触发加入认证并关联 */
    WLAN_CFGID_START_DEAUTH = 206,             /* 触发去认证 */
    WLAN_CFGID_DUMP_TIEMR = 207,               /* 打印所有timer的维测信息 */ /* mp13 mp15 rom化保留, 废弃 */
    WLAN_CFGID_KICK_USER = 208,                /* 去关联1个用户 */

    WLAN_CFGID_PAUSE_TID = 209,                /* 暂停指定用户的指定tid */ /* mp13 mp15 rom化保留, 废弃 */
    WLAN_CFGID_SET_USER_VIP = 210,             /* 设置用户为VIP用户 */ /* mp13 mp15 rom化保留, 废弃 */
    WLAN_CFGID_SET_VAP_HOST = 211,             /* 设置VAP是否为host vap */ /* mp13 mp15 rom化保留, 废弃 */

    WLAN_CFGID_AMPDU_TX_ON = 212,              /* 开启或关闭ampdu发送功能 */
    WLAN_CFGID_AMSDU_TX_ON = 213,              /* 开启或关闭ampdu发送功能 */
    WLAN_CFGID_SEND_BAR = 215,                 /* 指定用户的指定tid发送bar mp16已删除 */
    WLAN_CFGID_LIST_CHAN = 217,                /* 列举支持的管制域信道 */
    WLAN_CFGID_REGDOMAIN_PWR = 218,            /* 设置管制域功率 */ /* 废弃 */ /* mp15 rom化保留 */
    WLAN_CFGID_TXBF_SWITCH = 219,              /* 开启或关闭TXBF发送/接收功能 */
    WLAN_CFGID_TXBF_MIB_UPDATE = 220,          /* 刷新txbf mib能力 */
    WLAN_CFGID_FRAG_THRESHOLD_REG = 221,       /* 设置分片门限长度 */
    WLAN_CFGID_SET_VOWIFI_KEEP_ALIVE = 222,    /* vowifi nat keep alive */

    WLAN_CFGID_RESUME_RX_INTR_FIFO = 226, /* 是否使能恢复RX INTR FIFO开关 */
    WLAN_CFGID_SET_PSM_PARAM = 231, /* STA 低功耗tbtt offset/listen interval配置 */
    WLAN_CFGID_SET_STA_PM_ON = 232, /* STA低功耗开关接口 */

    WLAN_CFGID_DUMP_BA_BITMAP = 241, /* 发指定个数的报文 */ /* mp13 mp15 rom化保留, 废弃 */
    WLAN_CFGID_VAP_ROAM_MAIN_STATE_SYN = 242,
    WLAN_CFGID_AMSDU_AMPDU_SWITCH = 246,
    WLAN_CFGID_COUNTRY = 247,             /* 设置国家码管制域信息 */
    WLAN_CFGID_TID = 248,                 /* 读取最新接收到数据帧的TID */
    WLAN_CFGID_RESET_HW = 249,            /* Reset mac&phy mp16已删除 */

    WLAN_CFGID_UAPSD_DEBUG = 250,         /* UAPSD维测信息 */ /* mp13 mp15 rom化保留, 废弃 */
    WLAN_CFGID_DUMP_RX_DSCR = 251,        /* dump接收描述队列 */ /* mp13 mp15 rom化保留, 废弃 */
    WLAN_CFGID_DUMP_TX_DSCR = 252,        /* dump发送描述符队列 */ /* mp13 mp15 rom化保留, 废弃 */

    WLAN_CFGID_ALG_PARAM = 254,           /* 算法参数配置 */
    WLAN_CFGID_BEACON_CHAIN_SWITCH = 255, /* 设置beacon帧发送策略，0为关闭双路轮流发送，1为开启 */
    WLAN_CFGID_PROTOCOL_DBG = 256,        /* 设置协议中相关的带宽等信息参数 */
    WLAN_CFGID_ALG_ENTRY = 257,        /* 算法参数配置新框架入口 */
    WLAN_CFGID_CALI_ENTRY = 258,       /* 校准参数配置入口 */
    WLAN_CFGID_2040_COEXISTENCE = 260,    /* 设置20/40共存使能，0: 20/40共存使能，1: 20/40共存不使能 */
    WLAN_CFGID_RX_FCS_INFO = 261,         /* 打印接收帧FCS解析信息 */
    WLAN_CFGID_SEND_FRAME = 262,          /* 指定用户发送控制帧或者管理帧 */
    WLAN_CFGID_TX_PDET_VAL = 263,
#ifdef _PRE_WLAN_FEATURE_PMF
    WLAN_CFGID_PMF_ENABLE = 264, /* 配置强制使能pmf */
#endif
#if ((defined(_PRE_WLAN_EXPORT)) && (defined(_PRE_WLAN_NARROW_BAND)))
    WLAN_CFGID_GET_SW_VERSION = 265,
    WLAN_CFGID_SET_TPC_PARA = 266,
    WLAN_CFGID_NB_INFO_REPORT = 267,
    WLAN_CFGID_AUTORATE_PARAM = 268,
    WLAN_CFGID_AUTORATE_CFG_SET = 269,
#endif
    WLAN_CFGID_SCAN_ABORT = 287,       /* 扫描终止 */
    /* 以下命令为cfg80211下发的命令(通过内核) */
    WLAN_CFGID_CFG80211_START_SCHED_SCAN = 288, /* 内核下发启动PNO调度扫描命令 */
    WLAN_CFGID_CFG80211_STOP_SCHED_SCAN = 289,  /* 内核下发停止PNO调度扫描命令 */
    WLAN_CFGID_CFG80211_START_SCAN = 290,       /* 内核下发启动扫描命令 */
    WLAN_CFGID_CFG80211_START_CONNECT = 291,    /* 内核下发启动JOIN(connect)命令 */
    WLAN_CFGID_CFG80211_SET_CHANNEL = 292,      /* 内核下发设置信道命令 */
    WLAN_CFGID_CFG80211_SET_WIPHY_PARAMS = 293, /* 内核下发设置wiphy 结构体命令 */
    WLAN_CFGID_CFG80211_CONFIG_BEACON = 295,    /* 内核下发设置VAP信息 */
    WLAN_CFGID_ALG = 296,                       /* 算法配置命令 */
    WLAN_CFGID_ALG_CFG = 298,                   /* alg_cfg算法命令 */
    WLAN_CFGID_CALI_CFG = 299,                  /* 校准配置命令 */
    WLAN_CFGID_RADARTOOL = 310,                 /* DFS配置命令 */ /* mp13 mp15 rom化保留, 废弃 */

    /* BEGIN:以下命令为开源APP 程序下发的私有命令 */
    WLAN_CFGID_GET_ASSOC_REQ_IE = 311,   /* hostapd 获取ASSOC REQ 帧信息 */
    WLAN_CFGID_SET_WPS_IE = 312,         /* hostapd 设置WPS 信息元素到VAP */
    WLAN_CFGID_SET_RTS_THRESHHOLD = 313, /* hostapd 设置RTS 门限 */
    WLAN_CFGID_SET_WPS_P2P_IE = 314,     /* wpa_supplicant 设置WPS/P2P 信息元素到VAP */
    WLAN_CFGID_CHAN_STAT = 315,

    /* END:以下命令为开源APP 程序下发的私有命令 */
    WLAN_CFGID_ADJUST_PPM = 319,     /* 设置PPM校准算法 */ /* 废弃 */
    WLAN_CFGID_USER_INFO = 320,      /* 用户信息 */
    WLAN_CFGID_SET_DSCR = 321,       /* 配置用户信息 */
    WLAN_CFGID_SET_RATE = 322,       /* 设置non-HT速率 */
    WLAN_CFGID_SET_MCS = 323,        /* 设置HT速率 */
    WLAN_CFGID_SET_MCSAC = 324,      /* 设置VHT速率 */
    WLAN_CFGID_SET_NSS = 325,        /* 设置空间流个数 */
    WLAN_CFGID_SET_RFCH = 326,       /* 设置发射通道 */
    WLAN_CFGID_SET_BW = 327,         /* 设置带宽 */
    WLAN_CFGID_SET_BAND = 328,       /* 设置频段 */
    WLAN_CFGID_SET_ALWAYS_RX = 329,  /* 设置常发模式 */
    WLAN_CFGID_GET_THRUPUT = 330,    /* 获取芯片吞吐量信息 */ /* mp13 mp15 rom化保留, 废弃 */
    WLAN_CFGID_REG_INFO = 332,       /* 寄存器地址信息 */
    WLAN_CFGID_REG_WRITE = 333,      /* 写入寄存器信息 */
#ifdef _PRE_WLAN_EXPORT
    WLAN_CFGID_MAC_ADDR_WRITE = 334, /* 写入mac地址 */
    WLAN_CFGID_MAC_ADDR_READ = 335,  /* 读取mac地址 */
#endif
    WLAN_CFGID_WMM_SWITCH = 338,     /* 打开或者关闭wmm */
    WLAN_CFGID_HIDE_SSID = 339,      /* 打开或者关闭隐藏ssid */ /* mp13 mp15 rom化保留, 废弃 */
    WLAN_CFGID_CHIP_TEST_OPEN = 340, /* 打开芯片验证开关 */
    WLAN_CFGID_SET_ALWAYS_TX = 343, /* 设置mp12常发模式 */
    WLAN_CFGID_SET_MEMORY_MONITOR = 345, /* 配置memory monitor监控 */
    WLAN_CFGID_PROTECTION_UPDATE_STA_USER = 348, /* 增加用户更新保护模式 */
    WLAN_CFGID_40M_INTOL_UPDATE = 349, /* 增加host改变带宽事件 */
    WLAN_CFGID_SET_MCSAX = 350,    /* 设置HE速率 */
    WLAN_CFGID_SET_MCSAX_ER = 351, /* 设置HE ER速率 */
    WLAN_CFGID_SET_RU_INDEX = 352, /* 设置RU index */
#ifdef _PRE_WLAN_FEATURE_OFFLOAD_FLOWCTL
    WLAN_CFGID_SET_FLOWCTL_PARAM = 353, /* 设置流控相关参数 */
    WLAN_CFGID_GET_FLOWCTL_STAT = 354,  /* 获取流控相关状态信息 */
    WLAN_CFGID_GET_HIPKT_STAT = 355,    /* 获取高优先级报文的统计情况 mp16已删除 */
#endif
    WLAN_CFGID_SET_11AX_CLOSE_HTC       = 356,
    WLAN_CFGID_SET_11AX_WIFI6_BLACKLIST = 357,
    WALN_CFGID_SET_P2P_SCENES           = 358, /* 设置P2P业务场景 */
    WLAN_CFGID_SET_CLEAR_11N_BLACKLIST  = 359,
    WALN_CFGID_SET_CHANGE_GO_CHANNEL    = 360,

    WLAN_CFGID_SET_AUTO_PROTECTION = 362, /* 设置auto protection开关 mp16已删除 */

    WLAN_CFGID_SEND_2040_COEXT = 370, /* 发送20/40共存管理帧 */
    WLAN_CFGID_2040_COEXT_INFO = 371, /* VAP的所有20/40共存信息 */ /* 废弃 */

#ifdef _PRE_WLAN_FEATURE_FTM
    WLAN_CFGID_D2H_FTM_REPORT_CALI = 372,
    WLAN_CFGID_D2H_FTM_ISR = 373,
    WLAN_CFGID_FTM_DBG = 374, /* FTM调试命令 */
#endif

    WLAN_CFGID_GET_VERSION = 375, /* 获取版本 */
#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY
    WLAN_CFGID_SET_OPMODE_NOTIFY = 400, /* 设置工作模式通知能力 */ /* mp13 mp15 rom化保留, 废弃 */
    WLAN_CFGID_GET_USER_RSSBW = 401, /* 获取带宽 mp16已删除 */
#endif
#ifdef _PRE_WLAN_FEATURE_M2S
    WLAN_CFGID_SET_M2S_SWITCH = 402,    /* 设置mimo-siso切换 */
    WLAN_CFGID_SET_M2S_MSS = 403,       /* 上层下发MSS切换 */
    WLAN_CFGID_M2S_SWITCH_COMP = 405,   /* m2s切换完成信息上报host */
#endif
#ifdef _PRE_WLAN_FEATURE_TAS_ANT_SWITCH
    WLAN_CFGID_ANT_TAS_SWITCH_RSSI_NOTIFY = 406, /* TAS/默认态切换完成RSSI上报host */
#endif
    WLAN_CFGID_AMSDU_SWITCH_REPORT = 408, /* dmac是否需要打开amsdu信息通知host */

    WLAN_CFGID_SET_VAP_NSS = 410, /* 设置VAP的接收空间流  */ /* mp13 mp15 rom化保留, 废弃 */
#ifdef _PRE_WLAN_FEATURE_LP_MIRACAST
    WLAN_CFGID_SET_LP_MIRACAST = 411, /* mp15上层下发屏幕低功耗 */
    WLAN_CFGID_REPORT_STATE = 412, /* 上报device是否MIMO/SISO切换 */
    WLAN_CFGID_GET_M2S_STATE = 413, /* 上层下发获取m2s状态请求 */
#endif
#ifdef _PRE_WLAN_DFT_REG
    WLAN_CFGID_DUMP_REG = 420,
#endif

#ifdef _PRE_WLAN_FEATURE_PSD_ANALYSIS
    WLAN_CFGID_SET_PSD = 425, /* PSD使能 */
    WLAN_CFGID_CFG_PSD = 426, /* PSD配置 */
#endif
#ifdef _PRE_WLAN_FEATURE_CSI
    WLAN_CFGID_SET_CSI = 427, /* CSI使能 */
    WLAN_CFGID_SET_CSI_BLACK_LIST = 428, /* 获取CSI TA MAC黑名单 */
#endif

    WLAN_CFGID_ADD_BLACK_LIST = 440, /* 添加黑名单 */
    WLAN_CFGID_DEL_BLACK_LIST = 441, /* 删除黑名单 */
    WLAN_CFGID_BLACKLIST_MODE = 442,
    WLAN_CFGID_BLACKLIST_SHOW = 443,

    WLAN_CFGID_ADD_BLACK_LIST_ONLY = 458, /* 添加到黑名单,不做check user和删除user行为 */

    WLAN_CFGID_CFG80211_SET_MIB_BY_BW = 463, /* 根据带宽更新相关mib值:GI & bw_cap */
    WLAN_CFGID_SET_MULTI_TID = 464, /* multi_tid配置 */
    WLAN_CFGID_QUERY_STATION_STATS_EXT = 472, /* WLAN_CFGID_QUERY_STATION_STATS新增信息上报 */
    WLAN_CFGID_QUERY_TID_QUEUE = 473, /* 上报tid缓存队列情况查询命令 */
    WLAN_CFGID_SET_DTIM_PERIOD = 474, /* 设置暗屏的dtim周期 */
    WLAN_CFGID_SET_AGGR_NUM = 475, /* 设置聚合个数 */
    WLAN_CFGID_FREQ_ADJUST = 476,  /* 频偏调整 */

    WLAN_CFGID_SET_STBC_CAP = 477, /* 设置STBC能力 */
    WLAN_CFGID_SET_LDPC_CAP = 478, /* 设置LDPC能力 */

    WLAN_CFGID_VAP_CLASSIFY_EN = 479,  /* VAP流等级开关 */
    WLAN_CFGID_VAP_CLASSIFY_TID = 480, /* VAP流等级 */

    WLAN_CFGID_RESET_HW_OPERATE = 481,    /* Reset 同步 */
    WLAN_CFGID_QUERY_STATION_STATS = 483, /* 信息上报查询命令 */
    WLAN_CFGID_CONNECT_REQ = 484,         /*  */
    WLAN_CFIGD_BGSCAN_ENABLE = 485,       /* 禁用背景扫描命令 */
    WLAN_CFGID_QUERY_RSSI = 486,          /* 查询用户dmac rssi信息 mp16已删除 */
    WLAN_CFGID_QUERY_RATE = 487,          /* 查询用户当前使用的tx rx phy rate mp16已删除 */
    WLAN_CFGID_QUERY_PSM_STAT = 488,      /* 查询低功耗统计数据 */
    WLAN_CFGID_SET_LP_ENABLE = 489,   /* LP命令开关 */

    WLAN_CFGID_CFG80211_REMAIN_ON_CHANNEL = 490,        /* 停止在指定信道 */
    WLAN_CFGID_CFG80211_CANCEL_REMAIN_ON_CHANNEL = 491, /* 取消停止在指定信道 */

    WLAN_CFGID_DEVICE_MEM_LEAK = 492, /* device 侧mem leak打印接口 */ /* 废弃 */
    WLAN_CFGID_DEVICE_MEM_INFO = 493, /* device 侧mem 使用情况打印接口 */

    WLAN_CFGID_SET_PS_MODE = 494, /* 设置pspoll mode */
    WLAN_CFGID_SHOW_PS_INFO = 495, /* PSM状态查看 */

    WLAN_CFGID_SET_UAPSD_PARA = 496, /* 设置UAPSD参数 */
    WLAN_CFGID_SET_TRX_DEBUG = 497,
    WLAN_CFGID_CFG80211_MGMT_TX = 498, /* 发送管理帧 */
    WLAN_CFGID_CFG80211_MGMT_TX_STATUS = 499,

    /* 注: 当前文件最大枚举定义到499, 后续定义在wlan_cfg_id_1.h */
    WLAN_CFGID_END = WLAN_CFGID_1_START,
} wlan_cfgid_enum;
typedef uint16_t wlan_cfgid_enum_uint16;

#endif /* end of wlan_cfg_id.h */
