/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description:oam_log.c header file
 * Author: @CompanyNameTag
 */

#ifndef OAM_LOG_H
#define OAM_LOG_H

/* 其他头文件包含 */
#include "oal_ext_if.h"
#include "oal_util.h"

/* 宏定义 */
#define OAM_LOG_PARAM_MAX_NUM     4   /* 可打印最多的参数个数 */
#define OAM_LOG_PRINT_DATA_LENGTH 512 /* 每次写入文件的最大长度 */

#define OAM_LOG_VAP_INDEX_INTERVAL 2 /* 驱动的索引和产品上层适配层的索引间隔 */
#define REG_NUM 13  /* 通用寄存器个数 */

/* STRUCT定义 */
typedef struct {
    uint32_t bit16_file_id : 16;
    uint32_t bit8_feature_id : 8;
    uint32_t bit4_vap_id : 4;
    uint32_t bit4_log_level : 4;
} om_log_wifi_para_stru;

/* STRUCT定义 */
struct om_msg_header {
    uint8_t        frame_start;
    uint8_t        func_type;
    uint8_t        prime_id_t;
    uint8_t        reserver[1]; // sdt tool alreay used for sysinfo
    uint16_t       frame_len;
    uint16_t       sn_t;
} DECLARE_PACKED;
typedef struct om_msg_header om_msg_header_stru;

/* 临终遗言信息 */
struct om_exception_info {
    om_msg_header_stru msg_header;
    uint32_t stack_limit;
    uint32_t fault_type;
    uint32_t fault_reason;
    uint32_t address_t;
    uint32_t reg_value[REG_NUM];
    uint32_t msp_value;
    uint32_t lr_value;
    uint32_t pc_value;
    uint32_t psps_value;
    uint32_t pri_mask_value;
    uint32_t fault_mask_value;
    uint32_t bser_pri_value;
    uint32_t control_value;
    uint8_t msg_tail;
} DECLARE_PACKED;
typedef struct om_exception_info om_exception_info_stru;

uint32_t oam_log_init(void);
void oam_log_exit(void);

#endif /* end of oam_log.h */
