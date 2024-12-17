/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved. \n
 *
 * Description: kernel board log port. \n
 * Author: @CompanyNameTag \n
 * History: \n
 * 2023-05-06, Create file. \n
 */
#ifndef BOARD_LOG_PORT_H
#define BOARD_LOG_PORT_H

#include <linux/kernel.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/**
 * @defgroup tiot_board_port_log LOG port
 * @ingroup  tiot_board_port
 * @{
 */

#define TIOT_LOG_ALERT    0   /*!< Log level alert. */
#define TIOT_LOG_ERR      1   /*!< Log level error. */
#define TIOT_LOG_WARNING  2   /*!< Log level warning. */
#define TIOT_LOG_INFO     3   /*!< Log level info. */
#define TIOT_LOG_DEBUG    4   /*!< Log level debug. */

#define TIOT_BOARD_LOG_LEVEL  TIOT_LOG_INFO    /* 驱动日志默认为INFO等级，可按需要替换为其他等级. */

#define BOARD_PRINT_FUNCTION_NAME           printk(KERN_INFO KBUILD_MODNAME ":D]%s]", __func__)
#define board_print_dbg(s, args...)         printk(KERN_INFO KBUILD_MODNAME ":D]%s]" s, __func__, ##args)
#define board_print_info(s, args...)        printk(KERN_INFO KBUILD_MODNAME ":I]%s]" s, __func__, ##args)
#define board_print_suc(s, args...)         printk(KERN_INFO KBUILD_MODNAME ":S]%s]" s, __func__, ##args)
#define board_print_warning(s, args...)     printk(KERN_WARNING KBUILD_MODNAME ":W]%s]" s, __func__, ##args)
#define board_print_err(s, args...)         printk(KERN_ERR KBUILD_MODNAME ":E]%s]" s, __func__, ##args)
#define board_print_alter(s, args...)       printk(KERN_ERR KBUILD_MODNAME ":A]%s]" s, __func__, ##args)

/**
 * @}
 */

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif
