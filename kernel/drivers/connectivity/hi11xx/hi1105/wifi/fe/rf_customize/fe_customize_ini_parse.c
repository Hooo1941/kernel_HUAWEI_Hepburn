/*
 * 版权所有 (c) 华为技术有限公司 2015-2018
 * 功能说明   : wifi定制化函数实现
 * 作者       : wifi
 * 创建日期   : 2020年5月20日
 */
/* 头文件包含 */
#include "fe_depend_linux_os_if.h"
#include "fe_customize_main.h"
#include "fe_customize_ini_parse.h"

fe_custom_text_info_stru g_fe_cust_text_info;  // 分段偏移及大小
static char *cfg_tag[][INI_MODU_FE_BUTT] = {
    {INI_MODU_FE_2G_PART1, INI_MODU_FE_POW_2G, INI_MODU_FE_POW_CTRL_2G,
     INI_MODU_FE_POW_COMM_2G, INI_MODU_FE_2G_PART2}, \
    {INI_MODU_FE_5G_PART1, INI_MODU_FE_POW_5G, INI_MODU_FE_POW_CTRL_5G,
     INI_MODU_FE_POW_COMM_5G, INI_MODU_FE_5G_PART2}, \
    {INI_MODU_FE_6G_PART1, INI_MODU_FE_POW_6G, INI_MODU_FE_POW_CTRL_6G,
     INI_MODU_FE_POW_COMM_6G, INI_MODU_FE_6G_PART2}, \
};
static char *pow_tag_strs[][FE_REGDOMAIN_BUTT] = {
    {"[CUS_FE_POW_FCC_2G]", "[CUS_FE_POW_ETSI_2G]", "[CUS_FE_POW_JP_2G]", "[CUS_FE_POW_COMM_2G]"},
    {"[CUS_FE_POW_FCC_5G]", "[CUS_FE_POW_ETSI_5G]", "[CUS_FE_POW_JP_5G]", "[CUS_FE_POW_COMM_5G]"},
    {"[CUS_FE_POW_FCC_6G]", "[CUS_FE_POW_ETSI_6G]", "[CUS_FE_POW_JP_6G]", "[CUS_FE_POW_COMM_6G]"},
};

static int32_t ini_check_str(const char *buff, uint32_t buff_length, const char *str, uint32_t str_length)
{
    /* 内部接口保证buff不为空，只校验待查找的str */
    if (unlikely(str == NULL || str_length == 0)) {
        ini_error("ini_check_str: str error!"NEWLINE);
        return INI_FAILED;
    }

    /* 跳过无效行 */
    if (unlikely((buff[0] == '#') || (buff[0] == ' ') || (buff[0] == '\n') || (buff[0] == '\r'))) {
        return INI_FAILED;
    }

    /* 一行的字节数必须大于等于待查找变量长度 */
    if (unlikely(buff_length < str_length)) {
        return INI_FAILED;
    }

    if (strncmp(buff, str, str_length) == 0) {
        return INI_SUCC;
    } else {
        return INI_FAILED;
    }
}
static int32_t fe_ini_find_module_name(oal_file_stru *fp, const char *mod_name)
{
    char tmp_buf[FE_MAX_READ_LINE_NUM];
    int32_t l_ret;

    if (unlikely(fp == NULL || mod_name == NULL)) {
        ini_error("fe_ini_find_module_name: para ptr is NULL!"NEWLINE);
        return INI_FAILED;
    }

    /* 每次需要从头开始遍历，重要 */
    fp->f_pos = 0;

    while (1) {
        l_ret = ko_read_line(fp, tmp_buf, sizeof(tmp_buf));
        if (unlikely(l_ret == INI_FAILED)) {
            ini_warning("fe_ini_find_module_name: file read fail!"NEWLINE);
            return INI_FAILED;
        }
        l_ret = ini_check_str(tmp_buf, strlen(tmp_buf), mod_name, strlen(mod_name));
        if (l_ret == INI_SUCC) {
            return INI_SUCC;
        } else {
            continue;
        }
    }
}
static int32_t get_valid_config_line(oal_file_stru *fp, char *auc_tmp, int32_t rd_buf_length)
{
    int32_t ret;
    ret = ko_read_line(fp, auc_tmp, rd_buf_length);
    if (ret == INI_FAILED) {
        ini_error("not exist or contain wrong characters or end of file: %s\r\n", auc_tmp);
        return INI_FAILED;
    }

    if (auc_tmp[0] == '[') {
        return INI_FAILED;
    }

    if ((auc_tmp[0] == '#') || (auc_tmp[0] == ' ') || (auc_tmp[0] == ';') ||
        (auc_tmp[0] == '\n') || (auc_tmp[0] == '\r') || (auc_tmp[0] == '\t')) {
        return INI_CONTINUE;
    }

    return INI_SUCC;
}

