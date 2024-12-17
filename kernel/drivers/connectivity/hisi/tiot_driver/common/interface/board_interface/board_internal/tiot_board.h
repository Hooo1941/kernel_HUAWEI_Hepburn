/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved. \n
 *
 * Description:  \n
 * Author: @CompanyNameTag \n
 * History: \n
 * 2023-07-04, Create file. \n
 */
#ifndef TIOT_BOARD_H
#define TIOT_BOARD_H

#include "tiot_types.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/**
 * @defgroup tiot_common_interface_board_internal_base Board
 * @ingroup  tiot_common_interface_board_internal
 * @{
 */

/**
 * @brief  TIoT common board pins.
 */
typedef enum {
    TIOT_PIN_POWER_CTRL,
    TIOT_PIN_HOST_WAKEUP_DEVICE,
    TIOT_PIN_DEVICE_WAKEUP_HOST,
    TIOT_PIN_MAX
} tiot_board_pin;

/**
 * @brief  Board Firmware info.
 */
typedef struct {
    const char *cfg_path;   /*!< Config file path, if use file array, set NULL. */
} tiot_board_fw_info;

/**
 * @brief  Board hardware infomations.
 */
typedef struct {
    uintptr_t xmit_id;         /*!< Transmit(UART/I2C/SPI/...) port index. */
    const uint32_t *pm_info;   /*!< Power manager related infomations. */
} tiot_board_hw_info;

/**
 * @brief  Board infomations, includes hardware infos and firmware info.
 */
typedef struct {
    tiot_board_fw_info fw_info;
    tiot_board_hw_info hw_info;       /*!< Power manager related infomations. */
} tiot_board_info;

/**
 * @}
 */

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif