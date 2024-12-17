/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description:oam_config.c header file
 * Author: @CompanyNameTag
 */

#ifndef OAM_CONFIG_H
#define OAM_CONFIG_H

/* 其他头文件包含 */
#include "oal_ext_if.h"

/* 宏定义 */
/* 配置项的值在配置文件中占的最大字节数 */
#define OAM_CFG_VAL_MAX_LEN 20

/* 等于号(=)长度 */
#define OAM_CFG_EQUAL_SIGN_LEN 1

/* '\0'长度 */
#define OAM_CFG_STR_END_SIGN_LEN 1

typedef uint32_t (*p_oam_customize_init_func)(void);
/* 枚举定义 */
/* primID pattern */
enum OM_MSG_MODULEID {
    OM_WIFI_HOST = 0x00,
    OM_WIFI = 0x01,
    OM_BT = 0x02,
    OM_GNSS = 0x03,
    OM_FM = 0x04,
    OM_PF = 0x05,
    OM_MODULEID_BUTT
};

/* STRUCT定义 */
typedef struct {
    p_oam_customize_init_func customize_init[OM_MODULEID_BUTT]; /* 所有定制化的初始函数 */
} oam_customize_stru;

/* 函数声明 */
/* en_moduleid取值范围为oam_msg_moduleid_enum */
extern void oam_register_init_hook(uint8_t en_moduleid, p_oam_customize_init_func p_func);
extern int32_t oam_cfg_get_one_item(int8_t *pc_cfg_data_buf,
                                    int8_t *pc_section,
                                    int8_t *pc_key,
                                    int32_t *pl_val);
extern int32_t oam_cfg_read_file_to_buf(int8_t *pc_cfg_data_buf, uint32_t ul_file_size);

#endif /* end of oam_config.h */
