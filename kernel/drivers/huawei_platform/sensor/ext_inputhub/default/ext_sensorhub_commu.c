/*
 * ext_sensorhub_commu.c
 *
 * code for communication with external sensorhub
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
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

#include "ext_sensorhub_commu.h"

#include <linux/errno.h>
#include <linux/stddef.h>
#include <linux/of_gpio.h>
#include <linux/of_address.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/pm_wakeup.h>
#include <linux/suspend.h>
#ifndef CONFIG_EXT_INPUTHUB_QCOM
#include <soc_gpio_interface.h>
#endif
#include <linux/syscore_ops.h>
#include <linux/preempt.h>
#include <linux/syscalls.h>
#include <uapi/linux/sched/types.h>
#include <linux/version.h>

#include "securec.h"
#include "ext_sensorhub_sdio.h"
#include "ext_sensorhub_spi.h"
#include "ext_sensorhub_uart.h"
#include "ext_sensorhub_frame.h"
#include "spi_commu.h"

#define COMMU_READ_DELAY_TIME_US	100
#define COMMU_READ_MAX_DELAY_TIMES	200
#define COMMU_RETRY_INTERVAL_US		50
#define COMMU_MAX_RETRY_TIMES		200
#define AO_GPIO_GROUP_STEP		8
#define IO_BUFFER_LEN			40
/* multiple package */
#define DMA_READY_TIMEOUT_ERRNO 123
#define DMA_READY_WAKE_LOCK_TIME_MS 200

static int ap_status_gpio;
/* enum coumm driver */
static struct commu_data *commu_data_tbl[COMMU_DRIVER_COUNT];

int commu_request_common_gpio(int *gpio, const char *compat, const char *name)
{
	struct device_node *np = NULL;

	np = of_find_compatible_node(NULL, NULL, compat);
	if (!np) {
		pr_err("gpio_request %s: %s node not found\n", __func__, name);
		return -ENODEV;
	}

	*gpio = of_get_named_gpio(np, name, 0);
	if (*gpio < 0) {
		pr_err("gpio_request error %s:%d.\n", name, *gpio);
		return -ENODEV;
	}

	if (gpio_request(*gpio, name) < 0) {
		pr_err("gpio_request Failed to request gpio %d for %s\n",
		       *gpio, name);
		return -ENODEV;
	}

	return 0;
}

int commu_request_direction_gpio(
	int *gpio, const char *compat, const char *name)
{
	int ret;

	ret = commu_request_common_gpio(gpio, compat, name);
	if (ret < 0)
		return ret;

	if (gpio_direction_input(*gpio) < 0) {
		pr_err("gpio_request Failed to set dir %d for %s\n",
		       *gpio, compat);
		return -ENODEV;
	}

	return ret;
}

static int commu_request_gpio(struct device *dev, struct commu_data *commu_data)
{
	int ret;
	struct fwnode_handle *child = NULL;
	struct gpio_desc *gpiod = NULL;

	device_for_each_child_node(dev, child) {
		gpiod = devm_fwnode_get_index_gpiod_from_child(dev, NULL, commu_data->driver,
							       child, GPIOD_IN, "mcu,wakeupap");
		if (IS_ERR(gpiod)) {
			pr_err("%s get giod err\n", __func__);
			return -EINVAL;
		}
		commu_data->wakeup_ap_gpio = desc_to_gpio(gpiod);
		pr_info("%s get wakeup_ap_gpio:%u\n", __func__, commu_data->wakeup_ap_gpio);
		commu_data->wake_irq = gpiod_to_irq(gpiod);
	}

	/* Get AP Wakeup MCU Gpio */
	ret = commu_request_common_gpio(&commu_data->wakeup_mcu_gpio,
					"mcu,wakeupmcu",
					commu_data->wakeup_mcu);
	if (ret < 0) {
		pr_err(" %s: mcu,wakeupmcu node not found\n", __func__);
		return ret;
	}
	pr_info("ap_wakeup_mcu gpio:%d done\n",
		commu_data->wakeup_mcu_gpio);

	/* Get AP Read Sync Gpio */
	ret = commu_request_common_gpio(&commu_data->ap_read_ready_gpio,
					"mcu,readsync",
					commu_data->ap_read_ready);
	if (ret < 0) {
		pr_err("%s: mcu,readsync node not found\n", __func__);
		return ret;
	}
	pr_info("ap_read_ready_gpio gpio:%d done\n",
		commu_data->ap_read_ready_gpio);

	/* Get AP Write Sync Gpio */
	ret = commu_request_direction_gpio(&commu_data->mcu_read_ready_gpio,
					   "mcu,writesync",
					   commu_data->mcu_read_ready);
	if (ret < 0) {
		pr_err("%s: mcu,writesync node not found\n", __func__);
		return ret;
	}
	pr_info("mcu_read_ready_gpio gpio:%d done\n",
		commu_data->mcu_read_ready_gpio);
	/* for rx irq */
	commu_data->rx_ready_irq = gpio_to_irq(commu_data->mcu_read_ready_gpio);
	pr_info("request_irq_gpio commu_data.rx_ready_irq = %u",
		commu_data->rx_ready_irq);

	return 0;
}

#ifdef CONFIG_EXT_INPUTHUB_QCOM
static int hisi_get_gpio_base(struct commu_data *commu_data)
{
	return 0;
}
#else
/* do after gpio init */
static int hisi_get_gpio_base(struct commu_data *commu_data)
{
	struct device_node *dn = NULL;
	unsigned int group;
	char *io_buffer = NULL;
	int err;
	int ret = 0;

	pr_info("%s wake up gpio: %d\n", __func__, commu_data->wakeup_ap_gpio);
	group = commu_data->wakeup_ap_gpio / AO_GPIO_GROUP_STEP;
	io_buffer = kzalloc(IO_BUFFER_LEN, GFP_KERNEL);
	if (!io_buffer)
		return -ENOMEM;
	memset_s(io_buffer, IO_BUFFER_LEN, 0, IO_BUFFER_LEN);
	err = snprintf_s(io_buffer, IO_BUFFER_LEN, IO_BUFFER_LEN - 1,
			 "arm,primecell%u", group);
	if (err < 0) {
		pr_err("[%s]snprintf_s fail[%d]\n", __func__, ret);
		ret = err;
		goto err_free;
	}

	dn = of_find_compatible_node(NULL, NULL, io_buffer);
	if (!dn) {
		pr_err("%s: hisilicon,primecell%d No compatible node found\n",
		       __func__, group);
		ret = -ENODEV;
		goto err_free;
	}
	commu_data->ao_gpio_base = of_iomap(dn, 0);
	if (!commu_data->ao_gpio_base) {
		ret = -EINVAL;
		of_node_put(dn);
		goto err_free;
	}
	of_node_put(dn);
	pr_info("%s get ao gpio base success\n", __func__);

err_free:
	kfree(io_buffer);
	return ret;
}
#endif

static void commu_release_gpio(struct commu_data *commu_data)
{
	gpio_free(commu_data->wakeup_ap_gpio);
	gpio_free(commu_data->wakeup_mcu_gpio);
	gpio_free(commu_data->ap_read_ready_gpio);
	gpio_free(commu_data->mcu_read_ready_gpio);
}

