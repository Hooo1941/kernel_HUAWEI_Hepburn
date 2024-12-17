/**
 * Copyright (c) @CompanyNameMagicTag 2024-2024. All rights reserved. \n
 *
 * Description: TIoT Controllrt helper. \n
 * Author: @CompanyNameTag \n
 * History: \n
 * 2024-03-05, Create file. \n
 */
#ifndef TIOT_CONTROLLER_HELPER_H
#define TIOT_CONTROLLER_HELPER_H

#include "tiot_board_pin_port.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/**
 * @defgroup tiot_common_controller_helper Controller Helper
 * @ingroup  tiot_common_controllrt
 * @{
 */

static inline void tiot_pin_int_request(uint32_t pin, tiot_pin_int_callback cb)
{
    if (pin != TIOT_PIN_NONE) {
        (void)tiot_board_pin_register_int_callback(pin, cb);
        (void)tiot_board_pin_enable_int(pin, TIOT_PIN_INT_ENABLE);
    }
}

static inline void tiot_pin_int_release(uint32_t pin)
{
    if (pin != TIOT_PIN_NONE) {
        (void)tiot_board_pin_enable_int(pin, TIOT_PIN_INT_DISABLE);
        (void)tiot_board_pin_register_int_callback(pin, NULL);
    }
}

/**
 * @}
 */

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif