/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 * Description: provide gpio access function interfaces.
 * Create: 2019-6-12
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
#include "hiv500_gpio.h"
#include "gpio_enhance_internal.h"
#include <linux/spinlock.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/irqchip/chained_irq.h>
#include <linux/bitops.h>
#include <linux/gpio.h>
#include <linux/device.h>
#include <linux/amba/bus.h>
#include <linux/slab.h>
#include <linux/pinctrl/consumer.h>
#include <linux/pm.h>
#include <linux/arm-smccc.h>

#define INTR_CTRL_BIT(x) (0x08 + 0x04 * (x))
#define SET_MASK 0x00010001
#define CONFIG_MASK(x) (0x1 << (x))
#define DATA_OFFSET(x) (((x) > 15) ? 0x0 : 0x04)
#define MASK_OFFSET(x) (((x) > 15) ? (x) : (x) + 16)
#define SOURCE_DATA 0x0DC
#define DISABLE_IRQ_MASK 0xFFFFFFFF
#define CLEAR_IRQ_MASK 0xFFFFFFFF

#define HIV500_GPIO_NR 32
#define NORMAL_GPIO_NR 8
#define GPIOIM0 0x088
#define GPIOIM1 0x08C
#define GPIOIM2 0x090
#define GPIOIM3 0x094
#define GPIOIM4 0x098
#define GPIOIM5 0x09C
#define GPIOIM6 0x0A0
#define GPIOIM7 0x0A4
#define GPIOIC  0x0A8
#define GPIOIRS 0x0AC
#define GPIOIS0 0x0B0
#define GPIOIS1 0x0B4
#define GPIOIS2 0x0B8
#define GPIOIS3 0x0BC
#define GPIOIS4 0x0C0
#define GPIOIS5 0x0C4
#define GPIOIS6 0x0C8
#define GPIOIS7 0x0CC

unsigned int hiv500_readl(struct hiv500 *chip, unsigned int offset)
{
	unsigned int v;

	v = readl(chip->base + offset);

	return v;
}

void hiv500_writel(struct hiv500 *chip, unsigned int v, unsigned int offset)
{
	writel(v, chip->base + offset);
}

static int hiv500_get_direction(struct gpio_chip *gc, unsigned int offset)
{
	struct hiv500 *hiv500 = gpiochip_get_data(gc);
	return !(hiv500_readl(hiv500, INTR_CTRL_BIT(offset)) & BIT(HW_SW_SEL));
}

static int hiv500_direction_input(struct gpio_chip *gc, unsigned int offset)
{
	struct hiv500 *hiv500 = gpiochip_get_data(gc);
	unsigned long flags;
	unsigned int gpiodir = 0;

	if (offset >= gc->ngpio)
		return -EINVAL;

	raw_spin_lock_irqsave(&hiv500->lock, flags);
	gpiodir |= BIT(HW_SW_SEL_MASK);
	hiv500_writel(hiv500, gpiodir, (INTR_CTRL_BIT(offset)));
	raw_spin_unlock_irqrestore(&hiv500->lock, flags);

	return 0;
}

static int hiv500_direction_output(struct gpio_chip *gc, unsigned int offset,
		int value)
{
	struct hiv500 *hiv500 = gpiochip_get_data(gc);
	unsigned long flags;
	unsigned int gpiodir = 0;
	unsigned int gpioval = 0;

	if (offset >= gc->ngpio)
		return -EINVAL;

	raw_spin_lock_irqsave(&hiv500->lock, flags);
	gpioval |= BIT(MASK_OFFSET(offset));
	gpioval |= (!!value << (offset % GPIO_PER_REG));
	hiv500_writel(hiv500, gpioval, (DATA_OFFSET(offset)));
	gpiodir |= (SET_MASK << HW_SW_SEL);
	hiv500_writel(hiv500, gpiodir, (INTR_CTRL_BIT(offset)));

	raw_spin_unlock_irqrestore(&hiv500->lock, flags);

	return 0;
}

static int hiv500_get_value(struct gpio_chip *gc, unsigned int offset)
{
	struct hiv500 *hiv500 = gpiochip_get_data(gc);
	return (CONFIG_MASK(offset) & (hiv500_readl(hiv500, SOURCE_DATA))) >> offset;
}

static void hiv500_set_value(struct gpio_chip *gc, unsigned int offset,
		int value)
{
	unsigned int gpioval = 0;
	struct hiv500 *hiv500 = gpiochip_get_data(gc);

	gpioval |= BIT(MASK_OFFSET(offset));
	gpioval |= (!!value << (offset % GPIO_PER_REG));
	hiv500_writel(hiv500, gpioval, (DATA_OFFSET(offset)));
}

static int hiv500_irq_type(struct irq_data *d, unsigned trigger)
{
	struct gpio_chip *gc = irq_data_get_irq_chip_data(d);
	struct hiv500 *hiv500 = gpiochip_get_data(gc);
	irq_hw_number_t offset = irqd_to_hwirq(d);
	unsigned long flags;
	unsigned int intr_ctrl;

	if (offset >= HIV500_GPIO_NR)
		return -EINVAL;

	if ((trigger & (IRQ_TYPE_LEVEL_HIGH | IRQ_TYPE_LEVEL_LOW)) &&
	    (trigger & (IRQ_TYPE_EDGE_RISING | IRQ_TYPE_EDGE_FALLING)))
	{
		dev_err(gc->parent,
			"trying to configure line %d for both level and edge "
			"detection, choose one!\n",
			offset);
		return -EINVAL;
	}

	raw_spin_lock_irqsave(&hiv500->lock, flags);

	intr_ctrl = hiv500_readl(hiv500, INTR_CTRL_BIT(offset));

	if (trigger & (IRQ_TYPE_LEVEL_HIGH | IRQ_TYPE_LEVEL_LOW)) {
		bool polarity = trigger & IRQ_TYPE_LEVEL_HIGH;
		/* Disable edge detection */
		/* Enable level detection */
		intr_ctrl |= LEVEL_TRIGGER_ENABLE;
		/* Select polarity */
		if (polarity)
			intr_ctrl |= HIGH_LEVEL_ENABLE;
		else
			intr_ctrl |= LOW_LEVEL_ENABLE;
		irq_set_handler_locked(d, handle_level_irq);
		dev_dbg(gc->parent, "hiv500_debug line %d: IRQ on %s level intr_ctrl = 0x%x\n",
			offset,
			polarity ? "HIGH" : "LOW",intr_ctrl);
	} else if ((trigger & IRQ_TYPE_EDGE_BOTH) == IRQ_TYPE_EDGE_BOTH) {
		/* Disable level detection double edge */
		/* Select both edges */
		intr_ctrl |= EDGE_TRIGGER_ENABLE;
		intr_ctrl |= BOTH_EDGE_ENABLE;
		irq_set_handler_locked(d, handle_edge_irq);
		dev_dbg(gc->parent, "line %d: IRQ on both edges\n", offset);
	} else if ((trigger & IRQ_TYPE_EDGE_RISING) ||
		   (trigger & IRQ_TYPE_EDGE_FALLING)) {
		bool rising = trigger & IRQ_TYPE_EDGE_RISING;
		/* Disable level detection */
		/* Clear detection on both edges */
		intr_ctrl |= EDGE_TRIGGER_ENABLE;
		/* Select edge */
		if (rising)
			intr_ctrl |= RISING_EDGE_ENABLE;
		else
			intr_ctrl |= FALLING_EDGE_ENABLE;
		irq_set_handler_locked(d, handle_edge_irq);
		dev_dbg(gc->parent, "line %d: IRQ on %s edge\n",
			offset,
			rising ? "RISING" : "FALLING");
	} else {
		/* No trigger: disable everything */
		irq_set_handler_locked(d, handle_bad_irq);
		dev_dbg(gc->parent, "no trigger selected for line %d\n",
			 offset);
	}
	hiv500_writel(hiv500, intr_ctrl, (INTR_CTRL_BIT(offset)));

	raw_spin_unlock_irqrestore(&hiv500->lock, flags);

	return 0;
}

static void hiv500_irq_handler(struct irq_desc *desc)
{
	unsigned long pending;
	irq_hw_number_t offset;
	struct gpio_chip *gc = irq_desc_get_handler_data(desc);
	struct hiv500 *hiv500 = gpiochip_get_data(gc);
	struct irq_chip *irqchip = irq_desc_get_chip(desc);

	chained_irq_enter(irqchip, desc);

	pending = hiv500_readl(hiv500, GPIOIS0);
	if (pending) {
		for_each_set_bit(offset, &pending, HIV500_GPIO_NR)
			generic_handle_irq(irq_find_mapping(gc->irq.domain,
							    offset));
	}
	chained_irq_exit(irqchip, desc);
}

static void hiv500_irq_mask(struct irq_data *d)
{
	struct gpio_chip *gc = irq_data_get_irq_chip_data(d);
	struct hiv500 *hiv500 = gpiochip_get_data(gc);
	unsigned int mask = BIT(irqd_to_hwirq(d) % HIV500_GPIO_NR);
	unsigned int gpioim;
	unsigned long flags;

	raw_spin_lock_irqsave(&hiv500->lock, flags);
	gpioim = hiv500_readl(hiv500, GPIOIM0) | mask;
	hiv500_writel(hiv500, gpioim, GPIOIM0);
	raw_spin_unlock_irqrestore(&hiv500->lock, flags);
}

static void hiv500_irq_unmask(struct irq_data *d)
{
	struct gpio_chip *gc = irq_data_get_irq_chip_data(d);
	struct hiv500 *hiv500 = gpiochip_get_data(gc);
	unsigned int mask = BIT(irqd_to_hwirq(d) % HIV500_GPIO_NR);
	unsigned int gpioim;
	unsigned long flags;

	raw_spin_lock_irqsave(&hiv500->lock, flags);
	gpioim = hiv500_readl(hiv500, GPIOIM0) & ~mask;
	hiv500_writel(hiv500, gpioim, GPIOIM0);
	raw_spin_unlock_irqrestore(&hiv500->lock, flags);
}

