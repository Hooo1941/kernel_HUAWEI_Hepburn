/*
 * ext_sensorhub_spi_slave.c
 *
 * source file for external sensorhub spi slave driver
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
#include <linux/delay.h>
#include <linux/module.h>
#include "ext_sensorhub_commu.h"
#include "ext_sensorhub_spi.h"
#include "spi_slave_commu.h"
#include <linux/suspend.h>

void set_spi_xfer_timeout(unsigned long jiffies);

static struct spi_device *g_spidev;
static bool g_spi_resume;
static bool g_spi_slave_read;
static bool g_spi_slave_write;
static bool g_spi_probed;

int ext_sensorhub_spi_write(u8 *buf, u32 len)
{
	struct spi_transfer xfer = {
		.tx_buf = buf,
		.len = len,
		.rx_buf = NULL,
		.speed_hz = SPI_SPEED_DEFAULT,
	};
	struct spi_device *spidev = g_spidev;
	struct spi_message m;
	int ret;

	if (!g_spi_resume || !spidev) {
		pr_err("%s spi is in suspend or not initial\n", __func__);
		return -EACCES;
	}
	spi_message_init(&m);
	spi_message_add_tail(&xfer, &m);
	spidev->mode = (SPI_MODE_1 | SPI_CS_HIGH);
	/* 32 bits */
	spidev->bits_per_word = 32;
	g_spi_slave_write = true;
	ret = spi_sync(spidev, &m);
	g_spi_slave_write = false;
	if (ret != 0)
		pr_err("%s spi write len:%d ret:%d\n", __func__, len, ret);
	return ret;
}

int ext_sensorhub_spi_read(u8 *buf, u32 len)
{
	struct spi_transfer xfer = {
		.len = len,
		.rx_buf = buf,
		.speed_hz = SPI_SPEED_DEFAULT,
	};
	struct spi_device *spidev = g_spidev;
	struct spi_message m;
	int ret;

	if (!g_spi_resume || !spidev) {
		pr_err("%s spi is in suspend or not initial\n", __func__);
		return -EACCES;
	}
	spi_message_init(&m);
	spi_message_add_tail(&xfer, &m);
	spidev->mode = (SPI_MODE_1 | SPI_CS_HIGH);
	/* 32 bits */
	spidev->bits_per_word = 32;
	g_spi_slave_read = true;
	ret = spi_sync(spidev, &m);
	g_spi_slave_read = false;
	if (ret != 0)
		pr_err("%s spi read len:%d ret:%d\n", __func__, len, ret);
	return ret;
}

bool ext_sensorhub_spi_has_probed(void)
{
	return g_spi_probed;
}

static int ext_sensorhub_spi_probe(struct spi_device *spi_dev)
{
	pr_info("%s:get in", __func__);
	if (!spi_dev) {
		pr_err("%s:get spi_dev error", __func__);
		return -ENODEV;
	}

	g_spidev = spi_dev;
	g_spi_resume = true;
	g_spi_probed = true;
	return 0;
}

static int ext_sensorhub_spi_remove(struct spi_device *spi_dev)
{
	pr_info("%s get in", __func__);
	g_spi_resume = false;
	g_spidev = NULL;
	return 0;
}

static const struct of_device_id g_psoc_match_table[] = {
	{ .compatible = "huawei,ext_sensorhub", },
	{ },
};

static const struct spi_device_id g_device_id[] = {
	{ EXT_SENSORHUB_SPI_DEVICE_NAME, 0 },
	{ }
};
MODULE_DEVICE_TABLE(spi, g_device_id);

bool check_spi_slave_resume(void)
{
	return g_spi_resume;
}
EXPORT_SYMBOL(check_spi_slave_resume);

static int ext_sensorhub_spi_suspend(struct device *dev)
{
	pr_info("%s get in", __func__);
	g_spi_resume = false;
	commu_suspend(SPI_SLAVE_DRIVER);
	return 0;
}

static int ext_sensorhub_spi_resume(struct device *dev)
{
	pr_info("%s get in", __func__);
	g_spi_resume = true;
#ifdef CONFIG_DEEPSLEEP
	if (mem_sleep_current == PM_SUSPEND_MEM) {
		pr_info("%s exit from deep, set_spi_xfer_timeout 2ms", __func__);
		/* just set 2ms timed out for quick timeout of lacking bits when exit fsr */
		set_spi_xfer_timeout(msecs_to_jiffies(2));
	}
#endif
	commu_resume(SPI_SLAVE_DRIVER);
	return 0;
}

static const struct dev_pm_ops spi_dev_pm_ops = {
	.suspend = ext_sensorhub_spi_suspend,
	.resume = ext_sensorhub_spi_resume,
};

static struct spi_driver g_spi_driver = {
	.probe = ext_sensorhub_spi_probe,
	.remove = ext_sensorhub_spi_remove,
	.id_table = g_device_id,
	.driver = {
		.name = EXT_SENSORHUB_SPI_DEVICE_NAME,
		.owner = THIS_MODULE,
		.bus = &spi_bus_type,
		.of_match_table = g_psoc_match_table,
		.pm = &spi_dev_pm_ops,
	},
};

int ext_sensorhub_spi_init(void)
{
	int rc;

	pr_info("%s get in", __func__);
	rc = spi_register_driver(&g_spi_driver);
	if (rc < 0) {
		pr_err("%s register spi driver error: %d", __func__, rc);
		return rc;
	}

	pr_info("%s success", __func__);
	return 0;
}

void ext_sensorhub_spi_exit(void)
{
	pr_info("%s get in", __func__);
	spi_unregister_driver(&g_spi_driver);
}

void ext_sensosrhub_slave_state(struct spi_device *slv, bool slave_state)
{
	if (g_spidev != slv) {
		pr_warn("%s get in spi_device not match", __func__);
		return;
	}

	if (g_spi_slave_read)
		commu_spi_slave_ready(SLAVE_READ);

	if (g_spi_slave_write)
		commu_spi_slave_ready(SLAVE_WRITE);
}
EXPORT_SYMBOL(ext_sensosrhub_slave_state);