static int commu_request_irq(struct device *dev, struct commu_data *commu_data)
{
	int retval;

	if (!commu_data) {
		pr_err("%s commu_data is null\n", __func__);
		return -EINVAL;
	}

	if (commu_data->driver == SPI_SLAVE_DRIVER) {
		retval = devm_request_any_context_irq(dev, commu_data->wake_irq,
						      commu_data->commu_ops.op_commu_wake_irq,
						      IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
						      "mcu_wake", dev);
	} else {
		retval = devm_request_any_context_irq(dev, commu_data->wake_irq,
						      commu_data->commu_ops.op_commu_wake_irq,
						      IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
						      "bt_wake", dev);
	}

	if (retval < 0) {
		pr_err("couldn't acquire MCU HOST WAKE UP IRQ reval = %d\n",
		       retval);
		return retval;
	}

	if (commu_data->driver == SPI_DRIVER ||
	    commu_data->driver == SPI_SLAVE_DRIVER) {
		retval = request_irq(commu_data->rx_ready_irq,
				     commu_data->commu_ops.op_commu_rx_irq,
				     IRQF_TRIGGER_FALLING | IRQF_NO_SUSPEND,
				     "commu wake irq", commu_data);
	} else {
		retval = request_irq(commu_data->rx_ready_irq,
				     commu_data->commu_ops.op_commu_rx_irq,
				     IRQF_TRIGGER_RISING | IRQF_NO_SUSPEND,
				     "commu wake irq", commu_data);
	}
	if (retval < 0) {
		pr_err("couldn't acquire MCU HOST INIT IRQ reval = %d\n",
		       retval);
		return retval;
	}

	return 0;
}

static int commu_read_check(struct commu_data *commu_data)
{
	enum commu_driver driver = commu_data->driver;
	int ret;

	if (!commu_data->initialed) {
		pr_err("%s driver:%d commu data not init\n", __func__, driver);
		return -EINVAL;
	}

	if (commu_data->commu_direct_mode)
		return -EBUSY;

	if (gpio_get_value(commu_data->wakeup_ap_gpio) != 1) {
		pr_warn("%s driver:%d wake up ap has pull down, do nothing",
			__func__, driver);
		return -EINVAL;
	}
	/* wait 6 HZ timeout for resume */
	ret = down_timeout(&commu_data->sr_wake_sema, 6 * HZ);
	if (ret == -ETIME) {
		pr_err("%s driver:%d down sr wake sema timeout",
		       __func__, driver);
		return -EINVAL;
	}
	up(&commu_data->sr_wake_sema);
	/* check mcu rx ready is high, mcu reboot */
	if (gpio_get_value(commu_data->mcu_read_ready_gpio) == 1 &&
	    gpio_get_value(commu_data->wakeup_ap_gpio) == 1) {
		pr_err("%s driver:%d wakeup_ap_gpio is high, mcu rebooting",
		       __func__, driver);
		return -EINVAL;
	}

	return 0;
}

