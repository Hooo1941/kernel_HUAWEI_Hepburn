#include <linux/err.h>
#include <linux/gpio.h>
#include <linux/gpio/consumer.h>
#include <linux/kernel.h>
#include <linux/leds.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/property.h>
#include <linux/slab.h>
#include <linux/timer.h>

enum ufs_led_level {
	GPIO_OUT_LOW,
	GPIO_OUT_HIGH,
};

struct ufs_led_data {
	int led_gpio;
	struct led_classdev cdev;
	unsigned long delay_on;
	unsigned long delay_off;
	struct timer_list timer;
	struct device dev;
};

static void led_gpio_set_value(struct ufs_led_data *led_data)
{
	if (led_data->cdev.brightness == LED_OFF)
		gpio_set_value(led_data->led_gpio, GPIO_OUT_LOW);
	else
		gpio_set_value(led_data->led_gpio, GPIO_OUT_HIGH);
}

static void led_blink_timer(struct timer_list *tmr)
{
	unsigned long expires;
	struct ufs_led_data *led_data = from_timer(led_data, tmr, timer);

	if (led_data->cdev.brightness == LED_OFF) {
		led_data->cdev.brightness = LED_FULL;
		expires = jiffies + msecs_to_jiffies(led_data->delay_on);
	} else {
		led_data->cdev.brightness = LED_OFF;
		expires = jiffies + msecs_to_jiffies(led_data->delay_off);
	}

	led_gpio_set_value(led_data);
	mod_timer(&led_data->timer, expires);
}

static void ufs_led_brightness_set(
	struct led_classdev *led_cdev, enum led_brightness brightness)
{
	struct ufs_led_data *led_data =
		container_of(led_cdev, struct ufs_led_data, cdev);

	del_timer(&led_data->timer);

	led_data->cdev.brightness = brightness;
	led_gpio_set_value(led_data);
}

static int ufs_led_blink_set(struct led_classdev *led_cdev,
	unsigned long *delay_on, unsigned long *delay_off)
{
	struct ufs_led_data *led_data =
		container_of(led_cdev, struct ufs_led_data, cdev);
	int expires;

	if (*delay_on > 0 || *delay_off > 0) {
		led_data->delay_on = *delay_on;
		led_data->delay_off = *delay_off;

		if (led_data->cdev.brightness == LED_OFF)
			expires = jiffies + msecs_to_jiffies(*delay_off);
		else
			expires = jiffies + msecs_to_jiffies(*delay_on);

		led_gpio_set_value(led_data);
		mod_timer(&led_data->timer, expires);
	}

	return 0;
}

static int of_ufs_led_init(struct ufs_led_data *led_data)
{
	struct device_node *np = led_data->dev.of_node;
	int ret;

	ret = of_property_read_u32(np, "led_gpio", (u32 *)&led_data->led_gpio);
	if (ret < 0) {
		dev_err(&led_data->dev, "led_gpio not found\n");
		return ret;
	}

	ret = devm_gpio_request(&led_data->dev, led_data->led_gpio, "ufs_led_gpio");
	if (ret < 0) {
		dev_err(&led_data->dev, "ufs led request gpio fail\n");
		return ret;
	}

	ret = gpio_direction_output(led_data->led_gpio, GPIO_OUT_LOW);
	if (ret < 0) {
		dev_err(&led_data->dev, "ufs led direction output fail\n");
		return ret;
	}

	return 0;
}

static int ufs_led_probe(struct platform_device *pdev)
{
	struct ufs_led_data *dat;
	int ret;

	dat = devm_kzalloc(&pdev->dev, sizeof(struct ufs_led_data), GFP_KERNEL);
	if (!dat)
		return -ENOMEM;

	dat->dev = pdev->dev;
	
	ret = of_ufs_led_init(dat);
	if (ret < 0)
		return ret;

	dat->cdev.name = "ufs_led";
	dat->cdev.default_trigger = "timer";
	dat->cdev.max_brightness = LED_FULL;
	dat->cdev.brightness = LED_OFF;
	dat->cdev.brightness_set = ufs_led_brightness_set;
	dat->cdev.blink_set = ufs_led_blink_set;

	timer_setup(&dat->timer, led_blink_timer, 0);
	platform_set_drvdata(pdev, dat);

	ret = led_classdev_register(&pdev->dev, &dat->cdev);
	if (ret < 0) {
		led_classdev_unregister(&dat->cdev);
		dev_err(&pdev->dev, "led_classdev register fail\n");
		return ret;
	}

	return 0;
}

static int ufs_led_remove(struct platform_device *pdev)
{
	struct ufs_led_data *led_data = platform_get_drvdata(pdev);
	gpio_free(led_data->led_gpio);
	del_timer(&led_data->timer);
	led_classdev_unregister(&led_data->cdev);
	platform_set_drvdata(pdev, NULL);
	kfree(led_data);
	return 0;
}

static const struct of_device_id ufs_led_match[] = {
	{ .compatible = "huawei,ufs_led" },
	{},
};
MODULE_DEVICE_TABLE(of, ufs_led_match);

struct platform_driver ufs_led_driver = {
	.probe		= ufs_led_probe,
	.remove		= ufs_led_remove,
	.driver		= {
		.name	= "ufs_led_driver",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(ufs_led_match),
	},
};

static int __init ufs_led_init(void)
{
	return platform_driver_register(&ufs_led_driver);
}

static void __exit ufs_led_exit(void)
{
	platform_driver_unregister(&ufs_led_driver);
}
module_init(ufs_led_init);
module_exit(ufs_led_exit);

MODULE_ALIAS("ufs led control module");
MODULE_DESCRIPTION("ufs led control driver");
MODULE_LICENSE("GPL");