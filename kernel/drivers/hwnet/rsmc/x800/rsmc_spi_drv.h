/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
 * Description: The SPI peripherals are abstracted into four operations:
 * starting and stopping the SPI, and reading and writing the SPI.
 * This module is controlled by other modules..
 * Author: zhongjilei@huawei.com
 * Create: 2020-10-22
 */

#ifndef RSMC_SPI_DRV_H
#define RSMC_SPI_DRV_H

#include <huawei_platform/log/hw_log.h>
#include <linux/amba/pl022.h>
#include <linux/compat.h>
#include <linux/completion.h>
#include <linux/ctype.h>
#include <linux/input.h>
#include <linux/list.h>
#include <linux/platform_device.h>
#include <linux/pm_wakeup.h>
#include <linux/spi/spi.h>
#include <linux/workqueue.h>
#include <uapi/linux/sched/types.h>

#include "rsmc_msg_loop.h"

#define GPIO_LOW 0
#define GPIO_HIGH 1
#define SMC_SPI_SPEED_DEFAULT (6 * 1000 * 1000)
#define SMC_SPI_DEFAULT_BITS_PER_WORD 16

#define SPI_MSLEEP_LONG_TIME 100
#define SPI_MSLEEP_SHORT_TIME 5

#define TCXO_DEFAULT_DELAY 5
#define ANT_DEFAULT_DELAY 10

struct smc_gpios {
	u32 chip_pwr_gpio;
	u32 init_gpio;
	u32 reset_gpio;
	u32 cs_gpio;
	u32 tx_ant_gpio;
	u32 rx_ant_gpio;
	u32 pa_pwr_gpio;
	u32 tcxo_pwr_gpio;
	u32 tcxo_pwr_after_delay_ms;
	u32 ant_gpio_delay_ms;
};

struct smc_spi_config {
	u32 max_speed_hz;
	u16 mode;
	u8 bits_per_word;
	u8 bus_id;
	struct pl022_config_chip pl022_spi_config;
};

struct smc_device {
	char *ic_name;
	char *dev_node_name;
	struct spi_device *sdev;
	struct smc_core_data *smc_core;
	struct smc_gpios *gpios;
	void *private_data;
};

struct smc_feature_config {
	u32 cpu_affinity_offset;
	u32 cpu_affinity_mask;
};

struct smc_core_data {
	struct spi_transfer t;
	struct spi_message msg;
	u8 tx_buff[4];
	u8 rx_buff[4];
	struct spi_device *sdev;
	struct smc_device *smc_dev;
	struct smc_feature_config feature_config;
	struct device_node *smc_node;
	struct smc_spi_config spi_config;
	struct mutex spi_mutex;
	atomic_t register_flag;
	notify_event *msg_notify_event;
	struct smc_gpios gpios;
};

struct smc_core_data *smc_get_core_data(void);
int smc_parse_config(struct device_node *smc_node, struct smc_core_data *cd);
void smc_spi_cs_set(u32 control);
int rsmc_register_dev(struct smc_device *dev);
int rsmc_unregister_dev(struct smc_device *dev);
int rsmc_sync_read(u32 addr, u32 *value);
int rsmc_sync_write(u32 addr, u32 value);
void rsmc_sync_read_write(struct spi_transfer *tf, int len);
int rsmc_spi_driver_init(void);
void rsmc_spi_driver_exit(void);

#endif /* RSMC_SPI_DRV_H */

