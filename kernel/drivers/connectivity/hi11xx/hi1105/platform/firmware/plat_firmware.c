/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description: load firmware
 * Author: @CompanyNameTag
 */

#include <linux/moduleparam.h>
#include <linux/delay.h>
#include <linux/rtc.h>

#include "oal_sdio_host_if.h"
#include "oal_hcc_host_if.h"
#include "oal_kernel_file.h"
#include "oal_util.h"
#include "securec.h"
#include "oam_ext_if.h"
#include "bfgx_dev.h"
#include "bfgx_core.h"
#include "plat_debug.h"
#include "plat_common_clk.h"
#include "plat_uart.h"
#include "plat_cali.h"
#include "plat_pm.h"
#include "plat_exception_rst.h"
#include "plat_cust.h"
#include "plat_firmware.h"

/* 全局变量定义 */
/* mp13 pilot cfg文件路径 */
STATIC uint8_t *g_mp13_pilot_cfg_patch_in_vendor[CFG_FILE_TOTAL] = {
    BFGX_AND_WIFI_CFG_MP13_PILOT_PATH,
    WIFI_CFG_MP13_PILOT_PATH,
    BFGX_CFG_MP13_PILOT_PATH,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

/* mp15 */
STATIC uint8_t *g_mp15_asic_cfg_patch_in_vendor[CFG_FILE_TOTAL] = {
    BFGX_AND_WIFI_CFG_MP15_ASIC_PATH,
    WIFI_CFG_MP15_ASIC_PATH,
    BFGX_CFG_MP15_ASIC_PATH,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

/* mp16 */
#define FIRMWARE_MP16_PILOT_CFG_DIR "/vendor/firmware/shenkuo/pilot"
STATIC uint8_t *g_mp16_asic_cfg_patch_in_vendor[CFG_FILE_TOTAL] = {
    BFGX_AND_WIFI_CFG_FILE,
    WIFI_CFG_FILE,
    BFGX_CFG_FILE,
    RAM_WIFI_CHECK_FILE,
    NULL,
    NULL,
    NULL,
    NULL
};

/* mp16c */
#define FIRMWARE_MP16C_PILOT_CFG_DIR "/vendor/firmware/bisheng/pilot"
#define FIRMWARE_MP16C_FPGA_CFG_DIR  "/vendor/firmware/bisheng/fpga"
STATIC uint8_t *g_mp16c_cfg_patch_in_vendor[CFG_FILE_TOTAL] = {
    BFGX_AND_WIFI_CFG_FILE,
    WIFI_CFG_FILE,
    BFGX_CFG_FILE,
    NULL,
    NULL,
    SLE_CFG_FILE,
    BI_CFG_FILE,
    IR_CFG_FILE
};

/* gf61 */
STATIC uint8_t *g_gf61_asic_cfg_patch_in_vendor[CFG_FILE_TOTAL] = {
    BFGX_AND_WIFI_CFG_FILE,
    WIFI_CFG_FILE,
    BFGX_CFG_FILE,
    RAM_WIFI_CHECK_FILE,
    GT_CFG_FILE,
    SLE_CFG_FILE,
    NULL,
    NULL
};

typedef struct {
    int32_t chip_nr;
    int32_t is_asic;
    uint8_t **cfg_path;
    uint8_t *pre_directory;
} chip_firmware_cfg_str;

STATIC chip_firmware_cfg_str g_chip_firmware_cfg[] = {
    {BOARD_VERSION_MP13,  VERSION_ASIC, g_mp13_pilot_cfg_patch_in_vendor, NULL},
    {BOARD_VERSION_MP15,  VERSION_ASIC, g_mp15_asic_cfg_patch_in_vendor, NULL},
    {BOARD_VERSION_MP16,  VERSION_ASIC, g_mp16_asic_cfg_patch_in_vendor, FIRMWARE_MP16_PILOT_CFG_DIR},
    {BOARD_VERSION_MP16C, VERSION_ASIC, g_mp16c_cfg_patch_in_vendor, FIRMWARE_MP16C_PILOT_CFG_DIR},
    {BOARD_VERSION_MP16C, VERSION_FPGA, g_mp16c_cfg_patch_in_vendor, FIRMWARE_MP16C_FPGA_CFG_DIR},
    {BOARD_VERSION_GF61,  VERSION_ASIC, g_gf61_asic_cfg_patch_in_vendor, FIRMWARE_GF61_CFG_DIR}
};

/* 存储cfg文件信息，解析cfg文件时赋值，加载的时候使用该变量 */
firmware_globals g_cfg_info;

/* 保存firmware file内容的buffer，先将文件读到这个buffer中，然后从这个向device buffer发送 */
STATIC uint8_t *g_firmware_down_buf = NULL;

/* DataBuf的长度 */
STATIC uint32_t g_firmware_down_buf_len = 0;

#define DEVICE_MEM_DUMP_MAX_BUFF_SIZE 100
#define QUIT_TYPE_CMD_MAX_BUFF_SIZE 8
#define UART_RATE_PARA_LEN  16

STATIC int32_t exec_number_wmem_cmd(uint8_t *key, uint8_t *value);
STATIC int32_t exec_number_sleep_cmd(uint8_t *key, uint8_t *value);
STATIC int32_t exec_number_cali_count_cmd(uint8_t *key, uint8_t *value);
STATIC int32_t exec_number_cali_bfgx_data(uint8_t *key, uint8_t *value);
STATIC int32_t exec_number_cali_dcxo_data(uint8_t *key, uint8_t *value);
STATIC int32_t exec_number_plat_cust_data(uint8_t *key, uint8_t *value);
STATIC int32_t exec_number_jump_cmd(uint8_t *key, uint8_t *value);
STATIC int32_t exec_number_rmem_cmd(uint8_t *key, uint8_t *value);
STATIC int32_t exec_baudrate_cmd(uint8_t *key, uint8_t *value);

STATIC int32_t build_file_cmd(uint8_t *buff, uint32_t len, unsigned long addr_send, uint32_t data_len);

/* 根据命令解析 */
STATIC firmware_number g_number_type[] = {
    {WMEM_CMD_KEYWORD,  exec_number_wmem_cmd},
    {SLEEP_CMD_KEYWORD, exec_number_sleep_cmd},
    {CALI_COUNT_CMD_KEYWORD, exec_number_cali_count_cmd},
    {CALI_BFGX_DATA_CMD_KEYWORD, exec_number_cali_bfgx_data},
    {CALI_DCXO_DATA_CMD_KEYWORD, exec_number_cali_dcxo_data},
    {PLAT_CUST_CMD_KEYWORD, exec_number_plat_cust_data},
    {JUMP_CMD_KEYWORD, exec_number_jump_cmd},
    {RMEM_CMD_KEYWORD, exec_number_rmem_cmd},
    {BRT_CMD_KEYWORD, exec_baudrate_cmd}
};


/*
 * 函 数 名  : malloc_cmd_buf
 * 功能描述  : 解析cfg文件，将解析的结果保存在g_st_cfg_info全局变量中
 * 输入参数  : puc_cfg_info_buf: 保存有cfg文件内容的buffer
 *             ul_index        : 保存解析结果的数组索引值
 * 返 回 值  : NULL表示分配内存失败，否则返回指向保存解析cfg文件命令数组的首地址
 */
STATIC void *malloc_cmd_buf(uint8_t *puc_cfg_info_buf, uint32_t idx)
{
    int32_t l_len;
    uint8_t *flag = NULL;
    uint8_t *p_buf = NULL;

    if (puc_cfg_info_buf == NULL) {
        ps_print_err("malloc_cmd_buf: buf is NULL!\n");
        return NULL;
    }

    /* 统计命令个数 */
    flag = puc_cfg_info_buf;
    g_cfg_info.count[idx] = 0;
    while (flag != NULL) {
        /* 一个正确的命令行结束符为 ; */
        flag = os_str_chr(flag, CMD_LINE_SIGN);
        if (flag == NULL) {
            break;
        }
        g_cfg_info.count[idx]++;
        flag++;
    }
    ps_print_dbg("cfg file cmd count: al_count[%d] = %d\n", idx, g_cfg_info.count[idx]);

    /* 申请存储命令空间 */
    l_len = ((g_cfg_info.count[idx]) + CFG_INFO_RESERVE_LEN) * sizeof(struct cmd_type_st);
    p_buf = os_kmalloc_gfp(l_len);
    if (p_buf == NULL) {
        ps_print_err("kmalloc cmd_type_st fail\n");
        return NULL;
    }
    memset_s((void *)p_buf, l_len, 0, l_len);

    return p_buf;
}

/*
 * 函 数 名  : open_file_to_readm
 * 功能描述  : 打开文件，保存read mem读上来的内容
 * 返 回 值  : 返回打开文件的描述符
 */
STATIC os_kernel_file *open_file_to_readm(uint8_t *name)
{
    os_kernel_file *fp = NULL;
    uint8_t *file_name = NULL;
    mm_segment_t fs;

    if (name == NULL) {
        file_name = WIFI_DUMP_PATH "/readm_wifi";
    } else {
        file_name = name;
    }

    fs = get_fs();
    set_fs(KERNEL_DS);
    fp = filp_open(file_name, O_RDWR | O_CREAT, 0664); /* 0664权限，拥有者可读写 */
    set_fs(fs);

    return fp;
}

/*
 * 函 数 名  : recv_device_mem
 * 功能描述  : 接收device发送上来的内存，保存到指定的文件中
 * 输入参数  : fp : 保存内存的文件指针
 *             len: 需要保存的内存的长度
 * 返 回 值  : -1表示失败，否则返回实际保存的内存的长度
 */
STATIC int32_t recv_device_mem(os_kernel_file *fp, uint8_t *data_buf, int32_t len)
{
    int32_t l_ret = -EFAIL;
    mm_segment_t fs;
    uint8_t retry = 3;
    int32_t lenbuf = 0;

    if (oal_is_err_or_null(fp)) {
        ps_print_err("fp is error,fp = 0x%p\n", fp);
        return -EFAIL;
    }

    if (data_buf == NULL) {
        ps_print_err("data_buf is NULL\n");
        return -EFAIL;
    }

    ps_print_dbg("expect recv len is [%d]\n", len);

    fs = get_fs();
    set_fs(KERNEL_DS);
    ps_print_dbg("pos = %d\n", (int)fp->f_pos);
    while (len > lenbuf) {
        l_ret = read_msg(data_buf + lenbuf, len - lenbuf);
        if (l_ret > 0) {
            lenbuf += l_ret;
        } else {
            retry--;
            lenbuf = 0;
            if (retry == 0) {
                l_ret = -EFAIL;
                ps_print_err("time out\n");
                break;
            }
        }
    }

    if (len <= lenbuf) {
        oal_kernel_file_write(fp, data_buf, len);
    }
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35))
    vfs_fsync(fp, 0);
#else
    vfs_fsync(fp, fp->f_path.dentry, 0);
#endif
    set_fs(fs);

    return l_ret;
}

/*
 * 函 数 名  : check_version
 * 功能描述  : 发送命令读device版本号，并检查device上报的版本号和host的版本号是否匹配
 * 返 回 值  : -1表示失败，0表示成功
 */
STATIC int32_t check_version(void)
{
    int32_t l_ret;
    int32_t l_len;
    int32_t i;
    uint8_t rec_buf[VERSION_LEN];

    /* bfg独立加载时，wifi不在bootloader中，没有应答，并且wifi加载时，已经检查过了, 跳过检查 */
    if (!wlan_is_shutdown()) {
        return SUCC;
    }

    for (i = 0; i < HOST_DEV_TIMEOUT; i++) {
        memset_s(rec_buf, VERSION_LEN, 0, VERSION_LEN);

        l_ret = memcpy_s(rec_buf, sizeof(rec_buf), (uint8_t *)VER_CMD_KEYWORD, os_str_len(VER_CMD_KEYWORD));
        if (l_ret != EOK) {
            ps_print_err("rec_buf not enough\n");
            return -EFAIL;
        }
        l_len = (int32_t)os_str_len(VER_CMD_KEYWORD);

        rec_buf[l_len] = COMPART_KEYWORD;
        l_len++;

        l_ret = send_msg(rec_buf, l_len);
        if (l_ret < 0) {
            ps_print_err("send version fail![%d]\n", i);
            continue;
        }

        memset_s(g_cfg_info.dev_version, VERSION_LEN, 0, VERSION_LEN);
        memset_s(rec_buf, VERSION_LEN, 0, VERSION_LEN);
        msleep(1);

        l_ret = read_msg(rec_buf, VERSION_LEN);
        if (l_ret < 0) {
            ps_print_err("read version fail![%d]\n", i);
            continue;
        }

        memcpy_s(g_cfg_info.dev_version, VERSION_LEN, rec_buf, VERSION_LEN);

        if (!os_mem_cmp((char *)g_cfg_info.dev_version,
                        (char *)g_cfg_info.cfg_version,
                        os_str_len(g_cfg_info.cfg_version))) {
            ps_print_info("Device Version = [%s], CfgVersion = [%s].\n",
                          g_cfg_info.dev_version, g_cfg_info.cfg_version);
            return SUCC;
        } else {
            ps_print_err("ERROR version,Device Version = [%s], CfgVersion = [%s].\n",
                         g_cfg_info.dev_version, g_cfg_info.cfg_version);
        }
    }

    return -EFAIL;
}

/*
 * 函 数 名  : number_type_cmd_send
 * 功能描述  : 处理number类型的命令，并发送到device
 * 输入参数  : Key  : 命令的关键字
 *             Value: 命令的参数
 * 返 回 值  : -1表示失败，非零表示成功
 */
int32_t number_type_cmd_send(const char *key, const char *value)
{
    int32_t l_ret;
    int32_t data_len;
    int32_t value_len;
    int32_t i;
    int32_t n;
    uint8_t num[INT32_STR_LEN];
    uint8_t buff_tx[SEND_BUF_LEN];

    value_len = (int32_t)os_str_len(value);

    memset_s(num, INT32_STR_LEN, 0, INT32_STR_LEN);
    memset_s(buff_tx, SEND_BUF_LEN, 0, SEND_BUF_LEN);

    data_len = (int32_t)os_str_len(key);
    l_ret = memcpy_s(buff_tx, sizeof(buff_tx), key, data_len);
    if (l_ret != EOK) {
        ps_print_err("buff_tx not enough\n");
        return -EFAIL;
    }

    buff_tx[data_len] = COMPART_KEYWORD;
    data_len = data_len + 1;

    for (i = 0, n = 0; (i <= value_len) && (n < INT32_STR_LEN); i++) {
        if ((value[i] == ',') || (value_len == i)) {
            ps_print_dbg("num = %s, i = %d, n = %d\n", num, i, n);
            if (n == 0) {
                continue;
            }
            l_ret = memcpy_s((uint8_t *)&buff_tx[data_len], sizeof(buff_tx) - data_len, num, n);
            if (l_ret != EOK) {
                ps_print_err("buff_tx not enough\n");
                return -EFAIL;
            }
            data_len = data_len + n;

            buff_tx[data_len] = COMPART_KEYWORD;
            data_len = data_len + 1;

            memset_s(num, INT32_STR_LEN, 0, INT32_STR_LEN);
            n = 0;
        } else if (value[i] == COMPART_KEYWORD) {
            continue;
        } else {
            num[n] = value[i];
            n++;
        }
    }

    l_ret = send_msg(buff_tx, data_len);

    return l_ret;
}

/*
 * 函 数 名  : update_device_cali_count
 * 功能描述  : 使用WRITEM命令更新device的校准次数，首次上电时为全0
 * 返 回 值  : -1表示失败，0表示成功
 *             recv_expect_result
 */
STATIC int32_t update_device_cali_count(uint8_t *key, uint8_t *value)
{
    int32_t l_ret;
    uint32_t len, value_len;
    uint32_t number = 0;
    uint8_t *addr = NULL;
    uint8_t buff_tx[SEND_BUF_LEN];

    /* 重新组合Value字符串，入参Value只是一个地址，形式为"0xXXXXX" */
    /* 组合以后的形式为"数据宽度,要写的地址,要写的值"---"4,0xXXXX,value" */
    len = 0;
    memset_s(buff_tx, SEND_BUF_LEN, 0, SEND_BUF_LEN);

    /* buff_tx="" */
    buff_tx[len] = '4';
    len++;
    buff_tx[len] = ',';
    len++;

    /* buff_tx="4," */
    value_len = (uint32_t)os_str_len(value);
    addr = delete_space(value, &value_len);
    if (addr == NULL) {
        ps_print_err("addr is NULL, value[%s] value_len[%d]", value, value_len);
        return -EFAIL;
    }
    l_ret = memcpy_s(&buff_tx[len], sizeof(buff_tx) - len, addr, value_len);
    if (l_ret != EOK) {
        ps_print_err("buff_tx not enough\n");
        return -EFAIL;
    }
    len += value_len;
    buff_tx[len] = ',';
    len++;

    /* buff_tx="4,0xXXX," */
    l_ret = get_cali_count(&number);
    l_ret += num_to_string(&buff_tx[len], number);

    /* 此时buff_tx="4,0xXXX,value" */
    /* 使用WMEM_CMD_KEYWORD命令向device发送校准次数 */
    l_ret += number_type_cmd_send(WMEM_CMD_KEYWORD, buff_tx);
    if (l_ret < 0) {
        ps_print_err("send key=[%s],value=[%s] fail\n", key, buff_tx);
        return l_ret;
    }

    l_ret = recv_expect_result(MSG_FROM_DEV_WRITEM_OK);
    if (l_ret < 0) {
        ps_print_err("recv expect result fail!\n");
        return l_ret;
    }

    return SUCC;
}

/*
 * 函 数 名  : get_cali_data
 * 功能描述  : 获取加载bfgx或者dcxo的校准数据的buf
 * 输入参数  : cmd_name   : 校准命令
 * 返 回 值  : len：表示buf长度，len = -1表示失败，
 */
STATIC int32_t get_cali_data(uint8_t *cmd_name, uint32_t fw_addr)
{
    int32_t l_ret;
    uint32_t len;
    uint8_t *dcxo_buffer_addr = NULL;
    uint8_t *plat_cust_addr = NULL;

    if (!os_str_cmp(cmd_name, CALI_BFGX_DATA_CMD_KEYWORD)) {
        l_ret = get_bfgx_cali_data(g_firmware_down_buf, g_firmware_down_buf_len, fw_addr);
        if (l_ret < 0) {
            ps_print_err("get bfgx cali data failed");
            return -EFAIL;
        }
        len = (uint32_t)sizeof(bfgx_cali_data_stru);
    } else if (!os_str_cmp(cmd_name, CALI_DCXO_DATA_CMD_KEYWORD)) {
        dcxo_buffer_addr = (uint8_t *)get_dcxo_data_buf_addr();
        if (dcxo_buffer_addr == NULL) {
            ps_print_err("[dcxo] buffer alloc error");
            return -EFAIL;
        }
        l_ret = memcpy_s(g_firmware_down_buf, g_firmware_down_buf_len, dcxo_buffer_addr, DCXO_DEVICE_MAX_BUF_LEN);
        if (l_ret != EOK) {
            ps_print_err("data buff not enough\n");
            return -EFAIL;
        }
        len = DCXO_DEVICE_MAX_BUF_LEN;
    } else if (!os_str_cmp(cmd_name, PLAT_CUST_CMD_KEYWORD)) {
        plat_cust_addr = (uint8_t *)get_plat_cust_addr();
        if (plat_cust_addr == NULL) {
            ps_print_err("plat cust buffer alloc error");
            return -EFAIL;
        }
        l_ret = memcpy_s(g_firmware_down_buf, g_firmware_down_buf_len, plat_cust_addr, get_plat_cust_len());
        if (l_ret != EOK) {
            ps_print_err("data buff not enough\n");
            return -EFAIL;
        }
        len = get_plat_cust_len();
    } else {
        return -EFAIL;
    }
    return len;
}

/*
 * 函 数 名  : download_cali_data
 * 功能描述  : 使用files命令加载bfgx或者dcxo的校准数据
 * 返 回 值  : -1表示失败，0表示成功
 *             recv_expect_result
 */
STATIC int32_t download_cali_data(uint8_t *key, uint8_t *value, uint8_t *cmd_name)
{
    int32_t l_ret;
    int32_t len;
    uint32_t value_len;
    uint8_t *addr = NULL;
    uint32_t fw_addr;
    uint8_t buff_tx[SEND_BUF_LEN];

    /* 参考函数：parse_file_cmd，使用 simple_strtoul，自动解析数字 */
    value_len = (uint32_t)os_str_len(value);
    addr = delete_space(value, &value_len);
    if (addr == NULL) {
        ps_print_err("addr is NULL, value[%s] value_len[%d]", value, value_len);
        return -EFAIL;
    }
    /* 0: 由系统函数自动推断16进制 */
    fw_addr = simple_strtoul(addr, NULL, 0);

    /* 获取bfgx或者dcxo校准数据 */
    len = get_cali_data(cmd_name, fw_addr);
    if (len < 0) {
        ps_print_err("get cali data failed, len=%d\n", len);
        return -EFAIL;
    }

    l_ret = build_file_cmd(buff_tx, sizeof(buff_tx), fw_addr, len);
    if (l_ret < 0) {
        ps_print_err("log str format err line[%d]\n", __LINE__);
        return -EFAIL;
    }

    /* 组合命令， buff_tx="FILES 1 0xXXXX "， 或 buff_tx="FILES 1 0xXXXX 0xXXXX" */
    ps_print_info("cali data cmd is [%s]\n", buff_tx);

    /* 发送地址 */
    l_ret = msg_send_and_recv_except(buff_tx, os_str_len(buff_tx), MSG_FROM_DEV_READY_OK);
    if (l_ret < 0) {
        ps_print_err("SEND [%s] addr error\n", key);
        return -EFAIL;
    }

    /* Wait at least 5 ms */
    oal_usleep_range(FILE_CMD_WAIT_TIME_MIN, FILE_CMD_WAIT_TIME_MAX);

    /* 发送bfgx或者dcxo校准数据 */
    l_ret = msg_send_and_recv_except(g_firmware_down_buf, len, MSG_FROM_DEV_FILES_OK);
    if (l_ret < 0) {
        ps_print_err("send cali data fail\n");
        return -EFAIL;
    }

    return SUCC;
}

/*
 * 函 数 名  : parse_file_cmd
 * 功能描述  : 解析file命令参数
 * 输入参数  : string   : file命令的参数
 *             addr     : 发送的数据地址
 *             file_path: 发送文件的路径
 * 返 回 值  : -1表示失败，0表示成功
 */
int32_t parse_file_cmd(uint8_t *string, unsigned long *addr, char **file_path)
{
    uint8_t *tmp = NULL;
    int32_t count = 0;
    char *after = NULL;

    if (string == NULL || addr == NULL || file_path == NULL) {
        ps_print_err("param is error!\n");
        return -EFAIL;
    }

    /* 获得发送的文件的个数，此处必须为1，string字符串的格式必须是"1,0xXXXXX,file_path" */
    tmp = string;
    while (*tmp == COMPART_KEYWORD) {
        tmp++;
    }
    string_to_num(tmp, &count);
    if (count != FILE_COUNT_PER_SEND) {
        ps_print_err("the count of send file must be 1, count = [%d]\n", count);
        return -EFAIL;
    }

    /* 让tmp指向地址的首字母 */
    tmp = os_str_chr(string, ',');
    if (tmp == NULL) {
        ps_print_err("param string is err!\n");
        return -EFAIL;
    } else {
        tmp++;
        while (*tmp == COMPART_KEYWORD) {
            tmp++;
        }
    }

    *addr = simple_strtoul(tmp, &after, 16); /* 将字符串转换成16进制数 */

    ps_print_dbg("file to send addr:[0x%lx]\n", *addr);

    /* "1,0xXXXX,file_path"
     *         ^
     *       after
     */
    while (*after == COMPART_KEYWORD) {
        after++;
    }
    /* 跳过','字符 */
    after++;
    while (*after == COMPART_KEYWORD) {
        after++;
    }

    ps_print_dbg("after:[%s]\n", after);

    *file_path = after;

    return SUCC;
}

STATIC uint32_t get_read_mem_cmd_resopne_size(const char *cmd_value)
{
    uint32_t size = 0;

    /* cmd_value : "address, len" */
    const char *flag = os_str_chr(cmd_value, ',');
    if (flag == NULL) {
        ps_print_err("RECV LEN ERROR..\n");
        return 0;
    }
    flag++;
    ps_print_dbg("recv len [%s]\n", flag);
    while (*flag == COMPART_KEYWORD) {
        flag++;
    }
    string_to_num(flag, &size);

    return size;
}

STATIC int32_t alloc_sdio_dump_rx_buf(uint8_t **rx_buf, uint32_t *buf_len)
{
    uint8_t *data_buf = NULL;
    /* 导内存先考虑成功率, 页大小对齐的内存容易申请成功。 */
    uint32_t sdio_transfer_limit = hcc_get_max_trans_size(hcc_get_handler(HCC_EP_WIFI_DEV));
    sdio_transfer_limit = oal_min(PAGE_SIZE, sdio_transfer_limit);
    if (sdio_transfer_limit == 0) {
        ps_print_err("sdio_transfer_limit is zero\n");
        return -EFAIL;
    }

    do {
        ps_print_info("try to malloc sdio mem read buf len is [%d]\n", sdio_transfer_limit);
        data_buf = (uint8_t *)os_kmalloc_gfp(sdio_transfer_limit);
        if (data_buf == NULL) {
            ps_print_warning("malloc mem len [%d] fail, continue to try in a smaller size\n", sdio_transfer_limit);
            sdio_transfer_limit = sdio_transfer_limit >> 1;
        }
    } while ((data_buf == NULL) && (sdio_transfer_limit >= MIN_FIRMWARE_FILE_TX_BUF_LEN));

    if (data_buf == NULL) {
        ps_print_err("data_buf KMALLOC failed\n");
        return -ENOMEM;
    }

    *rx_buf = data_buf;
    *buf_len = sdio_transfer_limit;

    return SUCC;
}

int32_t sdio_read_mem(const char *key, const char *value, bool is_wifi)
{
    int32_t l_ret;
    int32_t size;
    uint32_t readlen;
    int32_t retry = 3;
    os_kernel_file *fp = NULL;
    uint8_t *data_buf = NULL;
    uint32_t sdio_transfer_limit;

    size = (int32_t)get_read_mem_cmd_resopne_size(value);
    if (size == 0) {
        return -EFAIL;
    }

    if (alloc_sdio_dump_rx_buf(&data_buf, &sdio_transfer_limit) != SUCC) {
        return -EFAIL;
    }

    fp = open_file_to_readm(is_wifi == true ? WIFI_DUMP_PATH "/readm_wifi" : WIFI_DUMP_PATH "/readm_bfgx");
    if (IS_ERR(fp)) {
        ps_print_err("create file error,fp = 0x%p\n", fp);
        os_mem_kfree(data_buf);
        return SUCC;
    }

    l_ret = number_type_cmd_send(key, value);
    if (l_ret < 0) {
        ps_print_err("send %s,%s fail \n", key, value);
        oal_file_close(fp);
        os_mem_kfree(data_buf);
        return l_ret;
    }

    ps_print_dbg("recv len [%d]\n", size);
    while (size > 0) {
        readlen = min((uint32_t)size, sdio_transfer_limit);
        l_ret = recv_device_mem(fp, data_buf, readlen);
        if (l_ret > 0) {
            size -= l_ret;
        } else {
            ps_print_err("read error retry:%d\n", retry);
            --retry;
            if (!retry) {
                ps_print_err("retry fail\n");
                break;
            }
        }
    }

    oal_file_close(fp);
    os_mem_kfree(data_buf);

    return l_ret;
}

/*
 * 函 数 名  : exec_number_wmem_cmd
 * 功能描述  : 写w mem的内存地址
 * 返 回 值  : 0表示成功，其他值表示失败
 */
STATIC int32_t exec_number_wmem_cmd(uint8_t *key, uint8_t *value)
{
    int32_t ret;

    if ((os_str_str((char *)value, (char *)STR_REG_PMU_CLK_REQ) != NULL) && pmu_clk_request_enable()) {
        ps_print_info("mpxx PMU clk request\n");
        return SUCC;
    }

    ret = number_type_cmd_send(key, value);
    if (ret < 0) {
        ps_print_err("send key=[%s],value=[%s] fail\n", key, value);
        return ret;
    }

    ret = recv_expect_result(MSG_FROM_DEV_WRITEM_OK);
    if (ret < 0) {
        ps_print_err("recv expect result fail!\n");
        return ret;
    }

    return SUCC;
}

/*
 * 函 数 名  : exec_number_sleep_cmd
 * 功能描述  : host sleep延时
 * 返 回 值  : 0表示成功，其他值表示失败
 */
STATIC int32_t exec_number_sleep_cmd(uint8_t *key, uint8_t *value)
{
    int32_t delay_ms;

    delay_ms = simple_strtol(value, NULL, 10);  /* 将字符串转换成10进制数 */
    ps_print_info("firmware download delay %d ms\n", delay_ms);
    // 最大等待时间5s，防止cfg配置时间太长导致加载超时
    if (delay_ms > 0 && delay_ms < 5000) { /* 延时的时间在0到5000ms之间 */
        msleep(delay_ms);
    } else {
        msleep(5); /* delay 5ms */
    }

    return SUCC;
}

/*
 * 函 数 名  : exec_number_cali_count_cmd
 * 功能描述  : 加载校准次数到device
 * 返 回 值  : 0表示成功，其他值表示失败
 */
STATIC int32_t exec_number_cali_count_cmd(uint8_t *key, uint8_t *value)
{
    int32_t ret;

    ret = update_device_cali_count(key, value);
    if (ret < 0) {
        ps_print_err("update device cali count fail\n");
    }
    return ret;
}

/*
 * 函 数 名  : exec_number_cali_bfgx_data
 * 功能描述  : 加载bfgx校准数据
 * 返 回 值  : 0表示成功，其他值表示失败
 */
STATIC int32_t exec_number_cali_bfgx_data(uint8_t *key, uint8_t *value)
{
    int32_t ret;
#ifndef  _PRE_WINDOWS_SUPPORT
    if (ps_core_ir_only_mode() == OAL_TRUE) {
        ps_print_info("ir only pass the download cali data cmd\n");
        return SUCC;
    }
#endif
    /* 加载BFGX的校准数据 */
    bt_txbf_get_wl_cali_data();
    ret = download_cali_data(FILES_CMD_KEYWORD, value, key);
    if (ret < 0) {
        ps_print_err("download bfgx cali data fail\n");
    }

    return ret;
}

/*
 * 函 数 名  : exec_number_cali_bfgx_data
 * 功能描述  : 加载共时钟相关的数据
 * 返 回 值  : 0表示成功，其他值表示失败
 */
STATIC int32_t exec_number_cali_dcxo_data(uint8_t *key, uint8_t *value)
{
    int32_t ret;
    ps_print_dbg("download dcxo cali data begin\n");
    /* 发送TCXO 的校准参数到devcie */
    ret = download_cali_data(FILES_CMD_KEYWORD, value, key);
    if (ret < 0) {
        ps_print_err("download dcxo cali data fail\n");
    }

    return ret;
}

/*
 * 函 数 名  : exec_number_plat_cust_data
 * 功能描述  : 加载plat cust相关的数据
 * 返 回 值  : 0表示成功，其他值表示失败
 */
STATIC int32_t exec_number_plat_cust_data(uint8_t *key, uint8_t *value)
{
    int32_t ret;
    ps_print_dbg("download plat cust data begin\n");
    /* 发送plat cust到devcie */
    ret = download_cali_data(FILES_CMD_KEYWORD, value, key);
    if (ret < 0) {
        ps_print_err("download plat cust data fail\n");
    }

    return ret;
}

/*
 * 函 数 名  : exec_number_jump_cmd
 * 功能描述  : 执行device指定地址的函数
 * 返 回 值  : 0表示成功，其他值表示失败
 */
STATIC int32_t exec_number_jump_cmd(uint8_t *key, uint8_t *value)
{
    int32_t ret;
    ret = number_type_cmd_send(key, value);
    if (ret < 0) {
        ps_print_err("send key=[%s],value=[%s] fail\n", key, value);
        return ret;
    }

    /* 100000ms timeout */
    ret = recv_expect_result_timeout(MSG_FROM_DEV_JUMP_OK, READ_MEG_JUMP_TIMEOUT);
    if (ret >= 0) {
        ps_print_info("JUMP success!\n");
    } else {
        ps_print_err("CMD JUMP timeout! l_ret=%d\n", ret);
    }

    return ret;
}

/*
 * 函 数 名  : exec_number_rmem_cmd
 * 功能描述  : 读device的内存
 * 返 回 值  : 0表示成功，其他值表示失败
 */
STATIC int32_t exec_number_rmem_cmd(uint8_t *key, uint8_t *value)
{
    return sdio_read_mem(key, value, true);
}

STATIC int32_t exec_baudrate_cmd(uint8_t *key, uint8_t *value)
{
    ps_print_info("skip baudrate cmd!\n");
    return SUCC;
}

/*
 * 函 数 名  : exec_number_type_cmd
 * 功能描述  : 执行number类型的命令
 * 输入参数  : Key  : 命令的关键字
 *             Value: 命令的参数
 * 返 回 值  : -1表示失败，0表示成功
 */
STATIC int32_t exec_number_type_cmd(uint8_t *key, uint8_t *value)
{
    int32_t ret = -EFAIL;
    int32_t loop;

    if (!os_mem_cmp(key, VER_CMD_KEYWORD, os_str_len(VER_CMD_KEYWORD))) {
        ret = check_version();
        if (ret < 0) {
            ps_print_err("check version FAIL [%d]\n", ret);
            return -EFAIL;
        }
    }

    for (loop = 0; loop < oal_array_size(g_number_type); loop++) {
        if (!os_str_cmp(key, g_number_type[loop].key)) {
            ret = (g_number_type[loop].cmd_exec)(key, value);
            break;
        }
    }

    return ret;
}

/*
 * 函 数 名  : exec_quit_type_cmd
 * 功能描述  : 执行quit类型的命令
 * 返 回 值  : -1表示失败，0表示成功
 */
STATIC int32_t exec_quit_type_cmd(void)
{
    int32_t ret;
    int32_t l_len;
    uint8_t buf[QUIT_TYPE_CMD_MAX_BUFF_SIZE];

    memset_s(buf, sizeof(buf), 0, sizeof(buf));

    ret = memcpy_s(buf, sizeof(buf), (uint8_t *)QUIT_CMD_KEYWORD, os_str_len(QUIT_CMD_KEYWORD));
    if (ret != EOK) {
        ps_print_err("buf not enough\n");
        return -EFAIL;
    }
    l_len = (int32_t)os_str_len(QUIT_CMD_KEYWORD);

    buf[l_len] = COMPART_KEYWORD;
    l_len++;

    ret = msg_send_and_recv_except(buf, l_len, MSG_FROM_DEV_QUIT_OK);

    return ret;
}

STATIC int32_t build_file_cmd(uint8_t *buff, uint32_t len, unsigned long addr_send, uint32_t data_len)
{
    int32_t ret;
    hcc_bus *bus = hcc_get_current_mpxx_bus();

    if (bus == NULL) {
        ps_print_err("firmware curr bus is null\n");
        return -EFAIL;
    }

#ifdef _PRE_PLAT_FEATURE_USB
    if (bus->bus_type == HCC_BUS_USB) {
        /* 差异原因：USB场景下需要获取发送data的长度，用于下一次传输中接收数据 */
        ret = snprintf_s(buff, len, len - 1, "%s%c%d%c0x%lx%c0x%x%c", FILES_CMD_KEYWORD, COMPART_KEYWORD,
                         FILE_COUNT_PER_SEND, COMPART_KEYWORD, addr_send, COMPART_KEYWORD, data_len, COMPART_KEYWORD);
    } else {
        ret = snprintf_s(buff, len, len - 1, "%s%c%d%c0x%lx%c", FILES_CMD_KEYWORD, COMPART_KEYWORD, FILE_COUNT_PER_SEND,
                         COMPART_KEYWORD, addr_send, COMPART_KEYWORD);
    }
#else
    ret = snprintf_s(buff, len, len - 1, "%s%c%d%c0x%lx%c", FILES_CMD_KEYWORD, COMPART_KEYWORD, FILE_COUNT_PER_SEND,
        COMPART_KEYWORD, addr_send, COMPART_KEYWORD);
#endif
    return ret;
}

STATIC int32_t read_file_context_and_send(os_kernel_file *fp,
                                          uint32_t file_len, unsigned long addr)
{
    unsigned long addr_send;
    uint32_t per_send_len;
    uint32_t send_count;
    int32_t rdlen;
    int32_t ret;
    uint32_t i;
    uint32_t offset = 0;
    uint8_t buff_tx[SEND_BUF_LEN] = {0};

    // g_firmware_down_buf_len在调用前保证为大于0
    per_send_len = (g_firmware_down_buf_len > file_len) ? file_len : g_firmware_down_buf_len;
    send_count = (file_len + per_send_len - 1) / per_send_len;

    for (i = 0; i < send_count; i++) {
        rdlen = oal_file_read_ext(fp, fp->f_pos, g_firmware_down_buf, per_send_len);
        if (rdlen > 0) {
            ps_print_dbg("len of kernel_read is [%d], i=%d\n", rdlen, i);
            fp->f_pos += rdlen;
        } else {
            ps_print_err("len of kernel_read is error! ret=[%d], i=%d\n", rdlen, i);
            return (rdlen < 0) ? rdlen : -EFAIL;
        }

        addr_send = addr + offset;
        ps_print_dbg("send addr is [0x%lx], i=%d\n", addr_send, i);

        ret = build_file_cmd(buff_tx, sizeof(buff_tx), addr_send, rdlen);
        if (ret < 0) {
            ps_print_err("log str format err line[%d]\n", __LINE__);
            break;
        }
        /* 发送地址 */
        ps_print_dbg("send file addr cmd is [%s]\n", buff_tx);
        ret = msg_send_and_recv_except(buff_tx, os_str_len(buff_tx), MSG_FROM_DEV_READY_OK);
        if (ret < 0) {
            ps_print_err("SEND [%s] error\n", buff_tx);
            return -EFAIL;
        }

        /* Wait at least 5 ms */
        oal_usleep_range(FILE_CMD_WAIT_TIME_MIN, FILE_CMD_WAIT_TIME_MAX);

        /* 发送文件内容 */
        ret = msg_send_and_recv_except(g_firmware_down_buf, rdlen, MSG_FROM_DEV_FILES_OK);
        if (ret < 0) {
            ps_print_err(" sdio send data fail\n");
            return -EFAIL;
        }
        offset += rdlen;
    }

    /* 发送的长度要和文件的长度一致 */
    if (offset != file_len) {
        ps_print_err("file send len is err! send len is [%d], file len is [%d]\n", offset, file_len);
        return -EFAIL;
    }

    return SUCC;
}

STATIC int32_t firmare_send_by_file_name(uint8_t *file_name, unsigned long chip_addr)
{
    int32_t ret;
    uint32_t file_len;
    os_kernel_file *fp = NULL;

    ret = file_open_get_len(file_name, &fp, &file_len);
    if (ret < 0) {
        return ret;
    }

    ps_print_dbg("file len is [%d]\n", file_len);

    ret = read_file_context_and_send(fp, file_len, chip_addr);
    if (ret < 0) {
        oal_file_close(fp);
        ps_print_err("firmware file send fail!\n");
        return ret;
    }

    oal_file_close(fp);

    return SUCC;
}

/*
 * 函 数 名  : exec_file_type_cmd
 * 功能描述  : 执行file类型的命令
 * 输入参数  : Key  : 命令的关键字
 *             Value: 命令的参数
 * 返 回 值  : -1表示失败，0表示成功
 */
STATIC int32_t exec_file_type_cmd(uint8_t *key, uint8_t *value)
{
    unsigned long frw_addr;
    char *frw_name = NULL;
    char *frw_abs_path = NULL;
    uint32_t path_len;
    int32_t ret;

#ifndef  _PRE_WINDOWS_SUPPORT
    if (ps_core_ir_only_mode() == OAL_TRUE) {
        ps_print_info("ir only pass the download file cmd\n");
        return SUCC;
    }
#endif
    ret = parse_file_cmd(value, &frw_addr, &frw_name);
    if (ret < 0) {
        ps_print_err("parse file cmd fail!\n");
        return ret;
    }

    ps_print_info("download firmware %s:addr:0x%x\n", frw_name, (uint32_t)frw_addr);

    // 如果相对路径，需要加上路径前缀
    if (g_cfg_info.file_info.pre_directory != NULL) {
        path_len = (uint32_t)(OAL_STRLEN(g_cfg_info.file_info.pre_directory) +
                   sizeof(char) + OAL_STRLEN(frw_name) + 1);
        frw_abs_path = (uint8_t *)os_kzalloc_gfp(path_len);
        if (frw_abs_path == NULL) {
            return -ENOMEM;
        }
        ret = sprintf_s(frw_abs_path, path_len, "%s%c%s", g_cfg_info.file_info.pre_directory,
                        '/', frw_name);
        ps_print_info("firmware absolute path %s, ret = %d", frw_abs_path, ret);
        ret = firmare_send_by_file_name(frw_abs_path, frw_addr);
        os_mem_kfree(frw_abs_path);

        return ret;
    }

    return firmare_send_by_file_name(frw_name, frw_addr);
}

/*
 * 函 数 名  : execute_download_cmd
 * 功能描述  : 执行firmware download的命令
 * 输入参数  : cmd_type: 加载命令的类型
 *             cmd_name: 命令的关键字
 *             cmd_para: 命令的参数
 * 返 回 值  : -1表示失败，非负数表示成功
 */
STATIC int32_t execute_download_cmd(int32_t cmd_type, uint8_t *cmd_name, uint8_t *cmd_para)
{
    int32_t ret;

    switch (cmd_type) {
        case FILE_TYPE_CMD:
            ps_print_dbg(" command type FILE_TYPE_CMD\n");
            ret = exec_file_type_cmd(cmd_name, cmd_para);
            break;
        case NUM_TYPE_CMD:
            ps_print_dbg(" command type NUM_TYPE_CMD\n");
            ret = exec_number_type_cmd(cmd_name, cmd_para);
            break;
        case QUIT_TYPE_CMD:
            ps_print_dbg(" command type QUIT_TYPE_CMD\n");
            ret = exec_quit_type_cmd();
            break;

        default:
            ps_print_err("command type error[%d]\n", cmd_type);
            ret = -EFAIL;
            break;
    }

    return ret;
}

/*
 * 函 数 名  : firmware_read_cfg
 * 功能描述  : 读取cfg文件的内容，放到驱动动态分配的buffer中
 * 输入参数  : cfg_patch    : cfg文件的路径
 *             buf : 保存cfg文件内容的buffer
 * 返 回 值  : 0表示成功，-1表示失败
 */
int32_t firmware_read_cfg(const char *cfg_patch, uint8_t *buf, uint32_t buf_len)
{
    os_kernel_file *fp = NULL;
    int32_t ret;
    mm_segment_t fs;

    if ((cfg_patch == NULL) || (buf == NULL)) {
        ps_print_err("para is NULL\n");
        return -EFAIL;
    }

    fs = get_fs();
    set_fs(KERNEL_DS);
    fp = filp_open(cfg_patch, O_RDONLY, 0);
    if (oal_is_err_or_null(fp)) {
        ps_print_err("open file %s fail, fp=%p, error %ld\n", cfg_patch, fp, PTR_ERR(fp));
        set_fs(fs);
        fp = NULL;
        return -EFAIL;
    }

    memset_s(buf, buf_len, 0, buf_len);

    ret = oal_file_read_ext(fp, fp->f_pos, buf, buf_len);

    filp_close(fp, NULL);
    set_fs(fs);
    fp = NULL;

    return ret;
}


/*
 * 函 数 名  : firmware_parse_special_cmd
 * 功能描述  : 解析cfg文件中的不带"="字符的特殊命令行:
               QUIT
               SHUTDOWN_WIFI
               SHUTDOWN_BFGX
               BUCK_MODE
 * 返 回 值  : 解析到特殊命令,返回命令的类型;否则返回-1.
 */
STATIC int32_t firmware_parse_special_cmd(uint8_t *puc_cfg_buffer, uint8_t *puc_cmd_name, uint32_t cmd_name_len,
                                          uint8_t *puc_cmd_para, uint32_t cmd_para_len)
{
#define MAX_BUFF_LEN    100
#define BUCK_PARA_LEN   16

    int32_t ret;

    /* 退出命令行 */
    if (os_str_str((char *)puc_cfg_buffer, QUIT_CMD_KEYWORD) != NULL) {
        return QUIT_TYPE_CMD;
    } else if (os_str_str((char *)puc_cfg_buffer, BUCK_MODE_CMD_KEYWORD) != NULL) {
         /*
         * mp15支持内置buck和外置buck,CFG文件中仅有关键字，实际值从ini文件中获取，方便定制化。
         * ini定制化格式[buck_mode=2,0x50001850,value],符合CFG文件的WRITEM的语法,0x50001850为mp15的BUCK_CUSTOM_REG
         * 地址, value根据实际要求配置:
         * （1）   BIT[15,14]：表示是否采用外置buck
         *      2'b00:  全内置buck
         *      2'b01:  I2C控制独立外置buck
         *      2'b10:  GPIO控制独立外置buck
         *      2'b11:  host控制共享外置buck电压
         * （2）   BIT[8]：表示buck_en,再调电压..
         * （3）   BIT[7，0]: 代表不同的Buck器件型号
         */
        char buff[MAX_BUFF_LEN];
        char buck_para[BUCK_PARA_LEN];
        ps_print_info("buck_mode para,get from ini file\n");
        if (get_cust_conf_string(INI_MODU_PLAT, "buck_custom_addr", buff, sizeof(buff) - 1) == INI_FAILED) {
            ps_print_err("can't found buck_mode ini");
            return ERROR_TYPE_CMD;
        }

        oal_itoa(g_st_board_info.buck_param, buck_para, sizeof(buck_para));

        ret = strcat_s(buff, sizeof(buff), ",");
        if (ret != EOK) {
            ps_print_err("strcat_s failed!");
            return ERROR_TYPE_CMD;
        }
        ret = strcat_s(buff, sizeof(buff), buck_para);
        if (ret != EOK) {
            ps_print_err("strcat_s buck_para failed!");
            return ERROR_TYPE_CMD;
        }

        ret = memcpy_s(puc_cmd_para, cmd_para_len, buff, OAL_STRLEN(buff));
        if (ret != EOK) {
            ps_print_err("buck_mode para set error");
            return ERROR_TYPE_CMD;
        }

        ret = memcpy_s(puc_cmd_name, cmd_name_len, "WRITEM", 6); /* 需要拷贝的个数为6 */
        if (ret != EOK) {
            ps_print_err("buck_mode cmd name set error");
            return ERROR_TYPE_CMD;
        }

        return NUM_TYPE_CMD;
    }

    return ERROR_TYPE_CMD;
}

static int32_t firmare_cmd_get_type(uint8_t* handle, int32_t cmd_len, uint8_t *cmd_name, uint32_t cmd_name_len)
{
    int32_t ret;
    int32_t cmd_type;
    uint8_t *handle_temp = NULL;

    /* 判断命令类型 */
    if ((cmd_len >= os_str_len(FILE_TYPE_CMD_KEY)) &&
        !os_mem_cmp(handle, FILE_TYPE_CMD_KEY, os_str_len(FILE_TYPE_CMD_KEY))) {
        handle_temp = os_str_str(handle, FILE_TYPE_CMD_KEY);
        if (handle_temp == NULL) {
            ps_print_err("'ADDR_FILE_'is not handle child string, handle=%s", handle);
            return ERROR_TYPE_CMD;
        }
        handle = handle_temp + (uint8_t)os_str_len(FILE_TYPE_CMD_KEY);
        cmd_len = cmd_len - (int32_t)os_str_len(FILE_TYPE_CMD_KEY);
        cmd_type = FILE_TYPE_CMD;
    } else if ((cmd_len >= os_str_len(NUM_TYPE_CMD_KEY)) &&
               !os_mem_cmp(handle, NUM_TYPE_CMD_KEY, os_str_len(NUM_TYPE_CMD_KEY))) {
        handle_temp = os_str_str(handle, NUM_TYPE_CMD_KEY);
        if (handle_temp == NULL) {
            ps_print_err("'PARA_' is not handle child string, handle=%s", handle);
            return ERROR_TYPE_CMD;
        }
        handle = handle_temp + (uint8_t)os_str_len(NUM_TYPE_CMD_KEY);
        cmd_len = cmd_len - (int32_t)os_str_len(NUM_TYPE_CMD_KEY);
        cmd_type = NUM_TYPE_CMD;
    } else {
        return ERROR_TYPE_CMD;
    }

    ret = memcpy_s(cmd_name, cmd_name_len, handle, cmd_len);
    if (ret != EOK) {
        ps_print_err("cmd len out range! ret = %d\n", ret);
        return ERROR_TYPE_CMD;
    }

    return  cmd_type;
}
/*
 * 函 数 名  : firmware_parse_cmd
 * 功能描述  : 解析cfg文件中的命令
 * 输入参数  : puc_cfg_buffer: 保存cfg文件内容的buffer
 *             puc_cmd_name  : 保存解析以后命令关键字的buffer
 *             puc_cmd_para  : 保存解析以后命令参数的buffer
 * 返 回 值  : 返回命令的类型
 */
int32_t firmware_parse_cmd(uint8_t *cfg_buffer, uint8_t *cmd_name, uint32_t cmd_name_len,
                           uint8_t *cmd_para, uint32_t cmd_para_len)
{
    int32_t ret;
    int32_t cmd_type;
    int32_t l_cmdlen;
    int32_t l_paralen;
    uint8_t *begin = NULL;
    uint8_t *end = NULL;
    uint8_t *link = NULL;
    uint8_t *handle = NULL;

    begin = cfg_buffer;
    if ((cfg_buffer == NULL) || (cmd_name == NULL) || (cmd_para == NULL)) {
        ps_print_err("para is NULL\n");
        return ERROR_TYPE_CMD;
    }

    /* 注释行 */
    if (cfg_buffer[0] == '@') {
        return ERROR_TYPE_CMD;
    }

    /* 错误行，或者退出命令行, 或者特殊字段,从INI文件中动态获取配置 */
    link = os_str_chr((char *)begin, '=');
    if (link == NULL) {
        return firmware_parse_special_cmd(cfg_buffer, cmd_name, cmd_name_len, cmd_para, cmd_para_len);
    }

    /* 错误行，没有结束符 */
    end = os_str_chr(link, ';');
    if (end == NULL) {
        return ERROR_TYPE_CMD;
    }

    l_cmdlen = link - begin;

    /* 删除关键字的两边空格 */
    handle = delete_space((uint8_t *)begin, &l_cmdlen);
    if (handle == NULL) {
        return ERROR_TYPE_CMD;
    }

    cmd_type = firmare_cmd_get_type(handle, l_cmdlen, cmd_name, cmd_name_len);
    if (cmd_type == ERROR_TYPE_CMD) {
        return cmd_type;
    }

    /* 删除值两边空格 */
    begin = link + 1;
    l_paralen = end - begin;

    handle = delete_space(begin, &l_paralen);
    if (handle == NULL) {
        return ERROR_TYPE_CMD;
    }
    ret = memcpy_s(cmd_para, cmd_para_len, handle, l_paralen);
    if (ret != EOK) {
        ps_print_err("para len out of range!ret = %d\n", ret);
        return ERROR_TYPE_CMD;
    }

    return cmd_type;
}

STATIC int32_t firmware_parse_cur_cmd(uint32_t index, uint32_t pos)
{
    int32_t ret;
    uint32_t cmd_para_len = 0;

    /* 获取配置版本号 */
    if (!os_mem_cmp(g_cfg_info.cmd[index][pos].cmd_name,
                    VER_CMD_KEYWORD,
                    os_str_len(VER_CMD_KEYWORD))) {
        cmd_para_len = (uint32_t)os_str_len(g_cfg_info.cmd[index][pos].cmd_para);
        ret = memcpy_s(g_cfg_info.cfg_version, sizeof(g_cfg_info.cfg_version),
                       g_cfg_info.cmd[index][pos].cmd_para, cmd_para_len);
        if (ret != EOK) {
            ps_print_dbg("cmd_para_len = %d over auc_CfgVersion length", cmd_para_len);
            return -EFAIL;
        }

        ps_print_dbg("g_CfgVersion = [%s].\n", g_cfg_info.cfg_version);
    } else if (!os_mem_cmp(g_cfg_info.cmd[index][pos].cmd_name,
                           CALI_DCXO_DATA_CMD_KEYWORD,
                           os_str_len(CALI_DCXO_DATA_CMD_KEYWORD))) {
        ret = read_dcxo_cali_data();
        if (ret < 0) {
            ps_print_err("read dcxo para from nv failed !\n");
            return -EFAIL;
        }
    }

    return SUCC;
}

/*
 * 函 数 名  : firmware_parse_cfg
 * 功能描述  : 解析cfg文件，将解析的结果保存在g_st_cfg_info全局变量中
 * 输入参数  : puc_cfg_info_buf: 保存了cfg文件内容的buffer
 *             l_buf_len       : puc_cfg_info_buf的长度
 *             ul_index        : 保存解析结果的数组索引值
 * 返 回 值  : 0表示成功，-1表示失败
 */
STATIC int32_t firmware_parse_cfg(uint8_t *puc_cfg_info_buf, int32_t l_buf_len, uint32_t ul_index)
{
    int32_t i;
    int32_t l_len;
    uint8_t *flag = NULL;
    uint8_t *begin = NULL;
    uint8_t *end = NULL;
    int32_t cmd_type;
    uint8_t cmd_name[DOWNLOAD_CMD_LEN];
    uint8_t cmd_para[DOWNLOAD_CMD_PARA_LEN];

    g_cfg_info.cmd[ul_index] = (struct cmd_type_st *)malloc_cmd_buf(puc_cfg_info_buf, ul_index);
    if (g_cfg_info.cmd[ul_index] == NULL) {
        ps_print_err(" malloc_cmd_buf fail!\n");
        return -EFAIL;
    }

    /* 解析CMD BUF */
    flag = puc_cfg_info_buf;
    l_len = l_buf_len;
    i = 0;
    while ((i < g_cfg_info.count[ul_index]) && (flag < &puc_cfg_info_buf[l_len])) {
        /*
         * 获取配置文件中的一行,配置文件必须是unix格式.
         * 配置文件中的某一行含有字符 @ 则认为该行为注释行
         */
        begin = flag;
        end = os_str_chr(flag, '\n');
        if (end == NULL) { /* 文件的最后一行，没有换行符 */
            ps_print_dbg("lost of new line!\n");
            end = &puc_cfg_info_buf[l_len];
        } else if (end == begin) {  /* 该行只有一个换行符 */
            ps_print_dbg("blank line\n");
            flag = end + 1;
            continue;
        }
        *end = '\0';

        ps_print_dbg("operation string is [%s]\n", begin);

        memset_s(cmd_name, DOWNLOAD_CMD_LEN, 0, DOWNLOAD_CMD_LEN);
        memset_s(cmd_para, DOWNLOAD_CMD_PARA_LEN, 0, DOWNLOAD_CMD_PARA_LEN);

        cmd_type = firmware_parse_cmd(begin, cmd_name, sizeof(cmd_name), cmd_para, sizeof(cmd_para));

        ps_print_dbg("cmd type=[%d],cmd_name=[%s],cmd_para=[%s]\n", cmd_type, cmd_name, cmd_para);

        if (cmd_type != ERROR_TYPE_CMD) { /* 正确的命令类型，增加 */
            g_cfg_info.cmd[ul_index][i].cmd_type = cmd_type;
            memcpy_s(g_cfg_info.cmd[ul_index][i].cmd_name, DOWNLOAD_CMD_LEN, cmd_name, DOWNLOAD_CMD_LEN);
            memcpy_s(g_cfg_info.cmd[ul_index][i].cmd_para, DOWNLOAD_CMD_PARA_LEN, cmd_para, DOWNLOAD_CMD_PARA_LEN);

            firmware_parse_cur_cmd(ul_index, i);
            i++;
        }
        flag = end + 1;
    }

    /* 根据实际命令个数，修改最终的命令个数 */
    g_cfg_info.count[ul_index] = i;
    ps_print_info("effective cmd count: al_count[%d] = %d\n", ul_index, g_cfg_info.count[ul_index]);

    return SUCC;
}

/*
 * 函 数 名  : firmware_get_cfg
 * 功能描述  : 读取cfg文件并解析，将解析的结果保存在g_st_cfg_info全局变量中
 * 输入参数  : puc_CfgPatch: cfg文件的路径
 *             ul_index     : 保存解析结果的数组索引值
 * 返 回 值  : 0表示成功，-1表示失败
 */
int32_t firmware_get_cfg(uint8_t *cfg_patch, uint32_t idx)
{
    uint8_t *read_cfg_buf = NULL;
    int32_t readlen;
    int32_t ret;

    if (cfg_patch == NULL) {
        ps_print_err("cfg file path is null!\n");
        return -EFAIL;
    }

    /* cfg文件限定在小于2048,如果cfg文件的大小确实大于2048，可以修改READ_CFG_BUF_LEN的值 */
    read_cfg_buf = os_kmalloc_gfp(READ_CFG_BUF_LEN);
    if (read_cfg_buf == NULL) {
        ps_print_err("kmalloc READ_CFG_BUF fail!\n");
        return -ENOMEM;
    }

    readlen = firmware_read_cfg(cfg_patch, read_cfg_buf, READ_CFG_BUF_LEN);
    if (readlen < 0) {
        ps_print_err("read cfg error!\n");
        os_mem_kfree(read_cfg_buf);
        read_cfg_buf = NULL;
        return -EFAIL;
    } else if (readlen > READ_CFG_BUF_LEN - 1) {
        /* 减1是为了确保cfg文件的长度不超过READ_CFG_BUF_LEN，因为firmware_read_cfg最多只会读取READ_CFG_BUF_LEN长度的内容 */
        ps_print_err("cfg file [%s] larger than %d\n", cfg_patch, READ_CFG_BUF_LEN);
        os_mem_kfree(read_cfg_buf);
        read_cfg_buf = NULL;
        return -EFAIL;
    } else {
        ps_print_dbg("read cfg file [%s] ok, size is [%d]\n", cfg_patch, readlen);
    }

    ret = firmware_parse_cfg(read_cfg_buf, readlen, idx);
    if (ret < 0) {
        ps_print_err("parse cfg error!\n");
    }

    os_mem_kfree(read_cfg_buf);
    read_cfg_buf = NULL;

    return ret;
}

STATIC int32_t firmware_download_buffer_alloc(hcc_bus *bus)
{
    STATIC uint64_t max_cost_time = 0;
    declare_time_cost_stru(cost);

    if (bus == NULL) {
        ps_print_err("firmware curr bus is null\n");
        return -EFAIL;
    }

    oal_get_time_cost_start(cost);
    g_firmware_down_buf = (uint8_t *)oal_memtry_alloc(oal_min(bus->cap.max_trans_size, MAX_FIRMWARE_FILE_TX_BUF_LEN),
        MIN_FIRMWARE_FILE_TX_BUF_LEN, &g_firmware_down_buf_len, HISDIO_BLOCK_SIZE);
    if (g_firmware_down_buf == NULL) {
        ps_print_err("g_pucDataBuf KMALLOC failed, min request:%u\n", MIN_FIRMWARE_FILE_TX_BUF_LEN);
        return -ENOMEM;
    }

    g_firmware_down_buf_len = OAL_ROUND_DOWN(g_firmware_down_buf_len, 8);  /* 清除低3bit，保证8字节对齐 */
    if (g_firmware_down_buf_len == 0) {
        oal_free(g_firmware_down_buf);
        ps_print_err("after round down, g_firmware_down_buf_len is 0\n");
        return -EFAIL;
    }

    oal_get_time_cost_end(cost);
    oal_calc_time_cost_sub(cost);
    if (time_cost_var_sub(cost) > max_cost_time) {
        max_cost_time = time_cost_var_sub(cost);
    }

    ps_print_info("download firmware file buf len is [%d], cost time %llu, max cost time %llu\n",
                  g_firmware_down_buf_len, time_cost_var_sub(cost), max_cost_time);
    return SUCC;
}

static void firmware_uart_rate_set(void)
{
    int32_t ret;
    uint8_t cmd_name[DOWNLOAD_CMD_LEN] = { 0 };
    uint8_t cmd_para[DOWNLOAD_CMD_PARA_LEN] = { 0 };
    uint8_t uart_rate_addr[UART_RATE_PARA_LEN] = { 0 };
    uint8_t uart_rate_value[UART_RATE_PARA_LEN] = { 0 };

    ret = get_cust_conf_string(INI_MODU_PLAT, "uart_rate_addr", uart_rate_addr, sizeof(uart_rate_addr) - 1);
    if (ret != INI_SUCC) {
        ps_print_info("can't found uart rate addr ini");
        return;
    }

    ret = get_cust_conf_string(INI_MODU_PLAT, "uart_rate_value", uart_rate_value, sizeof(uart_rate_value) - 1);
    if (ret != INI_SUCC) {
        ps_print_info("can't found uart rate value ini");
        return;
    }

    ret = strcat_s(cmd_para, sizeof(cmd_para), uart_rate_addr);
    if (ret != EOK) {
        ps_print_err("strcat_s failed!");
        return;
    }

    ret = strcat_s(cmd_para, sizeof(cmd_para), ",");
    if (ret != EOK) {
        ps_print_err("strcat_s failed!");
        return;
    }

    ret = strcat_s(cmd_para, sizeof(cmd_para), uart_rate_value);
    if (ret != EOK) {
        ps_print_err("strcat_s failed!");
        return;
    }

    ret = memcpy_s(cmd_name, DOWNLOAD_CMD_LEN, "WRITEM", 6); /* 需要拷贝的个数为6 */
    if (ret != EOK) {
        ps_print_err("uart rate cmd name set error");
        return;
    }

    ps_print_info("uart_rate_set cmd_type[%d], name[%s], para[%s]\n", NUM_TYPE_CMD, cmd_name, cmd_para);

    ret = execute_download_cmd(NUM_TYPE_CMD, cmd_name, cmd_para);
    if (ret != SUCC) {
        ps_print_err("uart_rate_set cmd_type[%d], name[%s], para[%s]\n", NUM_TYPE_CMD, cmd_name, cmd_para);
    }
}

/*
 * 函 数 名  : firmware_download_cmd
 * 功能描述  : firmware加载
 * 输入参数  : idx: 有效加载命令数组的索引
 * 返 回 值  : 0表示成功，-1表示失败
 */
int32_t firmware_download_cmd(uint32_t idx, hcc_bus *bus)
{
    int32_t ret;
    int32_t i;
    int32_t cmd_type;
    uint8_t *cmd_name = NULL;
    uint8_t *cmd_para = NULL;

    if (idx >= CFG_FILE_TOTAL) {
        ps_print_err("idx [%d] is error!\n", idx);
        return -EFAIL;
    }

    ps_print_info("start download firmware, idx = [%d]\n", idx);

    if (g_cfg_info.count[idx] == 0) {
        ps_print_err("firmware download cmd count is 0, idx = [%d]\n", idx);
        return -EFAIL;
    }

    if (firmware_download_buffer_alloc(bus) < 0) {
        return -ENOMEM;
    }

    firmware_uart_rate_set();

    for (i = 0; i < g_cfg_info.count[idx]; i++) {
        cmd_type = g_cfg_info.cmd[idx][i].cmd_type;
        cmd_name = g_cfg_info.cmd[idx][i].cmd_name;
        cmd_para = g_cfg_info.cmd[idx][i].cmd_para;

        ps_print_dbg("cmd[%d]:type[%d], name[%s], para[%s]\n", i, cmd_type, cmd_name, cmd_para);

        ps_print_dbg("firmware down start cmd[%d]:type[%d], name[%s]\n", i, cmd_type, cmd_name);

        ret = execute_download_cmd(cmd_type, cmd_name, cmd_para);
        if (ret < 0) {
            os_mem_kfree(g_firmware_down_buf);
            g_firmware_down_buf = NULL;
            ps_print_err("download firmware fail\n");

            return ret;
        }

        ps_print_dbg("firmware down finish cmd[%d]:type[%d], name[%s]\n", i, cmd_type, cmd_name);
    }

    os_mem_kfree(g_firmware_down_buf);
    g_firmware_down_buf = NULL;

    ps_print_info("finish download firmware\n");

    return SUCC;
}

STATIC int32_t print_firmware_download_cmd(uint32_t ul_index)
{
    int32_t i;
    int32_t l_cmd_type;
    uint8_t *puc_cmd_name = NULL;
    uint8_t *puc_cmd_para = NULL;
    int32_t count;

    count = g_cfg_info.count[ul_index];
    ps_print_info("[%s] download cmd, total count is [%d]\n",
                  g_cfg_info.file_info.cfg_path[ul_index], count);

    for (i = 0; i < count; i++) {
        l_cmd_type = g_cfg_info.cmd[ul_index][i].cmd_type;
        puc_cmd_name = g_cfg_info.cmd[ul_index][i].cmd_name;
        puc_cmd_para = g_cfg_info.cmd[ul_index][i].cmd_para;

        ps_print_info("cmd[%d]:type[%d], name[%s], para[%s]\n", i, l_cmd_type, puc_cmd_name, puc_cmd_para);
    }

    return 0;
}

int32_t print_cfg_file_cmd(void)
{
    int32_t i;

    for (i = 0; i < CFG_FILE_TOTAL; i++) {
        if (g_cfg_info.file_info.cfg_path[i] == NULL) {
            continue;
        }
        print_firmware_download_cmd(i);
    }

    return 0;
}

STATIC void free_cfg_file_memory(void)
{
    uint8_t i;

    if (g_cfg_info.file_info.pre_directory == NULL) {
        return;
    }

    for (i = 0; i < CFG_FILE_TOTAL; i++) {
        if (g_cfg_info.file_info.from_heap_mem[i] == OAL_TRUE) {
            os_mem_kfree(g_cfg_info.file_info.cfg_path[i]);
        }
    }
}

STATIC int32_t cfg_file_add_pre_direcotry(void)
{
    uint8_t i;
    uint32_t cfg_buf_size;
    uint8_t *path = NULL;
    int32_t ret;

    for (i = 0; i < CFG_FILE_TOTAL; i++) {
        if (g_cfg_info.file_info.cfg_path[i] == NULL) {
            continue;
        }

        cfg_buf_size = (uint32_t)((OAL_STRLEN(g_cfg_info.file_info.pre_directory) +
                       sizeof(char) +
                       OAL_STRLEN(g_cfg_info.file_info.cfg_path[i]) + 1));
        path = (uint8_t *)os_kzalloc_gfp(cfg_buf_size);
        if (path == NULL) {
            free_cfg_file_memory();
            return -ENOMEM;
        }
        g_cfg_info.file_info.from_heap_mem[i] = OAL_TRUE;
        ret = sprintf_s(path, cfg_buf_size, "%s%c%s", g_cfg_info.file_info.pre_directory,
                        '/', g_cfg_info.file_info.cfg_path[i]);
        g_cfg_info.file_info.cfg_path[i] = path;
        ps_print_dbg("cfg absolute path is %d, %s, ret %d", i, g_cfg_info.file_info.cfg_path[i], ret);
    }

    return SUCC;
}

/*
 * 函 数 名  : firmware_cfg_path_init
 * 功能描述  : 获取firmware的cfg文件路径
 * 返 回 值  : 0表示成功，-1表示失败
 */
STATIC int32_t firmware_cfg_path_init(void)
{
    uint8_t i;

    for (i = 0; i < oal_array_size(g_chip_firmware_cfg); i++) {
        if (get_mpxx_subchip_type() != g_chip_firmware_cfg[i].chip_nr ||
            g_chip_firmware_cfg[i].is_asic != mpxx_is_asic()) {
            continue;
        }

        if (g_chip_firmware_cfg[i].cfg_path == NULL) {
            continue;
        }

        g_cfg_info.file_info.cfg_path = g_chip_firmware_cfg[i].cfg_path;
        g_cfg_info.file_info.pre_directory = g_chip_firmware_cfg[i].pre_directory;
        break;
    }

    if (i == oal_array_size(g_chip_firmware_cfg)) {
        ps_print_err("chip [%d], is_asic [%d] need config", get_mpxx_subchip_type(),
                     mpxx_is_asic());
        return -EFAIL;
    }

    if (g_cfg_info.file_info.pre_directory != NULL &&
        cfg_file_add_pre_direcotry() != SUCC) {
        ps_print_err("cfg firmware add pre path %s fail",
                     g_chip_firmware_cfg[i].pre_directory);
        return -EFAIL;
    }

    ps_print_info("firmware info chip [%d], type [%d]",  get_mpxx_subchip_type(),
                  mpxx_is_asic());
    return SUCC;
}

/*
 * 函 数 名  : firmware_cfg_init
 * 功能描述  : firmware加载的cfg文件初始化，读取并解析cfg文件，将解析的结果保存在
 *             cfg_info全局变量中
 */
int32_t firmware_cfg_init(void)
{
    int32_t ret;
    uint32_t i;
    uint8_t **cfg_path = NULL;

    ret = firmware_cfg_path_init();
    if (ret != SUCC) {
        ps_print_err("firmware cfg path init fail!");
        return -EFAIL;
    }

    cfg_path = g_cfg_info.file_info.cfg_path;
    /* 解析cfg文件 */
    for (i = 0; i < CFG_FILE_TOTAL; i++) {
        if (cfg_path[i] == NULL) {
            continue;
        }
        ret = firmware_get_cfg(cfg_path[i], i);
        if (ret < 0) {
            if (i == RAM_REG_TEST_CFG || i == GT_CFG) {
                ps_print_warning("ram_reg_test_cfg maybe not exist, please check\n");
                continue;
            }

            ps_print_err("get cfg file [%s] fail\n", cfg_path[i]);
            goto cfg_file_init_fail;
        }
    }

    return SUCC;

cfg_file_init_fail:
    firmware_cfg_clear();

    return -EFAIL;
}

/*
 * 函 数 名  : firmware_cfg_clear
 * 功能描述  : 释放firmware_cfg_init时申请的内存
 * 返 回 值  : 总是返回0，表示成功
 */
void firmware_cfg_clear(void)
{
    int32_t i;

    for (i = 0; i < CFG_FILE_TOTAL; i++) {
        g_cfg_info.count[i] = 0;
        if (g_cfg_info.cmd[i] != NULL) {
            os_mem_kfree(g_cfg_info.cmd[i]);
            g_cfg_info.cmd[i] = NULL;
        }
    }

    free_cfg_file_memory();
}
