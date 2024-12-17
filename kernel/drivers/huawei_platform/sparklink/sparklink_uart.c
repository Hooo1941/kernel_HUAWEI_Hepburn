/*
 * Huawei spark Driver
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
#include <securec.h>
#include <asm/ioctls.h>
#include <linux/module.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/types.h>
#include <linux/mutex.h>
#include <linux/vmalloc.h>
#include <linux/delay.h>
#include <linux/jiffies.h>
#include <linux/completion.h>
#include "sparklink.h"

#define SPARKLINK_UART_STATE_EMPTY 0
#define SPARKLINK_UART_STATE_FULL 1

#define SPARKLINK_UART_OPEN 0
#define SPARKLINK_UART_CLOSE 1

#define SPARKLINK_CLEAR_RECV 0
#define SPARKLINK_SET_RECV 1

#define SPARKLINK_CLEAR_SEND 1
#define SPARKLINK_SET_SEND 1

#define SPARKLINK_CTR_NO_INIT 0
#define SPARKLINK_CTR_INIT 1
/* keyboard use 27 */
#define SPARKLINK_UART 28
#define PUBLIC_BUF_MAX (8 * 1024)
#define SPARKLINK_UART_WRITE_TIMEOUT 1000

#define SPARKLINK_UART_SEND_MAX 4000

static int g_sparklink_log_mask = SPARKLINK_LOG_INFO;

int sparklink_log_level(void)
{
	return g_sparklink_log_mask;
}

struct _sparklink_uart_recv {
	u8 buf[SPARKLINK_UART_BUFF_LEN];
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

static struct _sparklink_uart_recv g_uart_controll = {
	.buf = {0},
	.buf_state = SPARKLINK_UART_STATE_EMPTY,
	.recv_state = SPARKLINK_CLEAR_RECV,
	.send_state = SPARKLINK_CLEAR_SEND,
	.controll_init = SPARKLINK_CTR_NO_INIT,
	.open_flag = SPARKLINK_UART_CLOSE,
};

static struct tty_struct *g_sparklink_tty = NULL;
static spinlock_t *g_uart_lock = &g_uart_controll.uart_lock;
static struct completion *g_recv_complete = &g_uart_controll.recv_complete;
static struct completion *g_send_complete = &g_uart_controll.send_complete;

static int sparklink_tty_open(struct tty_struct *tty)
{
	if (!tty)
		return -EINVAL;

	/* don't do an wakeup for now */
	clear_bit(TTY_DO_WRITE_WAKEUP, &tty->flags);

	/* set mem already allocated */
	tty->receive_room = PUBLIC_BUF_MAX;
	/* Flush any pending characters in the driver and discipline. */
	tty_ldisc_flush(tty);
	tty_driver_flush_buffer(tty);
	sparklink_infomsg("tty: open\n");
	return 0;
}

static void sparklink_tty_close(struct tty_struct *tty)
{
	sparklink_infomsg("tty: close\n");
	return;
}

static void sparklink_tty_receive(struct tty_struct *tty,
	const unsigned char *cp, char *fp, int count)
{
	int ret;
	
	if (!cp || count > SPARKLINK_UART_BUFF_LEN)
		return;

	spin_lock(g_uart_lock);

	if (g_uart_controll.recv_state != SPARKLINK_SET_RECV) {
		spin_unlock(g_uart_lock);
		return;
	}

	mb();
	/* optimize the read buff size */
	if (g_uart_controll.len + count > SPARKLINK_UART_BUFF_LEN)
		count = SPARKLINK_UART_BUFF_LEN - g_uart_controll.len;

	ret = memcpy_s(g_uart_controll.buf + g_uart_controll.len,
		SPARKLINK_UART_BUFF_LEN - g_uart_controll.len, cp, count);
	if (ret < 0) {
		sparklink_errmsg("memcpy_s err\n");
		spin_unlock(g_uart_lock);
		return;
	}
	g_uart_controll.len += count;
	if (g_uart_controll.len >= g_uart_controll.read_buf_len) {
		g_uart_controll.buf_state = SPARKLINK_UART_STATE_FULL;
		g_uart_controll.recv_state = SPARKLINK_CLEAR_RECV;
		complete(g_recv_complete);
	}
	spin_unlock(g_uart_lock);

	return;
}

static void sparklink_tty_wakeup(struct tty_struct *tty)
{
	if (!tty)
		return;
	
	/* clear wakeup flage for now */
	clear_bit(TTY_DO_WRITE_WAKEUP, &tty->flags);

	spin_lock(g_uart_lock);
	if (g_uart_controll.send_state != SPARKLINK_SET_SEND) {
		spin_unlock(g_uart_lock);
		return;
	}
	g_uart_controll.send_state = SPARKLINK_CLEAR_SEND;
	spin_unlock(g_uart_lock);

	complete(g_send_complete);
	return;
}

static void sparklink_tty_flush_buffer(struct tty_struct *tty)
{
	sparklink_infomsg("tty: flush\n");
	return;
}

static struct tty_ldisc_ops sparklink_ldisc_ops = {
	.magic = TTY_LDISC_MAGIC,
	.name = "sparklink_tty",
	.open = sparklink_tty_open,
	.close = sparklink_tty_close,
	.receive_buf = sparklink_tty_receive,
	.write_wakeup = sparklink_tty_wakeup,
	.flush_buffer = sparklink_tty_flush_buffer,
	.owner = THIS_MODULE
};

static void sparklink_ktty_set_termios(struct tty_struct *tty, long baud_rate)
{
	struct ktermios ktermios;

	if (!tty)
		return;

	ktermios = tty->termios;
	/* close soft flowctrl */
	ktermios.c_iflag = 0;
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
	sparklink_errmsg("set baud_rate=%d, except=%d\n",
		(int)tty_termios_baud_rate(&tty->termios), (int)baud_rate);
}

static int sparklink_uart_open(const char *uart_port, long baud, dev_t *dev_no)
{
	int ret;

	if (!uart_port || !dev_no)
		return -EINVAL;

	/* open tty */
	g_sparklink_tty = tty_kopen(*dev_no);
	if (IS_ERR_OR_NULL(g_sparklink_tty)) {
		sparklink_errmsg("open tty %s failed\n", uart_port);
		goto SPARKLINK_KOPEN_ERROR;
	}

	if (g_sparklink_tty->ops->open) {
		ret = g_sparklink_tty->ops->open(g_sparklink_tty, NULL);
	} else {
		sparklink_errmsg("tty->ops->open is NULL\n");
		ret = -ENODEV;
	}

	if (ret) {
		tty_unlock(g_sparklink_tty);
		goto SPARKLINK_OPEN_ERROR;
	}

	sparklink_ktty_set_termios(g_sparklink_tty, baud);
	tty_unlock(g_sparklink_tty);

	ret = tty_set_ldisc(g_sparklink_tty, SPARKLINK_UART);
	if (ret) {
		sparklink_errmsg("tty_set_ldisc error\n");
		goto SPARKLINK_LDISC_ERROR;
	}
	g_uart_controll.open_flag = SPARKLINK_UART_OPEN;

	return 0;
SPARKLINK_LDISC_ERROR:
	g_sparklink_tty->ops->close(g_sparklink_tty, NULL);
SPARKLINK_OPEN_ERROR:
	tty_kclose(g_sparklink_tty);
SPARKLINK_KOPEN_ERROR:
	tty_unregister_ldisc(SPARKLINK_UART);
	g_sparklink_tty = NULL;
	return -1;
}

static int sparklink_uart_init(const char *uart_port, long baud)
{
	int ret;
	dev_t dev_no;

	if (!uart_port)
		return -EINVAL;

	if (g_uart_controll.open_flag == SPARKLINK_UART_OPEN) {
		sparklink_errmsg("init have done\n");
		return 0;
	}

	ret = tty_register_ldisc(SPARKLINK_UART, &sparklink_ldisc_ops);
	if (ret != 0)
		sparklink_errmsg("tty_register_ldisc error ret=%d\n", ret);

	ret = tty_dev_name_to_number(uart_port, &dev_no);
	if (ret != 0) {
		sparklink_errmsg("dev to number tty:%s ret=%d\n", uart_port, ret);
		tty_unregister_ldisc(SPARKLINK_UART);
		return -1;
	}

	ret = sparklink_uart_open(uart_port, baud, &dev_no);
	if (ret < 0)
		return -1;

	sparklink_infomsg("%s end\n", uart_port);

	return 0;
}

static void sparklink_uart_clear_state(void)
{
	if (!g_sparklink_tty)
		return;

	tty_ldisc_flush(g_sparklink_tty);
	tty_driver_flush_buffer(g_sparklink_tty);

	spin_lock(g_uart_lock);
	g_uart_controll.buf_state = SPARKLINK_UART_STATE_EMPTY;
	g_uart_controll.recv_state = SPARKLINK_CLEAR_RECV;
	g_uart_controll.len = 0;
	spin_unlock(g_uart_lock);
	return;
}

static void sparklink_reconfig_baudrate(long baud)
{
	if (!g_sparklink_tty)
		return;

	sparklink_ktty_set_termios(g_sparklink_tty, baud);
	return;
}

static int sparklink_uart_read(u8 *buf, size_t *len, int time_out, int want_len)
{
	int ret;

	if (!buf || !len)
		return -EINVAL;

	if (!g_sparklink_tty) {
		*len = 0;
		return -1;
	}

	spin_lock(g_uart_lock);
	g_uart_controll.buf_state = SPARKLINK_UART_STATE_EMPTY;
	g_uart_controll.recv_state = SPARKLINK_SET_RECV;
	g_uart_controll.len = 0;
	g_uart_controll.read_buf_len = want_len;
	spin_unlock(g_uart_lock);

	if (!wait_for_completion_timeout(g_recv_complete, msecs_to_jiffies(time_out))) {
		*len = 0;
		spin_lock(g_uart_lock);
		g_uart_controll.recv_state = SPARKLINK_CLEAR_RECV;
		spin_unlock(g_uart_lock);
		return -EIO;
	}

	spin_lock(g_uart_lock);
	*len = g_uart_controll.len;
	ret = memcpy_s(buf, SPARKLINK_UART_BUFF_LEN, g_uart_controll.buf, g_uart_controll.len);
	if (ret < 0) {
		sparklink_errmsg("memcpy_s err\n");
		spin_unlock(g_uart_lock);
		return -1;
	}
	g_uart_controll.buf_state = SPARKLINK_UART_STATE_EMPTY;
	g_uart_controll.len = 0;
	spin_unlock(g_uart_lock);

	return 0;
}

static int sparklink_uart_write(u8 *buf, size_t len)
{
	int remain = len;
	int size_send, addr;

	if (!buf || !g_sparklink_tty)
		return -EINVAL;

	addr = 0;
	while (remain > 0) {
		size_send = SPARKLINK_UART_SEND_MAX > remain ? remain : SPARKLINK_UART_SEND_MAX;
		set_bit(TTY_DO_WRITE_WAKEUP, &g_sparklink_tty->flags);

		spin_lock(g_uart_lock);
		g_uart_controll.send_state = SPARKLINK_SET_SEND;
		spin_unlock(g_uart_lock);

		g_sparklink_tty->ops->write(g_sparklink_tty, buf + addr, size_send);
		if (!wait_for_completion_timeout(g_send_complete,
				msecs_to_jiffies(SPARKLINK_UART_WRITE_TIMEOUT))) {
			spin_lock(g_uart_lock);
			g_uart_controll.send_state = SPARKLINK_CLEAR_SEND;
			spin_unlock(g_uart_lock);
			return -EIO;
		}
		remain -= size_send;
		addr += size_send;
	}
	return 0;
}

static int sparklink_uart_deinit(void)
{
	if (g_uart_controll.open_flag != SPARKLINK_UART_OPEN)
		return 0;

	if (!g_sparklink_tty)
		return -1;

	tty_ldisc_release(g_sparklink_tty);
	g_sparklink_tty->ops->close(g_sparklink_tty, NULL);
	tty_kclose(g_sparklink_tty);
	tty_unregister_ldisc(SPARKLINK_UART);
	g_sparklink_tty = NULL;

	spin_lock(g_uart_lock);
	g_uart_controll.send_state = SPARKLINK_CLEAR_SEND;
	g_uart_controll.recv_state = SPARKLINK_CLEAR_RECV;
	g_uart_controll.buf_state = SPARKLINK_UART_STATE_EMPTY;
	g_uart_controll.open_flag = SPARKLINK_UART_CLOSE;
	spin_unlock(g_uart_lock);

	return 0;
}

static struct sparklink_uart_ops uart_ops = {
	.uart_init = sparklink_uart_init,
	.uart_clear = sparklink_uart_clear_state,
	.reconfig_baudrate = sparklink_reconfig_baudrate,
	.uart_read = sparklink_uart_read,
	.uart_write = sparklink_uart_write,
	.uart_deinit = sparklink_uart_deinit,
};

static int __init sparklink_uart_module_init(void)
{
	sparklink_infomsg("sparklink_uart_module_init enter\n");
	if (g_uart_controll.controll_init == SPARKLINK_CTR_NO_INIT) {
		spin_lock_init(g_uart_lock);
		init_completion(g_recv_complete);
		init_completion(g_send_complete);
		g_uart_controll.controll_init = SPARKLINK_CTR_INIT;
	}
	return sparklink_uart_ops_register(&uart_ops);
}

static void __exit sparklink_uart_module_exit(void)
{
	sparklink_infomsg("sparklink_uart_module_exit\n");
	return;
}

module_init(sparklink_uart_module_init);
module_exit(sparklink_uart_module_exit);
MODULE_AUTHOR("Huawei Device Company");
MODULE_DESCRIPTION("Huawei sparklink uart Driver");
MODULE_LICENSE("GPL");
