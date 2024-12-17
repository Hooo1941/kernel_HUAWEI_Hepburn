/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: usb phy chip basic interface defination
 * Author: Hisilicon
 * Create: 2020-3-11
 *
 * This software is distributed under the terms of the GNU General
 * Public License ("GPL") as published by the Free Software Foundation,
 * either version 2 of that License or (at your option) any later version.
 */
#ifndef __HI9030_GPIO__
#define __HI9030_GPIO__

#define HW_SW_SEL 4
#define HW_SW_SEL_MASK 20
#define GPIO_PER_REG 16
#define SET_MASK 0x00010001
#define HIGH_LEVEL_ENABLE 0x00080008
#define LOW_LEVEL_ENABLE  0x00080000
#define RISING_EDGE_ENABLE  0x00060002
#define FALLING_EDGE_ENABLE  0x00060000
#define BOTH_EDGE_ENABLE  0x00060006
#define LEVEL_TRIGGER_ENABLE 0x00010001
#define EDGE_TRIGGER_ENABLE 0x00010000

#define intr_ctrl_bit(x) (0x08 + 0x04 * (x))
#define config_mask(x) (0x1 << (x))
#define data_offset(x) (((x) > 15) ? 0x0 : 0x04)
#define mask_offset(x) (((x) > 15) ? (x) : (x) + 16)

#define AO_GPIO_BASE 448
#define AO_GPIO_MAX_NUM (AO_GPIO_BASE + 8)
#define GPIO_GROUP 2
#define GPIO_PREE_IRQ 21
#define PERI_GPIO_BASE 384

#define GPIO_NUM_32 32
#define GPIO_NUM_08 8
#define GPIOIM0 0x088
#define GPIOIC  0x0A8
#define GPIOIS0 0x0B0
#define SOURCE_DATA 0x0DC
#define DISABLE_IRQ_MASK 0xFFFFFFFF
#define CLEAR_IRQ_MASK 0xFFFFFFFF
#define GPIO_FIRST_BIT 0x01
#define GPIO_IRQ_ENABLE_FLAG 0x01

struct hi9030 {
	void __iomem *base;
	struct gpio_chip gc;
	struct irq_chip irq_chip;
	uint32_t parent_irq;
	struct platform_device *pdev;
	uint32_t irqarray;
};

#ifdef CONFIG_GPIO_HI9030
int hi9030_gpio_irq_request(unsigned int gpio_id, void (*handler)(void *),
	unsigned int irqflags, void *data);
#else
int hi9030_gpio_irq_request(unsigned int gpio_id, void (*handler)(void *),
	unsigned int irqflags, void *data) { return 0; }
#endif

#endif /* __HI9030_GPIO__ */