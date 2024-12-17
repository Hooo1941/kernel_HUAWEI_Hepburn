/*
 * ext_vibrator.c
 * ext vibrator driver
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
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

#include "ext_vibrator.h"

#include <linux/cdev.h>
#include <linux/leds.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/string.h>
#include <linux/suspend.h>
#include <linux/syscalls.h>
#include <linux/workqueue.h>

#include <securec.h>
#include <huawei_platform/log/hw_log.h>
#include "ext_sensorhub_api.h"

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif

#define HWLOG_TAG ext_vibrator
HWLOG_REGIST();

struct huawei_pmic_vibrator_dev {
	struct device *dev;
	struct class *class;
	struct led_classdev led_dev;
	struct cdev cdev;
	struct mutex lock;
	dev_t version;
	struct work_struct vibrator_off_work;
};

static struct huawei_pmic_vibrator_dev *g_vdev;
static struct class *ext_sensorhub_dbg_class;

static int vibrator_data_comm(unsigned int val, u8 vibrator_mode)
{
	int w_pos = 0;
	struct command cmd;

	unsigned char buf[EXT_VIBRATOR_COMM_BUF_LEN] = {0};

	buf[w_pos++] = vibrator_mode;
	buf[w_pos++] = (u8)(val&0xFF); /* byte0 */
	buf[w_pos++] = (u8)((val>>8)&0xFF); /* shift 8bit for byte1 */
	buf[w_pos++] = (u8)((val>>16)&0xFF); /* shift 16bit for byte2 */
	buf[w_pos++] = (u8)((val>>24)&0xFF); /* shift 24bit for byte3 */

	cmd.service_id = EXT_VIBRATOR_SID;
	cmd.command_id = EXT_VIBRATOR_WORK_CID;
	cmd.send_buffer = buf;
	cmd.send_buffer_len = w_pos;

	return send_command(VIBRATOR_CHANNEL, &cmd, false, NULL);
}

static int ext_vibrator_on_off(unsigned int val)
{
	int ret;

	hwlog_info("[%s] vibrator timeon is %d\n", __func__, val);

	ret = vibrator_data_comm(val, TYPE_EXT_VIBR_WORK_TIME);
	if (ret < 0)
		hwlog_err("send vibrator worktime cmd failed ret %d\n", ret);

	return ret;
}

static int ext_vibra_set_haptic(unsigned int haptic_index)
{
	int ret;

	hwlog_info("[%s] vibrator haptic_index is %d\n", __func__, haptic_index);

	if (haptic_index == HAPTIC_INVALID)
		return 0;

	ret = vibrator_data_comm(haptic_index, TYPE_EXT_VIBR_WORK_SCENARIO);
	if (ret < 0)
		hwlog_err("send vibrator haptic_index comm failed ret %d\n", ret);

	return ret;
}

static s32 ext_vibrator_set_type(struct huawei_pmic_vibrator_dev *vdev, u32 type)
{
	if (vdev == NULL)
		return -EINVAL;

	if (!type) {
		hwlog_err("type:%d is invaild\n", type);
		return -EINVAL;
	}
	ext_vibra_set_haptic(type);

	return 0;
}

static int huawei_pmic_vibrator_power_on_off(unsigned int pmic_vibrator_power_onoff)
{
	int ret;

	mutex_lock(&g_vdev->lock);
	if (pmic_vibrator_power_onoff) {
		ret = ext_vibrator_on_off(pmic_vibrator_power_onoff);
		if (ret < 0)
			hwlog_err("ext_vibrator on failed\n");
	} else {
		ret = ext_vibrator_on_off(EXT_VIBRATOR_OFF);
		if (ret < 0)
			hwlog_err("ext_vibrator off failed\n");
	}
	mutex_unlock(&g_vdev->lock);

	return ret;
}

/* write type to /dev/haptics */
static ssize_t huawei_pmic_vibrator_haptic_write(
	struct file *filp, const char *buff, size_t len, loff_t *off)
{
	struct huawei_pmic_vibrator_dev *vdev = NULL;
	uint64_t type;
	s32 ret;
	char write_buf[PMIC_VIBRATOR_HAP_BUF_LENGTH] = { 0 };
	if (filp == NULL)
		return 0;

	vdev = (struct huawei_pmic_vibrator_dev *)filp->private_data;

	mutex_lock(&vdev->lock);
	if (len > PMIC_VIBRATOR_HAP_BUF_LENGTH || buff == NULL) {
		hwlog_err("huawei pmic vibrator haptic buf is bad\n");
		goto out;
	}

	if (copy_from_user(write_buf, buff, len)) {
		hwlog_err("huawei pmic vibrator haptic copy from user failed\n");
		goto out;
	}

	if (kstrtoull(write_buf, 10, &type)) { /* 10 is base */
		hwlog_err("huawei pmic vibrator haptic read value error\n");
		goto out;
	}

	ret = ext_vibrator_set_type(vdev, (u32)type);
	if (ret < 0) {
		hwlog_err("huawei pmic vibrator haptic cfg failed\n");
		goto out;
	}
out:
	mutex_unlock(&vdev->lock);

	return len;
}

void huawei_pmic_vibrator_haptics_set_type(int type)
{
	s32 ret;

	mutex_lock(&g_vdev->lock);
	ret = ext_vibrator_set_type(g_vdev, (u32)type);
	if (ret) {
		hwlog_err("%s huawei_haptic_test error\n", __func__);
		mutex_unlock(&g_vdev->lock);
		return;
	}
	hwlog_info("%s is running\n", __func__);
	mutex_unlock(&g_vdev->lock);
}
EXPORT_SYMBOL(huawei_pmic_vibrator_haptics_set_type);

static s32 huawei_pmic_vibrator_haptic_open(
	struct inode *i_node, struct file *filp)
{
	if (filp == NULL)
		return -1;
	filp->private_data = g_vdev;
	hwlog_err("%s:haptic open\n", __func__);
	return 0;
}

static const struct file_operations huawei_pmic_vibrator_fops = {
	.open = huawei_pmic_vibrator_haptic_open,
	.write = huawei_pmic_vibrator_haptic_write,
};

static s32 huawei_pmic_vibrator_haptic_probe(struct huawei_pmic_vibrator_dev *vdev)
{
	s32 ret;

	vdev->version = MKDEV(0, 0);
	ret = alloc_chrdev_region(&vdev->version, 0, 1, HUAWEI_PMIC_VIBRATOR_CDEVIE_NAME);
	if (ret < 0) {
		hwlog_err("[%s]:failed to alloc chrdev region, ret[%d]\n", __func__, ret);
		return ret;
	}

	/* /dev/haptics */
	vdev->class = class_create(THIS_MODULE, HUAWEI_PMIC_VIBRATOR_CDEVIE_NAME);
	if (!vdev->class) {
		hwlog_err("[%s]:failed to create class\n", __func__);
		ret = ENOMEM;
		goto unregister_cdev_region;
	}

	vdev->dev = device_create(vdev->class, NULL, vdev->version, NULL, HUAWEI_PMIC_VIBRATOR_CDEVIE_NAME);
	if (vdev->dev == NULL) {
		ret = ENOMEM;
		hwlog_err("[%s]:failed to create device\n", __func__);
		goto destory_class;
	}

	cdev_init(&vdev->cdev, &huawei_pmic_vibrator_fops);
	vdev->cdev.owner = THIS_MODULE;
	vdev->cdev.ops = &huawei_pmic_vibrator_fops;
	ret = cdev_add(&vdev->cdev, vdev->version, 1);
	if (ret) {
		hwlog_err("[%s]:failed to add cdev\n", __func__);
		goto destory_device;
	}

	return 0;
destory_device:
	device_destroy(vdev->class, vdev->version);
destory_class:
	class_destroy(vdev->class);
unregister_cdev_region:
	unregister_chrdev_region(vdev->version, 1);
	return ret;
}

static void huawei_pmic_vibrator_haptic_remove(struct huawei_pmic_vibrator_dev *vdev)
{
	cdev_del(&vdev->cdev);
	device_destroy(vdev->class, vdev->version);
	class_destroy(vdev->class);
	unregister_chrdev_region(vdev->version, 1);
}

static ssize_t enable_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return 0;
}

static ssize_t enable_store(struct device *dev, struct device_attribute *attr,
	const char *buf, size_t size)
{
	unsigned int val = 0;
	int ret;
	ret = sscanf_s(buf, "%u", &val);
	if (!ret) {
		hwlog_err("%s sscanf return invaild:%d\n", __func__, ret);
		return -EINVAL;
	}

	hwlog_err("[%s] time = %d\n", __func__, val);
	if (val > 0) {
		huawei_pmic_vibrator_power_on_off(val);
		hwlog_info("huawei_pmic_vibrator_RTP is running\n");
	} else {
		huawei_pmic_vibrator_power_on_off(val);
		hwlog_info("huawei_pmic_vibrator_RTP has closed\n");
	}

	return size;
}

static DEVICE_ATTR(enable, S_IRUGO | S_IWUSR, enable_show, enable_store);

static void huawei_pmic_vibrator_off_work(struct work_struct *work)
{
	hwlog_info("ext vibrator delay off work\n");
	huawei_pmic_vibrator_power_on_off(PMIC_VIBRATOR_POWER_OFF);
}

static int huawei_pmic_vibrator_probe(struct platform_device *pdev)
{
	struct huawei_pmic_vibrator_dev *vdev = NULL;
	struct device *dev = NULL;
	s32 ret;
	vdev = devm_kzalloc(
		&pdev->dev, sizeof(struct huawei_pmic_vibrator_dev), GFP_KERNEL);
	if (vdev == NULL)
		return -ENOMEM;

	vdev->dev = &pdev->dev;
	g_vdev = vdev;
	dev_set_drvdata(&pdev->dev, vdev);

	mutex_init(&vdev->lock);
	INIT_WORK(&vdev->vibrator_off_work, huawei_pmic_vibrator_off_work);

	ext_sensorhub_dbg_class = class_create(THIS_MODULE, "timed_output");
	if (IS_ERR(ext_sensorhub_dbg_class))
		return PTR_ERR(ext_sensorhub_dbg_class);
	dev = device_create(ext_sensorhub_dbg_class, NULL, MKDEV(0, 0), NULL, "vibrator");
	if (device_create_file(dev, &dev_attr_enable))
		hwlog_err("create file failed in %s", __func__);

	ret = huawei_pmic_vibrator_haptic_probe(vdev);
	if (ret) {
		hwlog_err("[%s]:failed to register haptic dev\n", __func__);
		goto fail_register_led_classdev;
	}
	hwlog_info("ext_vibrator probe succeed\n");

	return 0;

fail_register_led_classdev:
	cancel_work_sync(&vdev->vibrator_off_work);
	mutex_destroy(&vdev->lock);

	return ret;
}

static s32 huawei_pmic_vibrator_remove(struct platform_device *pdev)
{
	struct huawei_pmic_vibrator_dev *vdev = NULL;

	vdev = dev_get_drvdata(&pdev->dev);
	if (vdev == NULL) {
		hwlog_err("%s:failed to get drvdata\n", __func__);
		return -ENODEV;
	}
	cancel_work_sync(&vdev->vibrator_off_work);

	huawei_pmic_vibrator_haptic_remove(vdev);
	mutex_destroy(&vdev->lock);
	dev_set_drvdata(&pdev->dev, NULL);

	return 0;
}

static const struct of_device_id huawei_pmic_vibrator_match[] = {
	{
		.compatible = "huawei,ext-vibrator",
		.data = NULL,
	},
	{},
};
MODULE_DEVICE_TABLE(of, huawei_pmic_vibrator_match);

static struct platform_driver huawei_pmic_vibrator_driver = {
	.probe = huawei_pmic_vibrator_probe,
	.remove = huawei_pmic_vibrator_remove,
	.driver = {
			.name = "ext-vibrator",
			.owner = THIS_MODULE,
			.of_match_table = of_match_ptr(huawei_pmic_vibrator_match),
		},
};

static int __init huawei_pmic_vibrator_init(void)
{
	return platform_driver_register(&huawei_pmic_vibrator_driver);
}

static void __exit huawei_pmic_vibrator_exit(void)
{
	platform_driver_unregister(&huawei_pmic_vibrator_driver);
}

module_init(huawei_pmic_vibrator_init);
module_exit(huawei_pmic_vibrator_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("HUAWEI ext_vibrator driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
