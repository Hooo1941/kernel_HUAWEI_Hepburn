/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: provide gpio access function interfaces.
 * Create: 2022-3-12
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __GPIO_ENHANCE_I_H__
#define __GPIO_ENHANCE_I_H__

#include <linux/amba/bus.h>
#include <linux/hwspinlock.h>
#include <linux/gpio/driver.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/ioport.h>

#define PL061_GPIO_NR 8
#define GPIO_HWLOCK_ID 1
#define LOCK_TIMEOUT 1000
#define TRUE 1
#define FALSE 0

#ifdef CONFIG_PM
struct pl061_context_save_regs {
	u8 gpio_data;
	u8 gpio_dir;
	u8 gpio_is;
	u8 gpio_ibe;
	u8 gpio_iev;
	u8 gpio_ie;
};
#endif

struct pl061 {
	raw_spinlock_t lock;
	void __iomem *base;
	struct gpio_chip gc;
	struct irq_chip irq_chip;
	int parent_irq;

#ifdef CONFIG_PM
	struct pl061_context_save_regs csave_regs;
#endif
};

struct gpio_enhance_chip {
	struct hwspinlock *gpio_hwlock;
	struct pl061 *sub_chip;
	void __iomem *base;
	int parent_irq;
	struct gpio_chip gc;
	struct irq_chip irq_chip;
};

unsigned int get_gpio_hwspinlock_status(void);
int gpio_get_base_value(struct device_node *of_node);

int gpio_enhance_register(struct amba_driver *drv);
void gpio_sec_writeb(void __iomem *base, \
	unsigned char v, unsigned offset);

int gpiochip_usbphy_request(struct gpio_chip *chip, \
	unsigned int offset);
void gpiochip_usbphy_free(struct gpio_chip *chip, \
	unsigned int offset);


#define gpio_enhance_driver(__driver, __register, ...) \
static int __init __driver##_init(void) \
{ \
	return __register(&(__driver) , ##__VA_ARGS__); \
} \
subsys_initcall(__driver##_init)

#define module_gpio_enhance_driver(__amba_drv) \
	gpio_enhance_driver(__amba_drv, gpio_enhance_register)

#define gpio_hwspin_lock_irqsave(__ghwlock, __gc, __flag) \
	do { \
		if (hwspin_lock_timeout_irqsave(__ghwlock, LOCK_TIMEOUT, __flag)) { \
			dev_err((__gc)->parent, "%s: hwspinlock timeout!\n", __func__); \
			dev_err((__gc)->parent, "hwspinlock status is 0x%x\n", \
				get_gpio_hwspinlock_status()); \
			return -EBUSY; \
		} \
	} while(0)

#define gpio_hwspin_lock_void(__ghwlock, __gc) \
	do { \
		if (hwspin_lock_timeout(__ghwlock, LOCK_TIMEOUT)) { \
			dev_err((__gc)->parent, "%s: hwspinlock timeout!\n", __func__); \
			dev_err((__gc)->parent, "hwspinlock status is 0x%x\n", \
				get_gpio_hwspinlock_status()); \
			return; \
		} \
	} while(0)

#endif
