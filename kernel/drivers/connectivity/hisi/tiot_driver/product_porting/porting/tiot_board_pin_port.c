/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved. \n
 *
 * Description: Demo board PIN port. \n
 * Author: @CompanyNameTag \n
 * History: \n
 * 2023-04-20, Create file. \n
 */

#include "tiot_board_pin_port.h"
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include "tiot_board_pin_irq_helper.h"
#include "tiot_board_log.h"

int32_t tiot_board_pin_set_level(uint32_t pin, uint8_t level)
{
    if (pin == TIOT_PIN_NONE) {
        tiot_print_warning("pin set level with invalid pin.\r\n");
        return 0;
    }
    return gpio_direction_output(pin, (int)level);
}

int32_t tiot_board_pin_get_level(uint32_t pin, uint8_t *level)
{
    int32_t gpio_level;
    if (pin == TIOT_PIN_NONE) {
        tiot_print_warning("pin get level with invalid pin.\r\n");
        return 0;
    }
    gpio_level = gpio_get_value(pin);
    if (gpio_level < 0) {
        return gpio_level;
    }
    *level = (gpio_level == 0) ? TIOT_PIN_LEVEL_LOW : TIOT_PIN_LEVEL_HIGH;
    return 0;
}

int32_t tiot_board_pin_enable_int(uint32_t pin, uint8_t int_enable)
{
    unsigned long flags;
    tiot_irq_pin_node *irq_pin_node = NULL;
    if (pin == TIOT_PIN_NONE) {
        return -1;
    }
    irq_pin_node = tiot_board_irq_pin_node_by_pin(pin);
    if (irq_pin_node == NULL) {
        return -1;
    }
    spin_lock_irqsave(&irq_pin_node->lock, flags);
    if ((int_enable == TIOT_PIN_INT_ENABLE) && (irq_pin_node->is_enabled == TIOT_PIN_INT_DISABLE)) {
        enable_irq(irq_pin_node->irq);
        irq_pin_node->is_enabled = TIOT_PIN_INT_ENABLE;
    } else if ((int_enable == TIOT_PIN_INT_DISABLE) && (irq_pin_node->is_enabled == TIOT_PIN_INT_ENABLE)) {
        disable_irq_nosync(irq_pin_node->irq);
        irq_pin_node->is_enabled = TIOT_PIN_INT_DISABLE;
    }
    spin_unlock_irqrestore(&irq_pin_node->lock, flags);
    return 0;
}

static irqreturn_t tiot_board_gpio_irq_callback(int irq, void *node)
{
    tiot_irq_pin_node *irq_pin_node = (tiot_irq_pin_node *)node;
    tiot_unused(irq);
    /* RESUME TODO */
    if ((irq_pin_node != NULL) && (irq_pin_node->callback != NULL)) {
        tiot_print_dbg("gpio [%d] irq called!!!\n", irq_pin_node->pin);
        irq_pin_node->callback(irq_pin_node->pin);
    }
    return IRQ_HANDLED;
}

int32_t tiot_board_pin_register_int_callback(uint32_t pin, tiot_pin_int_callback int_cb)
{
    int32_t ret;
    tiot_irq_pin_node *irq_pin_node = tiot_board_irq_pin_node_by_pin(pin);
    if (irq_pin_node == NULL) {
        return -1;
    }
    irq_pin_node->callback = int_cb;
    if (irq_pin_node->irq == 0) {
        return -1;
    }
    if (int_cb != NULL) {
        ret = request_irq(irq_pin_node->irq, tiot_board_gpio_irq_callback,
                          irq_pin_node->irq_flags, NULL, irq_pin_node);
        if (ret < 0) {
            tiot_print_err("gpio index [%d] irq request fail ret = %d\n", pin, ret);
            return -1;
        }
        tiot_print_info("tiot gpio index [%d] irq request succ = %d\n", pin, irq_pin_node->irq);
        /* 默认先关闭中断，需要时再开启 */
        disable_irq_nosync(irq_pin_node->irq);
    } else {
        free_irq(irq_pin_node->irq, irq_pin_node);
    }
    return 0;
}