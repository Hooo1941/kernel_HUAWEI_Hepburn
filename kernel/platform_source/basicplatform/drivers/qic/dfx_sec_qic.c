/*
 * QIC. (QIC Mntn Module.)
 *
 * Copyright (c) 2021-2021 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */
#include <linux/compiler.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/interrupt.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_platform.h>
#include <securec.h>
#include "dfx_sec_qic.h"
#include "dfx_sec_qic_dump.h"
#if defined(CONFIG_DFX_SEC_QIC_V100)
#include "qic_v100/dfx_sec_qic_init.h"
#include "qic_v100/dfx_sec_qic_err_probe.h"
#elif defined(CONFIG_DFX_SEC_QIC_V300)
#include "qic_v300/dfx_sec_qic_init.h"
#include "qic_v300/dfx_sec_qic_err_probe.h"
#else
#endif
#ifdef CONFIG_DFX_DEBUG_FS
#include "platform_include/basicplatform/linux/dfx_mntn_test.h"
#endif

static struct dfx_sec_qic_device *g_qic_dev = NULL;
static struct of_device_id const dfx_sec_qic_match[] = {
	{
		.compatible = "dfx,sec_qic",
		.data = (void *)NULL
	},
	{}
};

struct dfx_sec_qic_device *dfx_sec_qic_get_dev(void)
{
	return g_qic_dev;
}

static irqreturn_t dfx_sec_qic_irq_handler(int irq, void *data)
{
	struct dfx_sec_qic_device *qic_dev = (struct dfx_sec_qic_device *)data;
	dfx_sec_qic_get_errinfo(qic_dev, true);
	dfx_sec_qic_reset_handler();
	return IRQ_HANDLED;
}

int dfx_sec_qic_get_component_reg(const struct device_node *qic_node, struct platform_device *pdev,
				     struct qic_src_reg **component_reg, u32 *component_reg_num, const char *name)
{
	u32 i;
	int ret;
	const struct device_node *np = NULL;
	char reg_field[QIC_REG_FIELD_NAME_SIZE] = {0};

	np = of_find_compatible_node((struct device_node *)qic_node, NULL, name);
	if (unlikely(!np)) {
		pr_err("[%s] cannot get %s node\n", __func__, name);
		return -ENODEV;
	}

	if (of_property_read_u32(np, "reg_num", component_reg_num) != 0) {
		pr_err("[%s] get %s reg_num from DTS error.\n", __func__, name);
		return -ENODEV;
	}

	*component_reg = devm_kzalloc(&pdev->dev,
		*component_reg_num * sizeof(*(*component_reg)), GFP_KERNEL);
	if (unlikely(!(*component_reg))) {
		pr_err("[%s] alloc %s fail\n", __func__, name);
		return -ENOMEM;
	}

	for (i = 0; i < *component_reg_num; i++) {
		(*component_reg)[i].reg_base = of_iomap((struct device_node *)np, i);
		if (unlikely(!((*component_reg)[i].reg_base))) {
			pr_err("[%s] %s:%u map fail\n", __func__, name, i);
			return -ENOMEM;
		}

		ret = snprintf_s(reg_field, sizeof(reg_field), sizeof(reg_field) - 1, "len%u", i);
		if (ret < 0) {
			pr_err("[%s] combine reg:%u len fail\n", __func__, i);
			return ret;
		}
		if (of_property_read_u32(np, (const char *)reg_field, &((*component_reg)[i].len)) != 0) {
			pr_err("[%s] get %s:%u len from DTS error.\n", __func__, name, i);
			return -ENODEV;
		}

		(*component_reg)[i].reg_mask = devm_kzalloc(&pdev->dev,
			(*component_reg)[i].len * sizeof(*((*component_reg)[i].reg_mask)), GFP_KERNEL);
		if (unlikely(!((*component_reg)[i].reg_mask))) {
			pr_err("[%s] alloc %s mask fail\n", __func__, name);
			return -ENOMEM;
		}
		ret = snprintf_s(reg_field, sizeof(reg_field), sizeof(reg_field) - 1, "masks%u", i);
		if (ret < 0) {
			pr_err("[%s] combine reg:%u mask fail\n", __func__, i);
			return ret;
		}
		if (of_property_read_u32_array(np, (const char *)reg_field,
			(*component_reg)[i].reg_mask, (*component_reg)[i].len) != 0) {
			pr_err("[%s] get %s:%u mask from DTS error.\n", __func__, name, i);
			return -ENODEV;
		}

		(*component_reg)[i].offsets = devm_kzalloc(&pdev->dev,
			(*component_reg)[i].len * sizeof(*((*component_reg)[i].offsets)), GFP_KERNEL);
		if (unlikely(!((*component_reg)[i].offsets))) {
			pr_err("[%s] alloc %s offset fail\n", __func__, name);
			return -ENOMEM;
		}
		ret = snprintf_s(reg_field, sizeof(reg_field), sizeof(reg_field) - 1, "offsets%u", i);
		if (ret < 0) {
			pr_err("[%s] combine reg:%u offset fail\n", __func__, i);
			return ret;
		}
		if (of_property_read_u32_array(np, (const char *)reg_field,
			(*component_reg)[i].offsets, (*component_reg)[i].len) != 0) {
			pr_err("[%s] get %s:%u offset from DTS error.\n", __func__, name, i);
			return -ENODEV;
		}
	}

	return 0;
}

static int dfx_sec_qic_irq_init(const struct device_node *qic_node, struct platform_device *pdev)
{
	int ret;
	struct dfx_sec_qic_device *qic_dev = platform_get_drvdata(pdev);

	if (unlikely(!qic_dev)) {
		dev_err(&pdev->dev, "cannot get qic dev\n");
		return -ENODEV;
	}

	ret = dfx_sec_qic_get_component_reg(qic_node, pdev, &(qic_dev->irq_reg),
		&(qic_dev->irq_reg_num), "dfx,sec_qic_irq_reg");
	if (ret) {
		dev_err(&pdev->dev, "cannot get qic irq reg\n");
		return ret;
	}

	ret = platform_get_irq(pdev, 0);
	if (ret < 0) {
		dev_err(&pdev->dev, "cannot find qic irq\n");
		return ret;
	}
	qic_dev->irq_number = (u32)ret;

	ret = devm_request_irq(&pdev->dev, qic_dev->irq_number, dfx_sec_qic_irq_handler,
		IRQF_TRIGGER_HIGH, "dfx_sec_qic", qic_dev);
	if (ret < 0) {
		dev_err(&pdev->dev, "request qic_irq fail!\n");
		return ret;
	}
	return 0;
}

static int dfx_sec_qic_probe(struct platform_device *pdev)
{
	struct dfx_sec_qic_device *qic_dev = NULL;
	const struct device_node *np = NULL;
	int ret;

	if (!pdev)
		return -ENOMEM;

	qic_dev = devm_kzalloc(&pdev->dev, sizeof(struct dfx_sec_qic_device), GFP_KERNEL);
	if (!qic_dev)
		return -ENOMEM;

	platform_set_drvdata(pdev, qic_dev);
	np = pdev->dev.of_node;

	ret = dfx_sec_qic_common_init(np, pdev);
	if (ret)
		goto qic_probe_err;

	ret = dfx_sec_qic_irq_init(np, pdev);
	if (ret)
		goto qic_probe_err;

	g_qic_dev = qic_dev;
	dev_notice(&pdev->dev, "qic init success!\n");
#ifdef CONFIG_DFX_SEC_QIC_V100
	qice_regs_print_init();
#endif
	return 0;

qic_probe_err:
	dfx_sec_qic_free_source(qic_dev);
	g_qic_dev = NULL;
	dev_err(&pdev->dev, "qic init fail!");
	return ret;
}

static int dfx_sec_qic_remove(struct platform_device *pdev)
{
	struct dfx_sec_qic_device *qic_dev = NULL;

	if (!pdev)
		return -ENOMEM;

	qic_dev = platform_get_drvdata(pdev);
	if (qic_dev != NULL)
		free_irq(qic_dev->irq_number, qic_dev);

	dfx_sec_qic_free_source(qic_dev);
	return 0;
}

MODULE_DEVICE_TABLE(of, dfx_sec_qic_match);

#if ((defined CONFIG_DFX_BB_DEBUG) && !(defined CONFIG_LIBLINUX))
void sec_qic_shutdown(struct platform_device *dev)
{
	mntn_test_shutdown_deadloop();
}
#endif

static struct platform_driver dfx_sec_qic_driver = {
	.probe = dfx_sec_qic_probe,
	.remove = dfx_sec_qic_remove,
	.driver = {
		   .name = QIC_MODULE_NAME,
		   .owner = THIS_MODULE,
		   .of_match_table = of_match_ptr(dfx_sec_qic_match),
		   },
#if ((defined CONFIG_DFX_BB_DEBUG) && !(defined CONFIG_LIBLINUX))
	.shutdown = sec_qic_shutdown,
#endif
};

static int __init dfx_sec_qic_init(void)
{
	return platform_driver_register(&dfx_sec_qic_driver);
}

static void __exit dfx_sec_qic_exit(void)
{
	platform_driver_unregister(&dfx_sec_qic_driver);
}

late_initcall(dfx_sec_qic_init);
module_exit(dfx_sec_qic_exit);
MODULE_LICENSE("GPL v2");
