/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : linux ioctl配置调试命令
 * 作    者 : wifi
 * 创建日期 : 2016年4月8日
 */

#ifdef WIFI_DEBUG_ENABLE
#include "wal_linux_ioctl_debug.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID      OAM_FILE_ID_WAL_LINUX_IOCTL_DEBUG_C

const wal_hipriv_cmd_entry_stru g_ast_hipriv_cmd_debug[] = {
    /***********************调试命令***********************/
    /* 设置TLV类型通用命令: hipriv "wlan0 set_tlv xxx 0 0"  */
    { "set_tlv", wal_hipriv_set_tlv_cmd },
    /* 设置Value类型通用命令: hipriv "wlan0 set_val xxx 0"  */
    { "set_val", wal_hipriv_set_val_cmd },
    /* 设置String类型通用命令: hipriv "wlan0 set_str 11ax_debug 3 tid 0 val 1 cnt 2"  */
    { "set_str", wal_hipriv_set_str_cmd },

#ifdef _PRE_WLAN_FEATURE_TWT
    { "twt_setup_req", _wal_hipriv_twt_setup_req },
    { "twt_teardown_req", _wal_hipriv_twt_teardown_req },
#endif

    { "destroy", wal_hipriv_del_vap }, /* 删除vap私有命令为: hipriv "vap0 destroy" */
    /* 特性的INFO级别日志开关 hipriv "VAPX feature_name {0/1}" */
    { "feature_log_switch", wal_hipriv_feature_log_switch },
#ifdef _PRE_WLAN_NARROW_BAND
    { "narrow_bw", wal_hipriv_narrow_bw }, /* Start DPD Calibration */
#ifdef _PRE_WLAN_EXPORT
    { "export_para_query", wal_hipriv_export_para_query },
    { "export_set_tpc", wal_hipriv_export_set_tpc_param },
    { "nb_info_report", wal_hipriv_nb_info_report },
    { "autorate_cfg_set", wal_hipriv_autorate_cfg_set },
#endif
#endif
    /* 设置是否上报接收描述符帧开关: hipriv "Hisilicon0 ota_rx_dscr_switch 0 | 1"，该命令针对所有的VAP */
    { "ota_switch",       wal_hipriv_ota_rx_dscr_switch },
    /* 设置建立BA会话的配置命令:hipriv "vap0 addba_req xx xx xx xx xx xx(mac地址)
       tidno ba_policy buffsize timeout" 该命令针对某一个VAP */
    { "addba_req",        wal_hipriv_addba_req },
    /* 设置删除BA会话的配置命令: hipriv "vap0 delba_req xx xx xx xx xx xx(mac地址)
       tidno direction" 该命令针对某一个VAP */
    { "delba_req",        wal_hipriv_delba_req },
#ifdef _PRE_WLAN_FEATURE_WMMAC
    /* 设置WMMAC开关，配置命令: hipriv "vap0 wmmac_switch 1/0(使能)
       0|1(WMM_AC认证使能) xxx(timeout_period) factor" 整个Device */
    { "wmmac_switch", wal_hipriv_wmmac_switch },
#endif
    { "memoryinfo", wal_hipriv_memory_info }, /* 打印内存池信息: hipriv "Hisilicon0 memoryinfo host/device" */
    /* 配置BA删建门限，sh hipriv.sh "vap0 set_coex_perf_param  0/1/2/3" */
    { "set_coex_perf_param", wal_hipriv_btcoex_set_perf_param },
#ifdef _PRE_WLAN_FEATURE_NRCOEX
    { "nrcoex_test", wal_hipriv_nrcoex_cfg_test }, /* 配置NR共存参数，sh hipriv.sh "nrcoex_test 0~20" */
#endif
    /* 设置打印phy维测的相关信息，sh hipriv.sh
       "wlan0 protocol_debug [band_force_switch 0|1|2] [2040_ch_swt_prohi 0|1] [40_intol 0|1]" */
    { "protocol_debug", wal_hipriv_set_protocol_debug_info },
    /* 配置发送功率参数: hipriv "vap0 set_tx_pow <param name> <value>" */
    { "set_tx_pow", wal_hipriv_set_tx_pow_param },
    /* 打印描述符信息: hipriv "vap0 set_ucast_data <param name> <value>" */
    { "set_ucast_data",  wal_hipriv_set_ucast_data_dscr_param },
    /* 打印描述符信息: hipriv "vap0 set_bcast_data <param name> <value>" */
    { "set_bcast_data",  wal_hipriv_set_bcast_data_dscr_param },
    /* 打印描述符信息: hipriv "vap0 set_ucast_mgmt <param name> <value>" */
    { "set_ucast_mgmt",  wal_hipriv_set_ucast_mgmt_dscr_param },
    /* 打印描述符信息: hipriv "vap0 set_mbcast_mgmt <param name> <value>" */
    { "set_mbcast_mgmt", wal_hipriv_set_mbcast_mgmt_dscr_param },
    { "nss",           wal_hipriv_set_nss }, /* 设置HT模式下的空间流个数:   hipriv "vap0 nss   <value>" */
    { "txch",          wal_hipriv_set_rfch }, /* 设置发射通道:              hipriv "vap0 rfch  <value>" */

    { "start_scan",   wal_hipriv_start_scan }, /* 触发sta扫描: hipriv "sta0 start_scan" */
    { "join_bssid",   wal_hipriv_start_join }, /* 删除1个用户 hipriv "vap0 join_bssid xx xx xx xx xx xx(mac地址)" */
    { "kick_user",    wal_hipriv_kick_user }, /* 删除1个用户 hipriv "vap0 kick_user xx xx xx xx xx xx(mac地址)" */
    { "ampdu_tx_on",  wal_hipriv_ampdu_tx_on },     /* 开启或关闭ampdu发送功能 hipriv "vap0 ampdu_tx_on 0\1" */
    { "amsdu_tx_on",  wal_hipriv_amsdu_tx_on },     /* 开启或关闭ampdu发送功能 hipriv "vap0 amsdu_tx_on 0\1" */
    { "send_bar",     wal_hipriv_send_bar }, /* 指定tid发送bar hipriv "vap0 send_bar A6C758662817(mac地址) tid_num" */
    /* 复位硬件phy&mac: hipriv "Hisilicon0 reset_hw 0|1|2|3|4|5|6|8|9|10|11
       (all|phy|mac|debug|mac_tsf|mac_cripto|mac_non_cripto|phy_AGC|phy_HT_optional|phy_VHT_optional|phy_dadar )
       0|1(reset phy reg) 0|1(reset mac reg) */
    { "reset_hw",             wal_hipriv_reset_device },
    /* 复位硬件phy&mac: hipriv "Hisilicon0 reset_hw 0|1|2|3(all|phy|mac|debug) 0|1(reset phy reg) 0|1(reset mac reg) */
    { "reset_operate",       wal_hipriv_reset_operate },
    /* 动态开启或者关闭wmm hipriv "vap0 wmm_switch 0|1"(0不使能，1使能) */
    { "wmm_switch",          wal_hipriv_wmm_switch },
    /* 设置以太网帧上报的开关，
       sh hipriv.sh "vap0 ether_switch user_macaddr oam_ota_frame_direction_type_enum(帧方向) 0|1(开关)" */
    { "ether_switch",        wal_hipriv_set_ether_switch },
    /* 设置80211单播帧上报的开关，sh hipriv.sh "vap0 80211_uc_switch user_macaddr
       0|1(帧方向tx|rx) 0|1(帧类型:管理帧|数据帧) 0|1(帧内容开关) 0|1(CB开关) 0|1(描述符开关)" */
    { "80211_uc_switch",     wal_hipriv_set_80211_ucast_switch },
    /* 设置打印phy维测的相关信息，sh hipriv.sh "Hisilicon0 phy_debug [snr 0|1] [rssi 0|1] [trlr 0|1] [count N]" */
    { "phy_debug",           wal_hipriv_set_phy_debug_switch },
    /* 设置所有用户的单播开关，sh hipriv.sh "Hisilicon0 80211_uc_all
       0|1(帧方向tx|rx) 0|1(帧类型:管理帧|数据帧) 0|1(帧内容开关) 0|1(CB开关) 0|1(描述符开关)" */
    { "80211_uc_all",    wal_hipriv_set_all_80211_ucast },
#ifdef _PRE_WLAN_DFT_STAT
    /* 上报或者清零用户队列统计信息: sh hipriv.sh "vap_name usr_queue_stat XX:XX:XX:XX:XX:XX 0|1" */
    { "usr_queue_stat", wal_hipriv_usr_queue_stat },
    /* 上报或者清零所有维测统计信息: sh hipriv.sh "Hisilicon0 reprt_all_stat type(phy/machw/mgmt/irq/all)  0|1" */
    { "reprt_all_stat", wal_hipriv_report_all_stat },
#endif
    /* 设置AMPDU聚合个数: aggr_num_switch非0时，aggr_num有效 */
    { "ampdu_aggr_num", wal_hipriv_set_ampdu_aggr_num },

    { "send_frame", wal_hipriv_send_frame }, /* 指定tid发送bar hipriv "vap0 send_frame (type) (num) (目的mac)" */
    /* chip test配置add key操作的私有配置命令接口
       sh hipriv.sh "xxx(cipher) xx(en_pairwise) xx(key_len) xxx(key_index)
       xxxx:xx:xx:xx:xx:xx...(key 小于32字节) xx:xx:xx:xx:xx:xx(目的地址)  */
    { "add_key", wal_hipriv_test_add_key },

    { "get_version",     wal_hipriv_get_version },         /* 获取软件版本: hipriv "vap0 get_version" */

#ifdef _PRE_WLAN_FEATURE_FTM
    /* hipriv.sh "wlan0 ftm_debug -----------------------*/
    /* -------------------------- enable_ftm_initiator [0|1] */
    /* -------------------------- send_initial_ftm_request channel[] burst[0|1] ftms_per_burst[n] \
                                  en_lci_civic_request[0|1] asap[0|1] bssid[xx:xx:xx:xx:xx:xx] */
    /* -------------------------- enable [0|1] */
    /* -------------------------- cali [0|1] */
    /* -------------------------- send_ftm bssid[xx:xx:xx:xx:xx:xx] */
    /* -------------------------- set_ftm_time t1[] t2[] t3[] t4[] */
    /* -------------------------- enable_ftm_responder [0|1] */
    /* -------------------------- send_range_req mac[] num_rpt[] delay[] ap_cnt[] bssid[] channel[] \
                                  bssid[] channel[] ... */
    /* -------------------------- enable_ftm_range [0|1] */
    /* -------------------------- get_cali */
    /* -------------------------- set_location type[] mac[] mac[] mac[] */
    /* -------------------------- set_ftm_m2s ftm_chain_selection[] */
    { "ftm_debug", _wal_hipriv_ftm },
#endif
#ifdef _PRE_WLAN_FEATURE_CSI
    /* 使能CSI上报: hipriv "Hisilicon0 set_csi xx xx xx xx xx xx(mac地址) ta_check csi_en" */
    { "set_csi", wal_hipriv_set_csi },
#endif

#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY
    /* 设置添加用户的配置命令: hipriv "vap0 get_user_nssbw xx xx xx xx xx xx(mac地址) "  该命令针对某一个VAP */
    { "get_user_nssbw", wal_hipriv_get_user_nssbw },
#endif

#ifdef _PRE_WLAN_FEATURE_M2S
    /* mimo和siso切换: hipriv "Hisilicon0 set_m2s_switch 0/1/2/3/4/5(参数查询/配置模式/resv/软切换或硬切换测试模式) */
    { "set_m2s_switch", wal_hipriv_set_m2s_switch },
#endif

    { "blacklist_add",  wal_hipriv_blacklist_add },       /* 1 */
    { "blacklist_del",  wal_hipriv_blacklist_del },       /* 2 */
    { "blacklist_mode", wal_hipriv_set_blacklist_mode }, /* 3 */
    /* device级别配置命令 设置基于vap的业务分类是否使能 hipriv "Hisilicon0 vap_classify_en 0/1" */
    { "vap_classify_en", wal_hipriv_vap_classify_en },
    { "vap_classify_tid", wal_hipriv_vap_classify_tid }, /* 设置vap的流等级 hipriv "vap0 classify_tid 0~7" */
    /* 设置最大发送功率，要求功率值按照扩大10倍来输入，例如最大功率要限制为20，输入200 */
    { "txpower", wal_hipriv_set_txpower },
    { "essid",   wal_hipriv_set_essid },        /* 设置AP ssid */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))
    { "bintval", wal_ioctl_set_beacon_interval }, /* 设置AP beacon 周期 */
    { "up",      wal_hipriv_start_vap },
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,44)) */
#if (defined(_PRE_WLAN_FEATURE_WAPI) || defined(_PRE_WLAN_FEATURE_PWL))
#ifdef _PRE_WAPI_DEBUG
    { "wpi_info", wal_hipriv_show_wpi_info }, /* echo "wlan0 pwl_info xx:xx:xx:xx:xx:xx" > /sys/hisys/hipriv */
#endif
#endif
    /* 漫游测试命令 sh hipriv.sh "wlan0 roam_start 0|1|2|3|4 0/1"
     * 0或者参数缺失表示漫游前不扫描, 1|2|3|4表示扫描+漫游
     * 第二个参数0或者参数缺失表示漫游到其它AP, 1表示漫游到自己
    */
    { "roam_start",       wal_hipriv_roam_start },
    /* 设置20/40 bss使能: hipriv "Hisilicon0 2040bss_enable 0|1" 0表示20/40 bss判断关闭，1表示使能 */
    { "2040bss_enable", wal_hipriv_enable_2040bss },

    { "dyn_cali", wal_hipriv_dyn_cali_cfg }, /*  动态校准参数配置 sh hipriv "wlan0 dyn_cali   " */

    { "dpd_cfg", wal_hipriv_dpd_cfg },     /* 打印定制化信息 */

#if defined(_PRE_WLAN_FEATURE_11V_ENABLE)
    /* 11v特性配置:  触发sta发送11v Query帧: hipriv "vap[x] 11v_tx_query [mac-addr]" */
    { "11v_tx_query", wal_hipriv_11v_tx_query },
#endif
     /* DBDC特性开关，sh hipriv.sh "wlan0 dbdc_debug [change_hal_dev 0|1] " */
    { "dbdc_debug", wal_hipriv_set_dbdc_debug_switch },
    { "pm_debug", wal_hipriv_set_pm_debug_switch }, /* 低功耗debug命令 */

    { "set_owe", wal_hipriv_set_owe }, /* 设置APUT模式支持的OWE group: hipriv "wlan0 set_owe <value>" */
    /* bit 0/1/2分别表示支持owe group 19/20/21 */
    /* 打印低功耗帧过滤统计 sh hipriv.sh "wlan0 psm_flt_stat [0|1|2|...]" */
    { "psm_flt_stat", wal_hipriv_psm_flt_stat },

#ifdef _PRE_WLAN_FEATURE_HIEX
    /* 设置某个用户hiex能力，sh hipriv.sh "wlan0 set_user_himit xx:xx:xx:xx:xx:xx(mac地址) 0xXXX" */
    { "set_user_hiex", _wal_hipriv_set_user_hiex_enable },
#endif
#ifdef _PRE_WLAN_FEATURE_GET_STATION_INFO_EXT
    { "get_station_info", wal_hipriv_get_station_info },
#endif
#ifdef _PRE_WLAN_FEATURE_NAN
    { "nan_dbg", wal_hipriv_nan_dbg},
#endif
#ifdef _PRE_WLAN_FEATURE_VSP
    { "vsp_dbg", wal_hipriv_vsp_dbg },
#endif
    { "set_extlna_bypass_rssi", wal_hipriv_set_extlna_bypass_rssi },
    /* 查询用户tid缓存队列情况：sh hipriv.sh "wlan0 get_tid_queue xx:xx:xx:xx:xx:xx(用户mac地址)" */
    { "get_tid_queue", wal_hipriv_get_tid_queue_info },

    /* 打印vap的所有参数信息: hipriv "vap0 info" */
    { "info",                 wal_hipriv_vap_info },
    /* 设置国家码命令 hipriv "Hisilicon0 setcountry param "param取值为大写的国家码字，例如 CN US */
    { "setcountry",           wal_hipriv_setcountry },
    /* 查询国家码命令 hipriv "Hisilicon0 getcountry" */
    { "getcountry",           wal_hipriv_getcountry },
    /* ip filter(功能调试接口)hipriv "wlan0 ip_filter cmd param0 param1 ...."
       举例:启动功能 "wlan0 ip_filter set_rx_filter_enable 1/0"
       清空黑名单 "wlan0 ip_filter clear_rx_filters"
       设置黑名单 "wlan0 ip_filter add_rx_filter_items 条目个数(0/1/2...)
       名单内容(protocol0 port0 protocol1 port1...)",目前该调试接口仅支持20对条目
     */
    { "ip_filter",            wal_hipriv_set_ip_filter },
    /* 打印指定mac地址user的所有参数信息: hipriv "vap0 userinfo XX XX XX XX XX XX(16进制oal_strtohex)" */
    { "userinfo",             wal_hipriv_user_info },
#ifdef PLATFORM_DEBUG_ENABLE
    /* 打印寄存器信息: hipriv "Hisilicon0 reginfo 16|32 regtype(soc/mac/phy) startaddr endaddr" */
    { "reginfo",              wal_hipriv_reg_info },
    /* 写寄存器: hipriv "Hisilicon0 regwrite 32/16(51没有16位写寄存器功能) regtype(soc/mac/phy) addr val"
     * addr val必须都是16进制0x开头
     */
    { "regwrite",             wal_hipriv_reg_write },
#endif
#ifdef _PRE_WLAN_EXPORT
    /* 设置固定mac地址, hipriv "Hisilicon0 set_mac XX XX XX XX XX XX(16进制)" */
    { "mac_write",            wal_hipriv_mac_addr_write },
    /* 读取mac地址, hipriv "Hisilicon0 mac_read" */
    { "mac_read",             wal_hipriv_mac_addr_read },
#endif
    { "create",               wal_hipriv_add_vap }, /* 创建vap私有命令为: hipriv "Hisilicon0 create vap0 ap|sta" */
    /* 设置常发模式:hipriv "vap0 al_tx <value: 0/1/2>  <len>" 由于mac限制，
       11a,b,g下只支持4095以下数据发送,可以使用set_mcast_data对速率进行设置 */
    { "al_tx",                wal_hipriv_always_tx },
    { "al_tx_1102",           wal_hipriv_always_tx },        /* al_tx_mp12命令先保留 */
    { "al_ru_index",          wal_hipriv_set_ru_index }, /* 设置ru index: hipriv "vap0 al_ru_index 106tone 0 1" */
    { "al_rx",                wal_hipriv_always_rx },  /* 设置常收模式: hipriv "vap0 al_rx <value: 0/1/2>" */
#ifdef _PRE_WLAN_FEATURE_SNIFFER
    { "sniffer",              wal_hipriv_set_sniffer }, /* 设置抓包模式: sh hipriv.sh "wlan0 sniffer <value: 0/1>" */
#endif
    { "monitor",              wal_hipriv_enable_monitor_mode }, /* sh hipriv.sh "wlan0 monitor <value: 0/1>" */
    { "rate",  wal_hipriv_set_rate },    /* 设置non-HT模式下的速率: hipriv "vap0 rate  <value>" */
    { "mcs",   wal_hipriv_set_mcs },     /* 设置HT模式下的速率: hipriv "vap0 mcs   <value>" */
    { "mcsac", wal_hipriv_set_mcsac },   /* 设置VHT模式下的速率: hipriv "vap0 mcsac <value>" */
#ifdef _PRE_WLAN_FEATURE_11AX
    { "mcsax", wal_hipriv_set_mcsax },       /* 设置HE模式下的速率:  hipriv "vap0 mcsax <value>" */
#ifdef  _PRE_WLAN_FEATURE_11AX_ER_SU_DCM
    { "mcsax_er", _wal_hipriv_set_mcsax_er }, /* 设置HE ER模式下的速率:      hipriv "vap0 mcsax_er <value>" */
#endif
#endif
    { "freq",              wal_hipriv_set_freq },   /* 设置AP 信道 */
    { "mode",              wal_hipriv_set_mode },   /* 设置AP 协议模式 */
    { "band",              wal_hipriv_set_band },   /* 设置VAP 工作频段 */
    { "bw",                wal_hipriv_set_bw },     /* 设置带宽: hipriv "vap0 bw    <value>" */
    /* 打印描述符信息: hipriv "vap0 set_mcast_data <param name> <value>" */
    { "set_mcast_data",    wal_hipriv_set_mcast_data_dscr_param },
    /* 打印接收帧的FCS正确与错误信息:hipriv "vap0 rx_fcs_info 0/1 1/2/3/4" 0/1  0代表不清除，1代表清除 */
    { "rx_fcs_info",       wal_hipriv_rx_fcs_info },
    /* 设置添加用户的配置命令: hipriv "vap0 add_user xx xx xx xx xx xx(mac地址) 0|1(HT能力位) " 该命令针对某一个VAP */
    { "add_user",          wal_hipriv_add_user },
    /* 设置删除用户的配置命令: hipriv "vap0 del_user xx xx xx xx xx xx(mac地址)" 该命令针对某一个VAP */
    { "del_user",        wal_hipriv_del_user },
    { "alg",     wal_hipriv_alg_cfg_entry }, /* 算法参数配置:sh hipriv.sh "wlan0 alg txmode_mode_sw <value>" */
    { "alg_cfg",         wal_hipriv_alg_cfg }, /* 算法参数配置: hipriv "vap0 alg_cfg sch_vi_limit 10" */
    { "sdio_flowctrl", wal_hipriv_sdio_flowctrl },
#ifdef _PRE_WLAN_DELAY_STATISTIC
    /* 报文时间戳调试开关: hipriv "Hisilicon0 pkt_time_switch on |off */
    { "pkt_time_switch", wal_hipriv_pkt_time_switch },
#endif
#ifdef _PRE_WLAN_FEATURE_11K
    { "send_neighbor_req", wal_hipriv_send_neighbor_req },  /* sh hipriv.sh "wlan0 send_neighbor_req WiFi1" */
#endif
    /* VAP级别日志级别 hipriv "VAPX log_level {1|2}"  Warning与Error级别日志以VAP为维度 */
    { "log_level", wal_hipriv_vap_log_level },

#ifdef _PRE_WLAN_FEATURE_TCP_ACK_BUFFER
    { "tcp_ack_buf", wal_hipriv_tcp_ack_buf_cfg }, /* 快速老化 */
#endif

#ifdef _PRE_WLAN_FEATURE_APF
    { "apf_filter_list", wal_hipriv_apf_filter_list },
#endif
#ifdef _PRE_WLAN_FEATURE_HID2D
    { "set_hid2d_acs_mode", wal_ioctl_set_hid2d_acs_mode },
#ifdef _PRE_WLAN_FEATURE_HID2D_PRESENTATION
    { "switch_chan", wal_hipriv_hid2d_switch_channel },
#endif
#endif
    { "cali",   wal_hipriv_cali_debug },
#ifdef _PRE_HOST_PERFORMANCE
    { "host_time",   wal_hipriv_show_host_time },
#endif
#ifdef _PRE_WLAN_FEATURE_HID2D
    /* sh hipriv.sh "chba0/wlan0 start_chan_meas  meas_type(当前只能取4) scan_chan scan_interval scan_time scan_band" */
    { "start_chan_meas", wal_hipriv_start_chan_meas },
#endif
#ifdef _PRE_WLAN_CHBA_MGMT
    { "set_chba_chanswitch", wal_hipriv_chba_chan_switch },
    { "chba_module_init", wal_hipriv_chba_module_init },
    /* sh hipriv.sh "chba0 auto_bitmap 0|1" 0:关闭bitmap自动调整；1：打开bitmap自动调整 */
    {"auto_bitmap", wal_hipriv_chba_set_auto_bitmap },
    /* sh hipriv.sh "chba0 set_user_bitmap user_mac  bitmap"  */
    {"set_user_bitmap", wal_hipriv_chba_set_user_bitmap },
    /* sh hipriv.sh "chba0 set_vap_bitmap  bitmap"  */
    {"set_vap_bitmap", wal_hipriv_chba_set_vap_bitmap },
    {"chba_log_level", wal_hipriv_chba_log_level },
    /* 设置chba低功耗档位切换门限命令: hipriv "chba0 set_chba_ps_thres [thres_type] [val]" */
    { "set_chba_ps_thres", wal_hipriv_chba_set_ps_thres },
    {"chba_island_chan_switch", wal_hipriv_chba_island_chan_switch },
    { "chba_feature_switch", wal_hipriv_set_chba_feature_switch },
    /* sh hipriv.sh "chba0 chba_set_battery  battery" 取值0~100, 档位为70 50 30 */
    { "chba_set_battery", wal_hipriv_set_chba_battery },
    /* sh hipriv.sh "chba0 set_coex_chan_info cmd_type[0:self 1:island] chan_cnt chan_list" */
    { "set_coex_chan_info", wal_hipriv_set_coex_chan_info },
#endif
    { "memory_monitor", wal_hipriv_memory_monitor },  /* sh hipriv.sh "wlan0 memory_monitor startaddr endaddr" */
    { "set_pt_mcast", wal_hipriv_set_pt_mcast }, // 组播图传报文打标记，1:使能 0:关闭
    { "sar_slide_debug", wal_hipriv_sar_slide_cfg },
    { "mintp_test", wal_hipriv_mintp_test },
};

uint32_t wal_hipriv_get_debug_cmd_size(void)
{
    return oal_array_size(g_ast_hipriv_cmd_debug);
}

#endif
