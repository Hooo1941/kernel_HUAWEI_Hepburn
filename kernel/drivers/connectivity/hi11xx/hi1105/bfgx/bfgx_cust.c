/**
 * Copyright (c) @CompanyNameMagicTag 2022-2023. All rights reserved.
 *
 * Description: UART rate customization
 * Author: @CompanyNameTag
 */

#include "bfgx_dev.h"
#include "board.h"
#include "plat_debug.h"
#include "plat_uart.h"
#include "bfgx_cust.h"

int32_t uart_baudrate_get_by_ini(uint32_t *baudrate)
{
    int32_t ret;
    int32_t ini_baudrate = 0;
    const uint32_t baudrate_map[INI_UART_RATE_NUM] = {
        0, // 只占位（芯片寄存器默认为0 ，ini文件配置值从1开始）
        UART_BAUD_RATE_DBG,
        UART_BAUD_RATE_1M,
        UART_BAUD_RATE_2M,
        UART_BAUD_RATE_3M,
        UART_BAUD_RATE_4M,
        UART_BAUD_RATE_5M,
        UART_BAUD_RATE_6M,
        UART_BAUD_RATE_7M,
        UART_BAUD_RATE_8M
    };

    ret = get_cust_conf_int32(INI_MODU_PLAT, "uart_rate_value", &ini_baudrate);
    if (ret == INI_FAILED) {
        ps_print_info("can't found uart rate ini");
        return -OAL_FAIL;
    }

    if ((ini_baudrate >= INI_UART_RATE_DBG) && (ini_baudrate < INI_UART_RATE_NUM)) {
        *baudrate = baudrate_map[ini_baudrate];
        return OAL_SUCC;
    }

    return -OAL_FAIL;
}
