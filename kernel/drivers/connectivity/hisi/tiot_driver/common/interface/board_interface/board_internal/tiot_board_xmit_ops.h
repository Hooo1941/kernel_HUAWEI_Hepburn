/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved. \n
 *
 * Description: Board transmit io common ops. \n
 * Author: @CompanyNameTag \n
 * History: \n
 * 2023-06-13, Create file. \n
 */
#ifndef TIOT_BOARD_XMIT_OPS_H
#define TIOT_BOARD_XMIT_OPS_H

#include "tiot_board_xmit.h"
#ifdef CONFIG_XMIT_WITH_UART
#include "tiot_board_uart_port.h"
#endif
#ifdef CONFIG_XMIT_WITH_SPI
#include "tiot_board_spi_port.h"
#endif
#ifdef CONFIG_XMIT_WITH_I2C
#include "tiot_board_i2c_port.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/**
 * @defgroup tiot_common_interface_board_internal_xmit Board Transmit Ops
 * @ingroup  tiot_common_interface_board_internal
 * @{
 */

/**
 * @brief  TIoT support transmit type.
 */
typedef enum {
#ifdef CONFIG_XMIT_WITH_UART
    TIOT_XMIT_TYPE_UART,
#endif
#ifdef CONFIG_XMIT_WITH_SPI
    TIOT_XMIT_TYPE_SPI,
#endif
#ifdef CONFIG_XMIT_WITH_I2C
    TIOT_XMIT_TYPE_I2C,
#endif
    TIOT_XMIT_TYPE_BUTT
} tiot_xmit_type;

/**
 * @brief  TIoT Transmit rx mode.
 */
typedef enum {
    TIOT_XMIT_RX_MODE_BUFFED,  /* TIoT Transmit rx mode with circular buffer. */
    TIOT_XMIT_RX_MODE_POLL,    /* TIoT Transmit rx mode without buffer, read data direct from io. */
} tiot_xmit_rx_mode;

/**
 * @brief  Transmit operations.
 */
typedef struct {
    int32_t (*open)(tiot_xmit *xmit, tiot_xmit_callbacks *cbs);                 /*!< Port open interface. */
    void (*close)(tiot_xmit *xmit);                                             /*!< Port close interface. */
    int32_t (*write)(tiot_xmit *xmit, const uint8_t *buff, uint32_t len);       /*!< Port write interface. */
    int32_t (*read)(tiot_xmit *xmit, uint8_t *buff, uint32_t len);              /*!< Port read interface. */
    int32_t (*set_config)(tiot_xmit *xmit, struct tiot_xmit_config *config);    /*!< Port config interface. */
    const uint16_t rx_mode;
} tiot_xmit_ops;

/**
 * @brief  Get selected transmit operations.
 * @return const tiot_xmit_ops* Selected transmit operations.
 */
const tiot_xmit_ops *tiot_xmit_get_ops(tiot_xmit_type type);

/**
 * @}
 */

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif