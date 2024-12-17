/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved. \n
 *
 * Description: TIoT firmware load module. \n
 * Author: @CompanyNameTag \n
 * History: \n
 * 2023-12-05, Create file. \n
 */
#ifndef TIOT_FIRMWARE_H
#define TIOT_FIRMWARE_H

#include "tiot_types.h"
#include "tiot_board.h"
#include "tiot_fileops.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/**
 * @defgroup tiot_common_firmware_load Firmware Load
 * @ingroup  tiot_common
 * @{
 */

typedef enum {
    TIOT_FW_LOAD_RET_ERR = -1,
    TIOT_FW_LOAD_RET_OK = 0,
    TIOT_FW_LOAD_RET_RETRY = 1,
    TIOT_FW_LOAD_RET_QUIT = 2,
} tiot_fw_ret;

/*
    固件文件格式：
        | Version | Extended Info | Command1 | Command2 | ... | CommandN |
        |---------|---------------|----------|----------|-----|----------|
        | 4bytes  |      --       |     -    |     -    |     |     -    |
    单条命令格式：
        | ext_cmd | cmd_len |  cmd   |
        |---------|---------|--------|
        |       2bytes      | Nbytes |
        | [12:15] | [0:11]  |        |
*/
/**
 * @brief  TIoT firmware infos.
 */
typedef struct {
    tiot_board_fw_info board_info;  /*!< Board firmware info, curently config file path. */
} tiot_fw_info;

/**
 * @brief  TIoT firmware extended infos.
 */
typedef struct {
    uint32_t file_len; /*!< img file size */
} tiot_fw_ext_version1;

struct _tiot_fw;

/**
 * @brief  TIoT firmware command execute callbacks.
 */
typedef struct {
    void (* pre_cmd_execute)(struct _tiot_fw *fw);   /*!< Before command execute. */
    void (* after_cmd_execute)(struct _tiot_fw *fw); /*!< After command execute. */
     /*!< After single command execute. */
    int32_t (* ext_cmd_handle)(struct _tiot_fw *fw, uint16_t ext_cmd, const uint8_t *cmd, uint16_t cmd_len);
} tiot_fw_ops;

/**
 * @brief  TIoT firmware object.
 */
typedef struct _tiot_fw {
    tiot_fw_info info;             /*!< Firmware info, config file and firmware files. */
    const tiot_fw_ops *ops;        /*!< Firmware ops. */
    tiot_file *fw_file;            /*!< Firmware file pointer. */
    uint32_t cur_offset;           /*!< Firmware file offset. */
} tiot_fw;

/**
 * @brief  TIoT firmware object init.
 * @param  [in]  fw           TIoT firmware object.
 * @param  [in]  board_info   Board firmware info, curently config file path.
 * @param  [in]  exec_cbs     Firmware command execute callbacks.
 * @return if OK return 0, else return -1.
 */
int32_t tiot_firmware_init(tiot_fw *fw, tiot_board_fw_info *board_info, const tiot_fw_ops *ops);

/**
 * @brief  TIoT firmware start load process.
 * @param  [in]  fw TIoT firmware object.
 * @param  [in/out] fw_cmd_info TIoT firmware load command info.
 * @return if OK return 0, else return -1.
 */
int32_t tiot_firmware_load(tiot_fw *fw);

/**
 * @}
 */

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif