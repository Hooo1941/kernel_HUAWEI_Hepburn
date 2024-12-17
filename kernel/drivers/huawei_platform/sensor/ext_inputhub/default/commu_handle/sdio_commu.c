/*
 * sdio_commu.c
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

#include "sdio_commu.h"

#include <linux/of_gpio.h>
#include "securec.h"
#include "ext_sensorhub_sdio.h"
#include "ext_sensorhub_frame.h"

#define COMMU_ALIGN_BYTES_COUNT 4

static void commu_init_work(struct work_struct *work)
{
	int ret;
	struct commu_data *commu_data = container_of(work, struct commu_data,
		init_work);

	pr_debug("%s begin to do communication init work check\n", __func__);
	if (!commu_data) {
		pr_err("%s commu data NULL\n", __func__);
		return;
	}

	/* ap is writing, not a initial work */
	if (commu_data->initialed ||
	    gpio_get_value(commu_data->wakeup_mcu_gpio) == 1)
		return;

	pr_info("%s is initial work detect sdio card\n", __func__);
	__pm_stay_awake(commu_data->wake_lock);
	/* wait 12 HZ timeout for resume */
	ret = down_timeout(&commu_data->sr_wake_sema, 12 * HZ);
	if (ret == -ETIME) {
		pr_err("%s down sr wake sema timeout", __func__);
		__pm_relax(commu_data->wake_lock);
		return;
	}
	up(&commu_data->sr_wake_sema);
	ext_sensorhub_detect_sdio_card();
	commu_data->initialed = true;
	__pm_relax(commu_data->wake_lock);
}

static int sdio_commu_init(struct commu_data *commu_data)
{
	int status;

	if (!commu_data)
		return -EINVAL;

	INIT_WORK(&commu_data->init_work, commu_init_work);
	status = ext_sensorhub_sdio_func_probe();

	return status;
}

static void sdio_commu_exit(struct commu_data *commu_data)
{
	if (!commu_data)
		return;

	ext_sensorhub_sdio_exit();
	cancel_work_sync(&commu_data->init_work);
}

static size_t sdio_write_count(size_t len)
{
	size_t send_len;
	size_t align_factor = COMMU_ALIGN_BYTES_COUNT;

	// 512 bytes
	if (len >= HISDIO_BLOCK_SIZE)
		align_factor = HISDIO_BLOCK_SIZE;

	if (len % align_factor == 0)
		send_len = len;
	else
		send_len = (len / align_factor + 1) * align_factor;

	return send_len;
}

static irqreturn_t sdio_commu_wake_irq(int irq, void *arg)
{
	struct commu_data *commu_data = (struct commu_data *)arg;

	if (!commu_data)
		return IRQ_HANDLED;

	/* check mcu wakeup ap is high, mcu reboot, clear init flag */
	if (gpio_get_value(commu_data->mcu_read_ready_gpio) == 1 &&
	    gpio_get_value(commu_data->wakeup_ap_gpio) == 1) {
		pr_err("%s mcu is rebooting, clear init flag", __func__);
		commu_data->initialed = false;
		notify_mcu_reboot(commu_data->driver);
		return IRQ_HANDLED;
	}

	highpri_work(&commu_data->wakeup_read_work);
	return IRQ_HANDLED;
}

static irqreturn_t sdio_commu_rx_irq(int irq, void *arg)
{
	struct commu_data *commu_data = (struct commu_data *)arg;

	if (!commu_data)
		return IRQ_HANDLED;

	if (gpio_get_value(commu_data->mcu_read_ready_gpio) == 1)
		schedule_work(&commu_data->init_work);
	return IRQ_HANDLED;
}

static void commu_data_sdio_init(struct commu_data *commu_data)
{
	if (!commu_data)
		return;

	commu_data->commu_ops.op_commu_init = sdio_commu_init;
	commu_data->commu_ops.op_commu_exit = sdio_commu_exit;
	commu_data->commu_ops.op_read_count =
	ext_sensorhub_mmc_sdio_get_xfercount;
	commu_data->commu_ops.op_commu_read =
	ext_sensorhub_mmc_sdio_dev_read;
	commu_data->commu_ops.op_write_count = sdio_write_count;
	commu_data->commu_ops.op_commu_write =
	ext_sensorhub_mmc_sdio_dev_write;
	commu_data->commu_ops.op_commu_rx_irq = sdio_commu_rx_irq;
	commu_data->commu_ops.op_commu_wake_irq = sdio_commu_wake_irq;

	commu_data->driver = SDIO_DRIVER;
	commu_data->active_handshake = 0;
}

static int __init register_sdio_commu(void)
{
	register_commu_driver(SDIO_DRIVER, commu_data_sdio_init);
	pr_info("%s call in", __func__);
	return 0;
}

fs_initcall(register_sdio_commu);
