/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description: firmware function module
 * Author: @CompanyNameTag
 */

/* 头文件包含 */
#include "oal_net.h"
#include "oal_hcc_bus.h"
#include "plat_debug.h"
#include "plat_firmware_util.h"

/*
 * 函 数 名  : delete_space
 * 功能描述  : 删除字符串两边多余的空格
 * 输入参数  : string: 原始字符串
 *            len   : 字符串的长度
 * 返 回 值  : 错误返回NULL，否则返回删除两边空格以后字符串的首地址
 */
uint8_t *delete_space(uint8_t *string, int32_t *len)
{
    int i;

    if ((string == NULL) || (len == NULL)) {
        return NULL;
    }

    /* 删除尾部的空格 */
    for (i = *len - 1; i >= 0; i--) {
        if (string[i] != COMPART_KEYWORD) {
            break;
        }
        string[i] = '\0';
    }
    /* 出错 */
    if (i < 0) {
        ps_print_err(" string is Space bar\n");
        return NULL;
    }
    /* 在for语句中减去1，这里加上1 */
    *len = i + 1;

    /* 删除头部的空格 */
    for (i = 0; i < *len; i++) {
        if (string[i] != COMPART_KEYWORD) {
            /* 减去空格的个数 */
            *len = *len - i;
            return &string[i];
        }
    }

    return NULL;
}

/*
 * 函 数 名  : string_to_num
 * 功能描述  : 将字符串转换成正整数
 * 输入参数  : string:输入的字符串
 * 输出参数  : number:字符串转换以后的正整数
 * 返 回 值  : 0表示成功，-1表示失败
 */
int32_t string_to_num(const char *string, int32_t *number)
{
    int32_t i;
    int32_t l_num;

    if (string == NULL) {
        ps_print_err("string is NULL!\n");
        return -EFAIL;
    }

    l_num = 0;
    for (i = 0; (string[i] >= '0') && (string[i] <= '9'); i++) {
        l_num = (l_num * 10) + (string[i] - '0'); /* 字符串转整数逻辑需要*10 */
    }

    *number = l_num;

    return SUCC;
}

/*
 * 函 数 名  : num_to_string
 * 功能描述  : 将正整数转换成字符串
 * 输入参数  : number:输入的正整数
 * 输出参数  : number:正整数转换以后的字符串
 * 返 回 值  : 0表示成功，-1表示失败
 */
int32_t num_to_string(uint8_t *string, uint32_t number)
{
    int32_t i = 0;
    int32_t j = 0;
    uint32_t tmp[INT32_STR_LEN];
    uint32_t num = number;

    if (string == NULL) {
        ps_print_err("string is NULL!\n");
        return -EFAIL;
    }

    do {
        tmp[i] = num % 10; /* 将正整数转字符串逻辑需要%10 */
        num = num / 10; /* 将正整数转字符串逻辑需要/10 */
        i++;
    } while (num != 0);

    do {
        string[j] = tmp[i - 1 - j] + '0';
        j++;
    } while (j != i);

    string[j] = '\0';

    return SUCC;
}

int32_t file_open_get_len(char *path, os_kernel_file **fp, uint32_t *file_len)
{
#define RETRY_CONT 3
    mm_segment_t fs;
    int32_t i;

    fs = get_fs();
    set_fs(KERNEL_DS);
    for (i = 0; i < RETRY_CONT; i++) {
        *fp = filp_open(path, O_RDONLY, 0);
        if (oal_is_err_or_null(*fp)) {
            ps_print_err("filp_open [%s] fail!!, fp=%pK, errno:%ld\n", path, *fp, PTR_ERR(*fp));
            *fp = NULL;
            continue;
        }
    }

    if (*fp == NULL) {
        ps_print_err("filp_open fail!!\n");
        set_fs(fs);
        return -OAL_ENOPERMI;
    }

    /* 获取file文件大小 */
    *file_len = vfs_llseek(*fp, 0, SEEK_END);
    if (*file_len <= 0) {
        ps_print_err("file size of %s is 0!!\n", path);
        filp_close(*fp, NULL);
        set_fs(fs);
        return -EFAIL;
    }

    /* 恢复fp->f_pos到文件开头 */
    vfs_llseek(*fp, 0, SEEK_SET);
    set_fs(fs);

    return SUCC;
}

