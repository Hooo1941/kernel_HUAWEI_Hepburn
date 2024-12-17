/* Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 and
* only version 2 as published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
*/

#include <linux/types.h>
#include <linux/slab.h>
#include <securec.h>
#include "upgrade_channel_manage.h"
#include "drv_panel_mcu.h"
#include "../common/log.h"

#define LCDPANEL_OTA_MAX_LEN (8 * 1024 * 1024)
#define PANEL_SHOULD_UPDATE 0
#define DECIMAL 10
#define HEX 16
#define PACKAGE_LENGTH 0x2025

static int32_t major_number;
static struct class* g_lcd_sw_ioctl_class = NULL;
static struct device* g_lcd_sw_ioctl_device = NULL;
static int32_t upgrade_start = 0;
static int32_t upgrade_succ = 0;
static uint32_t g_ota_updating = 0;

uint32_t get_ota_update_status(void)
{
	return g_ota_updating;
}

static long lcd_sw_manage_ioctl(struct file *file,
	uint32_t cmd, unsigned long param)
{
	cdc_info("enter sw_manage_ioctl\n");
	return 0;
}

static ssize_t test_mcu_get_version(struct device *dev, struct device_attribute *attr, char *buf)
{
	if (!buf) {
		cdc_err("buff is null\n");
		return -EINVAL;
	}
	cdc_info("enter!!");

	int ret = 0;
	int mcu_status = 0;
	char boot_version[10] = {0};
	char app_version[10] = {0};
	long long bootver = 0;
	long long appver = 0;
	ret = drv_panel_mcu_get_ota_version(boot_version, app_version, &mcu_status);
	if (ret < 0)
		cdc_err("TEST: drv_psanel_mcu get boot and app version failed\n");

	kstrtoll(boot_version, HEX, &bootver);
	kstrtoll(app_version, HEX, &appver);

	cdc_info("TEST: panel_mcu_boot_version %d", bootver);
	cdc_info("TEST: panel_mcu_app_version %d", appver);

	ret = snprintf_s(buf, PAGE_SIZE - strlen(buf), PAGE_SIZE - strlen(buf),
		"TEST: mcu boot version is %lx, app version is %lx\n", bootver, appver);
	if (ret == 0)
		cdc_err("snprintf_s error");
	return strlen(buf);
}

static ssize_t test_mcu_reset(struct device *dev, struct device_attribute *attr, char *buf)
{
	if (!buf) {
		cdc_err("buff is null\n");
		return -EINVAL;
	}
	cdc_info("enter!!");

	int ret = 0;
	uint32_t mcu_status = 0;
	ret = drv_panel_mcu_extended_session(&mcu_status);
	if (ret < 0)
		cdc_err("TEST: mcu_start_update panel mcu reset failed");

	cdc_info("TEST: panel mcu reset status %d", mcu_status);

	ret = snprintf_s(buf, PAGE_SIZE - strlen(buf), PAGE_SIZE - strlen(buf),
		"TEST: panel mcu reset status %d\n", mcu_status);
	if (ret == 0)
		cdc_err("snprintf_s error");
	return strlen(buf);
}

static ssize_t test_mcu_get_seed(struct device *dev, struct device_attribute *attr, char *buf)
{
	if (!buf) {
		cdc_err("buff is null\n");
		return -EINVAL;
	}
	cdc_info("enter!!");

	int ret = 0;
	uint32_t mcu_seed = 0;
	uint32_t mcu_status = 0;
	ret = drv_panel_mcu_get_seed(&mcu_seed, &mcu_status);
	if (ret < 0)
		cdc_err("TEST: The Panel MCU get seed error. nrc %d", mcu_seed);

	cdc_info("TEST: The Panel MCU get seed %d", mcu_seed);

	ret = snprintf_s(buf, PAGE_SIZE - strlen(buf), PAGE_SIZE - strlen(buf),
		"TEST: panel mcu get seed %d\n", mcu_seed);
	if (ret == 0)
		cdc_err("snprintf_s error");
	return strlen(buf);
}

static ssize_t test_mcu_send_key(struct device *dev, struct device_attribute *attr, char *buf)
{
	if (!buf) {
		cdc_err("buff is null\n");
		return -EINVAL;
	}
	cdc_info("enter!!");

	int ret = 0;
	uint32_t mcu_key = 0x01020304;
	uint32_t mcu_status = 0;
	ret = drv_panel_mcu_send_key(mcu_key, &mcu_status);
	if (ret < 0)
		cdc_err("TEST: The Panel MCU send key error");

	cdc_info("TEST: The Panel MCU send key %d", mcu_key);

	ret = snprintf_s(buf, PAGE_SIZE - strlen(buf), PAGE_SIZE - strlen(buf),
		"TEST: panel mcu send key %d\n", mcu_key);
	if (ret == 0)
		cdc_err("snprintf_s error");
	return strlen(buf);
}

static ssize_t test_mcu_send_addlen(struct device *dev, struct device_attribute *attr, char *buf)
{
	if (!buf) {
		cdc_err("buff is null\n");
		return -EINVAL;
	}
	cdc_info("enter!!");

	int ret = 0;
	uint32_t address = 0x01000100;
	uint32_t len = 0x2025;
	uint32_t mcu_status = 0;
	ret = drv_panel_mcu_erase_flash(address, len, &mcu_status);
	if (ret < 0)
		cdc_err("TEST: drv_panel_mcu_erase_flash failed, ret = %d", ret);

	cdc_info("TEST: address 0x%x, len 0x%x", address, len);

	ret = snprintf_s(buf, PAGE_SIZE - strlen(buf), PAGE_SIZE - strlen(buf),
		"TEST: address 0x%x, len 0x%x\n", address, len);
	if (ret == 0)
		cdc_err("snprintf_s error");
	return strlen(buf);
}

static ssize_t test_mcu_request_download(struct device *dev, struct device_attribute *attr, char *buf)
{
	int ret = 0;
	uint32_t address = 0x01000100;
	uint32_t len = 0x2025;
	uint32_t mcu_status = 0;
	if (!buf) {
		cdc_err("buff is null\n");
		return -EINVAL;
	}
	cdc_info("enter!!");

	ret = drv_panel_mcu_request_download(address, len, &mcu_status);
	if (ret != 0)
		cdc_err("drv_panel_mcu_request_download failed, ret = %d", ret);

	cdc_info("TEST: address 0x%x, len 0x%x", address, len);

	ret = snprintf_s(buf, PAGE_SIZE - strlen(buf), PAGE_SIZE - strlen(buf),
		"TEST: address 0x%x, len 0x%x\n", address, len);
	if (ret == 0)
		cdc_err("snprintf_s error");
	return strlen(buf);
}

static ssize_t test_mcu_transfer_data(struct device *dev, struct device_attribute *attr, char *buf)
{
	if (!buf) {
		cdc_err("buff is null\n");
		return -EINVAL;
	}
	cdc_info("enter!");

	int ret = 0;
	int i = 0;
	int len = 64;
	uint32_t mcu_status = 0;
	uint8_t version_bin[64];
	for (i = 0; i < len; i++)
		version_bin[i] = i;

	ret = drv_panel_mcu_ota_update(version_bin, len, &mcu_status);
	if (ret != 0)
		cdc_err("drv_panel_mcu_ota_update failed, ret = %d", ret);

	cdc_info("TEST: version_bin len %d", len);

	ret = snprintf_s(buf, PAGE_SIZE - strlen(buf), PAGE_SIZE - strlen(buf),
		"TEST: version_bin len %d\n", len);
	if (ret == 0)
		cdc_err("snprintf_s error");
	return strlen(buf);
}

static ssize_t test_mcu_transfer_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	uint32_t ret = 0;
	uint32_t i = 0;
	uint32_t test_num = 0;
	uint32_t test_len = 4096;
	uint32_t num_one = 1;
	uint32_t num_two = 2;
	uint32_t num_three = 3;
	uint32_t mcu_status = 0;
	uint8_t test_version_bin_4096[4096];
	uint8_t test_version_bin_x[PACKAGE_LENGTH];
	cdc_info("enter!");

	ret = kstrtoint(buf, DECIMAL, &test_num);
	if (ret != 0) {
		cdc_err("TEST: %s err: kstrtoint failed!", buf);
		return -EINVAL;
	}

	for (i = 0; i < test_len; i++)
		test_version_bin_4096[i] = num_one;

	for (i = 0; i < PACKAGE_LENGTH; i++) {
		if (i < test_len) {
			test_version_bin_x[i] = num_one;
		} else if (i >= test_len && i < 2 * test_len) {
			test_version_bin_x[i] = num_two;
		} else {
			test_version_bin_x[i] = num_three;
		}
	}

	if (test_num == 1) {
		ret = drv_panel_mcu_ota_update(test_version_bin_4096, test_len, &mcu_status);
		if (ret != 0)
			cdc_err("TEST: drv_panel_mcu_ota_update failed, ret = %d", ret);
	} else if (test_num == num_two) {
		ret = drv_panel_mcu_ota_update(test_version_bin_x, PACKAGE_LENGTH, &mcu_status);
		if (ret != 0)
			cdc_err("TEST: drv_panel_mcu_ota_update failed, ret = %d", ret);
	} else {
		cdc_err("TEST: Parameters err, test_num = %d, should 1 or 2", test_num);
	}

	cdc_info("TEST: test_mcu_transfer_store num is %d\n", test_num);
	return count;
}

static ssize_t test_mcu_exit_transfer(struct device *dev, struct device_attribute *attr, char *buf)
{
	int ret = 0;
	uint32_t mcu_status = 0;
	if (!buf) {
		cdc_err("buff is null\n");
		return -EINVAL;
	}
	cdc_info("enter!!");

	ret = drv_panel_mcu_exit_transfer(&mcu_status);
	if (ret != 0)
		cdc_err("drv_panel_mcu_exit_transfer error, ret = %d\n", ret);

	cdc_info("drv_panel_mcu_exit_transfer succ");

	ret = snprintf_s(buf, PAGE_SIZE - strlen(buf), PAGE_SIZE - strlen(buf),
		"TEST: exit transfer\n");
	if (ret == 0)
		cdc_err("snprintf_s error");
	return strlen(buf);
}

static ssize_t test_mcu_send_crc(struct device *dev, struct device_attribute *attr, char *buf)
{
	if (!buf) {
		cdc_err("buff is null\n");
		return -EINVAL;
	}
	cdc_info("enter!!");

	int ret = 0;
	uint32_t crc_hal = 0xABABABAB;
	uint32_t mcu_status = 0;
	ret = drv_panel_mcu_crc_checksum(crc_hal, &mcu_status);
	if (ret != 0)
		cdc_err("TEST: drv_panel_mcu_crc_checksum error, ret = %d\n", ret);

	cdc_info("TEST: crc checksum ret %d", ret);

	ret = snprintf_s(buf, PAGE_SIZE - strlen(buf), PAGE_SIZE - strlen(buf),
		"TEST: crc checksum ret %d\n", ret);
	if (ret == 0)
		cdc_err("snprintf_s error");
	return strlen(buf);
}

static ssize_t test_mcu_restart(struct device *dev, struct device_attribute *attr, char *buf)
{
	if (!buf) {
		cdc_err("buff is null\n");
		return -EINVAL;
	}
	cdc_info("enter!!");

	int ret = 0;
	uint32_t mcu_status = 0;
	ret = drv_panel_mcu_reset(&mcu_status);
	if (ret < 0)
		cdc_err("TEST: The Panel MCU enters the app state. nrc %d\n", mcu_status);

	cdc_info("TEST: The Panel MCU enters the app state. sid is %d\n", mcu_status);

	ret = snprintf_s(buf, PAGE_SIZE - strlen(buf), PAGE_SIZE - strlen(buf),
		"TEST: The Panel MCU enters the app state. sid is %d\n", mcu_status);
	if (ret == 0)
		cdc_err("snprintf_s error");
	return strlen(buf);
}

static ssize_t test_upgrade_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	if (!buf) {
		cdc_err("TEST: buff is null\n");
		return -EINVAL;
	}
	cdc_info("enter!!");

	uint32_t ret = 0;
	uint32_t mcu_status = 100;
	cdc_info("TEST: test_upgrade_printf %d\n", mcu_status);

	ret = snprintf_s(buf, PAGE_SIZE - strlen(buf), PAGE_SIZE - strlen(buf),
		"TEST: test upgrade printf %d\n", mcu_status);
	if (ret == 0)
		cdc_err("snprintf_s error");
	return strlen(buf);
}

