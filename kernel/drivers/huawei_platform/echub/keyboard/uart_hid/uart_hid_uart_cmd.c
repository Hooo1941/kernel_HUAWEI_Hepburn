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
#include <linux/mutex.h>
#include <linux/string.h>
#include <linux/kernel.h>
#include <securec.h>

#include "uart_hid_log.h"
#include "uart_hid_uart_protocol.h"
#include "uart_hid_uart_cmd.h"

#define UART_HID_CMD_HAED 0x5A6B

#define UART_HID_DEV_CHECK 0x0900
#define UART_HID_SEND 0x0901
#define UART_HID_RECV 0x0902
#define UART_HID_CHECK_ACK 0x0980
#define UART_HID_SEND_ACK 0x0981
#define UART_HID_RECV_ACK 0x0982

#define UART_SEND_DATA_LEN 6
#define UART_RECV_DATA_LEN 11
#define UART_CMD_BUF_LEN 11

#define UART_RES_BUF_MIN 6
#define UART_RES_BUF_MAX 20
#define UART_RECV_TIMEOUT 100

typedef struct _uart_hid_cmd_data {
	u16 cmd_head;
	u16 cmd_type;
	u8 cmd_len;
	u8 cmd_data[UART_CMD_BUF_LEN];
	u8 cmd_err;
	size_t response_len;
} uart_hid_cmd_data;

/* uart_hic_cmd_index is cmd index,
 if you want to add cmd,
 you need add a index in uart_hid_cmd_index enum
 and uart_hid_cmd_list */
typedef enum _uart_hic_cmd_index {
	UART_DEV_CHECK = 0x0,
	UART_DEV_SEND,
	UART_DEV_RECV,
	UART_DEV_CHECK_ACK,
	UART_DEV_SEND_ACK,
	UART_DEV_RECV_ACK
} uart_hic_cmd_index;

const static uart_hid_cmd_data uart_hid_cmd_list[] = {
	{ UART_HID_CMD_HAED, UART_HID_DEV_CHECK, 1, { 0x01 }, 0, 8 },
	{ UART_HID_CMD_HAED, UART_HID_SEND, 4, { 0x0 }, 0, 7 },
	{ UART_HID_CMD_HAED, UART_HID_RECV, 0, { 0x0 }, 0, 18 },
	{ UART_HID_CMD_HAED, UART_HID_CHECK_ACK, 1, { 0x01 }, 0, 0 },
	{ UART_HID_CMD_HAED, UART_HID_SEND_ACK, 0, { 0x00 }, 0, 0 },
	{ UART_HID_CMD_HAED, UART_HID_RECV_ACK, 11, { 0x00 }, 0, 0 },
};

#define UART_HID_CMD_BUF_LEN 20

static u8 uart_hid_cmd_send_buf[UART_HID_CMD_BUF_LEN];
static u8 uart_hid_cmd_recv_buf[UART_HID_CMD_BUF_LEN];
static u8 pce_init_value;
static struct mutex uart_hid_cmd_lock;

static uint8_t calc_bus_pec(u8 pec, u8 data)
{
	static const u8 pec_table[] = {
		0x00, 0x07, 0x0E, 0x09, 0x1C, 0x1B, 0x12, 0x15, 0x38, 0x3F,
		0x36, 0x31, 0x24, 0x23, 0x2A, 0x2D, 0x70, 0x77, 0x7E, 0x79,
		0x6C, 0x6B, 0x62, 0x65, 0x48, 0x4F, 0x46, 0x41, 0x54, 0x53,
		0x5A, 0x5D, 0xE0, 0xE7, 0xEE, 0xE9, 0xFC, 0xFB, 0xF2, 0xF5,
		0xD8, 0xDF, 0xD6, 0xD1, 0xC4, 0xC3, 0xCA, 0xCD, 0x90, 0x97,
		0x9E, 0x99, 0x8C, 0x8B, 0x82, 0x85, 0xA8, 0xAF, 0xA6, 0xA1,
		0xB4, 0xB3, 0xBA, 0xBD, 0xC7, 0xC0, 0xC9, 0xCE, 0xDB, 0xDC,
		0xD5, 0xD2, 0xFF, 0xF8, 0xF1, 0xF6, 0xE3, 0xE4, 0xED, 0xEA,
		0xB7, 0xB0, 0xB9, 0xBE, 0xAB, 0xAC, 0xA5, 0xA2, 0x8F, 0x88,
		0x81, 0x86, 0x93, 0x94, 0x9D, 0x9A, 0x27, 0x20, 0x29, 0x2E,
		0x3B, 0x3C, 0x35, 0x32, 0x1F, 0x18, 0x11, 0x16, 0x03, 0x04,
		0x0D, 0x0A, 0x57, 0x50, 0x59, 0x5E, 0x4B, 0x4C, 0x45, 0x42,
		0x6F, 0x68, 0x61, 0x66, 0x73, 0x74, 0x7D, 0x7A, 0x89, 0x8E,
		0x87, 0x80, 0x95, 0x92, 0x9B, 0x9C, 0xB1, 0xB6, 0xBF, 0xB8,
		0xAD, 0xAA, 0xA3, 0xA4, 0xF9, 0xFE, 0xF7, 0xF0, 0xE5, 0xE2,
		0xEB, 0xEC, 0xC1, 0xC6, 0xCF, 0xC8, 0xDD, 0xDA, 0xD3, 0xD4,
		0x69, 0x6E, 0x67, 0x60, 0x75, 0x72, 0x7B, 0x7C, 0x51, 0x56,
		0x5F, 0x58, 0x4D, 0x4A, 0x43, 0x44, 0x19, 0x1E, 0x17, 0x10,
		0x05, 0x02, 0x0B, 0x0C, 0x21, 0x26, 0x2F, 0x28, 0x3D, 0x3A,
		0x33, 0x34, 0x4E, 0x49, 0x40, 0x47, 0x52, 0x55, 0x5C, 0x5B,
		0x76, 0x71, 0x78, 0x7F, 0x6A, 0x6D, 0x64, 0x63, 0x3E, 0x39,
		0x30, 0x37, 0x22, 0x25, 0x2C, 0x2B, 0x06, 0x01, 0x08, 0x0F,
		0x1A, 0x1D, 0x14, 0x13, 0xAE, 0xA9, 0xA0, 0xA7, 0xB2, 0xB5,
		0xBC, 0xBB, 0x96, 0x91, 0x98, 0x9F, 0x8A, 0x8D, 0x84, 0x83,
		0xDE, 0xD9, 0xD0, 0xD7, 0xC2, 0xC5, 0xCC, 0xCB, 0xE6, 0xE1,
		0xE8, 0xEF, 0xFA, 0xFD, 0xF4, 0xF3
	};

	return pec_table[pec ^ data];
}

static void buf_log_print(char *name, u8 *buf, int len)
{
	int i;
	if (!uart_hid_debug_log_flag())
		return;
	uart_hid_dbg("=======%s start=======\n", name);
	for (i = 0; i < len; i++)
		uart_hid_dbg("0x%x,", buf[i]);

	uart_hid_dbg("=======%s end=======\n", name);
}

static int uart_hid_cmd_to_buf(uart_hid_cmd_data *cmd_data)
{
	int curr = 0;
	int pec = pce_init_value;
	int i;

	uart_hid_cmd_send_buf[curr] = (uint8_t)cmd_data->cmd_head;
	pec = calc_bus_pec(pec, uart_hid_cmd_send_buf[curr]);
	curr++;

	/* write the high 8bit to buf */
	uart_hid_cmd_send_buf[curr] = (uint8_t)(cmd_data->cmd_head >> 8);
	pec = calc_bus_pec(pec, uart_hid_cmd_send_buf[curr]);
	curr++;

	/* write the high 8bit to buf */
	uart_hid_cmd_send_buf[curr] = (uint8_t)(cmd_data->cmd_type);
	pec = calc_bus_pec(pec, uart_hid_cmd_send_buf[curr]);
	curr++;

	uart_hid_cmd_send_buf[curr] = (uint8_t)(cmd_data->cmd_type >> 8);
	pec = calc_bus_pec(pec, uart_hid_cmd_send_buf[curr]);
	curr++;

	uart_hid_cmd_send_buf[curr] = (uint8_t)(cmd_data->cmd_len + 1);
	pec = calc_bus_pec(pec, uart_hid_cmd_send_buf[curr]);
	curr++;

	for (i = 0; i < cmd_data->cmd_len; i++) {
		uart_hid_cmd_send_buf[curr] = cmd_data->cmd_data[i];
		pec = calc_bus_pec(pec, uart_hid_cmd_send_buf[curr]);
		curr++;
	}

	uart_hid_cmd_send_buf[curr] = pec;
	curr++;
	buf_log_print("send_buf", uart_hid_cmd_send_buf, curr);
	return curr;
}

static int uart_hid_buf_to_cmd(uart_hid_cmd_data *response, int len)
{
	int curr = 0;
	int pec = pce_init_value;
	int i;

	if (len > UART_RES_BUF_MAX || len < UART_RES_BUF_MIN) {
		uart_hid_err("buf len = %d err\n", len);
		return -EINVAL;
	}
	/* write buf to the head high 8bit */
	response->cmd_head =
		(((uint16_t)uart_hid_cmd_recv_buf[curr + 1]) << 8) +
		uart_hid_cmd_recv_buf[curr];
	if (response->cmd_head != UART_HID_CMD_HAED) {
		uart_hid_err("cmd_head err = 0x%x\n", response->cmd_head);
		return -EIO;
	}
	pec = calc_bus_pec(pec, uart_hid_cmd_recv_buf[curr]);
	curr++;
	pec = calc_bus_pec(pec, uart_hid_cmd_recv_buf[curr]);
	curr++;

	/* write buf to the head high 8bit */
	response->cmd_type =
		(((uint16_t)uart_hid_cmd_recv_buf[curr + 1]) << 8) +
		uart_hid_cmd_recv_buf[curr];

	pec = calc_bus_pec(pec, uart_hid_cmd_recv_buf[curr]);
	curr++;
	pec = calc_bus_pec(pec, uart_hid_cmd_recv_buf[curr]);
	curr++;

	response->cmd_len = uart_hid_cmd_recv_buf[curr] - 2;
	pec = calc_bus_pec(pec, uart_hid_cmd_recv_buf[curr]);
	curr++;

	if (response->cmd_len > UART_CMD_BUF_LEN || response->cmd_len < 0) {
		uart_hid_err("len = 0x%x err\n", response->cmd_len);
		return -EIO;
	}

	for (i = 0; i < response->cmd_len; i++) {
		response->cmd_data[i] = uart_hid_cmd_recv_buf[curr];
		pec = calc_bus_pec(pec, uart_hid_cmd_recv_buf[curr]);
		curr++;
	}

	response->cmd_err = uart_hid_cmd_recv_buf[curr];
	pec = calc_bus_pec(pec, uart_hid_cmd_recv_buf[curr]);
	curr++;

	if (uart_hid_cmd_recv_buf[curr] != pec) {
		uart_hid_err("uart_hid_buf_to_cmd =0x%x pec= 0x%x err\n",
			     uart_hid_cmd_recv_buf[curr], pec);
		return -EIO;
	}

	return 0;
}

static int uart_hid_cmd_send(uart_hid_cmd_data *cmd_data,
		uart_hid_cmd_data *response, size_t response_len)
{
	size_t len;
	int ret;

	mutex_lock(&uart_hid_cmd_lock);
	(void)memset_s(uart_hid_cmd_send_buf, sizeof(uart_hid_cmd_send_buf), 0,
		       sizeof(uart_hid_cmd_send_buf));
	(void)memset_s(uart_hid_cmd_recv_buf, sizeof(uart_hid_cmd_send_buf), 0,
		       sizeof(uart_hid_cmd_recv_buf));

	len = uart_hid_cmd_to_buf(cmd_data);
	ret = uart_hid_uart_write(uart_hid_cmd_send_buf, len);
	if (ret < 0) {
		uart_hid_err("uart_hid_uart_write error ret = %d\n", ret);
		goto err_cmd_send;
	}

	ret = uart_hid_uart_read(uart_hid_cmd_recv_buf, UART_HID_CMD_BUF_LEN,
			response_len, &len, UART_RECV_TIMEOUT);
	if (ret < 0) {
		uart_hid_err("uart_hid_uart_read error ret = %d\n", ret);
		goto err_cmd_send;
	}
	buf_log_print("recv_buf", uart_hid_cmd_recv_buf, len);
	ret = uart_hid_buf_to_cmd(response, len);
	if (ret < 0) {
		uart_hid_err("uart_hid_buf_to_cmd error ret = %d\n", ret);
		goto err_cmd_send;
	}
	mutex_unlock(&uart_hid_cmd_lock);
	return 0;

err_cmd_send:
	mutex_unlock(&uart_hid_cmd_lock);
	return ret;
}

