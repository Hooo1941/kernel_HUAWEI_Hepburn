/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description config devive source file.
 * Create 2023-09-20

 * This program is free software; you can redistribute it andor modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/errno.h>
#include <securec.h>
#include <linux/poll.h>
#include <linux/slab.h>
#include <linux/notifier.h>
#include <linux/minmax.h>

#include "../../cam_log.h"
#include "linux/of.h"
#include "linux/miscdevice.h"
#include "cam_misc.h"
#include "vibrator_event.h"

#define VID_SHAKE_MIN_DURATION 110

struct cam_vib_notify_dev* g_cam_vib_notify_dev;

static int cam_vib_notify_open(struct inode *inode, struct file *fp)
{
	cam_info("Enter %s", __func__);
	if (!g_cam_vib_notify_dev)
		return -EINVAL;
	fp->private_data = g_cam_vib_notify_dev;
	return 0;
}

static ssize_t cam_vib_notify_read(struct file *fp, char __user *buf, size_t count, loff_t *pos)
{
	int ret = -1;
	cam_info("Enter %s", __func__);
	if (!g_cam_vib_notify_dev || !buf)
		return -EINVAL;

	if (!atomic_read(&g_cam_vib_notify_dev->cam_vib_data.vib_status_update))
		if (wait_event_interruptible(g_cam_vib_notify_dev->wait,
			atomic_read(&g_cam_vib_notify_dev->cam_vib_data.vib_status_update)) != 0)
			return 0;
	cam_info("Enter %s, get event %d", __func__, atomic_read(&g_cam_vib_notify_dev->cam_vib_data.vib_status_update));
	mutex_lock(&g_cam_vib_notify_dev->mux_lock);

	atomic_set(&g_cam_vib_notify_dev->cam_vib_data.vib_status_update, 0);

	if (atomic_read(&g_cam_vib_notify_dev->cam_vib_data.vib_status_update) == 0) {
		  // wake up pm core for 2900ms + 1000ms for schedule
		__pm_wakeup_event(g_cam_vib_notify_dev->cam_vib_protect_ws, 2900 + 1000);
		cam_info("Enter %s, wake up", __func__);
	}
	cam_info("%s, vib_status:%d %d count:%d", __func__, g_cam_vib_notify_dev->cam_vib_data.vib_status,
		sizeof(g_cam_vib_notify_dev->cam_vib_data.vib_status), count);
	ret = copy_to_user((void __user *)buf, (void *)&g_cam_vib_notify_dev->cam_vib_data.vib_status,
		sizeof(g_cam_vib_notify_dev->cam_vib_data.vib_status));
	if (ret) {
		cam_err("%s:copy data failed ret:%d", __func__, ret);
		mutex_unlock(&g_cam_vib_notify_dev->mux_lock);
		return -EFAULT;
	}

	cam_info("Enter %s, %d, %d", __func__, g_cam_vib_notify_dev->cam_vib_data.vib_status,
		atomic_read(&g_cam_vib_notify_dev->cam_vib_data.vib_status_update));

	mutex_unlock(&g_cam_vib_notify_dev->mux_lock);
	return sizeof(g_cam_vib_notify_dev->cam_vib_data.vib_status);
}

static ssize_t cam_vib_notify_write(struct file *fp, const char __user *buf, size_t count, loff_t *pos)
{
	struct cam_vib_notify_data cam_vib_data;
	char data[50] = {0};
	cam_info("Enter %s", __func__);
	if (copy_from_user(data, buf, min_t(size_t, sizeof(data) - 1, count)))
		return -EFAULT;
	cam_info("%s write data %s", __func__, data);
	if (sscanf_s(data, "%d %lu", &cam_vib_data.vib_status, &cam_vib_data.vib_shake_time) == 0) {
		cam_info("%s write data width height error", __func__);
		return -1;
	}

	hw_actuator_protect_work(&cam_vib_data);

	return count;
}

static __poll_t cam_vib_notify_poll(struct file *fp, struct poll_table_struct *wait)
{
	unsigned int mask = 0;
	cam_info("Enter %s", __func__);
	if (!g_cam_vib_notify_dev)
		return -EINVAL;
	poll_wait(fp, &g_cam_vib_notify_dev->wait, wait);

	if (atomic_read(&g_cam_vib_notify_dev->cam_vib_data.vib_status_update) == 1)
		mask |= POLLIN | POLLRDNORM;

	cam_info("%s poll call out, mask = 0x%xn", __func__, mask);
	return mask;
}

static const struct file_operations cam_vib_notify_fops = {
	.owner = THIS_MODULE,
	.read = cam_vib_notify_read,
	.write = cam_vib_notify_write,
	.open = cam_vib_notify_open,
	.poll = cam_vib_notify_poll,
};

static void hw_actuator_protect_work(struct cam_vib_notify_data *cam_vib_data)
{
	cam_info("Enter %s", __func__);
	if (!g_cam_vib_notify_dev)
		return;
	mutex_lock(&g_cam_vib_notify_dev->mux_lock);
	if (cam_vib_data != NULL && g_cam_vib_notify_dev->cam_vib_data.vib_status != cam_vib_data->vib_status) {
		memcpy_s(&g_cam_vib_notify_dev->cam_vib_data,
			sizeof(g_cam_vib_notify_dev->cam_vib_data), cam_vib_data, sizeof(struct cam_vib_notify_data));
		atomic_set(&g_cam_vib_notify_dev->cam_vib_data.vib_status_update, 1);
		wake_up_interruptible(&g_cam_vib_notify_dev->wait);
		cam_err("Enter %s, updated %d", __func__, atomic_read(&g_cam_vib_notify_dev->cam_vib_data.vib_status_update));
	}
	cam_err("Enter %s %d, %d, cam_vib_data %d, %lu, g_protect_data_updated %d", __func__,
		g_cam_vib_notify_dev->cam_vib_data.vib_status, g_cam_vib_notify_dev->cam_vib_data.vib_shake_time,
		cam_vib_data->vib_status, cam_vib_data->vib_shake_time,
		atomic_read(&g_cam_vib_notify_dev->cam_vib_data.vib_status_update));
	mutex_unlock(&g_cam_vib_notify_dev->mux_lock);
}

static int cam_vib_notifier_cb(struct notifier_block *nb,
	unsigned long time, void *bar)
{
	struct cam_vib_notify_data cam_vib_notify_data;
	cam_info("Enter %s", __func__);
	if (time >= VID_SHAKE_MIN_DURATION) {
		cam_vib_notify_data.vib_status = 1;
		cam_vib_notify_data.vib_shake_time = time;
	} else {
		cam_vib_notify_data.vib_status = 0;
	}
	hw_actuator_protect_work(&cam_vib_notify_data);
	return 0;
}

static struct notifier_block vib_notify_to_cam = {
	.notifier_call = cam_vib_notifier_cb,
	.priority = -1,
};

static int cam_misc_probe(struct platform_device *pdev)
{
	int rc;
	struct cam_vib_notify_dev* cam_vib_notify_dev = NULL;
	cam_info("Enter %s", __func__);
	cam_vib_notify_dev = kzalloc(sizeof(struct cam_vib_notify_dev), GFP_KERNEL);
	if (!cam_vib_notify_dev)
		return -ENOMEM;
	cam_vib_notify_dev->cam_vib_notify_misc.minor = MISC_DYNAMIC_MINOR;
	cam_vib_notify_dev->cam_vib_notify_misc.name = "cam_vib_notify";
	cam_vib_notify_dev->cam_vib_notify_misc.fops = &cam_vib_notify_fops;

	rc = misc_register(&cam_vib_notify_dev->cam_vib_notify_misc);
	if (rc) {
		cam_err("%s misc register fail", __func__);
		goto misc_reg_fail;
	}

	atomic_set(&cam_vib_notify_dev->cam_vib_data.vib_status_update, 0);
	init_waitqueue_head(&cam_vib_notify_dev->wait);
	cam_vib_notify_dev->dev = &pdev->dev;
	mutex_init(&cam_vib_notify_dev->mux_lock);
	cam_vib_notify_dev->cam_vib_protect_ws =
		wakeup_source_register(cam_vib_notify_dev->dev, "vib_protect_wakelock");
	if (!cam_vib_notify_dev->cam_vib_protect_ws) {
		cam_err("%s: failed to init wakelock", __func__);
		goto wakelock_fail;
	}

	vibrator_register_notifier(&vib_notify_to_cam);
	g_cam_vib_notify_dev = cam_vib_notify_dev;

	return rc;
wakelock_fail:
	misc_deregister(&g_cam_vib_notify_dev->cam_vib_notify_misc);

misc_reg_fail:
	if (cam_vib_notify_dev) {
		kfree(cam_vib_notify_dev);
		cam_vib_notify_dev = NULL;
		g_cam_vib_notify_dev = NULL;
	}

	return rc;
}

static int cam_misc_remove(struct platform_device *pdev)
{
	cam_info("Enter %s", __func__);
	misc_deregister(&g_cam_vib_notify_dev->cam_vib_notify_misc);
	wakeup_source_unregister(g_cam_vib_notify_dev->cam_vib_protect_ws);
	vibrator_unregister_notifier(&vib_notify_to_cam);
	if (g_cam_vib_notify_dev) {
		kfree(g_cam_vib_notify_dev);
		g_cam_vib_notify_dev = NULL;
	}
	return 0;
}

static const struct of_device_id cam_misc_match[] = {
	{ .compatible = "huawei,cam_misc" },
	{},
}
MODULE_DEVICE_TABLE(of, cam_misc_match);

static struct platform_driver cam_misc_driver = {
	.probe = cam_misc_probe,
	.remove = cam_misc_remove,
	.driver = {
		.name = "huawei,cam_misc",
		.owner = THIS_MODULE,
		.of_match_table = cam_misc_match,
	},
};


static int __init cam_misc_init(void)
{
	int ret;
	cam_info("Enter %s", __func__);
	ret = platform_driver_register(&cam_misc_driver);
	if (ret)
		cam_info("%s platform driver register fail", __func__);
	return ret;
}

static void __exit cam_misc_exit(void)
{
	cam_info("Enter %s", __func__);
	platform_driver_unregister(&cam_misc_driver);
}

module_init(cam_misc_init);
module_exit(cam_misc_exit);
MODULE_DESCRIPTION("cam notify");
MODULE_LICENSE("GPL v2");