static ssize_t test_upgrade_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int ret = 0;
	int32_t test_num = 0;

	ret = kstrtoint(buf, DECIMAL, &test_num);
	if (ret != 0) {
		cdc_err("TEST: %s err: kstrtoint failed!", buf);
		return -EINVAL;
	}
	if (test_num < 0)
		test_num = 0;

	cdc_info("TEST: test_upgrade_store num is %d\n", test_num);
	return count;
}

static DEVICE_ATTR(mcu_get_version, 0640, test_mcu_get_version, NULL);
static DEVICE_ATTR(mcu_reset, 0640, test_mcu_reset, NULL);
static DEVICE_ATTR(mcu_get_seed, 0640, test_mcu_get_seed, NULL);
static DEVICE_ATTR(mcu_send_key, 0640, test_mcu_send_key, NULL);
static DEVICE_ATTR(mcu_send_addlen, 0640, test_mcu_send_addlen, NULL);
static DEVICE_ATTR(mcu_request_download, 0640, test_mcu_request_download, NULL);
static DEVICE_ATTR(mcu_transfer_data, 0640, test_mcu_transfer_data, test_mcu_transfer_store);
static DEVICE_ATTR(mcu_exit_transfer, 0640, test_mcu_exit_transfer, NULL);
static DEVICE_ATTR(mcu_send_crc, 0640, test_mcu_send_crc, NULL);
static DEVICE_ATTR(mcu_restart, 0640, test_mcu_restart, NULL);
static DEVICE_ATTR(upgrade_test_printf, 0640, test_upgrade_show, test_upgrade_store);

static struct attribute *g_sysfs_attrs[] = {
	&dev_attr_mcu_get_version.attr,
	&dev_attr_mcu_reset.attr,
	&dev_attr_mcu_get_seed.attr,
	&dev_attr_mcu_send_key.attr,
	&dev_attr_mcu_send_addlen.attr,
	&dev_attr_mcu_request_download.attr,
	&dev_attr_mcu_transfer_data.attr,
	&dev_attr_mcu_exit_transfer.attr,
	&dev_attr_mcu_send_crc.attr,
	&dev_attr_mcu_restart.attr,
	&dev_attr_upgrade_test_printf.attr,
	NULL,
};

struct attribute_group sysfs_attr_group_upgrade = {
	.attrs = g_sysfs_attrs,
};

struct attribute_group get_sysfs_attr_group_upgrade()
{
	return sysfs_attr_group_upgrade;
}

static const struct file_operations lcd_sw_manage_fo = {
	.owner = THIS_MODULE,
	.unlocked_ioctl = lcd_sw_manage_ioctl,
	.compat_ioctl = lcd_sw_manage_ioctl,
};

int32_t lcd_sw_manage_init(void)
{
	cdc_info("[lcd_sw_manage_init:] Entering\n");

	major_number = register_chrdev(0, DEVICE_NAME, &lcd_sw_manage_fo);
	if (major_number < 0) {
		cdc_err("[lcd_sw_manage_init:] Failed to register a major number\n");
		return major_number;
	}
	cdc_info("[lcd_sw_manage_init:] Successful to register a major number %d\n", major_number);

	/* register device class */
	g_lcd_sw_ioctl_class = class_create(THIS_MODULE, CLASS_NAME);
	if (IS_ERR(g_lcd_sw_ioctl_class)) {
		unregister_chrdev(major_number, DEVICE_NAME);
		cdc_err("[lcd_sw_manage_init:] Class device register failed!\n");
		return PTR_ERR(g_lcd_sw_ioctl_class);
	}
	cdc_info("[lcd_sw_manage_init:] Class device register success!\n");

	/* register device driver */
	g_lcd_sw_ioctl_device = device_create(g_lcd_sw_ioctl_class, NULL, MKDEV(major_number, 0), NULL, DEVICE_NAME);
	/* Clean up if there is an error */
	if (IS_ERR(g_lcd_sw_ioctl_device)) {
		/* Repeated code but the alternative is goto statements */
		class_destroy(g_lcd_sw_ioctl_class);
		unregister_chrdev(major_number, DEVICE_NAME);
		cdc_err("[lcd_sw_manage_init:] Failed to create the device\n");
		return PTR_ERR(g_lcd_sw_ioctl_device);
	}
	cdc_info("[lcd_sw_manage_init:]  Test module register successful\n");
	return 0;
}

static int32_t __init f2_panel_upgrade_init(void)
{
	return lcd_sw_manage_init();
}

late_initcall(f2_panel_upgrade_init);
