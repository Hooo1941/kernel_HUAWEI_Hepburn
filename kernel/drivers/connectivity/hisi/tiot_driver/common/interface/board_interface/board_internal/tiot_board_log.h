/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved. \n
 *
 * Description: Board log header. \n
 * Author: @CompanyNameTag \n
 * History: \n
 * 2023-03-24, Create file. \n
 */
#ifndef TIOT_BOARD_LOG_H
#define TIOT_BOARD_LOG_H

#include "tiot_board_log_port.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/**
 * @defgroup tiot_common_interface_board_internal_log Board Log
 * @ingroup  tiot_common_interface_board_internal
 * @{
 */

#define TIOT_LOG_ALERT    0   /*!< Log level alert. */
#define TIOT_LOG_ERR      1   /*!< Log level error. */
#define TIOT_LOG_WARNING  2   /*!< Log level warning. */
#define TIOT_LOG_INFO     3   /*!< Log level info. */
#define TIOT_LOG_DEBUG    4   /*!< Log level debug. */

#ifndef TIOT_BOARD_LOG_LEVEL
#define TIOT_BOARD_LOG_LEVEL  TIOT_LOG_INFO
#endif

/* 宏定义 */
#if TIOT_BOARD_LOG_LEVEL >= TIOT_LOG_DEBUG
#define TIOT_PRINT_FUNCTION_NAME    BOARD_PRINT_FUNCTION_NAME
#define tiot_print_dbg(s, ...)  board_print_dbg(s, ##__VA_ARGS__)
#else
#define TIOT_PRINT_FUNCTION_NAME
#define tiot_print_dbg(s, ...)
#endif

#if TIOT_BOARD_LOG_LEVEL >= TIOT_LOG_INFO
#define tiot_print_info(s, ...)  board_print_info(s, ##__VA_ARGS__)
#define tiot_print_suc(s, ...)   board_print_suc(s, ##__VA_ARGS__)
#else
#define tiot_print_info(s, ...)
#define tiot_print_suc(s, ...)
#endif

#if TIOT_BOARD_LOG_LEVEL >= TIOT_LOG_WARNING
#define tiot_print_warning(s, ...)  board_print_warning(s, ##__VA_ARGS__)
#else
#define tiot_print_warning(s, ...)
#endif

#if TIOT_BOARD_LOG_LEVEL >= TIOT_LOG_ERR
#define tiot_print_err(s, ...)      board_print_err(s, ##__VA_ARGS__)
#else
#define tiot_print_err(s, ...)
#endif

#if TIOT_BOARD_LOG_LEVEL >= TIOT_LOG_ALERT
#define tiot_print_alert(s, ...)      board_print_alter(s, ##__VA_ARGS__)
#else
#define tiot_print_alert(s, ...)
#endif

/**
 * @}
 */

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif
