/*
 * Huawei uart hid Driver for uart protocol
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

#ifndef UART_HID_UART_PROTOCOL_H
#define UART_HID_UART_PROTOCOL_H

int uart_hid_uart_init(const char *uart_port, long baud);
int uart_hid_uart_deinit(void);
int uart_hid_uart_write(u8 *buf, size_t len);
int uart_hid_uart_read(u8 *buf, size_t buf_len, size_t want_len, size_t *len,
			int time_out);
void uart_hid_reconfig_baudrate(long baud);
void uart_hid_uart_clear_state(void);

#endif /* UART_HID_UART_PROTOCOL_H */
