// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2017, 2019-2020 The Linux Foundation. All rights reserved.
 */

#include <linux/platform_device.h>
#include <linux/of_device.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/err.h>
#include <linux/mutex.h>
#include <linux/init.h>
#include <linux/types.h>

#define BUFSIZE 2048

struct audio_vqm_reg_dev {
	char *driver_name;
	struct cdev dev;
	struct mutex lmutex;

	dev_t ion_major;
	struct class *ion_class;
	struct device *chardev;
	char buf[BUFSIZE];
};

static int audio_vqm_open(struct inode *inode, struct file *file)
{
	int ret = 0;
	struct audio_vqm_reg_dev *dev;

	pr_info("Inside %s\n", __func__);
	dev = container_of(inode->i_cdev, struct audio_vqm_reg_dev, dev);
	file->private_data = dev;
	return ret;
}

static int audio_vqm_release(struct inode *inode, struct file *file)
{
	pr_info("Inside %s\n", __func__);
	return 0;
}

static ssize_t audio_vqm_read(struct file *file, char __user * buf,
			      size_t count, loff_t * f_ops)
{
	struct audio_vqm_reg_dev *dev =
	    (struct audio_vqm_reg_dev *)file->private_data;

	pr_info("read %s\n", __func__);
	mutex_lock(&(dev->lmutex));
	if (sizeof(dev->buf) < count) {
		mutex_unlock(&(dev->lmutex));
		return 0;
	}
	if (copy_to_user(buf, dev->buf, count)) {
		mutex_unlock(&(dev->lmutex));
		return 0;
	}
	mutex_unlock(&(dev->lmutex));
	return count;
}

static ssize_t audio_vqm_write(struct file *file, const char __user * buf,
			       size_t count, loff_t * f_ops)
{
	struct audio_vqm_reg_dev *dev =
	    (struct audio_vqm_reg_dev *)file->private_data;

	pr_info("write %s\n", __func__);

	mutex_lock(&(dev->lmutex));
	if (sizeof(dev->buf) < count) {
		mutex_unlock(&(dev->lmutex));
		return 0;
	}

	if (copy_from_user(dev->buf, buf, count)) {
		mutex_unlock(&(dev->lmutex));
		return 0;
	}
	mutex_unlock(&(dev->lmutex));
	return count;
}

static const struct file_operations audio_vqm_fops = {
	.owner = THIS_MODULE,
	.open = audio_vqm_open,
	.release = audio_vqm_release,
	.read = audio_vqm_read,
	.write = audio_vqm_write,
};

static int msm_audio_vqm_probe(struct platform_device *pdev)
{
	int err;
	struct audio_vqm_reg_dev *vqmdata = NULL;

	pr_info("%s: audio_vqm_probe driver called \n", __func__);

	vqmdata = devm_kzalloc(&pdev->dev,
			       (sizeof(struct audio_vqm_reg_dev)), GFP_KERNEL);
	if (!vqmdata)
		return -ENOMEM;

	vqmdata->driver_name = "audio_vqm";
	memset(vqmdata->buf, 0x0, sizeof(vqmdata->buf));

	dev_set_drvdata(&pdev->dev, vqmdata);

	mutex_init(&(vqmdata->lmutex));

	err =
	    alloc_chrdev_region(&vqmdata->ion_major, 0, 1,
				vqmdata->driver_name);
	if (err < 0) {
		pr_info("%s: failed to alloc char dev region. \n", __func__);
		return err;
	}

	pr_info("%s: major number %d", __func__, MAJOR(vqmdata->ion_major));

	vqmdata->ion_class = class_create(THIS_MODULE, vqmdata->driver_name);

	if (IS_ERR(vqmdata->ion_class)) {
		err = PTR_ERR(vqmdata->ion_class);
		pr_err("%s class create failed. ret : %d", __func__, err);
		goto err_class;
	}

	vqmdata->chardev = device_create(vqmdata->ion_class, NULL,
					 vqmdata->ion_major, NULL,
					 vqmdata->driver_name);

	if (IS_ERR(vqmdata->chardev)) {
		err = PTR_ERR(vqmdata->chardev);
		pr_err("%s device create failed. ret : %d", __func__, err);
		goto err_device;
	}

	cdev_init(&vqmdata->dev, &audio_vqm_fops);
	err = cdev_add(&vqmdata->dev, vqmdata->ion_major, 1);
	if (err) {
		pr_err("%s device create failed. ret : %d", __func__, err);
		goto err_cdev;
	}

	return err;

err_cdev:
	device_destroy(vqmdata->ion_class, vqmdata->ion_major);
err_device:
	class_destroy(vqmdata->ion_class);
err_class:
	unregister_chrdev_region(vqmdata->ion_major, 1);
	mutex_destroy(&vqmdata->lmutex);
	return err;
}

static int msm_audio_vqm_remove(struct platform_device *pdev)
{
	struct audio_vqm_reg_dev *vqmdata =
	    (struct audio_vqm_reg_dev *)dev_get_drvdata(&pdev->dev);

	pr_info("%s: audio_vqm_remove driver called \n", __func__);
	mutex_destroy(&vqmdata->lmutex);
	cdev_del(&vqmdata->dev);
	device_destroy(vqmdata->ion_class, vqmdata->ion_major);
	class_destroy(vqmdata->ion_class);
	unregister_chrdev_region(vqmdata->ion_major, 1);
	return 0;
}

static struct of_device_id audio_vqm_match[] = {
	{.compatible = "huawei,audio_vqm"},
	{}
};

static struct platform_driver audio_vqm_driver = {
	.driver = {
		   .name = "audio_vqm",
		   .owner = THIS_MODULE,
		   .of_match_table = of_match_ptr(audio_vqm_match)
		   },
	.probe = msm_audio_vqm_probe,
	.remove = msm_audio_vqm_remove
};

static int __init audio_vqm_init(void)
{
	pr_info("%s: audio_vqm_init driver called \n", __func__);
	return platform_driver_register(&audio_vqm_driver);
}

static void __exit audio_vqm_exit(void)
{
	pr_info("%s: audio_vqm_exit driver called \n", __func__);
	platform_driver_unregister(&audio_vqm_driver);
}

module_init(audio_vqm_init);
module_exit(audio_vqm_exit);

MODULE_DESCRIPTION("audio_vqm module");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
MODULE_LICENSE("GPL v2");