/*
 * 函 数 名  : send_msg
 * 功能描述  : host往device发送消息
 * 输入参数  : data: 发送buffer
 *             len : 发送数据的长度
 * 返 回 值  : -1表示失败，否则返回实际发送的长度
 */
int32_t send_msg(uint8_t *data, int32_t len)
{
    int32_t l_ret;
    hcc_bus *pst_bus = hcc_get_current_mpxx_bus();

    if (unlikely((pst_bus == NULL))) {
        ps_print_err("pst_bus is NULL\n ");
        return -EFAIL;
    }

    ps_print_dbg("len = %d\n", len);
#ifdef HW_DEBUG
    const uint32_t ul_max_print_len = 128;
    print_hex_dump_bytes("send_msg :", DUMP_PREFIX_ADDRESS, data,
                         (len < ul_max_print_len ? len : ul_max_print_len));
#endif
    l_ret = hcc_bus_patch_write(pst_bus, data, len);

    return l_ret;
}

/*
 * 函 数 名  : read_msg
 * 功能描述  : host接收device发来的消息
 * 输入参数  : data: 接收消息的buffer
 *             data_len : 接收buffer的长度
 * 返 回 值  : -1表示失败，否则返回实际接收的长度
 */
int32_t read_msg(uint8_t *data, int32_t data_len)
{
    int32_t len;
    hcc_bus *pst_bus = hcc_get_current_mpxx_bus();

    if (unlikely((data == NULL))) {
        ps_print_err("data is NULL\n ");
        return -EFAIL;
    }

    if (unlikely((pst_bus == NULL))) {
        ps_print_err("pst_bus is NULL\n ");
        return -EFAIL;
    }

    len = hcc_bus_patch_read(pst_bus, data, data_len, READ_MEG_TIMEOUT);
    ps_print_dbg("Receive len=[%d] \n", len);

    return len;
}

#define READ_DEVICE_MAX_BUF_SIZE 128
/* read device reg by bootloader */
int32_t read_device_reg16(uint32_t address, uint16_t *value)
{
    int32_t ret, buf_len;
    const uint32_t ul_read_msg_len = 4;
    const uint32_t ul_dump_len = 8;
    uint8_t buf_tx[READ_DEVICE_MAX_BUF_SIZE];
    uint8_t buf_result[READ_DEVICE_MAX_BUF_SIZE];
    void *addr = (void *)buf_result;

    memset_s(buf_tx, READ_DEVICE_MAX_BUF_SIZE, 0, READ_DEVICE_MAX_BUF_SIZE);
    memset_s(buf_result, READ_DEVICE_MAX_BUF_SIZE, 0, READ_DEVICE_MAX_BUF_SIZE);

    buf_len = snprintf_s(buf_tx, sizeof(buf_tx), sizeof(buf_tx) - 1, "%s%c0x%x%c%d%c",
                         RMEM_CMD_KEYWORD,
                         COMPART_KEYWORD,
                         address,
                         COMPART_KEYWORD,
                         4, /* 4表示长度 */
                         COMPART_KEYWORD); /* 组成 READM 0x... 4 这样的命令，4表示长度 */
    if (buf_len < 0) {
        ps_print_err("log str format err line[%d]\n", __LINE__);
        return buf_len;
    }

    ps_print_info("%s", buf_tx);

    ret = send_msg(buf_tx, buf_len);
    if (ret < 0) {
        ps_print_err("send msg [%s] failed, ret=%d", buf_tx, ret);
        return ret;
    }

    ret = read_msg(buf_result, ul_read_msg_len);
    if (ret > 0) {
         /* 解析回读的内存,都是小端直接转换 */
        *value = (uint16_t)oal_readl((uintptr_t)addr);
        oal_print_hex_dump(buf_result, ul_dump_len, HEX_DUMP_GROUP_SIZE, "reg16: ");
        return 0;
    }

    ps_print_err("read_device_reg16 failed, ret=%d", ret);

    return -1;
}

