/*
 * Huawei uart hid Driver for uart hid report
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

#ifndef UART_HID_REPORT_H
#define UART_HID_REPORT_H
#include "uart_hid_core.h"

int uart_hid_fetch_hid_descriptor(struct uart_hid *ihid);
struct hid_ll_driver *get_uart_hid_ll_driver(void);
void uart_hid_input_report(struct uart_hid *ihid);
#endif /* UART_HID_REPORT_H */
