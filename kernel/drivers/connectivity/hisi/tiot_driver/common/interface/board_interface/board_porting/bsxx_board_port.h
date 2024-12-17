/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved. \n
 *
 * Description: BSXX board port head. \n
 * Author: @CompanyNameTag \n
 * History: \n
 * 2023-06-13, Create file. \n
 */
#ifndef BSXX_BOARD_PORT_H
#define BSXX_BOARD_PORT_H

#include "tiot_types.h"
#include "tiot_board_pin_port.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/**
 * @defgroup tiot_device_bsxx_board_port  BSXX Board Port
 * @ingroup  tiot_device_bsxx
 * @{
 */

/**
 * @brief  BSXX pm pins.
 */
typedef enum {
    BSXX_PIN_POWER_CTRL,            /*!< BSXX power control pin. */
    BSXX_PIN_HOST_WAKEUP_DEVICE,    /*!< BSXX host wakeup device pin. */
    BSXX_PIN_DEVICE_WAKEUP_HOST,    /*!< BSXX device wakeup host pin. */
    BSXX_PIN_NUM,                   /*!< BSXX pin max. */
    BSXX_PIN_NONE = TIOT_PIN_NONE   /*!< BSXX pin none. */
} bsxx_pm_pins;

/**
 * @brief  BSXX board hardware info.
 */
typedef struct {
    uintptr_t xmit_id;
    uint32_t pm_info[BSXX_PIN_NUM];
} bsxx_board_hw_info;

/**
 * @brief  BSXX board info.
 */
typedef struct {
    const char *cfg_path;            /*!< cfg file path, NULL for file array. */
    bsxx_board_hw_info *hw_infos;    /*!< Pointer to array of BSXX hardware infos. */
} bsxx_board_info;

/**
 * @brief  Board provides resource informations, including pin indexs, ids, file paths, etc.
 * @return Provided resource information.
 */
const bsxx_board_info *bsxx_board_get_info(void);

/**
 * @brief  Board porting init, request board resources, including hardware io, interrupt，etc.
 * @param  param user customsize parameters, like device num.
 * @return zero if OK;
 *         non-zero if FAIL.
 */
int32_t bsxx_board_init(void *param);

/**
 * @brief  Board porting deinit, release board resources, including hardware io, interrupt，etc.
 * @param  param user customsize parameters, like device num.
 */
void bsxx_board_deinit(void *param);

/**
 * @}
 */

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif