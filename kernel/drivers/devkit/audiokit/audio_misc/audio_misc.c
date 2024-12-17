/*
 * audio_misc.c
 *
 * audio_misc driver
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

#include "audio_misc.h"
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/ioctl.h>
#include <linux/platform_device.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/poll.h>
#include <linux/delay.h>
#include <linux/compat.h>
#include <linux/of.h>
#include <linux/notifier.h>
#include <linux/fb.h>
#include <securec.h>
#include <huawei_platform/log/hw_log.h>
#include "vibrator_state/vibrator_state.h"

#define HWLOG_TAG audio_misc
#define BUF_SIZE 256

HWLOG_REGIST();

static struct audio_misc_device *g_dev = NULL;

static const struct audio_misc_module_table g_mod_table[] = {
	{ "vibrator_state", vibrator_state_probe }
};

static bool audio_misc_check_io_type(char io_type)
{
	struct audio_misc_module *p = NULL;

	mutex_lock(&g_dev->list_lock);
	list_for_each_entry(p, &g_dev->mod_head, node) {
		if (!p)
			continue;
		if (io_type == p->io_type) {
			mutex_unlock(&g_dev->list_lock);
			return false;
		}
	}
	mutex_unlock(&g_dev->list_lock);
	return true;
}

int audio_misc_module_constructor(const char *name, void *priv, char io_type,
	const struct audio_misc_module_ops *ops)
{
	struct audio_misc_module *mod = NULL;

	if (!name || !ops || !ops->remove) {
		hwlog_err("%s: module info is incomplete\n", __func__);
		return -EINVAL;
	}
	mod = kzalloc(sizeof(*mod), GFP_KERNEL);
	if (!mod)
		return -ENOMEM;
	mod->name = name;
	mod->priv = priv;
	if (ops->ioctl) {
		if (!audio_misc_check_io_type(io_type)) {
			hwlog_err("%s: module %s set duplicate io_type\n", __func__, name);
			kfree(mod);
			return -EINVAL;
		}
		mod->io_type = io_type;
	} else {
		mod->io_type = -1;
	}
	mod->ops = ops;
	mutex_lock(&g_dev->list_lock);
	list_add(&mod->node, &g_dev->mod_head);
	mutex_unlock(&g_dev->list_lock);
	return 0;
}

void audio_misc_module_destructor(const char *name)
{
	struct audio_misc_module *p = NULL;
	struct audio_misc_module *p_next = NULL;

	mutex_lock(&g_dev->list_lock);
	list_for_each_entry_safe(p, p_next, &g_dev->mod_head, node) {
		if (!p)
			continue;
		if (strcmp(name, p->name) == 0) {
			list_del(&p->node);
			kfree(p);
			mutex_unlock(&g_dev->list_lock);
			return;
		}
	}
	mutex_unlock(&g_dev->list_lock);
}

static int audio_misc_open(struct inode *inode, struct file *filp)
{
	filp->private_data = g_dev;
	return 0;
}

static long audio_misc_ioctl(struct file *file, unsigned int command,
	unsigned long arg)
{
	struct audio_misc_module *p = NULL;

	mutex_lock(&g_dev->list_lock);
	list_for_each_entry(p, &g_dev->mod_head, node) {
		if (!p || !p->ops)
			continue;
		if (_IOC_TYPE(command) == p->io_type && p->ops->ioctl) {
			mutex_unlock(&g_dev->list_lock);
			return p->ops->ioctl(command, arg);
		}
	}
	mutex_unlock(&g_dev->list_lock);
	return -EINVAL;
}

static const struct file_operations audio_misc_fops = {
	.owner          = THIS_MODULE,
	.open           = audio_misc_open,
	.unlocked_ioctl = audio_misc_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl   = audio_misc_ioctl,
#endif
};

static struct miscdevice audio_misc_dev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name  = "audio_misc",
	.fops  = &audio_misc_fops,
};

void audio_misc_report_uevent(const char *mod_name, const char *action)
{
	struct device_node *np = NULL;
	char *envp[2] = { NULL };
	char buf[BUF_SIZE];
	int ret;

	if (!mod_name || !action)
		return;
	if (!g_dev || !g_dev->dev)
		return;
	np = g_dev->dev->of_node;
	ret = snprintf_s(buf, sizeof(buf), sizeof(buf) - 1, "%s %s", mod_name, action);
	if (ret < 0)
		hwlog_err("%s: fill uevent buf failed\n", __func__);
	envp[0] = buf;
	ret = kobject_uevent_env(&np->kobj, KOBJ_CHANGE, envp);
	if (ret < 0)
		hwlog_err("%s: report uevent failed\n", __func__);
	else
		hwlog_info("%s: report uevent %s %s\n", __func__, mod_name, action);
}

static void audio_misc_module_probe(struct device_node *np)
{
	int ret = 0;
	size_t i;

	for (i = 0; i < sizeof(g_mod_table) / sizeof(struct audio_misc_module_table); ++i) {
		struct device_node *np_child = of_get_child_by_name(np, g_mod_table[i].name);
		if (!np_child)
			continue;
		ret = g_mod_table[i].probe(g_dev->dev, np);
		if (ret)
			hwlog_err("%s: module %s probe failed\n", __func__, g_mod_table[i].name);
		else
			hwlog_info("%s: module %s probe finished\n", __func__, g_mod_table[i].name);
	}
}

static int audio_misc_probe(struct platform_device *pdev)
{
	int ret;

	hwlog_info("%s: in\n", __func__);
	g_dev = kzalloc(sizeof(*g_dev), GFP_KERNEL);
	if (!g_dev)
		return -ENOMEM;
	INIT_LIST_HEAD(&(g_dev->mod_head));
	mutex_init(&(g_dev->list_lock));
	g_dev->dev = &pdev->dev;

	ret = misc_register(&audio_misc_dev);
	if (ret) {
		hwlog_err("%s: regist audio_misc_device failed\n", __func__);
		kfree(g_dev);
		g_dev = NULL;
		return ret;
	}
	audio_misc_module_probe(g_dev->dev->of_node);
	return ret;
}

static void audio_misc_module_remove(void)
{
	struct audio_misc_module *p = NULL;
	struct audio_misc_module *p_next = NULL;
	const char *mod_name;

	mutex_lock(&g_dev->list_lock);
	list_for_each_entry_safe(p, p_next, &g_dev->mod_head, node) {
		if (!p || !p->ops || !p->ops->remove)
			continue;
		mod_name = p->name;
		p->ops->remove(p->priv);
		list_del(&p->node);
		kfree(p);
		hwlog_info("%s: module %s remove finished\n", __func__, mod_name);
	}
	mutex_unlock(&g_dev->list_lock);
}

static int audio_misc_remove(struct platform_device *pdev)
{
	audio_misc_module_remove();
	misc_deregister(&audio_misc_dev);
	kfree(g_dev);
	g_dev = NULL;
	return 0;
}

static const struct of_device_id audio_misc_match[] = {
	{ .compatible = "huawei,audio_misc", },
	{},
};
MODULE_DEVICE_TABLE(of, audio_misc_match);

static struct platform_driver audio_misc_driver = {
	.driver = {
		.name           = "audio_misc",
		.owner          = THIS_MODULE,
		.of_match_table = of_match_ptr(audio_misc_match),
	},
	.probe  = audio_misc_probe,
	.remove = audio_misc_remove,
};

static int __init audio_misc_init(void)
{
	return platform_driver_register(&audio_misc_driver);
}

static void __exit audio_misc_exit(void)
{
	platform_driver_unregister(&audio_misc_driver);
}

late_initcall(audio_misc_init);
module_exit(audio_misc_exit);

MODULE_DESCRIPTION("audio misc driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
MODULE_LICENSE("GPL v2");