/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved. \n
 *
 * Description:  \n
 * Author: @CompanyNameTag \n
 * History: \n
 * 2023-12-26, Create file. \n
 */
#ifndef TIOT_BOARD_PIN_IRQ_HELPER_H
#define TIOT_BOARD_PIN_IRQ_HELPER_H

#include <linux/spinlock.h>
#include "tiot_types.h"
#include "tiot_board_pin_port.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

typedef struct {
    uint32_t pin;
    uint32_t irq;
    uint32_t irq_flags;
    uint32_t is_enabled;
    spinlock_t lock;
    tiot_pin_int_callback callback;
} tiot_irq_pin_node;

int32_t tiot_board_irq_pin_register(uint32_t irq_pin, uint32_t irq_flags);

tiot_irq_pin_node *tiot_board_irq_pin_node_by_pin(uint32_t pin);

void tiot_board_irq_pin_release(void);

/* FOR S4 */
void tiot_board_irq_pin_suspend(void);
/* FOR S4 */
void tiot_board_irq_pin_resume(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif