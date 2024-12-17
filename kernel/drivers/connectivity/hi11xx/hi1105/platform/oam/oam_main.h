/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description:oam_main.c header file
 * Author: @CompanyNameTag
 */

#ifndef OAM_MAIN_H
#define OAM_MAIN_H

#include "oal_types.h"
#include "oal_ext_if.h"
#include "oam_ext_if.h"

#define OAM_BEACON_HDR_LEN 24
#define OAM_TIMER_MAX_LEN  36
#define OAM_PRINT_CRLF_NUM 20 /* 输出换行符的个数 */

#define OAM_FEATURE_NAME_ABBR_LEN 12 /* 特性名称缩写最大长度 */

/* 打印方式定义 */
typedef uint32_t (*oal_print_func)(const char *pc_string, oam_mng_ctx_stru *ctx);

/* 变参数据格式定义 */
typedef int8_t *oam_va_list;

/* STRUCT定义 */
/* 日志特性列表结构体定义 */
typedef struct {
    uint8_t en_feature_id; /* 取值范围 oam_feature_enum */
    uint8_t auc_feature_name_abbr[OAM_FEATURE_NAME_ABBR_LEN]; /* 以0结束 */
} oam_software_feature_stru;

extern oam_software_feature_stru g_oam_feature_list[OAM_SOFTWARE_FEATURE_BUTT];

extern int32_t oam_main_init(void);
extern void oam_main_exit(void);
extern uint32_t oam_print_to_sdt(const char *pc_string, oam_mng_ctx_stru *ctx);
extern uint32_t oam_print_to_console(const char *pc_string, oam_mng_ctx_stru *ctx);
extern uint32_t oam_upload_log_to_sdt_ctx(int8_t *pc_string, oam_mng_ctx_stru *ctx);
extern void oam_dump_buff_by_hex(uint8_t *puc_buff, int32_t l_len, int32_t l_num);

#endif /* end of oam_main */