static int uart_hid_cmd_send_retry(uart_hid_cmd_data *send,
		uart_hid_cmd_data *response)
{
	int ret = uart_hid_cmd_send(send, response, send->response_len);
	if (ret < 0) {
		uart_hid_uart_clear_state();
		uart_hid_dbg("uart_hid_cmd_send error retry!");
		ret = uart_hid_cmd_send(send, response, send->response_len);
		if (ret < 0)
			uart_hid_err("uart_hid_cmd_send retry err, ret = %d\n", ret);
	}
	return ret;
}

int uart_hid_dev_check(void)
{
	int ret;
	uart_hid_cmd_data response;
	uart_hid_cmd_data dev_check = uart_hid_cmd_list[UART_DEV_CHECK];
	uart_hid_cmd_data result = uart_hid_cmd_list[UART_DEV_CHECK_ACK];

	ret = uart_hid_cmd_send_retry(&dev_check, &response);
	if (ret < 0)
		return ret;

	if (response.cmd_len != result.cmd_len ||
	    response.cmd_data[0] != result.cmd_data[0] ||
	    response.cmd_err != result.cmd_err ||
	    response.cmd_type != result.cmd_type) {
		uart_hid_err("value err = 0x%x, cmd_type = 0x%x, len = 0x%x\n",
			     response.cmd_err, response.cmd_type,
			     response.cmd_len);
		return -EIO;
	}
	return 0;
}

int uart_hid_cmd_send_report(u8 *data, size_t size)
{
	int ret;
	uart_hid_cmd_data response;
	uart_hid_cmd_data cmd_send = uart_hid_cmd_list[UART_DEV_SEND];
	uart_hid_cmd_data result = uart_hid_cmd_list[UART_DEV_SEND_ACK];

	if (!data || size != UART_SEND_DATA_LEN)
		return -EINVAL;

	ret = memcpy_s(cmd_send.cmd_data, UART_CMD_BUF_LEN, data, size);
	if (ret < 0) {
		uart_hid_err("memcpy_s\n");
		return ret;
	}
	cmd_send.cmd_len = size;

	ret = uart_hid_cmd_send_retry(&cmd_send, &response);
	if (ret < 0)
		return ret;

	if (response.cmd_type != result.cmd_type ||
	    response.cmd_err != result.cmd_err) {
		uart_hid_err("value len = %d, err =%d\n", response.cmd_len,
			response.cmd_err);
		return -EIO;
	}

	return 0;
}

int uart_hid_cmd_recv_report(u8 *data, size_t buf_size, size_t *ret_size)
{
	int ret;
	uart_hid_cmd_data response;
	uart_hid_cmd_data cmd_recv = uart_hid_cmd_list[UART_DEV_RECV];
	uart_hid_cmd_data result = uart_hid_cmd_list[UART_DEV_RECV_ACK];

	if (!data || !ret_size || buf_size < UART_RECV_DATA_LEN)
		return -EINVAL;

	ret = uart_hid_cmd_send_retry(&cmd_recv, &response);
	if (ret < 0)
		return ret;

	if (response.cmd_len != result.cmd_len) {
		uart_hid_err("len error = %d\n", response.cmd_len);
		return -EIO;
	}

	ret = memcpy_s(data, buf_size, response.cmd_data, response.cmd_len);
	if (ret < 0) {
		uart_hid_err("memcpy err\n");
		return ret;
	}

	*ret_size = response.cmd_len;
	return 0;
}

int uart_hid_cmd_init(const char *uart_port, long baud)
{
	/* the pec seed is 0x38 */
	pce_init_value = calc_bus_pec(0, 0x38 << 1);
	mutex_init(&uart_hid_cmd_lock);
	return uart_hid_uart_init(uart_port, baud);
}

int uart_hid_cmd_deinit(void)
{
	return uart_hid_uart_deinit();
}

void uart_hid_cmd_clear(void)
{
	uart_hid_uart_clear_state();
}