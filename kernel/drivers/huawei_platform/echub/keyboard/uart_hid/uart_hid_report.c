/*
 * Huawei uart hid Driver for hid control
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
#include <linux/err.h>
#include <linux/string.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/wait.h>
#include <linux/hid.h>
#include <linux/mutex.h>
#include <linux/delay.h>
#include <linux/irq.h>
#include <securec.h>
#include "uart_hid_log.h"
#include "uart_hid_desc.h"
#include "uart_hid_report.h"

#define UART_HID_CMD_LEN 6

/* the args head is
   ReportID(__u8)
   optional ReportID byte(__u8)
   data register(__u16)
   size of the report(__u16)
*/
#define UART_HID_ARGS_HEAD_LEN	\
	(sizeof(__u8) + sizeof(__u8) + sizeof(__u16) + sizeof(__u16))

void uart_hid_input_report(struct uart_hid *ihid)
{
	u32 ret_size;
	u8 *buf = ihid->input_buf;

	ret_size = buf[0] | (buf[1] << 8);

	if (!ret_size) {
		/* host or device initiated RESET completed */
		if (test_and_clear_bit(UART_HID_RESET_PENDING, &ihid->flags))
			return;
	}

	if (test_bit(UART_HID_STARTED, &ihid->flags))
		hid_input_report(ihid->hid, HID_INPUT_REPORT, buf + 2,
			ret_size - 2, 1);

	return;
}

void uart_hid_free_buffers(struct uart_hid *ihid)
{
	if (ihid->buf_size == 0)
		return;
	kfree(ihid->input_buf);
	kfree(ihid->args_buf);
	kfree(ihid->cmd_buf);
	ihid->input_buf = NULL;
	ihid->cmd_buf = NULL;
	ihid->args_buf = NULL;
	ihid->buf_size = 0;
}

int uart_hid_alloc_buffers(struct uart_hid *ihid, size_t report_size)
{
	int args_len = UART_HID_ARGS_HEAD_LEN + report_size;

	ihid->input_buf = kzalloc(report_size, GFP_KERNEL);
	ihid->args_buf = kzalloc(args_len, GFP_KERNEL);
	ihid->cmd_buf = kzalloc(UART_HID_CMD_LEN + args_len, GFP_KERNEL);

	if (!ihid->input_buf || !ihid->args_buf || !ihid->cmd_buf) {
		uart_hid_free_buffers(ihid);
		uart_hid_err("No mem err\n");
		return -ENOMEM;
	}

	ihid->buf_size = report_size;

	return 0;
}

/**
 * uart_hid_send_report: forward an incoming report to the device
 * @client: the uart_client of the device
 * @report_id: the report ID
 * @buf: the actual data to transfer, without the report ID
 * @data_len: size of buf
 */
static int uart_hid_send_report(struct uart_hid *ihid, u8 report_id,
			unsigned char *buf, size_t data_len)
{
	u8 *args = ihid->args_buf;
	u8 *cmd = ihid->cmd_buf;
	int ret;
	u16 output_register = le16_to_cpu(ihid->hdesc.output_register);
	u16 output_length = le16_to_cpu(ihid->hdesc.max_output_length);
	u16 size;
	int args_len;
	int index = 0;

	uart_hid_dbg("enter\n");

	if (data_len > ihid->buf_size)
		return -EINVAL;
	/* size len + report_id len + data_len */
	size = 2  + (report_id ? 1 : 0) + data_len ;
	/* optional third byte + data register len + args len */
	args_len = (report_id >= 0x0F ? 1 : 0) + 2 + size;

	if (output_length == 0)
		return -ENOSYS;

	args[index++] = output_register & 0xFF;
	args[index++] = output_register >> 8;
	args[index++] = size & 0xFF;
	args[index++] = size >> 8;

	if (report_id)
		args[index++] = report_id;

	ret = memcpy_s(&args[index],
		UART_HID_ARGS_HEAD_LEN - index + ihid->buf_size, buf,
		data_len);
	if (ret < 0) {
		uart_hid_err("failed memcpy_s buf to args[index]\n");
		return ret;
	}

	ret = memcpy_s(cmd,
		ihid->buf_size + UART_HID_ARGS_HEAD_LEN +
		UART_HID_CMD_LEN,
		args, args_len);
	if (ret < 0) {
		uart_hid_err("failed args to cmd buf\n");
		return ret;
	}

	ret = uart_hid_send_buf(cmd, args_len);
	if (ret) {
		uart_hid_err("failed to set a report to device\n");
		return ret;
	}

	return data_len;
}

static int uart_hid_get_report_length(struct hid_report *report)
{
	/* the length need to plus 2 */
	return ((report->size - 1) >> 3) + 1 +
	       report->device->report_enum[report->type].numbered + 2;
}

/* Traverse the supplied list of reports and find the longest */
static void uart_hid_find_max_report(struct hid_device *hid, unsigned int type,
		unsigned int *max)
{
	struct hid_report *report;
	unsigned int size;

	list_for_each_entry (report, &hid->report_enum[type].report_list,
			list) {
		size = uart_hid_get_report_length(report);
		if (*max < size)
			*max = size;
	}
}

static int uart_hid_output_report(struct hid_device *hid, __u8 *buf,
		size_t count)
{
	struct uart_hid *ihid = hid->driver_data;
	int report_id = buf[0];
	int ret;

	mutex_lock(&ihid->reset_lock);

	if (report_id) {
		buf++;
		count--;
	}

	ret = uart_hid_send_report(ihid, report_id, buf, count);
	if (report_id && ret >= 0)
		ret++;

	mutex_unlock(&ihid->reset_lock);

	return ret;
}

static int uart_hid_raw_request(struct hid_device *hid, unsigned char reportnum,
		__u8 *buf, size_t len, unsigned char rtype, int reqtype)
{
	uart_hid_dbg("uart_hid_raw_request do not support now (%u)\n", reqtype);
	return 0;
}

static int uart_hid_parse(struct hid_device *hid)
{
	struct uart_hid *ihid = hid->driver_data;
	struct uart_hid_desc *hdesc = &ihid->hdesc;
	unsigned int rsize;
	unsigned int ret_size;
	char *rdesc;
	int ret;

	__u8 *key_board_report = NULL;

	uart_hid_dbg("enter\n");

	rsize = le16_to_cpu(hdesc->report_desc_length);
	if (!rsize || rsize > HID_MAX_DESCRIPTOR_SIZE) {
		uart_hid_err("weird size of report descriptor (%u)\n", rsize);
		return -EINVAL;
	}

	rdesc = kzalloc(rsize, GFP_KERNEL);
	if (!rdesc) {
		uart_hid_err("couldn't allocate rdesc memory\n");
		return -ENOMEM;
	}

	key_board_report = uart_hid_get_keyboard_report_desc(&ret_size);
	ret = memcpy_s(rdesc, rsize, key_board_report, ret_size);
	if (ret < 0) {
		uart_hid_err("couldn't memcpy rdesc\n");
		kfree(rdesc);
		return ret;
	}

	uart_hid_dbg("Report Descriptor: %p, size:%d\n", rsize, ret_size);

	ret = hid_parse_report(hid, rdesc, rsize);

	kfree(rdesc);

	if (ret) {
		uart_hid_err("parsing report descriptor failed\n");
		return ret;
	}

	return 0;
}

static int uart_hid_start(struct hid_device *hid)
{
	struct uart_hid *ihid = hid->driver_data;
	int ret;
	unsigned int size = HID_MIN_BUFFER_SIZE;

	uart_hid_find_max_report(hid, HID_INPUT_REPORT, &size);
	uart_hid_find_max_report(hid, HID_OUTPUT_REPORT, &size);
	uart_hid_find_max_report(hid, HID_FEATURE_REPORT, &size);

	if (size > ihid->buf_size) {
		disable_irq(ihid->irq);
		uart_hid_free_buffers(ihid);

		ret = uart_hid_alloc_buffers(ihid, size);
		enable_irq(ihid->irq);

		if (ret)
			return ret;
	}

	return 0;
}

static void uart_hid_stop(struct hid_device *hid)
{
	struct uart_hid *ihid = hid->driver_data;
	uart_hid_free_buffers(ihid);
	hid->claimed = 0;
}

static int uart_hid_open(struct hid_device *hid)
{
	struct uart_hid *ihid = hid->driver_data;

	set_bit(UART_HID_STARTED, &ihid->flags);
	return 0;
}

static void uart_hid_close(struct hid_device *hid)
{
	struct uart_hid *ihid = hid->driver_data;

	clear_bit(UART_HID_STARTED, &ihid->flags);
}

static struct hid_ll_driver uart_hid_ll_driver = {
	.parse = uart_hid_parse,
	.start = uart_hid_start,
	.stop = uart_hid_stop,
	.open = uart_hid_open,
	.close = uart_hid_close,
	.output_report = uart_hid_output_report,
	.raw_request = uart_hid_raw_request,
};

struct hid_ll_driver *get_uart_hid_ll_driver(void)
{
	return &uart_hid_ll_driver;
}

int uart_hid_fetch_hid_descriptor(struct uart_hid *ihid)
{
	struct uart_hid_desc *hdesc = &ihid->hdesc;
	unsigned int dsize;
	int ret;
	ret = memcpy_s(ihid->hdesc_buffer, sizeof(struct uart_hid_desc),
			uart_hid_get_keyboard_hid_desc(),
			sizeof(struct uart_hid_desc));
	if (ret < 0) {
		uart_hid_err("memcpy_s error\n");
		return ret;
	}

	/* Check bcd_version == 1.0 */
	if (le16_to_cpu(hdesc->bcd_version) != 0x0100) {
		uart_hid_err(
		    "unexpected hid descriptor bcd_version (0x%04hx)\n",
		    le16_to_cpu(hdesc->bcd_version));
		return -ENODEV;
	}

	/* Check  the Descriptor length */
	dsize = le16_to_cpu(hdesc->hid_desc_length);
	if (dsize != sizeof(struct uart_hid_desc)) {
		uart_hid_err("hid descriptor len =(%u) error\n", dsize);
		return -ENODEV;
	}
	return 0;
}
