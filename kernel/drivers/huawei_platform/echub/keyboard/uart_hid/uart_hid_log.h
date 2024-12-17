/*
 * Huawei uart hid Driver for log
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

#ifndef UART_HID_LOG_H
#define UART_HID_LOG_H

/* debug option */
bool uart_hid_debug_log_flag();

#define uart_hid_dbg(fmt, args...)                              \
	do {                                                        \
		if (uart_hid_debug_log_flag())                            \
			printk(KERN_INFO "uart_hid:%s " fmt, __FUNCTION__,  \
			       ##args);                                     \
	} while (0)

#define uart_hid_err(fmt, args...)                              \
	do {                                                        \
			printk(KERN_ERR "uart_hid:%s " fmt, __FUNCTION__,   \
			       ##args);                                     \
	} while (0)

#endif /* UART_HID_LOG_H */
