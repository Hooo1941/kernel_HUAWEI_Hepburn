/* SPDX-License-Identifier: GPL-2.0 */
/*
 * hw_pogopin_sw.h
 *
 * huawei pogopin driver
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

#ifndef _HW_POGOPIN_SW_H_
#define _HW_POGOPIN_SW_H_

#include <linux/thermal.h>
#include <linux/kernel.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/err.h>
#include <linux/of_irq.h>
#include <linux/compiler.h>
#include <securec.h>
#include <linux/power_supply.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <chipset_common/hwpower/common_module/power_wakeup.h>
#include <chipset_common/hwpower/common_module/power_sysfs.h>
#include <linux/device.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/of_gpio.h>
#include <linux/delay.h>
#include <linux/workqueue.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <huawei_platform/log/hw_log.h>
#include <chipset_common/hwpower/common_module/power_pinctrl.h>
#include <chipset_common/hwpower/common_module/power_sysfs.h>
#include <chipset_common/hwpower/common_module/power_common_macro.h>
#include <linux/termios.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <linux/time.h>

#define ADC_SAMPLING_SUM_DEFAULT             0
#define ADC_SAMPLING_RETRY_TIMES             30
#define ADC_SAMPLE_COUNT_DEFAULT             0
#define VBATT_AVR_MAX_COUNT                  10
#define GET_ADC_FAIL                         (-1)
#define SMAPLING_OPTIMIZE                    5

#ifndef HIGH
#define HIGH                                 1
#endif
#ifndef LOW
#define LOW                                  0
#endif

#ifndef TRUE
#define TRUE                                 1
#endif
#ifndef FALSE
#define FALSE                                0
#endif

#define ACC_INFO_NO_LEN                      2
#define ACC_RECEIVE_MIN_LEN                  17
#define ACC_NAME_MAX_LEN                     23
#define ACC_INFO_LEN                         15
#define ACC_VALUE_MAX_LEN                    31
#define ACC_BT_MAC_PI_LEN                    3
#define ACC_BT_MAC_LEN                       16
#define ACC_BT_MAC_CP_FRONT                  8
#define ACC_BT_MAC_CP_LATER                  9
#define ACC_SUPPORT_SLE_CP_FRONT             1
#define ACC_SUPPORT_SLE_CP_LATER             2
#define ACC_DEV_INFO_LEN                     (ACC_NAME_MAX_LEN + ACC_VALUE_MAX_LEN + 2)
#define ACC_DEV_INFO_NUM_MAX                 20
#define N_HW_POGOPIN                         27
#define PUBLIC_BUF_MAX                       (3 * 1024)
#define UART_DIRECT_SPEED                    115200
#define ACC_DEV_INFO_MAX                     512
#define ACC_TIMEOUT_MSECS                    2000
#define KB_ONLINE_MIN_ADC_LIMIT              0
#define KB_ONLINE_MAX_ADC_LIMIT              1000

#define HW_POGO_SW_BITS_PER_BYTE             8
#define HW_POGO_SW_HANDSHAKE_PACKET_DAT1     0
#define HW_POGO_SW_HANDSHAKE_PACKET_DAT2     1
#define HW_POGO_SW_HANDSHAKE_ID              0x8866
#define HW_POGO_SW_TYPE_SLE                  0x88AA
#define HW_POGO_SW_NAME_MAX_LEN              23
#define HW_POGO_SW_VALUE_MAX_LEN             31
#define HW_POGO_SW_DEV_INFO_LEN              (HW_POGO_SW_NAME_MAX_LEN + HW_POGO_SW_VALUE_MAX_LEN + 2)
#define HW_POGO_SW_CALL_INFO_LEN             10
#define HW_POGO_SW_DETECT_WORK_MSECS         25000
#define HW_POGO_SW_VCC_ENABLE_MSECS          9000000
#define HW_POGO_SW_DETECT_DEBOUNCE_MSECS	 100

#define HW_POGO_SW_INVALID_CHANNEL          (-1)

enum hw_pogopin_sw_event {
	HW_POGOPIN_SW_UART_STATUS_OK = 0, /* notify pogopin uart status ok */
	HW_POGOPIN_SW_ADC_STATUS_FAIL,
	HW_POGOPIN_SW_UART_STATUS_FAIL,
	HW_POGOPIN_SW_UART_STATUS_TIMEOUT,
	HW_POGOPIN_PING_SUCC_SPARKLINK,
	HW_POGOPIN_DISCONNECT_SPARKLINK,
};

enum hw_pogopin_sw_status {
	DISCONNECTED = 0,
	CONNECTED,
	PING_SUCC,
	HW_POGOPIN_SW_KB_UNKNOWN,
};

struct hw_pogopin_sw_key_info {
	char name[HW_POGO_SW_NAME_MAX_LEN + 1];
	char value[HW_POGO_SW_VALUE_MAX_LEN + 1];
};

struct hw_pogopin_sw_dev_info {
	struct hw_pogopin_sw_key_info *key_info;
	u8 info_no; /* the informartion number of each device */
};

struct hw_pogopin_sw_acc_dev {
	struct device *dev;
	struct hw_pogopin_sw_dev_info dev_info;
};

struct hw_pogopin_sw_ldisc_data {
	char *buf;
	struct completion completion;
	bool buf_free;
	int count;
};

struct hw_pogopin_sw_info {
	struct platform_device *pdev;
	int hw_pogopin_sw_int_gpio;
	int hw_pogopin_sw_int_irq;
	int hw_pogopin_sw_vcc_en;
	enum hw_pogopin_sw_status hw_pogopin_sw_type;
	struct work_struct hw_pogopin_sw_work;
	int hw_pogopin_sw_gpio_status;
	int hw_pogopin_sw_adc_channel;
	struct wakeup_source *wakelock;
	struct notifier_block hw_pogopin_sw_check_uart_status_nb;
	struct delayed_work detect_acc_info_work;
	struct delayed_work detect_check_work;
	const char *tty_name;
	struct tty_struct *hw_pogopin_sw_tty;
	int handshak_offset;
	bool hw_pogopin_sw_open_flag;
	struct timer_list hw_pogopin_sw_timer;
	bool handshak_flag;
	bool data_complete;
	unsigned char hw_pogopin_sw_acc_info[ACC_VALUE_MAX_LEN];
	int hw_pogopin_sw_acc_info_num;
	bool hw_pogopin_sw_working;
	u32 support_sle;
	u32 acc_sle_flag;
	u32 uart_timeout;
};

enum hw_pogopin_sw_acc_info_index {
	HW_POGOPIN_SW_ACC_INFO_BEGIN = 0,
	HW_POGOPIN_SW_ACC_INFO_NO = HW_POGOPIN_SW_ACC_INFO_BEGIN,
	HW_POGOPIN_SW_ACC_INFO_STATE,
	HW_POGOPIN_SW_ACC_INFO_MAC,
	HW_POGOPIN_SW_ACC_INFO_MODEL_ID,
	HW_POGOPIN_SW_ACC_INFO_SUBMODEL_ID,
	HW_POGOPIN_SW_ACC_INFO_VERSION,
	HW_POGOPIN_SW_ACC_INFO_BUSINESS,
	HW_POGOPIN_SW_ACC_INFO_SLE,
	HW_POGOPIN_SW_ACC_INFO_END,
};

enum hw_pogopin_sw_acc_info_index_flag {
	HW_POGOPIN_SW_ACC_INFO_BEGIN_FLAG = 0,
	HW_POGOPIN_SW_ACC_INFO_MAC_FLAG = HW_POGOPIN_SW_ACC_INFO_BEGIN_FLAG,
	HW_POGOPIN_SW_ACC_INFO_MODEL_ID_FLAG = 6,
	HW_POGOPIN_SW_ACC_INFO_SUBMODEL_ID_FLAG = 9,
	HW_POGOPIN_SW_ACC_INFO_VERSION_FLAG,
	HW_POGOPIN_SW_ACC_INFO_BUSINESS_FLAG,
	HW_POGOPIN_SW_ACC_INFO_END_FLAG,
};

struct hw_pogopin_sw_acc_kb_info {
	char name[ACC_NAME_MAX_LEN + 1];
	char value[ACC_VALUE_MAX_LEN + 1];
};

/* dev no should begin from 1 */
enum hw_pogopin_sw_acc_dev_no {
	ACC_POGO_DEV_NO_BEGIN = 1,
	ACC_POGO_DEV_NO_PEN = ACC_POGO_DEV_NO_BEGIN,
	ACC_POGO_DEV_NO_KB,
	ACC_POGO_DEV_NO_END,
};

void hw_pogopin_sw_event_notify(enum hw_pogopin_sw_event event);
void hw_pogopin_sw_report_acc_info(struct hw_pogopin_sw_acc_kb_info *kb_info, enum hw_pogopin_sw_status type);
int hw_pogopin_sw_event_notifier_register(struct notifier_block *nb);
int hw_pogopin_sw_event_notifier_unregister(struct notifier_block *nb);
#endif /* _HW_POGOPIN_SW_H_ */
