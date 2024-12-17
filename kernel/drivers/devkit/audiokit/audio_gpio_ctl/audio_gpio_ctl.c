/*
 * audio_gpio_ctl.c
 *
 * audio_gpio_ctl driver
 *
 * Copyright (c) 2022-2023 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include "audio_gpio_ctl.h"

#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/ioctl.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/of_gpio.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/of_device.h>
#include <linux/regulator/consumer.h>
#include <linux/pinctrl/consumer.h>
#include <linux/workqueue.h>

static struct audio_gpio_ctrl_priv *g_audio_gpio_priv = NULL;

static void audio_gpio_parse_dts(struct device_node *node,
	struct audio_gpio_group* agg);

static struct audio_gpio_group *find_audio_group_by_name(const char *name)
{
	unsigned int i;

	if (!name) {
		pr_err("%s: group name is NULL\n", __func__);
		return NULL;
	}

	if (g_audio_gpio_priv->group_num <= 0) {
		pr_err("%s: no support audio group\n", __func__);
		return NULL;
	}

	for (i = 0; i < g_audio_gpio_priv->group_num; i++) {
		if (strncmp(g_audio_gpio_priv->audio_groups[i].group_name, name, strlen(name)) == 0)
			return &g_audio_gpio_priv->audio_groups[i];
	}

	return NULL;
}

static int audio_gpio_set_value(struct audio_gpio_group *agg, bool force)
{
	u8 mode;
	u8 bit_val;
	int ret;
	unsigned int i;

	if (!agg->gpios)
		return -1;

	mode = agg->group_mode[agg->cur_mode];
	for  (i = 0; i < agg->gpio_num; i++) {
		bit_val = (mode >> i) & 0x01;

		if (!force && agg->gpios[i].state == bit_val)
			continue;

		if (!gpio_is_valid(agg->gpios[i].gpio))
			continue;

		ret = gpio_direction_output(agg->gpios[i].gpio, bit_val);
		if (ret < 0) {
			pr_info("%s: set gpio %d fail\n", __func__, agg->gpios[i].gpio);
			return -1;
		}
		pr_info("%s: pull gpio=%d to %u success\n", __func__, agg->gpios[i].gpio, bit_val);

		agg->gpios[i].state = bit_val;
	}

	return 0;
}

static int audio_gpio_ctl(void __user *arg)
{
	int ret;
	struct audio_gpio_info audio_gpio_info;
	struct audio_gpio_group *agg = NULL;
	struct device_node *group_node = NULL;
	char name[AUDIO_GPIO_GROUP_NAME_MAX] = { 0 };
	int set_mode;

	if (!arg) {
		pr_err("%s: invalid argument\n", __func__);
		return -EINVAL;
	}

	ret = copy_from_user(&audio_gpio_info, arg, sizeof(struct audio_gpio_info));
	if (ret != 0) {
		pr_err("%s: copy_from_user fail\n", __func__);
		return ret;
	}

	if (strcmp(audio_gpio_info.gpio_mode, "default") == 0) {
		set_mode = AUDIO_GPIO_DEFAULT_MODE;
	} else if (strcmp(audio_gpio_info.gpio_mode, "active") == 0) {
		set_mode = AUDIO_GPIO_ACTIVE_MODE;
	} else {
		pr_err("%s: audio_group mode invalid\n", __func__);
		return -1;
	}

	agg = find_audio_group_by_name(audio_gpio_info.group_name);
	if (!agg) {
		pr_err("%s: audio_group is NULL\n", __func__);
		return -1;
	}

	if (agg->cur_mode == set_mode)
		return 0;
	agg->cur_mode = set_mode;

	snprintf(name, AUDIO_GPIO_GROUP_NAME_MAX, "group%d", agg->group_id);
	group_node = of_get_child_by_name(g_audio_gpio_priv->dev->of_node, name);
	if (!group_node) {
		pr_err("%s: get group node fail\n", __func__);
		return -1;
	}
	audio_gpio_parse_dts(group_node, agg);

	return audio_gpio_set_value(agg, false);
}

static int audio_gpio_ctl_do_ioctl(struct file *file, unsigned int cmd,
	void __user *arg, int compat_mode)
{
	int ret = 0;

	if (!file || !file->private_data) {
		pr_err("%s: invalid argument\n", __func__);
		return -EINVAL;
	}

	if (!g_audio_gpio_priv) {
		pr_err("%s: g_audio_gpio_priv is NULL\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&g_audio_gpio_priv->do_ioctl_lock);
	ret = audio_gpio_ctl(arg);
	mutex_unlock(&g_audio_gpio_priv->do_ioctl_lock);

	return ret;
}

static long audio_gpio_ctl_ioctl(struct file *file, unsigned int command,
	unsigned long arg)
{
	return audio_gpio_ctl_do_ioctl(file, command, (void __user *)(uintptr_t)arg, 0);
}

#ifdef CONFIG_COMPAT
static long audio_gpio_ctl_ioctl_compat(struct file *file,
	unsigned int command, unsigned long arg)
{
	return audio_gpio_ctl_do_ioctl(file, command,
		compat_ptr((unsigned int)arg), 1);
}
#else
#define audio_gpio_ctl_ioctl_compat NULL /* function pointer */
#endif /* CONFIG_COMPAT */

static const struct file_operations audio_gpio_ctl_fops = {
	.owner          = THIS_MODULE,
	.open           = simple_open,
	.unlocked_ioctl = audio_gpio_ctl_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl   = audio_gpio_ctl_ioctl_compat,
#endif /* CONFIG_COMPAT */
};

static struct miscdevice audio_gpio_ctl_miscdev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name  = "audio_gpio_ctl",
	.fops  = &audio_gpio_ctl_fops,
};

static int audio_gpio_group_get_prop_of_str_type(struct device_node *node,
	const char *key_str, const char **dst)
{
	int ret = 0;

	if (!dst) {
		pr_err("%s: invalid argument\n", __func__);
		return -EINVAL;
	}

	if (!of_property_read_bool(node, key_str)) {
		pr_err("%s: %s not existed\n", __func__, key_str);
		return -EINVAL;
	}

	ret = of_property_read_string(node, key_str, dst);
	if (ret < 0)
		pr_err("%s: get %s failed\n", __func__, key_str);

	return ret;
}

static int audio_gpio_group_get_prop_of_u32_type(struct device_node *node,
	const char *key_str, int *value)
{
	int ret = 0;

	if (!value) {
		pr_err("%s: invalid argument\n", __func__);
		return -EINVAL;
	}

	if (!of_property_read_bool(node, key_str)) {
		pr_err("%s: %s not existed\n", __func__, key_str);
		return -EINVAL;
	}

	ret = of_property_read_u32(node, key_str, value);
	if (ret < 0)
		pr_err("%s: get %s failed\n", __func__, key_str);

	return ret;
}

static void audio_gpio_parse_mode(struct device_node *node,
	struct audio_gpio_group *agg)
{
	u32 val;
	int ret;
	unsigned int i;
	char mode_arr[AUDIO_GPIO_MODE_NUM_MAX][AUDIO_GPIO_MODE_MAX] = { "default", "active" };

	for (i = 0; i < AUDIO_GPIO_MODE_NUM_MAX; i++) {
		ret = audio_gpio_group_get_prop_of_u32_type(node, mode_arr[i], &val);
		if (ret) {
			pr_err("%s: fail to read %s mode\n", __func__, mode_arr[i]);
			return;
		}
		agg->group_mode[i] = (u8)(val);
	}

	audio_gpio_set_value(agg, true);
}

static void audio_gpio_parse_dts(struct device_node *node,
	struct audio_gpio_group *agg)
{
	int ret;
	unsigned int i;
	char name[AUDIO_GPIO_NAME_MAX] = { 0 };

	if (agg->group_register)
		return;

	agg->gpios = kzalloc(agg->gpio_num * sizeof(struct audio_gpio), GFP_KERNEL);
	if (!agg->gpios)
		return;

	for (i = 0; i < agg->gpio_num; i++) {
		snprintf(name, AUDIO_GPIO_GROUP_NAME_MAX, "gpio%d", i);
		agg->gpios[i].gpio = of_get_named_gpio(node, name, 0);

		if (!gpio_is_valid(agg->gpios[i].gpio)) {
			pr_err("%s: get %s fail\n", __func__, name);
			goto err_out;
		}

		ret = gpio_request(agg->gpios[i].gpio, name);
		if (ret < 0) {
			pr_err("%s: %s request fail\n", __func__, name);
			goto err_out;
		}
	}

	audio_gpio_parse_mode(node, agg);
	agg->group_register = 1;
	pr_info("%s: %s gpios register success\n", __func__, agg->group_name);
	return;

err_out:
	kfree(agg->gpios);
	agg->gpios = NULL;
	return;
}

static int audio_gpio_parse_group_parms(struct device_node *node,
	struct audio_gpio_group *agg)
{
	unsigned int ret;

	if (!agg) {
		pr_err("%s: audio_gpio_group is NULL\n", __func__);
		return -1;
	}

	ret = audio_gpio_group_get_prop_of_str_type(node, "group_name",
		&agg->group_name);
	if (ret < 0) {
		pr_err("%s: get group name fail\n", __func__);
		return ret;
	}

	ret = audio_gpio_group_get_prop_of_u32_type(node, "num",
		&agg->gpio_num);
	if (ret < 0) {
		pr_err("%s: get group gpio num fail\n", __func__);
		return ret;
	}

	return 0;
}

static int audio_gpio_group_init(struct device_node *node,
	struct audio_gpio_ctrl_priv *gp)
{
	int ret = 0;
	unsigned int i;
	struct device_node *group_node = NULL;
	char name[AUDIO_GPIO_GROUP_NAME_MAX] = { 0 };
	struct audio_gpio_group *agg = NULL;

