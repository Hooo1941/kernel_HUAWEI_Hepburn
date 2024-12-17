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
#include <securec.h>
#include <linux/module.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/types.h>
#include <linux/mutex.h>
#include <linux/jiffies.h>
#include <linux/completion.h>
#include <asm/ioctls.h>
#include <linux/vmalloc.h>
#include <linux/delay.h>

#include "uart_hid_log.h"
#include "uart_hid_uart_protocol.h"

#define UART_HID_UART_BUFF_LEN 100

#define UART_HID_UART_STATE_EMPTY 0
#define UART_HID_UART_STATE_FULL 1

#define UART_HID_UART_OPEN 0
#define UART_HID_UART_CLOSE 1

#define UART_HID_CLEAR_RECV 0
#define UART_HID_SET_RECV 1

#define UART_HID_CLEAR_SEND 1
#define UART_HID_SET_SEND 1

#define UART_HID_CTR_NO_INIT 0
#define UART_HID_CTR_INIT 1

#define UART_HID_UART 26
#define PUBLIC_BUF_MAX (8 * 1024)
#define UART_HID_UART_WRITE_TIMEOUT 1000

struct uart_hid_uart_ops {
	int (*uart_init)(char *uart_port, long baud);
	void (*uart_clear)(void);
	void (*reconfig_baudrate)(long baud);
	int (*uart_read)(u8 *buf, size_t *len, int time_out);
	int (*uart_write)(u8 *buf, size_t len);
	int (*uart_deinit)(void);
};

struct _uart_hid_uart_recv {
	u8 buf[UART_HID_UART_BUFF_LEN];
	int len;
	int buf_state;
	int recv_state;
	int send_state;
	int controll_init;
	int open_flag;
	int read_buf_len;
	spinlock_t uart_lock;
	struct completion recv_complete;
	struct completion send_complete;
};

static struct _uart_hid_uart_recv uart_controll = {
	.buf = { 0 },
	.buf_state = UART_HID_UART_STATE_EMPTY,
	.recv_state = UART_HID_CLEAR_RECV,
	.send_state = UART_HID_CLEAR_SEND,
	.controll_init = UART_HID_CTR_NO_INIT,
	.open_flag = UART_HID_UART_CLOSE,
};

static spinlock_t *uart_lock = &uart_controll.uart_lock;
static struct completion *recv_complete = &uart_controll.recv_complete;
static struct completion *send_complete = &uart_controll.send_complete;
static struct tty_struct *uart_hid_tty = NULL;

static int uart_hid_tty_open(struct tty_struct *tty)
{
	/* don't do an wakeup for now */
	clear_bit(TTY_DO_WRITE_WAKEUP, &tty->flags);

	/* set mem already allocated */
	tty->receive_room = PUBLIC_BUF_MAX;
	/* Flush any pending characters in the driver and discipline. */
	tty_ldisc_flush(tty);
	tty_driver_flush_buffer(tty);
	uart_hid_dbg("tty: open\n");
	return 0;
}
static void uart_hid_tty_close(struct tty_struct *tty)
{
	uart_hid_dbg("tty: close\n");
	return;
}

static int uart_hid_tty_receive2(struct tty_struct *tty,
		const unsigned char *cp, char *fp, int count)
{
	int ret;
	if (count > UART_HID_UART_BUFF_LEN) {
		uart_hid_err("len err");
		return count;
	}

	spin_lock(uart_lock);

	if (uart_controll.recv_state != UART_HID_SET_RECV) {
		spin_unlock(uart_lock);
		uart_hid_err("recv state error\n");
		return count;
	}

	mb();
	ret = memcpy_s(uart_controll.buf + uart_controll.len,
		UART_HID_UART_BUFF_LEN - uart_controll.len, cp, count);
	if (ret < 0) {
		spin_unlock(uart_lock);
		uart_hid_err("memcpy_s error\n");
		return count;
	}
	uart_controll.len += count;
	if (uart_controll.len >= uart_controll.read_buf_len) {
		uart_controll.buf_state = UART_HID_UART_STATE_FULL;
		uart_controll.recv_state = UART_HID_CLEAR_RECV;
		complete(recv_complete);
	}
	spin_unlock(uart_lock);

	return count;
}
static void uart_hid_tty_wakeup(struct tty_struct *tty)
{
	/* clear wakeup flage for now */
	clear_bit(TTY_DO_WRITE_WAKEUP, &tty->flags);
	spin_lock(uart_lock);
	if (uart_controll.send_state != UART_HID_SET_SEND) {
		spin_unlock(uart_lock);
		return;
	}
	uart_controll.send_state = UART_HID_CLEAR_SEND;
	spin_unlock(uart_lock);

	complete(send_complete);
	return;
}

static void uart_hid_tty_flush_buffer(struct tty_struct *tty)
{
	return;
}

static struct tty_ldisc_ops uart_hid_ldisc_ops = {
	.magic = TTY_LDISC_MAGIC,
	.name = "uart_hid_tty",
	.open = uart_hid_tty_open,
	.close = uart_hid_tty_close,
	.receive_buf2 = uart_hid_tty_receive2,
	.write_wakeup = uart_hid_tty_wakeup,
	.flush_buffer = uart_hid_tty_flush_buffer,
	.owner = THIS_MODULE
};

static void uart_hid_ktty_set_termios(struct tty_struct *tty, long baud_rate)
{
	struct ktermios ktermios;

	ktermios = tty->termios;
	/* close soft flowctrl */
	ktermios.c_iflag &= 0;
	/* set uart cts/rts flowctrl */
	ktermios.c_cflag &= ~CRTSCTS;
	/* set csize */
	ktermios.c_cflag &= ~(CSIZE);
	ktermios.c_cflag |= CS8;
	/* set uart baudrate */
	ktermios.c_cflag &= ~CBAUD;
	ktermios.c_cflag |= BOTHER;
	tty_termios_encode_baud_rate(&ktermios, baud_rate, baud_rate);
	tty_set_termios(tty, &ktermios);
	uart_hid_dbg("set baud_rate=%d, except=%d\n",
		(int)tty_termios_baud_rate(&tty->termios), (int)baud_rate);
}

int uart_hid_open_uart_port(dev_t *dev_no, long baud)
{
	int ret = 0;
	/* open tty */
	uart_hid_tty = tty_kopen(*dev_no);
	if (IS_ERR_OR_NULL(uart_hid_tty)) {
		uart_hid_err("open tty failed \n");
		ret = -ENODEV;
		return ret;
	}

	if (uart_hid_tty->ops->open) {
		ret = uart_hid_tty->ops->open(uart_hid_tty, NULL);
	} else {
		uart_hid_err("tty->ops->open is NULL\n");
		ret = -ENODEV;
	}
	clear_bit(TTY_HUPPED, &uart_hid_tty->flags);

	if (ret < 0) {
		tty_unlock(uart_hid_tty);
		uart_hid_err("tty->ops->open err");
		goto UART_HID_OPEN_ERROR;
	}

	uart_hid_ktty_set_termios(uart_hid_tty, baud);

	tty_unlock(uart_hid_tty);

	ret = tty_set_ldisc(uart_hid_tty, UART_HID_UART);
	if (ret < 0) {
		uart_hid_err("tty_set_ldisc error\n");
		goto UART_HID_LDISC_ERROR;
	}
	uart_controll.open_flag = UART_HID_UART_OPEN;
	uart_hid_dbg("sucess\n");
	return 0;

UART_HID_LDISC_ERROR:
	uart_hid_tty->ops->close(uart_hid_tty, NULL);
UART_HID_OPEN_ERROR:
	tty_kclose(uart_hid_tty);
	uart_hid_err("error ret = %d\n", ret);
	return ret;
}

int uart_hid_uart_init(const char *uart_port, long baud)
{
	int ret;
	dev_t dev_no;

	if (uart_controll.controll_init == UART_HID_CTR_NO_INIT) {
		spin_lock_init(uart_lock);
		init_completion(recv_complete);
		init_completion(send_complete);
		uart_controll.controll_init = UART_HID_CTR_INIT;
	}

	if (uart_controll.open_flag == UART_HID_UART_OPEN) {
		uart_hid_dbg("init have done\n");
		return 0;
	}

	ret = tty_register_ldisc(UART_HID_UART, &uart_hid_ldisc_ops);
	if (ret < 0) {
		uart_hid_err("can't tty_register_ldisc error ret=%d\n", ret);
		return ret;
	}

	ret = tty_dev_name_to_number(uart_port, &dev_no);
	if (ret < 0)
		goto UART_HID_KOPEN_ERROR;

	ret = uart_hid_open_uart_port(&dev_no, baud);
	if (ret < 0)
		goto UART_HID_KOPEN_ERROR;

	return ret;

UART_HID_KOPEN_ERROR:
	tty_unregister_ldisc(UART_HID_UART);
	uart_hid_tty = NULL;
	uart_hid_err("%s open error\n", uart_port);
	return ret;
}

void uart_hid_uart_clear_state(void)
{
	if (!uart_hid_tty)
		return;

	tty_ldisc_flush(uart_hid_tty);
	tty_driver_flush_buffer(uart_hid_tty);

	spin_lock(uart_lock);
	uart_controll.buf_state = UART_HID_UART_STATE_EMPTY;
	uart_controll.recv_state = UART_HID_CLEAR_RECV;
	uart_controll.len = 0;
	spin_unlock(uart_lock);
	return;
}

void uart_hid_reconfig_baudrate(long baud)
{
	if (!uart_hid_tty)
		return;

	uart_hid_ktty_set_termios(uart_hid_tty, baud);
	return;
}

int uart_hid_uart_read(u8 *buf, size_t buf_len, size_t want_len, size_t *len,
		       int time_out)
{
	int ret;
	if (!buf || !len)
		return -EINVAL;

	if (!uart_hid_tty) {
		*len = 0;
		return -EINVAL;
	}

	spin_lock(uart_lock);
	uart_controll.buf_state = UART_HID_UART_STATE_EMPTY;
	uart_controll.recv_state = UART_HID_SET_RECV;
	uart_controll.len = 0;
	uart_controll.read_buf_len = want_len;
	spin_unlock(uart_lock);

	if (!wait_for_completion_timeout(recv_complete,
		msecs_to_jiffies(time_out))) {
		*len = 0;
		spin_lock(uart_lock);
		uart_controll.buf_state = UART_HID_UART_STATE_EMPTY;
		uart_controll.recv_state = UART_HID_CLEAR_RECV;
		spin_unlock(uart_lock);
		return -EIO;
	}

	spin_lock(uart_lock);
	*len = uart_controll.len;
	ret = memcpy_s(buf, buf_len, uart_controll.buf, uart_controll.len);
	if (ret < 0)
		uart_hid_err("tty_set_ldisc error\n");
	uart_controll.buf_state = UART_HID_UART_STATE_EMPTY;
	uart_controll.len = 0;
	spin_unlock(uart_lock);

	return ret;
}

int uart_hid_uart_write(u8 *buf, size_t len)
{
	if (!buf)
		return -EINVAL;

	if (!uart_hid_tty)
		return -EINVAL;

	set_bit(TTY_DO_WRITE_WAKEUP, &uart_hid_tty->flags);

	spin_lock(uart_lock);
	uart_controll.send_state = UART_HID_SET_SEND;
	spin_unlock(uart_lock);

	uart_hid_tty->ops->write(uart_hid_tty, buf, len);
	if (!wait_for_completion_timeout(
		send_complete,
		msecs_to_jiffies(UART_HID_UART_WRITE_TIMEOUT))) {
		spin_lock(uart_lock);
		uart_controll.send_state = UART_HID_CLEAR_SEND;
		spin_unlock(uart_lock);
		return -EIO;
	}
	return 0;
}

int uart_hid_uart_deinit(void)
{
	if (uart_controll.open_flag != UART_HID_UART_OPEN)
		return 0;

	if (!uart_hid_tty)
		return -EINVAL;

	tty_ldisc_release(uart_hid_tty);
	uart_hid_tty->ops->close(uart_hid_tty, NULL);
	tty_kclose(uart_hid_tty);
	tty_unregister_ldisc(UART_HID_UART);
	uart_hid_tty = NULL;

	spin_lock(uart_lock);
	uart_controll.send_state = UART_HID_CLEAR_SEND;
	uart_controll.recv_state = UART_HID_CLEAR_RECV;
	uart_controll.buf_state = UART_HID_UART_STATE_EMPTY;
	uart_controll.open_flag = UART_HID_UART_CLOSE;
	spin_unlock(uart_lock);

	return 0;
}
