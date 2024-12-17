/*
 * echub_update.c
 *
 * echub update interface
 *
 * Copyright (c) 2023-2023 Huawei Technologies Co., Ltd.
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

#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/fs.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include "../echub.h"
#include "../echub_i2c.h"

#define ECHUB_I2C_RDWR          _IOWR(0xEC, 0x01, struct i2c_rdwr_ioctl_data)
#define RDWR_IOCTL_MAX_MSGS     32
#define RDWR_IOCTL_MSG_MAX_LEN  256

#ifdef CONFIG_CMDLINE_PARSE
extern unsigned int get_boot_into_recovery_flag(void);
#else
static inline unsigned int get_boot_into_recovery_flag(void) { return 0;}
#endif

static int major;
static struct class *class = NULL;
static struct echub_i2c_dev *g_echub_dev = NULL;

static int echub_update_open(struct inode *inode, struct file *filp)
{
	int ret = 0;
	if ((inode == NULL) || (filp == NULL)) {
		echub_err("%s, invalid arguments\n", __func__);
		return -EINVAL;
	}

	ret = nonseekable_open(inode, filp);
	if (ret < 0) {
		echub_err("%s, nonseekable_open failed, %d\n", __func__, ret);
		return ret;
	}

	filp->private_data = (void *)g_echub_dev;
	return ret;
}

static int echub_update_release(struct inode *inode, struct file *filp)
{
	if ((inode == NULL) || (filp == NULL)) {
		echub_err("%s, invalid arguments\n", __func__);
		return -EINVAL;
	}

	filp->private_data = NULL;
	return 0;
}

static int echub_update_ioctl_rdwr(struct i2c_client *client, struct i2c_msg *msgs, unsigned nmsgs)
{
	u8 __user **user_data_ptrs;
	int i = 0;
	int j = 0;
	int ret = 0;

	user_data_ptrs = kmalloc_array(nmsgs, sizeof(u8 __user *), GFP_KERNEL);
	if (user_data_ptrs == NULL) {
		echub_err("%s, alloc user data failed\n", __func__);
		return -ENOMEM;
	}

	for (i = 0; i < nmsgs; i++) {
		if (msgs[i].len > RDWR_IOCTL_MSG_MAX_LEN) {
			echub_err("%s, msg data too long: %d\n", __func__, msgs[i].len);
			ret = -EINVAL;
			break;
		}
		msgs[i].addr = client->addr;
		user_data_ptrs[i] = (u8 __user *)msgs[i].buf;
		msgs[i].buf = memdup_user(user_data_ptrs[i], msgs[i].len);
		if (IS_ERR(msgs[i].buf)) {
			echub_err("%s, dump user data failed\n", __func__);
			ret = PTR_ERR(msgs[i].buf);
			break;
		}
		msgs[i].flags |= I2C_M_DMA_SAFE;
	}
	if (ret < 0) {
		for (j = 0; j < i; ++j)
			kfree(msgs[j].buf);
		kfree(user_data_ptrs);
		return ret;
	}

	ret = i2c_transfer(client->adapter, msgs, nmsgs);
	while (i-- > 0) {
		if (ret >= 0 && (msgs[i].flags & I2C_M_RD)) {
			if (copy_to_user(user_data_ptrs[i], msgs[i].buf, msgs[i].len)) {
			echub_err("%s, copy data to user failed\n", __func__);
			ret = -EFAULT;
		}
		}
		kfree(msgs[i].buf);
	}
	kfree(user_data_ptrs);
	return ret;
}

static long echub_update_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	struct echub_i2c_dev *echub_dev = (struct echub_i2c_dev *)file->private_data;

	if (!echub_dev) {
		echub_err("%s, invalid value\n", __func__);
		return -EINVAL;
	}

	switch (cmd) {
	case ECHUB_I2C_RDWR: {
		struct i2c_rdwr_ioctl_data rdwr_data;
		struct i2c_msg *msgs;
		if (!get_boot_into_recovery_flag()) {
			echub_err("%s, not in updater mode\n", __func__);
			return -EFAULT;
		}

		if (copy_from_user(&rdwr_data, (struct i2c_rdwr_ioctl_data __user *)arg, sizeof(rdwr_data))) {
			echub_err("%s, copy user data failed\n", __func__);
			return -EFAULT;
		}

		if (!rdwr_data.msgs || rdwr_data.nmsgs == 0 || rdwr_data.nmsgs > RDWR_IOCTL_MAX_MSGS) {
			echub_err("%s, invalid user param\n", __func__);
			return -EINVAL;
		}

		msgs = memdup_user(rdwr_data.msgs, rdwr_data.nmsgs * sizeof(struct i2c_msg));
		if (IS_ERR(msgs)) {
			echub_err("%s, dump user data failed\n", __func__);
			return PTR_ERR(msgs);
		}

		ret = echub_update_ioctl_rdwr(echub_dev->client, msgs, rdwr_data.nmsgs);
		kfree(msgs);
		return ret;
	}
	default:
		echub_err("%s, invalid cmd: 0x%x\n", __func__, cmd);
		return -ENOTTY;
	}
}

static struct file_operations echub_update_fops = {
	.owner = THIS_MODULE,
	.open = echub_update_open,
	.release = echub_update_release,
	.unlocked_ioctl = echub_update_ioctl,
};

static int echub_update_probe(struct platform_device *pdev)
{
	g_echub_dev = dev_get_drvdata(pdev->dev.parent);
	if (g_echub_dev == NULL) {
		echub_err("echub_dev is NULL\n");
		return -ENODEV;
	}

	major = register_chrdev(0, "echub_update", &echub_update_fops);
	class = class_create(THIS_MODULE, "echub_update");
	device_create(class, NULL, MKDEV(major, 0), NULL, "echub_update");
	return 0;
}


static const struct of_device_id of_echub_update_match_tbl[] = {
	{ .compatible = "huawei,echub-update" },
	{},
};

static struct platform_driver echub_update_driver = {
	.driver = {
		.name = "echub-update",
		.owner  = THIS_MODULE,
		.of_match_table = of_match_ptr(of_echub_update_match_tbl),
	},
	.probe = echub_update_probe,
};


static int __init echub_update_init(void)
{
	return platform_driver_register(&echub_update_driver);
}

static void __exit echub_update_exit(void)
{
	platform_driver_unregister(&echub_update_driver);
}

module_init(echub_update_init);
module_exit(echub_update_exit);

MODULE_DESCRIPTION("echub update driver");
MODULE_LICENSE("GPL");
