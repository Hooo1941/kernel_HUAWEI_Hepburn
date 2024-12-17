/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description:OAM LOG
 * Author: @CompanyNameTag
 */

/* 头文件包含 */
#include "oam_main.h"
#include "oam_log.h"
#include "securec.h"
#ifndef _PRE_LINUX_TEST
#include "board.h"
#endif
#ifdef CONFIG_PRINTK
#include <linux/kernel.h>
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_OAM_LOG_C

#define get_vap_id(comb_para) (uint8_t)(((comb_para) >> 24) & 0xf)
#define get_log_lev(comb_para) (uint8_t)(((comb_para) >> 28) & 0xf)
#define get_feature_id(comb_para) (uint8_t)(((comb_para) >> 16) & 0xff)
#define get_file_no(comb_para) (uint16_t)((comb_para) & 0xffff)

/* 全局变量定义 */
#ifdef CONFIG_PRINTK
OAL_STATIC char *g_loglevel_string[OAM_LOG_LEVEL_BUTT];
#endif

#ifdef _PRE_DFT_IO_PRINTK
/*
 * 函 数 名  : oal_print2kernel
 * 功能描述  : 把打印信息保存一份到内核中
 */
OAL_STATIC int32_t oal_print2kernel(uint32_t ul_para, uint16_t us_line_no, int8_t *fmt,
                                    oal_uint p1, oal_uint p2, oal_uint p3, oal_uint p4)
{
#ifdef CONFIG_PRINTK
    int32_t l_ret;
    int8_t pc_buf[OAM_LOG_PRINT_DATA_LENGTH];

    /* 将压缩参数解析成单个参数 */
    uint8_t uc_vap_id = get_vap_id(ul_para);
    uint16_t us_file_no = get_file_no(ul_para);
    uint8_t clog_level = get_log_lev(ul_para); // oam_log_level_enum

    pc_buf[0] = '\0';
    if (clog_level == OAM_LOG_LEVEL_ERROR) {
        declare_dft_trace_key_info("oam error log", OAL_DFT_TRACE_OTHER);
    }

    l_ret = snprintf_s(pc_buf, sizeof(pc_buf), sizeof(pc_buf) - 1, "%s[vap:%u]%s [F:%u][L:%u]\n",
                       g_loglevel_string[clog_level],
                       uc_vap_id,
                       fmt,
                       us_file_no,
                       us_line_no);
    if (l_ret < 0) {
        oam_io_printk("l_ret < 0 \r\n");
        return l_ret;
    }

    printk(pc_buf, (int32_t)p1, (int32_t)p2, (int32_t)p3, (int32_t)p4);
#endif

    return OAL_SUCC;
}
#endif

/*
 * 函 数 名  : oam_log_set_global_switch
 * 功能描述  : 设置日志全局开关状态
 */
void oam_log_set_global_switch_ctx(oal_switch_enum_uint8 en_log_switch, oam_mng_ctx_stru *ctx)
{
    ctx->st_log_ctx.en_global_log_switch = en_log_switch;
}

/*
 * 函 数 名  : oam_log_set_vap_level
 * 功能描述  : 设置VAP下所有特性日志级别为en_log_level
 *             不允许设置成OAM_LOG_LEVEL_INFO级别,在命令行里进行限制，本函数不处理
 * 输入参数  : en_log_level->oam_log_level_enum
 */
void oam_log_set_vap_level_ctx(uint8_t uc_vap_id, uint8_t en_log_level, oam_mng_ctx_stru *ctx)
{
    uint8_t en_feature_idx; // oam_feature_enum

    if (oal_unlikely(uc_vap_id >= OAM_VAP_SUPPORT_MAX_NUM_LIMIT)) {
        return;
    }

    /* 设置当前VAP的日志级别 */
    ctx->st_log_ctx.st_vap_log_info.aen_vap_log_level[uc_vap_id] = en_log_level;

    /* 同时设置当前VAP下所有特性日志级别 */
    for (en_feature_idx = 0; en_feature_idx < OAM_SOFTWARE_FEATURE_BUTT; en_feature_idx++) {
        oam_log_set_feature_level_ctx(uc_vap_id, en_feature_idx, en_log_level, ctx);
    }
}
/* en_feature_id->oam_feature_enum，en_log_level->oam_log_level_enum */
uint32_t oam_log_set_feature_level_ctx(uint8_t uc_vap_id,
                                       uint8_t en_feature_id,
                                       uint8_t en_log_level, oam_mng_ctx_stru *ctx)
{
    if (oal_unlikely(uc_vap_id >= OAM_VAP_SUPPORT_MAX_NUM_LIMIT)) {
        oam_io_printk("invalid uc_vap_id.[%d] \r\n", uc_vap_id);
        return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
    }

    if (oal_unlikely(en_feature_id >= OAM_SOFTWARE_FEATURE_BUTT)) {
        oam_io_printk("invalid feature_id[%d],sys id[%d]\r\n", en_feature_id, ctx->subsys_id);
        return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
    }

    if (oal_unlikely((en_log_level >= OAM_LOG_LEVEL_BUTT) || (en_log_level < OAM_LOG_LEVEL_ERROR))) {
        oam_io_printk("invalid en_log_level.[%d] \r\n", en_log_level);
        return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
    }

    ctx->st_log_ctx.st_vap_log_info.aen_feature_log_level[uc_vap_id][en_feature_id] = en_log_level;
    return OAL_SUCC;
}

/*
 * 函 数 名  : oam_get_feature_id
 * 功能描述  : 由特性名称获取特性ID
 * puc_feature_id取值oam_feature_enum
 */
