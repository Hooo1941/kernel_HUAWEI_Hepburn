/* Copyright (c) Huawei Technologies Co., Ltd. 2019-2021. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
 * GNU General Public License for more details.
 *
 */

#ifndef UPGRADE_CHANNEL_MANAGE_H
#define UPGRADE_CHANNEL_MANAGE_H

#include <linux/regmap.h>
#include <linux/debugfs.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/init.h>
#include <asm/uaccess.h>
#include <linux/types.h>

#define DEVICE_NAME "lcd_panel"
#define CLASS_NAME "lcd_panel_module"
#define VERSION_LEN 64

typedef enum {
	IOCTL_NUM_0,
	IOCTL_NUM_1,
	IOCTL_NUM_2,
	IOCTL_NUM_3,
	IOCTL_NUM_4,
	IOCTL_NUM_5,
	IOCTL_NUM_6,
	IOCTL_NUM_7,
	IOCTL_NUM_8,
	IOCTL_NUM_9,
	IOCTL_NUM_10,
	IOCTL_NUM_11,
}lcdpanel_ioctl_num;

struct lcd_panel_ota_version {
	char boot_version[VERSION_LEN];
	char app_version[VERSION_LEN];
	char hardware_version[VERSION_LEN];
	uint32_t boot_version_len;
	uint32_t app_version_len;
	uint32_t hardware_len;
	uint32_t panel_should_update;
	uint32_t reserved;
};

struct lcd_panel_ota_fireware {
	unsigned char *version_bin;
	uint32_t len;
	uint32_t address;
	uint32_t crc_hal;
	uint32_t nrc;
	uint32_t reserved1;
	uint32_t reserved2;
};

struct lcd_panel_ota_seed {
	uint32_t seed;
	uint32_t nrc;
	uint32_t index;
	uint32_t reserved;
};

struct lcd_panel_ota_key {
	uint32_t key;
	uint32_t nrc;
	uint32_t reserved;
};
#define LCDPANEL_IOCTL_BASE 'L'
#define LCDPANEL_IOCTL_CHECK_STATE _IOWR(LCDPANEL_IOCTL_BASE, IOCTL_NUM_0, int)
#define LCDPANEL_IOCTL_GET_VERSION _IOWR(LCDPANEL_IOCTL_BASE, IOCTL_NUM_1, struct lcd_panel_ota_version)
#define LCDPANEL_IOCTL_SESSION_CONTROL _IOWR(LCDPANEL_IOCTL_BASE, IOCTL_NUM_2, int)
#define LCDPANEL_IOCTL_GET_SLOT _IOWR(LCDPANEL_IOCTL_BASE, IOCTL_NUM_3, int)
#define LCDPANEL_IOCTL_GET_SEED _IOWR(LCDPANEL_IOCTL_BASE, IOCTL_NUM_4, struct lcd_panel_ota_seed)
#define LCDPANEL_IOCTL_SEND_KEY _IOWR(LCDPANEL_IOCTL_BASE, IOCTL_NUM_5, struct lcd_panel_ota_key)
#define LCDPANEL_IOCTL_FIREWARE_TRANSFER _IOWR(LCDPANEL_IOCTL_BASE, IOCTL_NUM_6, struct lcd_panel_ota_fireware)
#define LCDPANEL_IOCTL_SET_SLOT _IOWR(LCDPANEL_IOCTL_BASE, IOCTL_NUM_7, int)
#define LCDPANEL_IOCTL_MCU_RESET _IOWR(LCDPANEL_IOCTL_BASE, IOCTL_NUM_8, int)
#define LCDPANEL_IOCTL_UPDATE_PROGRESS _IOWR(LCDPANEL_IOCTL_BASE, IOCTL_NUM_9, int)
#define LCDPANEL_IOCTL_UPDATE_RESULT _IOWR(LCDPANEL_IOCTL_BASE, IOCTL_NUM_10, int)
#define LCDPANEL_IOCTL_SET_OTAMODE _IOWR(LCDPANEL_IOCTL_BASE, IOCTL_NUM_11, int)

int32_t lcd_sw_manage_init(void);
uint32_t get_ota_update_status(void);
struct attribute_group get_sysfs_attr_group_upgrade(void);
#endif /* UPGRADE_CHANNEL_MANAGE_H */
