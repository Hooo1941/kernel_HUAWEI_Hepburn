/*
 * ext_sensorhub_commu.h
 *
 * head file for communication with external sensorhub
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

#ifndef EXT_SENSORHUB_COMMU_H
#define EXT_SENSORHUB_COMMU_H
#include <linux/types.h>
#include <linux/device.h>
#include <linux/pm_wakeup.h>
#include <linux/interrupt.h>
#include <linux/kthread.h>
#include <linux/semaphore.h>

#define COMMU_SCHED_PRIORITY 2
#define COMMU_DRIVER_COUNT 4
#define COMMU_WRITE_DELAY_TIME_US 30
#define COMMU_WRITE_MAX_DELAY_TIMES 700

struct commu_data;

enum commu_driver {
	SDIO_DRIVER = 0x00,
	SPI_DRIVER = 0x01,
	UART_DRIVER = 0x02,
	SPI_SLAVE_DRIVER = 0x03,
};

struct commu_ops {
	int (*op_commu_init)(struct commu_data *commu_data);
	void (*op_commu_exit)(struct commu_data *commu_data);
	int (*op_read_count)(u32 *read_len);
	int (*op_commu_read)(u8 *buf, u32 len);
	size_t (*op_write_count)(size_t len);
	int (*op_commu_write)(u8 *buf, u32 len);
	int (*op_read_ready)(bool is_ready);
	void (*op_write_ready)(u8 *buf, size_t len);

	irqreturn_t (*op_commu_wake_irq)(int irq, void *arg);
	irqreturn_t (*op_commu_rx_irq)(int irq, void *arg);
};

struct commu_data {
	/* The mutex lock for communication operation */
	struct mutex commu_op_lock;
	unsigned int wake_irq;
	unsigned int rx_ready_irq;
	int wakeup_ap_gpio;
	int wakeup_mcu_gpio;
	int ap_read_ready_gpio;
	int mcu_read_ready_gpio;
	unsigned char *wakeup_ap;
	unsigned char *wakeup_mcu;
	unsigned char *ap_read_ready;
	unsigned char *mcu_read_ready;

	struct task_struct *read_kworker_task;
	struct kthread_work read_work;
	struct kthread_worker read_kworker;

	struct work_struct wakeup_read_work;
	struct work_struct init_work;
	struct wakeup_source *wake_lock;
	struct wakeup_source *user_wake_lock;
	bool initialed;
	bool resumed;

	struct semaphore sr_wake_sema;
	void __iomem *ao_gpio_base;
	struct commu_ops commu_ops;
	enum commu_driver driver;
	int active_handshake;

	int commu_direct_mode;

	int mcu_rst_gpio;
	int rst_flag_gpio;
	int rst_resp_gpio;
};

typedef void (*commu_driver_init)(struct commu_data *commu_data);
void register_commu_driver(enum commu_driver driver,
			   commu_driver_init init_func);

static inline bool highpri_work(struct work_struct *work)
{
	return queue_work_on(WORK_CPU_UNBOUND, system_highpri_wq, work);
}

int commu_request_common_gpio(int *gpio, const char *compat, const char *name);

int commu_request_direction_gpio(int *gpio, const char *compat, const char *name);

struct commu_data *get_commu_data(enum commu_driver driver);

void ext_sensorhub_commu_init(void);

void ext_sensorhub_commu_exit(void);

int commu_init(struct device *dev, struct commu_data *commu_data);

void commu_exit(struct commu_data *commu_data);

ssize_t commu_write(struct commu_data *commu_data,
		    unsigned char *buf, size_t len);

void commu_suspend(enum commu_driver driver);

void commu_resume(enum commu_driver driver);

#endif /* EXT_SENSORHUB_COMMU_H */
