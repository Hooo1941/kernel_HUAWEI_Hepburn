/*
 * 版权所有 (c) 华为技术有限公司 2015-2018
 * 功能说明   : wifi定制化头文件声明
 * 作者       : wifi
 * 创建日期   : 2015年10月08日
 */
/* 宏定义 */
#ifndef FE_CUSTOMIZE_INI_PARSE_H
#define FE_CUSTOMIZE_INI_PARSE_H
#include "fe_pub_def.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#define HOST_DEFAULT_CHIP_ID     0
#define FE_CUS_PARAMS_LEN_MAX    150  /* 定制项最大长度 */
#define FE_CUS_PARAMS_LEN_MIN    2    /* 定制项最小长度 */
#define FE_MAX_READ_LINE_NUM  192

/* 该编号与CUS_TAG_HOST含义一致，且紧接着该定义之后 */
#define CUS_TAG_FE_POW_CTRL          0x26
#define CUS_TAG_FE_POW               0x27
#define CUS_TAG_FE_ALL               0x28
#define CUS_TAG_FE_MAGIC_NUM         0x29


/* 前端定制化模块划分 */
#define INI_MODU_FE_POW_CTRL_2G       "[CUS_FE_POW_CTRL_2G]"
#define INI_MODU_FE_POW_CTRL_5G       "[CUS_FE_POW_CTRL_5G]"
#define INI_MODU_FE_POW_CTRL_6G       "[CUS_FE_POW_CTRL_6G]"
#define INI_MODU_FE_POW_COMM_2G       "[CUS_FE_POW_COMM_2G]"
#define INI_MODU_FE_POW_COMM_5G       "[CUS_FE_POW_COMM_5G]"
#define INI_MODU_FE_POW_COMM_6G       "[CUS_FE_POW_COMM_6G]"

#define INI_MODU_FE_POW_2G            "[CUS_FE_POW_2G]"
#define INI_MODU_FE_POW_5G            "[CUS_FE_POW_5G]"
#define INI_MODU_FE_POW_6G            "[CUS_FE_POW_6G]"
#define INI_MODU_FE_2G_PART1          "[CUS_FE_2G_PART1]"
#define INI_MODU_FE_5G_PART1          "[CUS_FE_5G_PART1]"
#define INI_MODU_FE_6G_PART1          "[CUS_FE_6G_PART1]"

#define INI_MODU_FE_2G_PART2          "[CUS_FE_2G_PART2]"
#define INI_MODU_FE_5G_PART2          "[CUS_FE_5G_PART2]"
#define INI_MODU_FE_6G_PART2          "[CUS_FE_6G_PART2]"
#define INI_MODU_FE_MAGIC_NUM         "[FE_INI_MAGIC_NUM]"

typedef enum {
    INI_MODU_FE_PART1,
    INI_MODU_FE_POW,
    INI_MODU_FE_POW_CTRL,
    INI_MODU_FE_POW_REGION,
    INI_MODU_FE_PART2,

    INI_MODU_FE_BUTT,
}ini_mode_fe_type_enum;

typedef struct {
    uint32_t mem_offset[FE_CALI_BAND_BUTT][INI_MODU_FE_BUTT];
    uint32_t mem_size[FE_CALI_BAND_BUTT][INI_MODU_FE_BUTT];
    uint32_t state[FE_CALI_BAND_BUTT][INI_MODU_FE_BUTT];
}fe_custom_text_info_stru;

typedef enum {
    FE_REGDOMAIN_FCC = 0,
    FE_REGDOMAIN_ETSI = 1,
    FE_REGDOMAIN_JAPAN = 2,
    FE_REGDOMAIN_COMMON = 3,

    FE_REGDOMAIN_BUTT
} fe_regdomain_enum;

extern fe_custom_text_info_stru g_fe_cust_text_info;
uint32_t fe_custom_host_read_cfg_ini(fe_regdomain_enum regdomain_type);
int32_t fe_config_update_region_pow(fe_regdomain_enum regdomain_type);
void fe_custom_get_section_info(uint8_t band, uint32_t *mmesize, uint32_t *offset, uint32_t *state);
int32_t fe_config_update_pow(void);
#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#endif // fe_customize_ini_parse.h