static void commu_wakeup_read(struct commu_data *commu_data)
{
	unsigned int delay_times = 0;
	int ret = -EFAULT;
	unsigned char *read_buf = NULL;
	u32 read_len = 0;
	enum commu_driver driver;
	bool need_decode = false;

	if (commu_read_check(commu_data) != 0)
		return;
	driver = commu_data->driver;
	mutex_lock(&commu_data->commu_op_lock);
	if (!commu_data->initialed)
		goto err;
	__pm_stay_awake(commu_data->wake_lock);
	if (!commu_data->resumed) {
		pr_err("%s driver:%d is not resumed", __func__, driver);
		/* sleep 10 ms wait for next read */
		msleep(10);
		goto err;
	}
	if (gpio_get_value(commu_data->wakeup_ap_gpio) != 1) {
		pr_err("%s driver:%d wakeup_ap_gpio is not high",
		       __func__, driver);
		ret = -DMA_READY_TIMEOUT_ERRNO;
		goto err;
	}
	/* for driver do some setting */
	if (commu_data->commu_ops.op_read_ready) {
		ret = commu_data->commu_ops.op_read_ready(true);
		if (ret < 0) {
			ret = -DMA_READY_TIMEOUT_ERRNO;
			goto err;
		}
	}
	gpio_direction_output(commu_data->ap_read_ready_gpio, 1);
	/* wait for, uart driver send and then pull down, do not need check */
	if (commu_data->driver == SPI_DRIVER) {
		while (gpio_get_value(commu_data->wakeup_ap_gpio) == 1) {
			if (!commu_data->initialed)
				goto err;
			if (delay_times++ > COMMU_READ_MAX_DELAY_TIMES) {
				pr_err("%s driver: %d delay times is out of range",
				       __func__, driver);
				goto err;
			}
			udelay(COMMU_READ_DELAY_TIME_US);
		}
	}
	ret = commu_data->commu_ops.op_read_count(&read_len);
	if (ret != 0 || read_len == 0) {
		pr_err("%s driver:%d get read count:%d, error %d",
		       __func__, driver, read_len, ret);
		goto err;
	}
	/* buffer has one more byte for spi driver */
	read_buf = kmalloc(read_len + 1, GFP_KERNEL);
	if (!read_buf)
		goto err;
	/* Execute Read Operation, initial already */
	ret = commu_data->commu_ops.op_commu_read(read_buf, read_len);
	if (ret < 0) {
		pr_err("%s driver:%d read from driver error, ret:%d\n",
		       __func__, driver, ret);
		goto err;
	}
	need_decode = true;
err:
	if (commu_data->commu_ops.op_read_ready)
		commu_data->commu_ops.op_read_ready(false);
	gpio_direction_output(commu_data->ap_read_ready_gpio, 0);
	mutex_unlock(&commu_data->commu_op_lock);
	if (need_decode) {
		ret = decode_frame_resp(read_buf, read_len, commu_data->driver);
		if (ret < 0)
			pr_err("%s driver:%d decode resp error: %d",
			       __func__, driver, ret);
	}
	kfree(read_buf);
	__pm_relax(commu_data->wake_lock);
	if (ret == -DMA_READY_TIMEOUT_ERRNO) {
		__pm_wakeup_event(commu_data->wake_lock,
				  DMA_READY_WAKE_LOCK_TIME_MS);
		pr_warn("%s driver:%d wake lock for:%d ms",
			__func__, driver, DMA_READY_WAKE_LOCK_TIME_MS);
	}
	read_buf = NULL;
}

static void commu_read_worker(struct kthread_work *work)
{
	struct commu_data *commu_data = container_of(work, struct commu_data,
		read_work);

	commu_wakeup_read(commu_data);
}

static void commu_wakeup_read_work(struct work_struct *work)
{
	struct commu_data *commu_data = container_of(work, struct commu_data,
		wakeup_read_work);

	commu_wakeup_read(commu_data);
}

static int check_write_gpio(struct commu_data *commu_data, enum commu_driver driver)
{
	int i;
	unsigned int write_retry_times = 0;
	unsigned int wait_retry_times = 0;

	/* max retry 2 times */
	for (i = 0 ; i <= 2 * COMMU_MAX_RETRY_TIMES; ++i) {
		mutex_lock(&commu_data->commu_op_lock);
		if (!commu_data->initialed)
			goto err;

		if (gpio_get_value(commu_data->mcu_read_ready_gpio) == 1) {
			write_retry_times++;
			mutex_unlock(&commu_data->commu_op_lock);
			usleep_range(COMMU_RETRY_INTERVAL_US, COMMU_RETRY_INTERVAL_US + 1);
			if (write_retry_times <= COMMU_MAX_RETRY_TIMES) {
				continue;
			} else {
				pr_err("%s driver:%d last package is writing\n", __func__, driver);
				return -ETIME;
			}
		}

		if (gpio_get_value(commu_data->wakeup_ap_gpio) == 1) {
			gpio_direction_output(commu_data->wakeup_mcu_gpio, 0);
			mutex_unlock(&commu_data->commu_op_lock);
			usleep_range(COMMU_RETRY_INTERVAL_US, COMMU_RETRY_INTERVAL_US + 1);
			wait_retry_times++;
			if (wait_retry_times <= COMMU_MAX_RETRY_TIMES) {
				continue;
			} else {
				pr_err("%s driver:%d bus compete times is out of range\n",
				       __func__, driver);
				return -ETIME;
			}
		}
		break;
	}
	return 0;
err:
	if (commu_data->commu_ops.op_write_ready)
		commu_data->commu_ops.op_write_ready(NULL, 0);
	gpio_direction_output(commu_data->wakeup_mcu_gpio, 0);
	mutex_unlock(&commu_data->commu_op_lock);
	return -EREMOTEIO;
}

ssize_t commu_write(struct commu_data *commu_data, unsigned char *buf, size_t len)
{
	ssize_t status;
	size_t send_len;
	enum commu_driver driver;
	unsigned int delay_times = 0;

	if (!commu_data)
		return -EINVAL;
	driver = commu_data->driver;
	if (commu_data->commu_direct_mode) {
		pr_err("%s driver:%d is on direct mode\n", __func__, driver);
		return -EBUSY;
	}

	/* send len may > data len,but must < FRAME_BUF_LEN(buf contains '0') */
	send_len = commu_data->commu_ops.op_write_count(len);
	if (send_len == 0)
		return -EBUSY;

	status = check_write_gpio(commu_data, driver);
	/* if ret >= 0, mutex has locked */
	if (status < 0)
		return status;

	if (commu_data->commu_ops.op_write_ready)
		commu_data->commu_ops.op_write_ready(buf, len);
	if (!commu_data->resumed) {
		pr_err("%s driver:%d is not resumed", __func__, driver);
		status = -EACCES;
		goto err;
	}
	gpio_direction_output(commu_data->wakeup_mcu_gpio, 1);
	if (commu_data->driver != UART_DRIVER) {
		while (gpio_get_value(commu_data->mcu_read_ready_gpio) != 1) {
			if (!commu_data->initialed) {
				status = -EREMOTEIO;
				goto err;
			}
			delay_times++;
			if (delay_times > COMMU_WRITE_MAX_DELAY_TIMES) {
				status = -ETIME;
				pr_err("%s driver:%d delay times is out of range\n",
				       __func__, driver);
				goto err;
			}
			udelay(COMMU_WRITE_DELAY_TIME_US);
		}
	}
	/* write to driver */
	status = commu_data->commu_ops.op_commu_write(buf, send_len);
err:
	if (commu_data->commu_ops.op_write_ready)
		commu_data->commu_ops.op_write_ready(NULL, 0);
	gpio_direction_output(commu_data->wakeup_mcu_gpio, 0);
	mutex_unlock(&commu_data->commu_op_lock);
	return status;
}

void commu_suspend(enum commu_driver driver)
{
	commu_data_tbl[driver]->resumed = false;
	if (down_interruptible(&commu_data_tbl[driver]->sr_wake_sema))
		pr_err("%s down sr wake sema error", __func__);
	notify_commu_wakeup(false, driver);
	if (ap_status_gpio != -1) {
		gpio_direction_output(ap_status_gpio, 0);
		pr_info("%s ap status notify suspend", __func__);
	}
}

void commu_resume(enum commu_driver driver)
{
	if (commu_data_tbl[driver] && commu_data_tbl[driver]->wake_irq == pm_wakeup_irq)
		notify_commu_wakeup(true, driver);

	commu_data_tbl[driver]->resumed = true;
	up(&commu_data_tbl[driver]->sr_wake_sema);
#ifdef CONFIG_DEEPSLEEP
	if ((mem_sleep_current == PM_SUSPEND_MEM) && (driver == SPI_SLAVE_DRIVER)) {
		long long big_value = 0;
		send_frame_data(TRANSACTION_ID_AP_TO_MCU, SERVICE_ID_COMMU,
				SERVICE_MCU_HANDSHAKE_REQ,
				(unsigned char *)&big_value, sizeof(big_value));
	}
#endif
	if (ap_status_gpio != -1) {
		gpio_direction_output(ap_status_gpio, 1);
		pr_info("%s ap status notify resume", __func__);
	}
}

#ifdef CONFIG_EXT_INPUTHUB_QCOM
static bool check_resume_irq(struct commu_data *commu_data)
{
	return false;
}
#else
static bool check_resume_irq(struct commu_data *commu_data)
{
	unsigned int offset;
	bool ret = false;
	unsigned int data;

	offset = commu_data->wakeup_ap_gpio % AO_GPIO_GROUP_STEP;
	data = readl(commu_data->ao_gpio_base + SOC_GPIO_GPIOIE_ADDR(0));
	data = data & readl(commu_data->ao_gpio_base +
		SOC_GPIO_GPIOMIS_ADDR(0));
	ret = (data & BIT((u32)offset)) != 0;
	pr_info("%s pending result: %d\n", __func__, ret);
	return ret;
}
#endif

static void record_commu_wakeup(void)
{
	int i;

	for (i = 0; i < COMMU_DRIVER_COUNT; ++i) {
		if (commu_data_tbl[i] && check_resume_irq(commu_data_tbl[i])) {
			pr_info("will record for dubai");
			notify_commu_wakeup(true, i);
			return;
		}
	}
}

static int record_commu_suspend(void)
{
	return 0;
}

static struct syscore_ops dubai_pm_syscore_ops = {
	.resume = record_commu_wakeup,
	.suspend = record_commu_suspend,
};

static void read_work_init_task(struct commu_data *commu_data)
{
	int ret;
	struct sched_param param = { .sched_priority = COMMU_SCHED_PRIORITY };

	kthread_init_worker(&commu_data->read_kworker);
	commu_data->read_kworker_task = kthread_run(
		kthread_worker_fn, &commu_data->read_kworker,
		"%s %d", "read_worker",
		commu_data->driver);
	if (IS_ERR(commu_data->read_kworker_task)) {
		pr_err("failed to create read work task");
		return;
	}
	kthread_init_work(&commu_data->read_work, commu_read_worker);

	ret = sched_setscheduler(commu_data->read_kworker_task,
				 SCHED_FIFO, &param);
	pr_info("%s get init sched_setscheduler ret:%d", __func__, ret);
}

int commu_init(struct device *dev, struct commu_data *commu_data)
{
	int status;

	sema_init(&commu_data->sr_wake_sema, 1);
	commu_data->initialed = false;
	commu_data->commu_direct_mode = 0;
	commu_data->resumed = true;
	pr_info("%s begin", __func__);
	mutex_init(&commu_data->commu_op_lock);

	status = commu_request_gpio(dev, commu_data);
	if (status < 0)
		pr_err("%s request gpio error, status: %d", __func__, status);

	INIT_WORK(&commu_data->wakeup_read_work, commu_wakeup_read_work);
	read_work_init_task(commu_data);
	status = commu_request_irq(dev, commu_data);
	if (status < 0)
		pr_err("%s request irq error, status: %d", __func__, status);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
	commu_data->wake_lock = wakeup_source_register(NULL, "mcu_commu");
	commu_data->user_wake_lock = wakeup_source_register(NULL, "mcu_user_commu");
#else
	commu_data->wake_lock = wakeup_source_register("mcu_commu");
	commu_data->user_wake_lock = wakeup_source_register("mcu_user_commu");
#endif
	status = hisi_get_gpio_base(commu_data);
	if (status < 0)
		return -EFAULT;
	/* driver probe */
	status = commu_data->commu_ops.op_commu_init(commu_data);
	if (status < 0)
		pr_err("%s driver probe error, status: %d", __func__, status);

	pr_info("%s end , status:%d\n", __func__, status);

	return status;
}

void commu_exit(struct commu_data *commu_data)
{
	commu_data->commu_ops.op_commu_exit(commu_data);
	/* stop qpio irq */
	free_irq(commu_data->wake_irq, commu_data);

	if (commu_data->initialed)
		/* stop read work */
		cancel_work_sync(&commu_data->wakeup_read_work);

	/* gpio release */
	commu_release_gpio(commu_data);
}

/* for global init */
void ext_sensorhub_commu_init(void)
{
	int i;
	int ret;

	/* init NULL */
	for (i = 0; i < COMMU_DRIVER_COUNT; ++i)
		commu_data_tbl[i] = NULL;

	/* Get ap status GPIO */
	ret = commu_request_common_gpio(&ap_status_gpio,
					"ap,status", "ap_status");
	if (ret < 0) {
		pr_err("%s: ap,status node not found", __func__);
		ap_status_gpio = -1;
	} else {
		pr_info("%s: request ap,status gpio success", __func__);
		gpio_direction_output(ap_status_gpio, 1);
	}

	register_syscore_ops(&dubai_pm_syscore_ops);
}

/* for global exit */
void ext_sensorhub_commu_exit(void)
{
	unregister_syscore_ops(&dubai_pm_syscore_ops);
}

struct commu_driver_init_list {
	struct list_head list;
	enum commu_driver driver;
	commu_driver_init init_func;
};

static LIST_HEAD(init_list_head);

void register_commu_driver(enum commu_driver driver,
			   commu_driver_init init_func)
{
	struct commu_driver_init_list *init_node = NULL;

	if (!init_func)
		return;

	init_node = kmalloc(sizeof(*init_node), GFP_KERNEL);
	if (!init_node)
		return;

	INIT_LIST_HEAD(&init_node->list);
	init_node->driver = driver;
	init_node->init_func = init_func;

	list_add_tail(&init_node->list, &init_list_head);
}

static bool init_commu_data(enum commu_driver driver, commu_driver_init init_func)
{
	struct commu_data *commu_data = NULL;

	if (commu_data_tbl[driver])
		return true;

	if (!init_func) {
		pr_err("%s driver:%d init_func in empty", __func__, driver);
		return false;
	}
	/* free after used */
	commu_data = kmalloc(sizeof(*commu_data), GFP_KERNEL);
	if (!commu_data)
		return false;

	commu_data->commu_ops.op_read_ready = NULL;
	commu_data->commu_ops.op_write_ready = NULL;

	init_func(commu_data);
	if (driver == UART_DRIVER) {
		commu_data->wakeup_ap = "bt_wakeup_ap";
		commu_data->wakeup_mcu = "ap_wakeup_bt";
		commu_data->ap_read_ready = "ap_read_bt_sync";
		commu_data->mcu_read_ready = "ap_write_bt_sync";
	} else {
		commu_data->wakeup_ap = "mcu_wakeup_ap";
		commu_data->wakeup_mcu = "ap_wakeup_mcu";
		commu_data->ap_read_ready = "ap_read_sync";
		commu_data->mcu_read_ready = "ap_write_sync";
	}

	commu_data_tbl[driver] = commu_data;
	return true;
}

static void free_commu_init_list(void)
{
	struct commu_driver_init_list *buf_node = NULL;
	struct commu_driver_init_list *tmp_node = NULL;

	list_for_each_entry_safe(buf_node, tmp_node,
				 &init_list_head, list) {
		list_del(&buf_node->list);
		kfree(buf_node);
		buf_node = NULL;
	}
}

static void init_commu_driver(void)
{
	struct commu_driver_init_list *buf_node = NULL;
	struct commu_driver_init_list *tmp_node = NULL;

	static int once;
	if (once)
		return;

	once = 1;
	list_for_each_entry_safe(buf_node, tmp_node,
				 &init_list_head, list) {
		if (!init_commu_data(buf_node->driver, buf_node->init_func))
			once = 0;
	}

	free_commu_init_list();
}

struct commu_data *get_commu_data(enum commu_driver driver)
{
	init_commu_driver();

	return commu_data_tbl[driver];
}
