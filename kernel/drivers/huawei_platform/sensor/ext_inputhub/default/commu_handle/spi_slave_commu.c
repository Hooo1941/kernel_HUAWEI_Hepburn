/*
 * spi_slave_commu.c
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

#include "spi_slave_commu.h"

#include <linux/delay.h>
#include <linux/of_gpio.h>
#include "securec.h"
#include "ext_sensorhub_spi.h"
#include "ext_sensorhub_frame.h"

#include <linux/jiffies.h>
#include <linux/timer.h>

#define COMMU_WAIT_NEXT_WRITE_DELAY_TIME_US 5
#define RESET_SENSORHUB_INTERVAL_MS 6
#define QUEUE_WORK_RETRY_COUNT 3
#define COMMU_WAIT_NEXT_WRITE_DELAY_TIMES 200

struct force_upgrade {
	int rst_flag_gpio;
	int resp_irq;
};

static u8 g_read_buffer[DEFAULT_SPI_READ_LEN];
static u8 *g_read_dma_buffer = NULL;
void set_spi_xfer_timeout(unsigned long jiffies);

static struct force_upgrade g_force_upgrade;
static struct timer_list mcu_resp_timer;
static struct wakeup_source *rst_mcu_wake_lock;


static irqreturn_t mcu_rst_resp_irq(int irq, void *arg)
{
	gpio_direction_input(g_force_upgrade.rst_flag_gpio);
	del_timer(&mcu_resp_timer);
	__pm_relax(rst_mcu_wake_lock);
	return IRQ_HANDLED;
}


static void force_upgrade_init(struct commu_data *commu_data)
{
	int ret;

	/* reset mcu */
	ret = commu_request_common_gpio(&commu_data->mcu_rst_gpio,
					"mcu,forceupgrade", "mcu_reset");
	if (ret < 0)
		pr_err("%s request mcu reset gpio failed", __func__);

	ret = commu_request_common_gpio(&commu_data->rst_flag_gpio,
					"mcu,reset_flag", "reset_flag");
	if (ret < 0)
		pr_err("%s request mcu reset_flag gpio failed", __func__);

	ret = commu_request_direction_gpio(&commu_data->rst_resp_gpio,
					   "mcu,reset_resp", "reset_resp");
	if (ret < 0)
		pr_err("%s request mcu reset_resp gpio failed", __func__);

	g_force_upgrade.rst_flag_gpio = commu_data->rst_flag_gpio;
	g_force_upgrade.resp_irq = gpio_to_irq(commu_data->rst_resp_gpio);

	pr_info("hardreset request_irq_gpio g_force_upgrade->resp_irq = %d",
		g_force_upgrade.resp_irq);

	ret = request_irq(g_force_upgrade.resp_irq,
			  mcu_rst_resp_irq, IRQF_TRIGGER_RISING, "mcu_rst_resp_irq", NULL);
	if (ret < 0)
		pr_err("hardrest couldn't acquire irq = %d\n", ret);

	rst_mcu_wake_lock = wakeup_source_register(NULL, "rst_mcu");
}

static void mcu_rst_resp_timer_callback(struct timer_list *arg)
{
	gpio_direction_input(g_force_upgrade.rst_flag_gpio);
	__pm_relax(rst_mcu_wake_lock);
	pr_info("mcu response rst req timeout");
}

static int spi_commu_init(struct commu_data *commu_data)
{
	int ret;

	if (!commu_data)
		return -EINVAL;
	g_read_dma_buffer = kmalloc(DEFAULT_SPI_READ_LEN, GFP_DMA);
	pr_info("init buffer:%p, malloc buffer:%p", g_read_buffer, g_read_dma_buffer);
	ret = ext_sensorhub_spi_init();
	commu_data->initialed = true;

	force_upgrade_init(commu_data);
	__init_timer(&mcu_resp_timer, NULL, 0);
	mcu_resp_timer.function = mcu_rst_resp_timer_callback;
	return ret;
}

static void spi_commu_exit(struct commu_data *commu_data)
{
	ext_sensorhub_spi_exit();
	if (g_read_dma_buffer)
		kfree(g_read_dma_buffer);
}

static int spi_read_count(u32 *read_len)
{
	*read_len = DEFAULT_SPI_READ_LEN;
	return 0;
}

static int spi_commu_read(u8 *buf, u32 len)
{
	if (memcpy_s(buf, len, g_read_dma_buffer, len) != EOK)
		return -EFAULT;

	return 0;
}

bool check_spi_slave_resume(void);
static size_t spi_write_count(size_t len)
{
	/* if spi slave is not ready, return write len invalid to stop write */
	if (!check_spi_slave_resume()) {
		pr_err("%s check_spi_slave_resume false ", __func__);
		return 0;
	}
	return SPI_SEND_LEN;
}

static int spi_commu_write(u8 *buffer, u32 length)
{
	int ret;

	ret = ext_sensorhub_spi_write(buffer, length);
	set_spi_xfer_timeout(0);

	return ret;
}

static irqreturn_t spi_commu_rx_irq(int irq, void *arg)
{
	return IRQ_HANDLED;
}

static irqreturn_t spi_commu_wake_irq(int irq, void *arg)
{
	struct commu_data *commu_data = get_commu_data(SPI_SLAVE_DRIVER);
	bool kret = false;
	int i;

	if (!commu_data)
		return IRQ_HANDLED;

	if (gpio_get_value(commu_data->wakeup_ap_gpio) == 0)
		return IRQ_HANDLED;
	/* check mcu wakeup ap is high, mcu reboot, clear init flag */
	if (gpio_get_value(commu_data->mcu_read_ready_gpio) == 1 &&
	    gpio_get_value(commu_data->wakeup_ap_gpio) == 1) {
		pr_err("%s mcu is rebooting, clear init flag", __func__);
		notify_mcu_reboot(commu_data->driver);
		return IRQ_HANDLED;
	}

	if (!ext_sensorhub_spi_has_probed())
		return IRQ_HANDLED;

	for (i = 0; i < QUEUE_WORK_RETRY_COUNT; ++i) {
		kret = kthread_queue_work(&commu_data->read_kworker,
					  &commu_data->read_work);
		if (kret) {
			/* 300ms timeout wakeup lock */
			__pm_wakeup_event(commu_data->wake_lock, 300);
			break;
		}
	}

	return IRQ_HANDLED;
}

static int spi_slave_read_ready(bool is_ready)
{
	int ret = 0;
	struct commu_data *commu_data = get_commu_data(SPI_SLAVE_DRIVER);

	if (!commu_data)
		return -EINVAL;

	if (!g_read_dma_buffer)
		g_read_dma_buffer = kmalloc(DEFAULT_SPI_READ_LEN, GFP_DMA);
	if (!g_read_dma_buffer)
		return -EINVAL;

	if (is_ready) {
		if (gpio_get_value(commu_data->wakeup_ap_gpio) != 1) {
			pr_err("%s uart driver wakeup_ap_gpio is not high",
			       __func__);
			return -ETIME;
		}

		ret = ext_sensorhub_spi_read(g_read_dma_buffer,
					     DEFAULT_SPI_READ_LEN);
		set_spi_xfer_timeout(0);
	}

	return ret;
}

void commu_spi_slave_ready(enum spi_slave_opt opt)
{
	struct commu_data *commu_data = get_commu_data(SPI_SLAVE_DRIVER);

	if (!commu_data)
		return;

	/* write flag, control wakeup down */
	if (opt == SLAVE_WRITE)
		gpio_direction_output(commu_data->wakeup_mcu_gpio, 0);

	/* read flag, control read ready up */
	if (opt == SLAVE_READ) {
		if (gpio_get_value(commu_data->wakeup_ap_gpio) != 1) {
			pr_err("%s driver wakeup_ap_gpio is not high", __func__);
			/* just set 1us timed out for quick exit */
			set_spi_xfer_timeout(usecs_to_jiffies(1));
		} else {
			gpio_direction_output(commu_data->ap_read_ready_gpio, 1);
		}
	}
}

int spi_slave_ext_sensorhub_reset(void)
{
	struct commu_data *commu_data = get_commu_data(SPI_SLAVE_DRIVER);
	if (!commu_data)
		return -EINVAL;

	pr_info("spi slave reset sensorhub start");
	mutex_lock(&commu_data->commu_op_lock);
	__pm_stay_awake(rst_mcu_wake_lock);
	gpio_direction_output(commu_data->rst_flag_gpio, 0);
	gpio_direction_output(commu_data->mcu_rst_gpio, 1);

	mcu_resp_timer.expires = jiffies + msecs_to_jiffies(10000);
	add_timer(&mcu_resp_timer);
	mdelay(200);
	gpio_direction_output(commu_data->mcu_rst_gpio, 0);
	mutex_unlock(&commu_data->commu_op_lock);
	pr_info("spi slave reset sensorhub complete");

	return 0;
}

static void commu_data_spi_slave_init(struct commu_data *commu_data)
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
	commu_data->commu_ops.op_read_ready = spi_slave_read_ready;

	commu_data->driver = SPI_SLAVE_DRIVER;
	commu_data->active_handshake = 1;
}

static int __init register_spi_slave_commu(void)
{
	register_commu_driver(SPI_SLAVE_DRIVER, commu_data_spi_slave_init);
	pr_info("%s call in", __func__);
	return 0;
}

fs_initcall(register_spi_slave_commu);
