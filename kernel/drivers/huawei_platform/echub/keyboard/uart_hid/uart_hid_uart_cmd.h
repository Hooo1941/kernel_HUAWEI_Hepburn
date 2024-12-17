/*
 * Huawei uart hid Driver for uart cmd
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
#ifndef UART_HID_UART_CMD_H
#define UART_HID_UART_CMD_H

#define SOC_CMD_ERR_OK          0x00
#define SOC_CMD_ERR_CODE        0x01
#define SOC_CMD_ERR_LEN         0x02
#define SOC_CMD_ERR_TIMEOUT     0x04
#define SOC_CMD_ERR_PEC         0x08
#define SOC_CMD_ERR_BUSY        0x10
#define SOC_CMD_ERR_DATA        0x20
#define SOC_CMD_ERR_STATUS      0x40

int uart_hid_dev_check(void);
int uart_hid_cmd_send_report(u8 *data, size_t size);
int uart_hid_cmd_recv_report(u8 *data, size_t buf_size, size_t *ret_size);
int uart_hid_cmd_init(const char* uart_port, long baud);
int uart_hid_cmd_deinit(void);
void uart_hid_cmd_clear(void);
#endif /* UART_HID_UART_CMD_H */
