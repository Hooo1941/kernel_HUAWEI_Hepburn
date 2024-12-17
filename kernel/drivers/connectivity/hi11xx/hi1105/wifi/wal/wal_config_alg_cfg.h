/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明   : wal_config_alg_cfg.c 的头文件
 * 作者       :  wifi
 * 创建日期   : 2018年6月26日
 */

#ifndef WAL_CONFIG_ALG_CFG_H
#define WAL_CONFIG_ALG_CFG_H

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "wal_linux_ioctl.h"
#include "alg_cfg.h"

/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define alg_cfg_pkt_type_tbl(_pkt_type_string, _pkt_type)      {_pkt_type_string, _pkt_type}

#define alg_cfg_info_tbl(_cfg_str, _cfg_id, _cfg_type, _need_host_process, _need_w4_host_return, \
                         _need_device_process, _need_w4_device_return) \
                        {(_cfg_str), (_cfg_id), (_cfg_type), {0}, \
                        {(_need_host_process), (_need_w4_host_return), \
                         (_need_device_process), (_need_w4_device_return)}}

#define alg_cfg_param_size_tbl(_en_param_type, _size)  {(_en_param_type), (_size), {0, 0}}

#define ALG_CFG_PARAM_MAX_TRG_UL_LEN           4095
#define ALG_CFG_PARAM_MAX_TRG_LDPC_EXTRA_SYM   1
#define ALG_CFG_PARAM_MAX_TRG_PACKET_EXT       7
#define ALG_CFG_PARAM_MAX_TRG_AP_TX_PWR        60
#define ALG_CFG_PARAM_MAX_TRG_MMSF             3
#define ALG_CFG_PARAM_MAX_TRG_TID_AGGR_LIMIT   7
#define ALG_CFG_PARAM_MAX_TRG_PREFER_AC        3
#define ALG_CFG_PARAM_MAX_TRG_PPDU_TIME_LEN_US 5400
#define ALG_CFG_PARAM_MAX_TRG_OFDMA_TXTIME     10000

#define ALG_CFG_PARAM_NUM_2                         2
#define ALG_CFG_PARAM_TXMODE_MAX_CHAIN_BITMAP  0x0f

typedef struct {
    oal_bool_enum_uint8 en_need_host_process;     /* 是否需要host侧跨线程处理 */
    oal_bool_enum_uint8 en_need_w4_host_return;   /* 是否需要返回host侧跨线程处理结果 */
    oal_bool_enum_uint8 en_need_device_process;   /* 是否需要device侧处理 */
    oal_bool_enum_uint8 en_need_w4_device_return; /* 是否需要返回device侧处理结果 */
} wlan_cfg_process_info_stru;
/*
功能      : wlan_cfg 各cfg_id处理入口函数, 进行非跨线程直接处理，或将待处理部分格式化为特定结构体内存
输入参数  : wlan_cfg_param_stru        *pst_wlan_cfg_param
输出参数  : uint8_t    *pen_msg_type
            uint8_t                  *puc_trans_by_event
            uint16_t                 *pus_trans_len
            wlan_cfg_process_info_stru *pst_process_info
            对于仅1级配置，该入参无需改变，对于类似算法的2级配置，该信息需作为出参进行相应修改
返 回 值  : int32_t

 */
typedef int32_t (*wal_wlan_cfg_process_entry_func)(wlan_cfg_param_stru *pst_wlan_cfg_param, uint8_t *pen_msg_type,
    uint8_t *puc_mem_block, uint16_t *pus_mem_len, wlan_cfg_process_info_stru *pst_process_info);
typedef struct {
    wlan_cfgid_enum_uint16          en_wlan_cfg_id;           /* wlan_cfg_id */
    uint8_t                       auc_resv[2];                /* 2代表保留字节数，结构体四字节对齐 */
    wlan_cfg_process_info_stru      st_wlan_cfg_process_info; /* process info */
    /* process entry function of wlan_cfg_id, analysis args and fill the mem_block needed to transfer to wal */
    wal_wlan_cfg_process_entry_func p_func;
} wal_wlan_cfg_process_info_table_stru;

/* 算法配置命令解析公共入参 */
typedef struct {
    int32_t len;
    void *point;
} alg_cfg_default_eqment_param_stru;

typedef struct {
    mac_vap_stru                       *pst_mac_vap;

    /* 入参字符指针数组，按序指向每个参数字符串首地址 */
    uint8_t                          *apuc_param_args[WLAN_CFG_MAX_ARGS_NUM];
    /* 出参字符指针, 用于在解析错误或非跨线程处理中回填自定义字符串 */
    uint8_t                          *puc_output_str;

    uint8_t                           uc_param_num;                           /* 入参个数 */
    uint8_t                           auc_resv[3];    // reserve 3
} alg_cfg_param_stru;

typedef struct {
    char                           *pc_alg_cfg_name;             /* 算法指令字符串 */

    mac_alg_cfg_enum_uint16             en_alg_cfg_id;                /* 算法指令id */
    alg_cfg_type_enum_uint8             en_cfg_type;                  /* SET or GET */
    uint8_t                           auc_resv[1];

    wlan_cfg_process_info_stru          st_process_info;              /* process info */
} alg_cfg_process_info_stru;

/* alg_cfg 字符串-数值 转换表 */
typedef struct {
    char                           *pc_name;        /* 配置命令字符串 */
    uint32_t                          value;        /* 配置命令字符串对应的值 */
} alg_cfg_param_tbl_stru;


typedef uint32_t (*alg_cfg_sub_alg_args_analysis_func)(alg_cfg_param_stru *alg_cfg_param,
    uint8_t *mem_block, uint16_t *len);

typedef struct {
    mac_alg_cfg_enum_uint16             en_alg_cfg_id;
    uint8_t                           auc_resv[2];  // reserve 2
    alg_cfg_sub_alg_args_analysis_func  p_sub_alg_args_analysis_func;
} wlan_alg_cfg_sub_alg_args_analysis_tbl_stru;

typedef enum {
    ALG_CFG_PARAM_PKT_TYPE,
    ALG_CFG_PARAM_RATE_MODE,
    ALG_CFG_PARAM_FEC_CODING,
    ALG_CFG_PARAM_LEGACY_RATE,
    ALG_CFG_PARAM_PROTOCOL_MODE,
    ALG_CFG_PARAM_BOOL,
    ALG_CFG_PARAM_CHAIN,
    ALG_CFG_PARAM_PROTECT_MODE,
    ALG_CFG_PARAM_HE_LTF,
    ALG_CFG_PARAM_PREAMBLE,
    ALG_CFG_PARAM_RTS_BW_MODE,
    ALG_CFG_PARAM_FREQ_BW_MODE,
    ALG_CFG_PARAM_BANDWIDTH_MODE,
    ALG_CFG_PARAM_TX_MODE,
    ALG_CFG_PARAM_SND_MODE,
    ALG_CFG_PARAM_SND_FBACK_TYPE,
    ALG_CFG_PARAM_SND_SCH_MODE,
    ALG_CFG_PARAM_SND_SW_TYPE,
    ALG_CFG_PARAM_SND_USER_MAT_TYPE,
    ALG_CFG_PARAM_FIX_POW_MODE,
    ALG_CFG_PARAM_TPC_POW_MODE,
    ALG_CFG_PARAM_GI_TYPE_NONAX,
    ALG_CFG_PARAM_GI_TYPE_AX,
    ALG_CFG_PARAM_AR_DEV_PARAM,
    ALG_CFG_PARAM_ULSCH_DEV_SCH_MODE,
    ALG_CFG_PARAM_SCEN_USER_INFO_TYPE,
    ALG_CFG_PARAM_AGGR_MODE,
    ALG_CFG_PARAM_AGGR_TIME_OPT,
    ALG_CFG_PARAM_BUTT,
} alg_cfg_param_tbl_size_enum;
typedef uint8_t alg_cfg_param_tbl_size_enum_uint8;

typedef struct {
    alg_cfg_param_tbl_size_enum_uint8   en_param_type;
    uint8_t                           uc_tbl_size;
    uint8_t                           auc_resv[2];    // 空余填充2字节
} alg_cfg_param_tbl_size_info_stru;

/*****************************************************************************
  8 UNION定义
*****************************************************************************/
/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/
/*****************************************************************************
  4 全局变量声明
*****************************************************************************/
extern alg_cfg_process_info_stru g_alg_cfg_process_info_table[];
extern alg_cfg_param_tbl_stru g_alg_cfg_pkt_type_info_table[];

/* 算法参数 字符串-数值转换表 声明 */
/* -------------------TXMODE START--------------------- */
extern alg_cfg_param_tbl_stru g_alg_cfg_tx_mode_tbl[ALG_CFG_TX_MODE_BUTT];
/* -------------------TXMODE END--------------------- */
/*****************************************************************************
  10 函数声明
*****************************************************************************/
static inline alg_cfg_process_info_stru *alg_cfg_get_process_tbl_array(void)
{
    return &g_alg_cfg_process_info_table[0];
}

static inline alg_cfg_param_tbl_stru *alg_cfg_get_pkt_type_tbl_array(void)
{
    return &g_alg_cfg_pkt_type_info_table[0];
}
uint16_t alg_cfg_process_tbl_size(void);

extern uint8_t alg_cfg_param_get_tbl_size(alg_cfg_param_tbl_size_enum_uint8 en_param_type);
extern uint32_t  alg_cfg_get_cmd_one_arg(const char *pc_cmd, char *pc_arg,
                                         uint32_t arg_size, uint32_t *pul_cmd_offset);
int32_t wal_wlan_cfg_module_process_entry(wlan_cfg_param_stru *wlan_cfg_param);
extern alg_cfg_process_info_stru *alg_cfg_search_process_info_by_cfg_name(const char *pc_alg_cfg_name);
extern alg_cfg_process_info_stru *alg_cfg_search_process_info_by_cfg_id(mac_alg_cfg_enum_uint16 en_alg_cfg_id,
                                                                        alg_cfg_type_enum_uint8 en_cfg_type);
extern alg_cfg_pkt_type_enum_uint8 alg_cfg_get_pkt_type(const char *pc_args);
extern uint32_t alg_cfg_analysis_args_head(char *apc_args[WLAN_CFG_MAX_ARGS_NUM],
    alg_cfg_process_info_stru **ppst_alg_cfg_info, uint8_t *puc_offset, alg_cfg_pkt_type_enum_uint8 *pen_pkt_type);
extern uint32_t alg_cfg_analysis_args(char *apc_args[WLAN_CFG_MAX_ARGS_NUM], uint8_t uc_args_num,
                                      alg_cfg_param_stru *pst_alg_cfg_param, alg_cfg_hdr_stru *pst_alg_cfg_hdr,
                                      wlan_cfg_process_info_stru *pst_process_info);
extern uint32_t alg_cfg_get_match_value(const alg_cfg_param_tbl_stru *pst_tbl, uint16_t us_tbl_size,
                                        const uint8_t *pc_str, uint32_t *pul_out_value);
extern int32_t wal_wlan_cfg_alg_process_entry(wlan_cfg_param_stru        *pst_wlan_cfg_param,
    uint8_t *pen_msg_type, uint8_t *puc_trans_by_event, uint16_t *pus_trans_len,
    wlan_cfg_process_info_stru *pst_process_info);
extern wlan_alg_cfg_sub_alg_args_analysis_tbl_stru *wal_get_wlan_alg_cfg_analysis_entry(
    wlan_alg_cfg_sub_alg_args_analysis_tbl_stru *pst_process_info_tbl,
    uint32_t ul_tbl_size, mac_alg_cfg_enum_uint16 en_cfg_id);

/* txmode Start */
extern uint32_t alg_cfg_args_analysis_txmode_user_fix_mode(alg_cfg_param_stru *pst_alg_cfg_param,
                                                           uint8_t *puc_mem_block, uint16_t *pus_len);
extern uint32_t alg_cfg_args_analysis_txmode_user_fix_chain(alg_cfg_param_stru *alg_cfg_param,
                                                            uint8_t *puc_mem_block, uint16_t *pus_len);
extern uint32_t alg_cfg_args_analysis_txmode_debug_log_switch(alg_cfg_param_stru *alg_cfg_param,
                                                              uint8_t *puc_mem_block, uint16_t *pus_len);
extern uint32_t alg_cfg_args_analysis_txmode_mode_sw(alg_cfg_param_stru *pst_alg_cfg_param,
                                                     uint8_t *puc_mem_block, uint16_t *pus_len);
extern uint32_t alg_cfg_args_analysis_txmode_chain_prob_sw(alg_cfg_param_stru *alg_cfg_param,
    uint8_t *pmem_block, uint16_t *len);
extern uint32_t alg_cfg_args_analysis_txmode_all_user_fix_mode(alg_cfg_param_stru *pst_alg_cfg_param,
                                                               uint8_t *puc_mem_block, uint16_t *pus_len);
extern uint32_t alg_cfg_args_analysis_txmode_mgmt_tx_chain(alg_cfg_param_stru *alg_cfg_param,
                                                           uint8_t *mem_block, uint16_t *len);
extern uint32_t alg_cfg_args_analysis_txmode_11b_tx_sw(alg_cfg_param_stru *alg_cfg_param,
                                                       uint8_t *mem_block, uint16_t *len);
extern uint32_t alg_cfg_args_analysis_txmode_bcast_data_tx_chain(alg_cfg_param_stru *alg_cfg_param,
                                                                 uint8_t *mem_block, uint16_t *len);

/* txmode End */
/* waterfilling Start */
extern uint32_t alg_cfg_args_analysis_waterfilling_mode(alg_cfg_param_stru *alg_cfg_param,
                                                        uint8_t *mem_block, uint16_t *len);
extern uint32_t alg_cfg_args_analysis_waterfilling_enable(alg_cfg_param_stru *alg_cfg_param,
                                                          uint8_t *mem_block, uint16_t *len);
extern uint32_t alg_cfg_args_analysis_hardamard_mode(alg_cfg_param_stru *alg_cfg_param,
                                                     uint8_t *mem_block, uint16_t *len);
extern uint32_t alg_cfg_args_analysis_hardamard_enable(alg_cfg_param_stru *alg_cfg_param,
                                                       uint8_t *mem_block, uint16_t *len);
extern uint32_t alg_cfg_args_analysis_waterfilling_limit(alg_cfg_param_stru *alg_cfg_param,
    uint8_t *mem_block, uint16_t *len);
extern uint32_t alg_cfg_args_analysis_waterfilling_bw_mode(alg_cfg_param_stru *alg_cfg_param, uint8_t *mem_block,
    uint16_t *len);
extern uint32_t alg_cfg_args_analysis_waterfilling_sub_asnr(alg_cfg_param_stru *alg_cfg_param, uint8_t *mem_block,
    uint16_t *len);
extern uint32_t alg_cfg_args_analysis_waterfilling_asnr(alg_cfg_param_stru *alg_cfg_param,
                                                        uint8_t *mem_block, uint16_t *len);
extern uint32_t alg_cfg_args_analysis_waterfilling_debug(alg_cfg_param_stru *alg_cfg_param,
                                                         uint8_t *mem_block, uint16_t *len);
extern uint32_t alg_cfg_args_analysis_wf_force_flag(alg_cfg_param_stru *alg_cfg_param,
    uint8_t *mem_block, uint16_t *len);
extern uint32_t alg_cfg_args_analysis_hdtx_force_flag(alg_cfg_param_stru *alg_cfg_param,
    uint8_t *mem_block, uint16_t *len);
/* waterfilling End */
/* 子算法配置命令解析接口   -------- END --------- */
#endif /* end of wal_config_alg_cfg.h */
