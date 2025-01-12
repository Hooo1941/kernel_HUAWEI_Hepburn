/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 *
 * Description: tof8801 driver
 */

#ifndef __TOF8801_DRIVER_H
#define __TOF8801_DRIVER_H

#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/miscdevice.h>
#include <linux/irq.h>
#include <linux/device.h>
#include <linux/string.h>
#include <linux/delay.h>
#include <linux/kfifo.h>
#include <linux/semaphore.h>
#include <linux/jiffies.h>
#include <linux/times.h>
#include <linux/kthread.h>
#include <linux/types.h>
// #include <sensors_class.h>
// #include <media/custom/cam_laser.h>
#include "tof8801.h"
#include "tof8801_bootloader.h"
#include "tof8801_app0.h"

#define DEV_NAME                       "laser"
#define MAX_REGS                       256
#define TOF_GPIO_INT_NAME              "irq"
#define TOF_GPIO_ENABLE_NAME           "enable"
#define TOF_PROP_NAME_POLLIO           "tof,tof_poll_period"
#define TOF_FWDL_TIMEOUT_MSEC          30000

#define SENSOR_PROXIMITY_LABEL         "Time of Flight Proximity Sensor"
#define PROXIMITY_MAX_RANGE            2500
#define PROXIMITY_POWER_CONSUMPTION    18
#define SENSORHAL_TOF_VERSION          2
#define SENSOR_TYPE_TIME_OF_FLIGHT    (40)

#define TOF_NOT_CONFIDENT             0
#define TOF_CONFIDENT                 1
#define TOF_SEMI_CONFIDENT            2
#define RANGESTATUS_CONFIDENT         0
#define RANGESTATUS_SEMI_CONFIDENT    7
#define RANGESTATUS_NOT_CONFIDENCT    255
#define DISTANCE_THRESHOLD            250
#define DISTANCE_THRESHOLD_INFINITY   0
#define CONFIDENCE_THRESHOLD_HIGH     5
#define CONFIDENCE_THRESHOLD_MID      2
#define CONFIDENCE_THRESHOLD_LOW      1
#define CONFIDENCE_THRESHOLD_INFINITY 0
#define NEAREST_DISTANCE_STATUS       8000

struct tof8801_platform_data {
	const char *tof_name;
	uint16_t *gpiod_enable;
	uint16_t *gpiod_interrupt;
	const char *fac_calib_data_fname;
	const char *config_calib_data_fname;
	const char *ram_patch_fname[];
};

typedef STRUCT_KFIFO_REC_2(PAGE_SIZE)tof_rec_2_fifo; /* VARIABLE SIZE fifo */

union tof8801_info_record {
	struct record {
		char app_id;
		char app_ver;
		char req_app_id;
		char reserved_1;
		char reserved_2;
		char reserved_3;
		char reserved_4;
		char reserved_5;
	} record;
	char data[sizeof(struct record)];
};

struct tof_sensor_chip {
	struct mutex lock;
	int poll_period;
	int driver_debug;
	int saved_clk_trim;
	uint16_t gpio_enable;
	uint16_t gpio_interrupt;
	int irq;
	struct timer_list meas_timer;
	union tof8801_info_record info_rec;
	tof_rec_2_fifo tof_output_fifo;
	struct input_dev *obj_input_dev;
	// struct sensors_classdev class_dev;
	struct completion ram_patch_in_progress;
	struct firmware *tof_fw;
	unsigned int xtalk_peak;
	struct tof8801_alg_state_info alg_info;
	struct tof8801_factory_calib_data ext_calib_data; /* 14 Bytes Reg 0x20-0x2D */
	struct tof8801_configuration_data config_data;
	struct tof8801_BL_application BL_app;
	struct tof8801_app0_application app0_app;
	struct tof8801_platform_data *pdata;
	u8 shadow[MAX_REGS];
	struct i2c_client *client;
	struct input_dev *idev;
	struct task_struct *app0_poll_irq;
	int major;
	struct class *cls;
	struct device *clsdev;

	/* raw data from device */
	int distance;
	int confidence;

	/* calibration data */
	hw_laser_ctrl_t *ctrl;
	hwlaser_info_t pinfo;
	hwlaser_RangingData_t udata;
	unsigned int xtalk;
	unsigned char offset_data[14];
};

extern int tof_i2c_read(struct i2c_client *client, char reg, char *buf, int len);
extern int tof_i2c_write(struct i2c_client *client, char reg, const char *buf, int len);
extern int tof_i2c_write_mask(struct i2c_client *client, char reg, const char *val, char mask);
extern int tof8801_get_register(struct i2c_client *client, char reg, char *value);
extern int tof8801_set_register(struct i2c_client *client, char reg, const char value);
extern int tof8801_set_register_mask(struct i2c_client *client, char reg, const char value, const char mask);
extern int tof_wait_for_cpu_ready(struct i2c_client *client);
extern int tof_wait_for_cpu_startup(struct i2c_client *client);
extern int tof_queue_frame(struct tof_sensor_chip *chip, void *buf, int size);
extern int tof_init_info_record(struct tof_sensor_chip *);
extern int tof_hard_reset(struct tof_sensor_chip *chip);
extern int tof_wait_for_cpu_ready_timeout(struct i2c_client *client, unsigned long usec);
extern void tof_dump_i2c_regs(struct tof_sensor_chip *chip, char offset, char end);
extern ssize_t tof8801_not_change_iterations(struct tof_sensor_chip *tof_chip);

int tof_probe(struct i2c_client *client, const struct i2c_device_id *idp);
int tof_remove(struct i2c_client *client);

#endif /* __TOF8801_DRIVER_H */
