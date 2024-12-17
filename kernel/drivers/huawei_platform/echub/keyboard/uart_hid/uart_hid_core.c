/*
 * Huawei uart hid Driver for device control
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
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
#include <linux/string.h>
#include <linux/list.h>
#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/wait.h>
#include <linux/mutex.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/pm.h>
#include <linux/device.h>
#include <linux/jiffies.h>
#include <linux/hid.h>
#include <linux/of.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/termios.h>
#include <securec.h>

#include "uart_hid_uart_protocol.h"
#include "uart_hid_report.h"
#include "uart_hid_uart_cmd.h"
#include "uart_hid_log.h"
#include "uart_hid_core.h"

static bool uart_is_suspend = false;

/* debug option */
static bool uart_hid_debug_flag = false;
bool uart_hid_debug_log_flag()
{
	return uart_hid_debug_flag;
}
module_param(uart_hid_debug_flag, bool, 0400);
MODULE_PARM_DESC(uart_hid_debug_flag, "print a lot of debug information");

int uart_hid_recv_buf(u8 *in_buf, int len)
{
	int ret;
	size_t len_out;
	ret = uart_hid_cmd_recv_report(in_buf, len, &len_out);
	if (ret < 0)
		return ret;

	return 0;
}

int uart_hid_send_buf(u8 *out_buf, int args_len)
{
	return uart_hid_cmd_send_report(out_buf, (size_t)args_len);
}

static irqreturn_t uart_hid_irq(int irq, void *dev_id)
{
	struct uart_hid *ihid = dev_id;
	int ret;
	int size = le16_to_cpu(ihid->hdesc.max_input_length);

	if (test_bit(UART_HID_READ_PENDING, &ihid->flags))
		return IRQ_HANDLED;

	if (uart_is_suspend)
		return IRQ_HANDLED;

	if (size > ihid->buf_size)
		size = ihid->buf_size;
	ret = uart_hid_recv_buf(ihid->input_buf, size);
	if (ret < 0) {
		uart_hid_err("got %d data instead of %d\n", ret, size);
		return IRQ_HANDLED;
	}

	uart_hid_input_report(ihid);

	return IRQ_HANDLED;
}

static int uart_hid_init_irq(struct uart_hid *ihid)
{
	int ret;

	uart_hid_dbg("Requesting IRQ: %d\n", ihid->pdata.irq_gpio);

	ret = gpio_request((unsigned int)ihid->pdata.irq_gpio, "uart_hid_irq");
	if (ret < 0) {
		uart_hid_err("Requesting gpio: %d,ret = %d\n",
			     ihid->pdata.irq_gpio, ret);
		return ret;
	}
	ihid->irq = gpio_to_irq(ihid->pdata.irq_gpio);
	ret = request_threaded_irq(ihid->irq, NULL, uart_hid_irq,
			IRQF_NO_SUSPEND | IRQF_TRIGGER_LOW |
			IRQF_ONESHOT,
			ihid->pdev->name, ihid);
	if (ret < 0) {
		uart_hid_err("Could not register for %s interrupt, irq = %d,"
			" ret = %d\n",
			ihid->pdev->name, ihid->irq, ret);
		return ret;
	}

	return 0;
}

#ifdef CONFIG_OF
static int uart_hid_of_dts_parse(struct platform_device *pdev,
			     struct uart_hid_platform_data *pdata)
{
	struct device_node *of_node = pdev->dev.of_node;
	u32 addr, baud, delay;
	int ret, gpio;
	const char *tty_name;

	ret = of_property_read_u32(of_node, "hid-descr-addr", &addr);
	if (ret || addr >> 16) {
		uart_hid_err("addr error ret = %d, add =%d\n", ret, addr);
		return -EINVAL;
	}
	gpio = of_get_named_gpio(of_node, "dev_irq", 0);
	if (!gpio_is_valid(gpio)) {
		uart_hid_err("dev_irq gpio: is invalid \n");
		return -EINVAL;
	}
	ret = of_property_read_string(of_node, "tty_name", &tty_name);
	if (ret) {
		uart_hid_err("tty_name dts error\n");
		return -EINVAL;
	}
	ret = of_property_read_u32(of_node, "tty_baud", &baud);
	if (ret) {
		uart_hid_err("baud dts error\n");
		return -EINVAL;
	}
	ret = of_property_read_u32(of_node, "post-power-on-delay-ms", &delay);
	if (ret) {
		uart_hid_err("post-power-on-delay-ms dts error\n");
		return -EINVAL;
	}

	pdata->hid_descriptor_address = addr;
	pdata->irq_gpio = gpio;
	pdata->tty_name = tty_name;
	pdata->tty_baud = baud;
	pdata->post_power_delay_ms = delay;

	pdata->supplies[0].supply = "vdd";
	pdata->supplies[1].supply = "vddl";

	ret = devm_regulator_bulk_get(&pdev->dev, ARRAY_SIZE(pdata->supplies),
			pdata->supplies);
	if (ret) {
		uart_hid_err("devm_regulator_bulk_get error ret=%d\n", ret);
		return ret;
	}

	return 0;
}

static const struct of_device_id uart_hid_of_match[] = {
	{ .compatible = "hid-over-uart" },
	{},
};
MODULE_DEVICE_TABLE(of, uart_hid_of_match);
#else
static inline int uart_hid_of_dts_parse(struct platform_device *pdev,
		struct uart_hid_platform_data *pdata)
{
	return -ENODEV;
}
#endif /* CONFIG_OF */

static int uart_hid_dev_init_check(struct uart_hid *ihid)
{
	int ret;
	ret = uart_hid_cmd_init(ihid->pdata.tty_name, ihid->pdata.tty_baud);
	if (ret < 0) {
		uart_hid_err("uart hid uart_hid_cmd_init error ret=%d\n", ret);
		return ret;
	}

	ret = uart_hid_dev_check();
	if (ret < 0) {
		uart_hid_err("uart hid uart_hid_dev_check error ret=%d\n", ret);
		goto err_cmd_init;
	}

	ret = uart_hid_init_irq(ihid);
	if (ret < 0)
		goto err_cmd_init;

	return 0;

err_cmd_init:
	uart_hid_cmd_deinit();
	uart_hid_err("error\n");
	return ret;
}

static int uart_hid_add_device(struct uart_hid *ihid)
{
	int ret;
	struct hid_device *hid;
	ret = uart_hid_fetch_hid_descriptor(ihid);
	if (ret < 0)
		return ret;

	hid = hid_allocate_device();
	if (IS_ERR(hid)) {
		ret = PTR_ERR(hid);
		return ret;
	}

	ihid->hid = hid;
	hid->driver_data = ihid;
	hid->ll_driver = get_uart_hid_ll_driver();
	hid->dev.parent = &ihid->pdev->dev;
	hid->bus = BUS_RS232;
	hid->version = le16_to_cpu(ihid->hdesc.bcd_version);
	hid->vendor = le16_to_cpu(ihid->hdesc.vendor_id);
	hid->product = le16_to_cpu(ihid->hdesc.product_id);

	ret = snprintf_s(hid->name, sizeof(hid->name), sizeof(hid->name) - 1,
		"%s %04X:%04X", ihid->pdev->name, (u16)hid->vendor,
		(u16)hid->product);
	if (ret < 0)
		goto err_mem_free;

	strlcpy(hid->phys, dev_name(&ihid->pdev->dev), sizeof(hid->phys));

	ret = hid_add_device(hid);
	if (ret) {
		if (ret != -ENODEV)
			uart_hid_err("can't add hid device: %d\n", ret);
		goto err_mem_free;
	}
	return ret;

err_mem_free:
	hid_destroy_device(hid);
	uart_hid_err("error\n");
	return ret;
}

static int uart_hid_probe(struct platform_device *pdev)
{
	int ret;
	struct uart_hid *ihid;
	__u16 hid_register;

	uart_hid_err("enter\n");
	ihid = devm_kzalloc(&pdev->dev, sizeof(*ihid), GFP_KERNEL);
	if (!ihid)
		return -ENOMEM;

	ret = uart_hid_of_dts_parse(pdev, &ihid->pdata);
	if (ret) {
		uart_hid_err("uart_hid_of_probe error \n");
		return ret;
	}

	ret = regulator_bulk_enable(ARRAY_SIZE(ihid->pdata.supplies),
			ihid->pdata.supplies);
	if (ret < 0) {
		uart_hid_err("regulator_bulk_enable error ret=%d\n", ret);
		return ret;
	}

	platform_set_drvdata(pdev, ihid);
	ihid->pdev = pdev;
	hid_register = ihid->pdata.hid_descriptor_address;
	ihid->hid_desc_reg = cpu_to_le16(hid_register);
	mutex_init(&ihid->reset_lock);

	device_enable_async_suspend(&pdev->dev);

	ret = uart_hid_dev_init_check(ihid);
	if (ret < 0)
		goto err_regulator;

	ret = uart_hid_add_device(ihid);
	if (ret < 0)
		goto err_dev_init_check;

	uart_hid_err("success\n");
	return 0;

err_dev_init_check:
	free_irq(ihid->irq, ihid);
	uart_hid_cmd_deinit();
err_regulator:
	regulator_bulk_disable(ARRAY_SIZE(ihid->pdata.supplies),
		ihid->pdata.supplies);
	uart_hid_err("error\n");
	return ret;
}

static int uart_hid_remove(struct platform_device *pdev)
{
	struct uart_hid *ihid = platform_get_drvdata(pdev);
	struct hid_device *hid;

	hid = ihid->hid;
	hid_destroy_device(hid);

	free_irq(ihid->irq, ihid);

	uart_hid_cmd_deinit();

	regulator_bulk_disable(ARRAY_SIZE(ihid->pdata.supplies),
			ihid->pdata.supplies);

	return 0;
}

static void uart_hid_shutdown(struct platform_device *pdev)
{
	struct uart_hid *ihid = platform_get_drvdata(pdev);
	free_irq(ihid->irq, ihid);
}

#ifdef CONFIG_PM_SLEEP
static int uart_hid_suspend(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct uart_hid *ihid = platform_get_drvdata(pdev);
	struct hid_device *hid = ihid->hid;
	int ret;
	int wake_status;

	if (hid->driver && hid->driver->suspend) {
		ret = hid->driver->suspend(hid, PMSG_SUSPEND);
		if (ret < 0)
			return ret;
	}

	/* The keyboard needs to wake up system,
    * so cannot disable its irq in S3. */
	enable_irq(ihid->irq);

	if (device_may_wakeup(&pdev->dev)) {
		wake_status = enable_irq_wake(ihid->irq);
		if (!wake_status)
			ihid->irq_wake_enabled = true;
		else
			uart_hid_err("Failed to enable irq wake: %d\n",
				wake_status);
	} else {
		regulator_bulk_disable(ARRAY_SIZE(ihid->pdata.supplies),
			ihid->pdata.supplies);
	}

	uart_is_suspend = true;
	return 0;
}

static int uart_hid_resume(struct device *dev)
{
	int ret;
	struct platform_device *pdev = to_platform_device(dev);
	struct uart_hid *ihid = platform_get_drvdata(pdev);
	struct hid_device *hid = ihid->hid;
	int wake_status;

	uart_is_suspend = false;

	if (!device_may_wakeup(&pdev->dev)) {
		ret = regulator_bulk_enable(ARRAY_SIZE(ihid->pdata.supplies),
				ihid->pdata.supplies);
		if (ret)
			uart_hid_dbg("Failed to enable supplies: %d\n", ret);

		if (ihid->pdata.post_power_delay_ms)
			msleep(ihid->pdata.post_power_delay_ms);
	} else if (ihid->irq_wake_enabled) {
		wake_status = disable_irq_wake(ihid->irq);
		if (!wake_status)
			ihid->irq_wake_enabled = false;
		else
			uart_hid_dbg("Failed to disable irq wake: %d\n",
				wake_status);
	}
	/* The irq of keyboard is not disabled in S3, so you
	* must call disable_irq before calling enable_irq. */
	disable_irq(ihid->irq);

	enable_irq(ihid->irq);

	if (hid->driver && hid->driver->reset_resume) {
		ret = hid->driver->reset_resume(hid);
		return ret;
	}

	return 0;
}
#endif

static const struct dev_pm_ops uart_hid_pm = { SET_SYSTEM_SLEEP_PM_OPS(
	uart_hid_suspend, uart_hid_resume) };

static const struct platform_device_id uart_hid_id_table[] = {
	{ "hid", 0 },
	{ "hid-over-uart", 0 },
	{},
};
MODULE_DEVICE_TABLE(platform, uart_hid_id_table);

static struct platform_driver uart_hid_driver = {
	.driver = {
		.name = "uart_hid",
		.pm = &uart_hid_pm,
		.of_match_table = of_match_ptr(uart_hid_of_match),
	},

	.probe = uart_hid_probe,
	.remove = uart_hid_remove,
	.shutdown = uart_hid_shutdown,
	.id_table = uart_hid_id_table,
};

static int __init uart_hid_init(void)
{
	printk("uart_hid_init init\n");
	return platform_driver_register(&uart_hid_driver);
}

static void __exit uart_hid_exit(void)
{
	printk("uart_hid_exit exit\n");
	platform_driver_unregister(&uart_hid_driver);
}

module_init(uart_hid_init);
module_exit(uart_hid_exit);

MODULE_DESCRIPTION("HID over UART core driver");
MODULE_AUTHOR("Benjamin Tissoires <benjamin.tissoires@gmail.com>");
MODULE_LICENSE("GPL");
