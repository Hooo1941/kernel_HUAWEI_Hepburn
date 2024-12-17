/*
 * hi116x_driver.h
 *
 * interface for huawei hi116x_fwupdate driver
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
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
#ifndef _HI116X_DRIVER_H_
#define _HI116X_DRIVER_H_
#include <securec.h>
#include <linux/pinctrl/consumer.h>
#include <linux/types.h>
#include <linux/device.h>
#include <linux/reboot.h>
#include <linux/pm_wakeup.h>
#include <linux/firmware.h>
#include "sparklink.h"
#include "sparklink_i2c_ops.h"
#include "hi116x_wltx.h"
#include <chipset_common/hwpower/common_module/power_wakeup.h>
#include <chipset_common/hwpower/common_module/power_sysfs.h>
#include <huawei_platform/usb/hw_pogopin_sw.h>
#include <chipset_common/hwpower/common_module/power_event_ne.h>

#define REG_ADDR 0
#define REG_SIZE 1
#define REG_VALUE 2
#define REG_MAX 3

enum hi1162_firmware_type {
	HI1162_SSBEFC_FW_INFO = 0,
	HI1162_CKFF_FW_INFO,
	HI1162_SSB_FW_INFO,
	HI1162_NV_FW_INFO,
	HI1162_BT_FW_INFO,
	HI1162_APP_FW_INFO,
	HI1162_OTA_FW_INFO,
	HI1162_FW_INFO_MAX,
};

#define HI1162_IRQ_ADDR                       0x0040
#define HI1162_IRQ_LEN                        2
#define HI1162_CLR_IRQ_ADDR                   0x0014
#define HI1162_IRQ_WLTX_REPORT_ACC_INFO       BIT(0)
#define HI1162_IRQ_WLTX_REPORT_HALL_STATUS    BIT(1)
#define HI1162_SWITCH_GPIO_DELAY              100
#define HI1162_SWITCH_GPIO_AVOID_OVERLAP      (HI1162_SWITCH_GPIO_DELAY + 100)
#define HI1162_SWITCH_GPIO_UP_WAIT            15

enum wireless_tx_sysfs_type {
	WL_TX_SYSFS_TX_OPEN = 0,
	WL_TX_SYSFS_RX_PRODUCT_TYPE,
	WL_TX_SYSFS_TX_STATUS,
	WL_TX_SYSFS_TX_VIN,
	WL_TX_SYSFS_TX_IIN,
	WL_TX_SYSFS_TX_IIN_AVG,
	WL_TX_SYSFS_HALL_STATE,
};

struct hi1162_device_info {
	struct device *dev;
	struct i2c_client *client;
	struct work_struct irq_work;
	struct delayed_work fwupdate_work;
	struct delayed_work check_update_work;
	struct delayed_work connect_i2c_work;
	int device_id;
	int reset_gpio;
	int irq_gpio;
	int buck_boost_en_gpio;
	int i2c_switch_gpio;
	u16 irq_val;
	int wakeup_gpio;
	int irq;
	bool irq_active;
	sparklink_reg version_app_reg;
	sparklink_reg version_ssb_reg;
	sparklink_reg version_bt_reg;
	sparklink_reg chip_id_reg;
	sparklink_reg fw_info[HI1162_FW_INFO_MAX];
	const char *fw_path[HI1162_FW_INFO_MAX];
	bool need_update[HI1162_FW_INFO_MAX];
	const char *tty_name;
	u32 default_baud;
	u32 max_baud;
	unsigned int support_wltx;
	unsigned int support_switch_channel;
	unsigned int support_pen_attach_q_detect;
	unsigned int support_kb_attach_pogopin;
	struct wakeup_source *wake_lock;
	struct pinctrl *pctrl;
	struct pinctrl_state *pin_default;
	struct notifier_block reboot_notifier;
	struct notifier_block fb_notifier;
	struct notifier_block pen_attach_notifier;
	struct notifier_block kb_attach_notifier;
	char rx_product_type[HI1162_RX_PRODUCT_TYPE_BUFF_LEN];
	u8 hall_state;
};

int hi1162_device_set_mode(int powermode);
void hi1162_device_set_state(int state);
int hi1162_device_get_state(void);
int hi1162_device_get_chipid_state(void);
int hi1162_check_id(struct hi1162_device_info *di);
int hi1162_wltx_open_tx(struct hi1162_device_info *di, long val);
void hi1162_wltx_get_rx_product_type(struct hi1162_device_info *di);
void hi1162_wltx_get_tx_info_byte(struct hi1162_device_info *di, u16 *info, u8 reg);
void hi1162_wltx_get_tx_info_block(struct hi1162_device_info *di, u16 *info, u8 reg);
void hi1162_get_hall_state(struct hi1162_device_info *di);
int hi1162_report_acc_info(struct hi1162_device_info *di);
void wakeup_116x_before_i2c_ops(struct hi1162_device_info *di);

#endif /* _HI116X_DRIVER_H_ */
