/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved. \n
 *
 * Description: TIoT firmware load module. \n
 * Author: @CompanyNameTag \n
 * History: \n
 * 2023-03-30, Create file. \n
 */
#include "tiot_firmware.h"
#include <stdbool.h>
#include "tiot_firmware_utils.h"
#include "securec.h"
#include "tiot_board_log.h"

/* 临时，将来通过device_info传入 */
#ifdef CONFIG_FILE_BY_ARRAY
#define TIOT_FW_FILE_ARRAY_CFG_NAME   "cfg"
#endif

#define FW_FILE_READ_BUF_LEN       CONFIG_FIRMWARE_FILE_READ_BUF_LEN

#define TIOT_FW_LOAD_FINISH    0
#define TIOT_FW_LOAD_NEXT      1

#define TIOT_FW_LOAD_RETRY_CNT 3

#define TIOT_FW_EXT_VERSION1 0x1

static inline uint32_t little_endian_to_u32(const uint8_t *data)
{
    uint32_t val;
    val = data[0x3];
    val <<= 0x8;
    val |= data[0x2];
    val <<= 0x8;
    val |= data[0x1];
    val <<= 0x8;
    val |= data[0x0];
    return val;
}

int32_t tiot_firmware_init(tiot_fw *fw, tiot_board_fw_info *board_info, const tiot_fw_ops *ops)
{
    if ((fw == NULL) || (board_info == NULL) || (ops == NULL) || (ops->ext_cmd_handle == NULL)) {
        tiot_print_dbg("[TIoT][fw]param invalid!\n");
        return -1;
    }

#ifdef CONFIG_FILE_BY_ARRAY
    /* 文件数组设为默认名, 脚本生成文件数组时也约定使用此名称. */
    fw->info.board_info.cfg_path = TIOT_FW_FILE_ARRAY_CFG_NAME;
#else
    /* 非文件数组必须输入config文件路径. */
    if (board_info->cfg_path == NULL) {
        tiot_print_err("[TIoT][fw]fw cfg file path is NULL!\n");
        return -1;
    }
    /* 复制config文件路径. */
    (void)memcpy_s(&fw->info.board_info, sizeof(tiot_board_fw_info), board_info, sizeof(tiot_board_fw_info));
#endif
    tiot_print_info("[TIoT][fw]fw cfg file path %s\n", fw->info.board_info.cfg_path);
    fw->ops = ops;
    fw->fw_file = NULL;
    return 0;
}

static int32_t tiot_firmware_send_cmd_retry(tiot_fw *fw, uint16_t ext_cmd, uint8_t *cmd, uint16_t cmd_len)
{
    uint8_t retry_cnt;
    int32_t ret = -1;
    for (retry_cnt = 0; retry_cnt < TIOT_FW_LOAD_RETRY_CNT; retry_cnt++) {
        ret = tiot_firmware_write(fw, cmd, (uint32_t)cmd_len);
        if (ret != cmd_len) {
            tiot_print_err("[TIoT:fw]cmd content send fail\r\n");
            return -1;
        }
        ret = fw->ops->ext_cmd_handle(fw, ext_cmd, cmd, cmd_len);
        // TIOT_FW_LOAD_RET_OK或TIOT_FW_LOAD_RET_ERR 退出循环
        // TIOT_FW_LOAD_RET_RETRY 重发
        if (ret != TIOT_FW_LOAD_RET_RETRY) {
            break;
        }
        tiot_print_warning("[TIoT:fw]single cmd retry %d\r\n", retry_cnt);
    }
    if (ret == TIOT_FW_LOAD_RET_QUIT) {
        tiot_print_info("[TIoT:fw]version matches, no need to upgrade firmware.\r\n");
    } else if ((ret != TIOT_FW_LOAD_RET_OK) || (retry_cnt == TIOT_FW_LOAD_RETRY_CNT)) {
        tiot_print_err("[TIoT:fw]cmd handle retry fail.\r\n");
        ret = -1;
    }
    return ret;
}

static int32_t tiot_firmware_load_cmds(tiot_fw *fw)
{
    int32_t ret;
    uint16_t cmd_len, ext_cmd;
#ifdef CONFIG_BOARD_DYNAMIC_ALLOC
    char *read_buff = (char *)osal_kmalloc(FW_FILE_READ_BUF_LEN, OSAL_GFP_KERNEL);
    if (read_buff == NULL) {
        return -1;
    }
    memset_s(read_buff, FW_FILE_READ_BUF_LEN, 0, FW_FILE_READ_BUF_LEN);
#else
    char read_buff[FW_FILE_READ_BUF_LEN] = { 0 };
#endif

    do {
        ret = tiot_file_read(fw->fw_file, (char *)&cmd_len, sizeof(uint16_t), fw->cur_offset);
        // 处于文件末尾，读文件结束
        if (ret == 0) {
            break;
        }
        if (ret != sizeof(uint16_t)) {
            tiot_print_err("[TIoT:fw]cmd len read fail\r\n");
            ret = -1;
            break;
        }
        ext_cmd = cmd_len >> 12;   // 拓展命令高4位, 右移12位
        cmd_len = cmd_len & 0xFFF; // 长度为低12位
        if (cmd_len > FW_FILE_READ_BUF_LEN) {
            tiot_print_err("[TIoT:fw]cmd len(%d) larger than buff len(%d)\r\n", cmd_len, FW_FILE_READ_BUF_LEN);
            ret = -1;
            break;
        }
        // 更新offset
        fw->cur_offset += sizeof(uint16_t);
        ret = tiot_file_read(fw->fw_file, read_buff, cmd_len, fw->cur_offset);
        if (ret != cmd_len) {
            tiot_print_err("[TIoT:fw]cmd content read fail\r\n");
            ret = -1;
            break;
        }
        fw->cur_offset += cmd_len;
        ret = tiot_firmware_send_cmd_retry(fw, ext_cmd, (uint8_t *)read_buff, cmd_len);
        if (ret != 0) {
            break;
        }
    } while (true);

#ifdef CONFIG_BOARD_DYNAMIC_ALLOC
    osal_kfree(read_buff);
#endif
    return ret == TIOT_FW_LOAD_RET_QUIT ? 0 : ret;
}

static inline int32_t firmware_read_u32(tiot_fw *fw, uint32_t *val)
{
    uint8_t data[4];
    int32_t ret = tiot_file_read(fw->fw_file, (char *)data, sizeof(uint32_t), fw->cur_offset);
    if (ret != sizeof(uint32_t)) {
        return -1;
    }
    fw->cur_offset += sizeof(uint32_t);
    *val = little_endian_to_u32(data);
    return 0;
}

static int32_t firmware_load_ext_version1(tiot_fw *fw, tiot_fw_ext_version1 *ext)
{
    if (firmware_read_u32(fw, &ext->file_len) != 0) {
        tiot_print_err("[TIoT:fw]read img file size fail.\n");
        return -1;
    }
    tiot_print_info("[TIoT:fw]img file size = %d\n", ext->file_len);
    return 0;
}

static int32_t tiot_firmware_load_file(tiot_fw *fw, const char *file_name)
{
    uint32_t version;
    int32_t ret = -1;
    tiot_controller *ctrl = tiot_container_of(fw, tiot_controller, firmware);
    const tiot_file_path *file_path = ctrl->dev_info->fw_path;
    tiot_fw_ext_version1 ext = { 0 };

    /* 初始化时保证cfg_path不为null */
    fw->fw_file = tiot_file_open(file_path, file_name, OSAL_O_RDONLY, 0);
    if (fw->fw_file == NULL) {
        tiot_print_err("open file %s fail\n", file_name);
        return ret;
    }
    /* 读取镜像格式版本号 */
    fw->cur_offset = 0;  /* 从偏移量为0开始读取镜像格式版本号 */
    if (firmware_read_u32(fw, &version) != 0) {
        goto file_close;
    }
    tiot_print_info("[TIoT:fw]cfg img version: 0x%x\n", version);
    if (version == TIOT_FW_EXT_VERSION1) { /* 存在拓展区 */
        if (firmware_load_ext_version1(fw, &ext) != 0) {
            goto file_close;
        }
#ifdef CONFIG_FILE_BY_ARRAY
        fw->fw_file->len = ext.file_len;
#else
        tiot_print_warning("[TIoT:fw]non file-array should not use ext version1.\n");
#endif
    }
    // 读取发送命令
    ret = tiot_firmware_load_cmds(fw);
file_close:
    tiot_file_close(fw->fw_file);
    fw->fw_file = NULL;
    return ret;
}

int32_t tiot_firmware_load(tiot_fw *fw)
{
    if (fw == NULL) {
        tiot_print_dbg("[TIoT][fw]fw object is NULL.\n");
        return -1;
    }
    if (fw->ops->pre_cmd_execute != NULL) {
        tiot_print_info("[TIoT][fw]Pre command execute\n");
        fw->ops->pre_cmd_execute(fw);
    }
    /* 执行cfg镜像内命令 */
    if (tiot_firmware_load_file(fw, fw->info.board_info.cfg_path) != 0) {
        tiot_print_err("[TIoT][fw]command execute err!\n");
        return -1;
    }
    if (fw->ops->after_cmd_execute != NULL) {
        tiot_print_info("[TIoT][fw]After command execute\n");
        fw->ops->after_cmd_execute(fw);
    }
    return 0;
}