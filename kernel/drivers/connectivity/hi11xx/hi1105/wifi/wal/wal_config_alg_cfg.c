/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明   : 算法配置命令相关接口函数
 * 创建日期   : 2018年6月26日
 */


/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "securec.h"
#include "wal_config_alg_cfg.h"
#include "hmac_vap.h"
#include "mac_vap.h"
#include "mac_frame.h"
#include "mac_mib_sta_config.h"
#include "wal_cfg_release_ioctl.h"
#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DRIVER_WAL_CONFIG_ALG_CFG_C

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
alg_cfg_process_info_stru g_alg_cfg_process_info_table[] = {
    /* alg_cfg_id
                     cfg_type          need_h_process   need_w4_h   need_d_process   need_w4_d */
    /* txmode */
    alg_cfg_info_tbl("txmode_mode_sw",                    MAC_ALG_CFG_TXMODE_MODE_SW,
                     ALG_CFG_TYPE_SET, OAL_FALSE,       OAL_FALSE,   OAL_TRUE,       OAL_TRUE),
    alg_cfg_info_tbl("get_txmode_mode_sw",                MAC_ALG_CFG_TXMODE_MODE_SW,
                     ALG_CFG_TYPE_GET, OAL_FALSE,       OAL_FALSE,   OAL_TRUE,       OAL_TRUE),
    alg_cfg_info_tbl("txmode_user_fix_mode",              MAC_ALG_CFG_TXMODE_USER_FIX_MODE,
                     ALG_CFG_TYPE_SET, OAL_FALSE,       OAL_FALSE,   OAL_TRUE,       OAL_TRUE),
    alg_cfg_info_tbl("get_txmode_user_fix_mode",          MAC_ALG_CFG_TXMODE_USER_FIX_MODE,
                     ALG_CFG_TYPE_GET, OAL_FALSE,       OAL_FALSE,   OAL_TRUE,       OAL_TRUE),
    alg_cfg_info_tbl("txmode_user_fix_chain",              MAC_ALG_CFG_TXMODE_USER_FIX_CHAIN,
                     ALG_CFG_TYPE_SET, OAL_FALSE,       OAL_FALSE,   OAL_TRUE,       OAL_TRUE),
    alg_cfg_info_tbl("get_txmode_user_fix_chain",          MAC_ALG_CFG_TXMODE_USER_FIX_CHAIN,
                     ALG_CFG_TYPE_GET, OAL_FALSE,       OAL_FALSE,   OAL_TRUE,       OAL_TRUE),
    alg_cfg_info_tbl("txmode_debug_log_switch",           MAC_ALG_CFG_TXMODE_DEBUG_LOG_SWITCH,
                     ALG_CFG_TYPE_SET, OAL_FALSE,       OAL_FALSE,   OAL_TRUE,       OAL_TRUE),
    alg_cfg_info_tbl("get_txmode_debug_log_switch",       MAC_ALG_CFG_TXMODE_DEBUG_LOG_SWITCH,
                     ALG_CFG_TYPE_GET, OAL_FALSE,       OAL_FALSE,   OAL_TRUE,       OAL_TRUE),
    alg_cfg_info_tbl("txmode_all_user_fix_mode",          MAC_ALG_CFG_TXMODE_ALL_USER_FIX_MODE,
                     ALG_CFG_TYPE_SET, OAL_FALSE,       OAL_FALSE,   OAL_TRUE,       OAL_TRUE),
    alg_cfg_info_tbl("get_txmode_all_user_fix_mode",      MAC_ALG_CFG_TXMODE_ALL_USER_FIX_MODE,
                     ALG_CFG_TYPE_GET, OAL_FALSE,       OAL_FALSE,   OAL_TRUE,       OAL_TRUE),
    alg_cfg_info_tbl("txmode_chain_prob_sw",          MAC_ALG_CFG_TXMODE_CHAIN_PROB_SW,
                     ALG_CFG_TYPE_SET, OAL_FALSE,       OAL_FALSE,   OAL_TRUE,       OAL_TRUE),

    /* waterfilling */
    alg_cfg_info_tbl("tx_wf_mode",            MAC_ALG_CFG_WATERFILLING_MODE,
                     ALG_CFG_TYPE_SET, OAL_FALSE,       OAL_FALSE,   OAL_TRUE,       OAL_TRUE),
    alg_cfg_info_tbl("get_tx_wf_mode",        MAC_ALG_CFG_WATERFILLING_MODE,
                     ALG_CFG_TYPE_GET, OAL_FALSE,       OAL_FALSE,   OAL_TRUE,       OAL_TRUE),
    alg_cfg_info_tbl("wf_enable",             MAC_ALG_CFG_WATERFILLING_ENABLE,
                     ALG_CFG_TYPE_SET, OAL_FALSE,       OAL_FALSE,   OAL_TRUE,       OAL_TRUE),
    alg_cfg_info_tbl("get_wf_enable",         MAC_ALG_CFG_WATERFILLING_ENABLE,
                     ALG_CFG_TYPE_GET, OAL_FALSE,       OAL_FALSE,   OAL_TRUE,       OAL_TRUE),
    alg_cfg_info_tbl("wf_hadamard_mode",                MAC_ALG_CFG_HARDAMARDTX_MODE,
                     ALG_CFG_TYPE_SET, OAL_FALSE,       OAL_FALSE,   OAL_TRUE,       OAL_TRUE),
    alg_cfg_info_tbl("get_wf_hadamard_mode",            MAC_ALG_CFG_HARDAMARDTX_MODE,
                     ALG_CFG_TYPE_GET, OAL_FALSE,       OAL_FALSE,   OAL_TRUE,       OAL_TRUE),
    alg_cfg_info_tbl("hadamard_enable",                 MAC_ALG_CFG_HARDAMARDTX_ENABLE,
                     ALG_CFG_TYPE_SET, OAL_FALSE,       OAL_FALSE,   OAL_TRUE,       OAL_TRUE),
    alg_cfg_info_tbl("get_hadamard_enable",             MAC_ALG_CFG_HARDAMARDTX_ENABLE,
                     ALG_CFG_TYPE_GET, OAL_FALSE,       OAL_FALSE,   OAL_TRUE,       OAL_TRUE),
    alg_cfg_info_tbl("wf_asnr",               MAC_ALG_CFG_WATERFILLING_CI_ASNR,
                     ALG_CFG_TYPE_SET, OAL_FALSE,       OAL_FALSE,   OAL_TRUE,       OAL_TRUE),
    alg_cfg_info_tbl("get_wf_asnr",           MAC_ALG_CFG_WATERFILLING_CI_ASNR,
                     ALG_CFG_TYPE_GET, OAL_FALSE,       OAL_FALSE,   OAL_TRUE,       OAL_TRUE),
    alg_cfg_info_tbl("set_wf_bw_mode",            MAC_ALG_CFG_WATERFILLING_BW_MODE,
                     ALG_CFG_TYPE_SET, OAL_FALSE,       OAL_FALSE,   OAL_TRUE,       OAL_TRUE),
    alg_cfg_info_tbl("get_wf_bw_mode",        MAC_ALG_CFG_WATERFILLING_BW_MODE,
                     ALG_CFG_TYPE_GET, OAL_FALSE,       OAL_FALSE,   OAL_TRUE,       OAL_TRUE),
    alg_cfg_info_tbl("set_wf_sub_asnr",           MAC_ALG_CFG_WATERFILLING_SUB_ASNR,
                     ALG_CFG_TYPE_SET, OAL_FALSE,       OAL_FALSE,   OAL_TRUE,       OAL_TRUE),
    alg_cfg_info_tbl("get_wf_sub_asnr",           MAC_ALG_CFG_WATERFILLING_SUB_ASNR,
                     ALG_CFG_TYPE_GET, OAL_FALSE,       OAL_FALSE,   OAL_TRUE,       OAL_TRUE),
    alg_cfg_info_tbl("wf_limit",              MAC_ALG_CFG_WATERFILLING_LIMIT_ASNR,
                     ALG_CFG_TYPE_SET, OAL_FALSE,       OAL_FALSE,   OAL_TRUE,       OAL_TRUE),
    alg_cfg_info_tbl("get_wf_limit",          MAC_ALG_CFG_WATERFILLING_LIMIT_ASNR,
                     ALG_CFG_TYPE_GET, OAL_FALSE,       OAL_FALSE,   OAL_TRUE,       OAL_TRUE),
    alg_cfg_info_tbl("wf_debug",              MAC_ALG_CFG_WATERFILLING_DEBUG,
                     ALG_CFG_TYPE_SET, OAL_FALSE,       OAL_FALSE,   OAL_TRUE,       OAL_TRUE),
    alg_cfg_info_tbl("get_wf_debug",          MAC_ALG_CFG_WATERFILLING_DEBUG,
                     ALG_CFG_TYPE_GET, OAL_FALSE,       OAL_FALSE,   OAL_TRUE,       OAL_TRUE),
    alg_cfg_info_tbl("wf_force_flag",              MAC_ALG_CFG_WATERFILLING_FORCE_FLAG,
                     ALG_CFG_TYPE_SET, OAL_FALSE,       OAL_FALSE,   OAL_TRUE,       OAL_TRUE),
    alg_cfg_info_tbl("get_wf_force_flag",          MAC_ALG_CFG_WATERFILLING_FORCE_FLAG,
                     ALG_CFG_TYPE_GET, OAL_FALSE,       OAL_FALSE,   OAL_TRUE,       OAL_TRUE),
    alg_cfg_info_tbl("hadamard_force_flag",              MAC_ALG_CFG_HARDAMARDTX_FORCE_FLAG,
                     ALG_CFG_TYPE_SET, OAL_FALSE,       OAL_FALSE,   OAL_TRUE,       OAL_TRUE),
    alg_cfg_info_tbl("get_hadamard_force_flag",          MAC_ALG_CFG_HARDAMARDTX_FORCE_FLAG,
                     ALG_CFG_TYPE_GET, OAL_FALSE,       OAL_FALSE,   OAL_TRUE,       OAL_TRUE),

};

alg_cfg_param_tbl_stru g_alg_cfg_pkt_type_info_table[] = {
    alg_cfg_pkt_type_tbl("ucast_data",           ALG_CFG_PKT_TYPE_UCAST_DATA),
    alg_cfg_pkt_type_tbl("mcast_data",           ALG_CFG_PKT_TYPE_MCAST_DATA),
    alg_cfg_pkt_type_tbl("bcast_data",           ALG_CFG_PKT_TYPE_BCAST_DATA),
    alg_cfg_pkt_type_tbl("mumimo_data",          ALG_CFG_PKT_TYPE_MUMIMO_DATA),
    alg_cfg_pkt_type_tbl("ofdma_data",           ALG_CFG_PKT_TYPE_OFDMA_DATA),

    alg_cfg_pkt_type_tbl("ucast_mgmt",           ALG_CFG_PKT_TYPE_UCAST_MGMT),
    alg_cfg_pkt_type_tbl("mcast_mgmt",           ALG_CFG_PKT_TYPE_MCAST_MGMT),
    alg_cfg_pkt_type_tbl("bcast_mgmt",           ALG_CFG_PKT_TYPE_BCAST_MGMT),

    alg_cfg_pkt_type_tbl("prot_ctrl",            ALG_CFG_PKT_TYPE_PROT_CTRL),
    alg_cfg_pkt_type_tbl("ndp",                  ALG_CFG_PKT_TYPE_NDP),
    alg_cfg_pkt_type_tbl("ndpa",                 ALG_CFG_PKT_TYPE_NDPA),
    alg_cfg_pkt_type_tbl("resp_ctrl",            ALG_CFG_PKT_TYPE_RESP_CTRL),
    alg_cfg_pkt_type_tbl("other_ctrl",           ALG_CFG_PKT_TYPE_OTHER_CTRL),
};

alg_cfg_param_tbl_stru g_alg_cfg_bool_tbl[] = {
    {"enable",       OAL_TRUE},
    {"disable",      OAL_FALSE},
};
/* ----------------------TXMODE START-------------------------- */
/* ALG_CFG_CMD: tx_mode */
alg_cfg_param_tbl_stru g_alg_cfg_tx_mode_tbl[ALG_CFG_TX_MODE_BUTT] = {
    {"auto", ALG_CFG_TX_MODE_AUTO},
    {"chain0_fix", ALG_CFG_TX_MODE_CHAIN0_FIX},
    {"chain1_fix", ALG_CFG_TX_MODE_CHAIN1_FIX},
    {"chain2_fix", ALG_CFG_TX_MODE_CHAIN2_FIX},
    {"chain3_fix", ALG_CFG_TX_MODE_CHAIN3_FIX},
    {"csd_fix", ALG_CFG_TX_MODE_CSD2_FIX},
    {"csd3_fix", ALG_CFG_TX_MODE_CSD3_FIX},
    {"csd4_fix", ALG_CFG_TX_MODE_CSD4_FIX},
    {"stbc_fix", ALG_CFG_TX_MODE_STBC2_FIX},
    {"stbc3_fix", ALG_CFG_TX_MODE_STBC3_FIX},
    {"stbc4_fix", ALG_CFG_TX_MODE_STBC4_FIX},
    {"txbf_fix", ALG_CFG_TX_MODE_TXBF_FIX},
    {"stbc_2nss_fix", ALG_CFG_TX_MODE_STBC_2NSS_FIX},
};

alg_cfg_param_tbl_stru g_ast_alg_cfg_bool_tbl[] = {
    {"enable",       OAL_TRUE},
    {"disable",      OAL_FALSE},
};
/* -----------------------TXMODE END--------------------------- */
/* -------    alg_cfg_param_tbl_size    ------------ */
alg_cfg_param_tbl_size_info_stru g_alg_cfg_param_size_tbl[] = {
    alg_cfg_param_size_tbl(ALG_CFG_PARAM_PKT_TYPE,              oal_array_size(g_alg_cfg_pkt_type_info_table)),
    alg_cfg_param_size_tbl(ALG_CFG_PARAM_BOOL,                  oal_array_size(g_alg_cfg_bool_tbl)),
    alg_cfg_param_size_tbl(ALG_CFG_PARAM_TX_MODE,               oal_array_size(g_alg_cfg_tx_mode_tbl)),
};

wlan_alg_cfg_sub_alg_args_analysis_tbl_stru g_wlan_alg_cfg_sub_alg_args_analysis_table[] = {
    /* TXMODE start */
    WLAN_CFG_ALG_SUB_ALG_ANALYSIS_TBL(MAC_ALG_CFG_TXMODE_MODE_SW,         alg_cfg_args_analysis_txmode_mode_sw),
    WLAN_CFG_ALG_SUB_ALG_ANALYSIS_TBL(MAC_ALG_CFG_TXMODE_USER_FIX_MODE,   alg_cfg_args_analysis_txmode_user_fix_mode),
    WLAN_CFG_ALG_SUB_ALG_ANALYSIS_TBL(MAC_ALG_CFG_TXMODE_USER_FIX_CHAIN,
                                      alg_cfg_args_analysis_txmode_user_fix_chain),
    WLAN_CFG_ALG_SUB_ALG_ANALYSIS_TBL(MAC_ALG_CFG_TXMODE_DEBUG_LOG_SWITCH,
                                      alg_cfg_args_analysis_txmode_debug_log_switch),
    WLAN_CFG_ALG_SUB_ALG_ANALYSIS_TBL(MAC_ALG_CFG_TXMODE_ALL_USER_FIX_MODE,
                                      alg_cfg_args_analysis_txmode_all_user_fix_mode),
    WLAN_CFG_ALG_SUB_ALG_ANALYSIS_TBL(MAC_ALG_CFG_TXMODE_CHAIN_PROB_SW,   alg_cfg_args_analysis_txmode_chain_prob_sw),
    /* TXMODE end */
    /* waterfilling start */
    WLAN_CFG_ALG_SUB_ALG_ANALYSIS_TBL(MAC_ALG_CFG_WATERFILLING_MODE, alg_cfg_args_analysis_waterfilling_mode),
    WLAN_CFG_ALG_SUB_ALG_ANALYSIS_TBL(MAC_ALG_CFG_WATERFILLING_ENABLE, alg_cfg_args_analysis_waterfilling_enable),
    WLAN_CFG_ALG_SUB_ALG_ANALYSIS_TBL(MAC_ALG_CFG_HARDAMARDTX_MODE,  alg_cfg_args_analysis_hardamard_mode),
    WLAN_CFG_ALG_SUB_ALG_ANALYSIS_TBL(MAC_ALG_CFG_HARDAMARDTX_ENABLE, alg_cfg_args_analysis_hardamard_enable),
    WLAN_CFG_ALG_SUB_ALG_ANALYSIS_TBL(MAC_ALG_CFG_WATERFILLING_LIMIT_ASNR, alg_cfg_args_analysis_waterfilling_limit),
    WLAN_CFG_ALG_SUB_ALG_ANALYSIS_TBL(MAC_ALG_CFG_WATERFILLING_BW_MODE, alg_cfg_args_analysis_waterfilling_bw_mode),
    WLAN_CFG_ALG_SUB_ALG_ANALYSIS_TBL(MAC_ALG_CFG_WATERFILLING_SUB_ASNR, alg_cfg_args_analysis_waterfilling_sub_asnr),
    WLAN_CFG_ALG_SUB_ALG_ANALYSIS_TBL(MAC_ALG_CFG_WATERFILLING_CI_ASNR, alg_cfg_args_analysis_waterfilling_asnr),
    WLAN_CFG_ALG_SUB_ALG_ANALYSIS_TBL(MAC_ALG_CFG_WATERFILLING_DEBUG, alg_cfg_args_analysis_waterfilling_debug),
    WLAN_CFG_ALG_SUB_ALG_ANALYSIS_TBL(MAC_ALG_CFG_WATERFILLING_FORCE_FLAG, alg_cfg_args_analysis_wf_force_flag),
    WLAN_CFG_ALG_SUB_ALG_ANALYSIS_TBL(MAC_ALG_CFG_HARDAMARDTX_FORCE_FLAG, alg_cfg_args_analysis_hdtx_force_flag),
    /* waterfilling end */
};

/* 解圈复杂度需要，wal_hipriv_process_rate_params建立的2个数组 */
OAL_STATIC int32_t g_al_mcs_min_table[WAL_HIPRIV_MCS_TYPE_NUM] = {
    WAL_HIPRIV_HT_MCS_MIN,
    WAL_HIPRIV_VHT_MCS_MIN,
    WAL_HIPRIV_HE_MCS_MIN,
#ifdef  _PRE_WLAN_FEATURE_11AX_ER_SU_DCM
    WAL_HIPRIV_HE_ER_MCS_MIN
#endif
};

OAL_STATIC int32_t g_al_mcs_max_table[WAL_HIPRIV_MCS_TYPE_NUM] = {
    WAL_HIPRIV_HT_MCS_MAX,
    WAL_HIPRIV_VHT_MCS_MAX,
    WAL_HIPRIV_HE_MCS_MAX,
#ifdef  _PRE_WLAN_FEATURE_11AX_ER_SU_DCM
    WAL_HIPRIV_HE_ER_MCS_MAX
#endif
};

OAL_STATIC const int8_t *g_pauc_tx_dscr_nss_tbl[WLAN_NSS_LIMIT] = {
    "1",
    "2",
    "3",
    "4"
};
const wal_ioctl_alg_cfg_stru g_ast_alg_cfg_map[] = {
    { "sch_method",         MAC_ALG_CFG_SCHEDULE_SCH_METHOD },
    { "sch_log",            MAC_ALG_CFG_SCHEDULE_LOG },

    { "txbf_txmode_enb",  MAC_ALG_CFG_TXBF_TXMODE_ENABLE },
    { "txbf_2g_bfer",     MAC_ALG_CFG_TXBF_2G_BFER_ENABLE },
    { "txbf_2nss_bfer",   MAC_ALG_CFG_TXBF_2NSS_BFER_ENABLE },
    { "txbf_fix_mode",    MAC_ALG_CFG_TXBF_FIX_MODE },
    { "txbf_fix_sound",   MAC_ALG_CFG_TXBF_FIX_SOUNDING },
    { "sounding_period",   MAC_ALG_CFG_SOUNDING_PERIOD },
    { "txbf_rm_worst",    MAC_ALG_CFG_TXBF_REMOVE_WORST },
    { "txbf_stbl_num",    MAC_ALG_CFG_TXBF_STABLE_NUM },
    { "txbf_probe_cnt",   MAC_ALG_CFG_TXBF_PROBE_COUNT },
    { "txbf_log_enable",  MAC_ALG_CFG_TXBF_DEBUG_ENABLE },

    /* 开启或关闭速率自适应算法: sh hipriv.sh "vap0 alg_cfg ar_enable [1|0]" */
    { "ar_enable",            MAC_ALG_CFG_AUTORATE_ENABLE },
    /* 设置rts模式:sh hipriv.sh "vap0 alg_cfg ar_rts_mode
       [0(都不开)|1(都开)|2(rate[0]动态RTS, rate[1..3]都开RTS)|3(rate[0]不开RTS, rate[1..3]都开RTS)]" */
    { "ar_rts_mode",          MAC_ALG_CFG_AUTORATE_RTS_MODE },
    /* 开始速率统计日志:sh hipriv.sh "vap0 alg_cfg ar_stat_log_do [mac地址] [业务类别] [包数目]" */
    /* 打印速率集合:sh hipriv.sh "vap0 alg_cfg ar_disp_rateset xx:xx:xx:xx:xx:xx" */
    { "ar_disp_rateset",      MAC_ALG_CFG_AUTORATE_DISPLAY_RATE_SET },
    /* 开启或关闭速率自适应日志: sh hipriv.sh "vap0 alg_cfg ar_log_enable [1|0]" */
    { "ar_log_enable",        MAC_ALG_CFG_AUTORATE_LOG_ENABLE },
    { "ar_txmode_probe",      MAC_ALG_CFG_AUTORATE_TXMODE_PROBE },
    /* 设置最大的VO速率: sh hipriv.sh "vap0 alg_cfg ar_max_vo_rate [速率值]" */
    { "ar_max_vo_rate",       MAC_ALG_CFG_AUTORATE_VO_RATE_LIMIT },
    /* 设置深衰弱的per门限值: sh hipriv.sh "vap0 alg_cfg ar_fading_per_th [per门限值(千分数)]" */
    { "ar_fading_per_th",     MAC_ALG_CFG_AUTORATE_JUDGE_FADING_PER_TH },
    /* 设置聚合自适应开关: sh hipriv.sh "vap0 alg_cfg ar_aggr_opt [1|0]" */
    { "ar_aggr_opt",          MAC_ALG_CFG_AUTORATE_AGGR_OPT },
    /* 设置最大聚合数目: sh hipriv.sh "vap0 alg_cfg ar_max_aggr_mode [聚合数目]" */
    { "ar_aggr_mode",         MAC_ALG_CFG_AUTORATE_AGGR_MODE },
    /* 设置最大聚合数目: sh hipriv.sh "vap0 alg_cfg ar_max_aggr_time_idx [聚合数目]" */
    { "max_aggr_time_idx",    MAC_ALG_CFG_AUTORATE_AGGR_TIME_IDX },
    /* 设置聚合自适应探测间隔: sh hipriv.sh "vap0 alg_cfg ar_aggr_pb_intvl [探测间隔]" */
    { "ar_aggr_pb_intvl",     MAC_ALG_CFG_AUTORATE_AGGR_PROBE_INTVL_NUM },
    /* 设置聚合自适应统计移位值: sh hipriv.sh "vap0 alg_cfg ar_aggr_st_shift [统计移位值]" */
    { "ar_aggr_st_shift",     MAC_ALG_CFG_AUTORATE_AGGR_STAT_SHIFT },
    /* 设置DBAC模式下的最大聚合时间: sh hipriv.sh "vap0 alg_cfg ar_dbac_aggrtime [最大聚合时间(us)]" */
    { "ar_dbac_aggrtime",     MAC_ALG_CFG_AUTORATE_DBAC_AGGR_TIME },
    /* 设置调试用的VI状态: sh hipriv.sh "vap0 alg_cfg ar_dbg_vi_status [0/1/2]" */
    { "ar_dbg_vi_status",     MAC_ALG_CFG_AUTORATE_DBG_VI_STATUS },
    /* 聚合自适应log开关: sh hipriv.sh "vap0 alg_cfg ar_dbg_aggr_log [0/1]" */
    { "ar_dbg_aggr_log",      MAC_ALG_CFG_AUTORATE_DBG_AGGR_LOG },
    /* 最优速率变化时不进行聚合探测的报文数: sh hipriv.sh "vap0 alg_cfg ar_aggr_pck_num [报文数]" */
    { "ar_aggr_pck_num",      MAC_ALG_CFG_AUTORATE_AGGR_NON_PROBE_PCK_NUM },
    /* 最小聚合时间索引: sh hipriv.sh "vap0 alg_cfg ar_aggr_min_idx [索引值]" */
    { "ar_min_aggr_idx",      MAC_ALG_CFG_AUTORATE_AGGR_MIN_AGGR_TIME_IDX },
    /* 设置聚合250us向上的deltaPER门限: sh hipriv.sh "vap0 alg_cfg ar_250us_dper_th [门限值]" */
    { "ar_250us_dper_th",     MAC_ALG_CFG_AUTORATE_AGGR_250US_DELTA_PER_TH },
    /* 设置最大聚合数目: sh hipriv.sh "vap0 alg_cfg ar_max_aggr_num [聚合数目]" */
    { "ar_max_aggr_num",      MAC_ALG_CFG_AUTORATE_MAX_AGGR_NUM },
    /* 设置最低阶MCS限制聚合为1的PER门限: sh hipriv.sh "vap0 alg_cfg ar_1mpdu_per_th [per门限值(千分数)]" */
    { "ar_1mpdu_per_th",      MAC_ALG_CFG_AUTORATE_LIMIT_1MPDU_PER_TH },
    /* 开启或关闭共存探测机制: sh hipriv.sh "vap0 alg_cfg ar_btcoxe_probe [1|0]" */
    { "ar_btcoxe_probe", MAC_ALG_CFG_AUTORATE_BTCOEX_PROBE_ENABLE },
    /* 开启或关闭共存聚合机制: sh hipriv.sh "vap0 alg_cfg ar_btcoxe_aggr [1|0]" */
    { "ar_btcoxe_aggr",  MAC_ALG_CFG_AUTORATE_BTCOEX_AGGR_ENABLE },
    /* 设置共存统计时间间隔参数: sh hipriv.sh "vap0 alg_cfg ar_coxe_intvl [统计周期ms]" */
    { "ar_coxe_intvl",   MAC_ALG_CFG_AUTORATE_COEX_STAT_INTVL },
    /* 设置共存abort低比例门限参数: sh hipriv.sh "vap0 alg_cfg ar_coxe_low_th [千分数]" */
    { "ar_coxe_low_th",  MAC_ALG_CFG_AUTORATE_COEX_LOW_ABORT_TH },
    /* 设置共存abort高比例门限参数: sh hipriv.sh "vap0 alg_cfg ar_coxe_high_th [千分数]" */
    { "ar_coxe_high_th", MAC_ALG_CFG_AUTORATE_COEX_HIGH_ABORT_TH },
    /* 设置共存聚合数目为1的门限参数: sh hipriv.sh "vap0 alg_cfg ar_coxe_agrr_th [千分数]" */
    { "ar_coxe_agrr_th", MAC_ALG_CFG_AUTORATE_COEX_AGRR_NUM_ONE_TH },
    /* 动态带宽特性使能开关: sh hipriv.sh "vap0 alg_cfg ar_dyn_bw_en [0/1]" */
    { "ar_dyn_bw_en",          MAC_ALG_CFG_AUTORATE_DYNAMIC_BW_ENABLE },
    /* 吞吐量波动优化开关: sh hipriv.sh "vap0 alg_cfg ar_thpt_wave_opt [0/1]" */
    { "ar_thpt_wave_opt",      MAC_ALG_CFG_AUTORATE_THRPT_WAVE_OPT },
    /* 设置判断吞吐量波动的goodput差异比例门限(千分数):
       sh hipriv.sh "vap0 alg_cfg ar_gdpt_diff_th [goodput相差比例门限(千分数)]" */
    { "ar_gdpt_diff_th",       MAC_ALG_CFG_AUTORATE_GOODPUT_DIFF_TH },
    /* 设置判断吞吐量波动的PER变差的门限(千分数): sh hipriv.sh "vap0 alg_cfg ar_per_worse_th [PER变差门限(千分数)]" */
    { "ar_per_worse_th",       MAC_ALG_CFG_AUTORATE_PER_WORSE_TH },
    /* 设置发RTS收到CTS但发DATA都不回BA的发送完成中断次数门限: sh hipriv.sh "vap0 alg_cfg ar_cts_no_ba_num [次数]" */
    { "ar_cts_no_ack_num",     MAC_ALG_CFG_AUTORATE_RX_CTS_NO_BA_NUM },
    /* 设置是否支持voice业务聚合: sh hipriv.sh "vap0 alg_cfg ar_vo_aggr [0/1]" */
    { "ar_vo_aggr",            MAL_ALG_CFG_AUTORATE_VOICE_AGGR },
    /* 设置快速平滑统计的平滑因子偏移量: sh hipriv.sh "vap0 alg_cfg ar_fast_smth_shft [偏移量]"(255表示取消快速平滑) */
    { "ar_fast_smth_shft",     MAC_ALG_CFG_AUTORATE_FAST_SMOOTH_SHIFT },
    /* 设置快速平滑统计的最小聚合数目门限: sh hipriv.sh "vap0 alg_cfg ar_fast_smth_aggr_num [最小聚合数目]" */
    { "ar_fast_smth_aggr_num", MAC_ALG_CFG_AUTORATE_FAST_SMOOTH_AGGR_NUM },
    /* 设置short GI惩罚的PER门限值(千分数): sh hipriv.sh "vap0 alg_cfg ar_sgi_punish_per [PER门限值(千分数)]" */
    { "ar_sgi_punish_per",     MAC_ALG_CFG_AUTORATE_SGI_PUNISH_PER },
    /* 设置short GI惩罚的等待探测数目: sh hipriv.sh "vap0 alg_cfg ar_sgi_punish_num [等待探测数目]" */
    { "ar_sgi_punish_num",     MAC_ALG_CFG_AUTORATE_SGI_PUNISH_NUM },
    /* 设置接收通道AGC优化的弱信号RSSI门限: sh hipriv.sh "vap0 alg_cfg ar_weak_rssi_th [RSSI绝对值]"
       (例如: RSSI为-90dBm, 则参数值为90) */
    { "ar_weak_rssi_th",     MAC_ALG_CFG_AUTORATE_WEAK_RSSI_TH },
    /* 设置每个速率等级的最大传输次数: sh hipriv.sh "vap0 alg_cfg ar_max_tx_cnt [传输次数]" */
    { "ar_max_tx_cnt",       MAC_ALG_CFG_AUTORATE_MAX_TX_COUNT },
    { "ar_collision_det",    MAC_ALG_CFG_AUTORATE_COLLISION_DET_EN },
    { "ar_switch_11b",       MAC_ALG_CFG_AUTORATE_SWITCH_11B },
    { "ar_rom_nss_auth",     MAC_ALG_CFG_AUTORATE_ROM_NSS_AUTH },

    /* 弱干扰免疫中non-direct使能: sh hipriv.sh "vap0 alg_cfg anti_inf_imm_en 0|1" */
    { "anti_inf_imm_en",       MAC_ALG_CFG_ANTI_INTF_IMM_ENABLE },
    /* 弱干扰免疫中dynamic unlock使能: sh hipriv.sh "vap0 alg_cfg anti_inf_unlock_en 0|1" */
    { "anti_inf_unlock_en",    MAC_ALG_CFG_ANTI_INTF_UNLOCK_ENABLE },
    /* 弱干扰免疫中rssi统计周期: sh hipriv.sh "vap0 anti_inf_stat_time [time]" */
    { "anti_inf_stat_time",    MAC_ALG_CFG_ANTI_INTF_RSSI_STAT_CYCLE },
    /* 弱干扰免疫中unlock关闭周期: sh hipriv.sh "vap0 anti_inf_off_time [time]" */
    { "anti_inf_off_time",     MAC_ALG_CFG_ANTI_INTF_UNLOCK_CYCLE },
    /* 弱干扰免疫中unlock关闭持续时间: sh hipriv.sh "vap0 anti_inf_off_dur [time]" */
    { "anti_inf_off_dur",      MAC_ALG_CFG_ANTI_INTF_UNLOCK_DUR_TIME },
    /* 抗干扰nav免疫使能: sh hipriv.sh "vap0 alg_cfg anti_inf_nav_en 0|1" */
    { "anti_inf_nav_en",       MAC_ALG_CFG_ANTI_INTF_NAV_IMM_ENABLE },
    /* 弱干扰免疫goodput下降门限: sh hipriv.sh "vap0 alg_cfg anti_inf_gd_th [num]" */
    { "anti_inf_gd_th",        MAC_ALG_CFG_ANTI_INTF_GOODPUT_FALL_TH },
    /* 弱干扰免疫探测保持最大周期数: sh hipriv.sh "vap0 alg_cfg anti_inf_keep_max [num]" */
    { "anti_inf_keep_max",     MAC_ALG_CFG_ANTI_INTF_KEEP_CYC_MAX_NUM },
    /* 弱干扰免疫探测保持最大周期数: sh hipriv.sh "vap0 alg_cfg anti_inf_keep_min [num]" */
    { "anti_inf_keep_min",     MAC_ALG_CFG_ANTI_INTF_KEEP_CYC_MIN_NUM },
    /* 弱干扰免疫是否使能tx time探测: sh hipriv.sh "vap0 anti_inf_tx_pro_en 0|1" */
    { "anti_inf_per_pro_en",   MAC_ALG_CFG_ANTI_INTF_PER_PROBE_EN },
    /* tx time下降门限: sh hipriv.sh "vap0 alg_cfg anti_inf_txtime_th [val]" */
    { "anti_inf_txtime_th",    MAC_ALG_CFG_ANTI_INTF_TX_TIME_FALL_TH },
    /* per下降门限: sh hipriv.sh "vap0 alg_cfg anti_inf_per_th [val]" */
    { "anti_inf_per_th",       MAC_ALG_CFG_ANTI_INTF_PER_FALL_TH },
    /* goodput抖动门限: sh hipriv.sh "vap0 alg_cfg anti_inf_gd_jitter_th [val]" */
    { "anti_inf_gd_jitter_th", MAC_ALG_CFG_ANTI_INTF_GOODPUT_JITTER_TH },
    /* 弱干扰免疫debug的打印信息: sh hipriv.sh "vap0 alg_cfg anti_inf_debug_mode 0|1|2" */
    { "immu_debug",   MAC_ALG_CFG_ANTI_INTF_DEBUG_MODE },
    /* 设置干扰检测周期(ms):sh hipriv.sh "vap0 alg_cfg intf_det_cycle [val]" */
    { "intf_det_cycle",          MAC_ALG_CFG_INTF_DET_CYCLE },
    /* 设置干扰检测模式(同频开/邻频叠频开/都开):sh hipriv.sh "vap0 alg_cfg intf_det_mode 0|1" */
    { "intf_det_mode",           MAC_ALG_CFG_INTF_DET_MODE },
    /* 设置干扰检测debug模式(每个bit表示一类):sh hipriv.sh "vap0 alg_cfg intf_det_debug 0|1" */
    { "intf_det_debug",          MAC_ALG_CFG_INTF_DET_DEBUG },
    /* 设置干扰检测sta阈值(千分之x):sh hipriv.sh "vap0 alg_cfg intf_det_cothr_sta [val]" */
    { "intf_det_cothr_sta",      MAC_ALG_CFG_INTF_DET_COCH_THR_STA },
    /* 设置ACI干扰检测高门限:sh hipriv.sh "vap0 alg_cfg aci_high_th [val]" */
    { "aci_high_th",      MAC_ALG_CFG_INTF_DET_ACI_HIGH_TH },
    /* 设置ACI干扰检测高门限:sh hipriv.sh "vap0 alg_cfg aci_low_th [val]" */
    { "aci_low_th",      MAC_ALG_CFG_INTF_DET_ACI_LOW_TH },
    { "aci_sync_ratio_th",      MAC_ALG_CFG_INTF_DET_ACI_SYNC_TH },
    /* 设置干扰检测sta无干扰阈值(千分之x):sh hipriv.sh "vap0 alg_cfg intf_det_nointf_thr_sta [val]" */
    { "intf_det_nointf_thr_sta", MAC_ALG_CFG_INTF_DET_COCH_NOINTF_STA },
    /* 设置干扰检测ap udp阈值(千分之x):sh hipriv.sh "vap0 alg_cfg intf_det_cothr_udp [val]" */
    { "intf_det_cothr_udp",      MAC_ALG_CFG_INTF_DET_COCH_THR_UDP },
    /* 设置干扰检测ap tcp阈值(千分之x):sh hipriv.sh "vap0 alg_cfg intf_det_cothr_tcp [val]" */
    { "intf_det_cothr_tcp",      MAC_ALG_CFG_INTF_DET_COCH_THR_TCP },
    /* 设置干扰检测邻频干扰扫描周期:sh hipriv.sh "vap0 alg_cfg intf_det_adjscan_cyc [val]" */
    { "intf_det_adjscan_cyc",    MAC_ALG_CFG_INTF_DET_ADJCH_SCAN_CYC },
    /* 设置干扰检测邻频叠频干扰繁忙度阈值(千分之x):sh hipriv.sh "vap0 alg_cfg intf_det_adjratio_thr [val]" */
    { "intf_det_adjratio_thr",   MAC_ALG_CFG_INTF_DET_ADJRATIO_THR },
    /* 设置干扰检测邻频叠频干扰sync error阈值(千分之x):sh hipriv.sh "vap0 alg_cfg intf_det_sync_th [val]" */
    { "intf_det_sync_th",        MAC_ALG_CFG_INTF_DET_SYNC_THR },
    /* 设置干扰检测零频叠频干扰平均rssi阈值(千分之x):sh hipriv.sh "vap0 alg_cfg intf_det_ave_rssi [val]" */
    { "intf_det_ave_rssi",       MAC_ALG_CFG_INTF_DET_AVE_RSSI },
    /* 设置干扰检测非邻/叠频干扰繁忙度阈值(千分之x):sh hipriv.sh "vap0 alg_cfg intf_det_no_adjratio_th [val]" */
    { "intf_det_no_adjratio_th", MAC_ALG_CFG_INTF_DET_NO_ADJRATIO_TH },
    /* 设置干扰检测非邻/叠频干扰计数阈值:sh hipriv.sh "vap0 alg_cfg intf_det_no_adjcyc_th [val]" */
    { "intf_det_no_adjcyc_th",   MAC_ALG_CFG_INTF_DET_NO_ADJCYC_TH },
    /* 设置干扰检测非邻/叠频干扰计数阈值:sh hipriv.sh "vap0 alg_cfg intf_det_collision_th [val] */
    { "intf_det_collision_th",   MAC_ALG_CFG_INTF_DET_COLLISION_TH },
    /* 设置检测到负增益后不探测阈值:sh hipriv.sh "vap0 alg_cfg neg_det_noprobe_th [val]" */
    { "neg_det_noprobe_th",      MAC_ALG_CFG_NEG_DET_NONPROBE_TH },
    /* 打印统计日志:sh hipriv.sh "vap0 alg_intf_det_log intf_det_stat_log_out" */
    { "intf_det_stat_log_out",   MAC_ALG_CFG_INTF_DET_STAT_LOG_WRITE },
    /* ap模式下edca优化使能模式: sh hipriv.sh "vap0 alg_cfg edca_opt_en_ap 0|1|2" */
    { "edca_opt_en_ap",          MAC_ALG_CFG_EDCA_OPT_AP_EN_MODE },
    /* sta模式下edca优化使能模式: sh hipriv.sh "vap0 alg_cfg edca_opt_en_sta 0|1" */
    { "edca_opt_en_sta",         MAC_ALG_CFG_EDCA_OPT_STA_EN },
    /* sta模式下edca txop limit优化使能模式: sh hipriv.sh "vap0 alg_cfg txop_limit_en_sta 0|1" */
    { "txop_limit_en_sta",       MAC_ALG_CFG_TXOP_LIMIT_STA_EN },
    /* sta模式下edca优化的weighting系数: sh hipriv.sh "vap0 alg_cfg edca_opt_sta_weight 0~3" */
    /* CCA优化功能使能: sh hipriv.sh "vap0 alg_cfg cca_opt_alg_en_mode 0|1" */
    { "cca_opt_alg_en_mode",      MAC_ALG_CFG_CCA_OPT_ALG_EN_MODE },
    /* CCA优化DEBUG模式启动: sh hipriv.sh "vap0 alg_cfg cca_opt_debug_mode 0|1" */
    { "cca_opt_debug_mode",       MAC_ALG_CFG_CCA_OPT_DEBUG_MODE },
    /* CCA优化信道扫描的时间(ms):sh hipriv.sh "vap0 alg_cfg cca_opt_set_sync_err_th [time]" */
    { "cca_opt_set_cca_th_debug", MAC_ALG_CFG_CCA_OPT_SET_CCA_TH_DEBUG },
    /* CCA log开关 sh hipriv.sh "vap0 alg_cfg cca_opt_log 0|1" */
    { "cca_opt_log",            MAC_ALG_CFG_CCA_OPT_LOG },
    /* tpc相关命令sh hipriv.sh "wlan0 alg_cfg tpc_level 0|1|2|3|4[TPC档位] */
    { "tpc_mode",               MAC_ALG_CFG_TPC_MODE },              /* 设置TPC工作模式 */
    { "tpc_dbg",                MAC_ALG_CFG_TPC_DEBUG },             /* 设置TPC的debug开关 */
    { "tpc_level",              MAC_ALG_CFG_TPC_LEVEL },             /* 设置TPC的level档位 */
    /* 设置TPC的log开关:sh hipriv.sh "vap0 alg_cfg tpc_log 1 */
    { "tpc_log",                MAC_ALG_CFG_TPC_LOG },
    { "tpc_over_temp_th",       MAC_ALG_CFG_TPC_OVER_TMP_TH },       /* TPC过温门限 */
    { "tpc_dpd_enable_rate",    MAC_ALG_CFG_TPC_DPD_ENABLE_RATE },   /* 配置DPD生效的速率INDEX */
    { "tpc_target_rate_11b",    MAC_ALG_CFG_TPC_TARGET_RATE_11B },   /* 11b目标速率设置 */
    { "tpc_target_rate_11ag",   MAC_ALG_CFG_TPC_TARGET_RATE_11AG },  /* 11ag目标速率设置 */
    { "tpc_target_rate_11n20",  MAC_ALG_CFG_TPC_TARGET_RATE_HT40 },  /* 11n20目标速率设置 */
    { "tpc_target_rate_11n40",  MAC_ALG_CFG_TPC_TARGET_RATE_HT40 },  /* 11n40目标速率设置 */
    { "tpc_target_rate_11ac20", MAC_ALG_CFG_TPC_TARGET_RATE_VHT20 }, /* 11ac20目标速率设置 */
    { "tpc_target_rate_11ac40", MAC_ALG_CFG_TPC_TARGET_RATE_VHT40 }, /* 11ac40目标速率设置 */
    { "tpc_target_rate_11ac80", MAC_ALG_CFG_TPC_TARGET_RATE_VHT80 }, /* 11ac80目标速率设置 */

#ifdef _PRE_WLAN_FEATURE_MU_TRAFFIC_CTL
    { "traffic_ctl_enable",       MAC_ALG_CFG_TRAFFIC_CTL_ENABLE },
    { "traffic_ctl_timeout",      MAC_ALG_CFG_TRAFFIC_CTL_TIMEOUT },
    { "traffic_ctl_min_thres",    MAC_ALG_CFG_TRAFFIC_CTL_MIN_THRESHOLD },
    { "traffic_ctl_log_debug",    MAC_ALG_CFG_TRAFFIC_CTL_LOG_DEBUG },
    { "traffic_ctl_buf_thres",    MAC_ALG_CFG_TRAFFIC_CTL_BUF_THRESHOLD },
    { "traffic_ctl_buf_adj_enb",  MAC_ALG_CFG_TRAFFIC_CTL_BUF_ADJ_ENABLE },
    { "traffic_ctl_buf_adj_num",  MAC_ALG_CFG_TRAFFIC_CTL_BUF_ADJ_NUM },
    { "traffic_ctl_buf_adj_cyc",  MAC_ALG_CFG_TRAFFIC_CTL_BUF_ADJ_CYCLE },
    { "traffic_ctl_rx_rst_enb",   MAC_ALG_CFG_TRAFFIC_CTL_RX_RESTORE_ENABLE },
    { "traffic_ctl_rx_rst_num",   MAC_ALG_CFG_TRAFFIC_RX_RESTORE_NUM },
    { "traffic_ctl_rx_rst_thres", MAC_ALG_CFG_TRAFFIC_RX_RESTORE_THRESHOLD },

    { "rx_dscr_ctl_enable", MAC_ALG_CFG_RX_DSCR_CTL_ENABLE },
    { "rx_dscr_ctl_log_debug", MAC_ALG_CFG_RX_DSCR_CTL_LOG_DEBUG },
#endif

    {"sifs_log",    MAC_ALG_CFG_SIFS_RSP_LOG},  /* Debug 使能开关 */
    {"sifs_sounding_rate", MAC_ALG_CFG_SIFS_RSP_SOUNDING},
    {"sifs_resp_rate",     MAC_ALG_CFG_SIFS_RSP_SIFS},

    {"sr_nonsrg_en",     MAC_ALG_CFG_NON_SRG_ENABLE},
    {"sr_srg_en",        MAC_ALG_CFG_SRG_ENABLE},
    {"sr_log",           MAC_ALG_CFG_SR_LOG_ENABLE},
    {"sr_duty_th",       MAC_ALG_CFG_SR_DUTY_TH},

    {"ddc_ctrl_en", MAC_ALG_CFG_DDC_CTRL_ENABLE},
    {"ddc_log",     MAC_ALG_CFG_DDC_LOG_ENABLE},

    {"tpclowpower_en", MAC_ALG_CFG_TPC_LOWPOWER_ENABLE},
    {"tpclowpower_log", MAC_ALG_CFG_TPC_LOWPOWER_LOG},

    {"aci_naci_det_en", MAC_ALG_CFG_ACI_NACI_DET_EN},
    {"aci_naci_mode",   MAC_ALG_CFG_ACI_NACI_FIX_MODE},
    {"aci_naci_log",    MAC_ALG_ACI_NACI_DEBUG_LOG},

    { NULL }
};
/*****************************************************************************
  3 函数实现
*****************************************************************************/
uint16_t alg_cfg_process_tbl_size(void)
{
    return sizeof(g_alg_cfg_process_info_table) / sizeof(g_alg_cfg_process_info_table[0]);
}
/*
 * 函 数 名   : alg_cfg_param_get_tbl_size
 * 功能描述   : 获取alg_cfg_param各表size
 * 1.日    期   : 2018年7月5日
 * 作    者   :     wifi
 * 修改内容   : 新生成函数
 */
uint8_t alg_cfg_param_get_tbl_size(alg_cfg_param_tbl_size_enum_uint8 param_type)
{
    alg_cfg_param_tbl_size_info_stru *size_info = g_alg_cfg_param_size_tbl;
    uint8_t index;
    uint8_t tbl_size = oal_array_size(g_alg_cfg_param_size_tbl);

    if (param_type >= ALG_CFG_PARAM_BUTT) {
        return 0;
    }

    for (index = 0; index < tbl_size; index++) {
        if (size_info[index].en_param_type == param_type) {
            return size_info[index].uc_tbl_size;
        }
    }

    return 0;
}

/*
 * 函 数 名   : alg_cfg_get_cmd_one_arg
 * 功能描述   : 获取入参字符串第一个参数，并返回第一个参数结束字符的下一字符的偏移
 * 1.日    期   : 2018年5月5日
 * 作    者   :     wifi
 * 修改内容   : 新生成函数
 */
uint32_t alg_cfg_get_cmd_one_arg(const char *cmd, char *arg, uint32_t arg_size,
    uint32_t *cmd_offset)
{
    const char *cmd_copy = NULL;
    uint32_t pos = 0;

    if (oal_likely((cmd == NULL) || (arg == NULL) || (cmd_offset == NULL))) {
        oam_error_log0(0, OAM_SF_ANY,
            "{alg_cfg_get_cmd_one_arg::pc_cmd/pc_arg/pul_cmd_offset null ptr error!}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    cmd_copy = cmd;

    /* 去掉字符串开始的空格 */
    while (*cmd_copy == ' ') {
        ++cmd_copy;
    }

    while ((*cmd_copy != ' ') && (*cmd_copy != '\0')) {
        arg[pos] = *cmd_copy;
        ++pos;
        ++cmd_copy;

        if (oal_unlikely(pos >= WLAN_CFG_MAX_LEN_EACH_ARG) || oal_unlikely(pos >= arg_size)) {
            oam_warning_log1(0, OAM_SF_ANY,
                             "{alg_cfg_get_cmd_one_arg::ul_pos >= WLAN_CFG_MAX_LEN_EACH_ARG, pos %d!}\r\n", pos);
            return OAL_ERR_CODE_ARRAY_OVERFLOW;
        }
    }

    arg[pos] = '\0';

    /* 字符串到结尾，返回结束码 */
    if (pos == 0) {
        *cmd_offset = 0;
        return OAL_ERR_CODE_CONFIG_ARGS_OVER;
    }

    *cmd_offset = (uint32_t)(cmd_copy - cmd);

    return OAL_SUCC;
}

/*
 * 函 数 名   : alg_cfg_search_process_info_by_cfg_name_normal
 * 功能描述   : 根据alg_cfg_id对应的字符串，查找出其在g_ast_alg_cfg_process_info_table对应的表项信息
 * 1.日    期   : 2020年1月11日
  * 修改内容   : 新生成函数
 */
void alg_cfg_search_process_info_by_cfg_name_normal(const char *args, uint32_t arg_size,
    alg_cfg_process_info_stru **target_cfg_info)
{
    alg_cfg_process_info_stru *cfg_info = alg_cfg_get_process_tbl_array();
    uint16_t index;

    if (oal_unlikely(target_cfg_info == NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "alg_cfg_search_process_info_by_cfg_name_normal::target_cfg_info null");
        return;
    }
    for (index = 0; index < alg_cfg_process_tbl_size(); index++) {
        if (!strncmp(args, (char *)cfg_info[index].pc_alg_cfg_name, arg_size)) {
            *target_cfg_info = &cfg_info[index];
            return;
        }
    }

    *target_cfg_info = NULL;
    return;
}

/*
 * 函 数 名   : alg_cfg_search_process_info_by_cfg_name
 * 功能描述   : 根据alg_cfg_id对应的字符串，查找出其在g_ast_alg_cfg_process_info_table对应的表项信息
 * 1.日    期   : 2018年5月5日
 * 作    者   :     wifi
 * 修改内容   : 新生成函数
 */
alg_cfg_process_info_stru *alg_cfg_search_process_info_by_cfg_name(const char *alg_cfg_name)
{
    char args[WLAN_CFG_MAX_LEN_EACH_ARG] = { 0 };
    uint32_t tmp_offset;
    alg_cfg_process_info_stru *target_cfg_info = NULL;

    if (alg_cfg_name == NULL) {
        return NULL;
    }

    if (alg_cfg_get_cmd_one_arg(alg_cfg_name, args, sizeof(args), &tmp_offset) != OAL_SUCC) {
        return NULL;
    }

    alg_cfg_search_process_info_by_cfg_name_normal(args, sizeof(args), &target_cfg_info);

    return target_cfg_info;
}

/*
 * 函 数 名   : alg_cfg_search_process_info_by_cfg_id_normal
 * 功能描述   : 根据alg_cfg_id及cfg_type，查找出其在g_ast_alg_cfg_process_info_table对应的表项信息
 * 1.日    期   : 2018年5月5日
 * 作    者   :     wifi
 * 修改内容   : 新生成函数
 */
void alg_cfg_search_process_info_by_cfg_id_normal(mac_alg_cfg_enum_uint16 alg_cfg_id,
    alg_cfg_type_enum_uint8 cfg_type, alg_cfg_process_info_stru **target_cfg_info)
{
    alg_cfg_process_info_stru *cfg_info = alg_cfg_get_process_tbl_array();
    uint16_t index;

    if (oal_unlikely(target_cfg_info == NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "alg_cfg_search_process_info_by_cfg_id_normal::target_cfg_info null");
        return;
    }
    for (index = 0; index < alg_cfg_process_tbl_size(); index++) {
        if ((alg_cfg_id == cfg_info[index].en_alg_cfg_id) && (cfg_type == cfg_info[index].en_cfg_type)) {
            *target_cfg_info = &cfg_info[index];
            return;
        }
    }

    *target_cfg_info = NULL;
    return;
}

/*
 * 函 数 名   : alg_cfg_search_process_info_by_cfg_id
 * 功能描述   : 根据alg_cfg_id及cfg_type，查找出其在g_ast_alg_cfg_process_info_table对应的表项信息
 * 1.日    期   : 2018年5月5日
 * 作    者   :     wifi
 * 修改内容   : 新生成函数
 */
alg_cfg_process_info_stru *alg_cfg_search_process_info_by_cfg_id(mac_alg_cfg_enum_uint16 alg_cfg_id,
    alg_cfg_type_enum_uint8 cfg_type)
{
    alg_cfg_process_info_stru *target_cfg_info = NULL;

    alg_cfg_search_process_info_by_cfg_id_normal(alg_cfg_id, cfg_type, &target_cfg_info);

    return target_cfg_info;
}

/*
 * 函 数 名   : alg_cfg_get_pkt_type
 * 功能描述   : 根据pkt_type的字符串，查找出其在g_ast_alg_cfg_pkt_type_info_table对应的表项信息
 * 1.日    期   : 2018年5月5日
 * 作    者   :     wifi
 * 修改内容   : 新生成函数
 */
alg_cfg_pkt_type_enum_uint8 alg_cfg_get_pkt_type(const char *cmd_args)
{
    alg_cfg_param_tbl_stru *pkt_type_info = alg_cfg_get_pkt_type_tbl_array();
    uint8_t index;
    char args[WLAN_CFG_MAX_LEN_EACH_ARG] = { 0 };
    uint32_t tmp_offset;

    if (cmd_args == NULL) {
        return ALG_CFG_PKT_TYPE_IRRELEVANT;
    }

    if (alg_cfg_get_cmd_one_arg(cmd_args, args, sizeof(args), &tmp_offset) != OAL_SUCC) {
        return ALG_CFG_PKT_TYPE_IRRELEVANT;
    }

    for (index = 0; index < alg_cfg_param_get_tbl_size(ALG_CFG_PARAM_PKT_TYPE); index++) {
        if (!strncmp(args, (char *)pkt_type_info[index].pc_name, sizeof(args))) {
            return (alg_cfg_pkt_type_enum_uint8)pkt_type_info[index].value;
        }
    }

    return ALG_CFG_PKT_TYPE_IRRELEVANT;
}


/*
 * 函 数 名   : alg_cfg_analysis_args_head
 * 功能描述   : analysis alg cfg head
 * 1.日    期   : 2018年6月21日
 * 作    者   :     wifi
 * 修改内容   : 新生成函数
 */
uint32_t alg_cfg_analysis_args_head(char *real_args[WLAN_CFG_MAX_ARGS_NUM],
    alg_cfg_process_info_stru **real_alg_cfg_info, uint8_t *real_args_offset,
    alg_cfg_pkt_type_enum_uint8 *real_pkt_type)
{
    /* alg_cfg_analysis_args_head */
    alg_cfg_pkt_type_enum_uint8 pkt_type;
    uint8_t offset = 0;
    alg_cfg_process_info_stru *alg_cfg_info = NULL;

    if (oal_unlikely((real_alg_cfg_info == NULL) || (real_args_offset == NULL) ||
        (real_pkt_type == NULL) || (real_args == NULL))) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    pkt_type = alg_cfg_get_pkt_type(real_args[0]);
    if (pkt_type != ALG_CFG_PKT_TYPE_IRRELEVANT) {
        /* first arg is [pkt_type] */
        offset++;
    }

    alg_cfg_info = alg_cfg_search_process_info_by_cfg_name(real_args[offset]);
    if (alg_cfg_info == NULL) {
        return OAL_ERR_CODE_ALG_PROCESS_INFO_NOT_FOUND;
    }

    *real_alg_cfg_info = alg_cfg_info;
    *real_args_offset = ++offset;
    *real_pkt_type = pkt_type;

    return OAL_SUCC;
}


/*
 * 函 数 名   : alg_cfg_analysis_args
 * 功能描述   : prepare alg_cfg_param
 * 1.日    期   : 2018年6月21日
 * 作    者   :     wifi
 * 修改内容   : 新生成函数
 */
uint32_t alg_cfg_analysis_args(char *args[WLAN_CFG_MAX_ARGS_NUM], uint8_t args_num,
    alg_cfg_param_stru *alg_cfg_param, alg_cfg_hdr_stru *alg_cfg_hdr, wlan_cfg_process_info_stru *process_info)
{
    uint32_t ret;
    alg_cfg_process_info_stru *alg_cfg_info = NULL;
    uint8_t real_args_offset = 0;
    alg_cfg_pkt_type_enum_uint8 pkt_type = ALG_CFG_PKT_TYPE_IRRELEVANT;
    uint8_t index;

    if (oal_unlikely(alg_cfg_hdr == NULL)) {
        return OAL_FAIL;
    }
    ret = alg_cfg_analysis_args_head(args, &alg_cfg_info, &real_args_offset, &pkt_type);
    if (ret != OAL_SUCC) {
        return ret;
    }

    /* analysis alg_cfg_hdr */
    alg_cfg_hdr->en_cfg_id = alg_cfg_info->en_alg_cfg_id;
    alg_cfg_hdr->en_cfg_type = alg_cfg_info->en_cfg_type;
    alg_cfg_hdr->en_pkt_type = pkt_type;
    alg_cfg_hdr->dev_ret = OAL_SUCC;
    alg_cfg_hdr->en_need_w4_dev_return = alg_cfg_info->st_process_info.en_need_w4_device_return;

    /* analysis args num */
    alg_cfg_param->uc_param_num = args_num - real_args_offset;

    /* prepare each arg */
    for (index = 0; index < alg_cfg_param->uc_param_num; index++) {
        alg_cfg_param->apuc_param_args[index] = (uint8_t *)args[index + real_args_offset];
    }

    /* update process info */
    *process_info = alg_cfg_info->st_process_info;

    return OAL_SUCC;
}


/*
 * 函 数 名   : alg_cfg_get_match_value
 * 功能描述   : 算法通用接口 -- 查表，找到string对应的enum
 * 1.日    期   : 2018年5月19日
 * 作    者   :     wifi
 * 修改内容   : 新生成函数
 */
uint32_t alg_cfg_get_match_value(const alg_cfg_param_tbl_stru *tbl, uint16_t tbl_size,
    const uint8_t *str, uint32_t *out_value)
{
    uint16_t index;

    if (tbl == NULL || str == NULL || out_value == NULL || tbl_size == 0) {
        return OAL_FAIL;
    }

    for (index = 0; index < tbl_size; index++) {
        if (!strncmp((char *)str, (char *)tbl[index].pc_name, OAL_STRLEN((const char *)tbl[index].pc_name) + 1)) {
            *out_value = tbl[index].value;
            return OAL_SUCC;
        }
    }

    return OAL_FAIL;
}

/*
 * 函 数 名   : wal_wlan_get_sub_alg_args_analysis_entry
 * 功能描述   : 根据配置命令cfg_id获取解析处理入口
 * 1.日    期   : 2020年1月11日
 * 修改内容   : 新生成函数
 */
wlan_alg_cfg_sub_alg_args_analysis_tbl_stru *wal_wlan_get_sub_alg_args_analysis_entry(
    mac_alg_cfg_enum_uint16 cfg_id)
{
    wlan_alg_cfg_sub_alg_args_analysis_tbl_stru *sub_alg_args_analysis_entry = NULL;

    sub_alg_args_analysis_entry = wal_get_wlan_alg_cfg_analysis_entry(g_wlan_alg_cfg_sub_alg_args_analysis_table,
        oal_array_size(g_wlan_alg_cfg_sub_alg_args_analysis_table), cfg_id);

    return sub_alg_args_analysis_entry;
}

/*
 * 函 数 名   : wal_wlan_cfg_alg_process_entry
 * 功能描述   : WLAN_CFGID_ALG_ENTRY在配置模块注册的入口函数
 * 1.日    期   : 2018年6月20日
 * 作    者   :     wifi
 * 修改内容   : 新生成函数
 */
int32_t wal_wlan_cfg_alg_process_entry(wlan_cfg_param_stru *wlan_cfg_param,
    uint8_t *msg_type, uint8_t *mem_block, uint16_t *mem_len,
    wlan_cfg_process_info_stru *process_info)
{
    uint32_t ret;
    alg_cfg_param_stru alg_cfg_param = { 0 };
    alg_cfg_hdr_stru alg_cfg_hdr = { 0 };

    wlan_alg_cfg_sub_alg_args_analysis_tbl_stru *sub_alg_args_analysis = NULL;

    if (wlan_cfg_param == NULL || msg_type == NULL || mem_block == NULL ||
        mem_len == NULL || process_info == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    ret = alg_cfg_analysis_args(wlan_cfg_param->apc_args, wlan_cfg_param->uc_args_num, &alg_cfg_param,
        &alg_cfg_hdr, process_info);
    if (ret != OAL_SUCC) {
        return ret;
    }

    if (oal_unlikely(wlan_cfg_param->pst_net_dev == NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }
    alg_cfg_param.pst_mac_vap = oal_net_dev_priv(wlan_cfg_param->pst_net_dev);
    alg_cfg_param.puc_output_str = (uint8_t *)wlan_cfg_param->pc_output;

    /* Analysis input args and copy mem_block used to transfer */
    sub_alg_args_analysis = wal_wlan_get_sub_alg_args_analysis_entry(alg_cfg_hdr.en_cfg_id);
    if (sub_alg_args_analysis == NULL) {
        oam_error_log0(0, OAM_SF_CFG,
            "{wal_wlan_cfgid_alg_param_process_entry:wal_get_wlan_alg_cfg_analysis_entry entry not found!.}\r\n");
        return OAL_ERR_CODE_SUB_ALG_ANALYSIS_ENTRY_NOT_FOUND;
    }
    if (sub_alg_args_analysis->p_sub_alg_args_analysis_func == NULL) {
        oam_error_log0(0, OAM_SF_CFG,
            "{wal_wlan_cfgid_alg_param_process_entry:wal_get_wlan_alg_cfg_analysis_entry p_sub_alg_args_analysis_func "
            "not register!.}\r\n");
        return OAL_ERR_CODE_SUB_ALG_ANALYSIS_ENTRY_NOT_FOUND;
    }

    /* package alg_cfg_hdr */
    *(alg_cfg_hdr_stru *)mem_block = alg_cfg_hdr;

    /* package spec_alg mem_block */
    ret = sub_alg_args_analysis->p_sub_alg_args_analysis_func(&alg_cfg_param, mem_block, mem_len);
    if (ret != OAL_SUCC) {
        return OAL_ERR_CODE_SUB_ALG_ARGS_INVALID;
    }

    /* cross thread process */
    *msg_type = (alg_cfg_hdr.en_cfg_type == ALG_CFG_TYPE_GET) ? WAL_MSG_TYPE_QUERY : WAL_MSG_TYPE_WRITE;

    return ret;
};
/*
 * 函 数 名   : wal_wlan_cfg_get_process_entry
 * 功能描述   : 获取 指定cfg_id 在 g_ast_wlan_cfg_process_info_table 中的入口函数
 * 1.日    期   : 2018年6月20日
 * 作    者   :     wifi
 * 修改内容   : 新生成函数
 */
wlan_alg_cfg_sub_alg_args_analysis_tbl_stru *wal_get_wlan_alg_cfg_analysis_entry(
    wlan_alg_cfg_sub_alg_args_analysis_tbl_stru *process_info_tbl, uint32_t tbl_size,
    mac_alg_cfg_enum_uint16 cfg_id)
{
    uint32_t index;
    wlan_alg_cfg_sub_alg_args_analysis_tbl_stru *current_entry = NULL;

    for (index = 0; index < tbl_size; index++) {
        current_entry = process_info_tbl + index;
        if (current_entry->en_alg_cfg_id == cfg_id) {
            return current_entry;
        }
    }

    return NULL;
}

OAL_STATIC uint32_t alg_cfg_param_analyze_txmode_user_fix_mode_set(
    alg_cfg_param_stru *cfg_param, alg_cfg_param_txmode_user_stru *txomde_param)
{
    uint32_t result;
    int32_t ret;
    uint32_t value;

    if (cfg_param->uc_param_num != 2) { // num2
        oam_error_log1(0, OAM_SF_ANY,
            "{alg_cfg_param_analyze_txmode_user_fix_mode: config txmode_user_fix_mode, uc_param_num(%u) should be 2!}",
            cfg_param->uc_param_num);
        ret = sprintf_s((char *)cfg_param->puc_output_str, WLAN_CFG_HOST_MAX_RSP_LEN,
            "[Error]config txmode_user_fix_mode, uc_param_num(%u) should be 2!\n", cfg_param->uc_param_num);
        if (ret < 0) {
            oam_error_log0(0, OAM_SF_ANY, "{alg_cfg_param_analyze_txmode_user_fix_mode::sprintf_s fail}");
            return OAL_FAIL;
        }

        return OAL_FAIL;
    }
    /* v1 */
    result = alg_cfg_get_match_value(g_alg_cfg_tx_mode_tbl, oal_array_size(g_alg_cfg_tx_mode_tbl),
        cfg_param->apuc_param_args[1], &value);
    if (result != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANY,
            "{alg_cfg_param_analyze_txmode_user_fix_mode: config txmode_user_fix_mode, <value> invalid, should be "
            "{auto, chain0_fix, chain1_fix, csd_fix, stbc_fix, txbf_fix}!}");
        ret = sprintf_s((char *)cfg_param->puc_output_str, WLAN_CFG_HOST_MAX_RSP_LEN,
            "[Error]config txmode_user_fix_mode, <value> = %s invalid, should be {auto, chain0_fix, chain1_fix, "
            "csd_fix, stbc_fix, txbf_fix}!\n", cfg_param->apuc_param_args[1]);
        if (ret < 0) {
            oam_warning_log0(0, OAM_SF_ANY, "{alg_cfg_param_analyze_txmode_user_fix_mode::sprintf_s fail}");
            return OAL_FAIL;
        }

        return result;
    }
    txomde_param->en_fix_txmode = (alg_cfg_param_tx_mode_enum)value;
    return OAL_SUCC;
}

/*
 * 函 数 名   : alg_cfg_param_analyze_txmode_user_fix_mode
 * 功能描述   : alg_cfg txmode user fix mode配置命令解析函数
 * iwpriv Hisilicon0 alg_cfg " txmode_user_fix_mode <v0> <v1>"
 * <v0>用户mac地址
 * <v1>配置模式
 * iwpriv Hisilicon0 alg_cfg "get_txmode_user_fix_mode <v0>"
 * 1.日    期   : 2018年8月15日
  * 修改内容   : 新生成函数
 */
OAL_STATIC uint32_t alg_cfg_param_analyze_txmode_user_fix_mode(alg_cfg_param_stru *cfg_param,
    alg_cfg_param_txmode_user_stru *txomde_param)
{
    uint8_t mac_addr[WLAN_MAC_ADDR_LEN] = {0};
    int32_t ret;

    if (cfg_param->uc_param_num >= 1) {
        /* v0 - mac */
        if (OAL_STRLEN(cfg_param->apuc_param_args[0]) != 17) { // num17
            oam_error_log1(0, OAM_SF_ANY,
                "{alg_cfg_param_analyze_txmode_fix_mode_sw: config txmode_user_fix_mode mac length(%u), "
                "should be 17!}", OAL_STRLEN(cfg_param->apuc_param_args[0]));
            ret = sprintf_s((char *)cfg_param->puc_output_str, WLAN_CFG_HOST_MAX_RSP_LEN,
                            "[Error]config txmode_user_fix_mode mac length(%u), should be 17!\n",
                            (unsigned int)OAL_STRLEN(cfg_param->apuc_param_args[0]));
            if (ret < 0) {
                oam_error_log0(0, OAM_SF_ANY, "{alg_cfg_param_analyze_txmode_fix_mode_sw::sprintf_s fail}");
                return OAL_FAIL;
            }

            return OAL_FAIL;
        }
        oal_strtoaddr((char *)cfg_param->apuc_param_args[0],
            OAL_STRLEN((const char *)cfg_param->apuc_param_args[0]) + 1, mac_addr, WLAN_MAC_ADDR_LEN);
        ret = memcpy_s(txomde_param->auc_mac_addr, WLAN_MAC_ADDR_LEN, mac_addr, WLAN_MAC_ADDR_LEN);
        if (ret != EOK) {
            oam_warning_log0(0, OAM_SF_ANY, "{alg_cfg_param_analyze_txmode_fix_mode_sw::memcpy_s fail}");
            return OAL_FAIL;
        }
    }

    /* 若参数配置类型为set，则对参数的取值进行匹配，并将取值拷贝出来 */
    if (txomde_param->st_alg_cfg_hdr.en_cfg_type == ALG_CFG_TYPE_SET) {
        if (alg_cfg_param_analyze_txmode_user_fix_mode_set(cfg_param, txomde_param) != OAL_SUCC) {
            oam_warning_log0(0, OAM_SF_ANY, "{alg_cfg_param_analyze_txmode_user_fix_mode_set:: fail}");
            return OAL_FAIL;
        }
    } else {
        if (cfg_param->uc_param_num != 1) {
            oam_error_log1(0, OAM_SF_ANY,
                "{alg_cfg_param_analyze_txmode_user_fix_mode: config get_txmode_user_fix_mode, uc_param_num(%u) "
                "should be 1!}", cfg_param->uc_param_num);
            ret = sprintf_s((char *)cfg_param->puc_output_str, WLAN_CFG_HOST_MAX_RSP_LEN,
                "[Error]config get_txmode_user_fix_mode, uc_param_num(%u) should be 1!\n", cfg_param->uc_param_num);
            if (ret < 0) {
                oam_warning_log0(0, OAM_SF_ANY, "{alg_cfg_param_analyze_txmode_user_fix_mode::sprintf_s fail}");
                return OAL_FAIL;
            }

            return OAL_FAIL;
        }
    }
    return OAL_SUCC;
}

/*
 * 函 数 名   : alg_cfg_args_analysis_txmode_user_fix_mode
 * 功能描述   : txmode user fix mode值配置命令参数解析接口
 * 1.日    期   : 2018年8月15日
  * 修改内容   : 新生成函数
 */
uint32_t alg_cfg_args_analysis_txmode_user_fix_mode(alg_cfg_param_stru *alg_cfg_param,
    uint8_t *mem_block, uint16_t *len)
{
    uint32_t ret;

    ret = alg_cfg_param_analyze_txmode_user_fix_mode(alg_cfg_param, (alg_cfg_param_txmode_user_stru *)mem_block);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ANY,
            "{alg_cfg_args_analysis_txmode_user_fix_mode: alg_cfg_param_analyze_txmode_user_fix_mode, return %u!}",
            ret);
        return ret;
    }

    *len = sizeof(alg_cfg_param_txmode_user_stru);

    return OAL_SUCC;
}

OAL_STATIC uint32_t alg_cfg_param_analyze_txmode_user_fix_chain_set(
    alg_cfg_param_stru *cfg_param, alg_cfg_param_txmode_fix_chain_stru *param)
{
    uint8_t value;
    int32_t  ret;

    if (cfg_param->uc_param_num != 2) { // num2
        oam_error_log1(0, OAM_SF_ANY,
            "{alg_cfg_param_analyze_txmode_user_fix_chain: config txmode_user_fix_chain, uc_param_num(%u) should be 2}",
            cfg_param->uc_param_num);
        ret = sprintf_s((char *)cfg_param->puc_output_str, WLAN_CFG_HOST_MAX_RSP_LEN,
            "[Error]config txmode_user_fix_chain, uc_param_num(%u) should be 2!\n", cfg_param->uc_param_num);
        if (ret < 0) {
            oam_warning_log0(0, OAM_SF_ANY, "{alg_cfg_param_analyze_txmode_user_fix_chain::sprintf_s fail}");
            return OAL_FAIL;
        }

        return OAL_FAIL;
    }
    /* v1 */
    value = (uint8_t)oal_atoi((char *)cfg_param->apuc_param_args[1]);
    if (value > ALG_CFG_PARAM_TXMODE_MAX_CHAIN_BITMAP) {
        oam_error_log1(0, OAM_SF_ANY,
            "{alg_cfg_param_analyze_txmode_user_fix_chain: config txmode_user_fix_chain, <%d> invalid, exceed "
            "[0, 15]!}", value);
        ret = sprintf_s((char *)cfg_param->puc_output_str, WLAN_CFG_HOST_MAX_RSP_LEN,
            "[Error]config txmode_user_fix_chain, <value> = %u invalid, exceed [0, 15]!\n",
            value);
        if (ret < 0) {
            oam_warning_log0(0, OAM_SF_ANY, "{alg_cfg_param_analyze_txmode_user_fix_chain::sprintf_s fail}");
            return OAL_FAIL;
        }
        return OAL_FAIL;
    }
    param->fix_chain = value;
    return OAL_SUCC;
}

/*
 * 函 数 名   : alg_cfg_param_analyze_txmode_user_fix_chain
 * 功能描述   : alg_cfg txmode user fix chain配置命令解析函数
 * iwpriv Hisilicon0 alg_cfg " txmode_user_fix_chain <v0> <v1>"
 * <v0>用户mac地址
 * <v1>配置通道
 * iwpriv Hisilicon0 alg_cfg "get_txmode_user_fix_chain <v0>"
 * 1.日    期   : 2020年10月09日
 * 修改内容   : 新生成函数
 */
OAL_STATIC uint32_t alg_cfg_param_analyze_txmode_user_fix_chain(alg_cfg_param_stru *cfg_param,
                                                                alg_cfg_param_txmode_fix_chain_stru *param)
{
    uint8_t mac_addr[WLAN_MAC_ADDR_LEN] = {0};
    int32_t ret;

    if (cfg_param->uc_param_num >= 1) {
        /* v0 - mac */
        if (OAL_STRLEN(cfg_param->apuc_param_args[0]) != 17) { // num17
            oam_error_log1(0, OAM_SF_ANY,
                "{alg_cfg_param_analyze_txmode_user_fix_chain: config txmode_user_fix_chain mac length(%u), "
                "should be 17!}", OAL_STRLEN(cfg_param->apuc_param_args[0]));
            ret = sprintf_s((char *)cfg_param->puc_output_str, WLAN_CFG_HOST_MAX_RSP_LEN,
                            "[Error]config txmode_user_fix_chain mac length(%u), should be 17!\n",
                            (unsigned int)OAL_STRLEN(cfg_param->apuc_param_args[0]));
            if (ret < 0) {
                oam_warning_log0(0, OAM_SF_ANY, "{alg_cfg_param_analyze_txmode_user_fix_chain::sprintf_s fail}");
                return OAL_FAIL;
            }

            return OAL_FAIL;
        }
        oal_strtoaddr((char *)cfg_param->apuc_param_args[0],
            OAL_STRLEN((const char *)cfg_param->apuc_param_args[0]) + 1, mac_addr, WLAN_MAC_ADDR_LEN);
        ret = memcpy_s(param->mac_addr, WLAN_MAC_ADDR_LEN, mac_addr, WLAN_MAC_ADDR_LEN);
        if (ret != EOK) {
            oam_warning_log0(0, OAM_SF_ANY, "{alg_cfg_param_analyze_txmode_user_fix_chain::memcpy_s fail}");
            return OAL_FAIL;
        }
    }

    /* 若参数配置类型为set，则对参数的取值进行匹配，并将取值拷贝出来 */
    if (param->st_alg_cfg_hdr.en_cfg_type == ALG_CFG_TYPE_SET) {
        if (alg_cfg_param_analyze_txmode_user_fix_chain_set(cfg_param, param) != OAL_SUCC) {
            oam_warning_log0(0, OAM_SF_ANY, "{alg_cfg_param_analyze_txmode_user_fix_chain_set:: fail}");
            return OAL_FAIL;
        }
    } else {
        if (cfg_param->uc_param_num != 1) {
            oam_error_log1(0, OAM_SF_ANY,
                "{alg_cfg_param_analyze_txmode_user_fix_chain: config get_txmode_user_fix_chain, uc_param_num(%u) "
                "should be 1!}", cfg_param->uc_param_num);
            ret = sprintf_s((char *)cfg_param->puc_output_str, WLAN_CFG_HOST_MAX_RSP_LEN,
                "[Error]config get_txmode_user_fix_mode, uc_param_num(%u) should be 1!\n",
                cfg_param->uc_param_num);
            if (ret < 0) {
                oam_warning_log0(0, OAM_SF_ANY, "{alg_cfg_param_analyze_txmode_user_fix_chain::sprintf_s fail}");
                return OAL_FAIL;
            }

            return OAL_FAIL;
        }
    }
    return OAL_SUCC;
}

/*
 * 函 数 名   : alg_cfg_args_analysis_txmode_user_fix_chain
 * 功能描述   : txmode user fix chain值配置命令参数解析接口
 * 1.日    期   : 2020年10月09日
 * 修改内容   : 新生成函数
 */
uint32_t alg_cfg_args_analysis_txmode_user_fix_chain(alg_cfg_param_stru *alg_cfg_param,
                                                     uint8_t *puc_mem_block, uint16_t *pus_len)
{
    uint32_t ret;

    ret = alg_cfg_param_analyze_txmode_user_fix_chain(alg_cfg_param,
                                                      (alg_cfg_param_txmode_fix_chain_stru *)puc_mem_block);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ANY,
            "{alg_cfg_args_analysis_txmode_user_fix_chain: alg_cfg_param_analyze_txmode_user_fix_chain, return %u!}",
            ret);
        return ret;
    }

    *pus_len = sizeof(alg_cfg_param_txmode_fix_chain_stru);

    return OAL_SUCC;
}

/*
 * 函 数 名   : alg_cfg_param_analyze_txmode_debug_log_switch
 * 功能描述   : debug_log_switch 配置命令解析函数
 * 1.日    期   : 2020年10月17日
 * 修改内容   : 新生成函数
 */
uint32_t alg_cfg_param_analyze_txmode_debug_log_switch(alg_cfg_param_stru *cfg_param,
                                                       alg_cfg_param_txmode_log_switch_stru *param_switch)
{
    uint32_t sw_on = 0;
    uint32_t ret1;
    int32_t  ret;

    /* Validate param cnt */
    if (param_switch->alg_cfg_hdr.en_cfg_type == ALG_CFG_TYPE_SET && cfg_param->uc_param_num != 1) {
        oam_error_log1(0, OAM_SF_ANY,
            "{alg_cfg_param_analyze_txmode_debug_log_switch: config txmode_debug_log_switch, uc_param_num(%u)\
            should be 1!}", cfg_param->uc_param_num);
        ret = snprintf_s((char *)cfg_param->puc_output_str, WLAN_CFG_HOST_MAX_RSP_LEN,
            WLAN_CFG_HOST_MAX_RSP_LEN - 1, "[Error]config txmode_debug_log_switch, uc_param_num(%u) should be 1!\n",
            cfg_param->uc_param_num);
        if (ret < 0) {
            oam_error_log1(0, OAM_SF_ANY, "{alg_cfg_param_analyze_txmode_debug_log_switch::invalid uc_param_num,\
                snprintf_s return %d!}", ret);
        }
        return OAL_FAIL;
    }

    if (param_switch->alg_cfg_hdr.en_cfg_type == ALG_CFG_TYPE_GET && cfg_param->uc_param_num != 0) {
        oam_error_log1(0, OAM_SF_ANY, "{alg_cfg_param_analyze_txmode_debug_log_switch: get txmode_debug_log_switch,\
                       uc_param_num(%u) should be 0!}", cfg_param->uc_param_num);
        ret = snprintf_s((char *)cfg_param->puc_output_str, WLAN_CFG_HOST_MAX_RSP_LEN,
                         WLAN_CFG_HOST_MAX_RSP_LEN - 1, "[Error]get txmode_debug_log_switch, uc_param_num(%u) \
                should be 0!\n", cfg_param->uc_param_num);
        if (ret < 0) {
            oam_error_log1(0, OAM_SF_ANY, "{alg_cfg_param_analyze_txmode_debug_log_switch:: snprintf_s(get) \
                return %d!}", ret);
        }
        return OAL_FAIL;
    }

    /* Analysis param */
    if (param_switch->alg_cfg_hdr.en_cfg_type == ALG_CFG_TYPE_SET) {
        /* param0: probe_switch */
        ret1 = alg_cfg_get_match_value(g_ast_alg_cfg_bool_tbl, alg_cfg_param_get_tbl_size(ALG_CFG_PARAM_BOOL),
                                       cfg_param->apuc_param_args[0], &sw_on);
        if (ret1 != OAL_SUCC) {
            oam_error_log1(0, OAM_SF_ANY, "{alg_cfg_param_analyze_txmode_debug_log_switch:config \
                <txmode_debug_log_switch> to be %u, exceed [0, 1]!}", sw_on);
            ret = snprintf_s((char *)cfg_param->puc_output_str, WLAN_CFG_HOST_MAX_RSP_LEN,
                WLAN_CFG_HOST_MAX_RSP_LEN - 1, "[Error]config <debug_log_swtich> to be %u, exceed [0, 1]!\n",
                sw_on);
            if (ret < 0) {
                oam_error_log1(0, OAM_SF_ANY, "{alg_cfg_param_analyze_txmode_debug_log_switch:: \
                    alg_cfg_get_match_value fail, snprintf_s return %d!}", ret);
            }
            return OAL_FAIL;
        }
    }

    param_switch->debug_log_switch =
        (param_switch->alg_cfg_hdr.en_cfg_type == ALG_CFG_TYPE_SET) ? (oal_bool_enum_uint8)sw_on : 0;
    return OAL_SUCC;
}

/*
 * 函 数 名   : alg_cfg_args_analysis_txmode_debug_log_switch
 * 功能描述   : txmode_debug_log_switch 配置命令参数解析接口
 * 1.日    期   : 2020年10月17日
 * 修改内容   : 新生成函数
 */
uint32_t alg_cfg_args_analysis_txmode_debug_log_switch(alg_cfg_param_stru *alg_cfg_param,
                                                       uint8_t *puc_mem_block, uint16_t *pus_len)
{
    uint32_t ret;

    ret = alg_cfg_param_analyze_txmode_debug_log_switch(alg_cfg_param,
                                                        (alg_cfg_param_txmode_log_switch_stru *)puc_mem_block);
    if (ret != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANY, "{alg_cfg_args_analysis_txmode_debug_log_switch::param analysis fail!}");
        return ret;
    }

    *pus_len = sizeof(alg_cfg_param_txmode_log_switch_stru);

    return OAL_SUCC;
}

/*
 * 函 数 名   : alg_cfg_param_analyze_txmode_all_user_fix_mode
 * 功能描述   : alg_cfg txmode all user fix mode配置命令解析函数
 * iwpriv Hisilicon0 alg_cfg " txmode_all_user_fix_mode <v0>"
 * <v0>配置模式
 * iwpriv Hisilicon0 alg_cfg "get_txmode_user_fix_mode <v0>"
 * 1.日    期   : 2019年12月9日
  * 修改内容   : 新生成函数
 */
OAL_STATIC uint32_t alg_cfg_param_analyze_txmode_all_user_fix_mode(alg_cfg_param_stru *cfg_param,
    alg_cfg_param_txmode_all_user_stru *txomde_param)
{
    uint32_t ret, get_value;
    int32_t ret_signed;

    /* 若参数配置类型为set，则对参数的取值进行匹配，并将取值拷贝出来 */
    if (txomde_param->st_alg_cfg_hdr.en_cfg_type == ALG_CFG_TYPE_SET) {
        /* 检查参数个数 */
        if (cfg_param->uc_param_num != 1) {
            oam_error_log1(0, OAM_SF_ANY, "{alg_cfg_param_analyze_txmode_all_user_fix_mode: \
                config txmode_all_user_fix_mode, uc_param_num(%u) should be 1!}", cfg_param->uc_param_num);
            ret_signed = sprintf_s((char *)cfg_param->puc_output_str, WLAN_CFG_HOST_MAX_RSP_LEN,
                "[Error]txmode_all_user_fix_mode, uc_param_num(%u) should be 1!\n", cfg_param->uc_param_num);
            if (ret_signed < 0) {
                oam_warning_log0(0, OAM_SF_ANY, "{alg_cfg_param_analyze_txmode_all_user_fix_mode::sprintf_s fail}");
                return OAL_FAIL;
            }

            return OAL_FAIL;
        }

        /* v0 */
        ret = alg_cfg_get_match_value(g_alg_cfg_tx_mode_tbl, alg_cfg_param_get_tbl_size(ALG_CFG_PARAM_TX_MODE),
            cfg_param->apuc_param_args[0], &get_value);
        if (ret != OAL_SUCC) {
            oam_error_log0(0, OAM_SF_ANY,
                "{alg_cfg_param_analyze_txmode_all_user_fix_mode: config txmode_all_user_fix_mode, \
                <value> invalid, should be {auto, chain0_fix, chain1_fix, csd_fix, stbc_fix, txbf_fix}!}");
            ret_signed = sprintf_s((char *)cfg_param->puc_output_str, WLAN_CFG_HOST_MAX_RSP_LEN,
                "[Error]config txmode_all_user_fix_mode, <value> = %s invalid, should be "
                "{auto, chain0_fix, chain1_fix, csd_fix, stbc_fix, txbf_fix}!\n",
                cfg_param->apuc_param_args[0]);
            if (ret_signed < 0) {
                oam_warning_log0(0, OAM_SF_ANY, "{alg_cfg_param_analyze_txmode_all_user_fix_mode::sprintf_s fail}");
                return OAL_FAIL;
            }

            return ret;
        }
        txomde_param->en_fix_txmode = (alg_cfg_param_tx_mode_enum)get_value;
    } else {
        if (cfg_param->uc_param_num != 1) {
            oam_error_log1(0, OAM_SF_ANY, "{alg_cfg_param_analyze_txmode_all_user_fix_mode: \
                config get_txmode_all_user_fix_mode, uc_param_num(%u) should be 1!}", cfg_param->uc_param_num);
            ret_signed = sprintf_s((char *)cfg_param->puc_output_str, WLAN_CFG_HOST_MAX_RSP_LEN,
                "[Error]config get_txmode_all_user_fix_mode, uc_param_num(%u) should be 1!\n", cfg_param->uc_param_num);
            if (ret_signed < 0) {
                oam_warning_log0(0, OAM_SF_ANY, "{alg_cfg_param_analyze_txmode_all_user_fix_mode::sprintf_s fail}");
                return OAL_FAIL;
            }

            return OAL_FAIL;
        }
    }
    return OAL_SUCC;
}
/*
 * 函 数 名   : alg_cfg_args_analysis_txmode_all_user_fix_mode
 * 功能描述   : txmode all user fix mode值配置命令参数解析接口
 * 1.日    期   : 2019年12月9日
  * 修改内容   : 新生成函数
 */
uint32_t alg_cfg_args_analysis_txmode_all_user_fix_mode(alg_cfg_param_stru *alg_cfg_param,
    uint8_t *mem_block, uint16_t *len)
{
    uint32_t ret;

    ret = alg_cfg_param_analyze_txmode_all_user_fix_mode(alg_cfg_param,
        (alg_cfg_param_txmode_all_user_stru *)mem_block);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ANY, "{alg_cfg_args_analysis_txmode_all_user_fix_mode: \
                       alg_cfg_param_analyze_txmode_all_user_fix_mode, return %u!}", ret);
        return ret;
    }

    *len = sizeof(alg_cfg_param_txmode_all_user_stru);

    return OAL_SUCC;
}

/*
 * 函 数 名   : alg_cfg_param_analyze_txmode_mode_sw
 * 功能描述   : alg_cfg txmode mode sw 配置命令解析函数
 * iwpriv Hisilicon0 alg_cfg " txmode_mode_sw <v0>"
 * <v0>txmode是否开启，取值：enable/disable
 * 1.日    期   : 2018年8月15日
  * 修改内容   : 新生成函数
 */
OAL_STATIC uint32_t alg_cfg_param_analyze_txmode_mode_sw(alg_cfg_param_stru *cfg_param,
    alg_cfg_param_txmode_dev_stru *txomde_param)
{
    uint32_t result, value;
    int32_t ret;

    /* 若参数配置类型为set，则对参数的取值进行匹配，并将取值拷贝出来 */
    if (txomde_param->st_alg_cfg_hdr.en_cfg_type == ALG_CFG_TYPE_SET) {
        if (cfg_param->uc_param_num != 1) {
            oam_error_log1(0, OAM_SF_ANY,
                "{alg_cfg_param_analyze_txmode_mode_sw: config txmode_mode_sw, uc_param_num(%u) should be 1!}",
                cfg_param->uc_param_num);
            ret = sprintf_s((char *)cfg_param->puc_output_str, WLAN_CFG_HOST_MAX_RSP_LEN,
                "[Error]config txmode_mode_sw, uc_param_num(%u) should be 1!\n", cfg_param->uc_param_num);
            if (ret < 0) {
                oam_warning_log0(0, OAM_SF_ANY, "{alg_cfg_param_analyze_txmode_mode_sw::sprintf_s fail}");
                return OAL_FAIL;
            }

            return OAL_FAIL;
        }

        /* v0 */
        result = alg_cfg_get_match_value(g_alg_cfg_bool_tbl, alg_cfg_param_get_tbl_size(ALG_CFG_PARAM_BOOL),
            cfg_param->apuc_param_args[0], &value);
        if (result != OAL_SUCC) {
            oam_error_log0(0, OAM_SF_ANY,
                "{alg_cfg_param_analyze_txmode_mode_sw: config txmode_mode_sw, <value> invalid, should be {enable, "
                "disable}!}");
            ret = sprintf_s((char *)cfg_param->puc_output_str, WLAN_CFG_HOST_MAX_RSP_LEN,
                "[Error]config txmode_mode_sw, <value> = %s invalid, should be {enable, disable}!\n",
                cfg_param->apuc_param_args[0]);
            if (ret < 0) {
                oam_warning_log0(0, OAM_SF_ANY, "{alg_cfg_param_analyze_txmode_mode_sw::sprintf_s fail}");
                return OAL_FAIL;
            }

            return result;
        }
        txomde_param->en_txmode_sw = (oal_bool_enum_uint8)value;
    }

    return OAL_SUCC;
}

/*
 * 函 数 名   : alg_cfg_args_analysis_txmode_mode_sw
 * 功能描述   : txmode mode switch 配置命令参数解析接口
 * 1.日    期   : 2018年8月15日
  * 修改内容   : 新生成函数
 */
uint32_t alg_cfg_args_analysis_txmode_mode_sw(alg_cfg_param_stru *alg_cfg_param,
    uint8_t *pmem_block, uint16_t *len)
{
    uint32_t ret;

    ret = alg_cfg_param_analyze_txmode_mode_sw(alg_cfg_param, (alg_cfg_param_txmode_dev_stru *)pmem_block);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ANY,
            "{alg_cfg_args_analysis_txmode_mode_sw: alg_cfg_param_analyze_txmode_mode_sw, return %u!}", ret);
        return ret;
    }

    *len = sizeof(alg_cfg_param_txmode_dev_stru);

    return OAL_SUCC;
}
/*
 * 功能描述   : txmode chain switch 配置命令参数解析接口
 * 1.日    期   : 2020年8月15日
  * 修改内容   : 新生成函数
 */
uint32_t alg_cfg_args_analysis_txmode_chain_prob_sw(alg_cfg_param_stru *alg_cfg_param,
    uint8_t *pmem_block, uint16_t *len)
{
    alg_cfg_param_txmode_chain_prob_stru *para = (alg_cfg_param_txmode_chain_prob_stru *)pmem_block;
    int32_t ret;

    if (alg_cfg_param->uc_param_num != 1) {
        ret = sprintf_s((char *)alg_cfg_param->puc_output_str, WLAN_CFG_HOST_MAX_RSP_LEN,
            "txmode_chain_prob_sw: para num(%u) error\n", alg_cfg_param->uc_param_num);
        if (ret < 0) {
            oam_warning_log0(0, OAM_SF_ANY,
                "{alg_cfg_args_analysis_txmode_chain_prob_sw:: invalid para num, sprintf_s fail!}");
        }
        return OAL_FAIL;
    }

    para->chain_sw = (uint8_t)oal_atoi((char *)alg_cfg_param->apuc_param_args[0]);
    if (para->chain_sw > 1) {
        ret = sprintf_s((char *)alg_cfg_param->puc_output_str, WLAN_CFG_HOST_MAX_RSP_LEN,
            "txmode_chain_prob_sw: invalid input\n");
        if (ret < 0) {
            oam_warning_log0(0, OAM_SF_ANY,
                "{alg_cfg_args_analysis_txmode_chain_prob_sw:: invalid input, sprintf_s fail}");
        }
        return OAL_FAIL;
    }

    *len = sizeof(alg_cfg_param_txmode_chain_prob_stru);
    return OAL_SUCC;
}

/* waterfilling */
OAL_STATIC uint32_t alg_cfg_param_analyze_waterfilling_debug(alg_cfg_param_stru *cfg_param,
                                                             alg_cfg_param_waterfilling_debug_stru *param)
{
    int32_t  ret;
    if (param->alg_cfg_hdr.en_cfg_type == ALG_CFG_TYPE_SET) {
        if (cfg_param->uc_param_num != 1) {
            oam_error_log1(0, OAM_SF_ANY,
                "{alg_cfg_param_analyze_waterfilling_debug: config wf_dbg, param_num(%u) should be 1!}",
                cfg_param->uc_param_num);
            ret = sprintf_s((char *)cfg_param->puc_output_str, WLAN_CFG_HOST_MAX_RSP_LEN,
                "[Error]config wf_dbg, param_num(%u) should be 1!\n", cfg_param->uc_param_num);
            if (ret < 0) {
                oam_warning_log0(0, OAM_SF_ANY, "{alg_cfg_param_analyze_wf_dbg::sprintf_s fail}");
            }
            return OAL_FAIL;
        }
        param->waterfilling_debug = (uint8_t)oal_atoi((char *)cfg_param->apuc_param_args[0]);
        /* 如果配置参数超过范围直接返回失败 */
        if (param->waterfilling_debug != ALG_CFG_TX_WATERFILLING_ENABLE &&
            param->waterfilling_debug != ALG_CFG_TX_WATERFILLING_DISABLE) {
            ret = snprintf_s((char *)cfg_param->puc_output_str, WLAN_CFG_HOST_MAX_RSP_LEN,
                WLAN_CFG_HOST_MAX_RSP_LEN - 1,
                "[Error]analyse_wf_dbg, param_mode(%u) invalid, disable:0 ; enable:1 ;\n",
                param->waterfilling_debug);
            if (ret < 0) {
                oam_error_log2(0, OAM_SF_ANY,
                               "{alg_cfg_param_analysis_wf_dbg: snprintf_s(set) return %d, wf_dbg = %d}", ret,
                               param->waterfilling_debug);
            }
            return OAL_FAIL;
        }
    } else { /* GET VALUE */
        if (cfg_param->uc_param_num != 0) {
            oam_error_log1(0, OAM_SF_ANY, "{alg_cfg_param_analysis_wf_dbg: wf_dbg param_num(%u) should be 1!}",
                           cfg_param->uc_param_num);
            ret = snprintf_s((char *)cfg_param->puc_output_str, WLAN_CFG_HOST_MAX_RSP_LEN,
                             WLAN_CFG_HOST_MAX_RSP_LEN - 1, "[Error]analyse_wf_dbg, param_num(%u) should be 1!\n",
                             cfg_param->uc_param_num);
            if (ret < 0) {
                oam_error_log1(0, OAM_SF_ANY, "{alg_cfg_param_analysis_wf_dbg: snprintf_s(get) return %d.}", ret);
            }
            return OAL_FAIL;
        }
    }
    return OAL_SUCC;
}

OAL_STATIC uint32_t alg_cfg_param_analyze_waterfilling_mode(alg_cfg_param_stru *cfg_param,
                                                            alg_cfg_param_waterfilling_mode_stru *param)
{
    int32_t  ret;
    if (param->alg_cfg_hdr.en_cfg_type == ALG_CFG_TYPE_SET) {
        if (cfg_param->uc_param_num != 1) {
            oam_error_log1(0, OAM_SF_ANY,
                "{alg_cfg_param_analyze_waterfilling_mode: config waterfilling_mode, uc_param_num(%u) should be 1!}",
                cfg_param->uc_param_num);
            ret = sprintf_s((char *)cfg_param->puc_output_str, WLAN_CFG_HOST_MAX_RSP_LEN,
                "[Error]config wf_mode, uc_param_num(%u) should be 1!\n", cfg_param->uc_param_num);
            if (ret < 0) {
                oam_warning_log0(0, OAM_SF_ANY, "{alg_cfg_param_analyze_wf_mode::sprintf_s fail}");
            }
            return OAL_FAIL;
        }
        param->waterfilling_mode = (uint8_t)oal_atoi((char *)cfg_param->apuc_param_args[0]);
        /* 如果配置参数超过范围直接返回失败 */
        if (param->waterfilling_mode != ALG_CFG_TX_WATERFILLING_AUTO_MODE &&
            param->waterfilling_mode != ALG_CFG_TX_WATERFILLING_FIX_MODE) {
            ret = snprintf_s((char *)cfg_param->puc_output_str, WLAN_CFG_HOST_MAX_RSP_LEN,
                WLAN_CFG_HOST_MAX_RSP_LEN - 1,
                "[Error]analyse_wf_mode, uc_param_mode(%u) invalid, fix mode:0 ; auto mode:1 ;\n",
                param->waterfilling_mode);
            if (ret < 0) {
                oam_error_log2(0, OAM_SF_ANY,
                               "{alg_cfg_param_analysis_wf_mode: snprintf_s(set) return %d, wf_mode = %d}", ret,
                               param->waterfilling_mode);
            }
            return OAL_FAIL;
        }
    } else { /* GET VALUE */
        if (cfg_param->uc_param_num != 0) {
            oam_error_log1(0, OAM_SF_ANY, "{alg_cfg_param_analysis_wf_mode: wf_mode uc_param_num(%u) should be 1!}",
                           cfg_param->uc_param_num);
            ret = snprintf_s((char *)cfg_param->puc_output_str, WLAN_CFG_HOST_MAX_RSP_LEN,
                             WLAN_CFG_HOST_MAX_RSP_LEN - 1, "[Error]analyse_wf_mode, uc_param_num(%u) should be 1!\n",
                             cfg_param->uc_param_num);
            if (ret < 0) {
                oam_error_log1(0, OAM_SF_ANY, "{alg_cfg_param_analysis_wf_mode: snprintf_s(get) return %d.}", ret);
            }
            return OAL_FAIL;
        }
    }
    return OAL_SUCC;
}

OAL_STATIC uint32_t alg_cfg_param_analyze_waterfilling_enable(alg_cfg_param_stru *cfg_param,
                                                              alg_cfg_param_waterfilling_enable_stru *param)
{
    int32_t  ret;
    if (param->alg_cfg_hdr.en_cfg_type == ALG_CFG_TYPE_SET) {
        if (cfg_param->uc_param_num != 1) {
            oam_error_log1(0, OAM_SF_ANY,
                "{alg_cfg_param_analyze_waterfilling_enable: config waterfilling_enable, param_num(%u) should be 1}",
                cfg_param->uc_param_num);
            ret = sprintf_s((char *)cfg_param->puc_output_str, WLAN_CFG_HOST_MAX_RSP_LEN,
                "[Error]config wf_enable, uc_param_num(%u) should be 1!\n", cfg_param->uc_param_num);
            if (ret < 0) {
                oam_warning_log0(0, OAM_SF_ANY, "{alg_cfg_param_analyze_wf_enable::sprintf_s fail}");
            }
            return OAL_FAIL;
        }
        param->waterfilling_enable = (uint8_t)oal_atoi((char *)cfg_param->apuc_param_args[0]);
        /* 如果配置参数超过范围直接返回失败 */
        if (param->waterfilling_enable != ALG_CFG_TX_WATERFILLING_ENABLE &&
            param->waterfilling_enable != ALG_CFG_TX_WATERFILLING_DISABLE) {
            ret = snprintf_s((char *)cfg_param->puc_output_str, WLAN_CFG_HOST_MAX_RSP_LEN,
                WLAN_CFG_HOST_MAX_RSP_LEN - 1,
                "[Error]analyse_wf_enable, uc_param_enable(%u) invalid, fix mode:0 ; auto mode:1 ;\n",
                param->waterfilling_enable);
            if (ret < 0) {
                oam_error_log2(0, OAM_SF_ANY,
                               "{alg_cfg_param_analysis_wf_enable: snprintf_s(set) return %d, wf_mode = %d}", ret,
                               param->waterfilling_enable);
            }
            return OAL_FAIL;
        }
    } else { /* GET VALUE */
        if (cfg_param->uc_param_num != 0) {
            oam_error_log1(0, OAM_SF_ANY, "{alg_cfg_param_analysis_wf_enable: wf_mode uc_param_num(%u) should be 1!}",
                           cfg_param->uc_param_num);
            ret = snprintf_s((char *)cfg_param->puc_output_str, WLAN_CFG_HOST_MAX_RSP_LEN,
                             WLAN_CFG_HOST_MAX_RSP_LEN - 1, "[Error]analyse_wf_mode, uc_param_num(%u) should be 1!\n",
                             cfg_param->uc_param_num);
            if (ret < 0) {
                oam_error_log1(0, OAM_SF_ANY, "{alg_cfg_param_analysis_wf_enable: snprintf_s(get) return %d.}", ret);
            }
            return OAL_FAIL;
        }
    }
    return OAL_SUCC;
}

OAL_STATIC uint32_t alg_cfg_param_analyze_hardamard_mode(alg_cfg_param_stru *cfg_param,
                                                         alg_cfg_param_hardamard_mode_stru *param)
{
    int32_t  ret;

    if (param->alg_cfg_hdr.en_cfg_type == ALG_CFG_TYPE_SET) {
        if (cfg_param->uc_param_num != 1) {
            oam_error_log1(0, OAM_SF_ANY,
                "{alg_cfg_param_analyze_hdtx_mode: config hdtx_mode, uc_param_num(%u) should be 1!}",
                cfg_param->uc_param_num);
            ret = sprintf_s((char *)cfg_param->puc_output_str, WLAN_CFG_HOST_MAX_RSP_LEN,
                "[Error]config hdtx_mode, uc_param_num(%u) should be 1!\n", cfg_param->uc_param_num);
            if (ret < 0) {
                oam_warning_log0(0, OAM_SF_ANY, "{alg_cfg_param_analyze_hdtx_mode::sprintf_s fail}");
            }
            return OAL_FAIL;
        }
        param->hardamard_mode = (uint8_t)oal_atoi((char *)cfg_param->apuc_param_args[0]);
        /* 如果配置参数超过范围直接返回失败 */
        if (param->hardamard_mode != ALG_CFG_TX_HARDMARD_AUTO_MODE &&
            param->hardamard_mode != ALG_CFG_TX_HARDMARD_FIX_MODE) {
            ret = snprintf_s((char *)cfg_param->puc_output_str, WLAN_CFG_HOST_MAX_RSP_LEN,
                WLAN_CFG_HOST_MAX_RSP_LEN - 1,
                "[Error]analyse_hdtx_mode, uc_param_mode(%u) invalid, fix mode:0 ; auto mode:1 ;\n",
                param->hardamard_mode);
            if (ret < 0) {
                oam_error_log2(0, OAM_SF_ANY,
                               "{alg_cfg_param_analysis_hdtx_mode: snprintf_s(set) return %d, hdtx_mode = %d}", ret,
                               param->hardamard_mode);
            }
            return OAL_FAIL;
        }
    } else { /* GET VALUE */
        if (cfg_param->uc_param_num != 0) {
            oam_error_log1(0, OAM_SF_ANY, "{alg_cfg_param_analysis_hdtx_mode: hdtx_mode uc_param_num(%u) should be 1!}",
                           cfg_param->uc_param_num);
            ret = snprintf_s((char *)cfg_param->puc_output_str, WLAN_CFG_HOST_MAX_RSP_LEN,
                             WLAN_CFG_HOST_MAX_RSP_LEN - 1, "[Error]analyse_hdtx_mode, uc_param_num(%u) should be 1!\n",
                             cfg_param->uc_param_num);
            if (ret < 0) {
                oam_error_log1(0, OAM_SF_ANY, "{alg_cfg_param_analysis_hdtx_mode: snprintf_s(get) return %d.}", ret);
            }
            return OAL_FAIL;
        }
    }
    return OAL_SUCC;
}
OAL_STATIC uint32_t alg_cfg_param_analyze_hardamard_enable(alg_cfg_param_stru *cfg_param,
                                                           alg_cfg_param_hardamard_enable_stru *param)
{
    int32_t  ret;

    if (param->alg_cfg_hdr.en_cfg_type == ALG_CFG_TYPE_SET) {
        if (cfg_param->uc_param_num != 1) {
            oam_error_log1(0, OAM_SF_ANY,
                "{alg_cfg_param_analyze_hardamard_enable: config hardamard_enable, uc_param_num(%u) should be 1!}",
                cfg_param->uc_param_num);
            ret = sprintf_s((char *)cfg_param->puc_output_str, WLAN_CFG_HOST_MAX_RSP_LEN,
                "[Error]config hardamard_enable, uc_param_num(%u) should be 1!\n", cfg_param->uc_param_num);
            if (ret < 0) {
                oam_warning_log0(0, OAM_SF_ANY, "{alg_cfg_param_analyze_wf_mode::sprintf_s fail}");
            }
            return OAL_FAIL;
        }
        param->hardamard_enable = (uint8_t)oal_atoi((char *)cfg_param->apuc_param_args[0]);
        /* 如果配置参数超过范围直接返回失败 */
        if (param->hardamard_enable != ALG_CFG_TX_WATERFILLING_ENABLE &&
            param->hardamard_enable != ALG_CFG_TX_WATERFILLING_DISABLE) {
            ret = snprintf_s((char *)cfg_param->puc_output_str, WLAN_CFG_HOST_MAX_RSP_LEN,
                WLAN_CFG_HOST_MAX_RSP_LEN - 1,
                "[Error]hardamard_enable, uc_param_enable(%u) invalid, eable : 1 disable: 0;\n",
                param->hardamard_enable);
            if (ret < 0) {
                oam_error_log2(0, OAM_SF_ANY,
                    "{alg_cfg_param_analysis_hardamard_enable: snprintf_s(set) return %d, hdtx_enable = %d}", ret,
                               param->hardamard_enable);
            }
            return OAL_FAIL;
        }
    } else { /* GET VALUE */
        if (cfg_param->uc_param_num != 0) {
            oam_error_log1(0, OAM_SF_ANY,
                "{alg_cfg_param_analysis_hardamard_enable: hdtx_mode uc_param_num(%u) should be 1!}",
                cfg_param->uc_param_num);
            ret = snprintf_s((char *)cfg_param->puc_output_str, WLAN_CFG_HOST_MAX_RSP_LEN,
                             WLAN_CFG_HOST_MAX_RSP_LEN - 1, "[Error]analyse_hdtx_enable, param_num(%u) should be 1!\n",
                             cfg_param->uc_param_num);
            if (ret < 0) {
                oam_error_log1(0, OAM_SF_ANY, "{alg_cfg_param_analysis_hdtx_enable: snprintf_s(get) return %d.}", ret);
            }
            return OAL_FAIL;
        }
    }
    return OAL_SUCC;
}

OAL_STATIC uint32_t alg_cfg_param_analyze_waterfilling_limit(alg_cfg_param_stru *cfg_param,
                                                             alg_cfg_param_waterfilling_limit_stru *param)
{
    int32_t  ret;

    if (param->st_alg_cfg_hdr.en_cfg_type == ALG_CFG_TYPE_SET) {
        if (cfg_param->uc_param_num != 1) {
            oam_error_log1(0, OAM_SF_ANY,
                "{alg_cfg_param_analyze_wf_limit: config wf_limit, uc_param_num(%u) should be 1!}",
                cfg_param->uc_param_num);
            ret = sprintf_s((char *)cfg_param->puc_output_str, WLAN_CFG_HOST_MAX_RSP_LEN,
                "[Error]config wf_limit, uc_param_num(%u) should be 1!\n", cfg_param->uc_param_num);
            if (ret < 0) {
                oam_warning_log0(0, OAM_SF_ANY, "{alg_cfg_param_analyze_wf_limit::sprintf_s fail}");
            }
            return OAL_FAIL;
        }
        param->waterfilling_limit = (int8_t)oal_atoi((char *)cfg_param->apuc_param_args[0]);
        /* 如果配置参数超过范围直接返回失败 */
        if (param->waterfilling_limit > ALG_CFG_TX_WATERFILLING_MAX_LIMIT) {
            ret = snprintf_s((char *)cfg_param->puc_output_str, WLAN_CFG_HOST_MAX_RSP_LEN,
                WLAN_CFG_HOST_MAX_RSP_LEN - 1,
                "[Error]wf_limit, uc_param_enable(%u) invalid, limit < -128 || limit > 127;\n",
                param->waterfilling_limit);
            if (ret < 0) {
                oam_error_log2(0, OAM_SF_ANY,
                               "{alg_cfg_param_analysis_wf_limit: snprintf_s(set) return %d, wf_limit = %d}", ret,
                               param->waterfilling_limit);
            }
            return OAL_FAIL;
        }
    } else { /* GET VALUE */
        if (cfg_param->uc_param_num != 0) {
            oam_error_log1(0, OAM_SF_ANY, "{alg_cfg_param_analysis_wf_limit: wf_limit uc_param_num(%u) should be 1!}",
                           cfg_param->uc_param_num);
            ret = snprintf_s((char *)cfg_param->puc_output_str, WLAN_CFG_HOST_MAX_RSP_LEN,
                             WLAN_CFG_HOST_MAX_RSP_LEN - 1, "[Error]analyse_wf_limit, uc_param_num(%u) should be 1!\n",
                             cfg_param->uc_param_num);
            if (ret < 0) {
                oam_error_log1(0, OAM_SF_ANY, "{alg_cfg_param_analysis_wf_limit: snprintf_s(get) return %d.}", ret);
            }
            return OAL_FAIL;
        }
    }
    return OAL_SUCC;
}

OAL_STATIC uint32_t alg_cfg_param_analyze_waterfilling_asnr(alg_cfg_param_stru *cfg_param,
                                                            alg_cfg_param_waterfilling_asnr_stru *param)
{
    uint8_t  idx;
    int32_t  ret;
    if (param->st_alg_cfg_hdr.en_cfg_type == ALG_CFG_TYPE_SET) {
        if (cfg_param->uc_param_num > ALG_CFG_TX_WATERFILLING_SNR_NUM) {
            oam_error_log1(0, OAM_SF_ANY,
                "{alg_cfg_param_analyze_wf_asnr: config wf_limit, uc_param_num(%u) should be 1!}",
                cfg_param->uc_param_num);
            ret = sprintf_s((char *)cfg_param->puc_output_str, WLAN_CFG_HOST_MAX_RSP_LEN,
                "[Error]config wf_lasnr, uc_param_num(%u) should be 1!\n", cfg_param->uc_param_num);
            if (ret < 0) {
                oam_warning_log0(0, OAM_SF_ANY, "{alg_cfg_param_analyze_wf_asnr::sprintf_s fail}");
            }
            return OAL_FAIL;
        }
        for (idx = 0; idx < cfg_param->uc_param_num; ++idx) {
            param->waterfilling_asnr[idx] = (uint8_t)oal_atoi((char *)cfg_param->apuc_param_args[idx]);
        }
    } else { /* GET VALUE */
        if (cfg_param->uc_param_num != 0) {
            oam_error_log1(0, OAM_SF_ANY, "{alg_cfg_param_analysis_wf_asnr: wf_asnr uc_param_num(%u) should be 2!}",
                           cfg_param->uc_param_num);
            ret = snprintf_s((char *)cfg_param->puc_output_str, WLAN_CFG_HOST_MAX_RSP_LEN,
                             WLAN_CFG_HOST_MAX_RSP_LEN - 1, "[Error]analyse_wf_asnr, uc_param_num(%u) should be 1!\n",
                             cfg_param->uc_param_num);
            if (ret < 0) {
                oam_error_log1(0, OAM_SF_ANY, "{alg_cfg_param_analysis_wf_asnr: snprintf_s(get) return %d.}", ret);
            }
            return OAL_FAIL;
        }
    }
    return OAL_SUCC;
}

uint32_t alg_cfg_args_analysis_waterfilling_debug(alg_cfg_param_stru *alg_cfg_param, uint8_t *mem_block, uint16_t *len)
{
    uint32_t ret;

    ret = alg_cfg_param_analyze_waterfilling_debug(alg_cfg_param,
        (alg_cfg_param_waterfilling_debug_stru *)mem_block);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ANY,
            "{alg_cfg_args_analysis_waterfilling_debug: alg_cfg_args_analysis_waterfilling_debug, return %u!}", ret);
        return ret;
    }

    *len = sizeof(alg_cfg_param_waterfilling_debug_stru);

    return OAL_SUCC;
}

OAL_STATIC uint32_t alg_cfg_param_analyze_wf_force_flag(alg_cfg_param_stru *cfg_param,
    alg_cfg_param_waterfilling_force_flag_stru *param)
{
    int32_t  ret;

    if (param->alg_cfg_hdr.en_cfg_type == ALG_CFG_TYPE_SET) {
        if (cfg_param->uc_param_num != 1) {
            oam_error_log1(0, OAM_SF_WATERFILLING,
                "{alg_cfg_param_analyze_wf_force_flag: uc_param_num(%u) should be 1!}",
                cfg_param->uc_param_num);
            ret = sprintf_s((char *)cfg_param->puc_output_str, WLAN_CFG_HOST_MAX_RSP_LEN,
                "[Error]config wf_force_flag, uc_param_num(%u) should be 1!\n", cfg_param->uc_param_num);
            if (ret < 0) {
                oam_warning_log0(0, OAM_SF_WATERFILLING, "{alg_cfg_param_analyze_wf_force_flag::sprintf_s fail}");
            }
            return OAL_FAIL;
        }
        param->wf_force_flag = (uint8_t)oal_atoi((char *)cfg_param->apuc_param_args[0]);
        /* 濡傛灉閰嶇疆鍙傛暟瓒呰繃鑼冨洿鐩存帴杩斿洖澶辫触 */
        if (param->wf_force_flag != OAL_TRUE &&
            param->wf_force_flag != OAL_FALSE) {
            ret = snprintf_s((char *)cfg_param->puc_output_str, WLAN_CFG_HOST_MAX_RSP_LEN,
                WLAN_CFG_HOST_MAX_RSP_LEN - 1,
                "[Error]wf_force_flag, uc_param_dbg(%u) invalid, eable : 1 disable: 0;\n",
                param->wf_force_flag);
            if (ret < 0) {
                oam_error_log2(0, OAM_SF_WATERFILLING,
                               "{alg_cfg_param_analyze_wf_force_flag: snprintf_s(set) return %d, wf_debug = %d}", ret,
                               param->wf_force_flag);
            }
            return OAL_FAIL;
        }
    } else { /* GET VALUE */
        if (cfg_param->uc_param_num != 0) {
            oam_error_log1(0, OAM_SF_WATERFILLING, "{alg_cfg_param_analyze_wf_force_flag: param_num(%u) should be 1!}",
                           cfg_param->uc_param_num);
            ret = snprintf_s((char *)cfg_param->puc_output_str, WLAN_CFG_HOST_MAX_RSP_LEN,
                             WLAN_CFG_HOST_MAX_RSP_LEN - 1, "[Error]wf_force_flag, param_num(%u) should be 1!\n",
                             cfg_param->uc_param_num);
            if (ret < 0) {
                oam_error_log1(0, OAM_SF_WATERFILLING, "{alg_cfg_param_analyze_wf_force_flag:snprintf_s(get) \
                    return %d.}", ret);
            }
            return OAL_FAIL;
        }
    }
    return OAL_SUCC;
}
OAL_STATIC uint32_t alg_cfg_param_analyze_hdtx_force_flag(alg_cfg_param_stru *cfg_param,
    alg_cfg_param_hardamard_force_flag_stru *param)
{
    int32_t  ret;

    if (param->alg_cfg_hdr.en_cfg_type == ALG_CFG_TYPE_SET) {
        if (cfg_param->uc_param_num != 1) {
            oam_error_log1(0, OAM_SF_WATERFILLING,
                "{alg_cfg_param_analyze_hdtx_force_flag: uc_param_num(%u) should be 1!}",
                cfg_param->uc_param_num);
            ret = sprintf_s((char *)cfg_param->puc_output_str, WLAN_CFG_HOST_MAX_RSP_LEN,
                "[Error]config hdtx_force_flag, uc_param_num(%u) should be 1!\n", cfg_param->uc_param_num);
            if (ret < 0) {
                oam_warning_log0(0, OAM_SF_WATERFILLING, "{alg_cfg_param_analyze_hdtx_force_flag::sprintf_s fail}");
            }
            return OAL_FAIL;
        }
        param->hdtx_force_flag = (oal_bool_enum_uint8)oal_atoi((char *)cfg_param->apuc_param_args[0]);
        /* 濡傛灉閰嶇疆鍙傛暟瓒呰繃鑼冨洿鐩存帴杩斿洖澶辫触 */
        if (param->hdtx_force_flag != OAL_TRUE &&
            param->hdtx_force_flag != OAL_FALSE) {
            ret = snprintf_s((char *)cfg_param->puc_output_str, WLAN_CFG_HOST_MAX_RSP_LEN,
                WLAN_CFG_HOST_MAX_RSP_LEN - 1,
                "[Error]hdtx_force_flag, uc_param_dbg(%u) invalid, eable : 1 disable: 0;\n",
                param->hdtx_force_flag);
            if (ret < 0) {
                oam_error_log2(0, OAM_SF_WATERFILLING,
                               "{alg_cfg_param_analyze_hdtx_force_flag: snprintf_s(set) return %d, wf_debug = %d}", ret,
                               param->hdtx_force_flag);
            }
            return OAL_FAIL;
        }
    } else { /* GET VALUE */
        if (cfg_param->uc_param_num != 0) {
            oam_error_log1(0, OAM_SF_WATERFILLING, "{alg_cfg_param_analyze_hdtx_force_flag: param_num(%u) \
                should be 1!}", cfg_param->uc_param_num);
            ret = snprintf_s((char *)cfg_param->puc_output_str, WLAN_CFG_HOST_MAX_RSP_LEN,
                             WLAN_CFG_HOST_MAX_RSP_LEN - 1, "[Error]hdtx_force_flag, param_num(%u) should be 1!",
                             cfg_param->uc_param_num);
            if (ret < 0) {
                oam_error_log1(0, OAM_SF_WATERFILLING, "{alg_cfg_param_analyze_hdtx_force_flag:snprintf_s(get) \
                    return %d.}", ret);
            }
            return OAL_FAIL;
        }
    }
    return OAL_SUCC;
}

uint32_t alg_cfg_args_analysis_waterfilling_mode(alg_cfg_param_stru *pst_alg_cfg_param,
                                                 uint8_t *puc_mem_block, uint16_t *len)
{
    uint32_t ret;

    ret = alg_cfg_param_analyze_waterfilling_mode(pst_alg_cfg_param,
        (alg_cfg_param_waterfilling_mode_stru *)puc_mem_block);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ANY,
            "{alg_cfg_args_analysis_waterfilling_mode: alg_cfg_args_analysis_waterfilling_mode, return %u!}", ret);
        return ret;
    }

    *len = sizeof(alg_cfg_param_waterfilling_mode_stru);

    return OAL_SUCC;
}

uint32_t alg_cfg_args_analysis_waterfilling_enable(alg_cfg_param_stru *alg_cfg_param,
                                                   uint8_t *mem_block, uint16_t *len)
{
    uint32_t ret;

    ret = alg_cfg_param_analyze_waterfilling_enable(alg_cfg_param,
        (alg_cfg_param_waterfilling_enable_stru *)mem_block);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ANY,
            "{alg_cfg_args_analysis_waterfilling_enable: alg_cfg_args_analysis_waterfilling_enable, return %u!}", ret);
        return ret;
    }

    *len = sizeof(alg_cfg_param_waterfilling_enable_stru);

    return OAL_SUCC;
}
uint32_t alg_cfg_args_analysis_hardamard_mode(alg_cfg_param_stru *alg_cfg_param,
                                              uint8_t *mem_block, uint16_t *len)
{
    uint32_t ret;

    ret = alg_cfg_param_analyze_hardamard_mode(alg_cfg_param,
        (alg_cfg_param_hardamard_mode_stru *)mem_block);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ANY,
            "{alg_cfg_args_analysis_hardamard_mode: alg_cfg_args_analysis_hardamard_mode, return %u!}", ret);
        return ret;
    }

    *len = sizeof(alg_cfg_param_hardamard_mode_stru);

    return OAL_SUCC;
}
uint32_t alg_cfg_args_analysis_hardamard_enable(alg_cfg_param_stru *alg_cfg_param,
                                                uint8_t *mem_block, uint16_t *len)
{
    uint32_t ret;

    ret = alg_cfg_param_analyze_hardamard_enable(alg_cfg_param,
        (alg_cfg_param_hardamard_enable_stru *)mem_block);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ANY,
            "{alg_cfg_args_analysis_hardamard_enable: alg_cfg_args_analysis_hardamard_enable, return %u!}", ret);
        return ret;
    }

    *len = sizeof(alg_cfg_param_hardamard_enable_stru);

    return OAL_SUCC;
}

uint32_t alg_cfg_args_analysis_waterfilling_limit(alg_cfg_param_stru *alg_cfg_param,
                                                  uint8_t *mem_block, uint16_t *len)
{
    uint32_t ret;

    ret = alg_cfg_param_analyze_waterfilling_limit(alg_cfg_param,
        (alg_cfg_param_waterfilling_limit_stru *)mem_block);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ANY,
            "{alg_cfg_args_analysis_waterfilling_limit: alg_cfg_args_analysis_waterfilling_limit, return %u!}",
            ret);
        return ret;
    }

    *len = sizeof(alg_cfg_param_waterfilling_limit_stru);

    return OAL_SUCC;
}

uint32_t alg_cfg_args_analysis_waterfilling_asnr(alg_cfg_param_stru *alg_cfg_param,
                                                 uint8_t *mem_block, uint16_t *len)
{
    uint32_t ret;

    ret = alg_cfg_param_analyze_waterfilling_asnr(alg_cfg_param,
        (alg_cfg_param_waterfilling_asnr_stru *)mem_block);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ANY,
            "{alg_cfg_args_analysis_waterfilling_asnr: alg_cfg_args_analysis_waterfilling_asnr, return %u!}", ret);
        return ret;
    }

    *len = sizeof(alg_cfg_param_waterfilling_asnr_stru);

    return OAL_SUCC;
}

/*
 * 函 数 名   : atoi_sub_asnr
 * 功能描述   ：将字符串"rank[x]"/"c[y]"转换为数字x/y。
 * 1.日    期   : 2022/06/02
 *   作    者   : wifi
 *   修改内容   : 新生成函数
 */
OAL_STATIC uint8_t atoi_sub_asnr(const char *c_string)
{
    uint32_t digit = 4; // 合法的返回值应该在[0,3]
    uint32_t len = 0;

    if (c_string == NULL) {
        oam_error_log0(0, OAM_SF_WATERFILLING, "{atoi_sub_asnr: c_string == NULL.}");
        return 0;
    }

    /* Skip any leading blanks */
    while ((*c_string) == ' ') {
        c_string += 1;
        len++;
    }

    if (*c_string == 'r') {
        digit = *(c_string + 4) - '0'; // 4: 从字符串"rank[x]"开头偏移4个字节，得到数字x
    }
    if (*c_string == 'c') {
        digit = *(c_string + 1) - '0'; // 1: 从字符串"c[y]"开头偏移1个字节，得到数字y
    }
    return digit;
}

/*
 * 函 数 名   : check_argument_sub_asnr
 * 功能描述   ：检查入参取值是否合法：alg_cfg_param_waterfilling_sub_asnr_stru
 * 1.日    期   : 2022/06/02
 *   作    者   : wifi
 *   修改内容   : 新生成函数
 */
OAL_STATIC uint32_t check_argument_sub_asnr(alg_cfg_param_stru *cfg_param,
                                            alg_cfg_param_waterfilling_sub_asnr_stru *param)
{
    int32_t ret;
    uint8_t i;

    if (param->waterfilling_rank_num > 3) { // 速率等级取值范围[0,3]
        ret = snprintf_s((char *)cfg_param->puc_output_str, WLAN_CFG_HOST_MAX_RSP_LEN,
            WLAN_CFG_HOST_MAX_RSP_LEN - 1,
            "[Error]analyze_waterfilling_sub_asnr, uc_param_value(%u) invalid, it should be in [0,3];\n",
            param->waterfilling_rank_num);
        if (ret < 0) {
            oam_error_log2(0, OAM_SF_ANY,
                "{alg_cfg_param_analyze_waterfilling_sub_asnr: snprintf_s(set) return %d, rank_num = %d}",
                ret, param->waterfilling_rank_num);
        }
        return OAL_FAIL;
    }
    if (param->waterfilling_nss_num > 3) { // 空间流取值范围[0,3]
        ret = snprintf_s((char *)cfg_param->puc_output_str, WLAN_CFG_HOST_MAX_RSP_LEN,
            WLAN_CFG_HOST_MAX_RSP_LEN - 1,
            "[Error]analyze_waterfilling_sub_asnr, uc_param_value(%u) invalid, it should be in [0,3];\n",
            param->waterfilling_nss_num);
        if (ret < 0) {
            oam_error_log2(0, OAM_SF_ANY,
                "{alg_cfg_param_analyze_waterfilling_sub_asnr: snprintf_s(set) return %d, nss_num = %d}",
                ret, param->waterfilling_nss_num);
        }
        return OAL_FAIL;
    }
    for (i = 0; i < HAL_WF_SUB_BW_NUM; i++) {
        if (param->waterfilling_asnr_value[i] > 63) { // 注水值在描述符里面只分配了6 bit，取值范围为[0,63]
            ret = snprintf_s((char *)cfg_param->puc_output_str, WLAN_CFG_HOST_MAX_RSP_LEN,
                WLAN_CFG_HOST_MAX_RSP_LEN - 1,
                "[Error]analyze_waterfilling_sub_asnr, uc_param_value(%u) invalid,  it should be in [0,63];\n",
                param->waterfilling_asnr_value[i]);
            if (ret < 0) {
                oam_error_log3(0, OAM_SF_ANY,
                    "{alg_cfg_param_analyze_waterfilling_sub_asnr: snprintf_s(set) return %d, \
                    waterfilling_asnr_value[%d] = %d}", ret, i, param->waterfilling_asnr_value[i]);
            }
            return OAL_FAIL;
        }
    }
    return OAL_SUCC;
}

/*
 * 函 数 名   : alg_cfg_param_analyze_waterfilling_sub_asnr
 * 功能描述   ：自带注水命令的入参解析函数
 * 1.日    期   : 2022/06/02
 *   作    者   : wifi
 *   修改内容   : 新生成函数
 */
OAL_STATIC uint32_t alg_cfg_param_analyze_waterfilling_sub_asnr(alg_cfg_param_stru *cfg_param,
                                                                alg_cfg_param_waterfilling_sub_asnr_stru *param)
{
    int32_t ret;
    uint8_t i;

    if (param->alg_cfg_hdr.en_cfg_type == ALG_CFG_TYPE_SET) {
        if (cfg_param->uc_param_num != 2 + HAL_WF_SUB_BW_NUM) { // 2: 前2个参数是rank nss
            oam_error_log2(0, OAM_SF_ANY, "{alg_cfg_param_analyze_waterfilling_sub_asnr:config waterfilling_sub_asnr,\
                uc_param_num(%u) should be %d!}", cfg_param->uc_param_num, 2 + HAL_WF_SUB_BW_NUM); // 2: 2个参数
            ret = sprintf_s((char *)cfg_param->puc_output_str, WLAN_CFG_HOST_MAX_RSP_LEN,
                "[Error]config waterfilling_sub_asnr, uc_param_num(%u) should be %d!\n", cfg_param->uc_param_num,
                2 + HAL_WF_SUB_BW_NUM); // 2: 2个参数
            if (ret < 0) {
                oam_warning_log0(0, OAM_SF_ANY, "{alg_cfg_param_analyze_waterfilling_sub_asnr::sprintf_s fail}");
            }
            return OAL_FAIL;
        }
        param->waterfilling_rank_num = (uint8_t)atoi_sub_asnr((char *)cfg_param->apuc_param_args[0]);
        param->waterfilling_nss_num = (uint8_t)atoi_sub_asnr((char *)cfg_param->apuc_param_args[1]);
        for (i = 0; i < HAL_WF_SUB_BW_NUM; i++) {
            param->waterfilling_asnr_value[i] =
                (uint8_t)oal_atoi((char *)cfg_param->apuc_param_args[BYTE_OFFSET_2 + i]);
        }
        /* 如果配置参数超过范围直接返回失败 */
        if (check_argument_sub_asnr(cfg_param, param) != OAL_SUCC) {
            return OAL_FAIL;
        }
    }

    return OAL_SUCC;
}
/*
 * 函 数 名   : alg_cfg_args_analysis_waterfilling_sub_asnr
 * 功能描述   ：自带注水命令的入参分析函数入口
 * 1.日    期   : 2022/06/02
 *   作    者   : wifi
 *   修改内容   : 新生成函数
 */
uint32_t alg_cfg_args_analysis_waterfilling_sub_asnr(alg_cfg_param_stru *alg_cfg_param, uint8_t *mem_block,
    uint16_t *len)
{
    uint32_t ret;

    ret = alg_cfg_param_analyze_waterfilling_sub_asnr(alg_cfg_param,
        (alg_cfg_param_waterfilling_sub_asnr_stru *)mem_block);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_WATERFILLING,
            "{alg_cfg_args_analysis_waterfilling_sub_asnr: alg_cfg_param_analyze_waterfilling_sub_asnr return %u!}",
            ret);
        return ret;
    }
    *len = sizeof(alg_cfg_param_waterfilling_sub_asnr_stru);

    return OAL_SUCC;
}

OAL_STATIC uint32_t alg_cfg_param_analyze_waterfilling_bw_mode(alg_cfg_param_stru *cfg_param,
    alg_cfg_param_waterfilling_bw_mode_stru *param)
{
    int32_t ret;

    if (param->alg_cfg_hdr.en_cfg_type == ALG_CFG_TYPE_SET) {
        if (cfg_param->uc_param_num != 1) {
            oam_error_log1(0, OAM_SF_ANY, "{alg_cfg_param_analyze_waterfilling_bw_mode:config waterfilling_bw_mode,\
                uc_param_num(%u) should be 1!}", cfg_param->uc_param_num);
            ret = sprintf_s((char *)cfg_param->puc_output_str, WLAN_CFG_HOST_MAX_RSP_LEN,
                "[Error]config waterfilling_bw_mode, uc_param_num(%u) should be %d!\n", cfg_param->uc_param_num, 1);
            if (ret < 0) {
                oam_warning_log0(0, OAM_SF_ANY, "{alg_cfg_param_analyze_waterfilling_bw_mode::sprintf_s fail}");
            }
            return OAL_FAIL;
        }
        param->waterfilling_bw_mode = (uint8_t)oal_atoi((char *)cfg_param->apuc_param_args[0]);
    }
    return OAL_SUCC;
}
uint32_t alg_cfg_args_analysis_waterfilling_bw_mode(alg_cfg_param_stru *alg_cfg_param, uint8_t *mem_block,
    uint16_t *len)
{
    uint32_t ret;

    ret = alg_cfg_param_analyze_waterfilling_bw_mode(alg_cfg_param,
        (alg_cfg_param_waterfilling_bw_mode_stru *)mem_block);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_WATERFILLING, "{alg_cfg_args_analysis_waterfilling_bw_mode:  return %u!}", ret);
        return ret;
    }
    *len = sizeof(alg_cfg_param_waterfilling_bw_mode_stru);

    return OAL_SUCC;
}

uint32_t alg_cfg_args_analysis_wf_force_flag(alg_cfg_param_stru *alg_cfg_param,
    uint8_t *mem_block, uint16_t *len)
{
    uint32_t ret;

    ret = alg_cfg_param_analyze_wf_force_flag(alg_cfg_param,
        (alg_cfg_param_waterfilling_force_flag_stru *)mem_block);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ANY,
            "{alg_cfg_args_analysis_wf_force_flag: analysis_wf_force_flag, return %u!}", ret);
        return ret;
    }

    *len = sizeof(alg_cfg_param_waterfilling_force_flag_stru);

    return OAL_SUCC;
}
uint32_t alg_cfg_args_analysis_hdtx_force_flag(alg_cfg_param_stru *alg_cfg_param,
    uint8_t *mem_block, uint16_t *len)
{
    uint32_t ret;

    ret = alg_cfg_param_analyze_hdtx_force_flag(alg_cfg_param,
        (alg_cfg_param_hardamard_force_flag_stru *)mem_block);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ANY,
            "{alg_cfg_args_analysis_hdtx_force_flag: analysis_hdtx_force_flag, return %u!}", ret);
        return ret;
    }

    *len = sizeof(alg_cfg_param_hardamard_force_flag_stru);

    return OAL_SUCC;
}

/*
 * 函 数 名  : wal_hipriv_set_fix_rate_pre_config
 * 功能描述  : 配置固定速率之前先配置自动速率和ampdu
 * 1.日    期  : 2017年2月04日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_set_fix_rate_pre_config(oal_net_device_stru *net_dev, mac_vap_stru *mac_vap,
    oal_bool_enum_uint8 en_fix_rate_enable, mac_cfg_set_dscr_param_stru *pc_stu)
{
    uint8_t ampdu_cfg_idx;
    uint32_t ret;
    int8_t ac_sw_ampdu_cmd[WAL_AMPDU_CFG_BUTT][WAL_HIPRIV_CMD_NAME_MAX_LEN] = { { "0" }, { "1" } };

    /* 配置无效固定速率时，开启autorate，恢复聚合开关值 */
    if (en_fix_rate_enable == OAL_FALSE) {
        /* 开启autorate算法 */
        ret = wal_hipriv_alg_cfg(net_dev, "ar_enable 1");
        if (ret != OAL_SUCC) {
            oam_error_log1(mac_vap->uc_vap_id, 0, "{wal_hipriv_set_fix_rate_pre_config:ar_enable fail:d%}", ret);
            return OAL_FAIL;
        }

        if (mac_vap->st_fix_rate_pre_para.en_rate_cfg_tag == OAL_TRUE) {
            /*  恢复ampdu聚合  */
            ampdu_cfg_idx = mac_vap->st_fix_rate_pre_para.en_tx_ampdu_last;
            ret = wal_hipriv_ampdu_tx_on(net_dev, ac_sw_ampdu_cmd[ampdu_cfg_idx]);
            if (ret != OAL_SUCC) {
                oam_error_log1(mac_vap->uc_vap_id, OAM_SF_CFG,
                    "{wal_hipriv_set_fix_rate_pre_config::ampdu cmd cfg fail[d%]}", ret);
                return OAL_FAIL;
            }

            /* 记录固定速率配置标记为未配置状态 */
            mac_vap->st_fix_rate_pre_para.en_rate_cfg_tag = OAL_FALSE;
        }

        return OAL_SUCC;
    }

    /*  关闭autorate算法  */
    ret = wal_hipriv_alg_cfg(net_dev, "ar_enable 0");
    if (ret != OAL_SUCC) {
        oam_error_log1(mac_vap->uc_vap_id, 0, "{wal_hipriv_set_fix_rate_pre_config::ar disable fail[d%]}", ret);
        return OAL_FAIL;
    }

    /* 11abg模式下配置固定速率前关闭ampdu聚合 */
    if (pc_stu->uc_function_index == WAL_DSCR_PARAM_RATE) {
        if (mac_vap->st_fix_rate_pre_para.en_rate_cfg_tag != OAL_TRUE) {
            /* 记录ampdu开关状态 */
            mac_vap->st_fix_rate_pre_para.en_tx_ampdu_last = mac_mib_get_CfgAmpduTxAtive(mac_vap);
            /* 记录固定速率配置标记为已配置状态 */
            mac_vap->st_fix_rate_pre_para.en_rate_cfg_tag = OAL_TRUE;
        }

        /*  关闭ampdu聚合  */
        ret = wal_hipriv_ampdu_tx_on(net_dev, ac_sw_ampdu_cmd[WAL_AMPDU_DISABLE]);
        if (ret != OAL_SUCC) {
            oam_error_log1(mac_vap->uc_vap_id, OAM_SF_CFG,
                "{wal_hipriv_set_fix_rate_pre_config::disable ampdu fail:%d}", ret);
            return OAL_FAIL;
        }
    }

    return OAL_SUCC;
}

OAL_STATIC uint32_t wal_hipriv_set_non_ht_rate(mac_cfg_non_ht_rate_stru *pst_set_non_ht_rate_param,
    wal_dscr_param_enum_uint8 en_param_index)
{
    if (en_param_index <= WLAN_SHORT_11B_11_M_BPS) {
        pst_set_non_ht_rate_param->en_protocol_mode = WLAN_11B_PHY_PROTOCOL_MODE;
    } else if (en_param_index >= WLAN_LEGACY_OFDM_48M_BPS && en_param_index <= WLAN_LEGACY_OFDM_9M_BPS) {
        pst_set_non_ht_rate_param->en_protocol_mode = WLAN_LEGACY_OFDM_PHY_PROTOCOL_MODE;
    } else {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_set_non_ht_rate::invalid rate!}");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }
    return OAL_SUCC;
}

OAL_STATIC uint32_t wal_hipriv_process_rate_params_rate(oal_net_device_stru *net_dev,
    int8_t *ac_arg, mac_cfg_set_dscr_param_stru *pc_stu)
{
    wal_dscr_param_enum_uint8 en_param_index;
    mac_cfg_non_ht_rate_stru *pst_set_non_ht_rate_param = NULL;
    mac_vap_stru *mac_vap = NULL;

    mac_vap = oal_net_dev_priv(net_dev);
    if (oal_unlikely(mac_vap == NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_hipriv_process_rate_params_rate::(net_dev)null ptr.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_set_non_ht_rate_param = (mac_cfg_non_ht_rate_stru *)(&(pc_stu->l_value));
    /* 解析参数 */
    for (en_param_index = 0; en_param_index < WLAN_LEGACY_RATE_VALUE_BUTT; en_param_index++) {
        if (!oal_strcmp(g_pauc_non_ht_rate_tbl[en_param_index], ac_arg)) {
            pst_set_non_ht_rate_param->en_rate = en_param_index;
            break;
        }
    }

    /* 根据速率配置TX描述符中的协议模式 ; 对于mp15或者mp13 ram 会在dmac 根据速率刷新协议 */
    if (OAL_SUCC != wal_hipriv_set_non_ht_rate(pst_set_non_ht_rate_param, en_param_index)) {
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    if (OAL_SUCC != wal_hipriv_set_fix_rate_pre_config(net_dev, mac_vap, OAL_TRUE, pc_stu)) {
        return OAL_FAIL;
    }

    return OAL_SUCC;
}

OAL_STATIC uint32_t wal_hipriv_process_rate_params_mcs(oal_net_device_stru *net_dev, int8_t *ac_arg,
    mac_cfg_set_dscr_param_stru *pc_stu)
{
    int32_t l_val;
    uint8_t uc_mcs_index;
    uint32_t ret;
    mac_vap_stru *mac_vap = NULL;

    mac_vap = oal_net_dev_priv(net_dev);
    if (oal_unlikely(mac_vap == NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_hipriv_process_rate_params_mcs::(net_dev)null ptr.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    l_val = oal_strtol(ac_arg, NULL, 0);
    uc_mcs_index = pc_stu->uc_function_index - WAL_DSCR_PARAM_MCS;
    if (l_val < g_al_mcs_min_table[uc_mcs_index] || l_val > g_al_mcs_max_table[uc_mcs_index]) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_process_rate_params::input mcs out of range[%d]}", l_val);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }
    pc_stu->l_value = l_val;
    ret = wal_hipriv_set_fix_rate_pre_config(net_dev, mac_vap, OAL_TRUE, pc_stu);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ANY, "{wal_hipriv_process_rate_params:cfg fixed rate hdl fail[%d]}", ret);
        return OAL_FAIL;
    }

    return OAL_SUCC;
}

OAL_STATIC uint32_t wal_hipriv_process_rate_params_nss(int8_t *ac_arg,
    mac_cfg_set_dscr_param_stru *pc_stu)
{
    wal_dscr_param_enum_uint8 en_param_index;

    for (en_param_index = 0; en_param_index < WLAN_NSS_LIMIT; en_param_index++) {
        if (!oal_strcmp(g_pauc_tx_dscr_nss_tbl[en_param_index], ac_arg)) {
            pc_stu->l_value = en_param_index;
            break;
        }
    }
    if (en_param_index == WAL_DSCR_PARAM_BUTT) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_process_rate_params::invalid param for nss!}");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    return OAL_SUCC;
}

OAL_STATIC uint32_t wal_hipriv_process_rate_params_bw(int8_t *ac_arg,
    mac_cfg_set_dscr_param_stru *pc_stu)
{
    wal_dscr_param_enum_uint8 en_param_index;

    for (en_param_index = 0; en_param_index < WLAN_BANDWITH_CAP_BUTT; en_param_index++) {
        if (!oal_strcmp(g_pauc_bw_tbl[en_param_index], ac_arg)) {
            pc_stu->l_value = en_param_index;
            break;
        }
    }
    if (en_param_index >= WLAN_BANDWITH_CAP_BUTT) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_process_rate_params::invalid param for bandwidth!}");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    return OAL_SUCC;
}

OAL_STATIC uint32_t wal_hipriv_process_rate_params_by_dscr_func(oal_net_device_stru *net_dev,
    int8_t *ac_arg, mac_cfg_set_dscr_param_stru *pc_stu, uint8_t ac_arg_len)
{
    uint32_t ret = OAL_FAIL;

    if (g_wlan_spec_cfg->feature_11ax_er_su_dcm_is_open
#ifdef  _PRE_WLAN_FEATURE_11AX_ER_SU_DCM
        && (pc_stu->uc_function_index == WAL_DSCR_PARAM_MCSAX_ER)
#endif
        ) {
        ret = wal_hipriv_process_rate_params_mcs(net_dev, ac_arg, pc_stu);
    } else {
        switch (pc_stu->uc_function_index) {
            case WAL_DSCR_PARAM_RATE:
                ret = wal_hipriv_process_rate_params_rate(net_dev, ac_arg, pc_stu);
                break;
            case WAL_DSCR_PARAM_MCS:
            case WAL_DSCR_PARAM_MCSAC:
            case WAL_DSCR_PARAM_MCSAX:
                ret = wal_hipriv_process_rate_params_mcs(net_dev, ac_arg, pc_stu);
                break;

            case WAL_DSCR_PARAM_NSS:
                ret = wal_hipriv_process_rate_params_nss(ac_arg, pc_stu);
                break;

            case WAL_DSCR_PARAM_BW:
                ret = wal_hipriv_process_rate_params_bw(ac_arg, pc_stu);
                break;

            default:
                oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_process_rate_params::invalid cmd!}");
                return OAL_ERR_CODE_INVALID_CONFIG;
        }
    }

    if (ret != OAL_SUCC) {
        return ret;
    }

    return OAL_SUCC;
}

/*
 *   功能描述  : 解析速率、空间流、带宽参数
 * 1.日    期  : 2016年7月18日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_process_rate_params(oal_net_device_stru *net_dev,
    int8_t *ac_arg, uint32_t arg_len, mac_cfg_set_dscr_param_stru *pc_stu)
{
    uint32_t ret;
    int32_t l_val;
    mac_vap_stru *mac_vap = NULL;

    mac_vap = oal_net_dev_priv(net_dev);
    if (oal_unlikely(mac_vap == NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_hipriv_process_rate_params::(net_dev)null ptr.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /*  配置无效速率值255时恢复自动速率 */
    l_val = oal_strtol(ac_arg, NULL, 0);
    if (wal_ioctl_is_invalid_fixed_rate(l_val, pc_stu)) {
        ret = wal_hipriv_set_fix_rate_pre_config(net_dev, mac_vap, OAL_FALSE, pc_stu);
        if (ret != OAL_SUCC) {
            return OAL_FAIL;
        }
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    /* 非legacy速率配置恢复ampdu聚合  */
    if ((pc_stu->uc_function_index >= WAL_DSCR_PARAM_MCS) && (pc_stu->uc_function_index <= WAL_DSCR_PARAM_MCSAC)) {
        ret = wal_hipriv_ampdu_tx_on(net_dev, "1");
        if (ret != OAL_SUCC) {
            oam_error_log1(mac_vap->uc_vap_id, OAM_SF_CFG,
                "{wal_hipriv_process_rate_params::enable ampdu fail:%d}", ret);
            return OAL_FAIL;
        }
    }

    ret = wal_hipriv_process_rate_params_by_dscr_func(net_dev, ac_arg, pc_stu, WAL_HIPRIV_CMD_NAME_MAX_LEN);
    if (ret != OAL_SUCC) {
        return ret;
    }

    return OAL_SUCC;
}

OAL_STATIC uint32_t wal_hipriv_alg_cfg_get_cmd_idx(int8_t *ac_name, uint8_t *map_idx)
{
    wal_ioctl_alg_cfg_stru st_alg_cfg;
    uint8_t idx = 0;

    /* 寻找匹配的命令 */
    st_alg_cfg = g_ast_alg_cfg_map[0];
    while (st_alg_cfg.pc_name != NULL) {
        if (0 == oal_strcmp(st_alg_cfg.pc_name, ac_name)) {
            break;
        }
        st_alg_cfg = g_ast_alg_cfg_map[++idx];
    }

    oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_alg_cfg_get_cmd_idx:: alg_cfg cmd[%d]!}", st_alg_cfg.en_alg_cfg);

    /* 没有找到对应的命令，则报错 */
    if (st_alg_cfg.pc_name == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_alg_cfg::invalid alg_cfg cmd!}");
        return OAL_FAIL;
    }

    *map_idx = idx;
    return OAL_SUCC;
}

/*
 * 函 数 名   : wal_wlan_cfg_get_process_entry
 * 功能描述   : 获取 指定cfg_id 在 g_ast_wlan_cfg_process_info_table 中的入口函数
 * 1.日    期   : 2018年6月20日
 *   作    者   :     wifi
 *   修改内容   : 新生成函数
 */
OAL_STATIC wal_wlan_cfg_process_info_table_stru *wal_get_wlan_cfg_info(
    wal_wlan_cfg_process_info_table_stru *process_info_tbl, uint32_t tbl_size,
    wlan_cfgid_enum_uint16 cfg_id)
{
    uint32_t index;
    wal_wlan_cfg_process_info_table_stru *current_info = NULL;

    for (index = 0; index < tbl_size; index++) {
        current_info = process_info_tbl + index;
        if (current_info->en_wlan_cfg_id == cfg_id) {
            return current_info;
        }
    }

    return NULL;
}

wal_wlan_cfg_process_info_table_stru g_wlan_cfg_process_info_table[] = {
    /* cfg_id need_host_process   need_w4_host_return   need_dev_process  need_w4_dev_return  process_entry_func */
    wlan_cfg_info_tbl(WLAN_CFGID_ALG_ENTRY, OAL_TRUE, OAL_TRUE, OAL_TRUE, OAL_TRUE, wal_wlan_cfg_alg_process_entry),

    /* OTHER wlan config process_info to register */
};
/*
 * 函 数 名   : wal_wlan_cfg_module_process_entry
 * 功能描述   : wlan配置模块 入口函数
 * 1.日    期   : 2018年6月20日
 *   作    者   :     wifi
 *   修改内容   : 新生成函数
 */
int32_t wal_wlan_cfg_module_process_entry(wlan_cfg_param_stru *wlan_cfg_param)
{
    wal_wlan_cfg_process_info_table_stru *wlan_cfg_process_info = NULL;
    uint16_t mem_block_len = 0;
    wal_msg_write_stru write_msg;
    uint8_t msg_type = WAL_MSG_TYPE_BUTT;
    int32_t ret;

    wlan_cfg_process_info = wal_get_wlan_cfg_info(g_wlan_cfg_process_info_table,
        oal_array_size(g_wlan_cfg_process_info_table), wlan_cfg_param->en_wlan_cfg_id);
    if (oal_any_null_ptr2(wlan_cfg_process_info, wlan_cfg_process_info->p_func)) {
        oam_error_log0(0, OAM_SF_CFG, "{wal_wlan_cfg_module_process_entry:wal_get_wlan_cfg_info NULL PARAM!.}\r\n");
        return OAL_ERR_CODE_CFG_PROCESS_ENTRY_NOT_FOUND;
    }

    /* process entry function of wlan_cfg_id
       function: 1. analysis args and output;
                 2. if needed process directly, do the process;
                 3. if needed further more process(need host/dev process),
                    fill the msg_type and mem_block needed to transfer to wal
                 4. update wlan_cfg_process_info if the cfg_id has sub_cfg_id
    */
    /* output: real_mem_block(wal_msg_write_stru.auc_value) to transfer */
    memset_s(&write_msg, sizeof(write_msg), 0, sizeof(write_msg));

    ret = wlan_cfg_process_info->p_func(wlan_cfg_param, &msg_type, write_msg.auc_value, &mem_block_len,
        &wlan_cfg_process_info->st_wlan_cfg_process_info);
    if (ret != OAL_SUCC) {
        return ret;
    }
    /* 抛事件到wal，进行host侧处理 */
    wal_write_msg_hdr_init(&write_msg, wlan_cfg_param->en_wlan_cfg_id, mem_block_len);

    ret = wal_send_cfg_event(wlan_cfg_param->pst_net_dev, WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + mem_block_len, (uint8_t *)&write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        return (uint32_t)ret;
    }

    return ret;
}

/*
 * 函 数 名  : wal_hipriv_alg_cfg
 * 功能描述  : 算法模块参数配置
 *       配置算法的配置命令: hipriv "vap0 alg_cfg vi_sch_limit 10"
 *       该命令针对某一个VAP
 * 1.日    期  : 2013年10月25日
  *   修改内容  : 新生成函数
 */
uint32_t wal_hipriv_alg_cfg(oal_net_device_stru *net_dev, int8_t *pc_param)
{
    wal_msg_write_stru write_msg = {0};
    uint32_t off_set = 0;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint32_t result;
    mac_ioctl_alg_param_stru *pst_alg_param = (mac_ioctl_alg_param_stru *)(write_msg.auc_value);

    uint8_t uc_map_index = 0;
    uint8_t hex_base = 10;
    uint8_t *cmd_value = NULL;
    uint32_t ret;
    wal_msg_stru *rsp_msg = NULL;

    /* 获取配置参数名称 */
    result = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_alg_cfg::get cmd one arg err[%d]!}", result);
        return result;
    }

    ret = wal_hipriv_alg_cfg_get_cmd_idx(ac_name, &uc_map_index);
    if (ret != OAL_SUCC) {
        return ret;
    }

    /* 记录命令对应的枚举值 */
    pst_alg_param->en_alg_cfg = g_ast_alg_cfg_map[uc_map_index].en_alg_cfg;

    /* 获取参数配置值 */
    result = wal_get_cmd_one_arg(pc_param + off_set, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_alg_cfg::get cmd one arg err[%d]!}", result);
        return result;
    }

    /* 记录参数配置值 */
    cmd_value = ac_name;
    if (basic_value_is_hex(ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN)) {
        hex_base = 16; // 16代表传进制参数为16
        cmd_value += BYTE_OFFSET_2;
    }
    pst_alg_param->value = (uint32_t)oal_strtol(cmd_value, NULL, hex_base);

    /* 抛事件到wal层处理 */
    wal_write_msg_hdr_init(&write_msg, WLAN_CFGID_ALG_PARAM, sizeof(mac_ioctl_alg_param_stru));

    ret = (uint32_t)wal_send_cfg_event(net_dev, WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_ioctl_alg_param_stru), (uint8_t *)&write_msg, OAL_TRUE, &rsp_msg);
    if (ret != OAL_SUCC) {
        return ret;
    }

    /* 读取返回的错误码 */
    result = wal_check_and_release_msg_resp(rsp_msg);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_DFR, "{wal_hipriv_alg_cfg fail, err code[%u]!}", result);
        return result;
    }

    return OAL_SUCC;
}
