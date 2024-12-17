/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description:hcc debugging file, which is used to issue debugging commands
 * Author: @CompanyNameTag
 */

#ifdef _PRE_CONFIG_HCC_TEST_RW
#define HISI_LOG_TAG "[HCC_TEST]"
#include "oal_hcc_host_if.h"
#include "securec.h"
#ifdef _PRE_CONFIG_CONN_HISI_SYSFS_SUPPORT
#include "oal_kernel_file.h"
#include <linux/kernel.h>

#define ALIGN_HALF_WORD             2
#define ALIGN_WORD                  4

typedef struct _hcc_rw_cmd_data_ {
    uint32_t valid;
    uint8_t *buf;
    uint32_t buf_size;
    uint8_t *offset;
} hcc_rw_cmd_data;

OAL_STATIC int32_t init_cmd_data(hcc_rw_cmd_data* data, uint8_t* buf, uint32_t buf_size)
{
    if (data == NULL) {
        return OAL_FAIL;
    }
    if (buf == NULL || buf_size < sizeof(int32_t)) {
        data->valid = OAL_FALSE;
        return OAL_FAIL;
    }
    data->valid = OAL_TRUE;
    data->buf = buf;
    data->buf_size = buf_size;
    data->offset = buf;
    return OAL_SUCC;
}

OAL_STATIC OAL_INLINE uint32_t cmd_data_is_valid(hcc_rw_cmd_data* data)
{
    if (data == NULL) {
        return OAL_FALSE;
    }
    return data->valid;
}

OAL_STATIC OAL_INLINE int32_t cmd_data_length(hcc_rw_cmd_data* data)
{
    if (!cmd_data_is_valid(data)) {
        return -1;
    }
    return (data->offset - data->buf);
}

OAL_STATIC OAL_INLINE uint32_t cmd_data_is_enough(hcc_rw_cmd_data* data, int32_t len)
{
    uint32_t left_buf;

    left_buf = data->buf_size - (data->offset - data->buf);
    if (left_buf < len) {
        return OAL_FALSE;
    }
    return OAL_TRUE;
}

OAL_STATIC int32_t cmd_data_push_int32(hcc_rw_cmd_data* data, int32_t value)
{
    int32_t len = 0;
    if (!cmd_data_is_valid(data)) {
        return OAL_FAIL;
    }

    if (!cmd_data_is_enough(data, sizeof(int32_t) + sizeof(int32_t))) {
        data->valid = OAL_FALSE;
        return OAL_FAIL;
    }

    /* 数据长度: 当前平台类型转换为小端数据类型 */
    len = cpu_to_le32(sizeof(int32_t));
    *(uint32_t *)(data->offset) = len;
    data->offset += sizeof(uint32_t);
    /* 数据内容: 当前平台类型转换为小端数据类型 */
    value = cpu_to_le32(value);
    *(int32_t *)(data->offset) = value;
    data->offset += sizeof(int32_t);
    return OAL_SUCC;
}

OAL_STATIC int32_t cmd_data_push_data(hcc_rw_cmd_data* data, uint8_t* data_value, uint32_t data_size)
{
    uint32_t align_data_size = 0;

    if (!cmd_data_is_valid(data)) {
        return OAL_FAIL;
    }
    if (data_value == NULL && data_size > 0) {
        return OAL_FAIL;
    }

    align_data_size = ALIGN(data_size, ALIGN_WORD);
    if (!cmd_data_is_enough(data, sizeof(int32_t) + align_data_size)) {
        data->valid = OAL_FALSE;
        return OAL_FAIL;
    }

    /* 数据长度: 当前平台类型转换为小端数据类型 */
    data_size = cpu_to_le32(data_size);
    *(uint32_t *)(data->offset) = data_size;
    data->offset += sizeof(uint32_t);
    /* 数据内容: 数组不需要转换 */
    if (memcpy_s(data->offset, align_data_size, data_value, data_size) != EOK) {
        data->offset -= sizeof(uint32_t);
        return OAL_FAIL;
    }
    data->offset += align_data_size;
    return OAL_SUCC;
}

OAL_STATIC int32_t cmd_data_pop_int32(hcc_rw_cmd_data* data, int32_t* value)
{
    uint32_t value_size = 0;
    if (!cmd_data_is_valid(data)) {
        return OAL_FAIL;
    }
    if (!cmd_data_is_enough(data, sizeof(int32_t) + sizeof(int32_t))) {
        data->valid = OAL_FALSE;
        return OAL_FAIL;
    }

    /* 数据长度: 小端数据类型转换为当前平台类型 */
    value_size = *(uint32_t *)(data->offset);
    value_size = le32_to_cpu(value_size);
    data->offset += sizeof(uint32_t);
    if (value_size != sizeof(uint32_t)) {
        data->valid = OAL_FALSE;
        return OAL_FAIL;
    }
    if (value != NULL) {
        int32_t tmp_value = *(int32_t *)(data->offset);
        /* 数据内容: 小端数据类型转换为当前平台类型 */
        *value = le32_to_cpu(tmp_value);
    }
    data->offset += sizeof(int32_t);
    return OAL_SUCC;
}

OAL_STATIC int32_t cmd_data_pop_data(hcc_rw_cmd_data* data, uint8_t** value, uint32_t* value_size)
{
    uint32_t align_data_size = 0;
    uint8_t *tmp_value = 0;
    uint32_t tmp_value_size = 0;

    if (!cmd_data_is_valid(data)) {
        return OAL_FAIL;
    }
    if (!cmd_data_is_enough(data, sizeof(int32_t))) {
        data->valid = OAL_FALSE;
        return OAL_FAIL;
    }
    /* 数据长度: 小端数据类型转换为当前平台类型 */
    tmp_value_size = *(uint32_t *)(data->offset);
    tmp_value_size = le32_to_cpu(tmp_value_size);
    data->offset += sizeof(uint32_t);
    /* 数据内容, 数组类型需要转换 */
    align_data_size = ALIGN(tmp_value_size, ALIGN_WORD);
    if (!cmd_data_is_enough(data, align_data_size)) {
        data->valid = OAL_FALSE;
        return OAL_FAIL;
    }
    tmp_value = data->offset;
    data->offset += align_data_size;

    /* 对输出参数赋值 */
    if (value != NULL) {
        *value = tmp_value;
    }
    if (value_size != NULL) {
        *value_size = tmp_value_size;
    }
    return OAL_SUCC;
}

#define MAX_WAIT_TIME_MSECOND       5000
#define TEST_DEBUG_COMMON_PARAM_LEN 2
#define TEST_DEBUG_READ_PARAM_LEN   6
#define TEST_DEBUG_WRITE_PARAM_LEN  5
#define TEST_DEBUG_DUMPMEM_WIDTH    16

typedef struct _hcc_rw_cmd_context_ {
    oal_mutex_stru cmd_lock;

    hcc_rw_cmd_data send_data;
    oal_netbuf_stru *send_netbuf;
    hcc_rw_cmd_data recv_data;
    oal_netbuf_stru *recv_netbuf;

    oal_mutex_stru recv_lock;
    int32_t cur_seq;
    oal_completion recv_done;
} hcc_rw_cmd_context;

OAL_STATIC hcc_rw_cmd_context g_hcc_rw_context;

/**
 *  命令工作流程:
 *  1. 函数 hcc_rw_context_init_cmd_data() 初始化context的发送参数send_data, 填写seq
 *  2. 业务处理: 将需要的指令/参数, 填写到context的发送参数中.
 *  3. 函数 hcc_rw_context_send_and_recv() 发送消息, 并等待device返回
 *  4. device返回后, 触发回调函数hcc_test_rw_rcvd_data(), 判断seq合法后, 将参数写入context的接收参数recv_data中
 *  5. 业务处理: 通过函数hcc_rw_context_get_recv(), 获取device返回的值, 并做相应处理
 *  6. 业务处理: 业务处理完成后, 负责释放recv数据
 *  7. 异常处理时, 通过hcc_rw_context_release()释放资源.
 *  注意:
 *  1. 通过锁 cmd_lock 保护, host往device的发送方向, 同时仅支持一个线程操作
 *  2. 通过锁 recv_lock 保护, device往host返回消息时,同时仅支持一个回调操作接收参数.
 */
OAL_STATIC int32_t hcc_rw_context_init_cmd_data(hcc_rw_cmd_context* dbg_ctx, int32_t len)
{
    int32_t ret = 0;
    /* 指令前添加参数seq */
    len += sizeof(int32_t) * TEST_DEBUG_COMMON_PARAM_LEN;
    do {
        dbg_ctx->send_netbuf = hcc_netbuf_alloc(len);
        if (dbg_ctx->send_netbuf == NULL) {
            ret = OAL_ENOMEM;
            oal_io_print("[E] hcc_netbuf_alloc fail. len[%d]" "\n", len);
            break;
        }
        dbg_ctx->cur_seq ++;

        /* 发送netbuf,准备内存, 然后写入seq */
        oal_netbuf_put(dbg_ctx->send_netbuf, len);
        init_cmd_data(&(dbg_ctx->send_data), oal_netbuf_data(dbg_ctx->send_netbuf), len);
        cmd_data_push_int32(&(dbg_ctx->send_data), dbg_ctx->cur_seq);
        if (!cmd_data_is_valid(&(dbg_ctx->send_data))) {
            ret = OAL_ENOMEM;
            oal_io_print("[E] memory enough, never here. len[%d]" "\n", len);
            break;
        }
    } while (0);
    return ret;
}

OAL_STATIC int32_t hcc_rw_context_send_and_recv(hcc_rw_cmd_context* dbg_ctx)
{
    int32_t ret;
    struct hcc_transfer_param st_hcc_transfer_param = { 0 };
    hcc_bus_dev *hcc_bus = hcc_get_get_one_alive_bus_dev();
    struct hcc_handler *hcc = NULL;

    if (hcc_bus != NULL) {
        hcc = hcc_bus->hcc;
    }
    if (hcc == NULL) {
        oal_io_print("[E] hcc_get_handler failed." "\n");
        return -OAL_EFAIL;
    }
    /* 准备等待device返回的资源 */
    oal_reinit_completion(dbg_ctx->recv_done);

    /* 向device发送指令 */
    hcc_hdr_param_init(&st_hcc_transfer_param, HCC_ACTION_TYPE_TEST, HCC_TEST_SUBTYPE_RW, 0, HCC_FC_WAIT, 0);
    if (OAL_SUCC != hcc_tx(hcc, dbg_ctx->send_netbuf, &st_hcc_transfer_param)) {
        oal_io_print("[E] hcc tx failed" "\n");
        return -OAL_EFAIL;
    }
    dbg_ctx->send_netbuf = NULL;

    /* 等待接收完成通知 */
    ret = wait_for_completion_interruptible_timeout(&(dbg_ctx->recv_done), oal_msecs_to_jiffies(MAX_WAIT_TIME_MSECOND));
    if (ret <= 0) {
        oal_io_print("[E] hcc wait recv failed. ret[%d]" "\n", ret);
        return -OAL_EFAIL;
    }
    return OAL_SUCC;
}

OAL_STATIC int32_t hcc_rw_context_get_recv(hcc_rw_cmd_context *dbg_ctx, oal_netbuf_stru **pst_netbuf,
    hcc_rw_cmd_data *cmd_data)
{
    /* 加锁的状态下, 获取接收参数 */
    mutex_lock(&(g_hcc_rw_context.recv_lock));
    *pst_netbuf = g_hcc_rw_context.recv_netbuf;
    g_hcc_rw_context.recv_netbuf = NULL;
    *cmd_data = g_hcc_rw_context.recv_data;
    mutex_unlock(&(g_hcc_rw_context.recv_lock));

    if (*pst_netbuf == NULL) {
        oal_io_print("[E] get recv failed, never here." "\n");
        return -OAL_EFAIL;
    }
    return OAL_SUCC;
}

void hcc_test_rcvd_rw(oal_netbuf_stru *pst_netbuf)
{
    int32_t ret_seq = 0;
    int32_t len = 0;
    hcc_rw_cmd_data recv_data;

    /* 解析接收的参数 */
    len = oal_netbuf_len(pst_netbuf);
    init_cmd_data(&recv_data, oal_netbuf_data(pst_netbuf), len);
    cmd_data_pop_int32(&recv_data, &ret_seq);
    if (!cmd_data_is_valid(&recv_data)) {
        oal_netbuf_free(pst_netbuf);
        oal_io_print("[E] invalid cmd data, pop seq failed. len[%d]" "\n", len);
        return;
    }

    /* seq不相等, 说明不是当前正在等待的指令 */
    if (ret_seq != g_hcc_rw_context.cur_seq) {
        oal_netbuf_free(pst_netbuf);
        oal_io_print("[E] seq not equal, not same cmd!!! ret_seq[%d], cur_seq[%d]" "\n",
            ret_seq, g_hcc_rw_context.cur_seq);
        return;
    }

    /* 在加锁的状态下, 设置返回参数 */
    mutex_lock(&(g_hcc_rw_context.recv_lock));
    if (g_hcc_rw_context.recv_netbuf != NULL) {
        oal_netbuf_free(g_hcc_rw_context.recv_netbuf);
        g_hcc_rw_context.recv_netbuf = NULL;
        oal_io_print("[E] last cmd not handle, never here." "\n");
    }
    g_hcc_rw_context.recv_netbuf = pst_netbuf;
    g_hcc_rw_context.recv_data = recv_data;
    mutex_unlock(&(g_hcc_rw_context.recv_lock));

    /* 通知接收完成 */
    oal_complete(&g_hcc_rw_context.recv_done);
    return;
}

OAL_STATIC void hcc_rw_context_release(hcc_rw_cmd_context* dbg_ctx)
{
    /* 发送数据的内存已经经过锁保护, 因此不需要再加锁 */
    if (dbg_ctx->send_netbuf != NULL) {
        oal_netbuf_free(dbg_ctx->send_netbuf);
        dbg_ctx->send_netbuf = NULL;
    }

    /* 接收数据, 由于回调可能存在多线程同时调度, 因此加锁保护. 内存实际已经交给业务处理, 正常不会进入该处释放 */
    mutex_lock(&(g_hcc_rw_context.recv_lock));
    if (dbg_ctx->recv_netbuf != NULL) {
        oal_netbuf_free(dbg_ctx->recv_netbuf);
        dbg_ctx->recv_netbuf = NULL;
        oal_io_print("[E] recv_netbuf not handle, never here." "\n");
    }
    mutex_unlock(&(g_hcc_rw_context.recv_lock));
}

#define MAX_READMEM_SIZE            1024
#define MAX_WRITEMEM_SIZE           1024
typedef enum _hcc_rw_cmd_type_ {
    DBG_CMD_DEVICE_READMEM = 0,
    DBG_CMD_DEVICE_WRITEMEM = 1,
} hcc_rw_cmd_type;

/**
 * 指令与指令返回值格式:
 *              |-- int32_t --|-- int32_t --|-- 指令参数1 --|-- 指令参数2 --|
 *   写指令     |--   seq   --|-- cmdtype --|--   addr   --|--   value  --|
 *   读指令     |--   seq   --|-- cmdtype --|--   addr   --|--   len    --|
 *  写指令结果  |--   seq   --|-- cmdtype --|--   addr   --|--   len    --|
 *  读指令结果  |--   seq   --|-- cmdtype --|--   addr   --|--   value  --|
 */
OAL_STATIC int32_t hcc_rw_readmem_impl(hcc_rw_cmd_context *dbg_ctx, uint32_t read_addr, int32_t read_len,
    uint8_t* buf)
{
    /* 三个参数：cmd_type, 地址，长度. 6: 每个类型前, 4个字节记录长度 */
    int32_t ret = OAL_FAIL, cmd_type = 0, send_size = sizeof(int32_t) * TEST_DEBUG_READ_PARAM_LEN;
    uint8_t* value = NULL;
    uint32_t value_size = 0;
    oal_netbuf_stru *recv_netbuf = NULL;
    hcc_rw_cmd_data recv_data;

    ret = hcc_rw_context_init_cmd_data(dbg_ctx, send_size);
    if (ret != OAL_SUCC) {
        return -ret;
    }
    do {
        cmd_data_push_int32(&(dbg_ctx->send_data), DBG_CMD_DEVICE_READMEM);
        cmd_data_push_int32(&(dbg_ctx->send_data), read_addr);
        cmd_data_push_int32(&(dbg_ctx->send_data), read_len);
        if (!cmd_data_is_valid(&(dbg_ctx->send_data))) {
            /* never here */
            break;
        }

        ret = hcc_rw_context_send_and_recv(dbg_ctx);
        if (ret != OAL_SUCC) {
            break;
        }
        ret = hcc_rw_context_get_recv(dbg_ctx, &recv_netbuf, &recv_data);
        if (ret != OAL_SUCC) {
            break;
        }

        cmd_data_pop_int32(&recv_data, &cmd_type);
        cmd_data_pop_data(&recv_data, &value, &value_size);
        if (!cmd_data_is_valid(&recv_data)) {
            oal_io_print("[E] read impl pop recv failed" "\n");
            break;
        }

        if (read_len != value_size) {
            oal_io_print("[E] readmem failed, read_len[%d] value_size[%d]" "\n", read_len, value_size);
            break;
        }
        ret = OAL_SUCC;
    } while (0);

    if (buf != NULL && memcpy_s(buf, read_len, value, value_size) != EOK) {
        oal_io_print("[E] copy failed, read_len[%d] value_size[%d]" "\n", read_len, value_size);
    }

    /* 释放内存之前尝试打印读取的值 */
    if (ret == OAL_SUCC) {
        oal_pcie_print_bits(value, read_len);
    }

    if (recv_netbuf != NULL) {
        oal_netbuf_free(recv_netbuf);
        recv_netbuf = NULL;
    }
    hcc_rw_context_release(dbg_ctx);
    return -ret;
}

OAL_STATIC int32_t hcc_rw_writemem_impl(hcc_rw_cmd_context *dbg_ctx, int32_t write_addr,
    uint8_t *write_value, uint32_t write_len)
{
    int32_t ret = OAL_FAIL;
    int32_t cmd_type, len;
    int32_t send_size;
    oal_netbuf_stru *recv_netbuf = NULL;
    hcc_rw_cmd_data recv_data;

    /* 三个参数：cmd_type, 地址，写入的值 */
    send_size = sizeof(int32_t) * TEST_DEBUG_WRITE_PARAM_LEN + ALIGN(write_len, ALIGN_WORD);
    ret = hcc_rw_context_init_cmd_data(dbg_ctx, send_size);
    if (ret != OAL_SUCC) {
        oal_io_print("[E] write impl, init cmd data fail, send_size[%d], ret[%d]" "\n", send_size, ret);
        return -ret;
    }
    do {
        cmd_data_push_int32(&(dbg_ctx->send_data), DBG_CMD_DEVICE_WRITEMEM);
        cmd_data_push_int32(&(dbg_ctx->send_data), write_addr);
        cmd_data_push_data(&(dbg_ctx->send_data), write_value, write_len);
        if (!cmd_data_is_valid(&(dbg_ctx->send_data))) {
            break;
        }

        ret = hcc_rw_context_send_and_recv(dbg_ctx);
        if (ret != OAL_SUCC) {
            break;
        }
        ret = hcc_rw_context_get_recv(dbg_ctx, &recv_netbuf, &recv_data);
        if (ret != OAL_SUCC) {
            break;
        }

        cmd_data_pop_int32(&recv_data, &cmd_type);
        cmd_data_pop_int32(&recv_data, &len);
        if (!cmd_data_is_valid(&recv_data)) {
            oal_io_print("[E] write impl pop recv failed" "\n");
            break;
        }

        if (len != write_len) {
            oal_io_print("[E] writemem failed, len[%d] write_len[%d]" "\n", len, write_len);
            break;
        }
        ret = OAL_SUCC;
    } while (0);
    if (recv_netbuf != NULL) {
        oal_netbuf_free(recv_netbuf);
        recv_netbuf = NULL;
    }
    hcc_rw_context_release(dbg_ctx);
    return -ret;
}

#define ARGC_NUM_ZERO_INDEX         0
#define ARGC_NUM_ONE_INDEX          1
#define ARGC_NUM_TWO_INDEX          2
#define ARGC_NUM_THREE_INDEX        3
#define MAX_ARGC_NUM                7       /* 最大支持七个参数 */
typedef struct _hcc_cmd_args_ {
    int32_t argc;
    char* argv[MAX_ARGC_NUM];
} hcc_cmd_args;

OAL_STATIC int32_t hcc_rw_check_param_addr(const hcc_cmd_args *args, uint32_t *param1)
{
    uint32_t addr = 0xFFFFFFFF;
    if (args == NULL || args->argc <= ARGC_NUM_ONE_INDEX) {
        oal_io_print("[E] args is NULL or invalid param num" "\n");
        return -OAL_EINVAL;
    }
    if ((sscanf_s(args->argv[ARGC_NUM_ONE_INDEX], "0x%x", &addr) != 1)) {
        oal_io_print("[E] invalid addr, addr[%s]" "\n", args->argv[ARGC_NUM_ONE_INDEX]);
        return -OAL_EINVAL;
    }
    if (addr != ALIGN(addr, ALIGN_WORD)) {
        oal_io_print("[E] addr not align. addr[0x%x]" "\n", addr);
        return -OAL_EINVAL;
    }
    *param1 = addr;
    return OAL_SUCC;
}

OAL_STATIC int32_t hcc_rw_check_param_addr_length(const hcc_cmd_args *args, uint32_t *param1, int32_t *param2)
{
    uint32_t addr = 0xFFFFFFFF;
    int32_t len = 0;
    if (args == NULL || args->argc <= ARGC_NUM_TWO_INDEX) {
        oal_io_print("[E] args is NULL or invalid param num" "\n");
        return -OAL_EINVAL;
    }
    if ((sscanf_s(args->argv[ARGC_NUM_ONE_INDEX], "0x%x", &addr) != 1)) {
        oal_io_print("[E] invalid addr, addr[%s]" "\n", args->argv[ARGC_NUM_ONE_INDEX]);
        return -OAL_EINVAL;
    }
    if ((sscanf_s(args->argv[ARGC_NUM_TWO_INDEX], "%d", &len) != 1)) {
        oal_io_print("[E] invalid len, len[%s]" "\n", args->argv[ARGC_NUM_TWO_INDEX]);
        return -OAL_EINVAL;
    }
    if (addr != ALIGN(addr, ALIGN_WORD)) {
        oal_io_print("[E] addr not align. addr[0x%x]" "\n", addr);
        return -OAL_EINVAL;
    }
    if (len <= 0 || len > MAX_READMEM_SIZE) {
        oal_io_print("[E] invald len. len[%d]" "\n", len);
        return -OAL_EINVAL;
    }
    *param1 = addr;
    *param2 = len;
    return OAL_SUCC;
}

OAL_STATIC int32_t hcc_rw_check_param_addr_length_path(const hcc_cmd_args *args, uint32_t *param1, int32_t *param2,
    const char** param3)
{
    uint32_t addr = 0xFFFFFFFF;
    int32_t len = 0;
    const char* path = NULL;
    if (args == NULL || args->argc <= ARGC_NUM_THREE_INDEX) {
        oal_io_print("[E] args is NULL or invalid param num" "\n");
        return -OAL_EINVAL;
    }
    if ((sscanf_s(args->argv[ARGC_NUM_ONE_INDEX], "0x%x", &addr) != 1)) {
        oal_io_print("[E] invalid addr, addr[%s]" "\n", args->argv[ARGC_NUM_ONE_INDEX]);
        return -OAL_EINVAL;
    }
    if ((sscanf_s(args->argv[ARGC_NUM_TWO_INDEX], "%d", &len) != 1)) {
        oal_io_print("[E] invalid len, len[%s]" "\n", args->argv[ARGC_NUM_TWO_INDEX]);
        return -OAL_EINVAL;
    }
    path = args->argv[ARGC_NUM_THREE_INDEX];
    if (addr != ALIGN(addr, ALIGN_WORD)) {
        oal_io_print("[E] addr not align. addr[0x%x]" "\n", addr);
        return -OAL_EINVAL;
    }
    if (len <= 0) {
        oal_io_print("[E] invald len. len[%d]" "\n", len);
        return -OAL_EINVAL;
    }
    *param1 = addr;
    *param2 = len;
    *param3 = path;
    return OAL_SUCC;
}

OAL_STATIC int32_t hcc_rw_check_param_addr_value(const hcc_cmd_args *args, uint32_t *param1, uint32_t *param2)
{
    uint32_t addr = 0xFFFFFFFF, value = 0;
    if (args == NULL || args->argc <= ARGC_NUM_TWO_INDEX) {
        oal_io_print("[E] args is NULL or invalid param num" "\n");
        return -OAL_EINVAL;
    }
    if ((sscanf_s(args->argv[ARGC_NUM_ONE_INDEX], "0x%x", &addr) != 1)) {
        oal_io_print("[E] invalid addr, addr[%s]" "\n", args->argv[ARGC_NUM_ONE_INDEX]);
        return -OAL_EINVAL;
    }
    if ((sscanf_s(args->argv[ARGC_NUM_TWO_INDEX], "0x%x", &value) != 1)) {
        oal_io_print("[E] invalid value, value[%s]" "\n", args->argv[ARGC_NUM_TWO_INDEX]);
        return -OAL_EINVAL;
    }
    if (addr != ALIGN(addr, ALIGN_WORD)) {
        oal_io_print("[E] addr not align. addr[0x%x]" "\n", addr);
        return -OAL_EINVAL;
    }
    *param1 = addr;
    *param2 = value;
    return OAL_SUCC;
}

OAL_STATIC int32_t hcc_rw_check_param_addr_path(const hcc_cmd_args *args, uint32_t *param1, const char** param2)
{
    uint32_t addr = 0xFFFFFFFF;
    const char* path = NULL;
    if (args == NULL || args->argc <= ARGC_NUM_TWO_INDEX) {
        oal_io_print("[E] args is NULL or invalid param num" "\n");
        return -OAL_EINVAL;
    }
    if ((sscanf_s(args->argv[ARGC_NUM_ONE_INDEX], "0x%x", &addr) != 1)) {
        oal_io_print("[E] invalid addr, addr[%s]" "\n", args->argv[ARGC_NUM_ONE_INDEX]);
        return -OAL_EINVAL;
    }
    path = args->argv[ARGC_NUM_TWO_INDEX];
    if (addr != ALIGN(addr, ALIGN_WORD)) {
        oal_io_print("[E] addr not align. addr[0x%x]" "\n", addr);
        return -OAL_EINVAL;
    }
    *param1 = addr;
    *param2 = path;
    return OAL_SUCC;
}

OAL_STATIC int32_t hcc_rw_read16(hcc_rw_cmd_context *dbg_ctx, const hcc_cmd_args *args)
{
    int32_t ret;
    uint32_t addr;

    ret = hcc_rw_check_param_addr(args, &addr);
    if (ret != OAL_SUCC) {
        oal_io_print("[E] check param failed, ret[%d]" "\n", ret);
        return -ret;
    }

    ret = hcc_rw_readmem_impl(dbg_ctx, addr, sizeof(int16_t), NULL);
    if (ret != OAL_SUCC) {
        oal_io_print("[E] read16 fail, ret[%d]" "\n", ret);
        return -ret;
    }
    return -ret;
}

OAL_STATIC int32_t hcc_rw_read32(hcc_rw_cmd_context *dbg_ctx, const hcc_cmd_args *args)
{
    int32_t ret;
    uint32_t addr;

    ret = hcc_rw_check_param_addr(args, &addr);
    if (ret != OAL_SUCC) {
        oal_io_print("[E] check param failed, ret[%d]" "\n", ret);
        return -ret;
    }

    ret = hcc_rw_readmem_impl(dbg_ctx, addr, sizeof(int32_t), NULL);
    if (ret != OAL_SUCC) {
        oal_io_print("[E] read32 fail, ret[%d]" "\n", ret);
        return -ret;
    }
    return -ret;
}

OAL_STATIC int32_t hcc_rw_readmem(hcc_rw_cmd_context *dbg_ctx, const hcc_cmd_args *args)
{
    int32_t ret;
    uint32_t addr;
    int32_t len;
    void *pst_buf = NULL;

    ret = hcc_rw_check_param_addr_length(args, &addr, &len);
    if (ret != OAL_SUCC) {
        oal_io_print("[E] check param failed, ret[%d]" "\n", ret);
        return -ret;
    }

    pst_buf = oal_memalloc(len);
    if (pst_buf == NULL) {
        oal_io_print("[E] oal_memalloc failed. len[%d]" "\n", len);
        return -OAL_ENOMEM;
    }
    do {
        ret = hcc_rw_readmem_impl(dbg_ctx, addr, len, pst_buf);
        if (ret != OAL_SUCC) {
            oal_io_print("[E] readmem fail, ret[%d]" "\n", ret);
            break;
        }
        print_hex_dump(KERN_INFO, "readmem: ", DUMP_PREFIX_OFFSET, TEST_DEBUG_DUMPMEM_WIDTH, sizeof(int32_t),
            pst_buf, len, true);
    } while (0);
    oal_free(pst_buf);
    return -ret;
}

