/*
 * Button.c
 *
 * Powerbutton/LID Button driver for echub
 *
 * Copyright (c) 2018-2019 Huawei Technologies Co., Ltd.
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
#include <linux/input.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/clk.h>
#include <linux/jiffies.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/pinctrl/consumer.h>
#include <linux/pm_wakeup.h>
#include <asm/irq.h>
#include <linux/uaccess.h>
#include <linux/proc_fs.h>
#include <linux/i2c.h>
#include <linux/suspend.h>
#include "../echub.h"
#include "../echub_i2c.h"

#ifdef CONFIG_ARMPC_ECHUB_LID_TEST
#include <linux/miscdevice.h>
#include <linux/ioctl.h>
#include "ec_state_sync.h"
#endif

#define GPIO_KEY_PRESS                 (1)
#define GPIO_KEY_RELEASE               (0)

#define GPIO_HIGH_VOLTAGE              (1)
#define GPIO_LOW_VOLTAGE               (0)
#define TIMER_DEBOUNCE                 (900)
#define TIMER_REPORT_DELAY             (100)
#define WAKELOCK_DELAY_TIME            (1000)

#define MFG_MODE_ACCESS_ADDR	0x0408
#define MFG_MODE_ADDR	0x0489
#define READ_LID_FLAG_ADDR	0x04e9

#ifdef CONFIG_ARMPC_ECHUB_LID_TEST
#define DEVICE_NAME                 "lid"
#define LID_MAGIC                   0xEF
#define WRITE_LID_FLAG_ADDR         0x0479
#define OPEN_LID                    (_IO(LID_MAGIC, 0x1))
#define CLOSE_LID                   (_IO(LID_MAGIC, 0x0))
#define READ_LID_STATUS             (_IO(LID_MAGIC, 0x2))
#define READ_SLEEP_COUNT            (_IO(LID_MAGIC, 0x3))
#endif

#define POWER_KEY_RELEASE 0
#define POWER_KEY_PRESS 1

struct hw_gpio_key {
	struct echub_i2c_dev *echub_dev;
	struct input_dev *input_dev;
	struct device *wakeup_dev;

	struct delayed_work report_lid_closed_event_work;
	struct timer_list key_lid_timer;
	struct delayed_work power_event_work;
	struct delayed_work report_lid_open_event_work;
	int gpio_lid;
	int lid_irq;
	bool lid_close_wakeup_enable;
};

#ifdef CONFIG_ARMPC_ECHUB_LID_TEST
static struct hw_gpio_key *g_hw_gpio_key = NULL;
static int dis_irq_flag = 0;
#endif
/* is the lid closed */
static bool g_lid_closed = false;
/* is the pc in s3 */
static bool g_suspended = false;
/* Should access to S3 be prohibited */
static bool g_abort_suspend = false;

/**********************************************************
*  Function:       hw_gpio_key_open
*  Discription:    open gpiokey
*  Parameters:     input_dev *dev
*  return value:   0-sucess or others-fail
**********************************************************/
static int hw_gpio_key_open(struct input_dev *dev)
{
	/* add input device open function */
	echub_info("%s\n", __func__);
	return 0;
}

/**********************************************************
*  Function:       hw_gpio_key_close
*  Discription:    close gpiokey
*  Parameters:     input_dev *dev
*  return value:   0-sucess or others-fail
**********************************************************/
static void hw_gpio_key_close(struct input_dev *dev)
{
	/* add input device close function */
	echub_info("%s\n", __func__);
}

/**********************************************************
*  Function:       report_lid_switch
*  Discription:    report lid event
*  Parameters:     input_dev *dev, int value
*  return value:   void
**********************************************************/
static inline void report_lid_switch(struct input_dev *dev, int value)
{
	input_report_switch(dev, SW_LID, value);
	input_sync(dev);
}

/**********************************************************
*  Function:       report_power_event
*  Discription:    report power event
*  Parameters:     input_dev *dev
*  return value:   void
**********************************************************/
static inline void report_power_event(struct input_dev *dev)
{
	input_report_key(dev, KEY_POWER, POWER_KEY_PRESS);
	input_sync(dev);
	input_report_key(dev, KEY_POWER, POWER_KEY_RELEASE);
	input_sync(dev);
}

/**********************************************************
*  Function:       hw_report_lid_close_work
*  Discription:    lid report closed work
*  Parameters:     work_struct *work
*  return value:   0-sucess or others-fail
**********************************************************/
static void hw_report_lid_close_work(struct work_struct *work)
{
	struct hw_gpio_key *gpio_key = container_of(work,
		struct hw_gpio_key, report_lid_closed_event_work.work);

	report_lid_switch(gpio_key->input_dev, GPIO_KEY_PRESS);
	g_lid_closed = true;
}

/**********************************************************
*  Function:       hw_report_lid_open_work
*  Discription:    report lid open work
*  Parameters:     work_struct *work
*  return value:   0-sucess or others-fail
**********************************************************/
static void hw_report_lid_open_work(struct work_struct *work)
{
	struct hw_gpio_key *gpio_key = container_of(work,
		struct hw_gpio_key, report_lid_open_event_work.work);

	report_lid_switch(gpio_key->input_dev, GPIO_KEY_RELEASE);
	g_lid_closed = false;
}

static void gpio_lid_timer(struct timer_list *t)
{
	g_abort_suspend = false;
	pm_system_wakeup();
}

static void hw_report_power_event_work(struct work_struct *work)
{
	struct hw_gpio_key *gpio_key = container_of(work,
		struct hw_gpio_key, power_event_work.work);

	report_power_event(gpio_key->input_dev);
	g_lid_closed = false;
}

/**********************************************************
*  Function:       hw_gpio_key_irq_handler
*  Discription:    gpio irq handler
*  Parameters:     int irq, void *dev_id
*  return value:   void
**********************************************************/
static irqreturn_t hw_gpio_key_irq_handler(int irq, void *dev_id)
{
	struct hw_gpio_key *gpio_key = (struct hw_gpio_key *)dev_id;
	int key_value = 0;
	int ret;

	key_value = gpio_get_value((unsigned int)gpio_key->gpio_lid);
	echub_info("irq_handler hander key_value:%d g_lid_closed:%d g_abort_suspend:%d g_suspended:%d\n",
		key_value,
		g_lid_closed,
		g_abort_suspend,
		g_suspended);
	if (key_value == GPIO_LOW_VOLTAGE) {
		pm_wakeup_dev_event(gpio_key->wakeup_dev, TIMER_DEBOUNCE + WAKELOCK_DELAY_TIME, true);
		schedule_delayed_work(&gpio_key->report_lid_closed_event_work, msecs_to_jiffies(TIMER_DEBOUNCE));
		return IRQ_HANDLED;
	}

	pm_wakeup_dev_event(gpio_key->wakeup_dev, TIMER_REPORT_DELAY + WAKELOCK_DELAY_TIME, true);
	cancel_delayed_work_sync(&gpio_key->report_lid_closed_event_work);
	if (g_lid_closed) {
		g_lid_closed = false;
		g_abort_suspend = true;
		mod_timer(&(gpio_key->key_lid_timer), jiffies + msecs_to_jiffies(TIMER_DEBOUNCE));
		schedule_delayed_work(&gpio_key->report_lid_open_event_work, msecs_to_jiffies(TIMER_REPORT_DELAY));
	} else if (g_suspended) {
		schedule_delayed_work(&gpio_key->power_event_work, msecs_to_jiffies(TIMER_REPORT_DELAY));
	}

	return IRQ_HANDLED;
}

/**********************************************************
*  Function:       set_lid_status
*  Discription:    Enable or disable the LID irq based on the MFG mode and LID flag.
*  Parameters:     struct hw_gpio_key *gpio_key
*  return value:   0-sucess or others-fail
**********************************************************/
static int set_lid_status(struct hw_gpio_key *gpio_key)
{
	int ret = 0;
	char value[4] = {0};

	/* Enter MFG test mode */
	ret = gpio_key->echub_dev->write_func(gpio_key->echub_dev, MFG_MODE_ACCESS_ADDR, 1);
	if (ret)
		echub_err("Enter MFG test mode failed: %d\n", ret);

	/* Read MFG mode */
	ret = gpio_key->echub_dev->read_func(gpio_key->echub_dev, MFG_MODE_ADDR, 0, value, 4);
	if (ret)
		echub_err("Read MFG mode failed: %d\n", ret);

	echub_info("Read MFG mode: ret = %d value = %*ph\n", ret, 4, value);

	/* Exist MFG test mode */
	ret = gpio_key->echub_dev->write_func(gpio_key->echub_dev, MFG_MODE_ACCESS_ADDR, 0);
	if (ret)
		echub_err("Exist MFG test mode failed: %d\n", ret);

	if (value[2] == 1) {
		echub_info("The current mode is MFG mode\n");
		/* Read lid flag */
		ret = gpio_key->echub_dev->read_func(gpio_key->echub_dev, READ_LID_FLAG_ADDR, 0, value, 4);
		echub_info("Get lid flag form EC. ret = %d value = %*ph\n", ret, 4, value);
		if (ret)
			echub_info("Read lid flag failed: %d\n", ret);

		if (value[2] == 1) {
			echub_info("Disable lid irq\n");
			disable_irq(gpio_key->lid_irq);
#ifdef CONFIG_ARMPC_ECHUB_LID_TEST
			dis_irq_flag = 1;
#endif
		}
	}

	return ret;
}

#ifdef CONFIG_OF
static const struct of_device_id hw_gpio_key_match[] = {
	{ .compatible = "huawei,echub-lid" },
	{},
};
MODULE_DEVICE_TABLE(of, hw_gpio_key_match);
#endif

static struct input_dev *init_input_dev(struct platform_device *pdev, void *data)
{
	struct input_dev *input_dev = NULL;

	if (!pdev)
		return NULL;

	input_dev = input_allocate_device();
	if (input_dev == NULL) {
		echub_err("Failed to allocate struct input_dev!\n");
		return NULL;
	}

	input_dev->name = pdev->name;
	input_dev->id.bustype = BUS_HOST;
	input_dev->dev.parent = &pdev->dev;
	input_set_drvdata(input_dev, data);

	set_bit(EV_SW, input_dev->evbit);
	set_bit(EV_SYN, input_dev->evbit);
	set_bit(SW_LID, input_dev->swbit);
	set_bit(EV_KEY, input_dev->evbit);
	set_bit(KEY_POWER, input_dev->keybit);

	input_dev->open = hw_gpio_key_open;
	input_dev->close = hw_gpio_key_close;

	return input_dev;
}

static int init_lid_gpio(struct platform_device *pdev, struct hw_gpio_key *gpio_key)
{
	int err = 0;

	if (!pdev || !gpio_key)
		return -EINVAL;

	/* initial work before we use it. */
	INIT_DELAYED_WORK(&(gpio_key->report_lid_closed_event_work), hw_report_lid_close_work);
	INIT_DELAYED_WORK(&(gpio_key->report_lid_open_event_work), hw_report_lid_open_work);
	INIT_DELAYED_WORK(&(gpio_key->power_event_work), hw_report_power_event_work);

	gpio_key->lid_close_wakeup_enable = of_property_read_bool(pdev->dev.of_node, "lid-close-wakeup-enable");

	/* inital gpio_lid */
	err = of_property_read_u32(pdev->dev.of_node, "gpio-key-lid", (u32 *)&gpio_key->gpio_lid);
	if (err) {
		gpio_key->gpio_lid = 0;
		echub_info("%s: Not support gpio_key lid\n", __func__);
		return -EFAULT;
	} else {
		echub_info("%s: Support gpio_key lid: %d\n", __func__, gpio_key->gpio_lid);
	}

	if (!gpio_is_valid(gpio_key->gpio_lid)) {
		echub_err("%s: gpio of lid is not valid, check DTS\n", __func__);
		return -EFAULT;
	}

	err = devm_gpio_request(&pdev->dev, (unsigned int)gpio_key->gpio_lid, "gpio_lid");
	if (err < 0) {
		echub_err("Fail request gpio:%d\n", gpio_key->gpio_lid);
		return -EFAULT;
	}

	err = gpio_direction_input((unsigned int)gpio_key->gpio_lid);
	if (err < 0) {
		echub_err("Failed to set gpio_lid directoin!\n");
		return -EFAULT;
	}

	gpio_key->lid_irq = gpio_to_irq((unsigned int)gpio_key->gpio_lid);
	if (gpio_key->lid_irq < 0) {
		echub_err("Failed to get gpio key press irq!\n");
		err = gpio_key->lid_irq;
		return -EFAULT;
	}

	timer_setup(&(gpio_key->key_lid_timer), gpio_lid_timer, 0);

	err = request_irq(gpio_key->lid_irq, hw_gpio_key_irq_handler,
		IRQF_NO_SUSPEND | IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, pdev->name, gpio_key);
	if (err) {
		echub_err("Failed to request press interupt handler!\n");
		return -EFAULT;
	}

	return 0;
}

/**********************************************************
*  Function:       hw_lid_is_closed
*  Discription:    get lid status
*  return value:   true-closed or false-open
**********************************************************/
bool hw_lid_is_closed()
{
    return g_lid_closed;
}
EXPORT_SYMBOL(hw_lid_is_closed);

/**********************************************************
*  Function:       hw_gpio_key_probe
*  Discription:    module probe
*  Parameters:     pdev:platform_device
*  return value:   0-sucess or others-fail
**********************************************************/
static int hw_gpio_key_probe(struct platform_device *pdev)
{
	struct echub_i2c_dev *echub_dev = dev_get_drvdata(pdev->dev.parent);
	struct hw_gpio_key *gpio_key = NULL;
	struct input_dev *input_dev = NULL;
	int err = 0;

	if (echub_dev == NULL) {
		echub_err("echub_dev is null, get echub_dev form tmp\n");
		echub_dev = get_echub_dev();
		if (echub_dev == NULL) {
			echub_err("echub_dev is NULL return\n");
			return -ENODEV;
		}
	}

	echub_info("armpc gpio key driver probes start!\n");

	gpio_key = devm_kzalloc(&pdev->dev, sizeof(struct hw_gpio_key), GFP_KERNEL);
	if (gpio_key == NULL) {
		echub_info("Failed to allocate struct hw_gpio_key!\n");
		return -ENOMEM;
	}

	input_dev = init_input_dev(pdev, gpio_key);
	if (input_dev == NULL)
		return -ENOMEM;

	gpio_key->input_dev = input_dev;
	gpio_key->echub_dev = echub_dev;
	gpio_key->wakeup_dev = &pdev->dev;

	err = init_lid_gpio(pdev, gpio_key);
	if (err)
		goto err_lid_gpio;

	err = input_register_device(gpio_key->input_dev);
	if (err) {
		echub_err("Failed to register input device!\n");
		goto err_register_dev;
	}

	device_init_wakeup(&pdev->dev, true);
	platform_set_drvdata(pdev, gpio_key);

#ifdef CONFIG_ARMPC_ECHUB_LID_TEST
	g_hw_gpio_key = gpio_key;
#endif
	err = set_lid_status(gpio_key);
	if (err)
		echub_info("Can't set lid status\n");

	echub_info("armpc gpio key driver probes successfully!\n");
	return 0;

err_register_dev:
	free_irq(gpio_key->lid_irq, gpio_key);
err_lid_gpio:
	input_free_device(input_dev);
	echub_err("[gpiokey] gpio key probe failed! ret = %d.\n", err);
	return err;
}

/**********************************************************
*  Function:       hw_gpio_key_remove
*  Discription:    module remove
*  Parameters:     pdev:platform_device
*  return value:   0-sucess or others-fail
**********************************************************/
static int hw_gpio_key_remove(struct platform_device *pdev)
{
	struct hw_gpio_key *gpio_key = platform_get_drvdata(pdev);

	if (gpio_key == NULL) {
		echub_err("get invalid gpio_key pointer\n");
		return -EINVAL;
	}

	free_irq(gpio_key->lid_irq, gpio_key);
	gpio_free((unsigned int)gpio_key->gpio_lid);
	cancel_delayed_work(&(gpio_key->report_lid_open_event_work));
	cancel_delayed_work(&(gpio_key->report_lid_closed_event_work));
	cancel_delayed_work(&(gpio_key->power_event_work));

	input_unregister_device(gpio_key->input_dev);
	platform_set_drvdata(pdev, NULL);

	kfree(gpio_key);
	gpio_key = NULL;
	return 0;
}

#ifdef CONFIG_PM
/**********************************************************
*  Function:       hw_gpio_key_suspend
*  Discription:    module remove
*  Parameters:     pdev:platform_device pm_message_t state
*  return value:   0-sucess or others-fail
**********************************************************/
static int hw_gpio_key_suspend(struct platform_device *pdev, pm_message_t state)
{
	int err;
	struct hw_gpio_key *gpio_key = platform_get_drvdata(pdev);
	if (g_abort_suspend) {
		echub_err("abort suspend\n");
		return -EFAULT;
	}
	g_suspended = true;
	if (gpio_key->lid_close_wakeup_enable)
		irq_set_irq_type(gpio_key->lid_irq, IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING);
	else
		irq_set_irq_type(gpio_key->lid_irq, IRQF_TRIGGER_RISING);
	return 0;
}

/**********************************************************
*  Function:       hw_gpio_key_resume
*  Discription:    module remove
*  Parameters:     pdev:platform_device
*  return value:   0-sucess or others-fail
**********************************************************/
static int hw_gpio_key_resume(struct platform_device *pdev)
{
	int err;
	struct hw_gpio_key *gpio_key = platform_get_drvdata(pdev);
	g_suspended = false;
	irq_set_irq_type(gpio_key->lid_irq, IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING);
	return 0;
}
#endif

struct platform_driver hw_gpio_key_driver = {
	.probe = hw_gpio_key_probe,
	.remove = hw_gpio_key_remove,
	.driver = {
		.name = "hw_gpio_key",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(hw_gpio_key_match),
	},
#ifdef CONFIG_PM
	.suspend = hw_gpio_key_suspend,
	.resume = hw_gpio_key_resume,
#endif
};

#ifdef CONFIG_ARMPC_ECHUB_LID_TEST
static int open_lid(struct hw_gpio_key *gpio_key)
{
	int ret;
	if (!dis_irq_flag)
		return -1;
	enable_irq(gpio_key->lid_irq);

	ret = gpio_key->echub_dev->write_func(gpio_key->echub_dev, WRITE_LID_FLAG_ADDR, 0);
	if (ret)
		printk("write 0x0 to EC failed\n");

	return 0;
}

static int close_lid(struct hw_gpio_key *gpio_key)
{
	int ret;
	if (dis_irq_flag)
		return -1;
	disable_irq(gpio_key->lid_irq);
	ret = gpio_key->echub_dev->write_func(gpio_key->echub_dev, WRITE_LID_FLAG_ADDR, 1);
	if (ret)
		printk("write 0x1 to EC failed\n");
	return 0;
}

static int read_lid_status(struct hw_gpio_key *gpio_key)
{
	int lid_status;

	lid_status = gpio_get_value((unsigned int)gpio_key->gpio_lid);
	return !!lid_status;
}

static int read_sleep_count(void)
{
	return get_sleep_count();
}

static int lid_open(struct inode *inode, struct file *flip)
{
	flip->private_data = g_hw_gpio_key;
	return 0;
}

static int lid_release(struct inode *inode, struct file *flip)
{
	return 0;
}

static long lid_ioctl(struct file *flip, unsigned int cmd, unsigned long arg)
{
	int ret = 0;

	switch (cmd) {
	case OPEN_LID:
		ret = open_lid(g_hw_gpio_key);
		if (!ret)
			dis_irq_flag = 0;
		break;
	case CLOSE_LID:
		ret = close_lid(g_hw_gpio_key);
		if (!ret)
			dis_irq_flag = 1;
		break;
	case READ_LID_STATUS:
		ret = read_lid_status(g_hw_gpio_key);
		if (ret != 0 && ret != 1)
			echub_err("lid_dev: read lid status error\n");
		break;
	case READ_SLEEP_COUNT:
		ret = read_sleep_count();
		if (ret < 0)
			echub_err("read sleep count error\n");
		break;
	default:
		echub_err("lid_dev ioctl cmd error: open: 1; close: 0; read: 2\n");
		return -EINVAL;
	}
	return ret;
}

static struct file_operations lid_dev_fops = {
	.owner          = THIS_MODULE,
	.unlocked_ioctl = lid_ioctl,
	.open			= lid_open,
	.release		= lid_release,
};

static struct miscdevice lid_dev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name  = DEVICE_NAME,
	.fops  = &lid_dev_fops,
};
#endif

/**********************************************************
*  Function:       hw_gpio_key_init
*  Discription:    module initialization
*  Parameters:     pdev:platform_device
*  return value:   0-sucess or others-fail
**********************************************************/
int __init hw_gpio_key_init(void)
{
	int ret;

	echub_info("%s\n", __func__);
	ret = platform_driver_register(&hw_gpio_key_driver);
	if (ret)
		echub_err("lid_dev misc_register err\n");

#ifdef CONFIG_ARMPC_ECHUB_LID_TEST
	ret = misc_register(&lid_dev);
	if (ret)
		echub_err("lid_dev misc_register err\n");
#endif
	return ret;
}

/**********************************************************
*  Function:       hw_gpio_key_exit
*  Discription:    module exit
*  Parameters:     NULL
*  return value:   NULL
**********************************************************/
void __exit hw_gpio_key_exit(void)
{
	echub_info("%s\n", __func__);
#ifdef CONFIG_ARMPC_ECHUB_LID_TEST
	misc_deregister(&lid_dev);
#endif
	platform_driver_unregister(&hw_gpio_key_driver);
}

module_init(hw_gpio_key_init);
module_exit(hw_gpio_key_exit);

MODULE_ALIAS("armpc echub button module");
MODULE_DESCRIPTION("armpc echub button driver");
MODULE_LICENSE("GPL");
