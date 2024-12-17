/*
 * npu_devinit.c
 *
 * about npu devinit
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
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
#include "npu_direct_devinit.h"

#include <linux/fs.h>
#include <linux/device.h>
#include <linux/mutex.h>
#include <linux/list.h>
#include <linux/module.h>

#include "npu_log.h"
#include "npu_ioctl_services.h"
#include "npu_direct_ioctl_services.h"

static unsigned int g_npu_direct_major;
static struct class *g_npu_direct_class;
static struct cdev g_dev_direct_info;

static int npu_direct_open(struct inode *inode, struct file *filep)
{
	return npu_open_common(inode, filep, DIRECT);
}

static const struct file_operations npu_dev_direct_fops = {
	.owner = THIS_MODULE,
	.open = npu_direct_open,
	.release = npu_release,
	.unlocked_ioctl = npu_direct_ioctl,
	.compat_ioctl = npu_direct_ioctl,
};

static char *npu_direct_node(struct device *dev, umode_t *mode)
{
	unused(dev);
	if (mode != NULL)
		*mode = 660;
	return NULL;
}

int npu_dev_direct_init(void)
{
	struct device *i_device = NULL;
	dev_t devno;
	int ret;

	ret = alloc_chrdev_region(&g_npu_direct_major, 0, NPU_MAX_DEVICE_NUM,
		"npu_direct");
	if (ret != 0) {
		npu_drv_err("npu acc alloc_chrdev_region error\n");
		return -1;
	}

	g_dev_direct_info.owner = THIS_MODULE;
	devno = MKDEV(MAJOR(g_npu_direct_major), 0);

	g_npu_direct_class = class_create(THIS_MODULE, "npu-direct-class");
	if (IS_ERR(g_npu_direct_class)) {
		npu_drv_err("npu_direct_class create error\n");
		ret = -EINVAL;
		goto class_fail;
	}

	g_npu_direct_class->devnode = npu_direct_node;

	cdev_init(&g_dev_direct_info, &npu_dev_direct_fops);
	ret = cdev_add(&g_dev_direct_info, devno, NPU_MAX_DEVICE_NUM);
	if (ret != 0) {
		npu_drv_err("npu direct cdev_add error\n");
		goto cdev_setup_fail;
	}

	i_device = device_create(g_npu_direct_class, NULL, devno, NULL,
		"npu_direct");
	if (i_device == NULL) {
		npu_drv_err("npu direct device_create error\n");
		ret = -ENODEV;
		goto device_create_fail;
	}

	npu_drv_warn("npu dev direct init succeed\n");
	return ret;

device_create_fail:
	cdev_del(&g_dev_direct_info);
cdev_setup_fail:
	class_destroy(g_npu_direct_class);
class_fail:
	unregister_chrdev_region(g_npu_direct_major, NPU_MAX_DEVICE_NUM);
	return ret;
}

void npu_dev_direct_exit(void)
{
	dev_t devno = MKDEV(MAJOR(g_npu_direct_major), 0);
	device_destroy(g_npu_direct_class, devno);
	cdev_del(&g_dev_direct_info);
	class_destroy(g_npu_direct_class);
	g_npu_direct_class = NULL;
	unregister_chrdev_region(g_npu_direct_major, NPU_MAX_DEVICE_NUM);
}

static int __init npu_direct_init(void) {
	return npu_dev_direct_init();
}
module_init(npu_direct_init);

static void __exit npu_direct_exit(void) {
	npu_dev_direct_exit();
}
module_exit(npu_direct_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("NPU direct driver");
MODULE_VERSION("V1.0");