/*
 * vendor_exception.c
 *
 * Copyright (C) 2018 Huawei Device Co., Ltd.
 * This file is licensed under GPL.
 *
 * Register vendor exceptions.
 */

#ifndef _WSZJGPIOINIT_H_
#define _WSZJGPIOINIT_H_

#include <linux/init.h>
#include <linux/module.h>
#include <linux/ioctl.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/list.h>
#include <linux/errno.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/compat.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/acpi.h>
#include <linux/wait.h>
#include <linux/interrupt.h>

#include <linux/cdev.h>
#include <linux/of_gpio.h>
#include <linux/gpio.h>
#include <linux/spi/spi.h>
#include <linux/spi/spidev.h>

#include <linux/regulator/consumer.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include "securec.h"

#define CLR_BIT 0x0
#define SET_BIT 0x1
#define UWB_FAIL (-1)
#define UWB_SUCC 0
#define POWER_DELAY 200
#define RESET_DELAY 1
#define WAKEUP_DELAY 3
#define RESET_ON 0
#define RESET_OFF 1
#define UWB_GPIO_LOW 0
#define UWB_GPIO_HIGH 1

struct wszj_data {
	dev_t devt;
	spinlock_t spi_lock;
	spinlock_t irq_lock;
	wait_queue_head_t wait;
	struct cdev cdev;
	struct device *device;
	struct spi_device *spi;
	struct list_head device_entry;

	/* TX/RX buffers are NULL unless this device is open (users > 0) */
	struct mutex buf_lock;
	struct mutex wait_lock;
	struct mutex wake_lock;
	unsigned int users;
	u8 *tx_buffer;
	u8 *rx_buffer;
	u32 speed_hz;
	u32 power;
	u32 wait_irq;
	int power_gpio;
	int reset_gpio;
	int wakeup_gpio;
	int cs_gpio;
	int irq_gpio;
	int irq;
};

typedef struct uwb_supply {
	struct regulator *ldo_reg54;
	struct regulator *ldo_reg53;
	struct regulator *ldo_reg25;
	struct regulator *ldo_reg3;
	struct clk *clk32kb;
	int gpio_enable2v85;
	int freq2div38m;
} uwb_power_supply;

typedef struct pmu_reg_control {
	int addr;    /* reg address */
	int value;   /* bit position */
} pmu_reg_control_stu;

int uwb_open_init(struct wszj_data *spidev, uwb_power_supply uwbldo);
int uwb_close_deinit(struct wszj_data *spidev, uwb_power_supply uwbldo);
int uwb_extra_clk_disable(uwb_power_supply uwbldo);
int uwb_extra_clk_enable(uwb_power_supply uwbldo);
int uwb_sim_levelshift_init(int sim_id);
void uwb_hardware_reset(struct wszj_data *wszj);

#endif

