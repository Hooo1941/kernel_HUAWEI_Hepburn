/* SPDX-License-Identifier: GPL-2.0 */
/*
 * cam_star_module_adapter.h
 *
 * cam_star_module_adapter header file
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

#ifndef _CAM_STAR_MODULE_H_
#define _CAM_STAR_MODULE_H_

#include <platform_include/camera/native/cam_star.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/gpio.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/workqueue.h>
#include <linux/bitops.h>
#include <linux/mutex.h>
#include <linux/notifier.h>
#include <linux/videodev2.h>
#include <media/v4l2-subdev.h>
#include <dsm/dsm_pub.h>

#define CALIB_INFO_NUM_OF_PAIR      72
#define NUM_OF_READOUT_HEIGHT_DATA  20
#define NUM_OF_CALIB_ADD_INFO       14

enum cam_star_module_gpio {
	MCU_INT = 0,
	MCU_BOOT,
	MCU_RESET,
	MOTOR_EN,
	UART_TX,
	UART_RX,
	GPIO_MAX,
};

enum gpio_direction {
	IN = 0,
	OUT,
};

enum gpio_value {
	LOW = 0,
	HIGH,
};

struct star_notify_intf_t {
	struct star_notify_vtbl *vtbl;
};

struct star_notify_vtbl {
	void (*rpmsg_cb)(struct star_notify_intf_t *i, struct star_event_t *star_ev);
};

enum mcu_log_info_t {
	MOTOR = 1,
	TMR,
	HEIGHT,
	PERI,
	ERROR,
};

enum power_state_t {
	POWER_OFF = 0,
	POWER_ON,
	BOOTLOADER,
	PROBE,
};

enum normal_calib_state_t {
	NORMAL = 0,
	CALIB = 1,
	CALIB_AND_RELEASE_WRITE_PROTECT = 3,
};

enum fw_update_state_t {
	UPDATE_WHEN_NOT_EQUAL = 0,
	UPDATE_WHEN_GREATER_THAN,
	UPDATE_ALWAYS,
	NOT_UPDATE,
};

struct mcu_state_t {
	int motor_speed;
	int target_pos;
	int target_step;
	int step_backward_flag;
};

struct power_setting_t {
	int mcu_int_gpio;
	int mcu_boot_gpio;
	int mcu_reset_gpio;
	int motor_en_gpio;
};

struct fw_state_t {
	int fw_size;
	u8 *fw_data;
	u32 fw_ver_id; // fw version id which is to update
	u32 version_id; // fw version id read from MCU
};

struct atcmd_param_t {
	int save_height_value;
	int height_value[NUM_OF_READOUT_HEIGHT_DATA];
};

struct dis_tmr_pair_t {
	u16 dis;
	u16 tmr;
};

struct calib_info_t {
	u16 num_of_pair;
	u16 system_accuracy_err;
	struct dis_tmr_pair_t dis_tmr_pair[CALIB_INFO_NUM_OF_PAIR];
};

struct calib_otp_t {
	u16 flash_offset;
	struct calib_info_t calib_info;
	/* calib add info:
	* u16 crc_check;
	* u16 calib_data_type;
	* u16 equipment_soft_version;
	* u16 reserved_1;
	* u16 site_id;
	* u16 reserved_2;
	*/
	u16 calib_add_info[NUM_OF_CALIB_ADD_INFO];
	u16 crc_readout;
};

struct i2c_read_write_op_t {
	u8 reg_u8;
	u8 data_u8;
	u16 reg_u16;
	u16 data_u16;
};

enum int_state_t {
	INT_DISABLE = 0,
	INT_ENABLE,
};

enum maintenance_state_t {
	NOT_REPAIRED = 0,
	REPAIRED,
};

enum flash_crc_check_t {
	CRC_SUC = 0,
	CRC_FAIL,
};

struct cam_star_module_device_info {
	const char *ic_name;
	struct mutex lock;
	struct i2c_client *client;
	struct device *dev;
	/* int */
	int irq;
	u32 isr_err;
	enum int_state_t int_state;
	/* state */
	enum power_state_t power_state;
	enum normal_calib_state_t normal_calib_state;
	/* height/speed of MCU */
	struct mcu_state_t mcu_state;
	/* power param */
	struct power_setting_t power_setting;
	unsigned int bootloader_delay;
	/* fw */
	struct fw_state_t fw_state;
	enum fw_update_state_t fw_update_state;
	/* bootloader id */
	u8 bootloader_id;
	/* v4l2 */
	struct v4l2_subdev subdev;
	struct star_notify_intf_t notify; // MCU IC exception event intf
	struct star_api_fn_t *api_func_tbl; // MCU IC func table
	int dev_major;
	struct class *dev_class;
	const struct attribute_group **star_attr_group;
	/* atcmd param */
	struct atcmd_param_t atcmd_param;
	/* calib info */
	struct calib_otp_t calib_otp;
	/* i2c reg read and write */
	struct i2c_read_write_op_t i2c_read_write_op;
	/* move_done int */
	volatile bool move_done;
	/* maintenance mode */
	enum maintenance_state_t maintenance_state;
	/* flash crc check tag */
	enum flash_crc_check_t crc_check;
	/* dmd report client */
	struct dsm_client *dsm_client;
};

struct star_api_fn_t {
	int (*open)(struct cam_star_module_device_info *di);
	int (*close)(struct cam_star_module_device_info *di);
	int (*get_info)(struct cam_star_module_device_info *di, struct sensormotor_info *info);
	int (*power_on)(struct cam_star_module_device_info *di);
	int (*power_off)(struct cam_star_module_device_info *di, enum power_off_state_t state);
	int (*chip_init)(struct cam_star_module_device_info *di);
	int (*set_bootloader)(struct cam_star_module_device_info *di);
	int (*matchid)(struct cam_star_module_device_info *di);
	int (*download_firmware)(struct cam_star_module_device_info *di);
	/* normal mode */
	int (*set_code)(struct cam_star_module_device_info *di, u16 code);
	int (*set_speed)(struct cam_star_module_device_info *di, u16 speed);
	int (*start)(struct cam_star_module_device_info *di);
	int (*stop)(struct cam_star_module_device_info *di);
	int (*get_real_height)(struct cam_star_module_device_info *di, u16 *real_code);
	/* calib mode */
	int (*set_calib_mode)(struct cam_star_module_device_info *di, enum calib_mode_t state);
	int (*calib_set_step)(struct cam_star_module_device_info *di, int step);
	int (*calib_set_speed)(struct cam_star_module_device_info *di, u16 speed);
	int (*calib_start)(struct cam_star_module_device_info *di);
	int (*calib_get_real_height)(struct cam_star_module_device_info *di, int16_t *real_code);
	int (*calib_get_real_tmr)(struct cam_star_module_device_info *di, int16_t *real_code);
	int (*calib_set_info)(struct cam_star_module_device_info *di);
	int (*calib_set_add_info)(struct cam_star_module_device_info *di);
	int (*calib_read_info)(struct cam_star_module_device_info *di, u16 *read_value, int *size_read_value);
	int (*calib_crc_check)(struct cam_star_module_device_info *di);
	/* if not calibrated, reload calib data from eeprom to flash after download fw */
	int (*reload_calib_data_from_eeprom)(struct cam_star_module_device_info *di);
	/* if MCU not return ack during i2c trans, dump i2c debug reg */
	int (*read_mcu_i2c_debug_data)(struct cam_star_module_device_info *di);
	/* maintenance mode */
	int (*set_repaired_tag)(struct cam_star_module_device_info *di);
	int (*set_maintenance_delta)(struct cam_star_module_device_info *di, u16 delta);
	/* flash crc check */
	int (*flash_crc_check)(struct cam_star_module_device_info *di);
};

struct cam_star_module_intf_t {
	char *ic_name;
	int (*ic_init)(struct i2c_client *client, const struct i2c_device_id *id, struct cam_star_module_device_info *di);
	int (*ic_remove)(struct i2c_client *client);
};

#endif /* _CAM_STAR_MODULE_H_ */
