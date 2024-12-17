/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 *
 * scharger_v700.c
 *
 * schargerV700 i2c driver
 *
 * Copyright (c) 2022 Huawei Technologies Co., Ltd.
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, pmicstributed, and mopmicfied under those terms.
 *
 * This program is pmicstributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */
#include "scharger_v700.h"
#include <linux/platform_drivers/usb/hisi_tcpc_ops.h>

struct opt_regs scharger_common_v700_option_regs[] = {
	/* reg, mask, shift, val, before,after */
	reg_cfg(0x966, 0xff, 0, 0x01, 0, 0),
	reg_cfg(0x957, 0xff, 0, 0x03, 0, 0),
	reg_cfg(0x9a5, 0xff, 0, 0xfb, 0, 0),
	reg_cfg(0x9a7, 0xff, 0, 0x04, 0, 0),
	reg_cfg(0x961, 0xff, 0, 0x01, 0, 0),
	reg_cfg(0x375, 0xff, 0, 0x13, 0, 0),
	reg_cfg(0x819, 0xff, 0, 0x1C, 0, 0),
	reg_cfg(0x510, 0xff, 0, 0xAC, 0, 0),
	reg_cfg(0x50e, 0xff, 0, 0x04, 0, 0),
	reg_cfg(0x510, 0xff, 0, 0x5C, 0, 0),
};

struct opt_regs scharger_common_v710_option_regs[] = {
	/* reg, mask, shift, val, before,after */
	reg_cfg(0x966, 0xff, 0, 0x01, 0, 0),
	reg_cfg(0x957, 0xff, 0, 0x03, 0, 0),
	reg_cfg(0x9a7, 0xff, 0, 0x04, 0, 0),
	reg_cfg(0x375, 0xff, 0, 0x13, 0, 0),
	reg_cfg(0x819, 0xff, 0, 0x1C, 0, 0),
	reg_cfg(0x510, 0xff, 0, 0xac, 0, 0),
	reg_cfg(0x587, 0xff, 0, 0x05, 0, 0),
	reg_cfg(0x510, 0xff, 0, 0x5a, 0, 0),
	reg_cfg(0x339, 0xff, 0, 0x00, 0, 0),
	reg_cfg(0x510, 0xff, 0, 0xAC, 0, 0),
	reg_cfg(0x50e, 0xff, 0, 0x04, 0, 0),
	reg_cfg(0x510, 0xff, 0, 0x5C, 0, 0),
};

static struct scharger_v700_priv *g_scharger = NULL;

static void opt_regs_set(struct opt_regs *opt, unsigned int len)
{
	unsigned int i;

	if (opt == NULL) {
		pr_err("%s is NULL!\n", __func__);
		return;
	}
	for (i = 0; i < len; i++) {
		(void)mdelay(opt[i].before);
		(void)regmap_update_bits(g_scharger->regmap, opt[i].reg, opt[i].mask,
					opt[i].val);
		(void)mdelay(opt[i].after);
	}
}

static int scharger_read_block(u16 addr, u8 *val, unsigned int num_bytes)
{
	u16 temp;
	int ret;
	struct i2c_msg msgs[2]; /* addr, val */
	struct scharger_v700_priv *scharger = g_scharger;

	if (scharger == NULL) {
		pr_err("%s scharger_v700_priv is NULL!\n", __func__);
		return -1;
	}

	temp = cpu_to_be16(addr);
	msgs[0].addr = scharger->client->addr;
	msgs[0].flags = 0;
	msgs[0].len = (int)sizeof(u16);
	msgs[0].buf = (char *)(&temp);

	msgs[1].addr = scharger->client->addr;
	msgs[1].flags = I2C_M_RD;
	msgs[1].len = num_bytes;
	msgs[1].buf = (char *)val;

	ret = i2c_transfer(scharger->client->adapter, msgs, 2);
	if (ret != 2) {
		pr_err("%s transfer err, ret %d\n", __func__, ret);
		return ret;
	}

	return 0;
}

