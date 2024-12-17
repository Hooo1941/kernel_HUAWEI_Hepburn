#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/gpio.h>
#include <linux/gpio/consumer.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <platform_include/cee/linux/adc.h>

#include <linux/miscdevice.h>
#include <linux/ioctl.h>

#define THERMAL_MAGIC 0xE0
#define READ_ADC_CHANNEL_VOLT _IOW(THERMAL_MAGIC, 0x1, int)
#define READ_ADC_CHANNEL_TEMP _IOW(THERMAL_MAGIC, 0x2, int)
#define READ_BOARD_ID _IO(THERMAL_MAGIC, 0x3)
#define READ_RTC_BATT_VOLT _IO(THERMAL_MAGIC, 0x4)

#define ARRAY_SIZE 26
#define MAX_BOARD_LEN 5
#define RTC_BATT_ADC 6
#define RTC_RATE 6

enum rtc_gpio_level {
	GPIO_OUT_LOW,
	GPIO_OUT_HIGH,
};

struct thermal_device {
	int rtc_gpio;
	struct miscdevice thermal_misc;
};

struct thermal_data {
	int volt;
	int temp;
};

struct thermal_device *g_thermal_dev = NULL;
struct thermal_data g_thermal_map[ARRAY_SIZE] = {{1619, -15000}, {1562, -10000},
{1493, -5000}, {1413, 0}, {1322, 5000}, {1223, 10000}, {1117, 15000}, {1008, 20000}, {900, 25000},
{795, 30000}, {697, 35000}, {606, 40000}, {524, 45000}, {451, 50000}, {387, 55000}, {332, 60000},
{284, 65000}, {243, 70000}, {208, 75000}, {179, 80000}, {153, 85000}, {132, 90000}, {114, 95000},
{99, 100000}, {86, 105000}, {74, 110000}};

static int get_adc_channel_temp(int adc_channel)
{
	int volt, temp;
	int i;
	volt = lpm_adc_get_value(adc_channel);
	if (volt < 0) {
		pr_err("adc_channel %d read volt failed\n", adc_channel);
		return volt;
	}

	if (volt > g_thermal_map[0].volt || volt < g_thermal_map[ARRAY_SIZE - 1].volt) {
		pr_err("adc volt is not in map\n");
		return -EINVAL;
	}

	for (i = 0; i < ARRAY_SIZE; i++)
		if (volt >= g_thermal_map[i].volt)
			break;

	temp = g_thermal_map[i - 1].temp + (volt - g_thermal_map[i - 1].volt) *
		(g_thermal_map[i].temp - g_thermal_map[i - 1].temp) /
		(g_thermal_map[i].volt - g_thermal_map[i - 1].volt);

	return temp;
}

static unsigned int get_base_board_id(void)
{
	struct device_node *np = NULL;
	unsigned int board_id = 0;
	unsigned int id[MAX_BOARD_LEN];
	int ret, i;

	np = of_find_node_by_path("/rootparam");
	if (np == NULL) {
		pr_err("of_find_node_by_path failed\n");
		return -ENODEV;
	}
	ret = of_property_read_u32_array(np, "hisi,boardid", id, MAX_BOARD_LEN);
	if (ret < 0) {
		pr_err("boardid not found\n");
		return ret;
	}

	for (i = 0; i < MAX_BOARD_LEN; i++)
		board_id = board_id * 10 + id[i];

	return board_id;
}

static int get_rtc_batt_volt()
{
	int volt;
	gpio_set_value(g_thermal_dev->rtc_gpio, GPIO_OUT_HIGH);
	mdelay(100);
	volt = lpm_adc_get_value(RTC_BATT_ADC);
	mdelay(100);
	gpio_set_value(g_thermal_dev->rtc_gpio, GPIO_OUT_LOW);

	return RTC_RATE * volt;
}

static int thermal_open(struct inode *inode, struct file *flip)
{
	return 0;
}

static int thermal_release(struct inode *inode, struct file *flip)
{
	return 0;
}

static long thermal_ioctl(struct file *flip, unsigned int cmd, unsigned long arg)
{
	int ret = 0;

	switch (cmd) {
	case READ_ADC_CHANNEL_VOLT:
		ret = lpm_adc_get_value(arg);
		break;
	case READ_ADC_CHANNEL_TEMP:
		ret = get_adc_channel_temp(arg);
		break;
	case READ_BOARD_ID:
		ret = get_base_board_id();
		break;
	case READ_RTC_BATT_VOLT:
		ret = get_rtc_batt_volt();
		break;
	default:
		pr_err("command not found\n");
		return -EINVAL;
	}

	return ret;
}

static struct file_operations thermal_fops = {
	.owner          = THIS_MODULE,
	.unlocked_ioctl = thermal_ioctl,
	.open			= thermal_open,
	.release		= thermal_release,
};

static int of_thermal_node_init(struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	struct thermal_device *tdev = platform_get_drvdata(pdev);
	int ret;

	ret = of_property_read_u32(np, "rtc-gpio", (u32 *)&tdev->rtc_gpio);
	if (ret < 0) {
		pr_err("rtc gpio not found\n");
		return ret;
	}

	ret = devm_gpio_request(&pdev->dev, tdev->rtc_gpio, "rtc-gpio");
	if (ret < 0) {
		pr_err("rtc gpio request gpio fail\n");
		return ret;
	}

	ret = gpio_direction_output(tdev->rtc_gpio, GPIO_OUT_LOW);
	if (ret < 0) {
		pr_err("rtc gpio direction output fail\n");
		return ret;
	}

	return 0;
}

static int thermal_probe(struct platform_device *pdev)
{
	struct thermal_device *tdev;
	int ret;

	tdev = devm_kzalloc(&pdev->dev, sizeof(struct thermal_device), GFP_KERNEL);
	if (!tdev)
		return -ENOMEM;

	platform_set_drvdata(pdev, tdev);

	ret = of_thermal_node_init(pdev);
	if (ret < 0)
		return ret;

	tdev->thermal_misc.minor = MISC_DYNAMIC_MINOR;
	tdev->thermal_misc.name = "thermal_interface";
	tdev->thermal_misc.fops = &thermal_fops;
	g_thermal_dev = tdev;

	ret = misc_register(&tdev->thermal_misc);
	if (ret < 0) {
		pr_err("thermal miscdevice register fail\n");
		return ret;
	}

	return 0;
}

static int thermal_remove(struct platform_device *pdev)
{
	struct thermal_device *tdev = platform_get_drvdata(pdev);
	misc_deregister(&tdev->thermal_misc);
	gpio_free(tdev->rtc_gpio);
	platform_set_drvdata(pdev, NULL);
	kfree(tdev);
	return 0;
}

static const struct of_device_id thermal_match[] = {
	{ .compatible = "huawei,thermal_interface" },
	{},
};
MODULE_DEVICE_TABLE(of, thermal_match);

struct platform_driver thermal_driver = {
	.probe		= thermal_probe,
	.remove		= thermal_remove,
	.driver		= {
		.name	= "thermal_driver",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(thermal_match),
	},
};

static int __init thermal_dev_init(void)
{
	return platform_driver_register(&thermal_driver);
}

static void __exit thermal_dev_exit(void)
{
	platform_driver_unregister(&thermal_driver);
}

module_init(thermal_dev_init);
module_exit(thermal_dev_exit);

MODULE_ALIAS("thermal interface module");
MODULE_DESCRIPTION("thermal interface driver");
MODULE_LICENSE("GPL");