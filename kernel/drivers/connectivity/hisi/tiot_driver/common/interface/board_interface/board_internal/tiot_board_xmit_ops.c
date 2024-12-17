/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved. \n
 *
 * Description: Board Transmit select. \n
 * Author: @CompanyNameTag \n
 * History: \n
 * 2023-03-27, Create file. \n
 */
#include "tiot_board_xmit_ops.h"

const tiot_xmit_ops g_tiot_xmit_ops_map[TIOT_XMIT_TYPE_BUTT] = {
#ifdef CONFIG_XMIT_WITH_UART
    {
        .open = tiot_board_uart_open,
        .close = tiot_board_uart_close,
        .write = tiot_board_uart_write,
        .read = tiot_board_uart_read,
        .set_config = tiot_board_uart_set_config,
#ifdef CONFIG_UART_RX_MODE_BUFFED
        .rx_mode = TIOT_XMIT_RX_MODE_BUFFED,
#else
        .rx_mode = TIOT_XMIT_RX_MODE_POLL,
#endif
    },
#endif
#ifdef CONFIG_XMIT_WITH_SPI
    {
        .open = tiot_board_spi_open,
        .close = tiot_board_spi_close,
        .write = tiot_board_spi_write,
        .read = tiot_board_spi_read,
        .set_config = tiot_board_spi_set_config,
#ifdef CONFIG_SPI_RX_MODE_BUFFED
        .rx_mode = TIOT_XMIT_RX_MODE_BUFFED,
#else
        .rx_mode = TIOT_XMIT_RX_MODE_POLL,
#endif
    },
#endif
#ifdef CONFIG_XMIT_WITH_I2C
    {
        .open = tiot_board_i2c_open,
        .close = tiot_board_i2c_close,
        .write = tiot_board_i2c_write,
        .read = tiot_board_i2c_read,
        .set_config = tiot_board_i2c_set_config,
#ifdef CONFIG_I2C_RX_MODE_BUFFED
        .rx_mode = TIOT_XMIT_RX_MODE_BUFFED,
#else
        .rx_mode = TIOT_XMIT_RX_MODE_POLL,
#endif
    },
#endif
};

const tiot_xmit_ops *tiot_xmit_get_ops(tiot_xmit_type type)
{
    if (type >= TIOT_XMIT_TYPE_BUTT) {
        return NULL;
    }
    return &g_tiot_xmit_ops_map[type];
}