	gp->audio_groups =
		kzalloc(gp->group_num * sizeof(struct audio_gpio_group), GFP_KERNEL);
	if (!gp->audio_groups)
		return -ENOMEM;

	for (i = 0; i < gp->group_num; i++) {
		snprintf(name, AUDIO_GPIO_GROUP_NAME_MAX, "group%d", i);
		group_node = of_get_child_by_name(node, name);
		if (!group_node) {
			pr_err("%s: get group node %s fail\n", __func__, name);
			goto err_out;
		}

		agg = &gp->audio_groups[i];
		agg->group_register = 0;
		agg->group_id = i;
		agg->cur_mode = AUDIO_GPIO_DEFAULT_MODE;

		ret = audio_gpio_parse_group_parms(group_node, agg);
		if (ret < 0) {
			pr_err("%s: parse group parms fail\n", __func__);
			goto err_out;
		}

		audio_gpio_parse_dts(group_node, agg);
	}
	return ret;

err_out:
	kfree(gp->audio_groups);
	gp->audio_groups = NULL;
	return -1;
}

static void audio_gpio_get_group_num(struct device_node *node,
	struct audio_gpio_ctrl_priv *gp)
{
	int ret;

	ret = of_property_read_u32(node, "group_num", &gp->group_num);
	if (ret < 0)
		pr_err("%s: read group_num err\n", __func__);
}

static void audio_gpio_free_group_memory(struct audio_gpio_ctrl_priv *gp)
{
	unsigned int i;

	if (!gp)
		return;

	if (gp->audio_groups) {
		for (i = 0; i < gp->group_num; i++) {
			if (gp->audio_groups[i].gpios) {
				kfree(gp->audio_groups[i].gpios);
				gp->audio_groups[i].gpios = NULL;
			}
		}
		kfree(gp->audio_groups);
		gp->audio_groups = NULL;
	}

	kfree(gp);
}

static int audio_gpio_ctl_probe(struct platform_device *pdev)
{
	int ret = -1;
	struct audio_gpio_ctrl_priv *gpio_priv = NULL;

	pr_info("%s: enter\n", __func__);

	if (!pdev) {
		pr_err("%s: invalid argument\n", __func__);
		return -EINVAL;
	}

	gpio_priv = kzalloc(sizeof(*gpio_priv), GFP_KERNEL);
	if (!gpio_priv)
		return -ENOMEM;

	audio_gpio_get_group_num(pdev->dev.of_node, gpio_priv);
	if (gpio_priv->group_num <= 0) {
		pr_err("%s: not support any audio gpio service\n", __func__);
		goto err_out;
	}

	ret = audio_gpio_group_init(pdev->dev.of_node, gpio_priv);
	if (ret < 0) {
		pr_err("%s init audio gpio group fail\n", __func__);
		goto err_out;
	}

	mutex_init(&gpio_priv->do_ioctl_lock);

	gpio_priv->dev = &pdev->dev;
	g_audio_gpio_priv = gpio_priv;

	ret = misc_register(&audio_gpio_ctl_miscdev);
	if (ret != 0) {
		pr_err("%s: register miscdev failed, %d\n", __func__, ret);
		goto err_out;
	}

	pr_info("%s: end success\n", __func__);
	return 0;

err_out:
	audio_gpio_free_group_memory(gpio_priv);
	g_audio_gpio_priv = NULL;
	return ret;
}

static int audio_gpio_ctl_remove(struct platform_device *pdev)
{
	misc_deregister(&audio_gpio_ctl_miscdev);

	audio_gpio_free_group_memory(g_audio_gpio_priv);
	g_audio_gpio_priv = NULL;

	return 0;
}

static const struct of_device_id audio_gpio_ctl_match[] = {
	{ .compatible = "huawei,audio_gpio_ctl", },
	{},
};
MODULE_DEVICE_TABLE(of, audio_gpio_ctl_match);

static struct platform_driver audio_gpio_ctl_driver = {
	.driver = {
		.name           = "audio_gpio_ctl",
		.owner          = THIS_MODULE,
		.of_match_table = of_match_ptr(audio_gpio_ctl_match),
	},
	.probe    = audio_gpio_ctl_probe,
	.remove   = audio_gpio_ctl_remove,
};

static int __init audio_gpio_ctl_init(void)
{
	int ret;

	ret = platform_driver_register(&audio_gpio_ctl_driver);
	if (ret != 0)
		pr_err("%s: platform_driver_register failed, %d\n",
			__func__, ret);

	return ret;
}

static void __exit audio_gpio_ctl_exit(void)
{
	platform_driver_unregister(&audio_gpio_ctl_driver);
}

late_initcall(audio_gpio_ctl_init);
module_exit(audio_gpio_ctl_exit);

MODULE_DESCRIPTION("audio gpio ctl driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
MODULE_LICENSE("GPL v2");