static uint8_t *calc_pad_process(uint8_t data_size, uint8_t *mem_addr, uint32_t *mem_size)
{
    static uint8_t  data_size_old = 0;
    uint8_t  pad_size = 0;

    if (data_size != data_size_old) {
        pad_size = (4 - *mem_size % 4) % 4; // 4 字节对齐
    }

    *mem_size = *mem_size + pad_size;
    data_size_old = data_size;
    return (mem_addr + pad_size);
}
static uint8_t *custom_main_init_calc_pad_process(uint8_t *addr, uint32_t *mem_size)
{
    const uint8_t data_len = 4;   // 数据类型长度 4字节 用于不同结构间做4字节对齐判断

    addr = addr + *mem_size;
    return calc_pad_process(data_len, addr, mem_size);
}
OAL_STATIC uint32_t fe_ini_get_cust_item_list(const char *cust_param, const int32_t *param_list,
    uint16_t *list_len, uint16_t max_len)
{
    char *pc_token = NULL;
    char *pc_ctx = NULL;
    const char *pc_end = ";";
    const char *pc_sep = ",";
    uint16_t param_num = 0;
    char cust_param_str[FE_CUS_PARAMS_LEN_MAX] = {0};
    int32_t ret;

    if (memcpy_s(cust_param_str, FE_CUS_PARAMS_LEN_MAX, cust_param, OAL_STRLEN(cust_param)) != EOK) {
        return OAL_FAIL;
    }
    pc_token = strtok_s(cust_param_str, pc_end, &pc_ctx);
    if (pc_token == NULL) {
        ini_error("fe_ini_get_cust_item_list read get null value check!\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }
    pc_token = strtok_s(pc_token, pc_sep, &pc_ctx);
    /* 获取定制化系数 */
    while (pc_token != NULL) {
        if (param_num == max_len) {
            ini_error("fe_ini_get_cust_item_list::nv or ini param is too many idx[%d] Max[%d]\r\n",
                param_num, max_len);
            return OAL_FAIL;
        }

        /* 将字符串转换成数字 */
        if (!strncmp(pc_token, "0x", strlen("0x")) || !strncmp(pc_token, "0X", strlen("0X"))) {
            ret = sscanf_s(pc_token, "%x", &param_list[param_num]);
        } else {
            ret = sscanf_s(pc_token, "%d", &param_list[param_num]);
        }

        if (ret <= 0) {
            ini_error("fe_ini_get_cust_item_list::sscanf_s fail\r\n");
            return OAL_FAIL;
        }
        pc_token = strtok_s(NULL, pc_sep, &pc_ctx);
        param_num++;
    }

    *list_len = param_num;

    return OAL_SUCC;
}
static void find_var_in_ini(oal_file_stru *fp, uint8_t *mem_addr, uint32_t *mem_size)
{
    int32_t  ret;
    char     auc_tmp[FE_MAX_READ_LINE_NUM] = {0};
    const char    *delim = "=";
    char    *sub_str1 = NULL;
    char    *sub_str2 = NULL;
    int32_t  param_list[FE_CUS_PARAMS_LEN_MAX] = {0};
    uint16_t param_num, param_idx;
    uint8_t  data_size;
    while (1) {
        ret = get_valid_config_line(fp, auc_tmp, sizeof(auc_tmp));
        if (ret == INI_FAILED) {
            break;
        } else if (ret == INI_CONTINUE) {
            continue;
        }
        sub_str1 = strtok_s(auc_tmp, delim, &sub_str2);
        if (fe_ini_get_cust_item_list(sub_str2, param_list, &param_num, FE_CUS_PARAMS_LEN_MAX) != OAL_SUCC ||
            param_num < FE_CUS_PARAMS_LEN_MIN) {
            ini_error("customize param num [%d] is wrong\r\n", param_num);
            break;
        }

        data_size = (uint8_t)param_list[param_num - 1];

        mem_addr = calc_pad_process(data_size, mem_addr, mem_size);

        for (param_idx = 0; param_idx < param_num - 1; param_idx++) {
            if (memcpy_s(mem_addr, data_size, &param_list[param_idx], data_size) != EOK) {
                ini_error("find_var_in_ini:: get val memcpy_s fail!\r\n");
                break;
            }
            mem_addr = mem_addr + data_size;
            *mem_size = *mem_size + data_size;
        }
    }
}
static int32_t fe_ini_find_var_value_by_path(const char *path, const char *tag, uint8_t *mem_addr, uint32_t *mem_size)
{
    oal_file_stru *fp = NULL;

#ifdef INI_TIME_TEST
    oal_timespec_stru tv0;
    oal_timespec_stru tv1;
#endif

    int32_t l_ret;

    if (mem_addr == NULL) {
        ini_error("check if mem_addr is NULL");
        return INI_FAILED;
    }
    if ((tag == NULL) || (tag[0] != '[') || (tag[strlen(tag) - 1] != ']')) {
        ini_error("tag invalid");
        return INI_FAILED;
    }

#ifdef INI_TIME_TEST
    oal_do_gettimeofday(&tv0);
#endif

    fp = oal_file_open_readonly(path);
    if (fp == NULL) {
        ini_error("open %s failed!!!", path);
        return INI_FAILED;
    }

    l_ret = fe_ini_find_module_name(fp, tag);
    if (l_ret == INI_FAILED) {
        ini_error("ini_find_modu %s failed!!!\r\n", path);
        oal_file_close(fp);
        return INI_FAILED;
    }

    /* find var in .ini return value */
    find_var_in_ini(fp, mem_addr, mem_size);

#ifdef INI_TIME_TEST
    oal_do_gettimeofday(&tv1);
    /* 1000 为单位转换 */
    ini_debug("time take = %llu", (tv1.tv_sec - tv0.tv_sec) * 1000 + (oal_get_usec(&tv1) - oal_get_usec(&tv0)) / 1000);
#endif

    oal_file_close(fp);

    return INI_SUCC;
}
/*
 *  Prototype    : fe_read_custom_ini_var
 *  Description  : get var value from .ini file
 */

int32_t fe_read_custom_ini_var(const char *tag, uint8_t *mem_addr, uint32_t *mem_size)
{
    /* read spec if exist */
    char *path = NULL;
    path = get_host_ini_conn_path(HOST_DEFAULT_CHIP_ID);
    if (ini_file_exist(path)) {
        if (fe_ini_find_var_value_by_path(path, tag, mem_addr, mem_size) == INI_SUCC) {
            return INI_SUCC;
        }
    }
    path = get_host_ini_path(HOST_DEFAULT_CHIP_ID);
    if (ini_file_exist(path) == 0) {
        return INI_FAILED;
    }

    return fe_ini_find_var_value_by_path(path, tag, mem_addr, mem_size);
}
int32_t fe_config_update_pow_ctrl(void)
{
    uint8_t  *addr;
    uint32_t  mem_size;
    int32_t   ret;
    uint8_t   band;

    for (band = FE_CALI_BAND_2G; band < FE_CALI_BAND_BUTT; band++) {
        if (g_fe_cust_text_info.state[band][INI_MODU_FE_POW_CTRL] != OAL_TRUE) {
            return INI_FAILED;
        }
        addr = fe_get_customize_mem_addr() + g_fe_cust_text_info.mem_offset[band][INI_MODU_FE_POW_CTRL];
        mem_size = 0;
        ret = fe_read_custom_ini_var(cfg_tag[band][INI_MODU_FE_POW_CTRL], addr, &mem_size);
        if (mem_size != g_fe_cust_text_info.mem_size[band][INI_MODU_FE_POW_CTRL] || (ret != INI_SUCC)) {
            ini_error("pow ctrl mem_size invalid record:%d != read %d!!!", \
                g_fe_cust_text_info.mem_size[band][INI_MODU_FE_POW_CTRL], mem_size);
            return INI_FAILED;
        }
    }

    return INI_SUCC;
}
int32_t fe_config_update_region_pow(fe_regdomain_enum regdomain_type)
{
    uint8_t  *addr;
    uint32_t  mem_size;
    int32_t   ret;
    uint8_t   band;

    for (band = FE_CALI_BAND_2G; band < FE_CALI_BAND_BUTT; band++) {
        if (g_fe_cust_text_info.state[band][INI_MODU_FE_POW_REGION] != OAL_TRUE) {
            return INI_FAILED;
        }
        addr = fe_get_customize_mem_addr() + g_fe_cust_text_info.mem_offset[band][INI_MODU_FE_POW_REGION];
        mem_size = 0;
        ret = fe_read_custom_ini_var(pow_tag_strs[band][regdomain_type], addr, &mem_size);
        if (mem_size != g_fe_cust_text_info.mem_size[band][INI_MODU_FE_POW_REGION] || (ret != INI_SUCC)) {
            ini_error("pow ctrl mem_size invalid record:%d != read %d!!!", \
                g_fe_cust_text_info.mem_size[band][INI_MODU_FE_POW_REGION], mem_size);
            return INI_FAILED;
        }
    }

    return INI_SUCC;
}
void fe_custom_get_section_info(uint8_t band, uint32_t *mmesize, uint32_t *offset, uint32_t *state)
{
    /* 这里将来idx传入，需要外部看到枚举 */
    *mmesize = g_fe_cust_text_info.mem_size[band][INI_MODU_FE_POW_REGION];
    *offset = g_fe_cust_text_info.mem_offset[band][INI_MODU_FE_POW_REGION];
    *state = g_fe_cust_text_info.state[band][INI_MODU_FE_POW_REGION];
}

int32_t fe_config_update_pow(void)
{
    uint8_t  *addr;
    uint32_t  mem_size;
    int32_t   ret;
    uint8_t   band;

    for (band = FE_CALI_BAND_2G; band < FE_CALI_BAND_BUTT; band++) {
        if (g_fe_cust_text_info.state[band][INI_MODU_FE_POW] != OAL_TRUE) {
            return INI_FAILED;
        }
        addr = fe_get_customize_mem_addr() + g_fe_cust_text_info.mem_offset[band][INI_MODU_FE_POW];
        mem_size = 0;
        ret = fe_read_custom_ini_var(cfg_tag[band][INI_MODU_FE_POW], addr, &mem_size);
        if (mem_size != g_fe_cust_text_info.mem_size[band][INI_MODU_FE_POW] || (ret != INI_SUCC)) {
            ini_error("pow mem_size invalid record:%d != read %d!!!",
                g_fe_cust_text_info.mem_size[band][INI_MODU_FE_POW], mem_size);
            return INI_FAILED;
        }
    }
    return INI_SUCC;
}
/* 读定制化配置文件&NVRAM */
/* 按照内存进行读取，和具体名字不相关 */
uint32_t fe_custom_host_read_cfg_ini(fe_regdomain_enum regdomain_type)
{
    uint8_t  *addr = fe_get_customize_mem_addr();
    uint8_t  *addr_start = addr;
    uint32_t  mem_size;
    uint32_t  custom_len = fe_get_customize_mem_size();
    int32_t   ret;
    uint8_t   band, tag_idx;

    memset_s(addr, sizeof(uint8_t) * custom_len, 0, sizeof(uint8_t) * custom_len);
    memset_s(&g_fe_cust_text_info, sizeof(fe_custom_text_info_stru), 0, sizeof(fe_custom_text_info_stru));
    mem_size = 0;
    for (band = FE_CALI_BAND_2G; band < FE_CALI_BAND_BUTT; band++) {
        for (tag_idx = 0; tag_idx < INI_MODU_FE_BUTT; tag_idx++) {
            g_fe_cust_text_info.mem_offset[band][tag_idx] = addr - addr_start;
            if (tag_idx == INI_MODU_FE_POW_REGION) {
                ret = fe_read_custom_ini_var(cfg_tag[band][tag_idx], addr, &mem_size);
            } else {
                ret = fe_read_custom_ini_var(pow_tag_strs[band][regdomain_type], addr, &mem_size);
            }
            if (ret != INI_SUCC) {
                return ret;
            }
            g_fe_cust_text_info.mem_size[band][tag_idx] = mem_size;
            g_fe_cust_text_info.state[band][tag_idx] = OAL_TRUE;
            addr = custom_main_init_calc_pad_process(addr, &mem_size);
            mem_size = 0;
        }
    }

    mem_size = 0;
    ret = fe_read_custom_ini_var(INI_MODU_FE_MAGIC_NUM, addr, &mem_size);
    return ret;
}
int32_t fe_update_custom_section_by_tag(int32_t cus_tag)
{
    int32_t   ret;

    switch (cus_tag) {
        case CUS_TAG_FE_POW: // 只更功率限制相关参数
            ret = fe_config_update_pow();
            break;
        case CUS_TAG_FE_POW_CTRL: // 只更新产线相关参数
            ret = fe_config_update_pow_ctrl();
            break;
        case CUS_TAG_FE_ALL:
            ret = fe_custom_host_read_cfg_ini(FE_REGDOMAIN_COMMON);
            break;
        default:
            ini_error("fe_update_custom_section_by_tag tag number[0x%x] not correct!", cus_tag);
            return INI_FAILED;
    }
    /* 定制化检查放在device侧做 */
    return ret;
}