/**
 * hiv500_irq_ack() - ACK an edge IRQ
 * @d: IRQ data for this IRQ
 *
 * This gets called from the edge IRQ handler to ACK the edge IRQ
 * in the GPIOIC (interrupt-clear) register. For level IRQs this is
 * not needed: these go away when the level signal goes away.
 */
static void hiv500_irq_ack(struct irq_data *d)
{
	struct gpio_chip *gc = irq_data_get_irq_chip_data(d);
	struct hiv500 *hiv500 = gpiochip_get_data(gc);
	unsigned int mask = BIT(irqd_to_hwirq(d) % HIV500_GPIO_NR);

	raw_spin_lock(&hiv500->lock);
	hiv500_writel(hiv500, mask, GPIOIC);
	raw_spin_unlock(&hiv500->lock);
}

static int hiv500_irq_set_wake(struct irq_data *d, unsigned int state)
{
	return 0;
}

static int hiv500_get_base_value(struct device_node *of_node)
{
	struct device_node *np = of_node;
	int base = 0;

	if(!of_property_read_s32(np, "base,offset", &base))
		return base;

	return -1;
}
static int hiv500_probe(struct amba_device *adev, const struct amba_id *id)
{
	struct device *dev = &adev->dev;
	struct hiv500 *hiv500 = NULL;
	struct gpio_irq_chip *girq = NULL;
	int ret;

	hiv500 = devm_kzalloc(dev, sizeof(*hiv500), GFP_KERNEL);
	if (!hiv500)
		return -ENOMEM;

	hiv500->adev = adev;
	hiv500->base = devm_ioremap_resource(dev, &adev->res);
	if (IS_ERR(hiv500->base))
		return PTR_ERR(hiv500->base);

	raw_spin_lock_init(&hiv500->lock);
	if (of_get_property(dev->of_node, "gpio-ranges", NULL)) {
		hiv500->gc.request = gpiochip_generic_request;
		hiv500->gc.free = gpiochip_generic_free;
	}

	hiv500->gc.base = hiv500_get_base_value(dev->of_node);
	hiv500->gc.get_direction = hiv500_get_direction;
	hiv500->gc.direction_input = hiv500_direction_input;
	hiv500->gc.direction_output = hiv500_direction_output;
	hiv500->gc.get = hiv500_get_value;
	hiv500->gc.set = hiv500_set_value;
	if (of_get_property(dev->of_node, "gpio-nr", NULL))
		hiv500->gc.ngpio = NORMAL_GPIO_NR;
	else
		hiv500->gc.ngpio = HIV500_GPIO_NR;
	hiv500->gc.label = dev_name(dev);
	hiv500->gc.parent = dev;
	hiv500->gc.owner = THIS_MODULE;

	/*
	 * irq_chip support
	 */
	hiv500->irq_chip.name = dev_name(dev);
	hiv500->irq_chip.irq_ack	= hiv500_irq_ack;
	hiv500->irq_chip.irq_mask = hiv500_irq_mask;
	hiv500->irq_chip.irq_unmask = hiv500_irq_unmask;
	hiv500->irq_chip.irq_set_type = hiv500_irq_type;
	hiv500->irq_chip.irq_set_wake = hiv500_irq_set_wake;
#ifdef CONFIG_ARCH_HISI
	hiv500->irq_chip.irq_disable = hiv500_irq_mask;
	hiv500->irq_chip.irq_enable = hiv500_irq_unmask;
#endif

	hiv500_writel(hiv500, CLEAR_IRQ_MASK, GPIOIC);
	hiv500_writel(hiv500, DISABLE_IRQ_MASK, GPIOIM0); /* disable irqs */
	hiv500->parent_irq = adev->irq[0];

	girq = &hiv500->gc.irq;
	girq->chip = &hiv500->irq_chip;
	girq->parent_handler = hiv500_irq_handler;
	girq->num_parents = 1;
	girq->parents = devm_kcalloc(dev, 1, sizeof(*girq->parents),
				     GFP_KERNEL);
	if (!girq->parents)
		return -ENOMEM;
	girq->parents[0] = adev->irq[0];
	girq->default_type = IRQ_TYPE_NONE;
	girq->handler = handle_bad_irq;

	ret = devm_gpiochip_add_data(dev, &hiv500->gc, hiv500);
	if (ret)
		return ret;

	amba_set_drvdata(adev, hiv500);
	dev_err(&adev->dev, "hiv500 GPIO chip registered\n");

	return 0;
}


static const struct amba_id hiv500_ids[] = {
	{
		.id = 0x00048500,
		.mask = 0x000fffff,
	},
	{ 0, 0 },
};

static struct amba_driver hiv500_gpio_driver = {
	.drv = {
		.name = "hiv500_gpio",
	},
	.id_table = hiv500_ids,
	.probe = hiv500_probe,
};

static int __init hiv500_gpio_init(void)
{
	return amba_driver_register(&hiv500_gpio_driver);
}
subsys_initcall(hiv500_gpio_init);
