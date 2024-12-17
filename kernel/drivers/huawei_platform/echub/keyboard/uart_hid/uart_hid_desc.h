/*
 * Huawei uart hid Driver for uart keyboard hid report
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

#ifndef UART_HID_DESC_H
#define UART_HID_DESC_H

__u8 *uart_hid_get_keyboard_hid_desc(void);
__u8 *uart_hid_get_keyboard_report_desc(unsigned int *size);

#endif /* UART_HID_DESC_H */