OAL_STATIC int32_t hcc_rw_savefile(hcc_rw_cmd_context *dbg_ctx, const hcc_cmd_args *args)
{
    int32_t ret;

    uint32_t addr;
    int32_t  len;
    const char *path = NULL;

    mm_segment_t fs;
    struct file *fp = NULL;
    void *pst_buf = NULL;
    int32_t write_len = 0;

    ret = hcc_rw_check_param_addr_length_path(args, &addr, &len, &path);
    if (ret != OAL_SUCC) {
        return -ret;
    }

    pst_buf = oal_memalloc(MAX_READMEM_SIZE);
    if (pst_buf == NULL) {
        oal_io_print("[E] oal_memalloc failed. MAX_READMEM_SIZE[%d]" "\n", MAX_READMEM_SIZE);
        return -OAL_ENOMEM;
    }

    fs = get_fs();
    set_fs(KERNEL_DS);
    fp = filp_open(path, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
    if (oal_is_err_or_null(fp)) {
        oal_free(pst_buf);
        set_fs(fs);
        oal_io_print("[E] open file error, path[%s]" "\n", path);
        return -OAL_EINVAL;
    }

    while (write_len < len) {
        /* 分块读取内存长度, 然后写入文件,每次最多读取 1024 字节 */
        int read_len = MAX_READMEM_SIZE < (len - write_len) ? MAX_READMEM_SIZE : (len - write_len);
        ret = hcc_rw_readmem_impl(dbg_ctx, addr + write_len, read_len, pst_buf);
        if (ret != OAL_SUCC) {
            break;
        }
        ret = oal_kernel_file_write(fp, pst_buf, read_len);
        if (ret != OAL_SUCC) {
            break;
        }
        write_len += read_len;
    }

    oal_io_print("[I] savefile. write_len[%u], len[%u]" "\n", write_len, len);

    oal_free(pst_buf);
#if (KERNEL_VERSION(2, 6, 35) <= LINUX_VERSION_CODE)
    vfs_fsync(fp, 0);
#else
    vfs_fsync(fp, fp->f_path.dentry, 0);
#endif
    filp_close(fp, 0);
    set_fs(fs);
    return OAL_SUCC;
}

OAL_STATIC int32_t hcc_rw_write16(hcc_rw_cmd_context *dbg_ctx, const hcc_cmd_args *args)
{
    int32_t ret;
    uint32_t addr, value;

    ret = hcc_rw_check_param_addr_value(args, &addr, &value);
    if (ret != OAL_SUCC) {
        oal_io_print("[E] check param failed, ret[%d]" "\n", ret);
        return -ret;
    }

    ret = hcc_rw_writemem_impl(dbg_ctx, addr, (uint8_t *)&value, sizeof(int16_t));
    if (ret != OAL_SUCC) {
        oal_io_print("[E] write16 failed. ret[%d]" "\n", ret);
    } else {
        oal_io_print("[I] write16 success. addr[0x%x], value[0x%x]" "\n", addr, value);
    }
    return -ret;
}

OAL_STATIC int32_t hcc_rw_write32(hcc_rw_cmd_context *dbg_ctx, const hcc_cmd_args *args)
{
    int32_t ret;
    uint32_t addr, value;

    ret = hcc_rw_check_param_addr_value(args, &addr, &value);
    if (ret != OAL_SUCC) {
        oal_io_print("[E] check param failed, ret[%d]" "\n", ret);
        return -ret;
    }

    ret = hcc_rw_writemem_impl(dbg_ctx, addr, (uint8_t *)&value, sizeof(int32_t));
    if (ret != OAL_SUCC) {
        oal_io_print("[E] write32 failed. ret[%d]" "\n", ret);
    } else {
        oal_io_print("[I] write32 success. addr[0x%x], value[0x%x]" "\n", addr, value);
    }
    return -ret;
}

OAL_STATIC int32_t hcc_rw_loadfile(hcc_rw_cmd_context *dbg_ctx, const hcc_cmd_args *args)
{
    int32_t ret;

    uint32_t addr = 0xFFFFFFFF;
    const char* path = NULL;

    mm_segment_t fs;
    struct file *fp = NULL;
    void *pst_buf = NULL;
    int32_t write_len = 0;

    ret = hcc_rw_check_param_addr_path(args, &addr, &path);
    if (ret != OAL_SUCC) {
        oal_io_print("[E] check param failed, ret[%d]" "\n", ret);
        return -ret;
    }

    pst_buf = oal_memalloc(MAX_WRITEMEM_SIZE);
    if (pst_buf == NULL) {
        oal_io_print("[E] pst_buf is null");
        return -OAL_ENOMEM;
    }

    fs = get_fs();
    set_fs(KERNEL_DS);
    fp = filp_open(path, O_RDONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
    if (oal_is_err_or_null(fp)) {
        oal_free(pst_buf);
        set_fs(fs);
        oal_io_print("[E] open file error, path[%s]" "\n", path);
        return -OAL_EINVAL;
    }

    forever_loop() {
        /* 读取文件, 直到不存在数据 */
        int32_t len = oal_kernel_file_read(fp, (void *)pst_buf, MAX_WRITEMEM_SIZE);
        if (len <= 0) {
            if (write_len <= 0) {
                oal_io_print("[E] read local file failed. write_len[%d], len[%d]" "\n", write_len, len);
            }
            break;
        }

        ret = hcc_rw_writemem_impl(dbg_ctx, addr + write_len, (uint8_t *)pst_buf, len);
        if (ret != OAL_SUCC) {
            oal_io_print("[E] writemem failed. ret[%u]" "\n", ret);
            break;
        }
        write_len += len;
    }

    oal_io_print("[I] load file, addr[0x%x] write_len[%u], path[%s]" "\n", addr, write_len, path);

    oal_free(pst_buf);
    filp_close(fp, 0);
    set_fs(fs);
    return OAL_SUCC;
}

OAL_STATIC int32_t hcc_rw_host_read32(hcc_rw_cmd_context *dbg_ctx, const hcc_cmd_args *args)
{
    int32_t addr, align_addr;
    void *pst_vaddr = NULL;
    int32_t value;

    int32_t ret = hcc_rw_check_param_addr(args, &addr);
    if (ret != OAL_SUCC) {
        oal_io_print("[E] check param failed, ret[%d]" "\n", ret);
        return -ret;
    }

    /* 找到对应的页地址, 整个映射 */
    align_addr = OAL_ROUND_DOWN(addr, PAGE_SIZE);
    pst_vaddr = oal_ioremap_nocache(align_addr, PAGE_SIZE);
    if (pst_vaddr == NULL) {
        oal_io_print("[E] read32 addr remap failed! align_addr[0x%x]\n", align_addr);
        return -OAL_EBUSY;
    }
    value = oal_readl(pst_vaddr + (addr - align_addr));
    oal_iounmap(pst_vaddr);

    oal_pcie_print_bits(&value, sizeof(uint32_t));
    return OAL_SUCC;
}

OAL_STATIC int32_t hcc_rw_host_write32(hcc_rw_cmd_context *dbg_ctx, const hcc_cmd_args *args)
{
    /* echo host_write32 address value > debug */
    uint32_t addr, value;
    uint32_t align_addr, old, read_value;
    void *pst_vaddr = NULL;
    void *pst_align_addr = NULL;

    int32_t ret = hcc_rw_check_param_addr_value(args, &addr, &value);
    if (ret != OAL_SUCC) {
        oal_io_print("[E] check param failed, ret[%d]" "\n", ret);
        return -ret;
    }

    /* 找到对应的页地址, 整个映射 */
    align_addr = OAL_ROUND_DOWN(addr, PAGE_SIZE);
    pst_vaddr = oal_ioremap_nocache(align_addr, PAGE_SIZE);
    if (pst_vaddr == OAL_SUCC) {
        oal_io_print("[E] addr remap failed! align_addr[0x%x]\n", align_addr);
        return -OAL_EBUSY;
    }
    pst_align_addr = pst_vaddr + (addr - align_addr);
    old = oal_readl((void *)pst_align_addr);
    oal_writel(value, (void *)pst_align_addr);
    read_value = oal_readl((void *)pst_align_addr);
    oal_iounmap(pst_vaddr);

    oal_io_print("[I] change addr[0x%x] 0x%8x -> 0x%8x,  callback-read[0x%8x]" "\n",
        addr, old, value, read_value);
    return OAL_SUCC;
}

typedef int32_t (*hcc_rw_cmd_fun)(hcc_rw_cmd_context *dbg_ctx, const hcc_cmd_args *args);
typedef struct _hcc_rw_cmd_ {
    char *name;
    char *usage;
    hcc_rw_cmd_fun dbg;
} hcc_rw_cmd;

OAL_STATIC int32_t hcc_rw_help(hcc_rw_cmd_context *dbg_ctx, const hcc_cmd_args *args);
OAL_STATIC hcc_rw_cmd g_hcc_rw_cmd[] = {
    { "help", "", hcc_rw_help },
    { "read16", "address(hex)", hcc_rw_read16 },
    { "read32", "address(hex)", hcc_rw_read32 },
    { "readmem", "address(hex) length(decimal)", hcc_rw_readmem },
    { "savefile", "address(hex) length(decimal) filename", hcc_rw_savefile },
    { "write16", "address(hex) value(hex)", hcc_rw_write16 },
    { "write32", "address(hex) value(hex)", hcc_rw_write32 },
    { "loadfile", "address(hex) filename", hcc_rw_loadfile },
    { "host_read32", "address(hex)", hcc_rw_host_read32 },
    { "host_write32", "address(hex) value(hex)", hcc_rw_host_write32 },
};

OAL_STATIC void hcc_rw_print_usage(int32_t i)
{
    int32_t ret;
    void *buf = oal_memalloc(PAGE_SIZE);
    if (buf == NULL) {
        return;
    }

    ret = snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "echo %s %s > /sys/hisys/hcc_dbg/hcc_dbg/debug" "\n",
                     g_hcc_rw_cmd[i].name ? : "", g_hcc_rw_cmd[i].usage ? : "");
    if (ret < 0) {
        oal_free(buf);
        oal_io_print("[E] log str format err line[%d]" "\n", __LINE__);
        return;
    }
    printk("%s", (char *)buf);

    oal_free(buf);
}

OAL_STATIC void hcc_rw_print_usages(void)
{
    int32_t i;
    for (i = 0; i < oal_array_size(g_hcc_rw_cmd); i++) {
        hcc_rw_print_usage(i);
    }
}

OAL_STATIC int32_t hcc_rw_help(hcc_rw_cmd_context *dbg_ctx, const hcc_cmd_args *args)
{
    hcc_rw_print_usages();
    return OAL_SUCC;
}

OAL_STATIC ssize_t hcc_rw_show_info(struct kobject *dev, struct kobj_attribute *attr, char *buf)
{
    int ret;
    int32_t i;
    int32_t count = 0;

    ret = snprintf_s(buf + count, PAGE_SIZE - count, PAGE_SIZE - count - 1, "pci debug cmds:" "\n");
    if (ret < 0) {
        oal_io_print("[E] log str failed, never here. " "\n");
        return count;
    }
    count += ret;
    for (i = 0; i < oal_array_size(g_hcc_rw_cmd); i++) {
        ret = snprintf_s(buf + count, PAGE_SIZE - count, PAGE_SIZE - count - 1, "%s %s" "\n",
            g_hcc_rw_cmd[i].name, g_hcc_rw_cmd[i].usage);
        if (ret < 0) {
            oal_io_print("[E] log str format err line[%d]" "\n", __LINE__);
            return count;
        }
        count += ret;
    }
    return count;
}

/* 解析buf中的参数, 空格作为分隔符, 转换为字符串结束符 */
int32_t parse_hcc_cmd_args(char *buf, uint32_t buf_size, hcc_cmd_args* args)
{
    int i, j;
    int param_len = 0;
    if (buf == NULL || args == NULL) {
        return -OAL_EINVAL;
    }

    for (i = 0; i < buf_size; ++i) {
        if (buf[i] == ' ') {  /* 参数不支持空格 */
            buf[i] = '\0';
        }
        if (buf[i] == '\n') { /* 命令，参数不支持换行 */
            for (j = i; j < buf_size; ++j) {
                buf[j] = '\0';
            }
            break;
        }
    }

    args->argc = 0;
    memset_s(args->argv, sizeof(args->argv), 0, sizeof(args->argv));
    param_len = 0;
    for (i = 0; i < buf_size; ++i) {
        if (buf[i] == '\0') {
            continue;
        }
        args->argv[args->argc] = buf + i;
        args->argc ++;
        if (args->argc >= MAX_ARGC_NUM) {
            break;
        }

        param_len = OAL_STRLEN(buf + i);
        i += param_len;
    }

    return OAL_SUCC;
}

OAL_STATIC int find_cmd_by_name(const char *buf, hcc_rw_cmd *cmds, uint32_t size)
{
    int32_t idx;
    for (idx = 0; idx < size; idx++) {
        if (oal_strcmp(g_hcc_rw_cmd[idx].name, buf) == 0) {
            return idx;
        }
    }
    return -1;
}

OAL_STATIC int handle_hcc_rw_cmd(hcc_cmd_args *args)
{
    int ret = 0;
    int32_t idx;
    if (args == NULL || args->argc <= ARGC_NUM_ZERO_INDEX) {
        oal_io_print("[E] args is NULL or invalid param num" "\n");
        return -OAL_EINVAL;
    }
    idx = find_cmd_by_name(args->argv[ARGC_NUM_ZERO_INDEX], g_hcc_rw_cmd, oal_array_size(g_hcc_rw_cmd));
    if (idx < 0) {
        oal_io_print("[E] invalid cmd, cmd[%s]" "\n", args->argv[ARGC_NUM_ZERO_INDEX]);
        return -OAL_EINVAL;
    }

    /* 所有的命令互斥操作，同一时刻只有一个命令运行 */
    ret = mutex_lock_interruptible(&(g_hcc_rw_context.cmd_lock));
    if (ret != 0) {
        oal_io_print("[E] cmd lock failed, ret[%d]" "\n", ret);
        return -OAL_EINVAL;
    }
    if (g_hcc_rw_cmd[idx].dbg(&g_hcc_rw_context, args) == -OAL_EINVAL) {
        hcc_rw_print_usage(idx);
    }
    mutex_unlock(&(g_hcc_rw_context.cmd_lock));
    return OAL_SUCC;
}

OAL_STATIC ssize_t hcc_rw_store_info(struct kobject *dev, struct kobj_attribute *attr, const char *buf, size_t count)
{
    hcc_cmd_args args;
    char *dump_buf = NULL;
    if (count <= 0) {
        oal_io_print("[E] invalid param, count[%d]" "\n", (int32_t)count);
        hcc_rw_print_usages();
        return count;
    }

    /* 将对应参数复制一份,同时添加字符串结束符 */
    dump_buf = (char *)oal_memalloc(count + 1);
    if (dump_buf == NULL) {
        oal_io_print("[E] alloc buf failed, count[%u]" "\n", (int32_t)count + 1);
        return -OAL_ENOMEM;
    }
    if (memcpy_s(dump_buf, count + 1, buf, count) != EOK) {
        oal_io_print("[E] copy failed, count[%d]" "\n", (int32_t)count);
        oal_free(dump_buf);
        dump_buf = NULL;
        return -OAL_ENOMEM;
    }
    dump_buf[count] = '\0';
    do {
        int ret = parse_hcc_cmd_args(dump_buf, count + 1, &args);
        if (ret != OAL_SUCC) {
            oal_io_print("[E] parse param failed, dump_buf[%s]" "\n", dump_buf);
            hcc_rw_print_usages();
            break;
        }
        handle_hcc_rw_cmd(&args);
    } while (0);
    oal_free(dump_buf);
    dump_buf = NULL;

    return count;
}

OAL_STATIC struct kobj_attribute g_hcc_rw_attr =
    __ATTR(debug, S_IRUGO | S_IWUSR, hcc_rw_show_info, hcc_rw_store_info);
OAL_STATIC struct attribute *g_hcc_rw_sysfs_entries[] = {
    &g_hcc_rw_attr.attr,
    NULL
};
OAL_STATIC struct attribute_group g_hcc_rw_attribute_group = {
    .name = "hcc_dbg",
    .attrs = g_hcc_rw_sysfs_entries,
};
OAL_STATIC oal_kobject *g_hisys_hcc_rw_object = NULL;

OAL_STATIC int32_t hcc_rw_init_cmd_context(hcc_rw_cmd_context *dbg_ctx)
{
    mutex_init(&(dbg_ctx->cmd_lock));
    dbg_ctx->send_netbuf = NULL;
    dbg_ctx->recv_netbuf = NULL;
    mutex_init(&(dbg_ctx->recv_lock));
    dbg_ctx->cur_seq = 0;
    oal_init_completion(&(dbg_ctx->recv_done));
    oal_io_print("[I] hcc_rw_init_cmd_context success." "\n");
    return OAL_SUCC;
}

int32_t hcc_test_rw_init_module(struct hcc_handler *hcc)
{
    int32_t ret = OAL_SUCC;
    oal_kobject *pst_root_object = NULL;

    if (oal_unlikely(hcc == NULL)) {
        oal_warn_on(1);
        return OAL_FAIL;
    }
    pst_root_object = oal_get_sysfs_root_object();
    if (pst_root_object == NULL) {
        oal_io_print("[E] get root sysfs object failed!" "\n");
        return -OAL_EFAIL;
    }
    g_hisys_hcc_rw_object = kobject_create_and_add("hcc_dbg", pst_root_object);
    if (g_hisys_hcc_rw_object == NULL) {
        oal_io_print("[E] create hcc_dbg failed!" "\n");
        return -OAL_EFAIL;
    }
    ret = oal_debug_sysfs_create_group(g_hisys_hcc_rw_object, &g_hcc_rw_attribute_group);
    if (ret) {
        oal_io_print("[E] create hcc_dbg group failed!" "\n");
        kobject_put(g_hisys_hcc_rw_object);
        return -OAL_ENOMEM;
    }

    hcc_rw_init_cmd_context(&g_hcc_rw_context);

    oal_io_print("[I] hcc_rw_init_module success." "\n");
    return OAL_SUCC;
}

void hcc_test_rw_exit_module(struct hcc_handler *hcc)
{
    if (oal_unlikely(hcc == NULL)) {
        oal_warn_on(1);
        return;
    }
    oal_debug_sysfs_remove_group(g_hisys_hcc_rw_object, &g_hcc_rw_attribute_group);
    kobject_put(g_hisys_hcc_rw_object);
    oal_io_print("[I] hcc_rw_exit_module success." "\n");
}
#endif

#endif /* _PRE_CONFIG_HCC_TEST_RW */