/* write device regs by bootloader */
int32_t write_device_reg16(uint32_t address, uint16_t value)
{
    int32_t ret, buf_len;
    uint8_t buf_tx[READ_DEVICE_MAX_BUF_SIZE];

    memset_s(buf_tx, sizeof(buf_tx), 0, sizeof(buf_tx));

    buf_len = snprintf_s(buf_tx, sizeof(buf_tx), sizeof(buf_tx) - 1, "%s%c2%c0x%x%c%u%c",
                         WMEM_CMD_KEYWORD,
                         COMPART_KEYWORD,
                         COMPART_KEYWORD,
                         address,
                         COMPART_KEYWORD,
                         value,
                         COMPART_KEYWORD);
    if (buf_len < 0) {
        ps_print_err("log str format err line[%d]\n", __LINE__);
        return buf_len;
    }

    ps_print_info("%s", buf_tx);

    ret = send_msg(buf_tx, buf_len);
    if (ret < 0) {
        ps_print_err("send msg [%s] failed, ret=%d", buf_tx, ret);
        return ret;
    }

    ret = recv_expect_result(MSG_FROM_DEV_WRITEM_OK);
    if (ret < 0) {
        ps_print_err("send msg [%s] recv failed, ret=%d", buf_tx, ret);
        return ret;
    }

    return 0;
}

/*
 * 函 数 名  : recv_expect_result
 * 功能描述  : 接收host期望device正确返回的内容
 * 输入参数  : expect: 期望device正确返回的内容
 * 返 回 值  : 0表示成功，-1表示失败
 */
int32_t recv_expect_result(const uint8_t *expect)
{
    uint8_t buf[RECV_BUF_LEN];
    int32_t len;
    int32_t i;

    if (!os_str_len(expect)) {
        ps_print_dbg("not wait device to respond!\n");
        return SUCC;
    }

    memset_s(buf, RECV_BUF_LEN, 0, RECV_BUF_LEN);
    for (i = 0; i < HOST_DEV_TIMEOUT; i++) {
        len = read_msg(buf, RECV_BUF_LEN);
        if (len < 0) {
            ps_print_err("recv result fail\n");
            continue;
        }

        if (!os_mem_cmp(buf, expect, os_str_len(expect))) {
            ps_print_dbg(" send SUCC, expect [%s] ok\n", expect);
            return SUCC;
        } else {
            ps_print_warning(" error result[%s], expect [%s], read result again\n", buf, expect);
        }
    }

    return -EFAIL;
}

STATIC int32_t read_msg_timeout(uint8_t *data, int32_t data_len, uint32_t timeout)
{
    int32_t len;
    hcc_bus *pst_bus = hcc_get_current_mpxx_bus();

    if (unlikely((data == NULL))) {
        ps_print_err("data is NULL\n ");
        return -EFAIL;
    }

    if (unlikely((pst_bus == NULL))) {
        ps_print_err("pst_bus is NULL\n ");
        return -EFAIL;
    }

    len = hcc_bus_patch_read(pst_bus, data, data_len, timeout);
    ps_print_dbg("Receive len=[%d], data = [%s]\n", len, data);

    return len;
}


int32_t recv_expect_result_timeout(const uint8_t *expect, uint32_t timeout)
{
    uint8_t buf[RECV_BUF_LEN];
    int32_t len;

    if (!os_str_len(expect)) {
        ps_print_dbg("not wait device to respond!\n");
        return SUCC;
    }

    memset_s(buf, RECV_BUF_LEN, 0, RECV_BUF_LEN);
    len = read_msg_timeout(buf, RECV_BUF_LEN, timeout);
    if (len < 0) {
        ps_print_err("recv result fail\n");
        return -EFAIL;
    }

    if (!os_mem_cmp(buf, expect, os_str_len(expect))) {
        ps_print_dbg(" send SUCC, expect [%s] ok\n", expect);
        return SUCC;
    } else {
        ps_print_warning(" error result[%s], expect [%s], read result again\n", buf, expect);
    }

    return -EFAIL;
}

/*
 * 函 数 名  : msg_send_and_recv_except
 * 功能描述  : host向device发送消息并等待device返回消息
 * 输入参数  : data  : 发送buffer
 *             len   : 发送内容的长度
 *             expect: 期望device回复的内容
 * 返 回 值  : -1表示失败，0表示成功
 */
int32_t msg_send_and_recv_except(uint8_t *data, int32_t len, const uint8_t *expect)
{
    int32_t i;
    int32_t ret;

    for (i = 0; i < HOST_DEV_TIMEOUT; i++) {
        ret = send_msg(data, len);
        if (ret < 0) {
            continue;
        }

        ret = recv_expect_result(expect);
        if (ret == 0) {
            return SUCC;
        }
    }

    return -EFAIL;
}
