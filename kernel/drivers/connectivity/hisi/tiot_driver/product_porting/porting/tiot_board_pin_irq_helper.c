/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved. \n
 *
 * Description:  \n
 * Author: @CompanyNameTag \n
 * History: \n
 * 2023-12-26, Create file. \n
 */

#include "tiot_board_pin_irq_helper.h"
#include <linux/gpio.h>
#include <linux/interrupt.h>

#define TIOT_PIN_IRQ_MAX   16  /* for now this is enough. */

static tiot_irq_pin_node g_tiot_board_irq_pin_map[TIOT_PIN_IRQ_MAX] = {{ 0 }};
static uint32_t g_tiot_board_irq_pin_num = 0;

int32_t tiot_board_irq_pin_register(uint32_t irq_pin, uint32_t irq_flags)
{
    tiot_irq_pin_node *cur_node = NULL;
    if (g_tiot_board_irq_pin_num >= TIOT_PIN_IRQ_MAX) {
        return -1;
    }
    cur_node = &g_tiot_board_irq_pin_map[g_tiot_board_irq_pin_num];
    cur_node->pin = irq_pin;
    cur_node->irq = gpio_to_irq(irq_pin);
    cur_node->irq_flags = irq_flags;
    cur_node->callback = NULL;
    cur_node->is_enabled = TIOT_PIN_INT_DISABLE;
    spin_lock_init(&(cur_node->lock));
    g_tiot_board_irq_pin_num++;
    return 0;
}

tiot_irq_pin_node *tiot_board_irq_pin_node_by_pin(uint32_t pin)
{
    int i;
    tiot_irq_pin_node *cur_node = NULL;
    for (i = 0; i < g_tiot_board_irq_pin_num; i++) {
        cur_node = &g_tiot_board_irq_pin_map[i];
        if (cur_node->pin == pin) {
            return cur_node;
        }
    }
    return NULL;
}

void tiot_board_irq_pin_release(void)
{
    g_tiot_board_irq_pin_num = 0;
}