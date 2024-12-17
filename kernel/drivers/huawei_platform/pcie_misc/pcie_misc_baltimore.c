/*
 * pcie_misc_baltimore.c
 *
 * Copyright (c) 2012-2021 Huawei Technologies Co., Ltd.
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
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/ioctl.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/usb.h>
#include <linux/io.h>
#include <linux/device.h>
#include <linux/platform_drivers/pcie-kport-api.h>
#include <linux/delay.h>
#include "pcie_misc.h"

struct pcie_misc {
	struct device *dev;
	u32 pcie_rc_idx;

	u32 pdpu_time; // Power down to power up (ms)
	u32 pvperl_time; // Power stable to PERSTB inactive (ms)

	int reset_gpio;
	u32 reset_gpio_op; // Pull or push to reset EP

	int power_gpio;
	u32 poweron_gpio_op; // Pull or push to poweron EP
	u32 poweroff_gpio_op;

	int wake_gpio;

	u32 s3_is_off;	 // Flag indicating whether S3 is powered off
};

static irqreturn_t pcieusb_wakeup_ep_handler(int irq, void *dev_id)
{
	pciedev_info("PCIe wakeup endpoint rc");
	return IRQ_HANDLED;
}

static int pcieusb_register_wakeup_gpio(struct platform_device *pdev)
{
	int ret;
	int irqn;
	struct pcie_misc *pmisc = platform_get_drvdata(pdev);

	pciedev_info("+%s+\n", __func__);

	ret = gpio_direction_input(pmisc->wake_gpio);
	if (ret != 0) {
		pciedev_err("wake-gpio set direction input failed\n");
		return ret;
	}

	irqn = gpio_to_irq(pmisc->wake_gpio);
	if (irqn < 0) {
		pciedev_err("gpio_to_irq failed!\n");
		gpio_free(pmisc->wake_gpio);
		return -1;
	}

	pciedev_info("requesting IRQ %d with IRQF_NO_SUSPEND\n", irqn);

	ret = request_irq(irqn, pcieusb_wakeup_ep_handler,
					IRQF_TRIGGER_FALLING | IRQF_NO_SUSPEND, "PCIE_WAKE", NULL);
	if (ret) {
		pciedev_err("request_irq failed\n");
		return ret;
	}

	ret = enable_irq_wake(irqn);
	if (ret != 0) {
		pciedev_err("enable_irq_wake failed\n");
		return ret;
	}

	pciedev_info("-%s-\n", __func__);
	return 0;
}

static int pcie_dev_poweroff(struct platform_device *pdev)
{
	struct pcie_misc *pmisc = platform_get_drvdata(pdev);

	pciedev_info("+%s+\n", __func__);
	if (pmisc->power_gpio < 0) {
		pciedev_err("Failed to get power-gpio number.\n");
		return -1;
	}
	gpio_set_value(pmisc->power_gpio, pmisc->poweroff_gpio_op);

	pciedev_info("-%s-\n", __func__);
	return 0;
}

static int pcie_dev_poweron(struct platform_device *pdev)
{
	struct pcie_misc *pmisc = platform_get_drvdata(pdev);

	pciedev_info("+%s+\n", __func__);
	if (pmisc->power_gpio < 0) {
		pciedev_err("Failed to get power-gpio number.\n");
		return -1;
	}
	gpio_set_value(pmisc->power_gpio, pmisc->poweron_gpio_op);

	pciedev_info("-%s-\n", __func__);
	return 0;
}

static int pcie_dev_reset(struct platform_device *pdev)
{
	int ret;
	struct pcie_misc *pmisc = platform_get_drvdata(pdev);

	pciedev_info("+%s+\n", __func__);

	if (pmisc->reset_gpio > 0) {
		ret = gpio_direction_output(pmisc->reset_gpio, pmisc->reset_gpio_op); // Reset ep
		if (ret != 0) {
			pciedev_err("Failed to set reset-gpio output.\n");
			return ret;
		}

		if (pmisc->power_gpio > 0 && pmisc->pdpu_time > 0) {
			ret = gpio_direction_output(pmisc->power_gpio, pmisc->poweroff_gpio_op); // Poweroff ep
			if (ret != 0) {
				pciedev_err("Failed to set power-gpio output poweroff.\n");
				return ret;
			}
			mdelay(pmisc->pdpu_time);   // Delay between power down and power on
			gpio_set_value(pmisc->power_gpio, pmisc->poweron_gpio_op);  // Poweron ep
		}

		if (pmisc->pvperl_time > 0)
			mdelay(pmisc->pvperl_time); // Delay between power stable and PERSTB
	}

	pciedev_info("-%s-\n", __func__);
	return 0;
}

static int init_pmisc_wake_gpio(struct platform_device *pdev)
{
	int ret;
	bool isval;
	struct pcie_misc *pmisc = platform_get_drvdata(pdev);

	pmisc->wake_gpio = of_get_named_gpio(pmisc->dev->of_node, "wake-gpio", 0);
	if (pmisc->wake_gpio < 0) {
		pciedev_err("Get wake-gpio from DTS failed!\n");
		return -EINVAL;
	}

	isval = gpio_is_valid(pmisc->wake_gpio);
	if (!isval) {
		pciedev_err("wake-gpio is not valid.\n");
		return -EINVAL;
	}

	ret = devm_gpio_request(pmisc->dev, pmisc->wake_gpio, "pcie_misc_wake");
	if (ret) {
		pciedev_err("Failed to request wake_gpio %d.\n", pmisc->wake_gpio);
		return ret;
	}

	return 0;
}

static int init_pmisc_power_gpio(struct platform_device *pdev)
{
	int ret;
	bool isval;
	struct pcie_misc *pmisc = platform_get_drvdata(pdev);

	pmisc->power_gpio = of_get_named_gpio(pmisc->dev->of_node, "power-gpio", 0);
	if (pmisc->power_gpio < 0) {
		pciedev_err("Get power-gpio from DTS failed!\n");
		return -EINVAL;
	}

	isval = gpio_is_valid(pmisc->power_gpio);
	if (!isval) {
		pciedev_err("power-gpio is not valid.\n");
		return -EINVAL;
	}

	ret = devm_gpio_request(pmisc->dev, pmisc->power_gpio, "pcie_misc_power");
	if (ret) {
		pciedev_err("Failed to request power-gpio %d.\n", pmisc->power_gpio);
		return ret;
	}

	return 0;
}

static int init_pmisc_reset_gpio(struct platform_device *pdev)
{
	int ret;
	bool isval;
	struct pcie_misc *pmisc = platform_get_drvdata(pdev);

	pmisc->reset_gpio = of_get_named_gpio(pmisc->dev->of_node, "reset-gpio", 0);
	if (pmisc->reset_gpio < 0) {
		pciedev_err("Get reset-gpio from DTS failed!\n");
		return -EINVAL;
	}

	isval = gpio_is_valid(pmisc->reset_gpio);
	if (!isval) {
		pciedev_err("reset-gpio %d is not valid.\n", pmisc->reset_gpio);
		return -EINVAL;
	}

	ret = devm_gpio_request(pmisc->dev, pmisc->reset_gpio, "pcie_misc_reset");
	if (ret) {
		pciedev_err("Failed to request reset-gpio %d.\n", pmisc->reset_gpio);
		return ret;
	}

	return 0;
}

static void pcie_misc_timing_init(struct platform_device *pdev)
{
	int ret;
	struct pcie_misc *pmisc = platform_get_drvdata(pdev);

	ret = of_property_read_u32(pmisc->dev->of_node, "pdpu_time", &pmisc->pdpu_time);
	if (ret != 0) {
		pciedev_err("Get pmisc pdpu_time failed! ret = %d.\n", ret);
		pmisc->pdpu_time = 0;
	}

	ret = of_property_read_u32(pmisc->dev->of_node, "pvperl_time", &pmisc->pvperl_time);
	if (ret != 0) {
		pciedev_err("Get pmisc pvperl_time failed! ret = %d.\n", ret);
		pmisc->pvperl_time = 0;
	}

	ret = of_property_read_u32(pmisc->dev->of_node, "s3_is_off", &pmisc->s3_is_off);
	if (ret != 0)
		pr_info("can't get s3_is_off.\n");
}

static int pcie_misc_data_init(struct platform_device *pdev)
{
	int ret;
	struct pcie_misc *pmisc = platform_get_drvdata(pdev);

	pciedev_info("+%s+\n", __func__);

	ret = of_property_read_u32(pmisc->dev->of_node, "pcie_rc_idx", &pmisc->pcie_rc_idx);
	if (ret != 0) {
		pciedev_err("Get pcie_rc_idx from DTS failed! ret = %d\n", ret);
		return ret;
	}

	ret = of_property_read_u32(pmisc->dev->of_node, "reset_gpio_op", &pmisc->reset_gpio_op);
	if (ret != 0) {
		pmisc->reset_gpio_op = 0;
		pciedev_err("Get reset_gpio_op failed! Set value %d defaultly\n", pmisc->reset_gpio_op);
	}

	ret = of_property_read_u32(pmisc->dev->of_node, "poweron_gpio_op", &pmisc->poweron_gpio_op);
	if (ret != 0) {
		pmisc->poweron_gpio_op = 0;
		pmisc->poweroff_gpio_op = 1;
		pciedev_err("Get poweron_gpio_op failed! Set value %d defaultly\n", pmisc->poweron_gpio_op);
	} else {
		pmisc->poweroff_gpio_op = (pmisc->poweron_gpio_op == 0) ? 1 : 0;
	}

	pcie_misc_timing_init(pdev);

	ret = init_pmisc_reset_gpio(pdev);
	if (ret != 0)
		pciedev_err("Init pmisc reset-gpio failed! ret = %d\n", ret);

	ret = init_pmisc_power_gpio(pdev);
	if (ret != 0)
		pciedev_err("Init pmisc power-gpio failed! ret = %d\n", ret);

	ret = init_pmisc_wake_gpio(pdev);
	if (ret != 0)
		pciedev_err("Init pmisc wake-gpio failed! ret = %d\n", ret);

	pciedev_info("-%s-\n", __func__);
	return 0;
}

static int pcie_misc_probe(struct platform_device *pdev)
{
	int ret;
	int power_status;
	struct pcie_misc *pmisc = NULL;

	pciedev_info("+%s+\n", __func__);

	pmisc = devm_kzalloc(&pdev->dev, sizeof(struct pcie_misc), GFP_KERNEL);
	if (!pmisc)
		return -ENOMEM;

	pmisc->dev = &pdev->dev;
	platform_set_drvdata(pdev, pmisc);

	ret = pcie_misc_data_init(pdev);
	if (ret != 0) {
		pciedev_err("Init pcie misc data failed! ret = %d\n", ret);
		return ret;
	}

	if (pmisc->power_gpio > 0) {
		power_status = gpio_get_value(pmisc->power_gpio);
		if (power_status == pmisc->poweroff_gpio_op) {
			pciedev_err("Stop PCIe enumerate due to EP poweroff.\n");
			return -1;
		}
	}

	ret = pcie_dev_reset(pdev);
	if (ret != 0)
		pciedev_err("Pcie device reset failed! ret = %d\n", ret);

	ret = pcie_kport_enumerate(pmisc->pcie_rc_idx);
	if (ret != 0) {
		pciedev_err("Pcie enumerate failed! ret = %d\n", ret);
		if (pcie_dev_poweroff(pdev))
			pciedev_err("Fail to poweroff device!\n");
		else
			pciedev_err("Poweroff device success.\n");
	}

	if (pmisc->wake_gpio > 0) {
		ret = pcieusb_register_wakeup_gpio(pdev);
		if (ret != 0)
			pciedev_err("pcieusb_register_wakeup_gpio failed.ret = %d\n", ret);
	}

	pciedev_info("-%s-\n", __func__);
	return 0;
}

static int pcie_misc_remove(struct platform_device *pdev)
{
	struct pcie_misc *pmisc = platform_get_drvdata(pdev);

	pciedev_info("+%s+\n", __func__);

	if (pmisc == NULL)
		return -ENODEV;

	platform_set_drvdata(pdev, NULL);
	kfree(pmisc);
	pmisc = NULL;

	pciedev_info("-%s-\n", __func__);
	return 0;
}

static int pcie_misc_pm_suspend(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct pcie_misc *pmisc = platform_get_drvdata(pdev);
	pciedev_info("+%s+\n", __func__);

	if (pmisc == NULL)
		return -ENODEV;

	if (pmisc->s3_is_off)
		pcie_dev_poweroff(pdev);

	pciedev_info("-%s-\n", __func__);
	return 0;
}

static int pcie_misc_pm_resume(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct pcie_misc *pmisc = platform_get_drvdata(pdev);
	pciedev_info("+%s+\n", __func__);

	if (pmisc == NULL)
		return -ENODEV;

	if (pmisc->s3_is_off)
		pcie_dev_poweron(pdev);

	pciedev_info("-%s-\n", __func__);
	return 0;
}

static const struct dev_pm_ops pcie_misc_pm_ops = {
	SET_NOIRQ_SYSTEM_SLEEP_PM_OPS(pcie_misc_pm_suspend, pcie_misc_pm_resume)
};

static const struct of_device_id pcie_misc_match[] = {
	{ .compatible = "huawei,hw_pcie" },
	{ .compatible = "huawei,hw_pcie1" },
	{},
};

static struct platform_driver pcie_misc_driver = {
	.probe      = pcie_misc_probe,
	.remove   = pcie_misc_remove,
	.driver   = {
		.name   = "pcie_misc_l5",
		.owner = THIS_MODULE,
		.pm = &pcie_misc_pm_ops,
		.of_match_table = of_match_ptr(pcie_misc_match),
	},
};

static int __init pcie_misc_init(void)
{
	return platform_driver_register(&pcie_misc_driver);
}

static void __exit pcie_misc_exit(void)
{
	platform_driver_unregister(&pcie_misc_driver);
}

late_initcall(pcie_misc_init);
module_exit(pcie_misc_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("huawei pcie misc control module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
