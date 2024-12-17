/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved. \n
 *
 * Description: Board UART porting header. \n
 * Author: @CompanyNameTag \n
 * History: \n
 * 2023-03-24, Create file. \n
 */
#ifndef TIOT_BOARD_UART_PORT_H
#define TIOT_BOARD_UART_PORT_H

#include "tiot_board_xmit.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/**
 * @defgroup tiot_common_interface_board_porting_uart Board UART Port
 * @ingroup  tiot_common_interface_board_porting
 * @{
 */

#define TIOT_UART_ATTR_DATABIT_8    0 /*!< UART word length is 8 data bits per frame. */
#define TIOT_UART_ATTR_DATABIT_7    1 /*!< UART word length is 7 data bits per frame. */
#define TIOT_UART_ATTR_DATABIT_6    2 /*!< UART word length is 6 data bits per frame. */
#define TIOT_UART_ATTR_DATABIT_5    3 /*!< UART word length is 5 data bits per frame. */
#define TIOT_UART_ATTR_DATABIT_BUTT 4 /*!< UART word length butt. */

#define TIOT_UART_ATTR_PARITY_NONE  0 /*!< no UART parity bit. */
#define TIOT_UART_ATTR_PARITY_ODD   1 /*!< odd UART parity bit. */
#define TIOT_UART_ATTR_PARITY_EVEN  2 /*!< even UART parity bit. */
#define TIOT_UART_ATTR_PARITY_MARK  3 /*!< UART parity bit is always 1. */
#define TIOT_UART_ATTR_PARITY_SPACE 4 /*!< UART parity bit is always 0. */
#define TIOT_UART_ATTR_PARITY_BUTT  5 /*!< UART parity bit butt. */

#define TIOT_UART_ATTR_STOPBIT_1    0 /*!< UART has 1 stop bit. */
#define TIOT_UART_ATTR_STOPBIT_1P5  1 /*!< UART has 1.5 stop bits. */
#define TIOT_UART_ATTR_STOPBIT_2    2 /*!< UART has 2 stop bits. */
#define TIOT_UART_ATTR_STOPBIT_BUTT 3 /*!< UART stop bits butt. */

#define TIOT_UART_ATTR_FLOW_CTRL_DISABLE  0 /*!< UART flow control is disabled. */
#define TIOT_UART_ATTR_FLOW_CTRL_ENABLE   1 /*!< UART flow control enabled. */

/**
 * @brief Uart attribute.
 *
 */
typedef struct {
    uint16_t data_bits : 4;
    uint16_t parity    : 4;
    uint16_t stop_bits : 4;
    uint16_t flow_ctrl : 4;
    uint16_t reserved  : 16;
} tiot_uart_attr;

/**
 * @brief Uart config.
 *
 */
typedef struct tiot_xmit_config {
    uint32_t baudrate;   /*!< UART baudrate. */
    tiot_uart_attr attr; /*!< UART attribute. */
} tiot_uart_config;

/**
 * @brief  Board UART open interface.
 * @param  [in]  xmit Transmit object.
 * @param  [in]  cb   Transmit callbacks.
 * @return zero if OK;
 *         non-zero if FAIL.
 */
int32_t tiot_board_uart_open(tiot_xmit *xmit, tiot_xmit_callbacks *cb);

/**
 * @brief  Board UART close interface.
 * @param  [in]  xmit Transmit object.
 */
void    tiot_board_uart_close(tiot_xmit *xmit);

/**
 * @brief  Board UART write interface.
 * @param  [in] xmit Transmit object.
 * @param  [in] buff Data buff.
 * @param  [in] len  Data length.
 * @return zero or postive for writen OK length;
 *         negtive if write FAIL.
 */
int32_t tiot_board_uart_write(tiot_xmit *xmit, const uint8_t *buff, uint32_t len);

/**
 * @brief  Board UART blocking read interface.
 * @param  [in] xmit Transmit object.
 * @param  [in] buff Read buff.
 * @param  [in] len  Expected read length.
 * @return zero or postive for read OK length;
 *         negtive if read FAIL or cannot implemented.
 */
int32_t tiot_board_uart_read(tiot_xmit *xmit, uint8_t *buff, uint32_t len);

/**
 * @brief  Board UART config interface.
 * @param  [in] xmit   Transmit object.
 * @param  [in] config UART config, see @ref tiot_uart_config.
 * @return zero if OK;
 *         non-zero if FAIL.
 */
int32_t tiot_board_uart_set_config(tiot_xmit *xmit, tiot_uart_config *config);

/**
 * @}
 */

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif
