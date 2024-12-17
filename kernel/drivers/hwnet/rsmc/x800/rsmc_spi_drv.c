/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
 * Description: The SPI peripherals are abstracted into four operations:
 * starting and stopping the SPI, and reading and writing the SPI.
 * This module is controlled by other modules..
 * Author: zhongjilei@huawei.com
 * Create: 2020-10-22
 */

#include "rsmc_spi_drv.h"

#include <huawei_platform/log/hw_log.h>
#include <securec.h>
#include <linux/cdev.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/gpio.h>
#include <linux/gpio/driver.h>
#include <linux/gpio/machine.h>
#include <linux/hwspinlock.h>
#include <linux/irq.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/spi/spi.h>
#include <linux/spinlock.h>
#include <uapi/linux/gpio.h>

#include "rsmc_msg_loop.h"

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif
#define HWLOG_TAG RSMC_SPI_DRV
HWLOG_REGIST();

#define RSMC_DEVICE_NAME "huawei_rsmc"
#define RSMC_MISC_DEVICE_NAME "rsmc"
#define TCXO_GPIO_TRY_MAX_COUNT 300
#define TCXO_GPIO_SLEEP_INTERVAL 100

static struct smc_core_data *g_smc_core = NULL;

struct smc_core_data *smc_get_core_data(void)
{
	return g_smc_core;
}

static int smc_setup_chip_pwr_gpio(struct smc_core_data *cd)
{
	int rc;

	if (cd == NULL)
		return -EINVAL;
	if (gpio_is_valid(cd->gpios.chip_pwr_gpio)) {
		rc = gpio_request(cd->gpios.chip_pwr_gpio, "smc_pwr");
		if (rc) {
			hwlog_err("%s: pwr gpio_request %d failed %d\n",
				__func__, cd->gpios.chip_pwr_gpio, rc);
			return rc;
		}

		rc = gpio_get_value(cd->gpios.chip_pwr_gpio);
		hwlog_info("%s: chip_pwr_gpio %d value %d",
			__func__, cd->gpios.chip_pwr_gpio, rc);

		rc = gpio_direction_output(cd->gpios.chip_pwr_gpio, GPIO_LOW);
		if (rc) {
			hwlog_err("%s: chip pwr gpio %d output high err %d",
				__func__, cd->gpios.chip_pwr_gpio, rc);
			return rc;
		}

		rc = gpio_get_value(cd->gpios.chip_pwr_gpio);
		hwlog_info("%s: chip_pwr_gpio %d value %d",
			__func__, cd->gpios.chip_pwr_gpio, rc);
	}
	return 0;
}

static int smc_setup_init_gpio(struct smc_core_data *cd)
{
	int rc;

	if (cd == NULL)
		return -EINVAL;
	if (gpio_is_valid(cd->gpios.init_gpio)) {
		rc = gpio_request(cd->gpios.init_gpio, "smc_init");
		if (rc) {
			hwlog_err("%s: int gpio %d request failed %d\n",
				__func__, cd->gpios.init_gpio, rc);
			return rc;
		}

		rc = gpio_get_value(cd->gpios.init_gpio);
		hwlog_info("%s: init_gpio %d value %d",
			__func__, cd->gpios.init_gpio, rc);

		rc = gpio_direction_output(cd->gpios.init_gpio, GPIO_LOW);
		if (rc) {
			hwlog_err("%s: chip init gpio %d output high err %d",
				__func__, cd->gpios.init_gpio, rc);
			return rc;
		}

		rc = gpio_get_value(cd->gpios.init_gpio);
		hwlog_info("%s: init_gpio %d value %d",
			__func__, cd->gpios.init_gpio, rc);
	}
	return 0;
}

static int smc_setup_reset_gpio(struct smc_core_data *cd)
{
	int rc;

	if (cd == NULL)
		return -EINVAL;
	if (gpio_is_valid(cd->gpios.reset_gpio)) {
		rc = gpio_request(cd->gpios.reset_gpio, "smc_reset");
		if (rc) {
			hwlog_err("%s: rst gpio_request %d failed %d\n",
				__func__, cd->gpios.reset_gpio, rc);
			return rc;
		}

		rc = gpio_get_value(cd->gpios.reset_gpio);
		hwlog_info("%s: reset_gpio %d value %d",
			__func__, cd->gpios.reset_gpio, rc);

		rc = gpio_direction_output(cd->gpios.reset_gpio, GPIO_LOW);
		if (rc) {
			hwlog_err("%s: reset_gpio %d output high err %d",
				__func__, cd->gpios.reset_gpio, rc);
			return rc;
		}

		rc = gpio_get_value(cd->gpios.reset_gpio);
		hwlog_info("%s: reset_gpio %d value %d",
			__func__, cd->gpios.reset_gpio, rc);
	}
	return 0;
}

static int smc_setup_cs_gpio(struct smc_core_data *cd)
{
	int rc;

	if (cd == NULL)
		return -EINVAL;
	if (gpio_is_valid(cd->gpios.cs_gpio)) {
		rc = gpio_request(cd->gpios.cs_gpio, "smc_cs");
		if (rc) {
			hwlog_err("%s: cs gpio_request %d failed %d\n",
				__func__, cd->gpios.cs_gpio, rc);
			return rc;
		}

		rc = gpio_get_value(cd->gpios.cs_gpio);
		hwlog_info("%s: cs_gpio %d value %d",
			__func__, cd->gpios.cs_gpio, rc);

		rc = gpio_direction_output(cd->gpios.cs_gpio, SSP_CHIP_DESELECT);
		if (rc) {
			hwlog_err("%s: cs_gpio %d output high err %d",
				__func__, cd->gpios.cs_gpio, rc);
			return rc;
		}

		rc = gpio_get_value(cd->gpios.cs_gpio);
		hwlog_info("%s: cs_gpio %d value %d",
			__func__, cd->gpios.cs_gpio, rc);
	}
	rc = gpio_direction_output(cd->gpios.cs_gpio, SSP_CHIP_DESELECT);
	return 0;
}

static int smc_setup_tx_ant_gpio(struct smc_core_data *cd)
{
	int rc;

	if (cd == NULL)
		return -EINVAL;
	if (gpio_is_valid(cd->gpios.tx_ant_gpio)) {
		rc = gpio_request(cd->gpios.tx_ant_gpio, "smc_tx_ant");
		if (rc) {
			hwlog_err("%s: ant tx gpio %d request failed %d\n",
				__func__, cd->gpios.tx_ant_gpio, rc);
			return rc;
		}

		rc = gpio_get_value(cd->gpios.tx_ant_gpio);
		hwlog_info("%s: tx_ant_gpio %d value %d",
			__func__, cd->gpios.tx_ant_gpio, rc);

		rc = gpio_direction_output(cd->gpios.tx_ant_gpio, GPIO_LOW);
		if (rc) {
			hwlog_err("%s: tx_ant_gpio %d output low err %d",
				__func__, cd->gpios.tx_ant_gpio, rc);
			return rc;
		}

		rc = gpio_get_value(cd->gpios.tx_ant_gpio);
		hwlog_info("%s: tx_ant_gpio %d value %d",
			__func__, cd->gpios.tx_ant_gpio, rc);
	}
	return 0;
}

static int smc_setup_rx_ant_gpio(struct smc_core_data *cd)
{
	int rc;

	if (cd == NULL)
		return -EINVAL;
	if (gpio_is_valid(cd->gpios.rx_ant_gpio)) {
		rc = gpio_request(cd->gpios.rx_ant_gpio, "smc_rx_ant");
		if (rc) {
			hwlog_err("%s: ant rx gpio %d request failed %d\n",
				__func__, cd->gpios.rx_ant_gpio, rc);
			return rc;
		}

		rc = gpio_get_value(cd->gpios.rx_ant_gpio);
		hwlog_info("%s: rx_ant_gpio %d value %d",
			__func__, cd->gpios.rx_ant_gpio, rc);

		rc = gpio_direction_output(cd->gpios.rx_ant_gpio, GPIO_LOW);
		if (rc) {
			hwlog_err("%s: rx_ant_gpio %d output high err %d",
				__func__, cd->gpios.rx_ant_gpio, rc);
			return rc;
		}

		rc = gpio_get_value(cd->gpios.rx_ant_gpio);
		hwlog_info("%s: rx_ant_gpio %d value %d",
			__func__, cd->gpios.rx_ant_gpio, rc);
	}
	return 0;
}

static int smc_setup_pa_pwr_gpio(struct smc_core_data *cd)
{
	int rc;

	if (cd == NULL)
		return -EINVAL;
	if (gpio_is_valid(cd->gpios.pa_pwr_gpio)) {
		rc = gpio_request(cd->gpios.pa_pwr_gpio, "smc_pa_pwr");
		if (rc) {
			hwlog_err("%s: pa pwr gpio %d request failed %d\n",
				__func__, cd->gpios.pa_pwr_gpio, rc);
			return rc;
		}

		rc = gpio_get_value(cd->gpios.pa_pwr_gpio);
		hwlog_info("%s: pa_pwr_gpio %d value %d",
			__func__, cd->gpios.pa_pwr_gpio, rc);

		rc = gpio_direction_output(cd->gpios.pa_pwr_gpio, GPIO_LOW);
		if (rc) {
			hwlog_err("%s: pa_pwr_gpio %d output high err %d",
				__func__, cd->gpios.pa_pwr_gpio, rc);
			return rc;
		}

		rc = gpio_get_value(cd->gpios.pa_pwr_gpio);
		hwlog_info("%s: pa_pwr_gpio %d value %d",
			__func__, cd->gpios.pa_pwr_gpio, rc);
	}
	return 0;
}

static int smc_setup_tcxo_pwr_gpio(struct smc_core_data *cd)
{
	int rc;
	int value;
	int count = 0;

	if (cd == NULL || cd->sdev == NULL)
		return -EINVAL;
	rc = gpio_is_valid(cd->gpios.tcxo_pwr_gpio);
	while (!rc && count < TCXO_GPIO_TRY_MAX_COUNT) {
		value = of_get_named_gpio(cd->sdev->dev.of_node, "tcxo_pwr_gpio", 0);
		cd->gpios.tcxo_pwr_gpio = value;
		msleep(TCXO_GPIO_SLEEP_INTERVAL);
		count++;
		rc = gpio_is_valid(cd->gpios.tcxo_pwr_gpio);
		hwlog_info("tcxo_pwr_gpio = %d,count=%d,rc=%d\n", value, count, rc);
	}
	if (!rc)
		return -EINVAL;
	rc = gpio_request(cd->gpios.tcxo_pwr_gpio, "smc_tcxo_pwr");
	if (rc) {
		hwlog_err("%s: tcxo pwr gpio %d request failed %d\n",
			__func__, cd->gpios.tcxo_pwr_gpio, rc);
		return rc;
	}

	rc = gpio_get_value(cd->gpios.tcxo_pwr_gpio);
	hwlog_info("%s: tcxo_pwr_gpio %d value %d",
		__func__, cd->gpios.tcxo_pwr_gpio, rc);

	rc = gpio_direction_output(cd->gpios.tcxo_pwr_gpio, GPIO_LOW);
	if (rc) {
		hwlog_err("%s: tcxo_pwr_gpio %d output low err %d",
			__func__, cd->gpios.tcxo_pwr_gpio, rc);
		return rc;
	}

	rc = gpio_get_value(cd->gpios.tcxo_pwr_gpio);
	hwlog_info("%s: tcxo_pwr_gpio %d value %d",
		__func__, cd->gpios.tcxo_pwr_gpio, rc);
	return 0;
}

static int smc_setup_gpio(struct smc_core_data *cd)
{
	int rc;

	if (cd == NULL)
		return -EINVAL;
	// chip pwr gpio
	rc = smc_setup_chip_pwr_gpio(cd);
	// init gpio
	rc = smc_setup_init_gpio(cd);
	// reset gpio
	rc = smc_setup_reset_gpio(cd);
	// cs gpio
	rc = smc_setup_cs_gpio(cd);
	// ant tx gpio
	rc = smc_setup_tx_ant_gpio(cd);
	// rx ant gpio
	rc = smc_setup_rx_ant_gpio(cd);
	// PA PWR
	rc = smc_setup_pa_pwr_gpio(cd);
	// TCXO PWR
	rc = smc_setup_tcxo_pwr_gpio(cd);
	return 0;
}

static void smc_free_gpio(struct smc_core_data *cd)
{
	if (cd == NULL)
		return;
	gpio_free(cd->gpios.chip_pwr_gpio);
	gpio_free(cd->gpios.init_gpio);
	gpio_free(cd->gpios.reset_gpio);
	gpio_free(cd->gpios.cs_gpio);
	gpio_free(cd->gpios.pa_pwr_gpio);
	gpio_free(cd->gpios.tx_ant_gpio);
	gpio_free(cd->gpios.rx_ant_gpio);
	gpio_free(cd->gpios.tcxo_pwr_gpio);
}

static int smc_setup_spi(struct smc_core_data *cd)
{
	int rc;

	hwlog_info("%s: enter", __func__);
	if (cd == NULL)
		return -EINVAL;
	cd->sdev->controller_data = &cd->spi_config.pl022_spi_config;
	cd->sdev->bits_per_word = cd->spi_config.bits_per_word;
	cd->sdev->mode = cd->spi_config.mode;
	cd->sdev->max_speed_hz = cd->spi_config.max_speed_hz;

	rc = spi_setup(cd->sdev);
	if (rc) {
		hwlog_err("%s: spi setup fail\n", __func__);
		return rc;
	}
	return 0;
}

static int smc_core_init(struct smc_core_data *cd)
{
	if (cd == NULL)
		return 0;
	dev_set_drvdata(&cd->sdev->dev, cd);
	atomic_set(&cd->register_flag, 1);
	return 0;
}

void rsmc_sync_read_write(struct spi_transfer *tf, int len)
{
	int index, rc;
	struct spi_message msg;
	struct smc_core_data *cd = smc_get_core_data();

	if (tf == NULL)
		return;
	spi_message_init(&msg);
	for (index = 0; index < len; index++) {
		tf[index].cs_change = 1;
		tf[index].bits_per_word = cd->spi_config.bits_per_word;
		spi_message_add_tail(&tf[index], &msg);
	}
	rc = spi_sync(cd->sdev, &msg);
	if (rc != 0)
		hwlog_err("%s: rc %d", __func__, rc);
}

int rsmc_sync_read(u32 addr, u32 *value)
{
	int rc;
	struct smc_core_data *cd = smc_get_core_data();

	if (value == NULL)
		return -EINVAL;
	mutex_lock(&cd->spi_mutex);

	cd->tx_buff[1] = 0x80 | addr;

	/* read data */
	rc = memset_s(&cd->t, sizeof(struct spi_transfer), 0, sizeof(struct spi_transfer));
	if (rc != EOK) {
		hwlog_err("%s: memset_s failed", __func__);
		return rc;
	}

	spi_message_init(&cd->msg);

	cd->t.rx_buf = cd->rx_buff;
	cd->t.tx_buf = cd->tx_buff;
	cd->t.len = sizeof(int);
	cd->t.cs_change = 1;
	cd->t.bits_per_word = cd->spi_config.bits_per_word;
	spi_message_add_tail(&cd->t, &cd->msg);

	rc = spi_sync(cd->sdev, &cd->msg);
	if (rc) {
		hwlog_err("%s: spi_sync %d\n", __func__, rc);
		msleep(SPI_MSLEEP_SHORT_TIME);
		rc = spi_sync(cd->sdev, &cd->msg);
		hwlog_err("%s: spi_sync %d\n", __func__, rc);
	}
	*value = (cd->rx_buff[0]<<16) |
		(cd->rx_buff[3]<<8) |
		cd->rx_buff[2];

	mutex_unlock(&cd->spi_mutex);

	return rc;
}

int rsmc_sync_write(u32 addr, u32 value)
{
	int rc;
	struct smc_core_data *cd = smc_get_core_data();

	mutex_lock(&cd->spi_mutex);

	/* set header */
	cd->tx_buff[1] = addr & 0x7F;
	cd->tx_buff[0] = (0xff & (value >> 16));
	cd->tx_buff[3] = (0xff & (value >> 8));
	cd->tx_buff[2] = (0xff & value);

	/* write data */
	rc = memset_s(&cd->t, sizeof(struct spi_transfer), 0x00, sizeof(struct spi_transfer));
	if (rc != EOK) {
		hwlog_err("%s: memset_s failed", __func__);
		return rc;
	}

	spi_message_init(&cd->msg);

	cd->t.tx_buf = cd->tx_buff;
	cd->t.rx_buf = cd->rx_buff;
	cd->t.len = sizeof(int);
	cd->t.cs_change = 1;
	cd->t.bits_per_word = cd->spi_config.bits_per_word;
	spi_message_add_tail(&cd->t, &cd->msg);

	rc = spi_sync(cd->sdev, &cd->msg);
	if (rc) {
		hwlog_err("%s: spi_sync %d\n", __func__, rc);
		msleep(SPI_MSLEEP_SHORT_TIME);
		rc = spi_sync(cd->sdev, &cd->msg);
		hwlog_err("%s: spi_sync %d\n", __func__, rc);
	}

	mutex_unlock(&cd->spi_mutex);

	return rc;
}

int rsmc_register_dev(struct smc_device *dev)
{
	int rc = -EINVAL;
	struct smc_core_data *cd = smc_get_core_data();

	hwlog_info("%s: enter", __func__);

	if (dev == NULL || cd == NULL) {
		hwlog_err("%s: input null", __func__);
		goto register_err;
	}
	/* check device configed ot not */
	if (atomic_read(&cd->register_flag)) {
		hwlog_err("%s: smc have registerd", __func__);
		goto register_err;
	}
	dev->smc_core = cd;
	dev->gpios = &cd->gpios;
	dev->sdev = cd->sdev;
	cd->smc_dev = dev;

	rc = smc_setup_gpio(cd);
	if (rc) {
		hwlog_err("%s: spi dev init fail", __func__);
		goto dev_init_err;
	}
	rc = smc_setup_spi(cd);
	if (rc) {
		hwlog_err("%s: spi dev init fail", __func__);
		goto err;
	}

	rc = smc_core_init(cd);
	if (rc) {
		hwlog_err("%s: core init", __func__);
		goto err;
	}
	return 0;
err:
	smc_free_gpio(cd);
dev_init_err:
	cd->smc_dev = 0;
register_err:
	return rc;
}

int rsmc_unregister_dev(struct smc_device *dev)
{
	struct smc_core_data *cd = smc_get_core_data();

	smc_free_gpio(cd);
	return 0;
}

int smc_parse_gpio_config(struct device_node *smc_node,
	struct smc_core_data *cd)
{
	int value, rc;

	if ((smc_node == NULL) || (cd == NULL)) {
		hwlog_info("%s: input null\n", __func__);
		return -ENODEV;
	}
	value = of_get_named_gpio(smc_node, "pwr_gpio", 0);
	hwlog_info("pwr_gpio = %d\n", value);
	cd->gpios.chip_pwr_gpio = (u32)value;

	value = of_get_named_gpio(smc_node, "init_gpio", 0);
	hwlog_info("init_gpio = %d\n", value);
	cd->gpios.init_gpio = (u32)value;

	value = of_get_named_gpio(smc_node, "rst_gpio", 0);
	hwlog_info("rst_gpio = %d\n", value);
	cd->gpios.reset_gpio = (u32)value;

	value = of_get_named_gpio(smc_node, "cs_gpio", 0);
	hwlog_info("cs_gpio = %d\n", value);
	cd->gpios.cs_gpio = (u32)value;

	value = of_get_named_gpio(smc_node, "pa_gpio", 0);
	hwlog_info("pa_gpio = %d\n", value);
	cd->gpios.pa_pwr_gpio = (u32)value;

	value = of_get_named_gpio(smc_node, "rx_ant_gpio", 0);
	hwlog_info("rx_ant_gpio = %d\n", value);
	cd->gpios.rx_ant_gpio = (u32)value;

	value = of_get_named_gpio(smc_node, "tx_ant_gpio", 0);
	hwlog_info("tx_ant_gpio = %d\n", value);
	cd->gpios.tx_ant_gpio = (u32)value;

	value = of_get_named_gpio(smc_node, "tcxo_pwr_gpio", 0);
	hwlog_info("tcxo_pwr_gpio = %d\n", value);
	cd->gpios.tcxo_pwr_gpio = (u32)value;

	rc = of_property_read_u32(smc_node, "tcxo_pwr_after_delay_ms", &value);
	if (!rc) {
		cd->gpios.tcxo_pwr_after_delay_ms = (u32)value;
		hwlog_info("%s: tcxo_pwr_after_delay_ms %d\n", __func__, value);
	}
	rc = of_property_read_u32(smc_node, "ant_gpio_delay_ms", &value);
	if (!rc) {
		cd->gpios.ant_gpio_delay_ms = (u32)value;
		hwlog_info("%s: ant_gpio_delay_ms %d\n", __func__, value);
	} else {
		cd->gpios.ant_gpio_delay_ms = ANT_DEFAULT_DELAY;
	}
	return 0;
}

int smc_parse_pl022_config(struct device_node *smc_node,
	struct smc_core_data *cd)
{
	int rc;
	u32 value;
	struct smc_spi_config *spi_config = NULL;
	struct pl022_config_chip *pl022_spi_config = NULL;

	if (smc_node == NULL || cd == NULL) {
		hwlog_info("%s: input null\n", __func__);
		return -ENODEV;
	}

	spi_config = &cd->spi_config;
	pl022_spi_config = &cd->spi_config.pl022_spi_config;

	rc = of_property_read_u32(smc_node, "pl022,interface", &value);
	if (!rc) {
		pl022_spi_config->iface = value;
		hwlog_info("%s: pl022,interface %d\n", __func__, value);
	}
	rc = of_property_read_u32(smc_node, "pl022,com-mode", &value);
	if (!rc) {
		pl022_spi_config->com_mode = value;
		hwlog_info("%s: pl022,com_mode %d\n", __func__, value);
	}
	rc = of_property_read_u32(smc_node, "pl022,rx-level-trig", &value);
	if (!rc) {
		pl022_spi_config->rx_lev_trig = value;
		hwlog_info("%s: pl022,rx-level-trig %d\n", __func__, value);
	}
	rc = of_property_read_u32(smc_node, "pl022,tx-level-trig", &value);
	if (!rc) {
		pl022_spi_config->tx_lev_trig = value;
		hwlog_info("%s: pl022,tx-level-trig %d\n", __func__, value);
	}
	rc = of_property_read_u32(smc_node, "pl022,ctrl-len", &value);
	if (!rc) {
		pl022_spi_config->ctrl_len = value;
		hwlog_info("%s: pl022,ctrl-len %d\n", __func__, value);
	}
	rc = of_property_read_u32(smc_node, "pl022,wait-state", &value);
	if (!rc) {
		pl022_spi_config->wait_state = value;
		hwlog_info("%s: pl022,wait-state %d\n", __func__, value);
	}
	rc = of_property_read_u32(smc_node, "pl022,duplex", &value);
	if (!rc) {
		pl022_spi_config->duplex = value;
		hwlog_info("%s: pl022,duplex %d\n", __func__, value);
	}
	return 0;
}

void smc_spi_cs_set(u32 control)
{
	struct smc_core_data *cd = smc_get_core_data();

	if (control)
		gpio_set_value(cd->gpios.cs_gpio, 1);
	else
		gpio_set_value(cd->gpios.cs_gpio, 0);
}

int smc_parse_spi_config(struct device_node *smc_node,
	struct smc_core_data *cd)
{
	int rc;
	u32 value;
	struct smc_spi_config *spi_config = NULL;
	struct pl022_config_chip *pl022_spi_config = NULL;

	if (smc_node == NULL || cd == NULL) {
		hwlog_info("%s: input null\n", __func__);
		return -ENODEV;
	}

	spi_config = &cd->spi_config;
	pl022_spi_config = &cd->spi_config.pl022_spi_config;

	rc = of_property_read_u32(smc_node, "spi-max-frequency", &value);
	if (!rc) {
		spi_config->max_speed_hz = value;
		hwlog_info("%s: spi-max-frequency configured %d\n",
				__func__, value);
	}
	rc = of_property_read_u32(smc_node, "spi-bus-id", &value);
	if (!rc) {
		spi_config->bus_id = (u8)value;
		hwlog_info("%s: spi-bus-id configured %d\n", __func__, value);
	}
	rc = of_property_read_u32(smc_node, "spi-mode", &value);
	if (!rc) {
		spi_config->mode = value;
		hwlog_info("%s: spi-mode configured %d\n", __func__, value);
	}
	rc = of_property_read_u32(smc_node, "bits-per-word", &value);
	if (!rc) {
		spi_config->bits_per_word = value;
		hwlog_info("%s: bits-per-word configured %d\n", __func__, value);
	}

	rc = smc_parse_pl022_config(smc_node, cd);

	cd->spi_config.pl022_spi_config.cs_control = smc_spi_cs_set;
	cd->spi_config.pl022_spi_config.hierarchy = SSP_MASTER;

	if (!cd->spi_config.max_speed_hz)
		cd->spi_config.max_speed_hz = SMC_SPI_SPEED_DEFAULT;
	if (!cd->spi_config.mode)
		cd->spi_config.mode = SPI_MODE_0;
	if (!cd->spi_config.bits_per_word)
		cd->spi_config.bits_per_word = SMC_SPI_DEFAULT_BITS_PER_WORD;

	cd->sdev->mode = spi_config->mode;
	cd->sdev->max_speed_hz = spi_config->max_speed_hz;
	cd->sdev->bits_per_word = spi_config->bits_per_word;
	cd->sdev->controller_data = &spi_config->pl022_spi_config;
	return 0;
}

int smc_parse_feature_config(struct device_node *node,
	struct smc_core_data *cd)
{
	u32 value = 0;
	int rc;

	if (node == NULL || cd == NULL) {
		hwlog_info("%s: input null\n", __func__);
		return -ENODEV;
	}

	rc = of_property_read_u32(node, "cpu-affinity-offset", &value);
	if (!rc) {
		cd->feature_config.cpu_affinity_offset = value;
		hwlog_info("%s: cpu_affinity_offset=%d\n", __func__, value);
	}
	rc = of_property_read_u32(node, "cpu-affinity-mask", &value);
	if (!rc) {
		cd->feature_config.cpu_affinity_mask = value;
		hwlog_info("%s: cpu_affinity_mask=%d\n", __func__, value);
	}
	return 0;
}

int smc_parse_config(struct device_node *smc_node,
	struct smc_core_data *cd)
{
	int ret;

	if ((smc_node == NULL) || (cd == NULL)) {
		hwlog_info("%s: input null\n", __func__);
		return -ENODEV;
	}

	ret = smc_parse_gpio_config(smc_node, cd);
	if (ret != 0)
		return ret;
	ret = smc_parse_spi_config(smc_node, cd);
	if (ret != 0)
		return ret;
	ret = smc_parse_feature_config(smc_node, cd);
	if (ret != 0)
		hwlog_info("%s: can not read feature config", __func__);

	cd->smc_node = smc_node;

	return 0;
}

static int smc_probe(struct spi_device *sdev)
{
	struct smc_core_data *cd = NULL;
	int rc;

	hwlog_info("%s: in\n", __func__);

	cd = kzalloc(sizeof(struct smc_core_data), GFP_KERNEL);
	if (cd == NULL)
		return -ENOMEM;
	cd->sdev = sdev;
	rc = smc_parse_config(sdev->dev.of_node, cd);
	if (rc) {
		hwlog_err("%s: parse dts fail\n", __func__);
		kfree(cd);
		return -ENODEV;
	}
	mutex_init(&cd->spi_mutex);
	atomic_set(&cd->register_flag, 0);
	spi_set_drvdata(sdev, cd);

	g_smc_core = cd;

	hwlog_info("%s: out\n", __func__);
	return 0;
}

static int smc_remove(struct spi_device *sdev)
{
	struct smc_core_data *cd = spi_get_drvdata(sdev);

	hwlog_info("%s: in\n", __func__);
	mutex_destroy(&cd->spi_mutex);
	kfree(cd);
	cd = NULL;
	return 0;
}

const struct of_device_id g_rsmc_psoc_match_table[] = {
	{.compatible = "huawei,rsmc"},
	{},
};
EXPORT_SYMBOL_GPL(g_rsmc_psoc_match_table);

static const struct spi_device_id g_rsmc_device_id[] = {
	{RSMC_DEVICE_NAME, 0},
	{}
};

MODULE_DEVICE_TABLE(spi, g_rsmc_device_id);

static struct spi_driver g_rsmc_spi_driver = {
	.probe = smc_probe,
	.remove = smc_remove,
	.id_table = g_rsmc_device_id,
	.driver = {
		.name = RSMC_DEVICE_NAME,
		.owner = THIS_MODULE,
		.bus = &spi_bus_type,
		.of_match_table = of_match_ptr(g_rsmc_psoc_match_table),
	},
};

int rsmc_spi_driver_init(void)
{
	int ret = spi_register_driver(&g_rsmc_spi_driver);

	hwlog_info("%s: call spi_register_driver ret %d", __func__, ret);
	return ret;
}

void rsmc_spi_driver_exit(void)
{
	spi_unregister_driver(&g_rsmc_spi_driver);
}

