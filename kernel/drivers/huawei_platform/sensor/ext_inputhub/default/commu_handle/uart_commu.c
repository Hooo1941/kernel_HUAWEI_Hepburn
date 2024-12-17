
/*
 * uart_commu.c
 *
 * source file for communication with external sensorhub
 *
 * Copyright (c) 2022-2022 Huawei Technologies Co., Ltd.
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

#include "uart_commu.h"

#include <linux/of_gpio.h>
#include <linux/preempt.h>
#include "securec.h"
#include "ext_sensorhub_uart.h"
#include "ext_sensorhub_frame.h"

#define COMMU_UART_FIRST_WRITE 128
#define COMMU_UART_SECOND_WRITE_TIMOUE_MS 40

struct uart_second_pkg {
	u8 *buffer;
	int len;
	int write_flag;
	wait_queue_head_t write_wait;
	bool write_done;
	int count;
};

static struct uart_second_pkg g_second_pkg;
static int g_uart_write_mode;
static DEFINE_SPINLOCK(write_lock);

void set_uart_commu_mode(int mode)
{
	g_uart_write_mode = mode;
	ext_sensorhub_set_uart_mode(mode);
}

int get_uart_commu_mode(void)
{
	return g_uart_write_mode;
}

static int uart_commu_init(struct commu_data *commu_data)
{
	int ret;

	if (!commu_data)
		return -EINVAL;

	g_uart_write_mode = 0;
	g_second_pkg.buffer = NULL;
	g_second_pkg.len = 0;
	g_second_pkg.write_done = false;
	g_second_pkg.count = UART_SHORT_PKG_COUNT;
	init_waitqueue_head(&g_second_pkg.write_wait);
	ret = ext_sensorhub_uart_init();
	ext_sensorhub_set_rx_gpio(commu_data->ap_read_ready_gpio);
	ext_sensorhub_set_wakeup_gpio(commu_data->wakeup_ap_gpio);
	commu_data->initialed = true;

	return ret;
}

static void uart_commu_exit(struct commu_data *commu_data)
{
	ext_sensorhub_uart_exit();
}

static void uart_commu_long_last(void)
{
	ext_sensorhub_uart_write(g_second_pkg.buffer, g_second_pkg.len);
	g_second_pkg.buffer = NULL;
	g_second_pkg.len = 0;
	g_second_pkg.write_done = true;
	wake_up_interruptible(&g_second_pkg.write_wait);
}

static bool uart_commu_write_second(void)
{
	ext_sensorhub_uart_write(g_second_pkg.buffer, g_second_pkg.len);
	if (g_second_pkg.count == UART_SHORT_PKG_COUNT) {
		g_second_pkg.buffer = NULL;
		g_second_pkg.len = 0;
		g_second_pkg.write_done = true;
		wake_up_interruptible(&g_second_pkg.write_wait);
		return true;
	} else if (g_second_pkg.count == UART_LONG_PKG_COUNT) {
		/* long pkg write ack */
		g_second_pkg.buffer =
		g_second_pkg.buffer + COMMU_UART_FIRST_WRITE;
		g_second_pkg.len = UART_ACK_PKG_LEN;
		return true;
	}
	return false;
}

/* uart is async, can do send in irq handler */
static irqreturn_t uart_commu_rx_irq(int irq, void *arg)
{
	int second_len;
	unsigned long flags;
	int len;
	u8 *buf = NULL;

	spin_lock_irqsave(&write_lock, flags);
	len = g_second_pkg.len;
	buf = g_second_pkg.buffer;
	g_second_pkg.write_flag++;

	pr_info("%s get write_flag: %d, cnt:%d",
		__func__, g_second_pkg.write_flag, g_second_pkg.count);
	/* 2 for max received rx ready irq */
	if (g_second_pkg.write_flag > 2 && g_second_pkg.buffer &&
	    g_second_pkg.count == UART_LONG_PKG_COUNT) {
		uart_commu_long_last();
		spin_unlock_irqrestore(&write_lock, flags);
		return IRQ_HANDLED;
	}

	if (g_second_pkg.write_flag > 1 && g_second_pkg.buffer) {
		if (uart_commu_write_second()) {
			spin_unlock_irqrestore(&write_lock, flags);
			return IRQ_HANDLED;
		}
	}

	if (g_second_pkg.write_flag > 0 && g_second_pkg.buffer) {
		ext_sensorhub_uart_write(buf, COMMU_UART_FIRST_WRITE);
		if (len <= COMMU_UART_FIRST_WRITE) {
			/* last 8 byte buffer */
			g_second_pkg.count = UART_SHORT_PKG_COUNT;
			g_second_pkg.buffer = buf + COMMU_UART_FIRST_WRITE;
			g_second_pkg.len = UART_ACK_PKG_LEN;
			spin_unlock_irqrestore(&write_lock, flags);
			return IRQ_HANDLED;
		}
		g_second_pkg.count = UART_LONG_PKG_COUNT;
		second_len = len - COMMU_UART_FIRST_WRITE;
		/* 8 for aligned bytes */
		second_len = second_len % 8 == 0 ? second_len : (second_len / 8 + 1) * 8;

		g_second_pkg.len = second_len;
		g_second_pkg.buffer = buf + COMMU_UART_FIRST_WRITE;
	}

	spin_unlock_irqrestore(&write_lock, flags);
	return IRQ_HANDLED;
}

static int uart_read_count(u32 *read_len)
{
	*read_len = MAX_UART_READ_BUF_LEN;
	return 0;
}

static int uart_commu_read(u8 *buf, u32 len)
{
	return ext_sensorhub_uart_read(buf, len);
}

static irqreturn_t uart_commu_wake_irq(int irq, void *arg)
{
	bool ret;
	struct commu_data *commu_data = get_commu_data(UART_DRIVER);

	if (!commu_data)
		return IRQ_HANDLED;

	if (gpio_get_value(commu_data->wakeup_ap_gpio) == 0)
		return IRQ_HANDLED;
	/* check mcu wakeup ap is high, bt reboot, clear init flag */
	if (gpio_get_value(commu_data->mcu_read_ready_gpio) == 1 &&
	    gpio_get_value(commu_data->wakeup_ap_gpio) == 1) {
		pr_err("%s bt core is rebooting, clear init flag", __func__);
		notify_mcu_reboot(commu_data->driver);
		return IRQ_HANDLED;
	}

	ret = highpri_work(&commu_data->wakeup_read_work);
	if (ret)
		/* 300ms timeout wakeup lock */
		__pm_wakeup_event(commu_data->wake_lock, 300);
	return IRQ_HANDLED;
}

static int uart_read_ready(bool is_ready)
{
	int ret;
	struct commu_data *commu_data = get_commu_data(UART_DRIVER);

	if (!commu_data)
		return -EINVAL;

	if (is_ready)
		/* begin read, already get op lock, here cannot close uart */
		/* here to open uart */
		ext_sensorhub_uart_open();

	ret = ext_sensorhub_uart_read_begin(is_ready);

	if (is_ready) {
		if (gpio_get_value(commu_data->wakeup_ap_gpio) != 1) {
			pr_err("%s uart driver wakeup_ap_gpio is not high",
			       __func__);
			return -ETIME;
		}
	}

	return ret;
}

static void uart_write_ready(unsigned char *buf, size_t len)
{
	unsigned long flags;

	if (buf && len > 0) {
		/* here to open uart already get op lock */
		ext_sensorhub_uart_open();
		preempt_disable();
		spin_lock_irqsave(&write_lock, flags);
		g_second_pkg.write_flag = 0;
		g_second_pkg.buffer = buf;
		g_second_pkg.len = len;
		g_second_pkg.write_done = false;
		spin_unlock_irqrestore(&write_lock, flags);
		preempt_enable();
	} else {
		preempt_disable();
		spin_lock_irqsave(&write_lock, flags);
		g_second_pkg.write_flag = 0;
		g_second_pkg.buffer = NULL;
		g_second_pkg.len = 0;
		g_second_pkg.write_done = false;
		spin_unlock_irqrestore(&write_lock, flags);
		preempt_enable();
	}
}

static int commu_uart_wait_complete(void)
{
	int ret;
	int status = 0;

	ret = wait_event_interruptible_timeout(
		g_second_pkg.write_wait, g_second_pkg.write_done,
		msecs_to_jiffies(COMMU_UART_SECOND_WRITE_TIMOUE_MS));
	if (ret == 0) {
		pr_err("%s wait for second complete timeout", __func__);
		return -ETIMEDOUT;
	}
	if (ret < 0) {
		pr_err("%s wait for second complete error: %d", __func__, ret);
		return ret;
	}

	return status;
}

static int uart_commu_write(u8 *buf, u32 len)
{
	int ret;
	unsigned long flags;

	if (g_uart_write_mode != 0) {
		pr_warn("%s uart is in direct mode", __func__);
		return -EBUSY;
	}

	ret = commu_uart_wait_complete();
	if (ret != 0) {
		pr_err("%s uart write wait next err:%d", __func__, ret);
		goto err;
	}
err:
	preempt_disable();
	spin_lock_irqsave(&write_lock, flags);
	if (ret != 0 && g_second_pkg.write_done) {
		ret = 0;
		pr_info("%s uart write wait timeout before rx done", __func__);
	}
	g_second_pkg.buffer = NULL;
	g_second_pkg.len = 0;
	g_second_pkg.write_flag = 0;
	spin_unlock_irqrestore(&write_lock, flags);
	preempt_enable();
	return ret;
}

static size_t uart_write_count(size_t len)
{
	int ret;
	struct commu_data *commu_data = get_commu_data(UART_DRIVER);

	if (!commu_data || !ext_sensorhub_uart_has_probed())
		return 0;
	/* wait 6 HZ timeout for resume */
	ret = down_timeout(&commu_data->sr_wake_sema, 6 * HZ);
	if (ret == -ETIME) {
		pr_err("%s driver:%d down sr wake sema timeout",
		       __func__, UART_DRIVER);
		return 0;
	}
	up(&commu_data->sr_wake_sema);
	return len;
}

int ext_sensorhub_commu_uart_close(void)
{
	int ret;
	struct commu_data *commu_data = get_commu_data(UART_DRIVER);

	if (!commu_data || !commu_data->initialed)
		return -EINVAL;

	mutex_lock(&commu_data->commu_op_lock);
	ret = ext_sensorhub_uart_close();
	mutex_unlock(&commu_data->commu_op_lock);

	return ret;
}

int ext_sensorhub_commu_uart_open(void)
{
	int ret;
	struct commu_data *commu_data = get_commu_data(UART_DRIVER);

	if (!commu_data || !commu_data->initialed)
		return -EINVAL;

	mutex_lock(&commu_data->commu_op_lock);
	ret = ext_sensorhub_uart_open();
	mutex_unlock(&commu_data->commu_op_lock);

	return ret;
}

static void commu_data_uart_init(struct commu_data *commu_data)
{
	if (!commu_data)
		return;

	commu_data->commu_ops.op_commu_init = uart_commu_init;
	commu_data->commu_ops.op_commu_exit = uart_commu_exit;
	commu_data->commu_ops.op_read_count = uart_read_count;
	commu_data->commu_ops.op_commu_read = uart_commu_read;
	commu_data->commu_ops.op_write_count = uart_write_count;
	commu_data->commu_ops.op_commu_write = uart_commu_write;
	commu_data->commu_ops.op_commu_rx_irq = uart_commu_rx_irq;
	commu_data->commu_ops.op_commu_wake_irq = uart_commu_wake_irq;
	commu_data->commu_ops.op_read_ready = uart_read_ready;
	commu_data->commu_ops.op_write_ready = uart_write_ready;

	commu_data->driver = UART_DRIVER;
	commu_data->active_handshake = 1;
}

static int __init register_uart_commu(void)
{
	register_commu_driver(UART_DRIVER, commu_data_uart_init);
	pr_info("%s call in", __func__);
	return 0;
}

fs_initcall(register_uart_commu);
