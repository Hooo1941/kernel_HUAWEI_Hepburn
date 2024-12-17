/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved. \n
 *
 * Description: Board transmit io common header. \n
 * Author: @CompanyNameTag \n
 * History: \n
 * 2023-03-25, Create file. \n
 */
#ifndef TIOT_BOARD_XMIT_H
#define TIOT_BOARD_XMIT_H

#include "tiot_types.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/**
 * @defgroup tiot_common_interface_board_internal_xmit Board Transmit
 * @ingroup  tiot_common_interface_board_internal
 * @{
 */

/**
 * @brief  Transmit handle, compatible for board transmit handle.
 */
typedef void *tiot_xmit_handle;

/**
 * @brief  Universal transmit config, compatible for all types of transmit io.
 */
struct tiot_xmit_config;

/**
 * @brief  Transmit object.
 */
typedef struct {
    uintptr_t id;               /*!< Transmit io id.
                                     If board io operations use index, set this id in port open interface. */
    tiot_xmit_handle handle;    /*!< Transmit handle compatible for board io handle.
                                     If board io operations use handle, set this handle in port open interface. */
} tiot_xmit;

/**
 * @brief  Transmit callbacks, registed in port open interface.
 */
typedef struct {
    /*!< Rx data receive notify callback when use asynchronous board interface to read data. */
    void (*rx_notify)(tiot_xmit *xmit, uint8_t *data, uint32_t len);
} tiot_xmit_callbacks;

/**
 * @}
 */

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif