/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description: plat_firmware_util.c header file
 * Author: @CompanyNameTag
 */

#ifndef PLAT_FIRMWARE_UTIL_H
#define PLAT_FIRMWARE_UTIL_H

typedef struct file os_kernel_file;

#define READ_MEG_TIMEOUT      2000  /* 200ms */

#define SEND_BUF_LEN 520
#define RECV_BUF_LEN 512
#define HOST_DEV_TIMEOUT 3
#define INT32_STR_LEN    32

#define COMPART_KEYWORD ' '
#define CMD_LINE_SIGN   ';'

/* 以下是发往device命令的关键字 */
#define VER_CMD_KEYWORD          "VERSION"
#define BUCK_MODE_CMD_KEYWORD    "BUCK_MODE"
#define BRT_CMD_KEYWORD          "BAUDRATE"
#define JUMP_CMD_KEYWORD         "JUMP"
#define FILES_CMD_KEYWORD        "FILES"
#define RMEM_CMD_KEYWORD         "READM"
#define WMEM_CMD_KEYWORD         "WRITEM"
#define QUIT_CMD_KEYWORD         "QUIT"

/* 以下命令字不会发往device，用于控制加载流程，但是会体现在cfg文件中 */
#define SLEEP_CMD_KEYWORD          "SLEEP"
#define CALI_COUNT_CMD_KEYWORD     "CALI_COUNT"
#define CALI_BFGX_DATA_CMD_KEYWORD "CALI_BFGX_DATA"
#define CALI_DCXO_DATA_CMD_KEYWORD "CALI_DCXO_DATA"
#define PLAT_CUST_CMD_KEYWORD      "PLAT_CUST"

/* 以下是device对命令执行成功返回的关键字，host收到一下关键字则命令执行成功 */
#define MSG_FROM_DEV_WRITEM_OK "WRITEM OK"
#define MSG_FROM_DEV_READM_OK  ""
#define MSG_FROM_DEV_FILES_OK  "FILES OK"
#define MSG_FROM_DEV_READY_OK  "READY"
#define MSG_FROM_DEV_JUMP_OK   "JUMP OK"
#define MSG_FROM_DEV_QUIT_OK   ""

uint8_t *delete_space(uint8_t *string, int32_t *len);
int32_t num_to_string(uint8_t *string, uint32_t number);
int32_t string_to_num(const char *string, int32_t *number);
int32_t file_open_get_len(char *path, os_kernel_file **fp, uint32_t *file_len);
int32_t read_msg(uint8_t *data, int32_t data_len);
int32_t send_msg(uint8_t *data, int32_t len);
int32_t read_device_reg16(uint32_t address, uint16_t *value);
int32_t write_device_reg16(uint32_t address, uint16_t value);
int32_t recv_expect_result(const uint8_t *expect);
int32_t recv_expect_result_timeout(const uint8_t *expect, uint32_t timeout);
int32_t msg_send_and_recv_except(uint8_t *data, int32_t len, const uint8_t *expect);
#endif
