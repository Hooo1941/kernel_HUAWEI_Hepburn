/* Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#ifndef __LCD_SYSFS_H__
#define __LCD_SYSFS_H__

#define LCD_SYSFS_MAX 101

#define ARRAY_SIZE(array) (sizeof(array) / sizeof((array)[0]))

#define LCD_MAX 2
#define LCD_DSI_INDEX 0
#define LCD_DP_INDEX 1

enum lcd_sysfs_index {
	SER_PATTERN_INDEX,
	DESER_PATTERN_INDEX,
	PANEL_INFO_INDEX,
	SER_MAIN_REGS,
	DESER_MAIN_REGS,
	SER_PANEL_ON,
	SER_PANEL_OFF,
	SER_BRIGHTNESS,
	LCD_OTA_VERSION,
	LIGHT_SENSOR,
	LCD_SN_CODE,
	LCD_FACTORY_OPS,
	LCD_FAIL_DET,
	LCD_DEFAULT,
};
/* sysfs support enum */
enum lcd_sysfs_support {
	SYSFS_NOT_SUPPORT,
	SYSFS_SUPPORT,
};

#define LCD_FAIL (-1)
#define LCD_OK 0

struct lcd_sysfs_ops *lcd_get_sysfs_ops(struct device *dev);
struct lcd_sysfs_lock *lcd_get_sysfs_lock(struct device *dev);
int lcd_sysfs_ops_register(struct lcd_sysfs_ops *ops, int index);
int lcd_sysfs_ops_unregister(struct lcd_sysfs_ops *ops);
int lcd_create_sysfs(struct i2c_client *client, int index);
void lcd_lock_init(struct lcd_sysfs_lock *tmp_lock, int index);
extern struct i2c_client *get_ti981_client(void);
struct lcd_sysfs_ops {
	int (*check_support)(int index);
	ssize_t (*ser_pattern_port0_show)(struct device *dev, struct device_attribute *attr, char *buf);
	ssize_t (*deser_pattern_port0_show)(struct device *dev, struct device_attribute *attr, char *buf);
	ssize_t (*panel_info_show)(struct device *dev, struct device_attribute *attr, char *buf);
	ssize_t (*ser_main_regs_show)(struct device *dev, struct device_attribute *attr, char *buf);
	ssize_t (*ser_main_regs_store)(struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
	ssize_t (*deser_main_regs_show)(struct device *dev, struct device_attribute *attr, char *buf);
	ssize_t (*deser_main_regs_store)(struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
	ssize_t (*ser_panel_on_store)(struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
	ssize_t (*ser_panel_off_store)(struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
	ssize_t (*ser_brightness_show)(struct device *dev, struct device_attribute *attr, char *buf);
	ssize_t (*ser_brightness_store)(struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
	ssize_t (*lcd_ota_version_show)(struct device *dev, struct device_attribute *attr, char *buf);
	ssize_t (*light_sensor_show)(struct device *dev, struct device_attribute *attr, char *buf);
	ssize_t (*light_sensor_store)(struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
	ssize_t (*lcd_sn_code_show)(struct device *dev, struct device_attribute *attr, char *buf);
	ssize_t (*lcd_sn_code_store)(struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
	ssize_t (*lcd_factory_show)(struct device *dev, struct device_attribute *attr, char *buf);
	ssize_t (*lcd_factory_store)(struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
	ssize_t (*lcd_fail_det_show)(struct device *dev, struct device_attribute *attr, char *buf);
	ssize_t (*lcd_default_test_show)(struct device *dev, struct device_attribute *attr, char *buf);
	ssize_t (*lcd_default_test_store)(struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
};

struct lcd_sysfs_lock {
	struct mutex ser_pattern_lock;
	struct mutex deser_pattern_lock;
	struct mutex panel_info_lock;
	struct mutex ser_main_regs_lock;
	struct mutex deser_main_regs_lock;
	struct mutex ser_panel_on_lock;
	struct mutex ser_panel_off_lock;
	struct mutex ser_brightness_lock;
	struct mutex lcd_ota_version_lock;
	struct mutex light_sensor_lock;
	struct mutex lcd_sn_lock;
	struct mutex lcd_factory_lock;
	struct mutex lcd_fail_det_lock;
	struct mutex lcd_default_test_lock;
};

enum {
	SYSFS_DEBUG_EMERG = 1 << 7,
	SYSFS_DEBUG_ALERT = 1 << 6,
	SYSFS_DEBUG_CRIT = 1 << 5,
	SYSFS_DEBUG_ERR = 1 << 4,
	SYSFS_DEBUG_WARING = 1 << 3,
	SYSFS_DEBUG_NOTICE = 1 << 2,
	SYSFS_DEBUG_INFO = 1 << 1,
	SYSFS_DEBUG_DEBUG = 1 << 0,
};

static uint32_t sys_debug_mask =
	SYSFS_DEBUG_DEBUG | SYSFS_DEBUG_INFO | SYSFS_DEBUG_WARING | SYSFS_DEBUG_EMERG | SYSFS_DEBUG_ALERT | SYSFS_DEBUG_ERR;

#define SYSFS_LOG_TAG "LCD_SYSFS"

#define sysfs_debug(fmt, ...) \
	do { \
		if (sys_debug_mask & SYSFS_DEBUG_DEBUG) \
			printk("[%s]" \
				   "[DEBUG] [%u %s]" fmt "\n", \
				SYSFS_LOG_TAG, \
				__LINE__, \
				__func__, \
				##__VA_ARGS__); \
	} while (0)

#define sysfs_info(fmt, ...) \
	do { \
		if (sys_debug_mask & SYSFS_DEBUG_INFO) \
			printk("[%s]" \
				   "[INFO] [%u %s]" fmt "\n", \
				SYSFS_LOG_TAG, \
				__LINE__, \
				__func__, \
				##__VA_ARGS__); \
	} while (0)

#define sysfs_notice(fmt, ...) \
	do { \
		if (sys_debug_mask & SYSFS_DEBUG_NOTICE) \
			printk("[%s]" \
				   "[NOTICE] [%u %s]" fmt "\n", \
				SYSFS_LOG_TAG, \
				__LINE__, \
				__func__, \
				##__VA_ARGS__); \
	} while (0)

#define sysfs_warn(fmt, ...) \
	do { \
		if (sys_debug_mask & SYSFS_DEBUG_WARING) \
			printk("[%s]" \
				   "[WARN] [%u %s]" fmt "\n", \
				SYSFS_LOG_TAG, \
				__LINE__, \
				__func__, \
				##__VA_ARGS__); \
	} while (0)

#define sysfs_err(fmt, ...) \
	do { \
		if (sys_debug_mask & SYSFS_DEBUG_ERR) \
			printk("[%s]" \
				   "[ERROR] [%u %s]" fmt "\n", \
				SYSFS_LOG_TAG, \
				__LINE__, \
				__func__, \
				##__VA_ARGS__); \
	} while (0)

#define sysfs_crit(fmt, ...) \
	do { \
		if (sys_debug_mask & SYSFS_DEBUG_CRIT) \
			printk("[%s]" \
				   "[CRIT] [%u %s]" fmt "\n", \
				SYSFS_LOG_TAG, \
				__LINE__, \
				__func__, \
				##__VA_ARGS__); \
	} while (0)

#define sysfs_alert(fmt, ...) \
	do { \
		if (sys_debug_mask & SYSFS_DEBUG_ALERT) \
			printk("[%s]" \
				   "[ALERT] [%u %s]" fmt "\n", \
				SYSFS_LOG_TAG, \
				__LINE__, \
				__func__, \
				##__VA_ARGS__); \
	} while (0)

#define sysfs_emerg(fmt, ...) \
	do { \
		if (sys_debug_mask & SYSFS_DEBUG_EMERG) \
			printk("[%s]" \
				   "[EMERG] [%u %s]" fmt "\n", \
				SYSFS_LOG_TAG, \
				__LINE__, \
				__func__, \
				##__VA_ARGS__); \
	} while (0)

#endif