/*
 * tcpci: rx/tx buffer size(32Bytes): 0x50 ~ 0x6F
 * addr: size: 2Bytes
 */
#define SCHARGER_MAX_ACCESS_BUF_SIZE (32)
static int scharger_write_block(u16 addr, u8 *val, unsigned int num_bytes)
{
	int ret;
	unsigned int i;
	struct i2c_msg msgs;
	struct scharger_v700_priv *scharger = g_scharger;
	u8 buf[SCHARGER_MAX_ACCESS_BUF_SIZE + 2] = {0};

	if (scharger == NULL) {
		pr_err("%s scharger_v700_priv is NULL!\n", __func__);
		return -1;
	}

	if (num_bytes > SCHARGER_MAX_ACCESS_BUF_SIZE) {
		pr_err("%s too many bytes %d\n", __func__, num_bytes);
		return -1;
	}

	buf[0] = (addr >> 8);
	buf[1] = (addr & 0xFF);
	for (i = 0; i < num_bytes; i++)
		buf[ i + 2] = *(val + i);

	msgs.addr = scharger->client->addr;
	msgs.flags = 0;
	msgs.len = (num_bytes + 2);
	msgs.buf = (char *)buf;

	ret = i2c_transfer(scharger->client->adapter, &msgs, 1);
	if (ret != 1) {
		pr_err("%s transfer err: ret %d\n", __func__, ret);
		return ret;
	}

	return 0;
}

/* 0-success or others-fail */
static int scharger_write(u16 reg, u8 value)
{
	return scharger_write_block(reg, &value, 1);
}

/* 0-success or others-fail */
static int scharger_read(u16 reg, u8 *value)
{
	if (value == NULL) {
		pr_err("%s input is NULL!\n", __func__);
		return -ENOMEM;
	}

	return scharger_read_block(reg, value, 1);
}

static struct hisi_tcpc_reg_ops scharger_tcpc_reg_ops = {
	.block_read = scharger_read_block,
	.block_write = scharger_write_block,
};

static int scharger_regmap_read(void *context, unsigned int reg, unsigned int *val)
{
	int ret = 0;

	unused(context);
	ret = scharger_read((u16)reg, (u8 *)val);

	return 0;
}
static int scharger_regmap_write(void *context, unsigned int reg, unsigned int val)
{
	int ret = 0;

	unused(context);

	ret = scharger_write((u16)reg, (u8)val);

	return 0;
}

static const struct regmap_config scharger_regmap_config = {
	.reg_bits = 16,
	.val_bits = 8,
	.max_register = 0x7fff,
};

static void _scharger_irq_mask(struct irq_data *d, int maskflag)
{
	struct scharger_v700_priv *scharger = irq_data_get_irq_chip_data(d);
	u32 data = 0;
	u32 offset;
	int ret;

	if (!scharger) {
		pr_err(" irq_mask scharger is NULL\n");
		return;
	}

	/* Convert interrupt  data to interrupt offset */
	offset = (irqd_to_hwirq(d) / ONE_IRQ_GROUP_NUM);
	offset = (u32)scharger->irq_mask_addr_arry[offset];
	ret = regmap_read(scharger->regmap, offset, &data);
	if (ret < 0) {
		pr_err("%s regmap read interrupt offset 0x%x failed\n",
			__func__, offset);
		return;
	}
	if (maskflag == MASK)
		data |= (1 << (irqd_to_hwirq(d) & 0x07));
	else
		data &= ~(u32)(1 << (irqd_to_hwirq(d) & 0x07));

	ret = regmap_write(scharger->regmap, offset, data);
	if (ret < 0)
		pr_err("%s regmap write interrupt offset 0x%x failed\n",
			__func__, offset);
}

static void scharger_v700_irq_bus_unlock(struct irq_data *d)
{
	struct scharger_v700_priv *scharger = irq_data_get_irq_chip_data(d);

	if (!scharger) {
		pr_err(" %s scharger is NULL\n", __func__);
		return;
	}

	_scharger_irq_mask(d, scharger->irq_ops);

	mutex_unlock(&scharger->irq_lock);
}

static void scharger_v700_irq_bus_lock(struct irq_data *d)
{
	struct scharger_v700_priv *scharger = irq_data_get_irq_chip_data(d);

	if (!scharger) {
		pr_err(" %s scharger is NULL\n", __func__);
		return;
	}

	mutex_lock(&scharger->irq_lock);
}

static void scharger_irq_mask(struct irq_data *d)
{
	struct scharger_v700_priv *scharger = irq_data_get_irq_chip_data(d);

	scharger->irq_ops = MASK;
}

static void scharger_irq_unmask(struct irq_data *d)
{
	struct scharger_v700_priv *scharger = irq_data_get_irq_chip_data(d);

	scharger->irq_ops = UNMASK;
}

static struct irq_chip scharger_v700_irqchip = {
	.name = "schargerV700-irq",
	.irq_mask = scharger_irq_mask,
	.irq_unmask = scharger_irq_unmask,
	.irq_disable = scharger_irq_mask,
	.irq_enable = scharger_irq_unmask,
	.irq_bus_lock = scharger_v700_irq_bus_lock,
	.irq_bus_sync_unlock = scharger_v700_irq_bus_unlock,
};

static int scharger_irq_map(
	struct irq_domain *d, unsigned int virq, irq_hw_number_t hw)
{
	struct scharger_v700_priv *scharger = d->host_data;

	irq_set_chip_data(virq, scharger);
	irq_set_chip_and_handler(virq, &scharger_v700_irqchip, handle_edge_irq);
	irq_set_nested_thread(virq, 1);
	irq_set_noprobe(virq);

	return 0;
}

static const struct irq_domain_ops scharger_domain_ops = {
	.map = scharger_irq_map,
	.xlate = irq_domain_xlate_twocell,
};

static void scharger_force_clear_all_irq(struct scharger_v700_priv *scharger)
{
	unsigned int pending = 0;
	unsigned int reg;
	int i;

	/* coul irq base */
	reg = COUL_IRQ_BASE;
	for(i = 0; i < COUL_IRQ_REGS_NUM; i++) {
		(void)regmap_read(scharger->regmap, reg + i , &pending);
		if(pending) {
			pr_err(" %s 0x%x = 0x%x\n", __func__, reg + i, pending);
			(void)regmap_write(scharger->regmap, reg + i , CLEAR_ALL);
		}
	}

	/* Buck/OVP_PWS/EIS/Others/PD_JTAG irq base */
	reg = CHG_IRQ_BASE;
	for(i = 0; i < CHG_IRQ_REGS_NUM; i++) {
		(void)regmap_read(scharger->regmap, reg + i , &pending);
		if(pending) {
			pr_err(" %s 0x%x = 0x%x\n", __func__, reg + i, pending);
			(void)regmap_write(scharger->regmap, reg + i , CLEAR_ALL);
		}
	}

	/* PD irqs */
	reg = PD_IRQ_BASE;
	for(i = 0; i < PD_IRQ_REGS_NUM; i++) {
		(void)regmap_read(scharger->regmap, reg + i , &pending);
		if(pending) {
			pr_err(" %s 0x%x = 0x%x\n", __func__, reg + i, pending);
			(void)regmap_write(scharger->regmap, reg + i , CLEAR_ALL);
		}
	}

	(void)regmap_read(scharger->regmap, PD_FAULT_REG , &pending);
	if(pending) {
		pr_err(" %s 0x%x = 0x%x\n", __func__, reg + i, pending);
		(void)regmap_write(scharger->regmap, PD_FAULT_REG, CLEAR_ALL);
	}
}

static void scharger_v700_irq_distribute(struct scharger_v700_priv *scharger)
{
	unsigned int pending = 0;
	unsigned int mask = 0;
	unsigned long pending_s;
	unsigned int i, offset;
	int ret;
	int gpio_val;

	for (i = 0; i < scharger->irqarray; i++) {
		ret = regmap_read(scharger->regmap, scharger->irq_addr_arry[i], &pending);
		if (ret < 0) {
			pr_err("%s regmap read 0x%x failed\n", __func__,
				scharger->irq_addr_arry[i]);
			return;
		}

		ret = regmap_read(scharger->regmap, scharger->irq_mask_addr_arry[i], &mask);
		if (ret < 0) {
			pr_err("%s regmap read 0x%x failed\n", __func__,
				scharger->irq_mask_addr_arry[i]);
			return;
		}

		if (pending != 0)
			dev_err(scharger->dev, "mask = 0x%x, pending[%d] 0x%x=0x%x\n\r",
				mask, i, scharger->irq_addr_arry[i], pending);

		pr_err("[%s] pending 0x%x, mask 0x%x\n", __func__, pending, mask);
		pending &= (~mask);
		pending_s = (unsigned long)pending;
		if (pending_s) {
			for_each_set_bit(offset, &pending_s, SCHARGER_BITS)
				handle_nested_irq(scharger->irqs[offset + i * SCHARGER_BITS]);
		}
	}

	gpio_val = gpio_get_value(scharger->gpio);
	if (gpio_val == 0)
		(void)queue_delayed_work(system_highpri_wq, &scharger->irq_work, msecs_to_jiffies(IRQ_DELAY));
}

static irqreturn_t scharger_v700_irq_handler(int irq, void *data)
{
	struct scharger_v700_priv *scharger = (struct scharger_v700_priv *)data;

	scharger_v700_irq_distribute(scharger);

	return IRQ_HANDLED;
}

static int scharger_v700_irq_create_mapping(struct scharger_v700_priv *scharger)
{
	int i;
	unsigned int virq;

	for (i = 0; i < scharger->irqnum; i++) {
		virq = irq_create_mapping(scharger->domain, i);
		if (virq == NO_IRQ) {
			pr_err("Failed mapping hwirq\n");
			return -ENOSPC;
		}
		scharger->irqs[i] = virq;
		dev_info(scharger->dev, "[%s] scharger->irqs[%d] = %d\n", __func__, i,
			scharger->irqs[i]);
	}
	return 0;
}

static void scharger_v700_irq_prc(struct scharger_v700_priv *scharger)
{
	int i;
	unsigned int pending;
	int ret;

	for (i = 0; i < scharger->irqarray; i++) {
		ret = regmap_write(scharger->regmap, scharger->irq_mask_addr_arry[i], SCHARGER_MASK_STATE);
		if (ret < 0) {
			pr_err("%s regmap write 0x%x failed\n", __func__,
				scharger->irq_mask_addr_arry[i]);
			return;
		}
	}

	for (i = 0; i < scharger->irqarray; i++) {
		ret = regmap_read(scharger->regmap, scharger->irq_addr_arry[i], &pending);
		if (ret < 0) {
			pr_err("%s regmap read 0x%x failed\n", __func__,
				scharger->irq_addr_arry[i]);
			return;
		}

		pr_debug("Scharger IRQ address value:irq[0x%x] = 0x%x\n",
			scharger->irq_addr_arry[i], pending);
	}
}

static void scharger_irq_work(struct work_struct *work)
{
	struct scharger_v700_priv *scharger =
		container_of(work, struct scharger_v700_priv, irq_work.work);
	int gpio_val;
	static int retry_cnt = 0;
	unsigned int pending = 0;

	gpio_val = gpio_get_value(scharger->gpio);
	if (gpio_val != 0) {
		retry_cnt = 0;
		return;
	}

	retry_cnt++;
	pr_err("[%s] retry_cnt %d\n", __func__, retry_cnt);
	if(retry_cnt > IRQ_RETRY_CNT) {
		retry_cnt = 0;
		scharger_force_clear_all_irq(scharger);
		return;
	}

	(void)regmap_read(scharger->regmap, CHG_IRQ_FLAG, &pending);
	if (pending & CHG_IRQ_NONE_PD) {
		scharger_v700_irq_distribute(scharger);
	} else {
		(void)queue_delayed_work(system_highpri_wq, &scharger->irq_work, msecs_to_jiffies(IRQ_DELAY));
	}
}

static int scharger_v700_irq_register(
	struct i2c_client *client, struct scharger_v700_priv *scharger)
{
	int ret;
	enum of_gpio_flags flags;
	struct device *dev = &client->dev;
	struct device_node *np = dev->of_node;

	scharger->gpio = of_get_gpio_flags(np, 0, &flags);
	if (scharger->gpio < 0) {
		dev_err(dev, "failed to get_gpio_flags:%d\n", scharger->gpio);
		return scharger->gpio;
	}

	if (!gpio_is_valid(scharger->gpio))
		return -EINVAL;

	ret = gpio_request_one(scharger->gpio, GPIOF_IN, "schargerV700");
	if (ret < 0)
		dev_err(dev, "failed to request gpio%d, %d\n",
			scharger->gpio, ret);

	scharger->irq = gpio_to_irq(scharger->gpio);
	dev_info(dev, "%s gpio %d, irq %d\n", __func__, scharger->gpio, scharger->irq);

	/* mask && clear IRQ status */
	scharger_v700_irq_prc(scharger);

	scharger->irqs = (unsigned int *)devm_kmalloc(
		dev, scharger->irqnum * sizeof(int), GFP_KERNEL);
	if (!scharger->irqs)
		return -ENODEV;

	scharger->domain = irq_domain_add_simple(
		np, scharger->irqnum, 0, &scharger_domain_ops, scharger);
	if (!scharger->domain) {
		dev_err(dev, "failed irq domain add simple!\n");
		return -ENODEV;
	}

	ret = scharger_v700_irq_create_mapping(scharger);
	if (ret < 0)
		return ret;

	INIT_DELAYED_WORK(&scharger->irq_work, scharger_irq_work);

	ret = request_threaded_irq(scharger->irq, NULL, scharger_v700_irq_handler,
		IRQF_SHARED | IRQF_TRIGGER_FALLING | IRQF_NO_SUSPEND | IRQF_ONESHOT, "schargerV700", scharger);
	if (ret < 0) {
		dev_err(dev, "request irq fail %d\n", ret);
		return -ENODEV;
	}

	return 0;
}

static int get_scharger_v700_irq_data(
	struct i2c_client *client, struct scharger_v700_priv *scharger)
{
	int ret;
	struct device *dev = &client->dev;
	struct device_node *np = dev->of_node;

	/* get scharger irq num */
	ret = of_property_read_u32_array(
		np, "scharger-irq-num", &(scharger->irqnum), 1);
	if (ret) {
		pr_err("no scharger-irq-num property set\n");
		return -ENODEV;
	}

	/* get scharger irq array number */
	ret = of_property_read_u32_array(
		np, "scharger-irq-array", &(scharger->irqarray), 1);
	if (ret) {
		pr_err("no scharger-irq-array property set\n");
		return -ENODEV;
	}

	scharger->irq_mask_addr_arry = (int *)devm_kzalloc(dev,
		sizeof(int) * scharger->irqarray, GFP_KERNEL);
	if (!scharger->irq_mask_addr_arry)
		return -ENOMEM;

	ret = of_property_read_u32_array(np,
		"scharger-irq-mask-addr",
		(int *)scharger->irq_mask_addr_arry, scharger->irqarray);
	if (ret) {
		pr_err("no scharger-irq-mask-addr property set\n");
		return -ENODEV;
	}

	scharger->irq_addr_arry = (int *)devm_kzalloc(dev,
		sizeof(int) * scharger->irqarray, GFP_KERNEL);
	if (!scharger->irq_addr_arry)
		return -ENOMEM;

	ret = of_property_read_u32_array(np, "scharger-irq-addr",
		(int *)scharger->irq_addr_arry, scharger->irqarray);
	if (ret) {
		pr_err("no scharger-irq-addr property set\n");
		return -ENODEV;
	}
	return 0;
}

int get_gpio_int(void)
{
	struct scharger_v700_priv *scharger = g_scharger;
	if (!scharger) {
		pr_err("%s scharger is null\n", __func__);
		return -1;
	}
	return scharger->gpio;
}

/* 0-success or others-fail */
static int device_check(void)
{
	int ret;
	u32 chip_id = 0;
	struct scharger_v700_priv *scharger = g_scharger;

	if (!scharger) {
		pr_err("%s scharger is null\n", __func__);
		return -1;
	}

	/* byte length of version 0 chip id is 4 */
	ret = regmap_bulk_read(scharger->regmap, CHIP_VERSION_0, &chip_id, 4);
	if (ret) {
		dev_err(scharger->dev, "[%s]:read chip_id fail\n", __func__);
		return CHARGE_IC_BAD;
	}

	dev_info(scharger->dev, "%s, chip id is 0x%x\n", __func__, chip_id);
	return CHARGE_IC_GOOD;
}

static unsigned int get_device_version(struct scharger_v700_priv *scharger)
{
	unsigned int scharger_version = 0;

	if (scharger == NULL) {
		pr_err("%s scharger is null\n", __func__);
		return -EINVAL;
	}

	/* byte length of version 0 is 2 */
	(void)regmap_bulk_read(scharger->regmap, CHIP_VERSION_4,
				&scharger_version, 2);

	pr_info("%s, chip version is 0x%x\n", __func__, scharger_version);
	return scharger_version;
}

#ifdef CONFIG_DFX_DEBUG_FS
unsigned int scharger_test_read(unsigned int reg)
{
	struct scharger_v700_priv *scharger = g_scharger;
	unsigned int val = 0;
	int ret;

	if (!scharger) {
		pr_err("%s scharger is null\n", __func__);
		return -1;
	}

	ret = regmap_read(scharger->regmap, reg, &val);
	if (ret)
		pr_err("%s regmap_read 0x%x failed, ret %d\n", __func__, reg, ret);

	ret = scharger_regmap_read(scharger->regmap, reg, &val);
	if (ret < 0) {
		pr_err("%s regmap read 0x%x failed\n", __func__, reg);
		return ret;
	}
	pr_err("%s reg 0x%x = 0x%x\n", __func__, reg, val);
	return val;
}
unsigned int scharger_test_write(unsigned int reg, unsigned int val)
{
	struct scharger_v700_priv *scharger = g_scharger;
	int ret;

	if (!scharger) {
		pr_err("%s scharger is null\n", __func__);
		return -1;
	}

	pr_err("%s reg 0x%x = 0x%x\n", __func__, reg, val);
	ret = scharger_regmap_write(scharger->regmap, reg, val);
	if (ret < 0) {
		pr_err("%s regmap write 0x%x failed\n", __func__, reg);
		return ret;
	}
	return scharger_test_read(reg);
}
#endif

static void scharger_v700_check_coul_ctrl_off(struct device_node *np)
{
	int ret;
	struct device_node *coul_np = NULL;
	const char *str;

	coul_np = of_find_compatible_node(np, NULL, "hisilicon,schargerV700_coul");
	if (coul_np == NULL) {
		pr_err("%s hisilicon,schargerV700_coul unfind\n", __func__);
		return;
	}

	ret = of_property_read_string(coul_np, "status", &str);
	if (ret < 0) {
		pr_err("%s status unfind\n", __func__);
		return;
	}

	if (!strcmp(str, "disabled")) {
		regmap_update_bits(g_scharger->regmap, DEBUG_WRITE_PRO_REG, DEBUG_WRITE_PRO_MSK,
			 WRITE_PRO_UNPROTECTED << DEBUG_WRITE_PRO_SHIFT);
		regmap_update_bits(g_scharger->regmap, CLJ_DEBUG1_REG, COUL_CTRL_ONOFF_MSK,
			 COUL_OFF << COUL_CTRL_ONOFF_SHIFT);
		regmap_update_bits(g_scharger->regmap, DEBUG_WRITE_PRO_REG, DEBUG_WRITE_PRO_MSK,
			 WRITE_PRO_PROTECTED << DEBUG_WRITE_PRO_SHIFT);
	}
}

static int scharger_v700_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int ret;
	struct scharger_v700_priv *scharger = NULL;
	struct device_node *np = NULL;

	scharger = devm_kzalloc(&client->dev, sizeof(*scharger), GFP_KERNEL);
	if (!scharger) {
		dev_err(&client->dev, "[%s]scharger is null\n", __func__);
		return -ENOMEM;
	}
	scharger->dev = &client->dev;
	np = scharger->dev->of_node;
	scharger->client = client;
	i2c_set_clientdata(client, scharger);
	g_scharger = scharger;
	scharger->regmap = devm_regmap_init_i2c(client, &scharger_regmap_config);
	if (IS_ERR(scharger->regmap))
		return PTR_ERR(scharger->regmap);

	ret = device_check();
	if (ret == CHARGE_IC_BAD)
		goto fail;

	scharger->scharger_version = get_device_version(scharger);
	scharger_v700_check_coul_ctrl_off(np);

	mutex_init(&scharger->irq_lock);
	rt_mutex_init(&scharger->i2c_lock);

	if (scharger->scharger_version == CHIP_ID_V700)
		opt_regs_set(scharger_common_v700_option_regs,
			 ARRAY_SIZE(scharger_common_v700_option_regs));
	else
		opt_regs_set(scharger_common_v710_option_regs,
			 ARRAY_SIZE(scharger_common_v710_option_regs));

	ret = get_scharger_v700_irq_data(client, scharger);
	if (ret)
		return ret;

	ret = scharger_v700_irq_register(client, scharger);
	if (ret) {
		dev_err(scharger->dev, "[%s] irq register fail ret %d\n",
			__func__, ret);
		goto fail;
	}

	(void)hisi_tcpc_reg_ops_register(client, &scharger_tcpc_reg_ops);
	(void)hisi_tcpc_irq_gpio_register(client, scharger->gpio);

	ret = of_platform_populate(np, NULL, NULL, scharger->dev);
	if (ret) {
		dev_info(scharger->dev, "%s populate fail %d\n", __func__, ret);
		goto fail;
	} else {
		dev_info(scharger->dev, "%s success\n", __func__);
	}
	pr_err("[%s] succ\n", __func__);
	return ret;
fail:
	gpio_free(scharger->gpio);
	mutex_destroy(&scharger->irq_lock);
	g_scharger = NULL;
	return ret;
}

static int scharger_v700_remove(struct i2c_client *client)
{
	struct scharger_v700_priv *scharger = i2c_get_clientdata(client);

	if (!scharger)
		return -1;

	free_irq(scharger->irq, scharger);
	gpio_free(scharger->gpio);
	mutex_destroy(&scharger->irq_lock);
	devm_kfree(&client->dev, scharger);
	g_scharger = NULL;
	return 0;
}

MODULE_DEVICE_TABLE(i2c, scharger_v700_drv);
const static struct of_device_id scharger_v700_of_match[] = {
	{
		.compatible = "hisilicon,scharger-v700",
		.data = NULL,
	},
	{},
};

static struct i2c_driver scharger_v700_driver = {
	.probe = scharger_v700_probe,
	.remove = scharger_v700_remove,
	.driver = {
		.owner = THIS_MODULE,
		.name = "scharger_v700_drv",
		.of_match_table = of_match_ptr(scharger_v700_of_match),
	},
};

static int __init scharger_v700_init(void)
{
	int ret;

	ret = i2c_add_driver(&scharger_v700_driver);
	if (ret)
		pr_err("%s: i2c_add_driver error!!!\n", __func__);

	return ret;
}

static void __exit scharger_v700_exit(void)
{
	i2c_del_driver(&scharger_v700_driver);
}

subsys_initcall_sync(scharger_v700_init);
module_exit(scharger_v700_exit);
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("schargerV700 i2c driver");