uint32_t oam_get_feature_id(uint8_t *puc_feature_name,
                            uint8_t *puc_feature_id)
{
    uint8_t en_feature_idx;

    if (oal_unlikely((puc_feature_name == NULL) || (puc_feature_id == NULL))) {
        oam_io_printk("null param \r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    for (en_feature_idx = 0; en_feature_idx < OAM_SOFTWARE_FEATURE_BUTT; en_feature_idx++) {
        if (oal_strcmp((int8_t *)g_oam_feature_list[en_feature_idx].auc_feature_name_abbr,
                       (int8_t *)puc_feature_name) == 0) {
            *puc_feature_id = en_feature_idx;
            return OAL_SUCC;
        }
    }

    return OAL_FAIL;
}

/*
 * 函 数 名  : oam_get_feature_name
 * 功能描述  : 获取指定特性ID的特性名称
 * 输入参数  ：en_feature_id->oam_feature_enum
 */
uint32_t oam_get_feature_name(uint8_t en_feature_id,
                              uint8_t *puc_feature_name,
                              uint8_t uc_size)
{
    uint8_t uc_feature_len;
    uint8_t uc_temp_name_len;

    if (oal_unlikely(en_feature_id >= OAM_SOFTWARE_FEATURE_BUTT)) {
        oam_io_printk("en_feature_id override. %d. \r\n", en_feature_id);
        return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
    }

    if (oal_unlikely(puc_feature_name == NULL)) {
        oam_io_printk("puc_feature_name is NULL. \r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    uc_temp_name_len = uc_size;
    uc_feature_len = (uint8_t)OAL_STRLEN((int8_t *)g_oam_feature_list[en_feature_id].auc_feature_name_abbr);
    uc_size = (uc_size > uc_feature_len) ? uc_feature_len : uc_size;

    if (memcpy_s(puc_feature_name, uc_temp_name_len,
                 g_oam_feature_list[en_feature_id].auc_feature_name_abbr, uc_size) != EOK) {
        oam_io_printk("memcpy_s error, destlen=%u, srclen=%u\n ", uc_temp_name_len, uc_size);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}

/*
 * 函 数 名  : oam_show_feature_list
 * 功能描述  : 显示特性缩写与全称的帮助信息
 */
uint32_t oam_show_feature_list(void)
{
    uint8_t en_feature_id; // oam_feature_enum

    oal_io_print("feature_list: \r\n");
    for (en_feature_id = 0; en_feature_id < OAM_SOFTWARE_FEATURE_BUTT; en_feature_id++) {
        oal_io_print("%s\r\n", g_oam_feature_list[en_feature_id].auc_feature_name_abbr);
    }

    return OAL_SUCC;
}

/*
 * 函 数 名  : oam_log_format_string
 * 功能描述  : 根据格式,将日志信息写入到指定的文件中
 * 输入参数  ：en_feature_id->oam_feature_enum
 */
OAL_STATIC uint32_t oam_log_format_string(int8_t *pac_output_data,
                                          uint16_t us_data_len,
                                          uint8_t uc_vap_id,
                                          uint8_t en_feature_id,
                                          uint16_t us_file_id,
                                          uint16_t us_line_num,
                                          uint8_t en_log_level,
                                          int8_t *pc_string,
                                          uint8_t uc_param_cnt,
                                          int32_t l_param1,
                                          int32_t l_param2,
                                          int32_t l_param3,
                                          int32_t l_param4,
                                          oam_mng_ctx_stru *ctx)
{
    int8_t *pac_print_level_tbl[] = { "OFF", "ERROR", "WARNING", "INFO" };
    uint32_t ul_tick;
    uint8_t auc_feature_name[OAM_FEATURE_NAME_ABBR_LEN] = {0};
    int8_t *pac_print_format[] = {
        "[LOG=%s]:Tick=%lu, FileId=%u, LineNo=%u, VAP=%u, FeatureName=%s, \"%s\", \r\n",
        "[LOG=%s]:Tick=%lu, FileId=%u, LineNo=%u, VAP=%u, FeatureName=%s, \"%s\", %d \r\n",
        "[LOG=%s]:Tick=%lu, FileId=%u, LineNo=%u, VAP=%u, FeatureName=%s, \"%s\", %d, %d \r\n",
        "[LOG=%s]:Tick=%lu, FileId=%u, LineNo=%u, VAP=%u, FeatureName=%s, \"%s\", %d, %d, %d \r\n",
        "[LOG=%s]:Tick=%lu, FileId=%u, LineNo=%u, VAP=%u, FeatureName=%s, \"%s\", %d, %d, %d, %d \r\n"
    };
    int32_t ret;

    /* 获取系统TICK值 */
    ul_tick = (uint32_t)oal_time_get_stamp_ms();

    oam_get_feature_name(en_feature_id, auc_feature_name, sizeof(auc_feature_name));

    /* 根据参数个数,将LOG信息保存到ac_file_data中 */
    switch (uc_param_cnt) {
        case 0: /* 参数个数 0 */
            ret = snprintf_s(pac_output_data, us_data_len, us_data_len - 1, pac_print_format[uc_param_cnt],
                             pac_print_level_tbl[en_log_level], ul_tick, us_file_id, us_line_num, uc_vap_id,
                             auc_feature_name, pc_string);
            break;

        case 1: /* 参数个数 1 */
            ret = snprintf_s(pac_output_data, us_data_len, us_data_len - 1, pac_print_format[uc_param_cnt],
                             pac_print_level_tbl[en_log_level], ul_tick, us_file_id, us_line_num, uc_vap_id,
                             auc_feature_name, pc_string, l_param1);
            break;

        case 2: /* 参数个数 2 */
            ret = snprintf_s(pac_output_data, us_data_len, us_data_len - 1, pac_print_format[uc_param_cnt],
                             pac_print_level_tbl[en_log_level], ul_tick, us_file_id, us_line_num, uc_vap_id,
                             auc_feature_name, pc_string, l_param1, l_param2);
            break;

        case 3: /* 参数个数 3 */
            ret = snprintf_s(pac_output_data, us_data_len, us_data_len - 1, pac_print_format[uc_param_cnt],
                             pac_print_level_tbl[en_log_level], ul_tick, us_file_id, us_line_num, uc_vap_id,
                             auc_feature_name, pc_string, l_param1, l_param2, l_param3);
            break;

        case 4: /* 参数个数 4 */
            ret = snprintf_s(pac_output_data, us_data_len, us_data_len - 1, pac_print_format[uc_param_cnt],
                             pac_print_level_tbl[en_log_level], ul_tick, us_file_id, us_line_num, uc_vap_id,
                             auc_feature_name, pc_string, l_param1, l_param2, l_param3, l_param4);
            break;

        default:
            oam_io_printk("invalid uc_param_cnt.[%u] \r\n", uc_param_cnt);
            return OAL_FAIL;
    }

    if (ret < 0) {
        oam_io_printk("log format str err\n");
        return OAL_FAIL;
    }

    return OAL_SUCC;
}

/*
 * 函 数 名  : oam_set_log_info_stru
 * 功能描述  : 填写log结构体信息
 * 输入参数  : pst_log_info : 输出内容
 *             uc_vap_id    : VAP编号
 *             en_feature_id: 特性ID 范围oam_feature_enum
 *             us_file_id   : 文件ID
 *             us_line_num  : 行号
 *             en_log_level : 打印级别 范围oam_log_level_enum
 *             l_param1     : 需要打印的第一个参数
 *             l_param2     : 需要打印的第二个参数
 *             l_param3     : 需要打印的第三个参数
 *             l_param4     : 需要打印的第四个参数
 */
OAL_STATIC void oam_set_log_info_stru(oam_log_info_stru *pst_log_info,
                                      uint8_t uc_vap_id,
                                      uint8_t en_feature_id,
                                      uint16_t us_file_id,
                                      uint16_t us_line_num,
                                      uint8_t en_log_level,
                                      int32_t l_param1,
                                      int32_t l_param2,
                                      int32_t l_param3,
                                      int32_t l_param4)
{
    uint32_t ul_tick;

    /* 获取系统TICK值 */
    ul_tick = (uint32_t)oal_time_get_stamp_ms();

    /* 为日志结构体整数成员赋值 */
    pst_log_info->st_vap_log_level.bit_vap_id = uc_vap_id;
    pst_log_info->st_vap_log_level.bit_log_level = en_log_level;
    pst_log_info->us_file_id = us_file_id;
    pst_log_info->us_line_num = us_line_num;
    pst_log_info->en_feature_id = en_feature_id;
    pst_log_info->ul_tick = ul_tick;
    pst_log_info->al_param[0] = l_param1;
    pst_log_info->al_param[1] = l_param2;
    pst_log_info->al_param[2] = l_param3; /* 数组下标2，保存需要打印的第二个参数 */
    pst_log_info->al_param[3] = l_param4; /* 数组下标3，保存需要打印的第三个参数 */
}

/*
 * 函 数 名  : oam_log_print_to_console
 * 功能描述  : 日志输出至串口终端
 * 输入参数  ：en_feature_id->oam_feature_enum
 *             en_log_level->oam_log_level_enum
 */
OAL_STATIC uint32_t oam_log_print_to_console(uint8_t uc_vap_id,
                                             uint8_t en_feature_id,
                                             uint16_t us_file_id,
                                             uint16_t us_line_num,
                                             uint8_t en_log_level,
                                             int8_t *pc_string,
                                             uint8_t uc_param_cnt,
                                             int32_t l_param1,
                                             int32_t l_param2,
                                             int32_t l_param3,
                                             int32_t l_param4,
                                             oam_mng_ctx_stru *ctx)
{
    int8_t ac_print_buff[OAM_PRINT_FORMAT_LENGTH]; /* 用于保存写入到文件中的格式 */

    oam_log_format_string(ac_print_buff,
                          OAM_PRINT_FORMAT_LENGTH,
                          uc_vap_id,
                          en_feature_id,
                          us_file_id,
                          us_line_num,
                          en_log_level,
                          pc_string,
                          uc_param_cnt,
                          l_param1,
                          l_param2,
                          l_param3,
                          l_param4,
                          ctx);

    oam_print_to_console(ac_print_buff, ctx);

    return OAL_SUCC;
}

/*
 * 函 数 名  : oam_alarm_print_to_sdt
 * 功能描述  : 日志的标准输出函数
 * 输入参数  : uc_vap_id    : VAP编号
 *           en_feature_id: 特性ID 范围oam_feature_enum
 *           us_file_id   : 文件ID
 *           us_line_num  : 行号
 *           en_log_level : 打印级别 范围oam_log_level_enum
 *           pc_string    : 要打印的字符串
 *           l_param1     : 需要打印的第一个参数
 *           l_param2     : 需要打印的第二个参数
 *           l_param3     : 需要打印的第三个参数
 *           l_param4     : 需要打印的第四个参数
 */
OAL_STATIC uint32_t oam_log_print_to_sdt(uint8_t uc_vap_id,
                                         uint8_t en_feature_id,
                                         uint16_t us_file_id,
                                         uint16_t us_line_num,
                                         uint8_t en_log_level,
                                         int8_t *pc_string,
                                         int32_t l_param1,
                                         int32_t l_param2,
                                         int32_t l_param3,
                                         int32_t l_param4,
                                         oam_mng_ctx_stru *ctx)
{
    uint32_t ul_ret;
    oam_log_info_stru st_log_info;

    memset_s(&st_log_info, sizeof(oam_log_info_stru), 0, sizeof(oam_log_info_stru));

    oam_set_log_info_stru(&st_log_info,
                          uc_vap_id,
                          en_feature_id,
                          us_file_id,
                          us_line_num,
                          en_log_level,
                          l_param1,
                          l_param2,
                          l_param3,
                          l_param4);

    ul_ret = oam_upload_log_to_sdt_ctx((int8_t *)&st_log_info, ctx);
    if (ul_ret != OAL_SUCC) {
        return ul_ret;
    }

    return OAL_SUCC;
}

/*
 * 函 数 名  : oam_log_print_n_param
 * 功能描述  : LOG模块提供的LOG打印函数总入口(N个参数)
 */
OAL_STATIC uint32_t oam_log_print_n_param(uint32_t ul_para,
                                          uint16_t us_line_num,
                                          int8_t *pc_string,
                                          uint8_t uc_param_cnt,
                                          int32_t l_param1,
                                          int32_t l_param2,
                                          int32_t l_param3,
                                          int32_t l_param4,
                                          oam_mng_ctx_stru *ctx)
{
    uint32_t ul_ret = OAL_SUCC;
    uint8_t en_output_type; // oam_output_type_enum

    uint8_t uc_vap_id = get_vap_id(ul_para);
    uint16_t us_file_id = get_file_no(ul_para);
    uint8_t en_log_level = get_log_lev(ul_para); // oam_log_level_enum
    uint8_t en_feature_id = get_feature_id(ul_para); // oam_feature_enum

    if (oal_unlikely(pc_string == NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 若输出条件满足，判断输出方向 */
    oam_get_output_type_ctx(&en_output_type, ctx);
    switch (en_output_type) {
        /* 输出至控制台 */
        case OAM_OUTPUT_TYPE_CONSOLE:
            ul_ret = oam_log_print_to_console(uc_vap_id,
                                              en_feature_id,
                                              us_file_id,
                                              us_line_num,
                                              en_log_level,
                                              pc_string,
                                              uc_param_cnt,
                                              l_param1,
                                              l_param2,
                                              l_param3,
                                              l_param4,
                                              ctx);
            break;

        /* 输出至PC侧调测工具平台 */
        case OAM_OUTPUT_TYPE_SDT:
            ul_ret = oam_log_print_to_sdt(uc_vap_id,
                                          en_feature_id,
                                          us_file_id,
                                          us_line_num,
                                          en_log_level,
                                          pc_string,
                                          l_param1,
                                          l_param2,
                                          l_param3,
                                          l_param4,
                                          ctx);

            break;

        /* 无效配置 */
        default:
            ul_ret = OAL_ERR_CODE_INVALID_CONFIG;
            break;
    }

    return ul_ret;
}

/*
 * 函 数 名  : oam_log_print0
 * 功能描述  : 日志打印函数：无参数日志输出
 */
uint32_t oam_log_print0_ctx(uint32_t ul_para,
                            uint16_t us_line_num,
                            int8_t *pc_string,
                            oam_mng_ctx_stru *ctx)
{
    uint32_t ul_ret = OAL_SUCC;
    if (oam_get_log_switch(ul_para, ctx) == OAL_SWITCH_ON) {
#ifdef _PRE_DFT_IO_PRINTK
        oal_print2kernel(ul_para, us_line_num, pc_string, 0, 0, 0, 0);
#endif
        ul_ret = oam_log_print_n_param(ul_para, us_line_num, pc_string, 0, 0, 0, 0, 0, ctx);
    }

    return ul_ret;
}

/*
 * 函 数 名  : oam_log_print1
 * 功能描述  : 日志打印函数：一个参数
 */
uint32_t oam_log_print1_ctx(uint32_t ul_para,
                            uint16_t us_line_num,
                            int8_t *pc_string,
                            oal_uint l_param1,
                            oam_mng_ctx_stru *ctx)
{
    uint32_t ul_ret = OAL_SUCC;
    if (oam_get_log_switch(ul_para, ctx) == OAL_SWITCH_ON) {
#ifdef _PRE_DFT_IO_PRINTK
        oal_print2kernel(ul_para, us_line_num, pc_string, l_param1, 0, 0, 0);
#endif
        ul_ret = oam_log_print_n_param(ul_para, us_line_num,
                                       pc_string, 1, (int32_t)l_param1, 0, 0, 0, ctx);
    }

    return ul_ret;
}

/*
 * 函 数 名  : oam_log_print2
 * 功能描述  : 日志打印函数：两个参数
 */
uint32_t oam_log_print2_ctx(uint32_t ul_para,
                            uint16_t us_line_num,
                            int8_t *pc_string,
                            oal_uint l_param1,
                            oal_uint l_param2,
                            oam_mng_ctx_stru *ctx)
{
    uint32_t ul_ret = OAL_SUCC;
    if (oam_get_log_switch(ul_para, ctx) == OAL_SWITCH_ON) {
#ifdef _PRE_DFT_IO_PRINTK
        oal_print2kernel(ul_para, us_line_num, pc_string, l_param1, l_param2, 0, 0);
#endif
        ul_ret = oam_log_print_n_param(ul_para, us_line_num,
            pc_string, 2, (int32_t)l_param1, (int32_t)l_param2, 0, 0, ctx); /* 参数个数为2 */
    }

    return ul_ret;
}

/*
 * 函 数 名  : oam_log_print3
 * 功能描述  : 日志打印函数：三个参数
 */
uint32_t oam_log_print3_ctx(uint32_t ul_para,
                            uint16_t us_line_num,
                            int8_t *pc_string,
                            oal_uint l_param1,
                            oal_uint l_param2,
                            oal_uint l_param3,
                            oam_mng_ctx_stru *ctx)
{
    uint32_t ul_ret = OAL_SUCC;
    if (oam_get_log_switch(ul_para, ctx) == OAL_SWITCH_ON) {
#ifdef _PRE_DFT_IO_PRINTK
        oal_print2kernel(ul_para, us_line_num, pc_string, l_param1, l_param2, l_param3, 0);
#endif
        ul_ret = oam_log_print_n_param(ul_para, us_line_num, pc_string,
            3, (int32_t)l_param1, (int32_t)l_param2, (int32_t)l_param3, 0, ctx); /* 参数个数为3 */
    }

    return ul_ret;
}

/*
 * 函 数 名  : oam_log_print4
 * 功能描述  : 日志打印函数：四个参数
 */
uint32_t oam_log_print4_ctx(uint32_t ul_para,
                            uint16_t us_line_num,
                            int8_t *pc_string,
                            oal_uint l_param1,
                            oal_uint l_param2,
                            oal_uint l_param3,
                            oal_uint l_param4,
                            oam_mng_ctx_stru *ctx)
{
    uint32_t ul_ret = OAL_SUCC;
    if (oam_get_log_switch(ul_para, ctx) == OAL_SWITCH_ON) {
#ifdef _PRE_DFT_IO_PRINTK
        oal_print2kernel(ul_para, us_line_num, pc_string, l_param1, l_param2, l_param3, l_param4);
#endif
        /* 参数个数为4 */
        ul_ret = oam_log_print_n_param(ul_para, us_line_num, pc_string, 4, (int32_t)l_param1,
                                       (int32_t)l_param2, (int32_t)l_param3, (int32_t)l_param4, ctx);
    }

    return ul_ret;
}

/*
 * 函 数 名  : oam_log_printk
 * 功能描述  : 串口输出日志
 */
OAL_STATIC uint32_t oam_log_printk(uint16_t us_file_no,
                                   uint16_t us_line_num,
                                   const int8_t *pc_func_name,
                                   const char *pc_args_buf)
{
    int8_t ac_output_data[OAM_PRINT_FORMAT_LENGTH]; /* 用于保存写入到文件中的格式 */
    int8_t *pac_printk_format = (int8_t *)"Tick=%lu, FileId=%u, LineNo=%u, FuncName::%s, \"%s\"\r\n";
    uint32_t ul_tick;
    int32_t ret;

    ul_tick = (uint32_t)oal_time_get_stamp_ms();

    ret = snprintf_s(ac_output_data, OAM_PRINT_FORMAT_LENGTH, OAM_PRINT_FORMAT_LENGTH - 1, pac_printk_format,
                     ul_tick, us_file_no, us_line_num, pc_func_name, pc_args_buf);
    if (ret < 0) {
        oal_io_print("log str format err\n");
        return OAL_FAIL;
    }
    oal_io_print("%s\r\n", ac_output_data);

    return OAL_SUCC;
}

/*
 * 函 数 名  : oam_log_console_printk
 * 功能描述  : 输出串口日志函数
 *             防止串口频繁打印，该串口使用了流控
 */
uint32_t oam_log_console_printk(uint16_t us_file_no,
                                uint16_t us_line_num,
                                const int8_t *pc_func_name,
                                const int8_t *pc_fmt, ...)
{
#ifdef _PRE_WINDOWS_SUPPORT
    return OAL_SUCC;
#else
    int8_t ac_args_buf[OAM_PRINT_FORMAT_LENGTH];
    OAL_VA_LIST pc_args;
    int32_t ret;

    if (oal_unlikely(oal_any_null_ptr2(pc_func_name, pc_fmt))) {
        oam_io_printk("null param. \r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    OAL_VA_START(pc_args, pc_fmt);
    ret = vsnprintf_s(ac_args_buf, OAM_PRINT_FORMAT_LENGTH, OAM_PRINT_FORMAT_LENGTH - 1, pc_fmt, pc_args);
    OAL_VA_END(pc_args);

    if (ret < 0) {
        oal_io_print("print str format err\n");
        return OAL_FAIL;
    }
    oam_log_printk(us_file_no, us_line_num, pc_func_name, ac_args_buf);

    return OAL_SUCC;
#endif
}

void oam_log_param_init(void)
{
#ifdef CONFIG_PRINTK
    int32_t i;
    for (i = 0; i < OAM_LOG_LEVEL_BUTT; i++) {
        g_loglevel_string[i] = KERN_INFO "[X][MPXX]";
    }
    g_loglevel_string[OAM_LOG_LEVEL_ERROR] = KERN_ERR "[E][MPXX]";
#ifdef _PRE_PRINT_OAM_LOG_LEVEL_ERROR  /* lowest print level: OAM_LOG_LEVEL_ERROR */
    g_loglevel_string[OAM_LOG_LEVEL_WARNING] = KERN_DEBUG "[W][MPXX]";
#else
    g_loglevel_string[OAM_LOG_LEVEL_WARNING] = KERN_INFO "[W][MPXX]";
#endif
    g_loglevel_string[OAM_LOG_LEVEL_INFO] = KERN_DEBUG "[I][MPXX]";
#endif
}

uint32_t oam_log_init(void)
{
    oam_ota_ctx_stru *wifi_ota_ctx = NULL;
    oam_ota_ctx_stru *gt_ota_ctx = NULL;
    oam_user_track_ctx_stru *wifi_user_track_ctx = NULL;
    oam_user_track_ctx_stru *gt_user_track_ctx = NULL;
    oam_log_param_init();
    /* 平台日志全局开关默认为开 */
    oam_log_set_global_switch_ctx(OAL_SWITCH_ON, &g_oam_mng_ctx[OAM_CTX_PLAT]);
    /* 平台日志VAP缺省为0 */
    oam_log_set_vap_level_ctx(0, OAM_LOG_DEFAULT_LEVEL, &g_oam_mng_ctx[OAM_CTX_PLAT]);

    wifi_ota_ctx = oal_memalloc(sizeof(oam_ota_ctx_stru));
    if (wifi_ota_ctx == NULL) {
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }

    memset_s(wifi_ota_ctx, sizeof(oam_ota_ctx_stru), 0, sizeof(oam_ota_ctx_stru));
    g_oam_mng_ctx[OAM_CTX_WIFI].pst_ota_ctx = wifi_ota_ctx;

    wifi_user_track_ctx = oal_memalloc(sizeof(oam_user_track_ctx_stru));
    if (wifi_user_track_ctx == NULL) {
        goto wifi_user_track_alloc_fail;
    }

    memset_s(wifi_user_track_ctx, sizeof(oam_user_track_ctx_stru), 0, sizeof(oam_user_track_ctx_stru));
    g_oam_mng_ctx[OAM_CTX_WIFI].pst_user_track_ctx = wifi_user_track_ctx;
    if (is_gt_support()) {
        gt_ota_ctx = oal_memalloc(sizeof(oam_ota_ctx_stru));
        if (gt_ota_ctx == NULL) {
            goto gt_ota_alloc_fail;
        }
        memset_s(gt_ota_ctx, sizeof(oam_ota_ctx_stru), 0, sizeof(oam_ota_ctx_stru));
        g_oam_mng_ctx[OAM_CTX_GT].pst_ota_ctx = gt_ota_ctx;

        gt_user_track_ctx = oal_memalloc(sizeof(oam_user_track_ctx_stru));
        if (gt_user_track_ctx == NULL) {
            goto gt_user_track_alloc_fail;
        }
        memset_s(gt_user_track_ctx, sizeof(oam_user_track_ctx_stru), 0, sizeof(oam_user_track_ctx_stru));
        g_oam_mng_ctx[OAM_CTX_GT].pst_user_track_ctx = gt_user_track_ctx;
    }
    return OAL_SUCC;

gt_user_track_alloc_fail:
    oal_free(gt_ota_ctx);
    g_oam_mng_ctx[OAM_CTX_GT].pst_ota_ctx = NULL;
gt_ota_alloc_fail:
    oal_free(wifi_user_track_ctx);
    g_oam_mng_ctx[OAM_CTX_WIFI].pst_user_track_ctx = NULL;
wifi_user_track_alloc_fail:
    oal_free(wifi_ota_ctx);
    g_oam_mng_ctx[OAM_CTX_WIFI].pst_ota_ctx = NULL;
    return OAL_ERR_CODE_ALLOC_MEM_FAIL;
}

void oam_log_exit(void)
{
    if (g_oam_mng_ctx[OAM_CTX_WIFI].pst_ota_ctx != NULL) {
        oal_free(g_oam_mng_ctx[OAM_CTX_WIFI].pst_ota_ctx);
        g_oam_mng_ctx[OAM_CTX_WIFI].pst_ota_ctx = NULL;
    }
    if (g_oam_mng_ctx[OAM_CTX_WIFI].pst_user_track_ctx != NULL) {
        oal_free(g_oam_mng_ctx[OAM_CTX_WIFI].pst_user_track_ctx);
        g_oam_mng_ctx[OAM_CTX_WIFI].pst_user_track_ctx = NULL;
    }
    if (is_gt_support()) {
        if (g_oam_mng_ctx[OAM_CTX_GT].pst_ota_ctx != NULL) {
            oal_free(g_oam_mng_ctx[OAM_CTX_GT].pst_ota_ctx);
            g_oam_mng_ctx[OAM_CTX_GT].pst_ota_ctx = NULL;
        }

        if (g_oam_mng_ctx[OAM_CTX_GT].pst_user_track_ctx != NULL) {
            oal_free(g_oam_mng_ctx[OAM_CTX_GT].pst_user_track_ctx);
            g_oam_mng_ctx[OAM_CTX_GT].pst_user_track_ctx = NULL;
        }
    }
}

oal_module_symbol(oam_log_print0_ctx);
oal_module_symbol(oam_log_set_global_switch_ctx);
oal_module_symbol(oam_log_set_vap_level_ctx);
oal_module_symbol(oam_log_set_feature_level_ctx);
oal_module_symbol(oam_log_console_printk);
oal_module_symbol(oam_get_feature_id);
oal_module_symbol(oam_show_feature_list);
oal_module_symbol(oam_log_print1_ctx);
oal_module_symbol(oam_log_print2_ctx);
oal_module_symbol(oam_log_print3_ctx);
oal_module_symbol(oam_log_print4_ctx);

