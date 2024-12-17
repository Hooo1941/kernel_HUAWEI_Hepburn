/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved. \n
 *
 * Description: Board pin porting header. \n
 * Author: @CompanyNameTag \n
 * History: \n
 * 2023-03-24, Create file. \n
 */
#ifndef TIOT_BOARD_PIN_PORT_H
#define TIOT_BOARD_PIN_PORT_H

#include "tiot_types.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/**
 * @defgroup tiot_common_interface_board_porting_pin Board Pin Port
 * @ingroup  tiot_common_interface_board_porting
 * @{
 */

/**
 * @brief  Pin none, board port should use this macro if pin is not connected.
 */
#define TIOT_PIN_NONE        0xFFFFFFFFU
#define TIOT_PIN_UART_WAKEUP_NUM    0x80000000U    /* 用于uart rx唤醒. */

/**
 * @brief  Pin level enum.
 */
typedef enum {
    TIOT_PIN_LEVEL_LOW,     /*!< 0: Pin level low. */
    TIOT_PIN_LEVEL_HIGH     /*!< 1: Pin level high. */
} tiot_pin_level;

/**
 * @brief  Pin interrupt enable enum.
 */
typedef enum {
    TIOT_PIN_INT_DISABLE,   /*!< 0: Pin interrupt disable. */
    TIOT_PIN_INT_ENABLE     /*!< 1: Pin interrupt enable. */
} tiot_pin_int_enable;

/**
 * @brief  Pin interrupt callback type, need input corresponding pin.
 */
typedef void (* tiot_pin_int_callback)(uint32_t pin);

/**
 * @brief  Board set pin level interface.
 * @param  [in] pin   Pin id.
 * @param  [in] level Pin level, see @ref tiot_pin_level.
 * @return zero if OK;
 *         non-zero if FAIL.
 */
int32_t tiot_board_pin_set_level(uint32_t pin, uint8_t level);

/**
 * @brief  Board get current pin level interface.
 * @param  [in]  pin   Pin id.
 * @param  [out] level Current pin level, see @ref tiot_pin_level.
 * @return zero if OK;
 *         non-zero if FAIL.
 */
int32_t tiot_board_pin_get_level(uint32_t pin, uint8_t *level);

/**
 * @brief  Board pin interrupt enable interface.
 * @param  [in] pin         Pin id.
 * @param  [in] int_enable  Interrupt enable or disable, see @ref tiot_pin_int_enable.
 * @return zero if OK;
 *         non-zero if FAIL.
 */
int32_t tiot_board_pin_enable_int(uint32_t pin, uint8_t int_enable);

/**
 * @brief  Board pin interrupt callback register interface.
 * @note   Porting should ensure this callback is called when enters pin irq if interrupt enabled.
 * @param  [in] pin     Pin id.
 * @param  [in] int_cb  Registed interrupt callback, see @ref tiot_pin_int_callback.
 * @return zero if OK;
 *         non-zero if FAIL.
 */
int32_t tiot_board_pin_register_int_callback(uint32_t pin, tiot_pin_int_callback int_cb);

/**
 * @}
 */

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif