/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description:Configuration file-related operation interfaces
 * Author: @CompanyNameTag
 */

/* 头文件包含 */
#include "oam_config.h"

/* 全局变量定义 */
OAL_STATIC oam_customize_stru g_oam_customize;

/* 函数实现 */
/* en_moduleid取值范围为oam_msg_moduleid_enum */
void oam_register_init_hook(uint8_t en_moduleid, p_oam_customize_init_func p_func)
{
    g_oam_customize.customize_init[en_moduleid] = p_func;
}

/*
 * 函 数 名  : oam_cfg_get_one_item
 * 功能描述  : 保存一个配置项的值到OAM内部结构中
 * 输入参数  : pc_cfg_data_buf:从配置文件中获取的内容，保存到此buf中
 *             pc_section     :配置项所属于的section
 *             pc_key         :配置项在配置文件中对应的字符串
 *             pl_val         :配置项的值
 */
int32_t oam_cfg_get_one_item(int8_t *pc_cfg_data_buf,
                             int8_t *pc_section,
                             int8_t *pc_key,
                             int32_t *pl_val)
{
    int8_t *pc_section_addr = NULL;
    int8_t *pc_key_addr = NULL;
    int8_t *pc_val_addr = NULL;
    int8_t *pc_equal_sign_addr = NULL; /* 等号的地址 */
    int8_t *pc_tmp = NULL;
    uint8_t uc_key_len;
    int8_t ac_val[OAM_CFG_VAL_MAX_LEN] = {0}; /* 暂存配置项的值 */
    uint8_t uc_index = 0;

    if (oal_unlikely(oal_any_null_ptr4(pc_cfg_data_buf, pc_section, pc_key, pl_val))) {
        oal_io_print("oam_cfg_get_one_item::input null pointer!\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 查找section在配置文件中的位置 */
    pc_section_addr = oal_strstr(pc_cfg_data_buf, pc_section);
    if (pc_section_addr == NULL) {
        oal_io_print("oam_cfg_get_one_item::section not found!\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 从section所在位置开始查找配置项对应的字符串 */
    pc_key_addr = oal_strstr(pc_section_addr, pc_key);
    if (pc_key_addr == NULL) {
        oal_io_print("oam_cfg_get_one_item::key not found!\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 查找配置项的值 */
    uc_key_len = (uint8_t)OAL_STRLEN(pc_key);

    /*
     * 检查key后面是否紧接着'=',如果不是的话，那当前要查找的key有可能是另一个
     * key的前缀，需要继续往后查找
     */
    pc_equal_sign_addr = pc_key_addr + uc_key_len;
    while (*(pc_equal_sign_addr) != '=') {
        pc_key_addr = oal_strstr(pc_equal_sign_addr, pc_key);
        if (pc_key_addr == NULL) {
            oal_io_print("oam_cfg_get_one_item::key not found!\n");
            return OAL_ERR_CODE_PTR_NULL;
        }

        pc_equal_sign_addr = pc_key_addr + uc_key_len;
    }

    /* 检查val是否存在 */
    pc_val_addr = pc_equal_sign_addr + OAM_CFG_EQUAL_SIGN_LEN;
    if ((*(pc_val_addr) == '\n') || (*(pc_val_addr) == '\0')) {
        return OAL_FAIL;
    }

    for (pc_tmp = pc_val_addr; (*pc_tmp != '\n') && (*pc_tmp != '\0'); pc_tmp++) {
        ac_val[uc_index] = *pc_tmp;
        uc_index++;
    }

    *pl_val = oal_atoi(ac_val);

    return OAL_SUCC;
}

/*
 * 函 数 名  : oam_cfg_read_file_to_buf
 * 功能描述  : 从配置文件中读取所有数据，保存到一个buf中
 * 输入参数  : pc_cfg_data_buf:从文件读出数据后存放的buf
 *             ul_file_size   :文件大小(字节数)
 */
int32_t oam_cfg_read_file_to_buf(int8_t *pc_cfg_data_buf, uint32_t ul_file_size)
{
    oal_file_stru *p_file;
    int32_t l_ret;

    p_file = oal_file_open_readonly(OAL_CFG_FILE_PATH);
    if (p_file == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    l_ret = oal_file_read(p_file, pc_cfg_data_buf, ul_file_size);
    if (l_ret <= 0) {
        oal_file_close(p_file);
        return OAL_FAIL;
    }

    oal_file_close(p_file);

    return OAL_SUCC;
}


oal_module_symbol(oam_register_init_hook);
oal_module_symbol(oam_cfg_get_one_item);
oal_module_symbol(oam_cfg_read_file_to_buf);

