/*
 * spi_commu.c
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

#include "spi_commu.h"

#include <linux/of_gpio.h>
#include <linux/delay.h>
#include <uapi/linux/sched/types.h>
#include "securec.h"
#include "ext_sensorhub_spi.h"
#include "ext_sensorhub_frame.h"

#define COMMU_WAIT_NEXT_WRITE_DELAY_TIME_US 5
#define RESET_SENSORHUB_INTERVAL_MS 6
#define QUEUE_WORK_RETRY_COUNT 3
#define COMMU_WAIT_NEXT_WRITE_DELAY_TIMES 200

struct force_upgrade {
	unsigned int recv_irq;
	struct work_struct recv_work;
	int mcu_rst_gpio;
};

struct spi_write_context {
	struct task_struct *kworker_task;
	struct kthread_work write_work;
	struct kthread_worker kworker;
	wait_queue_head_t spi_write_wait;

	u8 *buf;
	u32 len;
	bool spi_write_done;
	int spi_write_flag;
	int spi_ret;
};

static u32 g_spi_read_len = DEFAULT_SPI_READ_LEN;
static struct force_upgrade g_force_upgrade;
static struct spi_write_context g_spi_context;

static irqreturn_t force_upgrade_recv(int irq, void *arg)
{
	pr_info("%s get irq:%d", __func__, irq);
	if (!schedule_work(&g_force_upgrade.recv_work))
		pr_info("%s schedule work error", __func__);

	return IRQ_HANDLED;
}

static void force_upgrade_recv_work(struct work_struct *work)
{
	/* upload irq to commu msg */
	notify_mcu_force_upgrade_irq();
}

int get_spi_commu_mode(void)
{
	struct commu_data *commu_data = get_commu_data(SPI_DRIVER);

	if (!commu_data)
		return -EINVAL;
	return commu_data->commu_direct_mode;
}

int ext_sensorhub_mcu_boot(int value)
{
	struct commu_data *commu_data = get_commu_data(SPI_DRIVER);

	if (!commu_data)
		return -EINVAL;

	pr_info("%s set value:%d, mode:%d", __func__, value,
		commu_data->commu_direct_mode);
	if (commu_data->commu_direct_mode == 0)
		return -EPERM;

	mutex_lock(&commu_data->commu_op_lock);
	gpio_direction_output(commu_data->mcu_read_ready_gpio, value);
	mutex_unlock(&commu_data->commu_op_lock);

	return 0;
}

int ext_sensorhub_spi_direct_write(u8 *buf, u32 len, u32 tx_len, u8 *tx_buf)
{
	int ret;
	struct commu_data *commu_data = get_commu_data(SPI_DRIVER);

	if (!commu_data)
		return -EINVAL;

	mutex_lock(&commu_data->commu_op_lock);
	ret = ext_sensorhub_spi_update(buf, len, tx_len, tx_buf);
	mutex_unlock(&commu_data->commu_op_lock);

	return ret;
}

/* here to switch ap&mcu rdy gpio mode */
int ext_sensorhub_set_force_upgrade_mode(int mode)
{
	int retval;
	struct commu_data *commu_data = get_commu_data(SPI_DRIVER);

	if (!commu_data)
		return -EINVAL;

	pr_info("%s set mode:%d", __func__, mode);
	/* 0 normal mode */
	if (mode == 0) {
		/* set mcu_read_ready_gpio to input and request irq */
		gpio_direction_output(commu_data->mcu_read_ready_gpio, 0);
		gpio_direction_input(commu_data->mcu_read_ready_gpio);
		retval = request_irq(commu_data->rx_ready_irq,
				     commu_data->commu_ops.op_commu_rx_irq,
				     IRQF_TRIGGER_FALLING | IRQF_NO_SUSPEND,
				     "commu wake irq", commu_data);
		if (retval < 0)
			pr_warn("%s request irq failed", __func__);

		free_irq(g_force_upgrade.recv_irq, commu_data);
		gpio_direction_output(commu_data->ap_read_ready_gpio, 0);
		gpio_direction_output(g_force_upgrade.mcu_rst_gpio, 0);
		commu_data->commu_direct_mode = 0;
	} else {
		commu_data->commu_direct_mode = 1;
		/* switch to upgrade mode. 1. free irq, 2. set gpio to low */
		free_irq(commu_data->rx_ready_irq, commu_data);
		gpio_direction_output(commu_data->mcu_read_ready_gpio, 0);
		gpio_direction_output(commu_data->ap_read_ready_gpio, 0);

		/* get mcu initial interrupt */
		gpio_direction_input(commu_data->ap_read_ready_gpio);
		retval = request_irq(g_force_upgrade.recv_irq,
				     force_upgrade_recv,
				     IRQF_TRIGGER_RISING | IRQF_NO_SUSPEND,
				     "force upgrade", commu_data);
		if (retval < 0)
			pr_warn("%s request force upgrade irq failed", __func__);
	}

	return 0;
}

static void force_upgrade_init(struct commu_data *commu_data)
{
	int ret;

	if (!commu_data)
		return;

	INIT_WORK(&g_force_upgrade.recv_work, force_upgrade_recv_work);
	g_force_upgrade.recv_irq = gpio_to_irq(commu_data->ap_read_ready_gpio);
	ret = commu_request_common_gpio(&g_force_upgrade.mcu_rst_gpio,
					"mcu,forceupgrade", "mcu_reset");
	if (ret < 0)
		pr_warn("%s request mcu reset gpio failed", __func__);
}

static int commu_spi_wait_rdy(int index)
{
	int status = 0;
	int delay_times = 0;
	struct commu_data *commu_data = get_commu_data(SPI_DRIVER);

	if (!commu_data)
		return -EINVAL;

	while (gpio_get_value(commu_data->mcu_read_ready_gpio) != 1 ||
	       g_spi_context.spi_write_flag <= index) {
		delay_times++;
		if (delay_times > COMMU_WAIT_NEXT_WRITE_DELAY_TIMES) {
			status = -ETIME;
			pr_err("%s wait for rdy up delay times is out of range",
			       __func__);
			goto err;
		}
		udelay(COMMU_WAIT_NEXT_WRITE_DELAY_TIME_US);
	}

err:
	return status;
}

static void spi_write_worker(struct kthread_work *work)
{
	int ret;
	int i;
	int each_len = SPI_SEND_DATA_LEN;
	int pkg_count = g_spi_context.len / each_len +
	(g_spi_context.len % each_len == 0 ? 0 : 1);
	int send_len;
	int copy_len;
	u8 *buffer = NULL;

	/* write begin with one more byte 0x80 */
	buffer = kmalloc(each_len + 1, GFP_KERNEL);
	if (!buffer || g_spi_context.spi_write_done) {
		ret = -EFAULT;
		goto err;
	}
	g_spi_context.spi_write_flag = 0;
	for (i = 0; i < pkg_count; ++i) {
		copy_len = each_len;
		send_len = each_len + 1;
		if (i == pkg_count - 1) {
			copy_len = g_spi_context.len -
			each_len * (pkg_count - 1);
			send_len = SPI_SEND_LEN -
			(each_len + 1) * (pkg_count - 1);
		}

		ret = memcpy_s(&buffer[1], each_len,
			       g_spi_context.buf + i * SPI_SEND_DATA_LEN,
			       copy_len);
		if (ret != EOK) {
			ret = -EFAULT;
			goto err;
		}
		buffer[0] = 0x80;
		ret = ext_sensorhub_spi_write(buffer, send_len);
		if (ret < 0)
			goto err;
		/* wait for another rdy pull down and up, last pkg don't wait */
		if (i != pkg_count - 1) {
			ret = commu_spi_wait_rdy(i);
			if (ret != 0)
				goto err;
		}
	}
err:
	g_spi_context.spi_write_flag = 0;
	g_spi_context.spi_ret = ret;
	g_spi_context.spi_write_done = true;
	wake_up_interruptible(&g_spi_context.spi_write_wait);
	kfree(buffer);
}

void spi_suspend_clear_last_write(void)
{
	struct commu_data *commu_data = get_commu_data(SPI_DRIVER);

	pr_info("%s clear last write", __func__);
	g_spi_context.spi_write_flag = 0;
	g_spi_context.spi_ret = -EACCES;
	g_spi_context.spi_write_done = true;
	if (commu_data)
		gpio_direction_output(commu_data->wakeup_mcu_gpio, 0);

	wake_up_interruptible(&g_spi_context.spi_write_wait);
}

static void spi_init_task(void)
{
	int ret;
	struct sched_param param = { .sched_priority = COMMU_SCHED_PRIORITY };

	init_waitqueue_head(&g_spi_context.spi_write_wait);
	kthread_init_worker(&g_spi_context.kworker);
	g_spi_context.kworker_task = kthread_run(kthread_worker_fn,
						 &g_spi_context.kworker,
						 "%s", "spi_write_worker");
	if (IS_ERR(g_spi_context.kworker_task)) {
		pr_err("failed to create spi write task");
		return;
	}
	kthread_init_work(&g_spi_context.write_work, spi_write_worker);

	ret = sched_setscheduler(g_spi_context.kworker_task,
				 SCHED_FIFO, &param);
	pr_info("%s get init sched_setscheduler ret:%d", __func__, ret);
}

int ext_sensorhub_mcu_reset(int value)
{
	return 0;
}

int ext_sensorhub_reset(void)
{
	return 0;
}

static int spi_commu_init(struct commu_data *commu_data)
{
	int ret;

	if (!commu_data)
		return -EINVAL;

	spi_init_task();

	ret = ext_sensorhub_spi_init();
	commu_data->initialed = true;

	force_upgrade_init(commu_data);
	return ret;
}

static void spi_commu_exit(struct commu_data *commu_data)
{
	ext_sensorhub_spi_exit();
}

/* adapt for read len change */
void set_spi_read_len(u32 len)
{
	g_spi_read_len = len;
	pr_info("%s spi read len: %d", __func__, len);
}

static int spi_read_count(u32 *read_len)
{
	*read_len = g_spi_read_len - 1;
	return 0;
}

static int spi_commu_read(u8 *buf, u32 len)
{
	int ret;
	int i;

	/* read buffer already has one more byte */
	ret = ext_sensorhub_spi_read(buf, len + 1);
	if (ret < 0)
		return ret;

	/* remove data beginning 0x00 */
	for (i = 0; i < len; ++i)
		buf[i] = buf[i + 1];

	return ret;
}

static size_t spi_write_count(size_t len)
{
	return SPI_FRAME_BUF_LEN;
}

static int spi_commu_write(u8 *buffer, u32 length)
{
	int ret;
	bool kret = false;
	int i;
	int retry_count;
	struct commu_data *commu_data = get_commu_data(SPI_DRIVER);

	g_spi_context.buf = buffer;
	g_spi_context.len = length;
	g_spi_context.spi_write_done = false;
	for (i = 0; i < QUEUE_WORK_RETRY_COUNT; ++i) {
		kret = kthread_queue_work(&g_spi_context.kworker,
					  &g_spi_context.write_work);
		if (kret)
			break;
	}
	if (!kret) {
		g_spi_context.buf = NULL;
		g_spi_context.len = 0;
		g_spi_context.spi_write_done = true;
		if (commu_data)
			__pm_relax(commu_data->wake_lock);

		pr_err("%s queue spi kwork err", __func__);
		return -EAGAIN;
	}

	/* try to get in certain time */
	retry_count = 0;
	ret = 0;
	while (!g_spi_context.spi_write_done) {
		retry_count++;
		udelay(COMMU_WRITE_DELAY_TIME_US);
		if (retry_count >= COMMU_WRITE_MAX_DELAY_TIMES) {
			ret = -ETIME;
			break;
		}
	}
	if (ret < 0) {
		pr_info("%s cannot get write result in certain time, ret:%d, done flag:%d",
			__func__, ret, g_spi_context.spi_write_done);
		ret = wait_event_interruptible(g_spi_context.spi_write_wait,
					       g_spi_context.spi_write_done);
	}
	if (ret < 0) {
		g_spi_context.buf = NULL;
		g_spi_context.len = 0;
		pr_warn("%s spi write end get ret:%d", __func__, ret);
	}

	return g_spi_context.spi_ret;
}

static irqreturn_t spi_commu_rx_irq(int irq, void *arg)
{
	g_spi_context.spi_write_flag++;
	return IRQ_HANDLED;
}

static irqreturn_t spi_commu_wake_irq(int irq, void *arg)
{
	struct commu_data *commu_data = (struct commu_data *)arg;
	bool kret = false;
	int i;

	if (!commu_data)
		return IRQ_HANDLED;
	/* check mcu wakeup ap is high, mcu reboot, clear init flag */
	if (gpio_get_value(commu_data->mcu_read_ready_gpio) == 1 &&
	    gpio_get_value(commu_data->wakeup_ap_gpio) == 1) {
		pr_err("%s mcu is rebooting, clear init flag", __func__);
		notify_mcu_reboot(commu_data->driver);
		return IRQ_HANDLED;
	}

	for (i = 0; i < QUEUE_WORK_RETRY_COUNT; ++i) {
		kret = kthread_queue_work(&commu_data->read_kworker,
					  &commu_data->read_work);
		if (kret)
			break;
	}

	return IRQ_HANDLED;
}

static void commu_data_spi_init(struct commu_data *commu_data)
{
	if (!commu_data)
		return;

	commu_data->commu_ops.op_commu_init = spi_commu_init;
	commu_data->commu_ops.op_commu_exit = spi_commu_exit;
	commu_data->commu_ops.op_read_count = spi_read_count;
	commu_data->commu_ops.op_commu_read = spi_commu_read;
	commu_data->commu_ops.op_write_count = spi_write_count;
	commu_data->commu_ops.op_commu_write = spi_commu_write;
	commu_data->commu_ops.op_commu_rx_irq = spi_commu_rx_irq;
	commu_data->commu_ops.op_commu_wake_irq = spi_commu_wake_irq;

	commu_data->driver = SPI_DRIVER;
	commu_data->active_handshake = 1;
}

static int __init register_spi_commu(void)
{
	register_commu_driver(SPI_DRIVER, commu_data_spi_init);
	pr_info("%s call in", __func__);
	return 0;
}

fs_initcall(register_spi_commu);
