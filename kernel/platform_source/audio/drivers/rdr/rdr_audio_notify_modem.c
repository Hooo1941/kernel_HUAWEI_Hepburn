/*
 * rdr_audio_notify_modem.c
 *
 * dsp reset notify modem
 *
 * Copyright (c) 2019-2020 Huawei Technologies CO., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include "rdr_audio_notify_modem.h"

#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/string.h>
#include <linux/cdev.h>
#include <linux/wait.h>
#include <linux/errno.h>
#include <linux/uaccess.h>

#include "audio_log.h"
#include "rdr_print.h"

enum notify_mode_state {
	NOTIFY_NODEM_OFF,
	NOTIFY_NODEM_BEGIN,
	NOTIFY_NODEM_END,
	NOTIFY_MODEM_NUM
};

#define LOG_TAG "rdr_audio_notify_modem"

struct dsp_reset_node_info {
	char *name;
	dev_t devt;
	struct cdev reset_cdev;
	struct class *class;
	unsigned int dsp_reset_state;
	unsigned int flag;
	wait_queue_head_t wait;
};

static const char *const g_dsp_state_str[] = {
	"HIFI_RESET_STATE_OFF\n",
	"HIFI_RESET_STATE_READY\n",
	"HIFI_RESET_STATE_INVALID\n",
};

struct dsp_reset_node_info g_dsp_reset_node[NOTIFY_MODEM_NUM] = {
	[NOTIFY_NODEM_OFF] = {
		.name = "hifi_reset0",
		.dsp_reset_state = DSP_RESET_STATE_READY,
		.flag = 0,
	},
	[NOTIFY_NODEM_BEGIN] = {
		.name = "hifi_reset1",
		.dsp_reset_state = DSP_RESET_STATE_READY,
		.flag = 0,
	},
	[NOTIFY_NODEM_END] = {
		.name = "hifi_reset2",
		.dsp_reset_state = DSP_RESET_STATE_READY,
		.flag = 0,
	},
};

static int dsp_reset_open(struct inode *inode, struct file *filp)
{
	unsigned int dev_num;

	if (!inode)
		return -EIO;

	if (!filp)
		return -ENOENT;

	for (dev_num = 0; dev_num < NOTIFY_MODEM_NUM; dev_num++) {
		if (MINOR(g_dsp_reset_node[dev_num].devt) == iminor(inode)) {
			filp->private_data = &g_dsp_reset_node[dev_num];
			AUDIO_LOGI("dsp reset open succ devt %d, dev num %u",
				g_dsp_reset_node[dev_num].devt, dev_num);
			return 0;
		}
	}

	AUDIO_LOGE("No device can be opened during the DSP reset");

	return -EBUSY;
}

static int dsp_reset_release(struct inode *inode, struct file *filp)
{
	unsigned int dev_num;

	if (!inode)
		return -EIO;

	if (!filp)
		return -ENOENT;

	for (dev_num = 0; dev_num < NOTIFY_MODEM_NUM; dev_num++) {
		if (MINOR(g_dsp_reset_node[dev_num].devt) == iminor(inode)) {
			filp->private_data = NULL;
			AUDIO_LOGI("dsp reset release succ. devt %d, dev num %u",
				g_dsp_reset_node[dev_num].devt, dev_num);
			return 0;
		}
	}

	AUDIO_LOGE("dsp reset no dev to release");

	return -EBUSY;
}

static ssize_t dsp_reset_read(struct file *filp, char __user *buf, size_t count, loff_t *pos)
{
	size_t len;
	struct dsp_reset_node_info *node = NULL;

	if (!filp)
		return -ENOENT;

	if (!buf)
		return -EINVAL;

	if (!pos)
		return -EFAULT;

	node = (struct dsp_reset_node_info *)filp->private_data;
	if (!node) {
		AUDIO_LOGE("node is null");
		return -EIO;
	}

	if (node->dsp_reset_state >= DSP_RESET_STATE_INVALID) {
		AUDIO_LOGE("dsp reset state error %u", node->dsp_reset_state);
		return -EFAULT;
	}

	if (wait_event_interruptible(node->wait, (node->flag == 1))) { //lint !e578
		AUDIO_LOGI("dsp reset event state not change");
		return -ERESTARTSYS;
	}

	node->flag = 0;
	len = strlen(g_dsp_state_str[node->dsp_reset_state]) + 1;
	AUDIO_LOGI("dsp reset pos %lld, count %zu, len %zd", *pos, count, len);
	if (count < len) {
		AUDIO_LOGE("dsp reset usr count need larger, count %zu", count);
		return -EFAULT;
	}
	if (copy_to_user(buf, g_dsp_state_str[node->dsp_reset_state], len)) {
		AUDIO_LOGE("dsp reset copy to user fail");
		return -EFAULT;
	}

	return (ssize_t)len;
}


static const struct file_operations dsp_reset_fops = {
	.owner = THIS_MODULE,
	.open = dsp_reset_open,
	.read = dsp_reset_read,
	.release = dsp_reset_release,
};

static int dsp_reset_dev_init_by_num(dev_t devt, unsigned int dev_num)
{
	int ret;
	struct dsp_reset_node_info *node = &g_dsp_reset_node[dev_num];

	AUDIO_LOGI("init %s", node->name);

	init_waitqueue_head(&node->wait);
	node->devt = MKDEV(MAJOR(devt), dev_num);
	cdev_init(&node->reset_cdev, &dsp_reset_fops);
	ret = cdev_add(&node->reset_cdev, node->devt, 1);
	if (ret != 0) {
		AUDIO_LOGE("cdev add fail %d", ret);
		return -EFAULT;
	}

	node->class = class_create(THIS_MODULE, node->name);
	if (!node->class) {
		AUDIO_LOGE("class creat fail");
		goto class_create_error;
	}

	if (IS_ERR(device_create(node->class, NULL, node->devt, &node, "%s", node->name))) {
		AUDIO_LOGE("device create fail");
		goto device_create_error;
	}

	return 0;

device_create_error:
	class_destroy(node->class);
class_create_error:
	cdev_del(&node->reset_cdev);

	return -EPERM;
}

static void dsp_reset_dev_exit_by_num(unsigned int dev_num)
{
	struct dsp_reset_node_info *node = &g_dsp_reset_node[dev_num];

	device_destroy(node->class, node->devt);
	class_destroy(node->class);
	cdev_del(&node->reset_cdev);
}

void dsp_reset_dev_deinit(void)
{
	unsigned int dev_num;
	dev_t devt = g_dsp_reset_node[0].devt;

	for (dev_num = 0; dev_num < NOTIFY_MODEM_NUM; dev_num++)
		dsp_reset_dev_exit_by_num(dev_num);

	unregister_chrdev_region(MKDEV(MAJOR(devt), 0), NOTIFY_MODEM_NUM);
}

int dsp_reset_dev_init(void)
{
	int ret;
	unsigned int dev_num;
	dev_t devt;

	ret = alloc_chrdev_region(&devt, 0, NOTIFY_MODEM_NUM, "hifi_reset");
	if (ret != 0) {
		AUDIO_LOGE("alloc chrdev region fail");
		return -EFAULT;
	}

	for (dev_num = 0; dev_num < NOTIFY_MODEM_NUM; dev_num++) {
		ret =  dsp_reset_dev_init_by_num(devt, dev_num);
		if (ret != 0) {
			AUDIO_LOGE("init dev num %u fail. ret %d", dev_num, ret);
			goto dev_init_error;
		}
	}

	AUDIO_LOGI("dsp reset dev init succeed");

	return 0;

dev_init_error:
	while (dev_num > 0) {
		dev_num--;
		dsp_reset_dev_exit_by_num(dev_num);
	}

	unregister_chrdev_region(MKDEV(MAJOR(devt), 0), NOTIFY_MODEM_NUM);

	return -EPERM;
}

void dsp_reset_notify_modem(enum dsp_reset_state state)
{
	unsigned int dev_num;

	if (state >= DSP_RESET_STATE_INVALID) {
		AUDIO_LOGE("dsp reset state error state %d", state);
		return;
	}

	for (dev_num = 0; dev_num < NOTIFY_MODEM_NUM; dev_num++) {
		g_dsp_reset_node[dev_num].dsp_reset_state = state;
		g_dsp_reset_node[dev_num].flag = NOTIFY_NODEM_BEGIN;
		wake_up_interruptible(&g_dsp_reset_node[dev_num].wait);
		AUDIO_LOGI("notify modem[%u] done", dev_num);
	}
}

